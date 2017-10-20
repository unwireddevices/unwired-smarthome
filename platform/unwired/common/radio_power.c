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
 */

/*---------------------------------------------------------------------------*/
/*
 * \file
 *         Radio power on/off functions for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 *
 */
/*---------------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "clock.h"

#include "net/rpl/rpl.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ip/uip-debug.h"
#include "net/link-stats.h"

#include "dev/leds.h"
#include "sys/clock.h"

#include "xxf_types_helper.h"

#include "radio_power.h"

/*---------------------------------------------------------------------------*/

static struct ctimer net_off_timer;
volatile uint8_t radio_mode;

/*---------------------------------------------------------------------------*/

void net_on(uint8_t mode)
{
   if (CLASS == CLASS_B)
   {
      if (mode == RADIO_ON_NORMAL && radio_mode == RADIO_FREEDOM)
      {
         NETSTACK_MAC.on();
         uip_ds_6_interval_set(CLOCK_SECOND/2);
         printf("RADIO: Radio ON\n");
      }

      if (mode == RADIO_ON_TIMER_OFF && radio_mode == RADIO_FREEDOM)
      {
         NETSTACK_MAC.on();
         uip_ds_6_interval_set(CLOCK_SECOND/2);
         printf("RADIO: Radio ON\n");
         net_off(RADIO_OFF_ON_TIMER);
      }
   }
}

/*---------------------------------------------------------------------------*/

static void net_off_timer_now(void *ptr)
{
   printf("RADIO: Radio OFF on timer expired\n");
   NETSTACK_MAC.off(0);
}

/*---------------------------------------------------------------------------*/


void net_off(uint8_t mode)
{
   if (CLASS == CLASS_B)
   {

      uip_ds_6_interval_set(CLOCK_SECOND * 20);

      if (mode == RADIO_OFF_ON_TIMER && radio_mode == RADIO_FREEDOM)
      {
         ctimer_reset(&net_off_timer);
         ctimer_set(&net_off_timer, RADIO_OFF_DELAY, net_off_timer_now, NULL);
      }


      if (mode == RADIO_OFF_NOW && radio_mode == RADIO_FREEDOM)
      {
         ctimer_stop(&net_off_timer);
         printf("RADIO: Radio OFF immediately\n");
         NETSTACK_MAC.off(0);
      }
   }

}

/*---------------------------------------------------------------------------*/

void net_mode(uint8_t mode)
{
   if (CLASS == CLASS_B)
   {
      radio_mode = mode;
   }
}
/*---------------------------------------------------------------------------*/

