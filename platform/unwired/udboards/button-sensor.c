/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
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
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup srf06-common-peripherals
 * @{
 *
 * \file
 *  Driver for the SmartRF06EB buttons when a CC13xx/CC26xxEM is mounted on it
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "lib/sensors.h"
#include "button-sensor.h"
#include "gpio-interrupt.h"
#include "sys/timer.h"
#include "sys/process.h"
#include "stdio.h"

#include "lpm.h"

#include "ti-lib.h"

#include <stdint.h>

#include "xxf_types_helper.h"

/*---------------------------------------------------------------------------*/
#define BUTTON_GPIO_CFG         (IOC_CURRENT_2MA  | IOC_STRENGTH_AUTO | \
                                 IOC_IOPULL_UP    | IOC_SLEW_DISABLE  | \
                                 IOC_HYST_DISABLE | IOC_BOTH_EDGES    | \
                                 IOC_INT_ENABLE   | IOC_IOMODE_NORMAL | \
                                 IOC_NO_WAKE_UP   | IOC_INPUT_ENABLE)
/*---------------------------------------------------------------------------*/
#define LONG_INTERVAL_MS 720
#define DEBOUNCE_INTERVAL_MS 60

#define LONG_INTERVAL LONG_INTERVAL_MS*CLOCK_SECOND/1000
#define DEBOUNCE_INTERVAL DEBOUNCE_INTERVAL_MS*CLOCK_SECOND/1000

//#define DEBUG

#ifdef DEBUG
    #define printf_log(...) printf(__VA_ARGS__)
#else
    #define printf_log(...)
#endif


volatile uint32_t button_last_low_time[32];
volatile uint32_t button_last_state[32];
volatile uint8_t current_button;
volatile uint8_t current_button_long;

static struct etimer button_long_timer;

/*---------------------------------------------------------------------------*/

PROCESS(button_sensor_long_process, "Button sensor long process");
PROCESS_THREAD(button_sensor_long_process, ev, data)
{
  PROCESS_BEGIN();

  printf_log("SENSOR LONG PROCESS: enter on %"PRIu32" tick\n", clock_time());

  if (ev == PROCESS_EVENT_EXIT) {
      printf_log("SENSOR LONG PROCESS: event exit on %"PRIu32" tick\n", clock_time());
      return 1;
  }

  current_button_long = current_button;
  etimer_reset(&button_long_timer);
  etimer_set(&button_long_timer, LONG_INTERVAL);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&button_long_timer));

  printf_log("SENSOR LONG PROCESS: resume on %"PRIu32" tick\n", clock_time());

  if (ti_lib_gpio_read_dio(current_button_long) == 0) {
      if (current_button_long == BOARD_IOID_KEY_A)
        sensors_changed(&button_a_sensor_long_click);
      if (current_button_long == BOARD_IOID_KEY_B)
        sensors_changed(&button_b_sensor_long_click);
      if (current_button_long == BOARD_IOID_KEY_C)
        sensors_changed(&button_c_sensor_long_click);
      if (current_button_long == BOARD_IOID_KEY_D)
        sensors_changed(&button_d_sensor_long_click);
      if (current_button_long == BOARD_IOID_KEY_E)
        sensors_changed(&button_e_sensor_long_click);
  }

  printf_log("SENSOR LONG PROCESS: exit on %"PRIu32" tick\n", clock_time());

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the button sensor for all buttons.
 *
 * \param type This function does nothing unless type == SENSORS_ACTIVE
 * \param c 0: disable the button, non-zero: enable
 * \param key: One of BOARD_KEY_LEFT, BOARD_KEY_RIGHT etc
 */
static void
config_buttons(int type, int c, uint32_t key)
{
  switch(type) {
  case SENSORS_HW_INIT:
    ti_lib_gpio_clear_event_dio(key);
    ti_lib_rom_ioc_pin_type_gpio_input(key);
    ti_lib_rom_ioc_port_configure_set(key, IOC_PORT_GPIO, BUTTON_GPIO_CFG);
    gpio_interrupt_register_handler(key, button_press_handler);
    button_last_low_time[key] = 0;
    button_last_state[key] = 1;
    break;
  case SENSORS_ACTIVE:
    if(c) {
      ti_lib_gpio_clear_event_dio(key);
      ti_lib_rom_ioc_pin_type_gpio_input(key);
      ti_lib_rom_ioc_port_configure_set(key, IOC_PORT_GPIO, BUTTON_GPIO_CFG);
      ti_lib_rom_ioc_int_enable(key);
    } else {
      ti_lib_rom_ioc_int_disable(key);
    }
    break;
  default:
    break;
  }
}
/*---------------------------------------------------------------------------*/


