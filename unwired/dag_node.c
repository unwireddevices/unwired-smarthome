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
 *         DAG-node service for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 */
/*---------------------------------------------------------------------------*/
#define DPRINT printf(">dag_node.c:%"PRIu16"\n", __LINE__);watchdog_periodic();

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "clock.h"

#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ip/uip-debug.h"
#include "net/ip/simple-udp.h"
#include "net/link-stats.h"

#include "dev/leds.h"
#include "sys/clock.h"
#include "shell.h"
#include "serial-shell.h"
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "radio_power.h"
#include "dev/watchdog.h"
#include "dev/cc26xx-uart.h"
#include "board-peripherals.h"
#include "board.h"
#include "ti-lib.h"

#include "ota-main.h"
#include "ota-common.h"
#include "crypto-common.h"
#include "rtc-common.h"
#include "system-common.h"
#include "int-flash-common.h"

#include "xxf_types_helper.h"
#include "ud_binary_protocol.h"

#include "dag_node.h"

#ifdef IF_UD_BUTTON
#  include "smarthome/button.h"
#endif

#ifdef IF_UD_RELAY
#  include "smarthome/relay.h"
#endif

#ifdef IF_UD_DIMMER
#  include "smarthome/dimmer.h"
#endif

#ifdef IF_UD_LIGHT
#  include "asuno-light/light.h"
#endif

#ifdef IF_UD_MOTIONSENSOR
#  include "smarthome/motionsensor.h"
#endif

#ifdef IF_UD_WMETER
#  include "water-meter/wmeter.h"
#endif

#define MAINTENANCE_INTERVAL            (10 * 60 * CLOCK_SECOND)
#define SHORT_STATUS_INTERVAL           (10 * 60 * CLOCK_SECOND)
#define LONG_STATUS_INTERVAL            (20 * 60 * CLOCK_SECOND)
#define ROOT_FIND_INTERVAL                    (2 * CLOCK_SECOND)
#define ROOT_FIND_LIMIT_TIME             (2 * 60 * CLOCK_SECOND)
#define FW_DELAY                              (2 * CLOCK_SECOND)
#define FW_MAX_ERROR_COUNTER                    5

#define FALSE                                   0x00
#define TRUE                                    0x01

#define LED_OFF                                 0x00
#define LED_ON                                  0x01
#define LED_FLASH                               0x02
#define LED_SLOW_BLINK                          0x03
#define LED_FAST_BLINK                          0x04

#define MAX_NON_ANSWERED_PINGS                  3

#define HEXVIEW_MODE                            0x00
#define HEXRAW_MODE                             0x01

/*---------------------------------------------------------------------------*/

/* struct for simple_udp_send */
simple_udp_connection_t udp_connection;

volatile uint8_t node_mode;
volatile uint8_t spi_status;

volatile uint8_t led_mode;
static void led_mode_set(uint8_t mode);

volatile uint8_t non_answered_packet = 0;
volatile uip_ipaddr_t root_addr;
static struct command_data message_for_main_process;

static struct etimer maintenance_timer;
static struct etimer fw_timer;

volatile u8_u16_t fw_chunk_quantity;
volatile uint16_t fw_ext_flash_address = 0;
volatile uint8_t fw_error_counter = 0;

uint32_t ota_image_current_offset = 0;

rpl_dag_t *rpl_probing_dag;

/*---------------------------------------------------------------------------*/

PROCESS(dag_node_process, "DAG-node process");
PROCESS(dag_node_button_process, "DAG-node button process");
PROCESS(root_find_process, "Root find process");
PROCESS(status_send_process, "Status send process");
PROCESS(maintenance_process, "Maintenance process");
PROCESS(led_process, "Led process");
PROCESS(fw_update_process, "FW OTA update process");

/*---------------------------------------------------------------------------*/

void
flash_erase(size_t offset, size_t length)
{
   printf("SPIFLASH: flash clean\n");
   ext_flash_open();
   ext_flash_erase(offset, length);
   ext_flash_close();
}

/*---------------------------------------------------------------------------*/

void
flash_damp_hex(uint8_t mode)
{
   const uint32_t start_adress = (ota_images[1-1] << 12);
   const uint32_t read_length = 0x400;
   uint8_t flash_read_data_buffer[read_length];

   printf("SPIFLASH DAMP: \n");
   for (uint8_t page=0; page < 100; page++ )
   {
      watchdog_periodic();
      ext_flash_open();
      bool eeprom_access = ext_flash_read(start_adress+(read_length*page), read_length, flash_read_data_buffer);
      ext_flash_close();

      if(!eeprom_access)
      {
         printf("SPIFLASH: Error - Could not read EEPROM\n");
      }
      else
      {
         if (mode == HEXVIEW_MODE)
            hexview_print(read_length, flash_read_data_buffer, start_adress+(read_length*page));
         if (mode == HEXRAW_MODE)
            hexraw_print(read_length, flash_read_data_buffer);
      }
   }
   printf("\nSPIFLASH DAMP END \n");

}

/*---------------------------------------------------------------------------*/

void
verify_int_firmware_v()
{
   printf("\n\n");

   printf("Internal firmware:\n");
   OTAMetadata_t current_firmware;
   get_current_metadata( &current_firmware );
   print_metadata(&current_firmware);
   verify_current_firmware( &current_firmware );

   printf("\n\n");

   printf("Golden image firmware:\n");
   OTAMetadata_t golden_image;
   get_ota_slot_metadata(0, &golden_image);
   print_metadata(&golden_image);
   int verify_result = verify_ota_slot(0);
   if (verify_result == -2){
      printf("OTA slot 0 non-correct CRC\n");
   }
   if (verify_result == -1){
      printf("OTA slot 0 non-read flash\n");
   }
   if (verify_result == 0){
      printf("OTA slot 0 correct CRC\n");
   }
}

