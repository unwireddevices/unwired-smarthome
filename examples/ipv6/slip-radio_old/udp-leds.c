/*
 * Copyright (c) 2016, Unwired Devices LLC. All rights reserved.
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         UDP controlled LEDs service 
 * \author
 *         Mikhail Churikov mc@unwds.com
 */

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"

#include "net/netstack.h"
#include "dev/leds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define DEBUG 1
#include "net/ip/uip-debug.h"

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define UDP_CLIENT_PORT	0
#define UDP_SERVER_PORT1	4001

PROCESS_NAME(udp_leds_process);
PROCESS(udp_leds_process, "UDP GPIO Leds control process");
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
//  uint8_t num = 0;
//  uint8_t target_state = 0;
//  if(uip_newdata()) {
//    switch ((incoming_command_t)((uint8_t *)uip_appdata)[0]) {
//      case GET:
//        leds_status = leds_get();
//        PRINTF("Got GPIO Status request. Leds status = 0x%X\n", leds_status);
//        reply_data_leds.leds_status = leds_status;
//        memcpy(&reply_data_leds.ip_addr, &UIP_IP_BUF->srcipaddr, sizeof(uip_ip6addr_t));
//        process_post(&udp_leds_process, PROCESS_EVENT_CONTINUE, &reply_data_leds);
//        break;
//      case SET:
//#if GREEN_LAMP
//        target_state = atoi(uip_appdata + 2);
//        if (target_state == 1) {
//          leds_on(LEDS_YELLOW);
//          ctimer_set(&timer_ctimer, CLOCK_SECOND / 10, do_timeout_on, NULL);
//          PRINTF("Triac ON. Timer armed.\n");
//        }
//        else {
//          leds_off(LEDS_GREEN);
//          ctimer_set(&timer_ctimer, CLOCK_SECOND / 10, do_timeout_off, NULL);
//          PRINTF("Relay OFF. Timer armed.\n");
//        }
//#else
//        num = atoi(uip_appdata + 2);
//        target_state = atoi(uip_appdata + 4);
//        PRINTF("Got GPIO_SET command. Desired state: %d\n", target_state);
//        memcpy(&reply_data_leds.ip_addr, &UIP_IP_BUF->srcipaddr, sizeof(uip_ip6addr_t));
//        switch(num) {
//          case 1:
//            if (target_state == 1) {
//              leds_on(LEDS_GREEN);
//            }
//            else {
//              leds_off(LEDS_GREEN);
//            }
//            break;
//          case 2:
//            if (target_state == 1) {
//              leds_on(LEDS_YELLOW);
//            }
//            else {
//              leds_off(LEDS_YELLOW);
//            }
//            break;
//          default:
//            PRINTF("Got UDP packet on port 4001. Wrong Number.\n");
//        }
//#endif
//        break;
//      case TOGGLE:
//        num = atoi(uip_appdata + 2);
//        switch(num) {
//          case 1:
//            leds_toggle(LEDS_GREEN);
//            PRINTF("Toggle 1.\n");
//            break;
//          case 2:
//            PRINTF("Toggle 2.\n");
//            leds_toggle(LEDS_YELLOW);
//            break;
//          default:
//            PRINTF("Got UDP packet on port 4001. Wrong Number.\n");
//        }
//        break;
//      default:
//        PRINTF("Wrong command byte in the incoming packet.\n");
//    }
//  }

  return;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_leds_process, ev, data)
{
  PROCESS_BEGIN();


  leds_on(LEDS_GREEN);
//  if (open_udp_connection(server_conn0, UDP_SERVER_PORT1) < 0)
//      PROCESS_EXIT();
//  server_conn1 = udp_new(&dest_ip_addr, UIP_HTONS(4001), NULL);

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
  }
  printf("disable --- UDP 4001\r\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
