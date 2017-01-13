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
#ifdef BUTTON_SENSOR_CONF_ENABLE_SHUTDOWN
#define BUTTON_SENSOR_ENABLE_SHUTDOWN BUTTON_SENSOR_CONF_ENABLE_SHUTDOWN
#else
#define BUTTON_SENSOR_ENABLE_SHUTDOWN 1
#endif
/*---------------------------------------------------------------------------*/
#define BUTTON_GPIO_CFG         (IOC_CURRENT_2MA  | IOC_STRENGTH_AUTO | \
                                 IOC_IOPULL_UP    | IOC_SLEW_DISABLE  | \
                                 IOC_HYST_DISABLE | IOC_BOTH_EDGES    | \
                                 IOC_INT_ENABLE   | IOC_IOMODE_NORMAL | \
                                 IOC_NO_WAKE_UP   | IOC_INPUT_ENABLE)
/*---------------------------------------------------------------------------*/
#define SHORT_INTERVAL 5 //as timers tick(1 tick ~ 8ms)
#define LONG_INTERVAL 80 //as timers tick(1 tick ~ 8ms)
#define DEBOUNSE_SHORT_TIME 2

uint32_t button_a_last_activated = 0;
uint32_t button_a_last_low = 0;

uint32_t button_b_last_activated = 0;
uint32_t button_b_last_low = 0;

uint32_t button_c_last_activated = 0;
uint32_t button_c_last_low = 0;

uint32_t button_d_last_activated = 0;
uint32_t button_d_last_low = 0;

uint32_t button_e_last_activated = 0;
uint32_t button_e_last_low = 0;

uint8_t current_button;

static struct etimer button_short_timer;
static struct etimer button_long_timer;

/*---------------------------------------------------------------------------*/

PROCESS(button_sensor_short_process, "Button sensor process");
PROCESS_THREAD(button_sensor_short_process, ev, data)
{
    PROCESS_BEGIN();

    switch ( *(uint8_t *)data ) {
          case BOARD_IOID_KEY_A:
              if (clock_time() - button_a_last_low < LONG_INTERVAL)
                  sensors_changed(&button_a_sensor_click);
              etimer_set(&button_short_timer, SHORT_INTERVAL);
              PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&button_short_timer));
              if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_A) == 0)
                  sensors_changed(&button_a_sensor_change_off);
              else
                  sensors_changed(&button_a_sensor_change_on);
              break;

          case BOARD_IOID_KEY_B:
              if (clock_time() - button_b_last_low < LONG_INTERVAL)
                  sensors_changed(&button_b_sensor_click);
              etimer_set(&button_short_timer, SHORT_INTERVAL);
              PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&button_short_timer));
              if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_B) == 0)
                  sensors_changed(&button_b_sensor_change_off);
              else
                  sensors_changed(&button_b_sensor_change_on);
              break;

          case BOARD_IOID_KEY_C:
              if (clock_time() - button_c_last_low < LONG_INTERVAL)
                  sensors_changed(&button_c_sensor_click);
              etimer_set(&button_short_timer, SHORT_INTERVAL);
              PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&button_short_timer));
              if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_C) == 0)
                  sensors_changed(&button_c_sensor_change_off);
              else
                  sensors_changed(&button_c_sensor_change_on);
              break;

          case BOARD_IOID_KEY_D:
              if (clock_time() - button_d_last_low < LONG_INTERVAL)
                  sensors_changed(&button_d_sensor_click);
              etimer_set(&button_short_timer, SHORT_INTERVAL);
              PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&button_short_timer));
              if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_D) == 0)
                  sensors_changed(&button_d_sensor_change_off);
              else
                  sensors_changed(&button_d_sensor_change_on);
              break;

          case BOARD_IOID_KEY_E:
              if (clock_time() - button_e_last_low < LONG_INTERVAL)
                  sensors_changed(&button_e_sensor_click);
              etimer_set(&button_short_timer, SHORT_INTERVAL);
              PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&button_short_timer));
              if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_E) == 0)
                  sensors_changed(&button_e_sensor_change_off);
              else
                  sensors_changed(&button_e_sensor_change_on);
              break;

          default:
              break;
    }
    PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS(button_sensor_long_process, "Button sensor long process");
