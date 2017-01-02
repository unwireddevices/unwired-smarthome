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
/** \addtogroup cc26xx-srf-tag
 * @{
 *
 * \defgroup srf06-cc26xx-peripherals Peripherals for the SmartRF06EB + CC26xxEM
 *
 * Defines related to the SmartRF06 Evaluation Board with a CC26xxEM
 *
 * This file provides connectivity information on LEDs, Buttons, UART and
 * other peripherals
 *
 * This file can be used as the basis to configure other boards using the
 * CC13xx/CC26xx code as their basis.
 *
 * This file is not meant to be modified by the user.
 * @{
 *
 * \file
 * Header file with definitions related to the I/O connections on the TI
 * SmartRF06 Evaluation Board with a CC26xxEM
 *
 * \note   Do not include this file directly. It gets included by contiki-conf
 *         after all relevant directives have been set.
 */
/*---------------------------------------------------------------------------*/
#ifndef BOARD_H_
#define BOARD_H_
/*---------------------------------------------------------------------------*/
#include "ioc.h"
/*---------------------------------------------------------------------------*/
/**
 * \name LED configurations
 *
 * Those values are not meant to be modified by the user
 * @{
 */
/* Some files include leds.h before us, so we need to get rid of defaults in
 * leds.h before we provide correct definitions */
#undef LEDS_GREEN
#undef LEDS_YELLOW
#undef LEDS_RED
#undef LEDS_CONF_ALL

#define LEDS_RED       1 /**< LED1 (Red)    */
#define LEDS_YELLOW    2 /**< LED2 (Yellow) */
#define LEDS_GREEN     4 /**< LED3 (Green)  */
#define LEDS_ORANGE    8 /**< LED4 (Orange) */

#define LEDS_CONF_ALL 15

/* Notify various examples that we have LEDs */
#define PLATFORM_HAS_LEDS        1
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name LED IOID mappings
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_LED_1          IOID_22
#define BOARD_IOID_LED_2          IOID_2
#define BOARD_IOID_LED_3          IOID_3
#define BOARD_IOID_LED_4          IOID_1
#define BOARD_LED_1               (1 << BOARD_IOID_LED_1)
#define BOARD_LED_2               (1 << BOARD_IOID_LED_2)
#define BOARD_LED_3               (1 << BOARD_IOID_LED_3)
#define BOARD_LED_4               (1 << BOARD_IOID_LED_4)
#define BOARD_LED_ALL             (BOARD_LED_1 | BOARD_LED_2 | BOARD_LED_3 | \
                                   BOARD_LED_4)

