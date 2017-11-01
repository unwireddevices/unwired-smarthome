/*
 * Copyright (c) 2016, Unwired Devices LLC - http://www.unwireddevices.com/
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Unwired Devices nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
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
#include "system-common.h"

#include "ti-lib.h"
#include "dev/cc26xx-uart.h"

#include "../ud_binary_protocol.h"
#include "xxf_types_helper.h"
#include "dev/watchdog.h"
#include "root-node.h"
#include "lpm.h"
#include "crypto-common.h"
#include "rtc-common.h"


#define UART_DATA_POLL_INTERVAL 5 //in main timer ticks, one tick ~8ms

static uint8_t lpm_mode_return(void);
void send_time_sync_resp_packet(const uip_ip6addr_t *dest_addr, const uint8_t *data, const uint16_t length);

LPM_MODULE(root_lpm_module, lpm_mode_return, NULL, NULL, LPM_DOMAIN_NONE);

/*---------------------------------------------------------------------------*/

/* The sequence of start and end command */
uint8_t uart_magic_sequence[MAGIC_SEQUENCE_LENGTH] = {MAGIC_SEQUENCE};
uint8_t uart_data[MAX_UART_DATA_LENGTH];

PROCESS(send_command_process, "UDP command sender");

/*---------------------------------------------------------------------------*/

static uint8_t lpm_mode_return(void)
{
   return LPM_MODE_AWAKE;
}

/*---------------------------------------------------------------------------*/

void send_confirmation_packet(const uip_ip6addr_t *dest_addr)
{
   if (dest_addr == NULL)
   {
      printf("UDM: dest_addr in send_confirmation_packet null\n");
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
      printf("UDM: dest_addr in send_pong_packet null\n");
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
   if (addr == NULL || data == NULL)
      return;

   if (length != 10 && length != 23)
   {
      printf("UDM: Incompatible data length(%" PRIu16 ")!\n", length);
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
             data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9]);
   }

   if (length == 23)
   {
      printf("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
             data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9],
             data[10], data[11], data[12], data[13], data[14], data[15], data[16], data[17], data[18], data[19],
             data[20], data[21], data[22]);
   }

   printf("RAWEND   \n");
}

/*---------------------------------------------------------------------------*/

void decrypted_data_processed(const uip_ip6addr_t *sender_addr, const uint8_t *data, uint16_t datalen)
{
   uint8_t packet_type = data[2];

   if (packet_type == DATA_TYPE_JOIN)
   {
      send_confirmation_packet(sender_addr);
   }

   else if (packet_type == DATA_TYPE_STATUS || packet_type == DATA_TYPE_SENSOR_DATA)
   {
      send_pong_packet(sender_addr);
   }

   else if (packet_type == DATA_TYPE_SET_TIME && data[3] == DATA_TYPE_SET_TIME_REQUEST)
   {
      send_time_sync_resp_packet(sender_addr, data, datalen);
      return;
   }

   dag_root_raw_print(sender_addr, data, datalen);
}

/*---------------------------------------------------------------------------*/

void encrypted_data_processed(const uip_ip6addr_t *sender_addr, const uint8_t *data, uint16_t datalen)
{
   printf("UDM: encrypted data received\n");
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

   uip_ip6addr_t node_addr;
   uip_ip6addr_copy(&node_addr, sender_addr);

   if (data[0] == PROTOCOL_VERSION_V1)
   {
      decrypted_data_processed(&node_addr, data, datalen);
   }
   else if (data[0] == PROTOCOL_VERSION_V2)
   {
      encrypted_data_processed(&node_addr, data, datalen);
   }

   led_off(LED_A);
}

/*---------------------------------------------------------------------------*/

void rpl_initialize()
{
   /* Set MESH-mode for dc-power rpl-root(not leaf-mode) */
   rpl_set_mode(RPL_MODE_MESH);

   static uip_ipaddr_t ipaddr;

   /* Fill in the address with zeros and the local prefix */
   uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);

   /* Generate an address based on the chip ID */
   uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);

   /* Adding autoconfigured address as the device address */
   uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

   /* make local address as rpl-root */
   rpl_set_root(RPL_DEFAULT_INSTANCE, &ipaddr);
   rpl_dag_t *dag = rpl_get_any_dag();

   uip_ipaddr_t prefix;
   uip_ip6addr(&prefix, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
   rpl_set_prefix(dag, &prefix, 64);

   printf("UDM: Created a new RPL DAG, i'm root!\n");

   printf("UDM: Time sync needed\n");
}

/*---------------------------------------------------------------------------*/