/*---------------------------------------------------------------------------*/

void
verify_ext_firmware_e()
{
   printf("\n\n");

   printf("OTA slot 1 firmware:\n");
   OTAMetadata_t golden_image;
   get_ota_slot_metadata(1, &golden_image);
   print_metadata(&golden_image);
   int verify_result = verify_ota_slot(1);
   if (verify_result == -2){
      printf("OTA slot 1 non-correct CRC\n");
   }
   if (verify_result == -1){
      printf("OTA slot 1 non-read flash\n");
   }
   if (verify_result == 0){
      printf("OTA slot 1 correct CRC\n");
   }
}


/*---------------------------------------------------------------------------*/

void
uart_console(unsigned char uart_char)
{
   if (uart_char == 'd')
      flash_damp_hex(HEXVIEW_MODE);

   if (uart_char == 'r')
      flash_damp_hex(HEXRAW_MODE);

   if (uart_char == 'e')
   {
      erase_ota_image(1);
   }

   if (uart_char == 'o')
      {
         uint8_t aes_key[16] = {0x5a, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6c, 0x6c, 0x69, 0x61, 0x6e, 0x63, 0x65, 0x30, 0x39};
         uint8_t input_data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF};
         uint8_t nonce[16] = {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF};
         uint8_t encrypted_data[sizeof(input_data)] = {0};
         uint8_t decrypted_data[sizeof(input_data)] = {0};
         uint32_t start_uptime = rtc_s();
         uint32_t rounds_count = 500000;

         for (uint32_t i=0; i < rounds_count; i++ )
         {
            aes_cbc_encrypt((uint32_t*)aes_key, (uint32_t*)nonce, (uint32_t*)input_data, (uint32_t*)encrypted_data, sizeof(input_data));
            aes_cbc_decrypt((uint32_t*)aes_key, (uint32_t*)nonce, (uint32_t*)encrypted_data, (uint32_t*)decrypted_data, sizeof(input_data));
            if (!(i % 2000))
               watchdog_periodic();
            if (!(i % 20000))
               printf(".");
         }
         printf( "HW AES TEST(%" PRIu32 ") took %" PRIu32 " s, speed %" PRIu32 " r/s\n", rounds_count, rtc_s()-start_uptime, rounds_count/(rtc_s()-start_uptime) );
      }

   if (uart_char == 't')
   {
      uint8_t aes_key[16] = {0x5a, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6c, 0x6c, 0x69, 0x61, 0x6e, 0x63, 0x65, 0x30, 0x39};
      uint8_t input_data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xAD, 0xBE, 0xEF,
                              0xBE, 0xAD, 0xFA, 0xCE, 0xBE, 0xAD, 0xFA, 0xCE, 0xBE, 0xAD, 0xFA, 0xCE, 0xBE, 0xAD, 0xFA};
      uint8_t nonce[16] = {0};
      uint8_t encrypted_data[sizeof(input_data)] = {0};
      uint8_t decrypted_data[sizeof(input_data)] = {0};

      printf("\n");
      printf("\n");

      printf("Original data: \t\t");
      for (uint8_t i=0; i < sizeof(input_data); i++ )
         printf(" %"PRIXX8"", input_data[i]);
      printf("\n");
      printf("AES key: \t\t");
      for (uint8_t i=0; i < 16; i++ )
         printf(" %"PRIXX8"", aes_key[i]);
      printf("\n");

      printf("\n");
      printf("\n");

      aes_cbc_encrypt((uint32_t*)aes_key, (uint32_t*)nonce, (uint32_t*)input_data, (uint32_t*)encrypted_data, sizeof(input_data));

      printf("Crypted data: \t\t");
      for (uint8_t i=0; i < sizeof(input_data); i++ )
         printf(" %"PRIXX8"", encrypted_data[i]);
      printf("\n");

      printf("Nonce data: \t\t");
      for (uint8_t i=0; i < 16; i++ )
         printf(" %"PRIXX8"", nonce[i]);
      printf("\n");

      printf("\n");
      printf("\n");

      aes_cbc_decrypt((uint32_t*)aes_key, (uint32_t*)nonce, (uint32_t*)encrypted_data, (uint32_t*)decrypted_data, sizeof(input_data));

      printf("Decrypted data: \t");
      for (uint8_t i=0; i < sizeof(input_data); i++ )
         printf(" %"PRIXX8"", decrypted_data[i]);
      printf("\n");

      printf("\n");
      printf("\n");
   }
}


/*---------------------------------------------------------------------------*/

static void join_confirm_handler(const uip_ipaddr_t *sender_addr,
                                    const uint8_t *data,
                                    uint16_t datalen)
{
      uip_ipaddr_copy(&root_addr, sender_addr);
      process_post(&dag_node_process, PROCESS_EVENT_CONTINUE, NULL);
      etimer_set(&maintenance_timer, 0);
}

/*---------------------------------------------------------------------------*/

static void command_settings_handler(const uip_ipaddr_t *sender_addr,
                                    const uint8_t *data,
                                    uint16_t datalen)
{
      printf("DAG Node: Command/settings packet received\n");
      message_for_main_process.data_type = data[2];
      message_for_main_process.ability_target = data[3];
      message_for_main_process.ability_number = data[4];
      message_for_main_process.ability_state = data[5];
      process_post(&main_process, PROCESS_EVENT_CONTINUE, &message_for_main_process);
}


/*---------------------------------------------------------------------------*/

