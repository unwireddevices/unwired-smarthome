#include "contiki.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-debug.h"
#include "net/rpl/rpl.h"
#include "sys/node-id.h"
#include "simple-udp.h"
#include <stdio.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
#define DEBUG 1
#include "net/ip/uip-debug_UD.h"
/*---------------------------------------------------------------------------*/

#define UDP_PORT 4003
#define SERVICE_ID 190

#define SEND_INTERVAL		(2 * CLOCK_SECOND)
#define SEND_TIME		 (1 * CLOCK_SECOND)//(random_rand() % (SEND_INTERVAL))

static struct simple_udp_connection unicast_connection;

/*---------------------------------------------------------------------------*/
PROCESS(unicast_sender_process, "Unicast sender example process");
AUTOSTART_PROCESSES(&unicast_sender_process);
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static void
set_global_address(void)
{
  uip_ipaddr_t ipaddr;
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
      printf("\n");
    }
  }
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unicast_sender_process, ev, data)
{
  static struct etimer periodic_timer;
  static struct etimer send_timer;
  uip_ipaddr_t *addr;

  PROCESS_BEGIN();

  set_global_address();



  etimer_set(&periodic_timer, SEND_INTERVAL);
  while(1) {

    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    etimer_reset(&periodic_timer);
    etimer_set(&send_timer, SEND_TIME);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_timer));

    uip_ip6addr_t *globaladdr = NULL;
    rpl_dag_t *dag;
    static uip_ip6addr_t dest_addr;
    uip_ds6_addr_t *addr_desc = uip_ds6_get_global(ADDR_PREFERRED);
    if(addr_desc != NULL) {
      globaladdr = &addr_desc->ipaddr;
        printf("globaladdr: ");
        uip_debug_ipaddr_print(globaladdr); //fd00::212:7402:2:202 IP
        printf("\n");
      dag = rpl_get_any_dag();
        printf("rpl_get_any_dag: ");
        uip_debug_ipaddr_print(dag); //fd00::212:7401:1:101 IP
        printf("\n");
      if(dag) {
        uip_ipaddr_copy(&dest_addr, globaladdr);
        memcpy(&dest_addr.u8[8], &dag->dag_id.u8[8], sizeof(uip_ipaddr_t) / 2);
      }
    }


    if(&dest_addr != NULL) {
      static unsigned int message_number;
      char buf[20];

      printf("Sending unicast to ");
      uip_debug_ipaddr_print(&dest_addr); //Sending unicast to fd00::212:7401:1:101
      printf("\n");
      sprintf(buf, "Message %d", message_number);
      message_number++;
      simple_udp_register(&unicast_connection, UDP_PORT, NULL, UDP_PORT, NULL);
      simple_udp_sendto(&unicast_connection, buf, strlen(buf) + 1, &dest_addr);
    } else {
      printf("Service %d not found\n", SERVICE_ID);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