PROCESS_THREAD(button_sensor_long_process, ev, data)
{
  PROCESS_BEGIN();
  current_button = *(uint8_t *)data;
  etimer_reset(&button_long_timer);
  etimer_set(&button_long_timer, LONG_INTERVAL);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&button_long_timer));

  switch ( current_button ) {
        case BOARD_IOID_KEY_A:
            if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_A) == 0)
                sensors_changed(&button_a_sensor_long_click);
            break;
        case BOARD_IOID_KEY_B:
            if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_B) == 0)
                sensors_changed(&button_b_sensor_long_click);
            break;

        case BOARD_IOID_KEY_C:
            if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_C) == 0)
                sensors_changed(&button_c_sensor_long_click);
            break;

        case BOARD_IOID_KEY_D:
            if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_D) == 0)
                sensors_changed(&button_d_sensor_long_click);
            break;

        case BOARD_IOID_KEY_E:
            if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_E) == 0)
                sensors_changed(&button_e_sensor_long_click);
            break;

        default:
            break;
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

static void
button_press_handler(uint8_t ioid)
{

    uint8_t current_button_ioid;


    if(ioid == BOARD_IOID_KEY_A) {
        current_button_ioid = BOARD_IOID_KEY_A;
        if (clock_time() - button_a_last_activated < DEBOUNSE_SHORT_TIME)
            return;
        button_a_last_activated = clock_time();
        if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_A) == 0) {
            process_start(&button_sensor_short_process, (void *)&current_button_ioid);
            if (process_is_running(&button_sensor_long_process) == 0)
                process_start(&button_sensor_long_process, (void *)&current_button_ioid);
            button_a_last_low = clock_time();
        } else {
            process_start(&button_sensor_short_process, (void *)&current_button_ioid);
            if (process_is_running(&button_sensor_long_process) != 0)
                process_exit(&button_sensor_long_process);
        }
    }

    if(ioid == BOARD_IOID_KEY_B) {
        current_button_ioid = BOARD_IOID_KEY_B;
        if (clock_time() - button_b_last_activated < DEBOUNSE_SHORT_TIME)
            return;
        button_b_last_activated = clock_time();
        if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_B) == 0) {
            process_start(&button_sensor_short_process, (void *)&current_button_ioid);
            process_start(&button_sensor_long_process, (void *)&current_button_ioid);
            button_b_last_low = clock_time();
        } else {
            process_start(&button_sensor_short_process, (void *)&current_button_ioid);
            process_exit(&button_sensor_long_process);
        }
    }

    if(ioid == BOARD_IOID_KEY_C) {
        current_button_ioid = BOARD_IOID_KEY_C;
        if (clock_time() - button_c_last_activated < DEBOUNSE_SHORT_TIME)
            return;
        button_c_last_activated = clock_time();
        if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_C) == 0) {
            process_start(&button_sensor_short_process, (void *)&current_button_ioid);
            process_start(&button_sensor_long_process, (void *)&current_button_ioid);
            button_c_last_low = clock_time();
        } else {
            process_start(&button_sensor_short_process, (void *)&current_button_ioid);
            process_exit(&button_sensor_long_process);
        }
    }

    if(ioid == BOARD_IOID_KEY_D) {
        current_button_ioid = BOARD_IOID_KEY_D;
        if (clock_time() - button_d_last_activated < DEBOUNSE_SHORT_TIME)
            return;
        button_d_last_activated = clock_time();
        if (ti_lib_gpio_read_dio(BOARD_IOID_KEY_D) == 0) {
            process_start(&button_sensor_short_process, (void *)&current_button_ioid);
            process_start(&button_sensor_long_process, (void *)&current_button_ioid);
            button_d_last_low = clock_time();
        } else {
            process_start(&button_sensor_short_process, (void *)&current_button_ioid);
            process_exit(&button_sensor_long_process);
        }
    }


    if(ioid == BOARD_IOID_KEY_E) {
        current_button_ioid = BOARD_IOID_KEY_E;
        if (clock_time() - button_e_last_activated < DEBOUNSE_SHORT_TIME)
            return;
        button_e_last_activated = clock_time();
        if (ti_lib_gpio_read_dio(current_button_ioid) == 0) {
            process_start(&button_sensor_short_process, (void *)&current_button_ioid);
            process_start(&button_sensor_long_process, (void *)&current_button_ioid);
            button_e_last_low = clock_time();
        } else {
            process_start(&button_sensor_short_process, (void *)&current_button_ioid);
            process_exit(&button_sensor_long_process);
        }
    }


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
