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
 *
 * This file is part of the Contiki operating system.
 *
 */

/*---------------------------------------------------------------------------*/
/*
* \file
*         Relay service for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
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
#include "board.h"
#include "board-peripherals.h"
#include "simple-udp.h"

#include "relay.h"

#include "xxf_types_helper.h"

#include "ti-lib.h"
#include "../ud_binary_protocol.h"
#include "../flash-common.h"
#include "../dag_node.h"

#include "../fake_headers.h" //no move up! not "krasivo"!

/*---------------------------------------------------------------------------*/

uint8_t relay_1_state = 0;
uint8_t relay_2_state = 0;

/*---------------------------------------------------------------------------*/

/* Register buttons sensors */
SENSORS(&button_e_sensor_click, &button_e_sensor_long_click);

/* register relay process */
PROCESS(main_process, "Relay control process");

/* set autostart processes */
AUTOSTART_PROCESSES(&dag_node_process, &main_process);

/*---------------------------------------------------------------------------*/

void set_start_state_flash(uint8_t dio_number, uint8_t dio_state)
{
   if (dio_number == BOARD_IOID_RELAY_1)
      user_flash_update_byte(POWER_1_CH_START_STATE_OFFSET, dio_state);

   if (dio_number == BOARD_IOID_RELAY_2)
      user_flash_update_byte(POWER_2_CH_START_STATE_OFFSET, dio_state);
}

/*---------------------------------------------------------------------------*/

uint8_t get_start_state_flash(uint8_t dio_number)
{
   uint8_t state = 0xFF;

   if (dio_number == BOARD_IOID_RELAY_1)
      state = user_flash_read_byte(POWER_1_CH_START_STATE_OFFSET);

   if (dio_number == BOARD_IOID_RELAY_2)
      state = user_flash_read_byte(POWER_2_CH_START_STATE_OFFSET);

   if (state == 0xFF)
      state = START_ON_LAST_STATE;

   return state;
}


/*---------------------------------------------------------------------------*/

void set_last_state_flash(uint8_t dio_number, uint8_t dio_state)
{
   if (dio_number == BOARD_IOID_RELAY_1)
      user_flash_update_byte(POWER_1_CH_LAST_STATE_OFFSET, dio_state);

   if (dio_number == BOARD_IOID_RELAY_2)
      user_flash_update_byte(POWER_2_CH_LAST_STATE_OFFSET, dio_state);
}

/*---------------------------------------------------------------------------*/

uint8_t get_last_state_flash(uint8_t dio_number)
{
   uint8_t state = 0xFF;
   if (dio_number == BOARD_IOID_RELAY_1)
      state = user_flash_read_byte(POWER_1_CH_LAST_STATE_OFFSET);

   if (dio_number == BOARD_IOID_RELAY_2)
      state = user_flash_read_byte(POWER_2_CH_LAST_STATE_OFFSET);

   if (state == 0xFF)
      state = DEVICE_ABILITY_RELAY_COMMAND_OFF;

   return state;
}

/*---------------------------------------------------------------------------*/

uint8_t ability_to_dio(uint8_t ability)
{
   if (ability == DEVICE_ABILITY_RELAY_1)
      return BOARD_IOID_RELAY_1;

   if (ability == DEVICE_ABILITY_RELAY_2)
      return BOARD_IOID_RELAY_2;

   return 0xFF;
}

/*---------------------------------------------------------------------------*/

uint8_t dio_to_ability(uint8_t dio)
{
   if (dio == BOARD_IOID_RELAY_1)
      return DEVICE_ABILITY_RELAY_1;

   if (dio == BOARD_IOID_RELAY_2)
      return DEVICE_ABILITY_RELAY_2;

   return 0;
}

