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

/*---------------------------------------------------------------------------*/
#define DEBUG 1
#include "net/ip/uip-debug_UD.h"
/*---------------------------------------------------------------------------*/

#define PROTOCOL_VERSION 0x01 //protocol version 1

#define UDP_DAG_PORT       5005

#define SEND_INTERVAL   (10 * CLOCK_SECOND)
#define SEND_INTERVAL_RANDOM    (random_rand() % (SEND_INTERVAL))

/*---------------------------------------------------------------------------*/
static struct simple_udp_connection dag_node_connection; //struct for simple_udp_send
/*---------------------------------------------------------------------------*/
PROCESS(rpl_node_process, "RPL-node process");
/*---------------------------------------------------------------------------*/
void
send_join_packet(const uip_ip6addr_t dest_addr, struct simple_udp_connection dag_node_connection)
{
    char buf[10];
    //---header start---
    buf[0] = PROTOCOL_VERSION;
    buf[1] = device_version;
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
    simple_udp_sendto(&dag_node_connection, buf, strlen(buf) + 1, &dest_addr);

    //rpl_get_parent() - get parent
}

static void
udp_dag_node_receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  printf("Data received from ");
  uip_debug_ipaddr_print(sender_addr);
  printf(" on port %d from port %d with length %d: '%s'\n", receiver_port, sender_port, datalen, data);
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(rpl_node_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer periodic_timer;
  static struct etimer send_timer;

  connect_info_t message_for_button;
  message_for_button.connected = 0;
  message_for_button.root_addr = NULL;

  //register simple_udp_connection for dag_node
  simple_udp_register(&dag_node_connection, UDP_DAG_PORT, NULL, UDP_DAG_PORT, udp_dag_node_receiver);
  
  printf("DAG Node: started\n");

  etimer_set(&periodic_timer, SEND_INTERVAL_RANDOM);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
  etimer_reset(&periodic_timer);
  while(1) {
    etimer_set(&send_timer, SEND_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

    uip_ip6addr_t *globaladdr = NULL;
    rpl_dag_t *dag;
    static uip_ip6addr_t dest_addr;
    int found_rpl_root = 0;

    uip_ds6_addr_t *addr_desc = uip_ds6_get_global(ADDR_PREFERRED);
    if(addr_desc != NULL) {
      globaladdr = &addr_desc->ipaddr;
      //printf("DAG Node: Local address: ");
      //uip_debug_ip6addr_print(globaladdr);
      printf("\n");
      dag = rpl_get_any_dag();
      //printf("DAG Node: RPL root address: ");
      //uip_debug_ipaddr_print(&dag->dag_id);
      printf("\n");
      if(dag)
      {
        uip_ipaddr_copy(&dest_addr, globaladdr); //не понимаю, что оно делает.
        memcpy(&dest_addr.u8[8], &dag->dag_id.u8[8], sizeof(uip_ipaddr_t) / 2); //не понимаю, что оно делает.

        send_join_packet(dest_addr, dag_node_connection);

        leds_on(LED_A);
        message_for_button.connected = 1;
        message_for_button.root_addr = &dest_addr;
        process_post(&udp_button_process, PROCESS_EVENT_CONTINUE, &message_for_button);

        printf("DAG Node: Sending data to RPL root: ");
        uip_debug_ipaddr_print(&dest_addr);
        printf("\n");
      }
      else
      {
        printf("DAG Node: RPL Root not found\n");
        leds_off(LED_A);
        message_for_button.connected = 0;
        message_for_button.root_addr = NULL;
        process_post(&udp_button_process, PROCESS_EVENT_CONTINUE, &message_for_button);
      }
    }
    else
    {
        printf("DAG Node: RPL Root not found\n");
        leds_off(LED_A);
        message_for_button.connected = 0;
        message_for_button.root_addr = NULL;
        process_post(&udp_button_process, PROCESS_EVENT_CONTINUE, &message_for_button);
    }


    if(found_rpl_root == 1) {

    } 
    else {

    }
  }

  PROCESS_END();
}
