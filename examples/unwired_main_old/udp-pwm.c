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
 *         UDP controlled PWM generation service.  
 * \author
 *         Mikhail Churikov mc@unwds.com
 */
/*---------------------------------------------------------------------------*/

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
#include "pwm.h"
#include "sys/ctimer.h"
#include "udp-common.h"
#include "net/ip/uip-debug.h"
/*---------------------------------------------------------------------------*/
#define DEBUG 1

//#define PWM_PIN_1 2
#define PWM_PIN_1 24
//#define PWM_PIN_2 9//3
#define PWM_PIN_2 3
#define PWM_PIN_3 4
#define PWM_PIN_4 5
//#define PWM_PIN_5 10//6
#define PWM_PIN_5 6
#define PWM_PIN_6 7

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])

#define UDP_CLIENT_PORT	0
#define UDP_SERVER_PORT0	4002

static int counter_ctimer;
static struct ctimer timer_ctimer;
static clock_time_t timeout_ctimer = CLOCK_SECOND / 10;
bool direction = true;
uint8_t pwm_1_duty = 10;
uint8_t pwm_2_duty = 10;
uint8_t pwm_3_duty = 10;
uint8_t pwm_4_duty = 10;
uint8_t pwm_5_duty = 10;
uint8_t pwm_6_duty = 10;

