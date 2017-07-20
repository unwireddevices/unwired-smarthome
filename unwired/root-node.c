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
#include "lib/random.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"

#include "dev/leds.h"
#include "cc26xx/board.h"

#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ip/uip-debug.h"
#include "simple-udp.h"
#include "net/rpl/rpl.h"

#include <stdio.h>
#include <string.h>

#include "button-sensor.h"
#include "board-peripherals.h"

#include "ti-lib.h"
#include "dev/cc26xx-uart.h"

#include "../ud_binary_protocol.h"
#include "xxf_types_helper.h"
#include "dev/watchdog.h"
#include "root-node.h"


#include "../fake_headers.h" //no move up! not "krasivo"!

#define UART_DATA_POLL_INTERVAL 5 //in main timer ticks, one tick ~8ms

/*---------------------------------------------------------------------------*/

/* UART char iterator */
volatile uint8_t uart_iterator = 0;

/* UART-buffer for raw command */
volatile uint8_t uart_command_buf[UART_DATA_LENGTH];

/* The sequence of start and end command */
uint8_t uart_magic_sequence[6] = {0x01,0x16,0x16,0x16,0x16,0x10};

/*---------------------------------------------------------------------------*/

void send_confirmation_packet(const uip_ip6addr_t *dest_addr)
{
   if (dest_addr == NULL)
   {
      printf("ERROR: dest_addr in send_confirmation_packet null\n");
      return;
   }

   uint8_t length = 10;
   uint8_t udp_buffer[length];
   udp_buffer[0] = PROTOCOL_VERSION_V1;
   udp_buffer[1] = DEVICE_VERSION_V1;
   udp_buffer[2] = DATA_TYPE_JOIN_CONFIRM;
   udp_buffer[3] = DATA_RESERVED;
   udp_buffer[4] = DATA_RESERVED;
   udp_buffer[5] = DATA_RESERVED;
   udp_buffer[6] = DATA_RESERVED;
   udp_buffer[7] = DATA_RESERVED;
   udp_buffer[8] = DATA_RESERVED;
   udp_buffer[9] = DATA_RESERVED;
   simple_udp_sendto(&udp_connection, udp_buffer, length, dest_addr);
}

/*---------------------------------------------------------------------------*/

void send_pong_packet(const uip_ip6addr_t *dest_addr)
{
   if (dest_addr == NULL)
   {
      printf("ERROR: dest_addr in send_pong_packet null\n");
      return;
   }

   uint8_t length = 10;
   uint8_t udp_buffer[length];
   udp_buffer[0] = PROTOCOL_VERSION_V1;
   udp_buffer[1] = DEVICE_VERSION_V1;
   udp_buffer[2] = DATA_TYPE_PONG;
   udp_buffer[3] = DATA_RESERVED;
   udp_buffer[4] = DATA_RESERVED;
   udp_buffer[5] = DATA_RESERVED;
   udp_buffer[6] = DATA_RESERVED;
   udp_buffer[7] = DATA_RESERVED;
   udp_buffer[8] = DATA_RESERVED;
   udp_buffer[9] = DATA_RESERVED;
   simple_udp_sendto(&udp_connection, udp_buffer, length, dest_addr);
}

/*---------------------------------------------------------------------------*/

void dag_root_raw_print(const uip_ip6addr_t *addr, const uint8_t *data, const uint16_t length)
{
   if (addr == NULL)
   {
      printf("ERROR: addr in dag_root_raw_print null\n");
      return;
   }
   if (data == NULL)
   {
      printf("ERROR: data in dag_root_raw_print null\n");
      return;
   }

   if (length != 10 && length != 23)
   {
      printf("DAG NODE: Incompatible data length(%" PRIu16 ")!\n", length);
      return;
   }
   printf("DAGROOTRAW1");
   printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
          ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2],
          ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5],
          ((uint8_t *)addr)[6], ((uint8_t *)addr)[7], ((uint8_t *)addr)[8],
          ((uint8_t *)addr)[9], ((uint8_t *)addr)[10], ((uint8_t *)addr)[11],
          ((uint8_t *)addr)[12], ((uint8_t *)addr)[13], ((uint8_t *)addr)[14],
          ((uint8_t *)addr)[15]);

   if (length == 10)
   {
      printf("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02XFFFFFFFFFFFFFFFFFFFFFFFFFF",
             data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9]);
   }

   if (length == 23)
   {
      printf("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
             data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],
             data[10],data[11],data[12],data[13],data[14],data[15],data[16],data[17],data[18],data[19],
             data[20],data[21],data[22]);
   }

   printf("RAWEND   \n");
}

