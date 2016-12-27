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
 *         Common functions for UDP services. 
 * \author
 *         Mikhail Churikov mc@unwds.com
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/netstack.h"
#include "net/ip/uip-debug.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#define PRINT6ADDR(addr) PRINTF(" %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8], ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11], ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14], ((uint8_t *)addr)[15])
#define PRINTLLADDR(lladdr) PRINTF(" %02x:%02x:%02x:%02x:%02x:%02x ",lladdr->addr[0], lladdr->addr[1], lladdr->addr[2], lladdr->addr[3],lladdr->addr[4], lladdr->addr[5])
#else
#define PRINTF(...)
#define PRINT6ADDR(addr)
#endif
/*---------------------------------------------------------------------------*/
int 
open_udp_connection(struct uip_udp_conn *server_conn, uint32_t port)
{

  server_conn = udp_new(NULL, 0, NULL);
  if(server_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    return -1;
  }
  udp_bind(server_conn, UIP_HTONS(port));

  if(server_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
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
send_udp_to_mote(struct uip_udp_conn *server_conn, uip_ipaddr_t *ip_addr, uint32_t port, char *data)
{
  uip_ipaddr_copy(&server_conn->ripaddr, ip_addr);
  server_conn->rport = UIP_HTONS(port);

  PRINTF("Send reply to the addr:");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u, data: %s\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport), data);
  uip_udp_packet_send(server_conn, data, strlen(data));
  /* Restore server connection to allow data from any node */
  memset(&server_conn->ripaddr, 1, sizeof(server_conn->ripaddr));
  server_conn->rport = 0;
  return 0;
}
/*---------------------------------------------------------------------------*/
//static void
//print_local_addresses(void)
//{
//  int i;
//  uint8_t state;
//
//  PRINTF("Server IPv6 addresses: ");
//  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
//    state = uip_ds6_if.addr_list[i].state;
//    if(state == ADDR_TENTATIVE || state == ADDR_PREFERRED) {
//      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
//      PRINTF("\n");
//      /* hack to make address "final" */
//      if (state == ADDR_TENTATIVE) {
//	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
//      }
//    }
//  }
//}
/*---------------------------------------------------------------------------*/