static status_reply_data_t reply_data;
static struct uip_udp_conn *server_conn0;
static struct uip_udp_conn *server_conn1;
/*---------------------------------------------------------------------------*/
PROCESS(udp_pwm_process, "UDP pwm process");
/*---------------------------------------------------------------------------*/
//void
//do_timeout3(void *ptr)
//{
//  counter_ctimer++;
//  uint16_t next_duty = (counter_ctimer % 25) * 4;
//
//  if (next_duty == 0)
//    direction = !direction;
//
//  //printf("next_duty %d\n", next_duty);
//  if (direction) {
//    pwm_set_duty(PWM_TIMER_1, PWM_TIMER_A, 1000, next_duty);
//  } 
//  else {
//    pwm_set_duty(PWM_TIMER_1, PWM_TIMER_A, 1000, 96 - next_duty);
//  }
//
//  /* Re-arm ctimer */
//  ctimer_set(&timer_ctimer, timeout_ctimer, do_timeout3, NULL);
//}
/*---------------------------------------------------------------------------*/
static void
tcpip_handler(void)
{
  uint8_t num = 0;
  if(uip_newdata()) {
    switch ((incoming_command_t)((uint8_t *)uip_appdata)[0]) {
      case GET:
        num = atoi(uip_appdata + 2);
        switch (num) {
          case 1:
            reply_data.pwm_num = 1;
            reply_data.pwm_duty = pwm_1_duty;
            memcpy(&reply_data.ip_addr, &UIP_IP_BUF->srcipaddr, sizeof(uip_ip6addr_t));
            process_post(&udp_pwm_process, PROCESS_EVENT_CONTINUE, &reply_data);
            PRINTF("Status get command.Posted event.\n");
            break;
          case 2:
            reply_data.pwm_num = 2;
            reply_data.pwm_duty = pwm_2_duty;
            memcpy(&reply_data.ip_addr, &UIP_IP_BUF->srcipaddr, sizeof(uip_ip6addr_t));
            process_post(&udp_pwm_process, PROCESS_EVENT_CONTINUE, &reply_data);
            PRINTF("Status get command.Posted event.\n");
            break;
          case 3:
            reply_data.pwm_num = 3;
            reply_data.pwm_duty = pwm_3_duty;
            memcpy(&reply_data.ip_addr, &UIP_IP_BUF->srcipaddr, sizeof(uip_ip6addr_t));
            process_post(&udp_pwm_process, PROCESS_EVENT_CONTINUE, &reply_data);
            PRINTF("Status get command.Posted event.\n");
            break;
          case 4:
            reply_data.pwm_num = 4;
            reply_data.pwm_duty = pwm_4_duty;
            memcpy(&reply_data.ip_addr, &UIP_IP_BUF->srcipaddr, sizeof(uip_ip6addr_t));
            process_post(&udp_pwm_process, PROCESS_EVENT_CONTINUE, &reply_data);
            PRINTF("Status get command.Posted event.\n");
            break;
          case 5:
            reply_data.pwm_num = 5;
            reply_data.pwm_duty = pwm_5_duty;
            memcpy(&reply_data.ip_addr, &UIP_IP_BUF->srcipaddr, sizeof(uip_ip6addr_t));
            process_post(&udp_pwm_process, PROCESS_EVENT_CONTINUE, &reply_data);
            PRINTF("Status get command.Posted event.\n");
            break;
          case 6:
            reply_data.pwm_num = 6;
            reply_data.pwm_duty = pwm_6_duty;
            memcpy(&reply_data.ip_addr, &UIP_IP_BUF->srcipaddr, sizeof(uip_ip6addr_t));
            process_post(&udp_pwm_process, PROCESS_EVENT_CONTINUE, &reply_data);
            PRINTF("Status get command.Posted event.\n");
            break;
          default:
            PRINTF("Got UDP packet on port 4002. Wrong Number.\n");
        }
        break;
      case SET:
#if GREEN_LAMP
        pwm_1_duty = atoi(uip_appdata + 2);
        pwm_set_duty(PWM_TIMER_1, PWM_TIMER_A, 1000, pwm_1_duty);
        PRINTF("SET channel 1 DIO2 \n");
#else
        num = atoi(uip_appdata + 2);
        if ((atoi(uip_appdata + 4) < 0) || (atoi(uip_appdata + 4) > 99)) {
          PRINTF("Wrong incoming PWM level, exit.\n");
          return;
        }
        switch (num) {
          case 1:
            pwm_1_duty = atoi(uip_appdata + 4);
            pwm_set_duty(PWM_TIMER_1, PWM_TIMER_A, 1000, pwm_1_duty);
            PRINTF("SET channel 1 DIO2 \n");
            break;
          case 2:
            pwm_2_duty = atoi(uip_appdata + 4);
            pwm_set_duty(PWM_TIMER_1, PWM_TIMER_B, 1000, pwm_2_duty);
            PRINTF("SET channel 2 DIO3 \n");
            break;
          case 3:
            pwm_3_duty = atoi(uip_appdata + 4);
            pwm_set_duty(PWM_TIMER_2, PWM_TIMER_A, 1000, pwm_3_duty);
            PRINTF("SET channel 3 DIO4\n");
            break;
          case 4:
            pwm_4_duty = atoi(uip_appdata + 4);
            pwm_set_duty(PWM_TIMER_2, PWM_TIMER_B, 1000, pwm_4_duty);
            PRINTF("SET channel 4 DIO5 \n");
            break;
          case 5:
            pwm_5_duty = atoi(uip_appdata + 4);
            pwm_set_duty(PWM_TIMER_3, PWM_TIMER_A, 1000, pwm_5_duty);
            PRINTF("SET channel 5 DIO6\n");
            break;
          case 6:
            pwm_6_duty = atoi(uip_appdata + 4);
            pwm_set_duty(PWM_TIMER_3, PWM_TIMER_B, 1000, pwm_6_duty);
            PRINTF("SET channel 6 DIO7\n");
            break;
          default:
            PRINTF("Got UDP packet on port 4002. Wrong Number.\n");
        }
#endif
        break;
      case TOGGLE:
        num = atoi(uip_appdata + 2);
        switch (num) {
          case 1:
            pwm_1_duty = (pwm_1_duty + 20) % 99;
            pwm_set_duty(PWM_TIMER_1, PWM_TIMER_A, 1000, pwm_1_duty);
            PRINTF("Adjusted channel 1 DIO24 \n");
            break;
          case 2:
            pwm_2_duty = (pwm_2_duty + 20) % 99;
            pwm_set_duty(PWM_TIMER_1, PWM_TIMER_B, 1000, pwm_2_duty);
            PRINTF("Adjusted channel 2 DIO3\n");
            break;
          case 3:
            pwm_3_duty = (pwm_3_duty + 20) % 99;
            pwm_set_duty(PWM_TIMER_2, PWM_TIMER_A, 1000, pwm_3_duty);
            PRINTF("Adjusted channel 3 DIO4\n");
            break;
          case 4:
            pwm_4_duty = (pwm_4_duty + 20) % 99;
            pwm_set_duty(PWM_TIMER_2, PWM_TIMER_B, 1000, pwm_4_duty);
            PRINTF("Adjusted channel 4 DIO5 \n");
            break;
          case 5:
            pwm_5_duty = (pwm_5_duty + 20) % 99;
            pwm_set_duty(PWM_TIMER_3, PWM_TIMER_A, 1000, pwm_5_duty);
            PRINTF("Adjusted channel 5 DIO6\n");
            break;
          case 6:
            pwm_6_duty = (pwm_6_duty + 20) % 99;
            pwm_set_duty(PWM_TIMER_3, PWM_TIMER_B, 1000, pwm_6_duty);
            PRINTF("Adjusted channel 6 DIO7\n");
            break;
          default:
            PRINTF("Got UDP packet on port 4002. Wrong Number.\n");
        }
        break;
      default:
        PRINTF("Wrong command byte in the incoming packet.\n");
    }
  }

  return;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_pwm_process, ev, data)
{
  char buffer[32];
  PROCESS_BEGIN();

  pwm_enable(1200, 10, PWM_TIMER_1, PWM_TIMER_A);
  pwm_set_direction(PWM_TIMER_1, PWM_TIMER_A, PWM_SIGNAL_INVERTED);
  pwm_start(PWM_TIMER_1, PWM_TIMER_A, PWM_PIN_1);//27 - SRF06; 2 - UNWM1; 24 - GreenLamp
  pwm_set_duty(PWM_TIMER_1, PWM_TIMER_A, 1000, 10);

//  pwm_enable(1000, 10, PWM_TIMER_1, PWM_TIMER_B);
//  pwm_set_direction(PWM_TIMER_1, PWM_TIMER_B, PWM_SIGNAL_INVERTED);
//  pwm_start(PWM_TIMER_1, PWM_TIMER_B, PWM_PIN_2);//27 - SRF06; 24 - UNWM1
//  pwm_set_duty(PWM_TIMER_1, PWM_TIMER_B, 1000, 10);
//
//  pwm_enable(1000, 10, PWM_TIMER_2, PWM_TIMER_A);
//  pwm_set_direction(PWM_TIMER_2, PWM_TIMER_A, PWM_SIGNAL_INVERTED);
//  pwm_start(PWM_TIMER_2, PWM_TIMER_A, PWM_PIN_3);//27 - SRF06; 24 - UNWM1
//  pwm_set_duty(PWM_TIMER_2, PWM_TIMER_A, 1000, 10);
//
//  pwm_enable(1000, 10, PWM_TIMER_2, PWM_TIMER_B);
//  pwm_set_direction(PWM_TIMER_2, PWM_TIMER_B, PWM_SIGNAL_INVERTED);
//  pwm_start(PWM_TIMER_2, PWM_TIMER_B, PWM_PIN_4);//27 - SRF06; 24 - UNWM1
//  pwm_set_duty(PWM_TIMER_2, PWM_TIMER_B, 1000, 10);
//
//  pwm_enable(1000, 10, PWM_TIMER_3, PWM_TIMER_A);
//  pwm_set_direction(PWM_TIMER_3, PWM_TIMER_A, PWM_SIGNAL_INVERTED);
//  pwm_start(PWM_TIMER_3, PWM_TIMER_A, PWM_PIN_5);//27 - SRF06; 24 - UNWM1
//  pwm_set_duty(PWM_TIMER_3, PWM_TIMER_A, 1000, 10);
//
//  pwm_enable(1000, 10, PWM_TIMER_3, PWM_TIMER_B);
//  pwm_set_direction(PWM_TIMER_3, PWM_TIMER_B, PWM_SIGNAL_INVERTED);
//  pwm_start(PWM_TIMER_3, PWM_TIMER_B, PWM_PIN_6);//27 - SRF06; 24 - UNWM1
//  pwm_set_duty(PWM_TIMER_3, PWM_TIMER_B, 1000, 10);

  PROCESS_PAUSE();

  PRINTF("MY UDP PWM started\n");

  if (open_udp_connection(server_conn0, UDP_SERVER_PORT0) < 0) {
    PRINTF("PWM: Can't open UDP Port.");
    PROCESS_EXIT();
  }
  server_conn1 = udp_new(NULL, UIP_HTONS(4002), NULL);

  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
    if(ev == PROCESS_EVENT_CONTINUE) {
      if(data != NULL) {
        PRINTF("Woke up and try to send reply.\n");
        PRINTF("Destination address: ");
        PRINT6ADDR(&((status_reply_data_t *)data)->ip_addr);
        PRINTF("\n");
        sprintf(buffer, "%d,%d", ((status_reply_data_t *)data)->pwm_num, 
                                 ((status_reply_data_t *)data)->pwm_duty);
        send_udp_to_mote(server_conn1, &((status_reply_data_t *)data)->ip_addr, 
                          4002, buffer);
        PRINTF("Sended.\n");
      }
    }
  }
  printf("disable --- OK\r\n");

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
