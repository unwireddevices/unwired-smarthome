/*
 * Copyright (c) 2016, Unwired Devices LLC - http://www.unwireddevices.com/
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
 * This file is part of the Contiki operating system.
 *
 */

/*---------------------------------------------------------------------------*/
/*
* \file
*         Incotext-light service for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
* \author
*         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
*/
/*---------------------------------------------------------------------------*/

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ip/uip.h"
#include "net/rpl/rpl.h"

#include "net/netstack.h"
#include "uip-ds6-route.h"
#include "net/ip/uip-debug.h"
#include "dev/leds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "button-sensor.h"
#include "board.h"
#include "board-peripherals.h"
#include "simple-udp.h"

#include "light.h"
#include "../dag_node.h"
#include "gpio-interrupt.h"

#include "xxf_types_helper.h"

#include "ti-lib.h"
#include "clock.h"
#include "../ud_binary_protocol.h"
#include "../flash-common.h"

#include "../fake_headers.h" //no move up! not "krasivo"!

/*---------------------------------------------------------------------------*/
/* UART char iterator */
volatile static uint8_t uart_data_iterator = 0;

volatile static uint8_t uart_returned_data_lenth = 0;


/* UART-buffer */
volatile static uint8_t uart_returned_data_buf[23];

/*---------------------------------------------------------------------------*/


/* Register buttons sensors */
SENSORS(&button_e_sensor_click,
        &button_e_sensor_long_click);

/* register dimmer process */
PROCESS(main_process, "Incotext-light control process");

/* set autostart processes */
AUTOSTART_PROCESSES(&dag_node_process, &main_process);

/*---------------------------------------------------------------------------*/

static int uart_data_receiver(unsigned char uart_char)
{
   if (uart_returned_data_lenth > 0)
   {
      uart_returned_data_buf[uart_data_iterator] = uart_char;

      if (uart_data_iterator < uart_returned_data_lenth)
      {
         uart_data_iterator++;
      }
      else
      {
         struct command_data uart_data;
         uart_data.data_type = DATA_TYPE_UART;
         uart_data.uart_returned_data_lenth = 0;

         for (int i = 0; i <= uart_returned_data_lenth; i++)
         {
            uart_data.payload[i] = uart_returned_data_buf[i];
         }

         send_uart_data(&uart_data);

         uart_returned_data_lenth = 0;
         uart_data_iterator = 0;
      }

   }
   return 1;
}

/*---------------------------------------------------------------------------*/

static void send_uart_command(struct command_data *uart_data)
{
   cc26xx_uart_write_byte(uart_data->payload[0]);
   cc26xx_uart_write_byte(uart_data->payload[1]);
   cc26xx_uart_write_byte(uart_data->payload[2]);
   cc26xx_uart_write_byte(uart_data->payload[3]);
   cc26xx_uart_write_byte(uart_data->payload[4]);
   uart_returned_data_lenth_iterator = uart_data->uart_returned_data_lenth;
}



/*---------------------------------------------------------------------------*/

PROCESS_THREAD(main_process, ev, data)
{
   PROCESS_BEGIN();

   static struct command_data *message_data = NULL;

   PROCESS_PAUSE();

   printf("Unwired Incotext-light device. HELL-IN-CODE free. I hope.\n");

   /* set incoming uart-data handler(uart_data_receiver) */
   cc26xx_uart_set_input(&uart_data_receiver);

   while (1)
   {
      PROCESS_YIELD();
      if (ev == PROCESS_EVENT_CONTINUE)
      {
         message_data = data;
         if (message_data->data_type == DATA_TYPE_UART)
         {
            send_uart_command(message_data);
         }
      }
      if (ev == sensors_event)
      {
         if (data == &button_e_sensor_click)
         {
            printf("BCP: Button E click\n");
            ext_flash_probe();
         }
      }
   }

   PROCESS_END();
}
