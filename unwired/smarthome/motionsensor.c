/*
 * Copyright (c) 2016, Unwired Devices LLC - http://www.unwireddevices.com/
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Unwired Devices nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
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
*         Motion sensor service for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
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
#include "gpio-interrupt.h"
#include "board.h"
#include "board-peripherals.h"
#include "dev/watchdog.h"
#include "simple-udp.h"

#include "motionsensor.h"

#include "ti-lib.h"
#include "../ud_binary_protocol.h"
#include "../dag_node.h"

#include "net/rpl/rpl-private.h"

#define BUTTON_GPIO_CFG         (IOC_CURRENT_2MA  | IOC_STRENGTH_AUTO | \
                                 IOC_IOPULL_DOWN  | IOC_SLEW_DISABLE  | \
                                 IOC_HYST_DISABLE | IOC_BOTH_EDGES   | \
                                 IOC_INT_ENABLE   | IOC_IOMODE_NORMAL | \
                                 IOC_NO_WAKE_UP   | IOC_INPUT_ENABLE)

/*---------------------------------------------------------------------------*/
/* Register button sensors */
SENSORS(&button_e_sensor_click, &button_e_sensor_long_click);

/* register main button process */
PROCESS(main_process, "UD motion sensor control process");

/* set autostart processes */
AUTOSTART_PROCESSES(&dag_node_process, &main_process);

uint32_t motion_sensor_ioid = IOID_30;

/*---------------------------------------------------------------------------*/

void send_motion_sensor_packet(uint8_t motion_event)
{
   struct sensor_packet motion_sensor_packet;
   motion_sensor_packet.protocol_version = CURRENT_PROTOCOL_VERSION;
   motion_sensor_packet.device_version = CURRENT_DEVICE_VERSION;
   motion_sensor_packet.data_type = DATA_TYPE_SENSOR_DATA;
   motion_sensor_packet.number_ability = DEVICE_ABILITY_MOTION_SENSOR;
   motion_sensor_packet.sensor_number = 1;
   motion_sensor_packet.sensor_event = motion_event;
   send_sensor_event(&motion_sensor_packet);

   led_blink(LED_A);
}

void
gpio_int_handler(uint8_t ioid)
{
   if (motion_sensor_ioid == ioid)
   {
      if (ti_lib_gpio_read_dio(motion_sensor_ioid) == 0)
         send_motion_sensor_packet(DEVICE_ABILITY_MOTION_SENSOR_EVENT_NO_MOTION);
      else
         send_motion_sensor_packet(DEVICE_ABILITY_MOTION_SENSOR_EVENT_MOTION);
   }
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(main_process, ev, data)
{
   PROCESS_BEGIN();
   printf("Unwired motion sensor device. HELL-IN-CODE free. I hope.\n");

   PROCESS_PAUSE();

   ti_lib_gpio_clear_event_dio(motion_sensor_ioid);
   ti_lib_rom_ioc_pin_type_gpio_input(motion_sensor_ioid);
   ti_lib_rom_ioc_port_configure_set(motion_sensor_ioid, IOC_PORT_GPIO, BUTTON_GPIO_CFG);
   ti_lib_rom_ioc_int_enable(motion_sensor_ioid);
   gpio_interrupt_register_handler(motion_sensor_ioid, gpio_int_handler);

   PROCESS_END();
}
