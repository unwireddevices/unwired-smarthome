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
#include "lpm.h"


#include "xxf_types_helper.h"

#include "ti-lib.h"
#include "clock.h"
#include "../ud_binary_protocol.h"

#define ZEROCROSS_GPIO_CFG       (IOC_CURRENT_2MA  | IOC_STRENGTH_AUTO | \
                                 IOC_IOPULL_UP    | IOC_SLEW_DISABLE  | \
                                 IOC_HYST_DISABLE | IOC_RISING_EDGE    | \
                                 IOC_INT_ENABLE   | IOC_IOMODE_NORMAL | \
                                 IOC_NO_WAKE_UP   | IOC_INPUT_ENABLE)
/*---------------------------------------------------------------------------*/
#include "../fake_headers.h" //no move up! not "krasivo"!

LPM_MODULE(buzzer_module, NULL, NULL, NULL, LPM_DOMAIN_PERIPH);

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


/*---------------------------------------------------------------------------*/

void configure_DIO()
{

    ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_DIMMER_1);
    ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_DIMMER_2);
    ti_lib_gpio_clear_dio(BOARD_IOID_DIMMER_1);
    ti_lib_gpio_clear_dio(BOARD_IOID_DIMMER_2);

    uint32_t freq = 10000;

    uint32_t load;

    /* Enable GPT0 clocks under active, sleep, deep sleep */
    ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_TIMER0);
    ti_lib_prcm_peripheral_sleep_enable(PRCM_PERIPH_TIMER0);
    ti_lib_prcm_peripheral_deep_sleep_enable(PRCM_PERIPH_TIMER0);
    ti_lib_prcm_load_set();
    while(!ti_lib_prcm_load_get());

    /* Drive the I/O ID with GPT0 / Timer A */
    ti_lib_ioc_port_configure_set(BOARD_IOID_DIMMER_1, IOC_PORT_MCU_PORT_EVENT0, IOC_STD_OUTPUT);

    /* GPT0 / Timer A: PWM, Interrupt Enable */
    HWREG(GPT0_BASE + GPT_O_TAMR) = (TIMER_CFG_A_PWM & 0xFF) | GPT_TAMR_TAPWMIE;


    /*
     * Register ourself with LPM. This will keep the PERIPH PD powered on
     * during deep sleep, allowing the buzzer to keep working while the chip is
     * being power-cycled
     */
    lpm_register_module(&buzzer_module);

    /* Stop the timer */
    ti_lib_timer_disable(GPT0_BASE, TIMER_A);

    if(freq > 0) {
      load = (GET_MCU_CLOCK / freq);

      ti_lib_timer_load_set(GPT0_BASE, TIMER_A, load);
      ti_lib_timer_match_set(GPT0_BASE, TIMER_A, load / 3);

      /* Start */
      ti_lib_timer_enable(GPT0_BASE, TIMER_A);
    }


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
