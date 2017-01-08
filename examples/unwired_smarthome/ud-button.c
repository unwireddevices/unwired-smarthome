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
 * This file is part of the Contiki operating system.
 *
 */

 /*---------------------------------------------------------------------------*/
 /*
 * \file
 *         UDP reporting button service for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
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

#include "button-sensor.h"
#include "cc26xx/board.h"
#include "board-peripherals.h"
#include "dev/watchdog.h"
#include "simple-udp.h"

#include "ud-button.h"
#include "ud-dag_node.h"

#include "ti-lib.h"
#include "ud_binary_protocol.h"

/*---------------------------------------------------------------------------*/
#define DEBUG 1
#include "net/ip/uip-debug_UD.h"
/*---------------------------------------------------------------------------*/
const uint8_t device_ability_1 = 0b10000001; //device ability(1 - buttons)
const uint8_t device_ability_2 = 0b00000001; //device ability(none)
const uint8_t device_ability_3 = 0b00000001; //device ability(none)
const uint8_t device_ability_4 = 0b00000001; //device ability(none)

SENSORS(&button_a_sensor, &button_b_sensor, &button_c_sensor, &button_d_sensor, &button_e_sensor); //register button sensors

PROCESS(udp_button_process, "UDP Buttons control process"); //register main button process
AUTOSTART_PROCESSES(&udp_button_process, &dag_node_process); //set autostart processes
/*---------------------------------------------------------------------------*/
void
send_button_status_packet(const uip_ip6addr_t *dest_addr, struct simple_udp_connection *connection, char button_number)
{
    if(dag_active == 1 && dest_addr != NULL && connection != NULL)
    {
        PRINTF("Buttons control process: send message to RPL root node on ");
        PRINT6ADDR(dest_addr);
        PRINTF("\n");

        char buf[10];
        buf[0] = PROTOCOL_VERSION_V1;
        buf[1] = CURRENT_DEVICE_VERSION;
        buf[2] = DATA_TYPE_SENSOR_DATA;
        buf[3] = DEVICE_TYPE_BUTTON;
        buf[4] = DATA_RESERVED;
        buf[5] = button_number;
        buf[6] = DEVICE_BUTTON_EVENT_CLICK;
        buf[7] = DATA_RESERVED;
        buf[8] = DATA_RESERVED;
        buf[9] = DATA_RESERVED;
        simple_udp_sendto(connection, buf, strlen(buf) + 1, dest_addr);
    }

    led_blink(LED_A);
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(udp_button_process, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("Unwired buttons device. HELL-IN-CODE free. I hope.\n");

  PROCESS_PAUSE();
  
  while(1) {
    PROCESS_YIELD();
    if(ev == sensors_event) {
      if(data == &button_a_sensor) {
        PRINTF("Buttons control process: Button A\n");
        send_button_status_packet(&root_addr, &udp_connection, 0x01);
      }
      if(data == &button_b_sensor) {
        PRINTF("Buttons control process: Button B\n");
        send_button_status_packet(&root_addr, &udp_connection, 0x02);
      }
      if(data == &button_c_sensor) {
        PRINTF("Buttons control process: Button C\n");
        send_button_status_packet(&root_addr, &udp_connection, 0x03);
      }
      if(data == &button_d_sensor) {
        PRINTF("Buttons control process: Button D\n");
        send_button_status_packet(&root_addr, &udp_connection, 0x04);
      }
      if(data == &button_e_sensor) {
        PRINTF("Buttons control process: Button E\n");
        send_button_status_packet(&root_addr, &udp_connection, 0x05);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