static void uart_packet_handler(const uip_ipaddr_t *sender_addr,
                                    const uint8_t *data,
                                    uint16_t datalen)
{
      //printf("DAG Node: Uart packet received\n");
      message_for_main_process.data_type = data[2];
      message_for_main_process.uart_returned_data_length = data[3];
      message_for_main_process.uart_data_length = data[4];
      if ( message_for_main_process.uart_returned_data_length > 16 )
            message_for_main_process.uart_returned_data_length = 16;

      message_for_main_process.payload[0] = data[5];
      message_for_main_process.payload[1] = data[6];
      message_for_main_process.payload[2] = data[7];
      message_for_main_process.payload[3] = data[8];
      message_for_main_process.payload[4] = data[9];
      message_for_main_process.payload[5] = data[10];
      message_for_main_process.payload[6] = data[11];
      message_for_main_process.payload[7] = data[12];
      message_for_main_process.payload[8] = data[13];
      message_for_main_process.payload[9] = data[14];
      message_for_main_process.payload[10] = data[15];
      message_for_main_process.payload[11] = data[16];
      message_for_main_process.payload[12] = data[17];
      message_for_main_process.payload[13] = data[18];
      message_for_main_process.payload[14] = data[19];
      message_for_main_process.payload[15] = data[20];
      process_post(&main_process, PROCESS_EVENT_CONTINUE, &message_for_main_process);
}


/*---------------------------------------------------------------------------*/

static void pong_handler(const uip_ipaddr_t *sender_addr,
                        const uint8_t *data,
                        uint16_t datalen)
{
      non_answered_packet = 0;
      //printf("DAG Node: Pong packet received, non-answered packet counter: %"PRId8" \n", non_answered_packet);
      net_off(RADIO_OFF_NOW);
}

/*---------------------------------------------------------------------------*/

static void firmware_data_handler(const uip_ipaddr_t *sender_addr,
                                    const uint8_t *data,
                                    uint16_t datalen)
{
      printf(" Firmware packet received(%"PRIu16" bytes)", datalen - FIRMWARE_PAYLOAD_OFFSET);

      uint8_t flash_write_buffer[FIRMWARE_PAYLOAD_LENGTH];

      for (uint16_t i = 0; i < FIRMWARE_PAYLOAD_LENGTH; i++)
      {
            flash_write_buffer[i] = data[i + FIRMWARE_PAYLOAD_OFFSET];
      }

      fw_error_counter = 0;
      uint32_t current_ota_ext_flash_address = (ota_images[1-1] << 12) + ota_image_current_offset;
      while(store_firmware_data(current_ota_ext_flash_address, flash_write_buffer, FIRMWARE_PAYLOAD_LENGTH));
      ota_image_current_offset = ota_image_current_offset + FIRMWARE_PAYLOAD_LENGTH;

      etimer_set( &fw_timer, 0 );
}

/*---------------------------------------------------------------------------*/

static void firmware_cmd_new_fw_handler(const uip_ipaddr_t *sender_addr,
                                          const uint8_t *data,
                                          uint16_t datalen)
{
      fw_chunk_quantity.u8[0] = data[5];
      fw_chunk_quantity.u8[1] = data[4];

      ota_image_current_offset = 0;

      printf("DAG Node: DATA_TYPE_FIRMWARE_COMMAND_NEW_FW command received, %"PRIu16"(0x%"PRIXX8" 0x%"PRIXX8") chunks\n", fw_chunk_quantity.u16, data[5], data[4]);

      if (spi_status == SPI_EXT_FLASH_ACTIVE)
      {
            printf("DAG Node: OTA update process start\n");
            process_start(&fw_update_process, NULL);
      }
      else
      {
            send_message_packet(DEVICE_MESSAGE_OTA_SPI_NOTACTIVE, DATA_NONE, DATA_NONE);
            printf("DAG Node: OTA update not processed, spi flash not-active\n");
      }
}

/*---------------------------------------------------------------------------*/

static void shedule_data_handler(const uint8_t *data, uint16_t datalen)
{

}

/*---------------------------------------------------------------------------*/

static void udp_receiver(struct simple_udp_connection *c,
                         const uip_ipaddr_t *sender_addr,
                         uint16_t sender_port,
                         const uip_ipaddr_t *receiver_addr,
                         uint16_t receiver_port,
                         const uint8_t *data, //TODO: make "parse" function(data[0] -> data.protocol_version)
                         uint16_t datalen)
{
      uint8_t protocol_version = data[0];
      uint8_t device_version = data[1];
      uint8_t packet_type = data[2];
      uint8_t packet_subtype = data[3];

      if (protocol_version == PROTOCOL_VERSION_V1 && device_version == CURRENT_DEVICE_VERSION)
      {

            if (packet_type == DATA_TYPE_JOIN_CONFIRM)
                  join_confirm_handler(sender_addr, data, datalen);

            else if (packet_type == DATA_TYPE_COMMAND || data[2] == DATA_TYPE_SETTINGS)
                  command_settings_handler(sender_addr, data, datalen);

            else if (packet_type == DATA_TYPE_UART)
                  uart_packet_handler(sender_addr, data, datalen);

            else if (packet_type == DATA_TYPE_PONG)
                  pong_handler(sender_addr, data, datalen);

            else if (packet_type == DATA_TYPE_FIRMWARE)
                  firmware_data_handler(sender_addr, data, datalen);

            else if (packet_type == DATA_TYPE_SET_TIME)
            {
               if (packet_subtype == DATA_TYPE_SET_TIME_RESPONSE)
                  time_data_handler(data, datalen);

               else if (packet_subtype == DATA_TYPE_SET_TIME_COMMAND_SYNC)
                  send_time_sync_req_packet(data, datalen);
            }

            else if (packet_type == DATA_TYPE_SET_SCHEDULE)
                  shedule_data_handler(data, datalen);

            else if (packet_type == DATA_TYPE_FIRMWARE_CMD)
            {
                  if (packet_subtype == DATA_TYPE_FIRMWARE_COMMAND_NEW_FW)
                        firmware_cmd_new_fw_handler(sender_addr, data, datalen);

                  else if (packet_subtype == DATA_TYPE_FIRMWARE_COMMAND_REBOOT)
                        watchdog_reboot();

                  else if (packet_subtype == DATA_TYPE_FIRMWARE_COMMAND_CLEAN_GI)
                        erase_ota_image(0);

                  else if (packet_subtype == DATA_TYPE_FIRMWARE_COMMAND_FLASH_GI)
                  {
                     write_fw_flag(FW_FLAG_NEW_IMG_INT);
                     watchdog_reboot();
                  }

                  else
                  {
                        printf("DAG Node: Incompatible FW CMD command from ");
                        uip_debug_ipaddr_print(sender_addr);
                        printf(", command: 0x%02x\n", data[3]);
                  }
            }

            else
            {
                  printf("DAG Node: Incompatible data type UDP packer from ");
                  uip_debug_ipaddr_print(sender_addr);
                  printf(", data type: 0x%02x\n", data[2]);
            }
      }
      else
      {
         printf("DAG Node: Incompatible data type UDP packer from ");
         uip_debug_ipaddr_print(sender_addr);
         printf(", data type: 0x%02x\n", data[2]);
      }

   led_mode_set(LED_FLASH);
}

