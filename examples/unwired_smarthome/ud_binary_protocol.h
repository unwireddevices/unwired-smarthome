/*
 * Copyright (c) 2010, Swedish Institute of Computer Science.
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
 */
/*---------------------------------------------------------------------------*/
/**
 * \file
 *         Defines file for Unwired Devices Binary Protocol(UDBP)
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 */
/*---------------------------------------------------------------------------*/

#define UDP_DATA_PORT                       4004

/* Inter-device protocol versions */
#define PROTOCOL_VERSION_V1                 0x01
#define DEVICE_VERSION_V1                   0x01

/* Data types */
#define DATA_TYPE_JOIN                      0x01
#define DATA_TYPE_SENSOR_DATA               0x02
#define DATA_TYPE_CONFIRM                   0x03
#define DATA_TYPE_PING                      0x04
#define DATA_TYPE_COMMAND                   0x05
#define DATA_TYPE_STATUS                    0x06

/* Reserved data */
#define DATA_RESERVED                       0xFF

/* Devices groups */
/* Показывает только общее назначение устройства */
#define DEVICE_GROUP_BUTTON_SWITCH          0x00
#define DEVICE_GROUP_SENSORS                0x01
#define DEVICE_GROUP_MOTION_SENSOR          0x02
#define DEVICE_GROUP_OPEN_SENSORS           0x03
#define DEVICE_GROUP_METERS                 0x04
#define DEVICE_GROUP_RELAY                  0x05
#define DEVICE_GROUP_DIMMER                 0x06
#define DEVICE_GROUP_LIGHT                  0x07
#define DEVICE_GROUP_RGB_LIGHT              0x08
#define DEVICE_GROUP_BRIDGE_CONVERTER       0x09
#define DEVICE_GROUP_OTHER                  0xFF

/* Devices ability's */
/* Определяет наличие в устройстве конкретных возможностей */
#define DEVICE_ABILITY_NONE                    0

#define DEVICE_ABILITY_BUTTON                  1
#define DEVICE_ABILITY_TEMPERATURE             2
#define DEVICE_ABILITY_HUMIDITY                3
#define DEVICE_ABILITY_PRESSURE                4
#define DEVICE_ABILITY_LIGHT_SENSOR            5
#define DEVICE_ABILITY_NOISE_SENSOR            6
#define DEVICE_ABILITY_MOTION_SENSOR           7
#define DEVICE_ABILITY_RESERVED1               8

#define DEVICE_ABILITY_C02_SENSOR              9
#define DEVICE_ABILITY_CO_SENSOR               10
#define DEVICE_ABILITY_GAS_SENSOR              11
#define DEVICE_ABILITY_POWER_METER             12
#define DEVICE_ABILITY_RESERVED2               13
#define DEVICE_ABILITY_RESERVED3               14
#define DEVICE_ABILITY_RESERVED4               15
#define DEVICE_ABILITY_RESERVED5               16

#define DEVICE_ABILITY_RELAY                   17
#define DEVICE_ABILITY_DIMMER                  18
#define DEVICE_ABILITY_RESERVED6               19
#define DEVICE_ABILITY_RESERVED7               20
#define DEVICE_ABILITY_RESERVED8               21
#define DEVICE_ABILITY_RESERVED9               22
#define DEVICE_ABILITY_RESERVED10              23
#define DEVICE_ABILITY_LED                     24

#define DEVICE_ABILITY_RESERVED11              25
#define DEVICE_ABILITY_RESERVED12              26
#define DEVICE_ABILITY_RESERVED13              27
#define DEVICE_ABILITY_RESERVED14              28
#define DEVICE_ABILITY_RESERVED15              29
#define DEVICE_ABILITY_RESERVED16              30
#define DEVICE_ABILITY_RS485_BRIGE             31
#define DEVICE_ABILITY_DALI_BRIGE              32

/* Devices sleep types */
#define DEVICE_SLEEP_TYPE_NORMAL                        0x01
#define DEVICE_SLEEP_TYPE_LEAF                          0x02

/* DEVICE_ABILITY_BUTTON events */
#define DEVICE_ABILITY_BUTTON_EVENT_CLICK               0x01
#define DEVICE_ABILITY_BUTTON_EVENT_LONG_CLICK          0x02
#define DEVICE_ABILITY_BUTTON_EVENT_ON                  0x03
#define DEVICE_ABILITY_BUTTON_EVENT_OFF                 0x04

/* DEVICE_ABILITY_LED commands */
#define DEVICE_ABILITY_LED_COMMAND_OFF                  0x00
#define DEVICE_ABILITY_LED_COMMAND_ON                   0x01
#define DEVICE_ABILITY_LED_COMMAND_BLINK                0x02

/* DEVICE_ABILITY_RELAY commands */
#define DEVICE_ABILITY_RELAY_COMMAND_OFF                0x00
#define DEVICE_ABILITY_RELAY_COMMAND_ON                 0x01
#define DEVICE_ABILITY_RELAY_COMMAND_TOGGLE             0x02


/* UART Binary data */
#define UART_PROTOCOL_VERSION_V1                        0x01
#define UART_DATA_LENGTH                                42
#define MAGIC_SEQUENCE_LENGTH                           6