/*---------------------------------------------------------------------------*/

void udp_data_receiver(struct simple_udp_connection *connection,
                              const uip_ipaddr_t *sender_addr,
                              uint16_t sender_port,
                              const uip_ipaddr_t *receiver_addr,
                              uint16_t receiver_port,
                              const uint8_t *data,
                              uint16_t datalen)
{
   led_on(LED_A);

   dag_root_raw_print(sender_addr, data, datalen);

   if (data[0] == PROTOCOL_VERSION_V1 && data[2] == DATA_TYPE_JOIN)
   {
      send_confirmation_packet(sender_addr);
   }

   if (data[0] == PROTOCOL_VERSION_V1 && (data[2] == DATA_TYPE_STATUS ||
                                          data[2] == DATA_TYPE_SENSOR_DATA) )
   {
      send_pong_packet(sender_addr);
   }

   led_off(LED_A);
}

/*---------------------------------------------------------------------------*/

uip_ipaddr_t *set_global_address(void)
{
   static uip_ipaddr_t ipaddr;

   /* Fill in the address with zeros and the local prefix */
   uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);

   /* Generate an address based on the chip ID */
   uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);

   /* Adding autoconfigured address as the device address */
   uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

   return &ipaddr;
}

/*---------------------------------------------------------------------------*/


