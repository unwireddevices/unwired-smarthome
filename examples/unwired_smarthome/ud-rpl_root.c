/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
 * All rights reserved.
 *
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
 *
 * 
 *  
 */

 /*---------------------------------------------------------------------------*/
 /*
 * \file
 *         RPL-root service for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 */
 /*---------------------------------------------------------------------------*/

#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-debug.h"

#include "simple-udp.h"

#include "net/rpl/rpl.h"

#include <stdio.h>
#include <string.h>

#include "button-sensor.h"
#include "board-peripherals.h"

#define UDP_DATA_PORT      4004
#define PROTOCOL_VERSION   0x01 //protocol version 1
#define DEVICE_VERSION     0x01 //device version 1

#define DEBUG 1
#include "net/ip/uip-debug_UD.h"

static struct simple_udp_connection udp_connection;


SENSORS(&button_a_sensor);

/*---------------------------------------------------------------------------*/
PROCESS(rpl_root_process, "Unwired RPL root and udp data receiver");
AUTOSTART_PROCESSES(&rpl_root_process);
/*---------------------------------------------------------------------------*/
void
send_confirmation_packet(const uip_ip6addr_t *dest_addr, struct simple_udp_connection *connection)
{
    char buf[10];
    //---header start---
    buf[0] = PROTOCOL_VERSION;
    buf[1] = DEVICE_VERSION;
    buf[2] = 0x03; //data type(0x03 - Data received confirmation)
    //---header end---
    //---data start---
    buf[3] = 0xFF; //reserved
    buf[4] = 0xFF; //reserved
    buf[5] = 0xFF; //reserved
    buf[6] = 0xFF; //reserved
    buf[7] = 0xFF; //reserved
    buf[8] = 0xFF; //reserved
    buf[9] = 0xFF; //reserved
    //---data end---
    simple_udp_sendto(connection, buf, strlen(buf) + 1, dest_addr);
}


static void
udp_data_receiver(struct simple_udp_connection *connection,
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

  if (data[0] == 0x01 && data[1] == 0x01) {  //device and protocol version(0x01, 0x01)
      switch ( data[2] ) {
      case 0x01: //data type(0x01 - Join network packet)
          printf("DEBUG: DAG join packet received, confirmation packet sending\n");
          send_confirmation_packet(sender_addr, connection);
          break;
      case 0x02: //data type(0x02 - data from sensors)
          printf("DEBUG: data from sensor received, confirmation packet NOT sending\n");
          printf("Button: 0x%02X type event: 0x%02X\n", data[5], data[6]);
          //send_confirmation_packet(sender_addr, connection);
          break;
      default:
        break;
      }
  }
  else
  {
      printf("Incompatible device or protocol version!\n");
  }
}
/*---------------------------------------------------------------------------*/
static uip_ipaddr_t *
set_global_address(void)
{
  static uip_ipaddr_t ipaddr;
  int i;
  uint8_t state;

  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

  printf("IPv6 addresses: ");
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    state = uip_ds6_if.addr_list[i].state;
    if(uip_ds6_if.addr_list[i].isused &&
       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
      uip_debug_ipaddr_print(&uip_ds6_if.addr_list[i].ipaddr);
      printf(", ");
    }
  }
  printf("\n");

  return &ipaddr;
}
/*---------------------------------------------------------------------------*/
static void
create_rpl_dag(uip_ipaddr_t *ipaddr)
{
  struct uip_ds6_addr *root_if;
  root_if = uip_ds6_addr_lookup(ipaddr);
  if(root_if != NULL) {
    rpl_dag_t *dag;
    uip_ipaddr_t prefix;
    rpl_set_root(RPL_DEFAULT_INSTANCE, ipaddr);
    dag = rpl_get_any_dag();
    uip_ip6addr(&prefix, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
    rpl_set_prefix(dag, &prefix, 64);
    printf("Created a new RPL DAG, i'm root!\n");
  } else {
    printf("Failed to create a new RPL DAG :(\n");
  }
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rpl_root_process, ev, data)
{
  uip_ipaddr_t *ipaddr;

  PROCESS_BEGIN();
  printf("Unwired RLP root and UDP data receiver. HELL-IN-CODE free. I hope.\n");

  ipaddr = set_global_address();
  create_rpl_dag(ipaddr);

  simple_udp_register(&udp_connection, UDP_DATA_PORT, NULL, UDP_DATA_PORT, udp_data_receiver);

  while(1) {
    PROCESS_WAIT_EVENT();
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
