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

/* Resarved data */
#define DATA_RESERVED                       0xFF

/* Devices types */
#define DEVICE_TYPE_BUTTON                  0x01
#define DEVICE_TYPE_TEMPERATURE             0x02
#define DEVICE_TYPE_HUMIDITY                0x03
#define DEVICE_TYPE_PRESSURE                0x04
#define DEVICE_TYPE_LIGHT_SENSOR            0x05
#define DEVICE_TYPE_NOISE_SENSOR            0x06
#define DEVICE_TYPE_MOTION_SENSOR           0x07

/* Devices sleep types */
#define DEVICE_SLEEP_TYPE_NORMAL            0x01
#define DEVICE_SLEEP_TYPE_LEAF              0x02

/* Button events */
#define DEVICE_BUTTON_EVENT_CLICK           0x01
#define DEVICE_BUTTON_EVENT_LONG_CLICK      0x02
#define DEVICE_BUTTON_EVENT_ON              0x03
#define DEVICE_BUTTON_EVENT_OFF             0x04


/* UART Binary data */
#define UART_PROTOCOL_VERSION_V1            0x01
#define UART_DATA_LENGTH                    42
#define MAGIC_SEQUENCE_LENGTH               6

