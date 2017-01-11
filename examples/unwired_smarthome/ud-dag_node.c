/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

 /*---------------------------------------------------------------------------*/
 /*
 * \file
 *         RPL-node service for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 *         
 */
 /*---------------------------------------------------------------------------*/

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "dev/leds.h"
#include "cc26xx/board.h"
#include "net/ip/uip-debug.h"

#include <string.h>
#include <stdio.h>
#include "simple-udp.h"

#include "ud-dag_node.h"

#include "ti-lib.h"
#include "ud_binary_protocol.h"

#ifdef IF_UD_BUTTON
    #include "ud-button.h"
#endif

#ifdef IF_UD_RELAY
    #include "ud-relay.h"
#endif

#include "fake_headers.h"
/*---------------------------------------------------------------------------*/
//#define DEBUG 1
//#include "net/ip/uip-debug_UD.h"
/*---------------------------------------------------------------------------*/
#define MIN_INTERVAL                (5 * CLOCK_SECOND)
#define MAX_INTERVAL                (50 * CLOCK_SECOND)
#define MAX_NON_ANSWERED_PINGS      5
/*---------------------------------------------------------------------------*/
struct simple_udp_connection udp_connection; //struct for simple_udp_send
uint8_t dag_active = 0; //set to 1, if rpl root found and answer to join packet
uint8_t non_answered_ping = 0;
uip_ip6addr_t root_addr;
clock_time_t dag_interval = MIN_INTERVAL;
/*---------------------------------------------------------------------------*/
PROCESS(dag_node_process, "DAG-node process");
/*---------------------------------------------------------------------------*/
static void
udp_receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
    led_on(LED_A);
    if (data[0] == PROTOCOL_VERSION_V1 && data[1] == CURRENT_DEVICE_VERSION) {
      switch ( data[2] ) {
      case DATA_TYPE_CONFIRM:
          printf("DEBUG: DAG join packet confirmation received, DAG active\n");
          led_off(LED_A);
          dag_active = 1;
          root_addr = *sender_addr;
          non_answered_ping = 0;
          break;
      case DATA_TYPE_COMMAND:
          printf("DEBUG: Command packet received\n");
          static struct command_data message_for_main_process;
          message_for_main_process.ability_target = data[3];
          message_for_main_process.ability_number = data[4];
          message_for_main_process.ability_state = data[5];
          process_post(&main_process, PROCESS_EVENT_CONTINUE, &message_for_main_process);
          break;
      default:
          printf("Incompatible data type!\n");
          break;
      }
  }
  else  {
      printf("DEBUG: Incompatible device or protocol version!\n");
  }
  led_off(LED_A);
}

void
send_join_packet(const uip_ip6addr_t *dest_addr, struct simple_udp_connection *connection)
{
    uint8_t lenght = 10;
    uint8_t buf[lenght];
    buf[0] = PROTOCOL_VERSION_V1;
    buf[1] = CURRENT_DEVICE_VERSION;
    buf[2] = DATA_TYPE_JOIN;
    buf[3] = CURRENT_DEVICE_GROUP;
    buf[4] = CURRENT_DEVICE_SLEEP_TYPE;
    buf[5] = CURRENT_ABILITY_1BYTE; //TODO: заменить на нормальную схему со сдвигами
    buf[6] = CURRENT_ABILITY_2BYTE;
    buf[7] = CURRENT_ABILITY_3BYTE;
    buf[8] = CURRENT_ABILITY_4BYTE;
    buf[9] = DATA_RESERVED;
    simple_udp_sendto(connection, buf, lenght + 1, dest_addr);
}

static void
dag_root_find(void)
{
    rpl_dag_t *dag;
    uip_ip6addr_t addr;

    uip_ds6_addr_t *addr_desc = uip_ds6_get_global(ADDR_PREFERRED);
    if (addr_desc != NULL) {
        dag = rpl_get_any_dag();
        if (dag) {
            led_blink(LED_A);
            if (dag->instance->def_route) {
                if (dag_active == 0) {
                    uip_ip6addr_copy(&addr, &dag->instance->def_route->ipaddr);
                    printf("RPL: default route destination: ");
                    uip_debug_ip6addr_print(&addr);
                    printf("\n");

                    printf("DAG node: send join packet to root \n");
                    send_join_packet(&addr, &udp_connection);
                    if (non_answered_ping < 100) {
                        non_answered_ping++;
                    }
                }
            }
            else
            {
                //printf("RPL: address destination: none \n");
                dag_active = 0;
            }
        }
    }

    if (non_answered_ping > MAX_NON_ANSWERED_PINGS) {
        dag_active = 0;
    }
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(dag_node_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer dag_timer;
  simple_udp_register(&udp_connection, UDP_DATA_PORT, NULL, UDP_DATA_PORT, udp_receiver);

  PROCESS_PAUSE();

  if (RPL_CONF_LEAF_ONLY == 1)
      rpl_set_mode(RPL_MODE_LEAF);
  else
      rpl_set_mode(RPL_MODE_MESH);

  printf("DAG Node: started, %s mode\n", rpl_get_mode() ==  RPL_MODE_LEAF ? "leaf" : "no-leaf");

  //led_on(LED_A);

  while(1) {
     if (dag_active == 0 && dag_interval != MIN_INTERVAL && non_answered_ping < 10) {
         dag_interval = MIN_INTERVAL;
         printf("DAG: Change timer to SHORT interval\n");
     }
     if ((dag_active == 1 && dag_interval != MAX_INTERVAL) || non_answered_ping > 10) {
         dag_interval = MAX_INTERVAL;
         printf("DAG: Change timer to LONG interval\n");
     }
    etimer_set(&dag_timer, dag_interval + (random_rand() % dag_interval));
    if (non_answered_ping > 0)
        printf("DAG: Non-answer ping count: %u\n", non_answered_ping);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&dag_timer));
    dag_root_find();

  }

  PROCESS_END();
}