/*---------------------------------------------------------------------------*/

void send_time_sync_req_packet()
{
   if (node_mode != MODE_NORMAL)
      return;

   uip_ipaddr_t addr;
   uip_ip6addr_copy(&addr, &root_addr);

   time_data_t local_time = get_epoch_time();

   uint8_t udp_buffer[PROTOCOL_VERSION_V2_16BYTE];
   udp_buffer[0] = PROTOCOL_VERSION_V1;
   udp_buffer[1] = DEVICE_VERSION_V1;
   udp_buffer[2] = DATA_TYPE_SET_TIME;
   udp_buffer[3] = DATA_TYPE_SET_TIME_REQUEST;
   udp_buffer[4] = DATA_NONE;
   udp_buffer[5] = DATA_NONE;
   udp_buffer[6] = DATA_NONE;
   udp_buffer[7] = DATA_NONE;
   udp_buffer[8] = DATA_NONE;
   udp_buffer[9] = DATA_NONE;
   udp_buffer[10] = local_time.seconds_u8[0];
   udp_buffer[11] = local_time.seconds_u8[1];
   udp_buffer[12] = local_time.seconds_u8[2];
   udp_buffer[13] = local_time.seconds_u8[3];
   udp_buffer[14] = local_time.milliseconds_u8[0];
   udp_buffer[15] = local_time.milliseconds_u8[1]; // << 16-byte packet, ready to encrypt v2 protocol

   net_on(RADIO_ON_TIMER_OFF);
   simple_udp_sendto(&udp_connection, udp_buffer, PROTOCOL_VERSION_V2_16BYTE, &addr);
}

/*---------------------------------------------------------------------------*/

void send_confirmation_packet(const uip_ipaddr_t *dest_addr)
{
   if (node_mode != MODE_NORMAL)
      return;

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

   net_on(RADIO_ON_TIMER_OFF);
   simple_udp_sendto(&udp_connection, udp_buffer, length, dest_addr);
}

/*---------------------------------------------------------------------------*/

void send_message_packet(uint8_t message_type, uint8_t data_1, uint8_t data_2)
{
   if (node_mode != MODE_NORMAL)
      return;

   uip_ipaddr_t addr;
   uip_ip6addr_copy(&addr, &root_addr);

   uint8_t length = 10;
   uint8_t udp_buffer[length];
   udp_buffer[0] = PROTOCOL_VERSION_V1;
   udp_buffer[1] = DEVICE_VERSION_V1;
   udp_buffer[2] = DATA_TYPE_MESSAGE;
   udp_buffer[3] = message_type;
   udp_buffer[4] = data_1;
   udp_buffer[5] = data_2;
   udp_buffer[6] = DATA_RESERVED;
   udp_buffer[7] = DATA_RESERVED;
   udp_buffer[8] = DATA_RESERVED;
   udp_buffer[9] = DATA_RESERVED;

   net_on(RADIO_ON_TIMER_OFF);
   simple_udp_sendto(&udp_connection, udp_buffer, length, &addr);
}

/*---------------------------------------------------------------------------*/

void send_sensor_event(struct sensor_packet *sensor_packet)
{
   if (node_mode != MODE_NORMAL)
      return;

   if (sensor_packet == NULL)
      return;

   uip_ipaddr_t addr;
   uip_ip6addr_copy(&addr, &root_addr);

   printf("DAG Node: Send sensor-event message to DAG-root node: ");
   uip_debug_ipaddr_print(&addr);
   printf("\n");

   uint8_t length = 10;
   uint8_t udp_buffer[length];
   udp_buffer[0] = sensor_packet->protocol_version;
   udp_buffer[1] = sensor_packet->device_version;
   udp_buffer[2] = sensor_packet->data_type;

   udp_buffer[3] = sensor_packet->number_ability;
   udp_buffer[4] = DATA_RESERVED;
   udp_buffer[5] = sensor_packet->sensor_number;
   udp_buffer[6] = sensor_packet->sensor_event;
   udp_buffer[7] = DATA_RESERVED;
   udp_buffer[8] = DATA_RESERVED;
   udp_buffer[9] = DATA_RESERVED;

   net_on(RADIO_ON_TIMER_OFF);
   simple_udp_sendto(&udp_connection, udp_buffer, length, &addr);
   led_mode_set(LED_FLASH);
}