#define BOARD_IOID_LED_A          IOID_22
#define BOARD_IOID_LED_B          IOID_2
#define BOARD_IOID_LED_C          IOID_3
#define BOARD_IOID_LED_D          IOID_1
#define BOARD_LED_A               (1 << BOARD_IOID_LED_A)
#define BOARD_LED_B               (1 << BOARD_IOID_LED_B)
#define BOARD_LED_C               (1 << BOARD_IOID_LED_C)
#define BOARD_LED_D               (1 << BOARD_IOID_LED_D)
#define BOARD_LED_ALL             (BOARD_LED_A | BOARD_LED_B | BOARD_LED_C | \
                                   BOARD_LED_D)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name UART IOID mapping
 *
 * Those values are not meant to be modified by the user
 * @{
 */
//#define BOARD_IOID_UART_RX        IOID_26 //UMDK generic UART
//#define BOARD_IOID_UART_TX        IOID_25 //UMDK generic UART
#define BOARD_IOID_UART_RX        IOID_2 //Unwired mesh usb adapter
#define BOARD_IOID_UART_TX        IOID_3 //Unwired mesh usb adapter
#define BOARD_IOID_UART_CTS       IOID_UNUSED
#define BOARD_IOID_UART_RTS       IOID_UNUSED
#define BOARD_UART_RX             (1 << BOARD_IOID_UART_RX)
#define BOARD_UART_TX             (1 << BOARD_IOID_UART_TX)
#define BOARD_UART_CTS            (1 << BOARD_IOID_UART_CTS)
#define BOARD_UART_RTS            (1 << BOARD_IOID_UART_RTS)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Button IOID mapping
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_KEY_LEFT       IOID_4
#define BOARD_IOID_KEY_RIGHT      IOID_1
#define BOARD_IOID_KEY_UP         IOID_5
#define BOARD_IOID_KEY_DOWN       IOID_7
#define BOARD_IOID_KEY_SELECT     IOID_6
#define BOARD_KEY_LEFT            (1 << BOARD_IOID_KEY_LEFT)
#define BOARD_KEY_RIGHT           (1 << BOARD_IOID_KEY_RIGHT)
#define BOARD_KEY_UP              (1 << BOARD_IOID_KEY_UP)
#define BOARD_KEY_DOWN            (1 << BOARD_IOID_KEY_DOWN)
#define BOARD_KEY_SELECT          (1 << BOARD_IOID_KEY_SELECT)

#define BOARD_IOID_KEY_A            IOID_4
#define BOARD_IOID_KEY_B            IOID_1
#define BOARD_IOID_KEY_C            IOID_5
#define BOARD_IOID_KEY_D            IOID_7
#define BOARD_IOID_KEY_E            IOID_6
#define BOARD_KEY_A                 (1 << BOARD_IOID_KEY_A)
#define BOARD_KEY_B                 (1 << BOARD_IOID_KEY_B)
#define BOARD_KEY_C                 (1 << BOARD_IOID_KEY_C)
#define BOARD_KEY_D                 (1 << BOARD_IOID_KEY_D)
#define BOARD_KEY_E                 (1 << BOARD_IOID_KEY_E)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name 3.3V domain IOID mapping
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_3V3_EN         IOID_13
#define BOARD_3V3_EN              (1 << BOARD_IOID_3V3_EN)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SPI IOID mapping
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_SPI_SCK        IOID_UNUSED
#define BOARD_IOID_SPI_MOSI       IOID_UNUSED
#define BOARD_IOID_SPI_MISO       IOID_UNUSED
#define BOARD_SPI_SCK             (1 << BOARD_IOID_SPI_SCK)
#define BOARD_SPI_MOSI            (1 << BOARD_IOID_SPI_MOSI)
#define BOARD_SPI_MISO            (1 << BOARD_IOID_SPI_MISO)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \brief I2C IOID mappings
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_SDA            IOID_29 /**< Interface 0 SDA: All sensors bar MPU */
#define BOARD_IOID_SCL            IOID_30 /**< Interface 0 SCL: All sensors bar MPU */
#define BOARD_IOID_SDA_HP         IOID_29 /**< Interface 1 SDA: MPU */
#define BOARD_IOID_SCL_HP         IOID_30 /**< Interface 1 SCL: MPU */
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name LCD IOID mapping
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_LCD_MODE       IOID_UNUSED
#define BOARD_IOID_LCD_RST        IOID_UNUSED
#define BOARD_IOID_LCD_CS         IOID_UNUSED
#define BOARD_IOID_LCD_SCK        BOARD_IOID_SPI_SCK
#define BOARD_IOID_LCD_MOSI       BOARD_IOID_SPI_MOSI
#define BOARD_LCD_MODE            (1 << BOARD_IOID_LCD_MODE)
#define BOARD_LCD_RST             (1 << BOARD_IOID_LCD_RST)
#define BOARD_LCD_CS              (1 << BOARD_IOID_LCD_CS)
#define BOARD_LCD_SCK             BOARD_SPI_SCK
#define BOARD_LCD_MOSI            BOARD_SPI_MOSI
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name SD Card IOID mapping
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_SDCARD_CS      IOID_UNUSED
#define BOARD_SDCARD_CS           (1 << BOARD_IOID_SDCARD_CS)
#define BOARD_IOID_SDCARD_SCK     BOARD_IOID_SPI_SCK
#define BOARD_SDCARD_SCK          BOARD_SPI_SCK
#define BOARD_IOID_SDCARD_MOSI    BOARD_IOID_SPI_MOSI
#define BOARD_SDCARD_MOSI         BOARD_SPI_MOSI
#define BOARD_IOID_SDCARD_MISO    BOARD_IOID_SPI_MISO
#define BOARD_SDCARD_MISO         BOARD_SPI_MISO
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name ALS IOID mapping
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_ALS_PWR        IOID_UNUSED
#define BOARD_IOID_ALS_OUT        IOID_UNUSED
#define BOARD_ALS_PWR             (1 << BOARD_IOID_ALS_PWR)
#define BOARD_ALS_OUT             (1 << BOARD_IOID_ALS_OUT)
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name ACC IOID mapping
 *
 * Those values are not meant to be modified by the user
 * @{
 */
#define BOARD_IOID_ACC_PWR        IOID_UNUSED
#define BOARD_IOID_ACC_INT        IOID_UNUSED
#define BOARD_IOID_ACC_INT1       IOID_UNUSED
#define BOARD_IOID_ACC_INT2       IOID_UNUSED
#define BOARD_IOID_ACC_CS         IOID_UNUSED
#define BOARD_ACC_PWR             (1 << BOARD_IOID_ACC_PWR)
#define BOARD_ACC_INT             (1 << BOARD_IOID_ACC_INT)
#define BOARD_ACC_INT1            (1 << BOARD_IOID_ACC_INT1)
#define BOARD_ACC_INT2            (1 << BOARD_IOID_ACC_INT2)
#define BOARD_ACC_CS              (1 << BOARD_IOID_ACC_CS)
#define BOARD_IOID_ACC_SCK        BOARD_IOID_SPI_SCK
#define BOARD_ACC_SCK             BOARD_SPI_SCK
#define BOARD_IOID_ACC_MOSI       BOARD_IOID_SPI_MOSI
#define BOARD_ACC_MOSI            BOARD_SPI_MOSI
#define BOARD_IOID_ACC_MISO       BOARD_IOID_SPI_MISO
#define BOARD_ACC_MISO            BOARD_SPI_MISO
/** @} */
/*---------------------------------------------------------------------------*/
/**
 * \name Device string used on startup
 * @{
 */
#define BOARD_STRING "Unwired Devices udboards/CC2650 7x7"
/** @} */
/*---------------------------------------------------------------------------*/
#endif /* BOARD_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
