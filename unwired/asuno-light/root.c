/*
 * Copyright (c) 2016, Unwired Devices LLC - http://www.unwireddevices.com/
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

#include "dev/leds.h"
#include "cc26xx/board.h"

#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-debug.h"
#include "simple-udp.h"
#include "net/rpl/rpl.h"

#include <stdio.h>
#include <string.h>

#include "button-sensor.h"
#include "board-peripherals.h"

#include "ti-lib.h"
#include "dev/cc26xx-uart.h"

#include "../ud_binary_protocol.h"
#include "xxf_types_helper.h"
#include "dev/watchdog.h"
#include "root.h"
#include "../root-node.h"


#include "../fake_headers.h" //no move up! not "krasivo"!

#define UART_DATA_POLL_INTERVAL 5 //in main timer ticks, one tick ~8ms
/*---------------------------------------------------------------------------*/

/* Buttons on DIO 1 */
SENSORS(&button_e_sensor_click, &button_e_sensor_long_click);

PROCESS(rpl_root_process,"Unwired RPL root and udp data receiver");
PROCESS(send_command_process,"UDP command sender");

AUTOSTART_PROCESSES(&rpl_root_process);


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(send_command_process, ev, data)
{
   PROCESS_BEGIN();

   static struct etimer send_command_process_timer;
   PROCESS_PAUSE();

   while (1)
   {
      etimer_set(&send_command_process_timer, UART_DATA_POLL_INTERVAL);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_command_process_timer));

      if (command_message.ready_to_send != 0)
      {
         disable_interrupts();
         send_command_packet(&command_message);
         command_message.ready_to_send = 0;
         enable_interrupts();
      }

      if (firmware_message.ready_to_send != 0)
      {
         disable_interrupts();
         send_firmware_packet(&firmware_message);
         firmware_message.ready_to_send = 0;
         enable_interrupts();
      }

      if (firmware_cmd_message.ready_to_send != 0)
      {
         disable_interrupts();
         send_firmware_cmd_packet(&firmware_cmd_message);
         firmware_cmd_message.ready_to_send = 0;
         enable_interrupts();
      }

      if (uart_message.ready_to_send != 0)
      {
         disable_interrupts();
         send_uart_packet(&uart_message);
         uart_message.ready_to_send = 0;
         enable_interrupts();
      }
   }

   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(rpl_root_process, ev, data)
{
   static uip_ipaddr_t *ipaddr = NULL;

   PROCESS_BEGIN();

   printf("Unwired RLP root and UDP data receiver. HELL-IN-CODE free. I hope. \n");

   /* if you do not execute "cleanall" target, rpl-root can build in "leaf" configuration. Diagnostic message */
   if (RPL_CONF_LEAF_ONLY == 1)
   {
      printf("\nWARNING: leaf mode on rpl-root!\n");
   }

   /* Set MESH-mode for dc-power rpl-root(not leaf-mode) */
   rpl_set_mode(RPL_MODE_MESH);

   /* set local address */
   ipaddr = set_global_address(); //не очень понятно, нафига это вообще выносить

   /* make local address as rpl-root */
   create_rpl_dag(ipaddr);

   /* register udp-connection, set incoming upd-data handler(udp_data_receiver) */
   simple_udp_register(&udp_connection, UDP_DATA_PORT, NULL, UDP_DATA_PORT, udp_data_receiver);

   /* set incoming uart-data handler(uart_data_receiver) */
   cc26xx_uart_set_input(&uart_data_receiver);

   /* blink-blink LED */
   led_blink(LED_A);
   led_blink(LED_A);

   /* start flag "data for udp ready" poller process */
   process_start(&send_command_process, NULL);

   while (1)
   {
      PROCESS_WAIT_EVENT();
      if (ev == sensors_event)
      {
         if (data == &button_e_sensor_click)
         {
            printf("Initiating global repair\n");
            rpl_repair_root(RPL_DEFAULT_INSTANCE);
         }
         if (data == &button_e_sensor_long_click)
         {
            led_on(LED_A);
            printf("SYSTEM: Button E long click, reboot\n");
            watchdog_reboot();
         }
      }
   }
   PROCESS_END();
}
