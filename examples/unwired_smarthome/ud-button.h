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
PROCESS_NAME(udp_button_process);


void send_button_status_packet(const uip_ip6addr_t *dest_addr, struct simple_udp_connection *connection, char button_number);

const uint8_t device_version;
const uint8_t device_sleep_type;
const uint8_t device_type;
const uint8_t device_ability_1;
const uint8_t device_ability_2;
const uint8_t device_ability_3;
const uint8_t device_ability_4;

/*---------------------------------------------------------------------------*/
#endif /* UDP_LEDS_H_ */
