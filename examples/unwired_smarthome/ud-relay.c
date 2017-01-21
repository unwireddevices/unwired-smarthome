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
#include "simple-udp.h"

#include "ud-relay.h"
#include "ud-dag_node.h"

#include "xxf_types_helper.h"

#include "ti-lib.h"
#include "ud_binary_protocol.h"

#define DPRINT  printf(">>%s:%"PRIu16"\n", __FILE__, __LINE__);


/*---------------------------------------------------------------------------*/

uint8_t relay_1_state = 0;
uint8_t relay_2_state = 0;

/*---------------------------------------------------------------------------*/

PROCESS(main_process, "Relay control process"); //register main button process
AUTOSTART_PROCESSES(&main_process, &dag_node_button_process); //set autostart processes

/*---------------------------------------------------------------------------*/

void change_DIO_state(uint8_t dio_number, uint8_t dio_state) //TODO: куча кода дублируется, сделай с этим что-нибудь
{
    DPRINT
    if (dio_number == 1)
    {
        switch ( dio_state ) {
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
            if (relay_1_state == 1) {
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
    }
    if (dio_number == 2)
    {
        switch ( dio_state ) {
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
            if (relay_2_state == 1) {
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
    }

}

/*---------------------------------------------------------------------------*/

void configure_DIO()
{
    DPRINT
    ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_RELAY_1); DPRINT
    ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_RELAY_2); DPRINT
    ti_lib_gpio_clear_dio(BOARD_IOID_RELAY_1); DPRINT
    ti_lib_gpio_clear_dio(BOARD_IOID_RELAY_2); DPRINT
}

/*---------------------------------------------------------------------------*/

void exe_relay_command(struct command_data *command_relay)
{
    DPRINT
    printf("RELAY: new command, target: %02X state: %02X number: %02X \n", command_relay->ability_target, command_relay->ability_state, command_relay->ability_number);
    uint8_t number_ability = command_relay->ability_number; DPRINT
    if (number_ability == 1 || number_ability == 2)
    {
        DPRINT
        change_DIO_state(number_ability, command_relay->ability_state); DPRINT
    }
    else
    {
        DPRINT
        printf("Not support relay number\n"); DPRINT
    }
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(main_process, ev, data)
{
  PROCESS_BEGIN();   DPRINT
  printf("Unwired relay device. HELL-IN-CODE free. I hope.\n"); DPRINT

  struct command_data *message_data = NULL; DPRINT

  PROCESS_PAUSE(); DPRINT
  
  configure_DIO(); DPRINT

  while(1)
  {
    DPRINT
    PROCESS_YIELD(); DPRINT
    if(ev == PROCESS_EVENT_CONTINUE)
    {
      DPRINT
      message_data = data; DPRINT

      if (message_data->ability_target == DEVICE_ABILITY_RELAY)
      {
          DPRINT
          exe_relay_command(data); DPRINT
      }
      DPRINT
    }
    DPRINT
  }
  DPRINT
  PROCESS_END();
}
