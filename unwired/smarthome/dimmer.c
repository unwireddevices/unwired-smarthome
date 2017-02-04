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
 *         Dimmer service for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
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

#include "dimmer.h"
#include "dag_node.h"
#include "gpio-interrupt.h"

#include "xxf_types_helper.h"

#include "ti-lib.h"
#include "clock.h"
#include "../ud_binary_protocol.h"

#include "../fake_headers.h" //no move up! not "krasivo"!


#include "driverlib/flash.h"

/*---------------------------------------------------------------------------*/

/* Register buttons sensors */
SENSORS(&button_e_sensor_click,
        &button_e_sensor_long_click);

/* register dimmer process */
PROCESS(main_process, "Dimmer control process");

/* set autostart processes */
AUTOSTART_PROCESSES(&dag_node_process, &main_process);

volatile uint8_t dimmer_1_percent = 0;
volatile uint8_t dimmer_2_percent = 0;

/*---------------------------------------------------------------------------*/

static void exe_dimmer_command(struct command_data *command_dimmer)
{
    printf("DIMMER: new command, target: %02X, state: %02X, number: %02X\n",
           command_dimmer->ability_target,
           command_dimmer->ability_state,
           command_dimmer->ability_number);

    if (command_dimmer->ability_number != DEVICE_ABILITY_DIMMER_1 &&
        command_dimmer->ability_number != DEVICE_ABILITY_DIMMER_2)
    {
        printf("Not support dimmer number\n");
        return;
    }

    dimmer_1_percent = command_dimmer->ability_state;
}



/*---------------------------------------------------------------------------*/
void
FlashRead(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count) {
  uint8_t *pui8ReadAddress = (uint8_t *)ui32Address;
  while (ui32Count--) {
    *pui8DataBuffer++ = *pui8ReadAddress++;
  }
}


void configure_DIO()
{

    uint8_t write_data[1];
    write_data[0] = 0x42;
    FlashProgram(write_data, 0x1D9C5, 0x1);



    uint8_t read_buffer[1];
    FlashRead(read_buffer, 0x1D9C5, 1);
    printf("%"PRIX32":%" PRIXX8 "\n", 0x1D9C0, read_buffer[0]);


}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(main_process, ev, data)
{
  PROCESS_BEGIN();

  static struct command_data *message_data = NULL;

  PROCESS_PAUSE();
  
  printf("Unwired dimmer device. HELL-IN-CODE free. I hope.\n");
  configure_DIO();

  while(1)
  {
    PROCESS_YIELD();
    if(ev == PROCESS_EVENT_CONTINUE)
    {
      message_data = data;
      if (message_data->ability_target == DEVICE_ABILITY_DIMMER)
      {
          exe_dimmer_command(message_data);
      }
    }
  }

  PROCESS_END();
}
