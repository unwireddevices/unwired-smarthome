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
 *         Header file
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 */
/*---------------------------------------------------------------------------*/
#ifndef UDP_BUTTON_H_
#define UDP_BUTTON_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "net/ip/uip.h"
/*---------------------------------------------------------------------------*/
#define CURRENT_DEVICE_SLEEP_TYPE             DEVICE_SLEEP_TYPE_LEAF
#define CURRENT_DEVICE_GROUP                  DEVICE_GROUP_BUTTON_SWITCH
#define CURRENT_DEVICE_VERSION                DEVICE_VERSION_V1
#define CURRENT_PROTOCOL_VERSION              PROTOCOL_VERSION_V1
#define CURRENT_ABILITY_1BYTE                 0b10000000
#define CURRENT_ABILITY_2BYTE                 0b00000000
#define CURRENT_ABILITY_3BYTE                 0b00000000
#define CURRENT_ABILITY_4BYTE                 0b00000000

PROCESS_NAME(main_process);

struct sensor_packet {
uint8_t protocol_version;
uint8_t device_version;
uint8_t data_type;
uint8_t number_ability;
uint8_t sensor_number;
uint8_t sensor_event;
};

void send_sensor_event(struct sensor_packet *packet,
                 const uip_ip6addr_t *dest_addr,
                 struct simple_udp_connection *connection);

/*---------------------------------------------------------------------------*/
#endif /* UDP_LEDS_H_ */