/*---------------------------------------------------------------------------*/

void send_uart_data(struct command_data *uart_data)
{
   if (node_mode != MODE_NORMAL)
      return;

   if (uart_data == NULL)
      return;

   uip_ipaddr_t addr;
   uip_ip6addr_copy(&addr, &root_addr);

   //printf("DAG Node: Send uart data to DAG-root node: ");
   //uip_debug_ipaddr_print(&addr);
   //printf("\n");

   uint8_t length = 23;
   uint8_t udp_buffer[length];
   udp_buffer[0] = uart_data->protocol_version;
   udp_buffer[1] = uart_data->device_version;
   udp_buffer[2] = uart_data->data_type;

   udp_buffer[3] = uart_data->uart_returned_data_length;
   udp_buffer[4] = uart_data->uart_data_length;

   udp_buffer[5] = uart_data->payload[0];
   udp_buffer[6] = uart_data->payload[1];
   udp_buffer[7] = uart_data->payload[2];
   udp_buffer[8] = uart_data->payload[3];
   udp_buffer[9] = uart_data->payload[4];
   udp_buffer[10] = uart_data->payload[5];
   udp_buffer[11] = uart_data->payload[6];
   udp_buffer[12] = uart_data->payload[7];
   udp_buffer[13] = uart_data->payload[8];
   udp_buffer[14] = uart_data->payload[9];
   udp_buffer[15] = uart_data->payload[10];
   udp_buffer[16] = uart_data->payload[11];
   udp_buffer[17] = uart_data->payload[12];
   udp_buffer[18] = uart_data->payload[13];
   udp_buffer[19] = uart_data->payload[14];
   udp_buffer[20] = uart_data->payload[15];
   udp_buffer[21] = DATA_RESERVED;
   udp_buffer[22] = DATA_RESERVED;

   net_on(RADIO_ON_TIMER_OFF);
   simple_udp_sendto(&udp_connection, udp_buffer, length, &addr);
   led_mode_set(LED_FLASH);
}

/*---------------------------------------------------------------------------*/

void send_status_packet(const uip_ipaddr_t *parent_addr,
                        uint32_t uptime_raw,
                        int16_t rssi_parent_raw,
                        uint8_t temp,
                        uint8_t voltage)
{
   if (parent_addr == NULL)
      return;

   u8_u32_t uptime;
   u8_i16_t rssi_parent;

   uptime.u32 = uptime_raw;
   rssi_parent.i16 = rssi_parent_raw;
   uip_ipaddr_t addr;
   uip_ip6addr_copy(&addr, &root_addr);

   printf("DAG Node: Send status packet to DAG-root node: ");
   uip_debug_ipaddr_print(&addr);
   printf("\n");

   uint8_t length = 23;
   uint8_t udp_buffer[length];
   udp_buffer[0] = PROTOCOL_VERSION_V1;
   udp_buffer[1] = CURRENT_DEVICE_VERSION;
   udp_buffer[2] = DATA_TYPE_STATUS;
   udp_buffer[3] = parent_addr->u8[8];
   udp_buffer[4] = parent_addr->u8[9];
   udp_buffer[5] = parent_addr->u8[10];
   udp_buffer[6] = parent_addr->u8[11];
   udp_buffer[7] = parent_addr->u8[12];
   udp_buffer[8] = parent_addr->u8[13];
   udp_buffer[9] = parent_addr->u8[14];
   udp_buffer[10] = parent_addr->u8[15];
   udp_buffer[11] = uptime.u8[0];
   udp_buffer[12] = uptime.u8[1];
   udp_buffer[13] = uptime.u8[2];
   udp_buffer[14] = uptime.u8[3];
   udp_buffer[15] = rssi_parent.u8[0];
   udp_buffer[16] = rssi_parent.u8[1];
   udp_buffer[17] = temp;
   udp_buffer[18] = voltage;
   udp_buffer[19] = BIG_VERSION;
   udp_buffer[20] = LITTLE_VERSION;
   udp_buffer[21] = spi_status;
   udp_buffer[22] = DATA_RESERVED;

   net_on(RADIO_ON_TIMER_OFF);
   simple_udp_sendto(&udp_connection, udp_buffer, length, &addr);
   led_mode_set(LED_FLASH);
}


/*---------------------------------------------------------------------------*/

void send_join_packet(const uip_ipaddr_t *dest_addr)
{
   if (dest_addr == NULL)
      return;

   uip_ipaddr_t addr;
   uip_ip6addr_copy(&addr, dest_addr);

   printf("DAG Node: Send join packet to DAG-root node: ");
   uip_debug_ipaddr_print(&addr);
   printf("\n");

   uint8_t length = 10;
   uint8_t udp_buffer[length];
   udp_buffer[0] = PROTOCOL_VERSION_V1;
   udp_buffer[1] = CURRENT_DEVICE_VERSION;
   udp_buffer[2] = DATA_TYPE_JOIN;
   udp_buffer[3] = CURRENT_DEVICE_GROUP;
   udp_buffer[4] = CURRENT_DEVICE_SLEEP_TYPE;
   udp_buffer[5] = CURRENT_ABILITY_1BYTE;       //TODO: заменить на нормальную схему со сдвигами
   udp_buffer[6] = CURRENT_ABILITY_2BYTE;
   udp_buffer[7] = CURRENT_ABILITY_3BYTE;
   udp_buffer[8] = CURRENT_ABILITY_4BYTE;
   udp_buffer[9] = DATA_RESERVED;
   simple_udp_sendto(&udp_connection, udp_buffer, length, &addr);
}

