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

#include "ud-button.h"
#include "ud-dag_node.h"

#include "ti-lib.h"

/*---------------------------------------------------------------------------*/
#define DEBUG 1
#include "net/ip/uip-debug_UD.h"
/*---------------------------------------------------------------------------*/

#define UDP_DATA_PORT      4004
#define PROTOCOL_VERSION   0x01 //protocol version 1
#define DEVICE_VERSION     0x01 //device version 1
#define MIN_INTERVAL       (5 * CLOCK_SECOND)
#define MAX_INTERVAL       (50 * CLOCK_SECOND)

/*---------------------------------------------------------------------------*/
struct simple_udp_connection udp_connection; //struct for simple_udp_send
uint8_t dag_active = 0; //set to 1, if rpl root found and answer to join packet
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
  //printf("DEBUG: DAG data received from ");
  //uip_debug_ipaddr_print(sender_addr);
  //printf(" on port %d: ", receiver_port);
  //printf("0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X \n",
  //       data[0], data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9]);

  if (data[0] == 0x01 && data[1] == 0x01) { //device and protocol version(0x01, 0x01)
      if (data[2] == 0x03) { //data type(0x03 - Data received confirmation)
          printf("DEBUG: DAG join packet confirmation received, DAG active\n");
          leds_on(LED_A);
          dag_active = 1;
          root_addr = *sender_addr;
      }
  }
  else  {
      printf("DEBUG: Incompatible device or protocol version!\n");
  }
}

void
send_join_packet(const uip_ip6addr_t *dest_addr, struct simple_udp_connection *connection)
{
    char buf[10];
    //---header start---
    buf[0] = PROTOCOL_VERSION;
    buf[1] = DEVICE_VERSION;
    buf[2] = 0x01; //data type(01 - net join packet + device profile)
    //---header end---
    //---data start---
    buf[3] = device_type;
    buf[4] = device_sleep_type;
    buf[5] = device_ability_1;
    buf[6] = device_ability_2;
    buf[7] = device_ability_3;
    buf[8] = device_ability_4;
    buf[9] = 0xFF; //reserved
    //---data end---
    simple_udp_sendto(connection, buf, strlen(buf) + 1, dest_addr);

    //rpl_get_parent() - get parent
}

static void
dag_root_find(void)
{
    rpl_dag_t *dag;
    uip_ip6addr_t addr;

    uip_ds6_addr_t *addr_desc = uip_ds6_get_global(ADDR_PREFERRED);
    if(addr_desc != NULL) {
        dag = rpl_get_any_dag();
        if(dag) {
            led_blink(LED_A);
            if(dag->instance->def_route) {
                if (dag_active == 0) {
                    uip_ip6addr_copy(&addr, &dag->instance->def_route->ipaddr);
                    PRINTF("RPL: default route destination: ");
                    PRINT6ADDR(&addr);
                    PRINTF("\n");

                    PRINTF("DAG node: send join packet to root \n");
                    send_join_packet(&addr, &udp_connection);
                }
            }
            else
            {
                PRINTF("RPL: address destination: none \n");
                dag_active = 0;
            }
        }
    }
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(dag_node_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer dag_timer;
  simple_udp_register(&udp_connection, UDP_DATA_PORT, NULL, UDP_DATA_PORT, udp_receiver);

  printf("DAG Node: started\n");

  while(1) {
     if (dag_active == 0 && dag_interval != MIN_INTERVAL) {
         dag_interval = MIN_INTERVAL;
         printf("DAG: Change timer to min interval\n");
     }
     if (dag_active == 1 && dag_interval != MAX_INTERVAL) {
         dag_interval = MAX_INTERVAL;
         printf("DAG: Change timer to max interval\n");

     }

    etimer_set(&dag_timer, dag_interval);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&dag_timer));
    dag_root_find();

  }

  PROCESS_END();
}
