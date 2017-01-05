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
 *         Mikhail Churikov mc@unwds.com
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

/*---------------------------------------------------------------------------*/
#define DEBUG 1
#include "net/ip/uip-debug_UD.h"
/*---------------------------------------------------------------------------*/
#define UDP_DATA_PORT       4004
#define PROTOCOL_VERSION    0x01 //protocol version 1
/*---------------------------------------------------------------------------*/
uip_ip6addr_t dest_ip_addr;
uint8_t connected_flag = 0;
char udp_message_buf[20]; //buffer for simple_udp_send
static struct simple_udp_connection button_connection; //struct for simple_udp_send

const int device_version = 0x01; //device version 1
const int device_sleep_type = 0x03; //device sleep type(03 - sleep, int)
const int device_type = 0x01; //device type(01 - buttons/switches)
const int device_ability_1 = 0b10000001; //device ability(1 - buttons)
const int device_ability_2 = 0b00000001; //device ability(none)
const int device_ability_3 = 0b00000001; //device ability(none)
const int device_ability_4 = 0b00000001; //device ability(none)

/*---------------------------------------------------------------------------*/
SENSORS(&button_a_sensor, &button_b_sensor, &button_c_sensor, &button_d_sensor, &button_e_sensor); //register button sensors

PROCESS(udp_button_process, "UDP Buttons control process"); //register main button process
AUTOSTART_PROCESSES(&udp_button_process, &rpl_node_process); //set autostart processes
/*---------------------------------------------------------------------------*/
void
ipv6_addr_copy(uip_ip6addr_t *dest, uip_ip6addr_t *source)
{
  memcpy(dest, source, sizeof(uip_ip6addr_t));
}

static void
udp_data_receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
  printf("Data received from ");
  uip_debug_ipaddr_print(sender_addr);
  printf(" on port %d from port %d with length %d: '%s'\n", receiver_port, sender_port, datalen, data);
}

void
send_button_status_packet(const uip_ip6addr_t dest_addr, struct simple_udp_connection button_connection, char button_number)
{
    PRINTF("Buttons control process: send message to RPL root node on ");
    PRINT6ADDR(&dest_ip_addr);
    PRINTF("\n");

    char buf[10];
    //---header start---
    buf[0] = PROTOCOL_VERSION;
    buf[1] = device_version;
    buf[2] = 0x02; //data type(02 - data from sensors)
    //---header end---

    //---data start---
    buf[3] = 0x01; //Ability data number(0x01 - buttons status)
    buf[4] = 0xFF; //reserved
    buf[5] = button_number; //Button number
    buf[6] = 0x01; //Button status(0x01 - button fast pressed)
    buf[7] = 0xFF; //reserved
    buf[8] = 0xFF; //reserved
    buf[9] = 0xFF; //reserved
    //---data end---
    simple_udp_sendto(&button_connection, buf, strlen(buf) + 1, &dest_addr);
}
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(udp_button_process, ev, data)
{
  PROCESS_BEGIN();
  PRINTF("Buttons control process: started\n");

  simple_udp_register(&button_connection, UDP_DATA_PORT, NULL, UDP_DATA_PORT, udp_data_receiver);
  //register simple_udp_connection for button event

  PROCESS_PAUSE();
  
  while(1) {
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_CONTINUE) {
      if(data != NULL) {
          if (((connect_info_t *)data)->root_addr != NULL)
          {
              PRINTF("DEBUG: IP ");
              uip_debug_ip6addr_print(((connect_info_t *)data)->root_addr);
              PRINTF("\n");
          }
          else
          {
              PRINTF("DEBUG: IP NULL");
          }

        PRINTF("DEBUG: connected = %u \n", ((connect_info_t *)data)->connected);
        PRINTF("DEBUG: 1 = %u\n", 1);
        if(((connect_info_t *)data)->root_addr != NULL && ((connect_info_t *)data)->connected == 1) {
          connected_flag = 1;
          PRINTF("DEBUG: connected_flag = %u\n", connected_flag);
          ipv6_addr_copy(&dest_ip_addr, ((connect_info_t *)data)->root_addr); //replace dest_ip_addr<-rlp_root(see dag node)
          PRINTF("Buttons control process: Found RPL root. Addr: ");
          PRINT6ADDR(&dest_ip_addr);
          PRINTF("\n");
        }
      }
    }
    else
    if(ev == sensors_event) {
      if(data == &button_a_sensor) {
        PRINTF("Buttons control process: Button A\n");
        if(connected_flag == 1) {
          send_button_status_packet(dest_ip_addr, button_connection, 0x01);
        }
        led_blink(LED_B);
      }
      if(data == &button_b_sensor) {
        PRINTF("Buttons control process: Button B\n");
        if(connected_flag == 1) {
            send_button_status_packet(dest_ip_addr, button_connection, 0x02);
        }
        led_blink(LED_B);
      }
      if(data == &button_c_sensor) {
        PRINTF("Buttons control process: Button C\n");
        if(connected_flag == 1) {
            send_button_status_packet(dest_ip_addr, button_connection, 0x03);
        }
        led_blink(LED_B);
      }
      if(data == &button_d_sensor) {
        PRINTF("Buttons control process: Button D\n");
        if(connected_flag == 1) {
            send_button_status_packet(dest_ip_addr, button_connection, 0x04);
        }
        led_blink(LED_B);
      }
      if(data == &button_e_sensor) {
        PRINTF("Buttons control process: Button E\n");
        //lpm_shutdown(BOARD_IOID_KEY_RIGHT, IOC_IOPULL_UP, IOC_WAKE_ON_LOW);
        if(connected_flag == 1) {
            send_button_status_packet(dest_ip_addr, button_connection, 0x05);
        }
        led_blink(LED_B);
      }
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