/*---------------------------------------------------------------------------*/

void send_fw_chunk_req_packet(uint16_t chunk_num_raw)
{
   uip_ipaddr_t addr;
   uip_ip6addr_copy(&addr, &root_addr);

   u8_u16_t chunk_num;
   chunk_num.u16 = chunk_num_raw;

   uint8_t length = 10;
   uint8_t udp_buffer[length];
   udp_buffer[0] = PROTOCOL_VERSION_V1;
   udp_buffer[1] = CURRENT_DEVICE_VERSION;
   udp_buffer[2] = DATA_TYPE_FIRMWARE_CMD;
   udp_buffer[3] = DATA_TYPE_FIRMWARE_COMMAND_CHANK_REQ;
   udp_buffer[4] = chunk_num.u8[0];
   udp_buffer[5] = chunk_num.u8[1];
   udp_buffer[6] = DATA_RESERVED;
   udp_buffer[7] = DATA_RESERVED;
   udp_buffer[8] = DATA_RESERVED;
   udp_buffer[9] = DATA_RESERVED;
   simple_udp_sendto(&udp_connection, udp_buffer, length, &addr);
}

/*---------------------------------------------------------------------------*/

static void led_mode_set(uint8_t mode)
{
   led_mode = mode;
   if (led_mode == LED_OFF)
      led_off(LED_A);

   if (led_mode == LED_ON)
      led_on(LED_A);

   if (led_mode == LED_SLOW_BLINK || led_mode == LED_FAST_BLINK || led_mode == LED_FLASH)
      process_start(&led_process, NULL);
   else
      process_exit(&led_process);
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(led_process, ev, data)
{
   PROCESS_BEGIN();
   if (ev == PROCESS_EVENT_EXIT)
      return 1;
   static struct etimer led_mode_timer;

   while (led_mode == LED_SLOW_BLINK || led_mode == LED_FAST_BLINK || led_mode == LED_FLASH)
   {
      if (led_mode == LED_FAST_BLINK)
         etimer_set( &led_mode_timer, CLOCK_SECOND/10);

      if (led_mode == LED_SLOW_BLINK)
         etimer_set( &led_mode_timer, CLOCK_SECOND/2);

      if (led_mode == LED_FLASH)
         etimer_set( &led_mode_timer, 1);

      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&led_mode_timer) );

      led_on(LED_A);

      if (led_mode == LED_FAST_BLINK)
         etimer_set( &led_mode_timer, CLOCK_SECOND/32);

      if (led_mode == LED_SLOW_BLINK)
         etimer_set( &led_mode_timer, CLOCK_SECOND/32);

      if (led_mode == LED_FLASH)
         etimer_set( &led_mode_timer, CLOCK_SECOND/16);

      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&led_mode_timer) );

      led_off(LED_A);

      if (led_mode == LED_FLASH)
         led_mode = LED_OFF;
   }

   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(dag_node_button_process, ev, data)
{
   PROCESS_BEGIN();
   if (ev == PROCESS_EVENT_EXIT)
      return 1;

   PROCESS_PAUSE();

   while (1)
   {
      PROCESS_YIELD();

      if (ev == sensors_event)
      {
         if (data == &button_e_sensor_long_click)
         {
            led_mode_set(LED_ON);
            printf("SYSTEM: Button E long click, reboot\n");
            watchdog_reboot();
         }
      }
   }

   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(maintenance_process, ev, data)
{
   PROCESS_BEGIN();
   if (ev == PROCESS_EVENT_EXIT)
      return 1;

   PROCESS_PAUSE();

   while (1)
   {
      if (node_mode == MODE_NEED_REBOOT)
      {
            static struct etimer maintenance_reboot_timer;
            etimer_set( &maintenance_reboot_timer, (5 * CLOCK_SECOND));
            PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&maintenance_reboot_timer) );
            watchdog_reboot();
      }

      if (node_mode == MODE_NORMAL)
      {
         led_mode_set(LED_OFF);
         if (process_is_running(&status_send_process) == 0)
            process_start(&status_send_process, NULL);

         if (process_is_running(&root_find_process) == 1)
            process_exit(&root_find_process);

         if (non_answered_packet > MAX_NON_ANSWERED_PINGS)
         {
            printf("DAG Node: Root not available, reboot\n");
            watchdog_reboot();
         }
      }

      if (node_mode == MODE_NOTROOT)
      {
         if (CLASS == CLASS_B)
         {
            led_mode_set(LED_OFF);
            printf("DAG Node: Root not found, sleep\n");
            if (process_is_running(&dag_node_button_process) == 1)
               process_exit(&dag_node_button_process);

            if (process_is_running(&root_find_process) == 1)
               process_exit(&root_find_process);

            if (process_is_running(&status_send_process) == 1)
               process_exit(&status_send_process);

            if (process_is_running(&maintenance_process) == 1)
               process_exit(&maintenance_process);
            net_mode(RADIO_FREEDOM);
            net_off(RADIO_OFF_NOW);
            net_mode(RADIO_HOLD);
         }

         if (CLASS == CLASS_C)
         {
            led_mode_set(LED_FAST_BLINK);
            printf("DAG Node: Root not found, reboot\n"); //почему-то не перезагружается!
            watchdog_reboot();
         }
      }

      if (node_mode == MODE_JOIN_PROGRESS)
      {
         net_on(RADIO_ON_NORMAL);
         net_mode(RADIO_HOLD);
         led_mode_set(LED_SLOW_BLINK);

         if (process_is_running(&root_find_process) == 0)
            process_start(&root_find_process, NULL);

         if (process_is_running(&status_send_process) == 1)
            process_exit(&status_send_process);
      }

      etimer_set( &maintenance_timer, MAINTENANCE_INTERVAL);
      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&maintenance_timer) );
   }
   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(status_send_process, ev, data)
{
   PROCESS_BEGIN();
   if (ev == PROCESS_EVENT_EXIT)
      return 1;

   static struct etimer status_send_timer;
   const rpl_dag_t *dag = NULL;
   PROCESS_PAUSE();

   while (1)
   {
      dag = rpl_get_any_dag();

      if (dag != NULL && node_mode == MODE_NORMAL)
      {

         if (rpl_parent_is_reachable(dag->preferred_parent) == 0)
         {
            printf("DAG Node: Parent is not reachable\n");
            watchdog_reboot();
         }

         const uip_ipaddr_t *ipaddr_parent = rpl_get_parent_ipaddr(dag->preferred_parent);
         const struct link_stats *stat_parent = rpl_get_parent_link_stats(dag->preferred_parent);
         uint8_t temp = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
         uint8_t voltage = ( (batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT) * 125) >> 5 ) / VOLTAGE_PRESCALER;
         if (ipaddr_parent != NULL && stat_parent != NULL)
         {
            send_status_packet(ipaddr_parent, rtc_s(), stat_parent->rssi, temp, voltage);
         }
         non_answered_packet++;
         if (non_answered_packet != 1)
         {
            printf("DAG Node: Non-answered packet counter increase(status message): %"PRId8" \n", non_answered_packet);
         }
      }

      if (CLASS == CLASS_B)
      {
         printf("DAG Node: Next status message planned on long interval(%"PRId8" min)\n", LONG_STATUS_INTERVAL/CLOCK_SECOND/60);
         etimer_set( &status_send_timer, LONG_STATUS_INTERVAL + (random_rand() % LONG_STATUS_INTERVAL) );
      }
      if (CLASS == CLASS_C)
      {
         printf("DAG Node: Next status message planned on short interval(%"PRId8" min)\n", SHORT_STATUS_INTERVAL/CLOCK_SECOND/60);
         etimer_set( &status_send_timer, SHORT_STATUS_INTERVAL + (random_rand() % SHORT_STATUS_INTERVAL) );
      }

      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&status_send_timer) && node_mode == MODE_NORMAL );
   }

   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(fw_update_process, ev, data)
{
   PROCESS_BEGIN();

   if (ev == PROCESS_EVENT_EXIT)
      return 1;

   static uint16_t chunk_num = 0;
   static struct etimer fw_timer_deadline;
   static struct etimer fw_timer_delay_chunk;
   static struct etimer ota_image_erase_timer;
   static uint32_t page;

   /* Стираем память */

   printf("[OTA]: Erasing OTA slot 1 [%#x, %#x)...\n", (ota_images[0]<<12), ((ota_images[0]+25)<<12));
   for (page=0; page<25; page++)
   {
     printf("\r[OTA]: Erasing page %"PRIu32" at 0x%"PRIX32"..", page, (( ota_images[0] + page ) << 12));
     while( erase_extflash_page( (( ota_images[0] + page ) << 12) ) );

     send_message_packet(DEVICE_MESSAGE_OTA_SPI_ERASE_IN_PROGRESS, page, DATA_NONE);
     etimer_set( &ota_image_erase_timer, (CLOCK_SECOND/20) );
     PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&ota_image_erase_timer) );
   }
   printf("\r[OTA]: OTA slot 1 erased                        \n");


   /* Начинаем процесс обновления */

   while (1)
   {

      if (chunk_num < fw_chunk_quantity.u16) //Если остались незапрошенные пакеты
      {
         send_fw_chunk_req_packet(chunk_num);
         printf("\r[OTA]: Request %"PRId16"/%"PRId16" chunk... ", chunk_num + 1, fw_chunk_quantity.u16);
         chunk_num++;
      }
      else //Если все пакеты запрошены
      {
         printf("\n[OTA]: End chunks\n");
         chunk_num = 0;
         fw_error_counter = 0;
         int crc_status_ota_slot = verify_ota_slot(1);
         OTAMetadata_t current_firmware;
         OTAMetadata_t ota_slot_1_firmware;
         get_current_metadata( &current_firmware );
         get_ota_slot_metadata(1, &ota_slot_1_firmware);

         if (crc_status_ota_slot == CORRECT_CRC)
         {
            printf("[OTA]: New FW in OTA slot 1 correct CRC\n");
            if (current_firmware.uuid == ota_slot_1_firmware.uuid) //TODO: add univeral uuid(0xFFFFFFFF)
            {
               printf("[OTA]: New FW in OTA slot 1 correct UUID, set FW_FLAG_NEW_IMG_EXT, reboot\n");
               write_fw_flag(FW_FLAG_NEW_IMG_EXT);
               ti_lib_sys_ctrl_system_reset();
            }
            else
            {
               printf("[OTA]: New FW in OTA slot 1 non-correct firmware UUID\n");
               send_message_packet(DEVICE_MESSAGE_OTA_NONCORRECT_UUID, DATA_NONE, DATA_NONE);
            }

         }
         else
         {
            printf("[OTA]: New FW in OTA slot 1 non-correct CRC\n");
            send_message_packet(DEVICE_MESSAGE_OTA_NONCORRECT_CRC, DATA_NONE, DATA_NONE);
         }
         process_exit(&fw_update_process);
         return 0;
      }

      etimer_set( &fw_timer, FW_DELAY + 1); //Таймер, который сбрасывается при получении пакета
      etimer_set( &fw_timer_deadline, FW_DELAY); //Таймер максимального ожидания

      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&fw_timer) ); //Ждем сброса таймера после получения пакета или истечения времени ожидания пакета

      if (etimer_expired(&fw_timer_deadline) && (chunk_num < fw_chunk_quantity.u16)) //Если истек таймер максимального ожидания(fw_timer_deadline)
      {
         if (fw_error_counter > FW_MAX_ERROR_COUNTER)
         {
            printf("[OTA]: Not delivered chunk(>%"PRId8" errors), exit\n", FW_MAX_ERROR_COUNTER);
            send_message_packet(DEVICE_MESSAGE_OTA_NOT_DELIVERED_CHUNK, DATA_NONE, DATA_NONE);
            process_exit(&fw_update_process);
            chunk_num = 0;
            fw_error_counter = 0;
            return 0;
         }
         else
         {
            fw_error_counter++;
            chunk_num--;
            printf("[OTA]: Request %"PRId16"/%"PRId16" chunk again(%"PRId8" errors)\n", chunk_num + 1, fw_chunk_quantity.u16, fw_error_counter);
         }
      }
      etimer_set( &fw_timer_delay_chunk, (CLOCK_SECOND/20) ); //Таймер задержки перед запросом следующего чанка
      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&fw_timer_delay_chunk) );
   }

   PROCESS_END();
}


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(root_find_process, ev, data)
{
   PROCESS_BEGIN();

   if (ev == PROCESS_EVENT_EXIT)
      return 1;

   static struct etimer find_root_timer;
   static struct etimer find_root_limit_timer;
   static rpl_dag_t *root_find_dag = NULL;
   static uip_ds6_addr_t *ds6_addr = NULL;
   PROCESS_PAUSE();

   etimer_set( &find_root_limit_timer, ROOT_FIND_LIMIT_TIME);

   while (1)
   {
      if (node_mode == MODE_JOIN_PROGRESS)
      {
         if (!etimer_expired(&find_root_limit_timer))
         {
            ds6_addr = uip_ds6_get_global(ADDR_PREFERRED);
            if (ds6_addr != NULL)
            {
               root_find_dag = rpl_get_dag(&ds6_addr->ipaddr);
               if (root_find_dag != NULL)
               {
                  if ( led_mode != LED_FAST_BLINK)
                     led_mode_set(LED_FAST_BLINK);

                  send_join_packet(&root_find_dag->dag_id);
               }
            }
         }
         else
         {
            node_mode = MODE_NOTROOT;
            printf("DAG Node: mode set to MODE_NOTROOT\n");
            process_exit(&maintenance_process);
            process_start(&maintenance_process, NULL);
         }

      }
      etimer_set( &find_root_timer, ROOT_FIND_INTERVAL + (random_rand() % ROOT_FIND_INTERVAL) );
      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&find_root_timer) );
   }

   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(dag_node_process, ev, data)
{
   PROCESS_BEGIN();

   PROCESS_PAUSE();

   simple_udp_register(&udp_connection, UDP_DATA_PORT, NULL, UDP_DATA_PORT, udp_receiver);

   if (CLASS == CLASS_B)
      rpl_set_mode(RPL_MODE_LEAF);
   else
      rpl_set_mode(RPL_MODE_MESH);

   node_mode = MODE_JOIN_PROGRESS;

   spi_status = spi_test();

   printf("Node started, %s mode, %s class, SPI %s, version %"PRIu8".%"PRIu8"\n",
                rpl_get_mode() == RPL_MODE_LEAF ? "leaf" : "no-leaf",
                CLASS == CLASS_B ? "B(sleep)" : "C(non-sleep)",
                spi_status == SPI_EXT_FLASH_ACTIVE ? "active" : "non-active",
                BIG_VERSION, LITTLE_VERSION);

   process_start(&dag_node_button_process, NULL);
   process_start(&maintenance_process, NULL);

   serial_shell_init();
   shell_reboot_init();
   shell_time_init();
   unwired_shell_init();
   printf("DAG Node: Shell activated, type \"help\" for command list\n");

   SENSORS_ACTIVATE(batmon_sensor);

   PROCESS_YIELD_UNTIL(ev == PROCESS_EVENT_CONTINUE);

   printf("DAG Node: DAG active, join packet confirmation received, mode set to MODE_NORMAL\n");
   led_mode_set(LED_SLOW_BLINK);
   node_mode = MODE_NORMAL;
   net_mode(RADIO_FREEDOM);
   net_off(RADIO_OFF_NOW);
   process_start(&time_sync_process, NULL);

   if (spi_status == SPI_EXT_FLASH_ACTIVE)
   {
      if (verify_ota_slot(0) == VERIFY_SLOT_CRC_ERROR)
      {
         printf("[OTA]: bad golden image, write current FW\n");
         send_message_packet(DEVICE_MESSAGE_OTA_BAD_GOLDEN_IMAGE, DATA_NONE, DATA_NONE);
         backup_golden_image();
         watchdog_reboot();
      }
   }

   uint8_t current_ota_flag_status = read_fw_flag();
   if (current_ota_flag_status == FW_FLAG_NEW_IMG_INT)
   {
      write_fw_flag(FW_FLAG_PING_OK);
      printf("DAG Node: OTA flag changed to FW_FLAG_PING_OK\n");
      send_message_packet(DEVICE_MESSAGE_OTA_UPDATE_SUCCESS, DATA_NONE, DATA_NONE);
      node_mode = MODE_NEED_REBOOT;
      printf("DAG Node: mode set to MODE_NEED_REBOOT(reboot after ota-update)\n");
      process_exit(&maintenance_process);
      process_start(&maintenance_process, NULL);
   }

   PROCESS_END();
}
