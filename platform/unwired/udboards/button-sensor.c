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
#include "srf06/button-sensor.h"
#include "gpio-interrupt.h"
#include "sys/timer.h"
#include "lpm.h"

#include "ti-lib.h"

#include <stdint.h>
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
//#define DEBOUNCE_DURATION (CLOCK_SECOND >> 5)

struct btn_timer {
  struct timer debounce;
  clock_time_t start;
  clock_time_t duration;
};

static struct btn_timer e_timer, a_timer, b_timer, c_timer,
              d_timer;
/*---------------------------------------------------------------------------*/
/**
 * \brief Handler for SmartRF button presses
 */
static void
button_press_handler(uint8_t ioid)
{
  if(ioid == BOARD_IOID_KEY_A) {
    if(!timer_expired(&a_timer.debounce)) {
      return;
    }
    timer_set(&a_timer.debounce, DEBOUNCE_DURATION);
    /*
     * Start press duration counter on press (falling), notify on release
     * (rising)
     */
    if(ti_lib_gpio_read_dio(BOARD_IOID_KEY_A) == 0) {
      a_timer.start = clock_time();
      a_timer.duration = 0;
    } else {
      a_timer.duration = clock_time() - a_timer.start;
      sensors_changed(&button_a_sensor);
    }
  }

  if(ioid == BOARD_IOID_KEY_B) {
    if(!timer_expired(&b_timer.debounce)) {
      return;
    }
    timer_set(&b_timer.debounce, DEBOUNCE_DURATION);
    /*
     * Start press duration counter on press (falling), notify on release
     * (rising)
     */
    if(ti_lib_gpio_read_dio(BOARD_IOID_KEY_B) == 0) {
      b_timer.start = clock_time();
      b_timer.duration = 0;
    } else {
      b_timer.duration = clock_time() - b_timer.start;
      sensors_changed(&button_b_sensor);
    }
  }

  if(ioid == BOARD_IOID_KEY_C) {
    if(BUTTON_SENSOR_ENABLE_SHUTDOWN == 0) {
      if(!timer_expired(&c_timer.debounce)) {
        return;
      }

      timer_set(&c_timer.debounce, DEBOUNCE_DURATION);

      /*
       * Start press duration counter on press (falling), notify on release
       * (rising)
       */
      if(ti_lib_gpio_read_dio(BOARD_IOID_KEY_C) == 0) {
        c_timer.start = clock_time();
        c_timer.duration = 0;
      } else {
        c_timer.duration = clock_time() - c_timer.start;
        sensors_changed(&button_c_sensor);
      }
    } else {
      lpm_shutdown(BOARD_IOID_KEY_C, IOC_IOPULL_UP, IOC_WAKE_ON_LOW);
    }
  }

  if(ioid == BOARD_IOID_KEY_D) {
    if(!timer_expired(&d_timer.debounce)) {
      return;
    }

    timer_set(&d_timer.debounce, DEBOUNCE_DURATION);

    /*
     * Start press duration counter on press (falling), notify on release
     * (rising)
     */
    if(ti_lib_gpio_read_dio(BOARD_IOID_KEY_D) == 0) {
      d_timer.start = clock_time();
      d_timer.duration = 0;
    } else {
      d_timer.duration = clock_time() - d_timer.start;
      sensors_changed(&button_d_sensor);
    }
  }

  if(ioid == BOARD_IOID_KEY_E) {
    if(!timer_expired(&e_timer.debounce)) {
      return;
    }
    timer_set(&e_timer.debounce, DEBOUNCE_DURATION);
    /*
     * Start press duration counter on press (falling), notify on release
     * (rising)
     */
    if(ti_lib_gpio_read_dio(BOARD_IOID_KEY_E) == 0) {
      e_timer.start = clock_time();
      e_timer.duration = 0;
    } else {
      e_timer.duration = clock_time() - e_timer.start;
      sensors_changed(&button_e_sensor);
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
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the select button.
 *
 * Parameters are passed onto config_buttons, which does the actual
 * configuration
 * Parameters are ignored. They have been included because the prototype is
 * dictated by the core sensor api. The return value is also required by
 * the API but otherwise ignored.
 *
 * \param type passed to config_buttons as-is
 * \param value passed to config_buttons as-is
 *
 * \return ignored
 */
static int
config_e(int type, int value)
{
  config_buttons(type, value, BOARD_IOID_KEY_E);
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the left button.
 *
 * Parameters are passed onto config_buttons, which does the actual
 * configuration
 * Parameters are ignored. They have been included because the prototype is
 * dictated by the core sensor api. The return value is also required by
 * the API but otherwise ignored.
 *
 * \param type passed to config_buttons as-is
 * \param value passed to config_buttons as-is
 *
 * \return ignored
 */
static int
config_a(int type, int value)
{
  config_buttons(type, value, BOARD_IOID_KEY_A);
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the right button.
 *
 * Parameters are passed onto config_buttons, which does the actual
 * configuration
 * Parameters are ignored. They have been included because the prototype is
 * dictated by the core sensor api. The return value is also required by
 * the API but otherwise ignored.
 *
 * \param type passed to config_buttons as-is
 * \param value passed to config_buttons as-is
 *
 * \return ignored
 */
static int
config_b(int type, int value)
{
  config_buttons(type, value, BOARD_IOID_KEY_B);
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the up button.
 *
 * Parameters are passed onto config_buttons, which does the actual
 * configuration
 * Parameters are ignored. They have been included because the prototype is
 * dictated by the core sensor api. The return value is also required by
 * the API but otherwise ignored.
 *
 * \param type passed to config_buttons as-is
 * \param value passed to config_buttons as-is
 *
 * \return ignored
 */
static int
config_c(int type, int value)
{
  config_buttons(type, value, BOARD_IOID_KEY_C);
  return 1;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Configuration function for the down button.
 *
 * Parameters are passed onto config_buttons, which does the actual
 * configuration
 * Parameters are ignored. They have been included because the prototype is
 * dictated by the core sensor api. The return value is also required by
 * the API but otherwise ignored.
 *
 * \param type passed to config_buttons as-is
 * \param value passed to config_buttons as-is
 *
 * \return ignored
 */
static int
config_d(int type, int value)
{
  config_buttons(type, value, BOARD_IOID_KEY_D);
  return 1;
}
/*---------------------------------------------------------------------------*/
static int
value_e(int type)
{
  if(type == BUTTON_SENSOR_VALUE_STATE) {
    return ti_lib_gpio_read_dio(BOARD_IOID_KEY_E) == 0 ?
           BUTTON_SENSOR_VALUE_PRESSED : BUTTON_SENSOR_VALUE_RELEASED;
  } else if(type == BUTTON_SENSOR_VALUE_DURATION) {
    return (int)e_timer.duration;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value_a(int type)
{
  if(type == BUTTON_SENSOR_VALUE_STATE) {
    return ti_lib_gpio_read_dio(BOARD_IOID_KEY_A) == 0 ?
           BUTTON_SENSOR_VALUE_PRESSED : BUTTON_SENSOR_VALUE_RELEASED;
  } else if(type == BUTTON_SENSOR_VALUE_DURATION) {
    return (int)a_timer.duration;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value_b(int type)
{
  if(type == BUTTON_SENSOR_VALUE_STATE) {
    return ti_lib_gpio_read_dio(BOARD_IOID_KEY_B) == 0 ?
           BUTTON_SENSOR_VALUE_PRESSED : BUTTON_SENSOR_VALUE_RELEASED;
  } else if(type == BUTTON_SENSOR_VALUE_DURATION) {
    return (int)b_timer.duration;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value_c(int type)
{
  if(type == BUTTON_SENSOR_VALUE_STATE) {
    return ti_lib_gpio_read_dio(BOARD_IOID_KEY_C) == 0 ?
           BUTTON_SENSOR_VALUE_PRESSED : BUTTON_SENSOR_VALUE_RELEASED;
  } else if(type == BUTTON_SENSOR_VALUE_DURATION) {
    return (int)c_timer.duration;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value_d(int type)
{
  if(type == BUTTON_SENSOR_VALUE_STATE) {
    return ti_lib_gpio_read_dio(BOARD_IOID_KEY_D) == 0 ?
           BUTTON_SENSOR_VALUE_PRESSED : BUTTON_SENSOR_VALUE_RELEASED;
  } else if(type == BUTTON_SENSOR_VALUE_DURATION) {
    return (int)d_timer.duration;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Status function for all buttons
 * \param type SENSORS_ACTIVE or SENSORS_READY
 * \param key_io_id BOARD_IOID_KEY_LEFT, BOARD_IOID_KEY_RIGHT etc
 * \return 1 if the button's port interrupt is enabled (edge detect)
 *
 * This function will only be called by status_left, status_right and the
 * called will pass the correct key_io_id
 */
static int
status(int type, uint32_t key_io_id)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    if(ti_lib_ioc_port_configure_get(key_io_id) & IOC_INT_ENABLE) {
      return 1;
    }
    break;
  default:
    break;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Status function for the select button.
 * \param type SENSORS_ACTIVE or SENSORS_READY
 * \return 1 if the button's port interrupt is enabled (edge detect)
 *
 * This function will call status. It will pass type verbatim and it will also
 * pass the correct key_io_id
 */
static int
status_e(int type)
{
  return status(type, BOARD_IOID_KEY_E);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Status function for the left button.
 * \param type SENSORS_ACTIVE or SENSORS_READY
 * \return 1 if the button's port interrupt is enabled (edge detect)
 *
 * This function will call status. It will pass type verbatim and it will also
 * pass the correct key_io_id
 */
static int
status_a(int type)
{
  return status(type, BOARD_IOID_KEY_A);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Status function for the right button.
 * \param type SENSORS_ACTIVE or SENSORS_READY
 * \return 1 if the button's port interrupt is enabled (edge detect)
 *
 * This function will call status. It will pass type verbatim and it will also
 * pass the correct key_io_id
 */
static int
status_b(int type)
{
  return status(type, BOARD_IOID_KEY_B);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Status function for the up button.
 * \param type SENSORS_ACTIVE or SENSORS_READY
 * \return 1 if the button's port interrupt is enabled (edge detect)
 *
 * This function will call status. It will pass type verbatim and it will also
 * pass the correct key_io_id
 */
static int
status_c(int type)
{
  return status(type, BOARD_IOID_KEY_C);
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Status function for the down button.
 * \param type SENSORS_ACTIVE or SENSORS_READY
 * \return 1 if the button's port interrupt is enabled (edge detect)
 *
 * This function will call status. It will pass type verbatim and it will also
 * pass the correct key_io_id
 */
static int
status_d(int type)
{
  return status(type, BOARD_IOID_KEY_D);
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_e_sensor, BUTTON_SENSOR, value_e, config_e, status_e);
SENSORS_SENSOR(button_a_sensor, BUTTON_SENSOR, value_a, config_a, status_a);
SENSORS_SENSOR(button_b_sensor, BUTTON_SENSOR, value_b, config_b, status_b);
SENSORS_SENSOR(button_c_sensor, BUTTON_SENSOR, value_c, config_c, status_c);
SENSORS_SENSOR(button_d_sensor, BUTTON_SENSOR, value_d, config_d, status_d);
/*---------------------------------------------------------------------------*/
/** @} */
