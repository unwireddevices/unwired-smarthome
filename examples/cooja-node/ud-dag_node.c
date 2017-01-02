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
 *         Mikhail Churikov mc@unwds.com
 */
 /*---------------------------------------------------------------------------*/


#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "simple-udp.h"


#include <string.h>
#include <stdio.h>

#include "ud-button.h"

/*---------------------------------------------------------------------------*/
#define DEBUG 1
#include "net/ip/uip-debug_UD.h"
/*---------------------------------------------------------------------------*/

#define MESSAGE_PORT 4003

#define MAX_PAYLOAD_LEN    60
#define MSG_INTERVAL       (10 * CLOCK_SECOND)

/*---------------------------------------------------------------------------*/
static struct uip_udp_conn *client_conn = NULL;
static struct etimer et;
static uip_ip6addr_t dest_addr;

/*---------------------------------------------------------------------------*/
PROCESS(rpl_node_process, "RPL-node process");
/*---------------------------------------------------------------------------*/
static char *
add_ipaddr(char *buf, const uip_ipaddr_t *addr)
{
  uint16_t a;
  unsigned int i;
  int f;
  char *p = buf;

  for(i = 0, f = 0; i < sizeof(uip_ipaddr_t); i += 2) {
    a = (addr->u8[i] << 8) + addr->u8[i + 1];
    if(a == 0 && f >= 0) {
      if(f++ == 0) {
        p += sprintf(p, "::");
      }
    } else {
      if(f > 0) {
        f = -1;
      } else if(i > 0) {
        p += sprintf(p, ":");
      }
      p += sprintf(p, "%04x", a);
    }
  }
  return p;
}
/*---------------------------------------------------------------------------*/
static void
timeout_handler(void)
{
  static int seq_id;
  char buf[MAX_PAYLOAD_LEN];
  int i;
  uip_ip6addr_t *globaladdr = NULL;
  uint16_t dest_port = MESSAGE_PORT;
  int has_dest = 0;
  rpl_dag_t *dag;
  char *end;
  
  connect_info_t message_for_button;
  message_for_button.connected = 0;
  message_for_button.root_addr = NULL;

  uip_ds6_addr_t *addr_desc = uip_ds6_get_global(ADDR_PREFERRED);
  if(addr_desc != NULL) {
    globaladdr = &addr_desc->ipaddr;
    dag = rpl_get_any_dag();
    if(dag) {
      uip_ipaddr_copy(&dest_addr, globaladdr);
      memcpy(&dest_addr.u8[8], &dag->dag_id.u8[8], sizeof(uip_ipaddr_t) / 2);
      has_dest = 1;
    }
  }


}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(rpl_node_process, ev, data)
{

  PROCESS_BEGIN();

  printf("RPL Node: started\n");

  memset(&dest_addr, 0, sizeof(uip_ipaddr_t));

  etimer_set(&et, MSG_INTERVAL);
  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {
      timeout_handler();
      etimer_set(&et, MSG_INTERVAL);
    } else if(ev == tcpip_event) {
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

