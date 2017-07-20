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
 */

/*---------------------------------------------------------------------------*/
/*
* \file
*         RPL-root service for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
* \author
*         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
*/
/*---------------------------------------------------------------------------*/

#include "contiki.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"

#include <stdio.h>
#include <string.h>

#include "../fake_headers.h" //no move up! not "krasivo"!

/*---------------------------------------------------------------------------*/

typedef struct firmware_packet
{
   uint8_t data[FIRMWARE_PAYLOAD_LENGTH];
} firmware_packet;

struct firmware_data
{
   volatile uip_ip6addr_t destination_address;
   volatile uint8_t protocol_version;
   volatile uint8_t device_version;
   volatile firmware_packet firmware_payload;
   volatile uint8_t chunk_number_b1;
   volatile uint8_t chunk_number_b2;
   volatile uint8_t reserved_b1;
   volatile uint8_t reserved_b2;
   volatile uint8_t ready_to_send;
};

struct firmware_cmd
{
   volatile uip_ip6addr_t destination_address;
   volatile uint8_t protocol_version;
   volatile uint8_t device_version;
   volatile uint8_t firmware_command;
   volatile uint8_t chunk_quantity_b1;
   volatile uint8_t chunk_quantity_b2;
   volatile uint8_t ready_to_send;
};

struct command_data
{
   volatile uip_ip6addr_t destination_address;
   volatile uint8_t protocol_version;
   volatile uint8_t device_version;
   volatile uint8_t ability_target;
   volatile uint8_t ability_number;
   volatile uint8_t ability_state;
   volatile uint8_t ready_to_send;
};

struct uart_data
{
   volatile uip_ip6addr_t destination_address;
   volatile uint8_t protocol_version;
   volatile uint8_t device_version;
   volatile uint8_t data_lenth;
   volatile uint8_t returned_data_lenth;
   volatile uint8_t payload[16];
   volatile uint8_t ready_to_send;
};

/* Received data via UART */
static struct command_data command_message;
static struct firmware_data firmware_message;
static struct firmware_cmd firmware_cmd_message;
static struct uart_data uart_message;


/* main UPD connection */
struct simple_udp_connection udp_connection;

/*---------------------------------------------------------------------------*/

void dag_root_raw_print(const uip_ip6addr_t *addr, const uint8_t *data, const uint16_t length);

void send_confirmation_packet(const uip_ip6addr_t *dest_addr);

void send_pong_packet(const uip_ip6addr_t *dest_addr);

void create_rpl_dag(uip_ipaddr_t *ipaddr);

uip_ipaddr_t *set_global_address(void);

void udp_data_receiver(struct simple_udp_connection *connection,
                              const uip_ipaddr_t *sender_addr,
                              uint16_t sender_port,
                              const uip_ipaddr_t *receiver_addr,
                              uint16_t receiver_port,
                              const uint8_t *data,
                              uint16_t datalen);

void send_uart_packet(struct uart_data *uart_message);

void send_firmware_cmd_packet(struct firmware_cmd *firmware_cmd_message);

void send_firmware_packet(struct firmware_data *firmware_message);

void send_command_packet(struct command_data *command_message);

int uart_data_receiver(unsigned char uart_char);

void uart_packet_dump(volatile uint8_t *uart_command_buf);

/*---------------------------------------------------------------------------*/

