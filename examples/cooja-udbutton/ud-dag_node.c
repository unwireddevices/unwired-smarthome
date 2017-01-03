#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/rpl/rpl.h"
#include "net/ip/uip.h"
#include "simple-udp.h"
#include <string.h>
#include <stdio.h>
#include "ud-dag_node.h"

#define DEBUG 1
#include "net/ip/uip-debug_UD.h"

#define MSG_INTERVAL       (1 * CLOCK_SECOND)

static struct uip_udp_conn *client_conn = NULL;
static struct etimer et;
static uip_ip6addr_t dest_addr;

char udp_message_buf[20]; //buffer for simple_udp_send
static struct simple_udp_connection unicast_connection; //struct for simple_udp_send


PROCESS(rpl_node_process, "RPL-node process");
AUTOSTART_PROCESSES(&rpl_node_process);
PROCESS_THREAD(rpl_node_process, ev, data)
{

  PROCESS_BEGIN();

  memset(&dest_addr, 0, sizeof(uip_ipaddr_t));

  etimer_set(&et, MSG_INTERVAL);
  while(1) {
    PROCESS_YIELD();
    if(etimer_expired(&et)) {
      

  uip_ip6addr_t *globaladdr = NULL;
  rpl_dag_t *dag;
  

  uip_ds6_addr_t *addr_desc = uip_ds6_get_global(ADDR_PREFERRED);
  if(addr_desc != NULL) {
    globaladdr = &addr_desc->ipaddr;
    dag = rpl_get_any_dag();
    if(dag) {
      uip_ipaddr_copy(&dest_addr, globaladdr);
      memcpy(&dest_addr.u8[8], &dag->dag_id.u8[8], sizeof(uip_ipaddr_t) / 2);
      sprintf(udp_message_buf, "ping", 1);
      PRINTF("RPL Node: Client sending to: ");
      PRINT6ADDR(&dest_addr);
      PRINTF("\n");
      simple_udp_register(&unicast_connection, 4003, NULL, 4003, NULL); //register simple_udp_connection
      simple_udp_sendto(&unicast_connection, udp_message_buf, strlen(udp_message_buf) + 1, &dest_addr);
    }
  }


      etimer_set(&et, MSG_INTERVAL);
    } 
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

