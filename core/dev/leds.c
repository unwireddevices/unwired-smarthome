/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
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

#include "dev/leds.h"
#include "sys/clock.h"
#include "sys/energest.h"

static unsigned char leds;
/*---------------------------------------------------------------------------*/
static void
show_leds(unsigned char new_leds)
{
  unsigned char changed;
  changed = leds ^ new_leds;
  leds = new_leds;

  if(changed & LED_A) {
    /* Green did change */
    if(leds & LED_A) {
      ENERGEST_ON(ENERGEST_TYPE_LED_A);
    } else {
      ENERGEST_OFF(ENERGEST_TYPE_LED_A);
    }
  }
  if(changed & LED_B) {
    if(leds & LED_B) {
      ENERGEST_ON(ENERGEST_TYPE_LED_B);
    } else {
      ENERGEST_OFF(ENERGEST_TYPE_LED_B);
    }
  }
  if(changed & LED_C) {
    if(leds & LED_C) {
      ENERGEST_ON(ENERGEST_TYPE_LED_C);
    } else {
      ENERGEST_OFF(ENERGEST_TYPE_LED_C);
    }
  }
  leds_arch_set(leds);
}
/*---------------------------------------------------------------------------*/
void
leds_init(void)
{
  leds_arch_init();
  leds = 0;
}
/*---------------------------------------------------------------------------*/
void
leds_blink(void)
{
  /* Blink all leds that were initially off. */
  unsigned char blink;
  blink = ~leds;
  led_toggle(blink);

  clock_delay(400);

  led_toggle(blink);
}


/*---------------------------------------------------------------------------*/
unsigned char
leds_get(void) {
  return leds_arch_get();
}
/*---------------------------------------------------------------------------*/
void
leds_set(unsigned char ledv)
{
  show_leds(ledv);
}
/*---------------------------------------------------------------------------*/
void
led_blink(unsigned char ledv)
{
    show_leds(leds ^ ledv);
    clock_delay(200);
    show_leds(leds ^ ledv);
}
void
led_blink_long(unsigned char ledv)
{
    show_leds(leds ^ ledv);
    clock_delay(1000);
    show_leds(leds ^ ledv);
}
/*---------------------------------------------------------------------------*/
void
led_on(unsigned char ledv)
{
  show_leds(leds | ledv);
}
/*---------------------------------------------------------------------------*/
void
led_off(unsigned char ledv)
{
  show_leds(leds & ~ledv);
}
/*---------------------------------------------------------------------------*/
void
led_toggle(unsigned char ledv)
{
  show_leds(leds ^ ledv);
}
/*---------------------------------------------------------------------------*/
