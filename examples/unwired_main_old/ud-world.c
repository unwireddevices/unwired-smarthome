/*
 * Copyright (c) 2006, Swedish Institute of Computer Science.
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
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

/*---------------------------INCLUDE-----------------------------------------*/

#include "contiki.h"
//#include "contiki-lib.h"
//#include "contiki-net.h"
//#include "sys/rtimer.h"
#include "net/ip/uip.h"
#include "dev/leds.h"
#include "udp-leds.h"
#include "cetic-6lbr-client.h"
#include "udp-gpio-input.h"

#include "mac.h"
#include "udp-pwm.h"
#include "udp-adc.h"
#include "udp-dali.h"
#include "udp-sht21.h"
#include "udp-lps331.h"
#include "udp-button.h"

//#include <string.h>
#include <stdio.h>

/*---------------------------DEFINES-----------------------------------------*/

#define MACDEBUG 1

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

#define PING6_NB 5
#define PING6_DATALEN 16

#define UIP_IP_BUF                ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_ICMP_BUF            ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

/*---------------------------GLOBAL VARS---------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS(ud_world_process, "Unwired Devices process");
AUTOSTART_PROCESSES( &ud_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ud_world_process, ev, data)
{
  //uint8_t cont = 1;

  PROCESS_BEGIN();
  PRINTF("Unwired Devices process started. Hell inside.\n");

  process_start(&cetic_6lbr_client_process, NULL);
  leds_on(LEDS_RED);

#if UNWM_LEDS
  process_start(&udp_leds_process, NULL);
#endif
#if UNWM_LPS331
  process_start(&udp_lps331_process, NULL);
#endif
#if UNWM_SHT21
  process_start(&udp_sht21_process, NULL);
#endif
#if UNWM_DALI
  process_start(&udp_dali_process, NULL);
#endif
#if UNWM_ADC
  process_start(&udp_adc_process, NULL);
#endif
#if UNWM_GPIO_INPUT
  process_start(&udp_gpio_input_process, NULL);
#endif
#if UNWM_PWM
  process_start(&udp_pwm_process, NULL);
#endif
#if UNWM_BUTTON
  process_start(&udp_button_process, NULL);
#endif

  /*etimer_set(&ping6_periodic_timer, 15*CLOCK_SECOND);

  while(cont) {
    PROCESS_YIELD();
    cont = ping6handler(ev, data);
  }
*/
  PRINTF("END UD\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