void create_rpl_dag(uip_ipaddr_t *ipaddr)
{
   if (ipaddr == NULL)
   {
      printf("ERROR: ipaddr in create_rpl_dag null\n");
      return;
   }
   struct uip_ds6_addr *root_if = uip_ds6_addr_lookup(ipaddr);
   if (root_if != NULL)
   {
      uip_ipaddr_t prefix;
      rpl_set_root(RPL_DEFAULT_INSTANCE, ipaddr);
      rpl_dag_t *dag = rpl_get_any_dag();
      uip_ip6addr(&prefix, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
      rpl_set_prefix(dag, &prefix, 64);
      printf("Created a new RPL DAG, i'm root!\n");
   }
   else
   {
      printf("Failed to create a new RPL DAG :(\n");
   }
}

/*---------------------------------------------------------------------------*/

void send_uart_packet(struct uart_data *uart_message)
{
   if (&uart_message->destination_address == NULL)
   {
      printf("ERROR: dest_addr in send_uart_packet null\n");
      return;
   }
   if (&udp_connection.udp_conn == NULL)   //указатель на что?
   {
      printf("ERROR: connection in send_uart_packet null\n");
      return;
   }

   uip_ip6addr_t addr;
   uip_ip6addr_copy(&addr, &uart_message->destination_address);


   printf("UART: returned_data_lenth: %" PRIXX8 "\n", uart_message->returned_data_lenth);
   printf("UART: data_lenth: %" PRIXX8 "\n", uart_message->data_lenth);
   printf("UART: payload: ");
   for (int i = 0; i < uart_message->data_lenth; i++)
   {
      printf("0x%" PRIXX8 " ", uart_message->payload[i]);
   }
   printf("\n");


   uint8_t length = 23;
   uint8_t udp_buffer[length];

   udp_buffer[0] = uart_message->protocol_version;
   udp_buffer[1] = uart_message->device_version;
   udp_buffer[2] = DATA_TYPE_UART;

   udp_buffer[3] = uart_message->returned_data_lenth;
   udp_buffer[4] = uart_message->data_lenth;
   udp_buffer[5] = uart_message->payload[0];
   udp_buffer[6] = uart_message->payload[1];
   udp_buffer[7] = uart_message->payload[2];
   udp_buffer[8] = uart_message->payload[3];
   udp_buffer[9] = uart_message->payload[4];
   udp_buffer[10] = uart_message->payload[5];
   udp_buffer[11] = uart_message->payload[6];
   udp_buffer[12] = uart_message->payload[7];
   udp_buffer[13] = uart_message->payload[8];
   udp_buffer[14] = uart_message->payload[9];
   udp_buffer[15] = uart_message->payload[10];
   udp_buffer[16] = uart_message->payload[11];
   udp_buffer[17] = uart_message->payload[12];
   udp_buffer[18] = uart_message->payload[13];
   udp_buffer[19] = uart_message->payload[14];
   udp_buffer[20] = uart_message->payload[15];
   udp_buffer[21] = DATA_RESERVED;
   udp_buffer[22] = DATA_RESERVED;

   simple_udp_sendto(&udp_connection, udp_buffer, length, &addr);
}

/*---------------------------------------------------------------------------*/

void send_firmware_cmd_packet(struct firmware_cmd *firmware_cmd_message)
{
   if (&firmware_cmd_message->destination_address == NULL)
   {
      printf("ERROR: dest_addr in send_command_packet null\n");
      return;
   }
   if (&udp_connection.udp_conn == NULL)   //указатель на что?
   {
      printf("ERROR: connection in send_command_packet null\n");
      return;
   }

   uip_ip6addr_t addr;
   uip_ip6addr_copy(&addr, &firmware_cmd_message->destination_address);

   uint8_t length = 10;
   uint8_t udp_buffer[length];
   udp_buffer[0] = PROTOCOL_VERSION_V1;
   udp_buffer[1] = DEVICE_VERSION_V1;
   udp_buffer[2] = DATA_TYPE_FIRMWARE_CMD;
   udp_buffer[3] = DATA_TYPE_FIRMWARE_COMMAND_NEW_FW;
   udp_buffer[4] = firmware_cmd_message->chunk_quantity_b1;
   udp_buffer[5] = firmware_cmd_message->chunk_quantity_b2;
   udp_buffer[6] = DATA_RESERVED;
   udp_buffer[7] = DATA_RESERVED;
   udp_buffer[8] = DATA_RESERVED;
   udp_buffer[9] = DATA_RESERVED;
   simple_udp_sendto(&udp_connection, udp_buffer, length, &addr);
}

/*---------------------------------------------------------------------------*/

void send_firmware_packet(struct firmware_data *firmware_message)
{
   if (&firmware_message->destination_address == NULL)
   {
      printf("ERROR: dest_addr in send_command_packet null\n");
      return;
   }
   if (&udp_connection.udp_conn == NULL)   //указатель на что?
   {
      printf("ERROR: connection in send_command_packet null\n");
      return;
   }

   uip_ip6addr_t addr;
   uip_ip6addr_copy(&addr, &firmware_message->destination_address);

   uint8_t payload_length = FIRMWARE_PAYLOAD_LENGTH;
   uint8_t packet_length = payload_length + FIRMWARE_PAYLOAD_OFFSET;
   uint8_t udp_buffer[packet_length];

   udp_buffer[0] = firmware_message->protocol_version;
   udp_buffer[1] = firmware_message->device_version;
   udp_buffer[2] = DATA_TYPE_FIRMWARE;
   udp_buffer[3] = firmware_message->chunk_number_b1;
   udp_buffer[4] = firmware_message->chunk_number_b2;
   udp_buffer[5] = firmware_message->reserved_b1;
   udp_buffer[6] = firmware_message->reserved_b2; //FIRMWARE_PAYLOAD_OFFSET

   for (uint16_t i = 0; i < payload_length; i++)
   {
      udp_buffer[FIRMWARE_PAYLOAD_OFFSET + i] = firmware_message->firmware_payload.data[i];
   }

   simple_udp_sendto(&udp_connection, udp_buffer, packet_length, &addr);

}

/*---------------------------------------------------------------------------*/

void send_command_packet(struct command_data *command_message)
{
   if (&command_message->destination_address == NULL)
   {
      printf("ERROR: dest_addr in send_command_packet null\n");
      return;
   }
   if (&udp_connection.udp_conn == NULL)   //указатель на что?
   {
      printf("ERROR: connection in send_command_packet null\n");
      return;
   }

   uip_ip6addr_t addr;
   uip_ip6addr_copy(&addr, &command_message->destination_address);

   uint8_t length = 10;
   uint8_t udp_buffer[length];
   udp_buffer[0] = command_message->protocol_version;
   udp_buffer[1] = command_message->device_version;
   udp_buffer[2] = DATA_TYPE_COMMAND;
   udp_buffer[3] = command_message->ability_target;
   udp_buffer[4] = command_message->ability_number;
   udp_buffer[5] = command_message->ability_state;
   udp_buffer[6] = DATA_RESERVED;
   udp_buffer[7] = DATA_RESERVED;
   udp_buffer[8] = DATA_RESERVED;
   udp_buffer[9] = DATA_RESERVED;
   simple_udp_sendto(&udp_connection, udp_buffer, length, &addr);

}

/*---------------------------------------------------------------------------*/

void uart_packet_dump(volatile uint8_t *uart_command_buf)
{
   if (uart_command_buf == NULL)
   {
      printf("ERROR: uart_command_buf in uart_packet_dump null\n");
      return;
   }

   printf("\nUART->6LP: ");
   for (int i = 0; i < UART_DATA_LENGTH; i++)
   {
      printf("%"PRIXX8, uart_command_buf[i]);
   }
   printf("\n");
}

/*---------------------------------------------------------------------------*/

int uart_data_receiver(unsigned char uart_char)
{
   led_blink(LED_A);

   if (uart_iterator < UART_DATA_LENGTH)
   {
      uart_command_buf[uart_iterator] = uart_char;
      //printf("UDCP: New char(%" PRIXX8 ") in buffer: %" PRIu8 "\n", uart_char, uart_iterator);
      if (uart_iterator < MAGIC_SEQUENCE_LENGTH)
      {
         if (uart_char != uart_magic_sequence[uart_iterator])
         {
            //printf("UDCP: Char 0x%" PRIXX8 "(#%" PRIu8 ") non-MQ!\n", uart_char, uart_iterator);
            uart_iterator = 0;
            return 1;
         }
      }
      uart_iterator++;
   }

   if (uart_iterator == UART_DATA_LENGTH)
   {
      uart_iterator = 0;
      uart_packet_dump(uart_command_buf);
      if (uart_command_buf[6] == UART_PROTOCOL_VERSION_V2)
      {
         if (uart_command_buf[26] == DATA_TYPE_COMMAND)
         {
            for (uint8_t i = 0; i < 16; i++)
            {
               command_message.destination_address.u8[i] = uart_command_buf[8 + i];
            }
            command_message.protocol_version = uart_command_buf[24];
            command_message.device_version = uart_command_buf[25];
            command_message.ability_target = uart_command_buf[27];
            command_message.ability_number = uart_command_buf[28];
            command_message.ability_state = uart_command_buf[29];
            command_message.ready_to_send = 1;
         }

         if (uart_command_buf[26] == DATA_TYPE_FIRMWARE)
         {
            for (uint8_t i = 0; i < 16; i++)
            {
               firmware_message.destination_address.u8[i] = uart_command_buf[8 + i];
            }
            firmware_message.protocol_version = uart_command_buf[24];
            firmware_message.device_version = uart_command_buf[25];
            firmware_message.chunk_number_b1 = uart_command_buf[27];
            firmware_message.chunk_number_b2 = uart_command_buf[28];
            firmware_message.reserved_b1 = uart_command_buf[29];
            firmware_message.reserved_b2 = uart_command_buf[30];
            for (uint8_t i = 0; i < FIRMWARE_PAYLOAD_LENGTH; i++)
            {
               firmware_message.firmware_payload.data[i] = uart_command_buf[31 + i];
            }
            firmware_message.ready_to_send = 1;
         }

         if (uart_command_buf[26] == DATA_TYPE_FIRMWARE_CMD)
         {
            for (uint8_t i = 0; i < 16; i++)
            {
               firmware_cmd_message.destination_address.u8[i] = uart_command_buf[8 + i];
            }
            firmware_cmd_message.protocol_version = uart_command_buf[24];
            firmware_cmd_message.device_version = uart_command_buf[25];
            firmware_cmd_message.firmware_command = uart_command_buf[27];
            firmware_cmd_message.chunk_quantity_b1 = uart_command_buf[28];
            firmware_cmd_message.chunk_quantity_b2 = uart_command_buf[29];
            firmware_cmd_message.ready_to_send = 1;
         }

         if (uart_command_buf[26] == DATA_TYPE_UART)
         {
            for (uint8_t i = 0; i < 16; i++)
            {
               uart_message.destination_address.u8[i] = uart_command_buf[8 + i];
            }
            uart_message.protocol_version = uart_command_buf[24];
            uart_message.device_version = uart_command_buf[25];
            uart_message.data_lenth = uart_command_buf[27];
            uart_message.returned_data_lenth = uart_command_buf[28];
            for (uint8_t i = 0; i < 16; i++)
            {
               uart_message.payload[i] = uart_command_buf[29 + i];
            }
            uart_message.ready_to_send = 1;
         }
      }
      else
      {
         printf("UDCP: Incompatible protocol version: %" PRIXX8 "!\n", uart_command_buf[6]);
      }

   }

   return 1;
}

/*---------------------------------------------------------------------------*/