void root_node_initialize()
{
   /* register udp-connection, set incoming upd-data handler(udp_data_receiver) */
   simple_udp_register(&udp_connection, UDP_DATA_PORT, NULL, UDP_DATA_PORT, udp_data_receiver);

   /* set incoming uart-data handler(uart_data_receiver) */
   cc26xx_uart_set_input(&uart_data_receiver);

   /* blink-blink LED */
   led_blink(LED_A);
   led_blink(LED_A);

   /* set LPM mode to always awake */
   lpm_register_module(&root_lpm_module);

   /* start flag "data for udp ready" poller process */
   process_start(&send_command_process, NULL);
}

/*---------------------------------------------------------------------------*/

void send_time_sync_resp_packet(const uip_ip6addr_t *dest_addr, const uint8_t *data, const uint16_t length)
{
   if (dest_addr == NULL && udp_connection.udp_conn == NULL)
      return;

   time_data_t root_time;
   root_time = get_epoch_time();

   uint8_t udp_buffer[PROTOCOL_VERSION_V2_16BYTE];
   udp_buffer[0] = PROTOCOL_VERSION_V1;
   udp_buffer[1] = DEVICE_VERSION_V1;
   udp_buffer[2] = DATA_TYPE_SET_TIME;
   udp_buffer[3] = DATA_TYPE_SET_TIME_RESPONSE;
   udp_buffer[4] = root_time.seconds_u8[0];
   udp_buffer[5] = root_time.seconds_u8[1];
   udp_buffer[6] = root_time.seconds_u8[2];
   udp_buffer[7] = root_time.seconds_u8[3];
   udp_buffer[8] = root_time.milliseconds_u8[0];
   udp_buffer[9] = root_time.milliseconds_u8[1];
   udp_buffer[10] = data[10];
   udp_buffer[11] = data[11];
   udp_buffer[12] = data[12];
   udp_buffer[13] = data[13];
   udp_buffer[14] = data[14];
   udp_buffer[15] = data[15]; // << 16-byte packet, ready to encrypt v2 protocol

   printf("UDM: time sync responce sended: %" PRIu32 " sec, %" PRIu16 " ms\n", root_time.seconds, root_time.milliseconds);

   simple_udp_sendto(&udp_connection, udp_buffer, PROTOCOL_VERSION_V2_16BYTE, dest_addr);
}

/*---------------------------------------------------------------------------*/

void send_uart_packet(struct uart_data *uart_message)
{
   if (udp_connection.udp_conn == NULL)
      return;

   uip_ip6addr_t addr;
   uip_ip6addr_copy(&addr, &uart_message->destination_address);

   printf("UDM: returned_data_lenth: %" PRIXX8 "\n", uart_message->returned_data_lenth);
   printf("UDM: data_lenth: %" PRIXX8 "\n", uart_message->data_lenth);
   printf("UDM: payload: ");
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
   if (udp_connection.udp_conn == NULL)
      return;

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
   if (udp_connection.udp_conn == NULL)
      return;

   uip_ip6addr_t addr;
   uip_ip6addr_copy(&addr, &firmware_message->destination_address);

   uint16_t payload_length = firmware_message->chunk_size;
   uint16_t packet_length = payload_length + FIRMWARE_PAYLOAD_OFFSET;
   uint8_t udp_buffer[packet_length];

   udp_buffer[0] = firmware_message->protocol_version;
   udp_buffer[1] = firmware_message->device_version;
   udp_buffer[2] = DATA_TYPE_FIRMWARE;
   udp_buffer[3] = firmware_message->chunk_number_b1;
   udp_buffer[4] = firmware_message->chunk_number_b2;
   udp_buffer[5] = firmware_message->reserved_b1;
   udp_buffer[6] = firmware_message->reserved_b2; //7 = FIRMWARE_PAYLOAD_OFFSET

   for (uint16_t i = 0; i < payload_length; i++)
   {
      udp_buffer[FIRMWARE_PAYLOAD_OFFSET + i] = firmware_message->firmware_payload.data[i];
   }
   //printf("UDM: send fw packet %" PRIu16 " b\n", payload_length);
   simple_udp_sendto(&udp_connection, udp_buffer, packet_length, &addr);
}

/*---------------------------------------------------------------------------*/

