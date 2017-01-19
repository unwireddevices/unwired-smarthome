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

#include "net/rpl/rpl-private.h"

/*---------------------------------------------------------------------------*/

PROCESS(main_process, "UD Buttons control process"); //register main button process
AUTOSTART_PROCESSES(&dag_node_process); //set autostart processes

/*---------------------------------------------------------------------------*/
void send_sensor_event(struct sensor_packet *packet,
                 const uip_ip6addr_t *dest_addr,
                 struct simple_udp_connection *connection)
{
    uint8_t lenght = 10;
    uint8_t udp_buffer[lenght];
    udp_buffer[0] = packet->protocol_version;
    udp_buffer[1] = packet->device_version;
    udp_buffer[2] = packet->data_type;

    udp_buffer[3] = packet->number_ability;
    udp_buffer[4] = DATA_RESERVED;
    udp_buffer[5] = packet->sensor_number;
    udp_buffer[6] = packet->sensor_event;
    udp_buffer[7] = DATA_RESERVED;
    udp_buffer[8] = DATA_RESERVED;
    udp_buffer[9] = DATA_RESERVED;
    simple_udp_sendto(connection, udp_buffer, lenght + 1, dest_addr);
}

/*---------------------------------------------------------------------------*/

void send_button_status_packet(const uip_ip6addr_t *dest_addr,
                          struct simple_udp_connection *connection,
                          uint8_t button_number,
                          int click_type)
{

    if(dest_addr != NULL && connection != NULL)//(dag_active == 1 && dest_addr != NULL && connection != NULL)
    {
        printf("Buttons: send message to RPL root node\n");

        struct sensor_packet button_sensor_packet;
        button_sensor_packet.protocol_version = CURRENT_PROTOCOL_VERSION;
        button_sensor_packet.device_version = CURRENT_DEVICE_VERSION;
        button_sensor_packet.data_type = DATA_TYPE_SENSOR_DATA;
        button_sensor_packet.number_ability = DEVICE_ABILITY_BUTTON;
        button_sensor_packet.sensor_number = button_number;
        button_sensor_packet.sensor_event = click_type;
        send_sensor_event(&button_sensor_packet, dest_addr, connection);
    }

    led_blink(LED_A);
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(main_process, ev, data)
{
  PROCESS_BEGIN();
  printf("Unwired buttons device. HELL-IN-CODE free. I hope.\n");

  PROCESS_PAUSE();
  
  static struct etimer debug_timer1;
  while(1) {
    etimer_set(&debug_timer1, (CLOCK_SECOND/10));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&debug_timer1));
    send_button_status_packet(&root_addr, &udp_connection, 'b', DEVICE_ABILITY_BUTTON_EVENT_CLICK);
  }

  while(1) {
    PROCESS_YIELD();
    if(ev == sensors_event) {
      if(data == &button_a_sensor_click) {
        printf("Buttons control process: Button A click\n");
        send_button_status_packet(&root_addr, &udp_connection, 'a', DEVICE_ABILITY_BUTTON_EVENT_CLICK);
      }
      if(data == &button_a_sensor_long_click) {
        printf("Buttons control process: Button A long click\n");
        send_button_status_packet(&root_addr, &udp_connection, 'a', DEVICE_ABILITY_BUTTON_EVENT_LONG_CLICK);
      }
      if(data == &button_b_sensor_click) {
        printf("Buttons control process: Button B click\n");
        send_button_status_packet(&root_addr, &udp_connection, 'b', DEVICE_ABILITY_BUTTON_EVENT_CLICK);
      }
      if(data == &button_b_sensor_long_click) {
        printf("Buttons control process: Button B long click\n");
        send_button_status_packet(&root_addr, &udp_connection, 'b', DEVICE_ABILITY_BUTTON_EVENT_LONG_CLICK);
      }
      if(data == &button_c_sensor_click) {
        printf("Buttons control process: Button C click\n");
        send_button_status_packet(&root_addr, &udp_connection, 'c', DEVICE_ABILITY_BUTTON_EVENT_CLICK);
      }
      if(data == &button_c_sensor_long_click) {
        printf("Buttons control process: Button C long click\n");
        send_button_status_packet(&root_addr, &udp_connection, 'c', DEVICE_ABILITY_BUTTON_EVENT_LONG_CLICK);
      }
      if(data == &button_d_sensor_click) {
        printf("Buttons control process: Button D click\n");
        send_button_status_packet(&root_addr, &udp_connection, 'd', DEVICE_ABILITY_BUTTON_EVENT_CLICK);
      }
      if(data == &button_d_sensor_long_click) {
        printf("Buttons control process: Button D long click\n");
        send_button_status_packet(&root_addr, &udp_connection, 'd', DEVICE_ABILITY_BUTTON_EVENT_LONG_CLICK);
      }

    }
  }

  PROCESS_END();
}
