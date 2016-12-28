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
 *
 * This file is part of the Contiki operating system.
 *
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
#include "ud-world.h"

#define DEBUG 1
#include "net/ip/uip-debug.h"

typedef enum {
  GET = 'g',
  SET = 's',
  TOGGLE = 't'
} incoming_command_t;

typedef struct {
  uip_ipaddr_t ip_addr;
  uint8_t leds_status;
} status_relpy_data_t;
status_relpy_data_t reply_data;
#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define UDP_CLIENT_PORT	0
#define UDP_SERVER_PORT1	4001

static struct uip_udp_conn *server_conn0;
static struct uip_udp_conn *server_conn1;
uip_ip6addr_t dest_ip_addr;
static struct ctimer timer_ctimer;

PROCESS_NAME(udp_leds_process);
PROCESS(udp_leds_process, "UDP GPIO Leds control process");
/*---------------------------------------------------------------------------*/
void
do_timeout_on(void *ptr)
{
  leds_on(LEDS_GREEN);
  PRINTF("Timer event ON.\n");
  /* Re-arm ctimer */
  //ctimer_set(&timer_ctimer, CLOCK_SECOND / 10, do_timeout, NULL);
}
/*---------------------------------------------------------------------------*/
void
do_timeout_off(void *ptr)
{
  leds_off(LEDS_YELLOW);
  PRINTF("Timer event OFF.\n");
  /* Re-arm ctimer */
  //ctimer_set(&timer_ctimer, CLOCK_SECOND / 10, do_timeout, NULL);
}
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  uint8_t num = 0;
  uint8_t target_state = 0;
  uint8_t leds_status = 0;
  uip_ipaddr_t *reply_ip_addr;
  if(uip_newdata()) {
    //memset(buffer, 0, MAX_MSG_SIZE);
    //msg_len = MIN(uip_datalen(), MAX_MSG_SIZE - 1);
    switch ((incoming_command_t)((uint8_t *)uip_appdata)[0]) {
      case GET:
        //uip_ipaddr_copy(reply_ip_addr, &UIP_IP_BUF->srcipaddr);
        leds_status = leds_get();
        PRINTF("Got GPIO Status request. Leds status = 0x%X\n", leds_status);
        reply_data.leds_status = leds_status;
        memcpy(&reply_data.ip_addr, &UIP_IP_BUF->srcipaddr, sizeof(uip_ip6addr_t));
        process_post(&udp_leds_process, PROCESS_EVENT_CONTINUE, &reply_data);
        //send_udp_to_mote_leds(reply_ip_addr, 4001, "1|1");
//        uip_ipaddr_copy(&server_conn0->ripaddr, &UIP_IP_BUF->srcipaddr);
//        server_conn0->rport = UIP_HTONS(4001);
//        //sprintf(buf, "%d!", 99);
//
//        uip_udp_packet_send(server_conn0, "1|1", 4);
        break;
      case SET:
        num = atoi(uip_appdata + 2);
        target_state = atoi(uip_appdata + 4);
//        target_state = atoi(uip_appdata + 2);
        PRINTF("Got GPIO_SET command. Desired state: %d\n", target_state);
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
        switch(num) {
          case 1:
            if (target_state == 1) {
              leds_on(LEDS_GREEN);
            }
            else {
              leds_off(LEDS_GREEN);
            }
            break;
          case 2:
            if (target_state == 1) {
              leds_on(LEDS_YELLOW);
            }
            else {
              leds_off(LEDS_YELLOW);
            }
            break;
          default:
            PRINTF("Got UDP packet on port 4001. Wrong Number.\n");
        }
        break;
      case TOGGLE:
        num = atoi(uip_appdata + 2);
        switch(num) {
          case 1:
            leds_toggle(LEDS_GREEN);
            PRINTF("Toggle 1.\n");
            break;
          case 2:
            PRINTF("Toggle 2.\n");
            leds_toggle(LEDS_YELLOW);
            break;
          default:
            PRINTF("Got UDP packet on port 4001. Wrong Number.\n");
        }
        break;
      default:
        PRINTF("Wrong command byte in the incoming packet.\n");
    }

    //PRINTF("Got UDP packet on port 4001. Toggle out.\n");
    //leds_toggle(LEDS_GREEN);
    /* Copy data */
    //memcpy(buffer, uip_appdata, msg_len);
    //printf("%s", (char *)buffer);
  }

  return;
}
/*---------------------------------------------------------------------------*/
static int open_udp_connection(struct uip_udp_conn *server_conn, uint32_t port)
{
  /* The data sink runs with a 100% duty cycle in order to ensure high 
     packet reception rates. */
  //NETSTACK_MAC.off(1);

  server_conn = udp_new(NULL, UIP_HTONS(UDP_CLIENT_PORT), NULL);
  if(server_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    return -1;
  }
  udp_bind(server_conn, UIP_HTONS(port));

  if(server_conn == NULL) {
    printf("No UDP connection available, exiting the process!\n");
    return -1;
  }
  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));
  return 1;
}
/*---------------------------------------------------------------------------*/
int
send_udp_to_mote_leds(uip_ipaddr_t *ip_addr, uint32_t port, char *data)
{
  uip_ipaddr_copy(&server_conn1->ripaddr, ip_addr);
  server_conn1->rport = UIP_HTONS(port);
  //sprintf(buf, "%d!", 99);

  PRINTF("Send reply to the addr:");
  PRINT6ADDR(&server_conn1->ripaddr);
  PRINTF(" local/remote port %u/%u, data: %s\n", UIP_HTONS(server_conn1->lport),
         UIP_HTONS(server_conn1->rport), data);
  uip_udp_packet_send(server_conn1, data, strlen(data));
  /* Restore server connection to allow data from any node */
  memset(&server_conn1->ripaddr, 1, sizeof(server_conn1->ripaddr));
  server_conn1->rport = 0;
  return 0;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_gpio_process, ev, data)
{
  PROCESS_BEGIN();

  //aaaa::212:4b00:8fb:7d28
  dest_ip_addr.u16[0] = UIP_HTONS(0xAAAA);
  dest_ip_addr.u16[1] = UIP_HTONS(0x0000);
  dest_ip_addr.u16[2] = UIP_HTONS(0x0000);
  dest_ip_addr.u16[3] = UIP_HTONS(0x0000);
  dest_ip_addr.u16[4] = UIP_HTONS(0x0212);
  dest_ip_addr.u16[5] = UIP_HTONS(0x4B00);
  dest_ip_addr.u16[6] = UIP_HTONS(0x08FB);
  dest_ip_addr.u16[7] = UIP_HTONS(0x7D28);

  PRINTF("Destination address: ");
  PRINT6ADDR(&dest_ip_addr);
  PRINTF("\n");
  PROCESS_PAUSE();

  PRINTF("MY UDP LEDS server started\n");

  if (open_udp_connection(server_conn0, UDP_SERVER_PORT1) < 0)
      PROCESS_EXIT();
  server_conn1 = udp_new(&dest_ip_addr, UIP_HTONS(4001), NULL);

  //send_udp_to_mote_leds(&dest_ip_addr, 4003, "1|1");
  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
    if(ev == PROCESS_EVENT_CONTINUE) {
      if(data != NULL) {
        PRINTF("Woke up and try to send reply.\n");
        PRINTF("Destination address: ");
        PRINT6ADDR(&((status_relpy_data_t *)data)->ip_addr);
        PRINTF("\n");
        //send_udp_to_mote_leds(&((status_relpy_data_t *)data)->ip_addr, 4001, "1|1");
        if((((status_relpy_data_t *)data)->leds_status & LEDS_GREEN) == LEDS_GREEN) {
          send_udp_to_mote_leds(&((status_relpy_data_t *)data)->ip_addr, 4001, "1|1");
          PRINTF("Sended.\n");
        }
        else {
          send_udp_to_mote_leds(&((status_relpy_data_t *)data)->ip_addr, 4001, "1|0");
          PRINTF("Sended.\n");
        }
        if((((status_relpy_data_t *)data)->leds_status & LEDS_YELLOW) == LEDS_YELLOW) {
          send_udp_to_mote_leds(&((status_relpy_data_t *)data)->ip_addr, 4001, "2|1");
          PRINTF("Sended.\n");
        }
        else {
          send_udp_to_mote_leds(&((status_relpy_data_t *)data)->ip_addr, 4001, "2|0");
          PRINTF("Sended.\n");
        }
      }
    }
  }
  printf("disable --- UDP 4001\r\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