void send_command_packet(struct command_data *command_message)
{
   if (udp_connection.udp_conn == NULL)
      return;

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

void uart_packet_dump(uint8_t *uart_buf, uint16_t uart_data_size)
{
   if (uart_buf == NULL)
   {
      printf("UDM: uart_command_buf in uart_packet_dump null\n");
      return;
   }

   printf("UDM: UART->6LP: ");
   for (uint16_t i = 0; i < uart_data_size; i++)
   {
      printf("%" PRIXX8, uart_buf[i]);
   }
   printf("\n");
}

/*---------------------------------------------------------------------------*/
/*
 * Локальная команда — команда, отправляемая координатору роутером, не передающаяся на другие устройства.
 */

void local_command(uint8_t *uart_data, uint16_t uart_data_length)
{
   uint8_t command_type = uart_data[18];
   uint8_t command_subtype = uart_data[19];

   if (command_type == DATA_TYPE_LOCAL_CMD)
   {
      if (command_subtype == LOCAL_ROOT_COMMAND_TIME_SET)
      {
         printf("UDM: local command time set\n");
         u8_u32_t epoch;
         for (uint8_t i = 0; i < 4; i++)
            epoch.u8[i] = uart_data[i+20];

         time_data_t new_time;
         new_time.seconds = epoch.u32;
         new_time.milliseconds = 0;

         set_epoch_time(new_time);

         printf("UDM: epoch time: %" PRIu32 "(%" PRIXX8 " %" PRIXX8 " %" PRIXX8 " %" PRIXX8 ")\n", epoch.u32, epoch.u8[0], epoch.u8[1], epoch.u8[2], epoch.u8[3]);
      }
      if (command_subtype == LOCAL_ROOT_COMMAND_BOOTLOADER_ACTIVATE)
      {
         printf("UDM: local command bootloader activate\n");
         ti_lib_flash_sector_erase(0x0001F000);
      }
      if (command_subtype == LOCAL_ROOT_COMMAND_REBOOT)
      {
         printf("UDM: local command reboot\n");
         ti_lib_sys_ctrl_system_reset();
      }
   }
}

/*---------------------------------------------------------------------------*/

void uart_packet_processed(uint8_t *uart_data, uint16_t uart_data_length)
{
   if (uart_data[18] == DATA_TYPE_LOCAL_CMD)
   {
      local_command(uart_data, uart_data_length);
   }

   if (uart_data[18] == DATA_TYPE_COMMAND)
   {
      for (uint8_t i = 0; i < 16; i++)
         command_message.destination_address.u8[i] = uart_data[i];

      command_message.protocol_version = uart_data[16];
      command_message.device_version = uart_data[17];

      command_message.ability_target = uart_data[19];
      command_message.ability_number = uart_data[20];
      command_message.ability_state = uart_data[21];
      command_message.ready_to_send = 1;
   }

   if (uart_data[18] == DATA_TYPE_FIRMWARE)
   {
      uint16_t fw_payload_lenght = uart_data_length - 23; // 23 = 16(address) + 7(packet header)
      for (uint8_t i = 0; i < 16; i++)
         firmware_message.destination_address.u8[i] = uart_data[i];

      firmware_message.protocol_version = uart_data[16];
      firmware_message.device_version = uart_data[17];

      firmware_message.chunk_number_b1 = uart_data[19];
      firmware_message.chunk_number_b2 = uart_data[20];
      firmware_message.reserved_b1 = uart_data[21];
      firmware_message.reserved_b2 = uart_data[22];
      firmware_message.chunk_size = fw_payload_lenght;

      for (uint16_t i = 0; i < fw_payload_lenght; i++)
         firmware_message.firmware_payload.data[i] = uart_data[23 + i];

      firmware_message.ready_to_send = 1;
   }

   if (uart_data[18] == DATA_TYPE_FIRMWARE_CMD)
   {
      for (uint8_t i = 0; i < 16; i++)
         firmware_cmd_message.destination_address.u8[i] = uart_data[i];

      firmware_cmd_message.protocol_version = uart_data[16];
      firmware_cmd_message.device_version = uart_data[17];

      firmware_cmd_message.firmware_command = uart_data[19];
      firmware_cmd_message.chunk_quantity_b1 = uart_data[20];
      firmware_cmd_message.chunk_quantity_b2 = uart_data[21];
      firmware_cmd_message.ready_to_send = 1;
      //printf("UDM: chunk_quantity: 0x%" PRIXX8 " 0x%" PRIXX8 "\n", uart_data[20], uart_data[21]);
   }

   if (uart_data[26] == DATA_TYPE_UART)
   {
      for (uint8_t i = 0; i < 16; i++)
         uart_message.destination_address.u8[i] = uart_data[i];

      uart_message.protocol_version = uart_data[16];
      uart_message.device_version = uart_data[17];

      uart_message.data_lenth = uart_data[19];
      uart_message.returned_data_lenth = uart_data[20];
      for (uint8_t i = 0; i < 16; i++)
         uart_message.payload[i] = uart_data[21 + i];

      uart_message.ready_to_send = 1;
   }
}

/*---------------------------------------------------------------------------*/
int uart_data_receiver(unsigned char uart_char)
{
   led_blink(LED_A);

   static uint8_t uart_header_iterator = 0;
   static uint8_t uart_header_data[PACKET_HEADER_LENGTH - MAGIC_SEQUENCE_LENGTH];
   static uint16_t uart_data_iterator = 0;
   static uint16_t uart_data_length = 0;

   static uint8_t uart_packet_crc = 0;

   if (uart_header_iterator < PACKET_HEADER_LENGTH &&
       uart_header_iterator < MAGIC_SEQUENCE_LENGTH) //Read and verify magic seq
   {
      //printf("UDM: New char(%" PRIXX8 ") in mq: %" PRIu16 "\n", uart_char, uart_header_iterator);
      if (uart_char != uart_magic_sequence[uart_header_iterator])
      {
         uart_header_iterator = 0;
         uart_data_iterator = 0;
         uart_data_length = 0;
         uart_packet_crc = 0;
         return 1;
      }
      uart_header_iterator++;
      return 1;
   }

   if (uart_header_iterator < PACKET_HEADER_LENGTH &&
       uart_header_iterator >= MAGIC_SEQUENCE_LENGTH) //Read header data
   {
      //printf("UDM: New char(%" PRIXX8 ") in data header buffer: %" PRIu16 "\n", uart_char, uart_header_iterator);
      uart_header_data[uart_header_iterator - MAGIC_SEQUENCE_LENGTH] = uart_char;

      uart_header_iterator++;
   }

   if (uart_header_iterator == PACKET_HEADER_LENGTH)
   {
      //printf("UDM: check uart header data\n");
      if (uart_header_data[0] != UART_PROTOCOL_VERSION_V3) //Check uart protocol version
      {
         printf("UDM: Incompatible protocol version: %" PRIXX8 "!\n", uart_header_data[0]);
         uart_header_iterator = 0;
         uart_data_iterator = 0;
         uart_data_length = 0;
         uart_packet_crc = 0;
         return 1;
      }
      uint8_t uart_data_length_uint8[2];
      uart_data_length_uint8[0] = uart_header_data[1]; //Можно использовать u8_u16_t
      uart_data_length_uint8[1] = uart_header_data[2];
      uint16_t *uart_data_length_uint16_t = (uint16_t *)&uart_data_length_uint8; //Convert data length
      uart_data_length = *uart_data_length_uint16_t;
      if (uart_data_length > MAX_UART_DATA_LENGTH)
      {
         printf("UDM: too big packet size: %" PRIu16 "\n", uart_data_length);
         uart_header_iterator = 0;
         uart_data_iterator = 0;
         uart_data_length = 0;
         uart_packet_crc = 0;
         return 1;
      }
      uart_packet_crc = uart_header_data[3];
      uart_header_iterator++;
      //printf("UDM: new buffer %" PRIu16 " bytes, uart version %" PRIXX8 ", crc %" PRIXX8 "\n", uart_data_length, uart_header_data[0], uart_packet_crc);
      return 1;
   }

   if (uart_data_iterator >= MAX_UART_DATA_LENGTH)
   {
      printf("UDM: too big packet size\n");
      uart_header_iterator = 0;
      uart_data_iterator = 0;
      uart_data_length = 0;
      uart_packet_crc = 0;
      return 1;
   }

   if (uart_data_length > 0) //Read uart data
   {
      //printf("UDM: New byte(%"PRIu16"/%"PRIu16") in buffer: %" PRIXX8 "\n", uart_data_iterator, uart_data_length, uart_char);
      uart_data[uart_data_iterator] = uart_char;
      uart_data_iterator++;
   }

   if (uart_data_length > 0 && uart_data_iterator == uart_data_length) //Convert uart data after full buffer
   {
      //printf("UDM: end packet\n");
      //uart_packet_dump(uart_data, uart_data_length);

      uart_packet_processed(uart_data, uart_data_length);

      uart_header_iterator = 0;
      uart_data_iterator = 0;
      uart_data_length = 0;
      uart_packet_crc = 0;
      return 1;
   }

   return 1;
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(send_command_process, ev, data)
{
   PROCESS_BEGIN();

   static struct etimer send_command_process_timer;
   PROCESS_PAUSE();

   while (1)
   {
      etimer_set(&send_command_process_timer, UART_DATA_POLL_INTERVAL);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_command_process_timer));

      if (command_message.ready_to_send != 0)
      {
         disable_interrupts();
         send_command_packet(&command_message);
         command_message.ready_to_send = 0;
         enable_interrupts();
      }

      if (firmware_message.ready_to_send != 0)
      {
         disable_interrupts();
         send_firmware_packet(&firmware_message);
         firmware_message.ready_to_send = 0;
         enable_interrupts();
      }

      if (firmware_cmd_message.ready_to_send != 0)
      {
         disable_interrupts();
         send_firmware_cmd_packet(&firmware_cmd_message);
         firmware_cmd_message.ready_to_send = 0;
         enable_interrupts();
      }

      if (uart_message.ready_to_send != 0)
      {
         disable_interrupts();
         send_uart_packet(&uart_message);
         uart_message.ready_to_send = 0;
         enable_interrupts();
      }
   }

   PROCESS_END();
}

/*---------------------------------------------------------------------------*/