/*---------------------------------------------------------------------------*/
void change_dio_state(uint8_t ability_number, uint8_t dio_state)    //TODO: куча кода дублируется, сделай с этим что-нибудь
//TODO: и убери switch-case!
{
   //сюда передается ability, хотя функция называется change_dio.
   printf("RELAY: change dio state, number: %02X, state: %02X\n",
          ability_number,
          dio_state);

   if (ability_number == DEVICE_ABILITY_RELAY_1)
   {
      switch ( dio_state )
      {
      case DEVICE_ABILITY_RELAY_COMMAND_OFF:
         ti_lib_gpio_clear_dio(BOARD_IOID_RELAY_1);
         printf("RELAY: Relay 1 set to OFF\n");
         relay_1_state = 0;
         break;
      case DEVICE_ABILITY_RELAY_COMMAND_ON:
         ti_lib_gpio_set_dio(BOARD_IOID_RELAY_1);
         printf("RELAY: Relay 1 set to ON\n");
         relay_1_state = 1;
         break;
      case DEVICE_ABILITY_RELAY_COMMAND_TOGGLE:
         if (relay_1_state == 1)
         {
            ti_lib_gpio_clear_dio(BOARD_IOID_RELAY_1);
            printf("RELAY: Relay 1 set to OFF\n");
            relay_1_state = 0;
         }
         else
         {
            ti_lib_gpio_set_dio(BOARD_IOID_RELAY_1);
            printf("RELAY: Relay 1 set to ON\n");
            relay_1_state = 1;
         }
         break;
      default:
         printf("RELAY: Incompatible relay 1 state!\n");
         break;
      }
      set_last_state_flash(BOARD_IOID_RELAY_1, relay_1_state);

   }
   if (ability_number == DEVICE_ABILITY_RELAY_2)
   {
      switch ( dio_state )
      {
      case DEVICE_ABILITY_RELAY_COMMAND_OFF:
         ti_lib_gpio_clear_dio(BOARD_IOID_RELAY_2);
         printf("RELAY: Relay 2 set to OFF\n");
         relay_2_state = 0;
         break;
      case DEVICE_ABILITY_RELAY_COMMAND_ON:
         ti_lib_gpio_set_dio(BOARD_IOID_RELAY_2);
         printf("RELAY: Relay 2 set to ON\n");
         relay_2_state = 2;
         break;
      case DEVICE_ABILITY_RELAY_COMMAND_TOGGLE:
         if (relay_2_state == 1)
         {
            ti_lib_gpio_clear_dio(BOARD_IOID_RELAY_2);
            printf("RELAY: Relay 2 set to OFF\n");
            relay_2_state = 0;
         }
         else
         {
            ti_lib_gpio_set_dio(BOARD_IOID_RELAY_2);
            printf("RELAY: Relay 2 set to ON\n");
            relay_2_state = 1;
         }
         break;
      default:
         printf("RELAY: Incompatible relay 2 state!\n");
         break;
      }
      set_last_state_flash(BOARD_IOID_RELAY_2, relay_2_state);
   }

}

/*---------------------------------------------------------------------------*/

void configure_channel(uint8_t channel_dio)
{
   ti_lib_ioc_pin_type_gpio_output(channel_dio);
   ti_lib_gpio_clear_dio(channel_dio);

   uint8_t start_state = get_start_state_flash(channel_dio);

   if (start_state == START_ON_LAST_STATE)
      change_dio_state(dio_to_ability(channel_dio), get_last_state_flash(channel_dio));

   if (start_state == START_ON_ON_STATE)
      change_dio_state(dio_to_ability(channel_dio), DEVICE_ABILITY_RELAY_COMMAND_ON);

   if (start_state == START_ON_OFF_STATE)
      change_dio_state(dio_to_ability(channel_dio), DEVICE_ABILITY_RELAY_COMMAND_OFF);
}

/*---------------------------------------------------------------------------*/


void settings_change(struct command_data *command_relay)
{
   printf("RELAY: new settings, target: %02X, state: %02X, number: %02X \n",
          command_relay->ability_target,
          command_relay->ability_state,
          command_relay->ability_number);

   if (command_relay->ability_number != DEVICE_ABILITY_RELAY_1 &&
         command_relay->ability_number != DEVICE_ABILITY_RELAY_2)
   {
      printf("Not support relay number\n");
      return;
   }

   if (command_relay->ability_target == DEVICE_ABILITY_RELAY_1_SETTINGS_START_STATE)
      set_start_state_flash(BOARD_IOID_RELAY_1, command_relay->ability_state);

   if (command_relay->ability_target == DEVICE_ABILITY_RELAY_2_SETTINGS_START_STATE)
      set_start_state_flash(BOARD_IOID_RELAY_2, command_relay->ability_state);
}

/*---------------------------------------------------------------------------*/


void exe_relay_command(struct command_data *command_relay)
{
   printf("RELAY: new command, target: %02X, state: %02X, number: %02X \n",
          command_relay->ability_target,
          command_relay->ability_state,
          command_relay->ability_number);

   if (command_relay->ability_number != DEVICE_ABILITY_RELAY_1 &&
         command_relay->ability_number != DEVICE_ABILITY_RELAY_2)
   {
      printf("Not support relay number\n");
      return;
   }
   change_dio_state(command_relay->ability_number, command_relay->ability_state);
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(main_process, ev, data)
{
   PROCESS_BEGIN();

   static struct command_data *message_data = NULL;

   PROCESS_PAUSE();

   printf("Unwired relay device. HELL-IN-CODE free. I hope.\n");
   configure_channel(BOARD_IOID_RELAY_1);
   configure_channel(BOARD_IOID_RELAY_2);

   while (1)
   {
      PROCESS_YIELD();
      if (ev == PROCESS_EVENT_CONTINUE)
      {
         message_data = data;
         if (message_data != NULL)
         {
            if (message_data->ability_target == DEVICE_ABILITY_RELAY)
            {
               if (message_data->data_type == DATA_TYPE_COMMAND)
                  exe_relay_command(message_data);

               if (message_data->data_type == DATA_TYPE_SETTINGS)
                  settings_change(message_data);
            }
         }
      }
   }

   PROCESS_END();
}