void
button_press_handler(uint8_t ioid)
{
    uint8_t current_button_ioid = ioid;
    uint32_t current_button_state = ti_lib_gpio_read_dio(ioid);
    uint32_t current_time = clock_time();
    printf_log("SENSOR: button %"PRIu8" change state to %"PRIu32" on %"PRIu32" tick\n",
               current_button_ioid,
               current_button_state,
               current_time);


        if (button_last_state[current_button_ioid] != current_button_state)
        {
            config_buttons(SENSORS_ACTIVE, 0, (uint32_t)current_button_ioid);

            button_last_state[current_button_ioid] = current_button_state;
            if (current_button_state == 0)
            {
                button_last_low_time[current_button_ioid] = current_time;
                if (process_is_running(&button_sensor_long_process) == 0)
                {
                    current_button = current_button_ioid;
                    process_start(&button_sensor_long_process, NULL);
                }

            }
            if (current_button_state == 1)
            {
                printf_log("SENSOR: current_time: %"PRIu32", button_last_low_time[%"PRIu8"]: %"PRIu32", diff %"PRIu32", debounce interval %"PRIu8"\n",
                           current_time,
                           current_button_ioid,
                           button_last_low_time[current_button_ioid],
                           current_time - button_last_low_time[current_button_ioid],
                           DEBOUNCE_INTERVAL);

                if (current_time - button_last_low_time[current_button_ioid] > DEBOUNCE_INTERVAL &&
                    current_time - button_last_low_time[current_button_ioid] < LONG_INTERVAL)
                {
                    if (process_is_running(&button_sensor_long_process) == 1)
                        process_exit(&button_sensor_long_process);

                    if (current_button_ioid == BOARD_IOID_KEY_A)
                      sensors_changed(&button_a_sensor_click);
                    if (current_button_ioid == BOARD_IOID_KEY_B)
                      sensors_changed(&button_b_sensor_click);
                    if (current_button_ioid == BOARD_IOID_KEY_C)
                      sensors_changed(&button_c_sensor_click);
                    if (current_button_ioid == BOARD_IOID_KEY_D)
                      sensors_changed(&button_d_sensor_click);
                    if (current_button_ioid == BOARD_IOID_KEY_E)
                      sensors_changed(&button_e_sensor_click);
                }
            }

            config_buttons(SENSORS_ACTIVE, 1, (uint32_t)current_button_ioid);
        }

}

/*---------------------------------------------------------------------------*/

static int config_a(int type, int value)
{
  config_buttons(type, value, BOARD_IOID_KEY_A);
  return 1;
}

static int config_b(int type, int value)
{
  config_buttons(type, value, BOARD_IOID_KEY_B);
  return 1;
}

static int config_c(int type, int value)
{
  config_buttons(type, value, BOARD_IOID_KEY_C);
  return 1;
}

static int config_d(int type, int value)
{
  config_buttons(type, value, BOARD_IOID_KEY_D);
  return 1;
}

static int config_e(int type, int value)
{
  config_buttons(type, value, BOARD_IOID_KEY_E);
  return 1;
}


/*---------------------------------------------------------------------------*/

SENSORS_SENSOR(button_a_sensor_click, BUTTON_SENSOR, NULL, config_a, NULL);
SENSORS_SENSOR(button_a_sensor_long_click, BUTTON_SENSOR, NULL, config_a, NULL);
SENSORS_SENSOR(button_a_sensor_change_on, BUTTON_SENSOR, NULL, config_a, NULL);
SENSORS_SENSOR(button_a_sensor_change_off, BUTTON_SENSOR, NULL, config_a, NULL);

SENSORS_SENSOR(button_b_sensor_click, BUTTON_SENSOR, NULL, config_b, NULL);
SENSORS_SENSOR(button_b_sensor_long_click, BUTTON_SENSOR, NULL, config_b, NULL);
SENSORS_SENSOR(button_b_sensor_change_on, BUTTON_SENSOR, NULL, config_b, NULL);
SENSORS_SENSOR(button_b_sensor_change_off, BUTTON_SENSOR, NULL, config_b, NULL);

SENSORS_SENSOR(button_c_sensor_click, BUTTON_SENSOR, NULL, config_c, NULL);
SENSORS_SENSOR(button_c_sensor_long_click, BUTTON_SENSOR, NULL, config_c, NULL);
SENSORS_SENSOR(button_c_sensor_change_on, BUTTON_SENSOR, NULL, config_c, NULL);
SENSORS_SENSOR(button_c_sensor_change_off, BUTTON_SENSOR, NULL, config_c, NULL);

SENSORS_SENSOR(button_d_sensor_click, BUTTON_SENSOR, NULL, config_d, NULL);
SENSORS_SENSOR(button_d_sensor_long_click, BUTTON_SENSOR, NULL, config_d, NULL);
SENSORS_SENSOR(button_d_sensor_change_on, BUTTON_SENSOR, NULL, config_d, NULL);
SENSORS_SENSOR(button_d_sensor_change_off, BUTTON_SENSOR, NULL, config_d, NULL);

SENSORS_SENSOR(button_e_sensor_click, BUTTON_SENSOR, NULL, config_e, NULL);
SENSORS_SENSOR(button_e_sensor_long_click, BUTTON_SENSOR, NULL, config_e, NULL);
SENSORS_SENSOR(button_e_sensor_change_on, BUTTON_SENSOR, NULL, config_e, NULL);
SENSORS_SENSOR(button_e_sensor_change_off, BUTTON_SENSOR, NULL, config_e, NULL);
