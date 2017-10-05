/*
 * Copyright (c) 2016, Unwired Devices LLC - http://www.unwireddevices.com/
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
 *         Header file for DAG-node service
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 */
/*---------------------------------------------------------------------------*/

#include "contiki.h"
#include "net/ip/uip.h"
/*---------------------------------------------------------------------------*/

#define MODE_NORMAL                             0x01
#define MODE_NOTROOT                            0x02
#define MODE_JOIN_PROGRESS                      0x03
#define MODE_NEED_REBOOT                        0x04

/*---------------------------------------------------------------------------*/

struct simple_udp_connection udp_connection;
volatile uip_ipaddr_t root_addr;
volatile uint8_t node_mode;

struct command_data
{
   volatile uint8_t data_type;
   volatile uint8_t protocol_version;
   volatile uint8_t device_version;
   volatile uint8_t ability_target;
   volatile uint8_t ability_number;
   volatile uint8_t ability_state;
   volatile uint8_t uart_returned_data_length;
   volatile uint8_t uart_data_length;
   volatile uint8_t ready_to_send;
   volatile uint8_t payload[16];
};

struct sensor_packet
{
   uint8_t protocol_version;
   uint8_t device_version;
   uint8_t data_type;
   uint8_t number_ability;
   uint8_t sensor_number;
   uint8_t sensor_event;
};

typedef union u8_u16_t
{
   uint16_t u16;
   uint8_t u8[2];
} u8_u16_t;

typedef union u8_i16_t
{
   int16_t i16;
   uint8_t u8[2];
} u8_i16_t;

typedef union u8_u32_t
{
   uint32_t u32;
   uint8_t u8[4];
} u8_u32_t;

void send_sensor_event(struct sensor_packet *sensor_packet);
void send_message_packet(uint8_t message_type, uint8_t data_1, uint8_t data_2);
void send_uart_data(struct command_data *uart_data);
void uart_console(unsigned char uart_char);
void send_time_sync_req_packet();

PROCESS_NAME(dag_node_process);
PROCESS_NAME(dag_node_button_process);
PROCESS_NAME(root_find_process);
PROCESS_NAME(status_send_process);
PROCESS_NAME(maintenance_process);
PROCESS_NAME(led_process);
PROCESS_NAME(fw_update_process);

/*---------------------------------------------------------------------------*/
