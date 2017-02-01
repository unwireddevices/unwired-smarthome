/*
 * Copyright (c) 2011, Swedish Institute of Computer Science.
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


#include "../fake_headers.h" //no move up! not "krasivo"!

#define UART_DATA_POLL_INTERVAL 5 //in main timer ticks, one tick ~8ms
/*---------------------------------------------------------------------------*/

struct command_data
{
    volatile uip_ip6addr_t destination_address;
    volatile uint8_t ability_target;
    volatile uint8_t ability_number;
    volatile uint8_t ability_state;
    volatile uint8_t ready_to_send;
};

/* Received data via UART */
static struct command_data command_message;

/* UART-buffer for raw command */
volatile static uint8_t uart_command_buf[UART_DATA_LENGTH];

/* UART char iterator */
volatile static uint8_t uart_iterator = 0;

/* The sequence of start and end command */
static uint8_t uart_magic_sequence[UART_DATA_LENGTH] =
{0x01,0x16,0x16,0x16,0x16,0x10,
 0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0x01,0x01,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
 0x03,0x16,0x16,0x16,0x17,0x04};

/* UPD connection structure */
static struct simple_udp_connection udp_connection;

/*---------------------------------------------------------------------------*/
/* Buttons on DIO 1 */
SENSORS(&button_e_sensor_click, &button_e_sensor_long_click);

PROCESS(rpl_root_process,"Unwired RPL root and udp data receiver");
PROCESS(send_command_process,"UDP command sender");

AUTOSTART_PROCESSES(&rpl_root_process);

/*---------------------------------------------------------------------------*/
void send_confirmation_packet(const uip_ip6addr_t *dest_addr)
{
    if (dest_addr == NULL) {
        printf("ERROR: dest_addr in send_confirmation_packet null\n");
        return;
    }

    int length = 10;
    char buf[length];
    buf[0] = PROTOCOL_VERSION_V1;
    buf[1] = DEVICE_VERSION_V1;
    buf[2] = DATA_TYPE_CONFIRM;
    buf[3] = DATA_RESERVED;
    buf[4] = DATA_RESERVED;
    buf[5] = DATA_RESERVED;
    buf[6] = DATA_RESERVED;
    buf[7] = DATA_RESERVED;
    buf[8] = DATA_RESERVED;
    buf[9] = DATA_RESERVED;
    simple_udp_sendto(&udp_connection, buf, length + 1, dest_addr);
}

/*---------------------------------------------------------------------------*/

void send_command_packet(struct command_data *command_message)
{
    if (&command_message->destination_address == NULL) {
        printf("ERROR: dest_addr in send_command_packet null\n");
        return;
    }
    if (&udp_connection.udp_conn == NULL) { //указатель на что?
        printf("ERROR: connection in send_command_packet null\n");
        return;
    }

    uip_ip6addr_t addr;
    uip_ip6addr_copy(&addr, &command_message->destination_address);

    int length = 10;
    char udp_buffer[length];
    udp_buffer[0] = PROTOCOL_VERSION_V1;
    udp_buffer[1] = DEVICE_VERSION_V1;
    udp_buffer[2] = DATA_TYPE_COMMAND;
    udp_buffer[3] = command_message->ability_target;
    udp_buffer[4] = command_message->ability_number;
    udp_buffer[5] = command_message->ability_state;
    udp_buffer[6] = DATA_RESERVED;
    udp_buffer[7] = DATA_RESERVED;
    udp_buffer[8] = DATA_RESERVED;
    udp_buffer[9] = DATA_RESERVED;
    simple_udp_sendto(&udp_connection, udp_buffer, length + 1, &addr);

}

/*---------------------------------------------------------------------------*/

void dag_root_raw_print(const uip_ip6addr_t *addr, const uint8_t *data, const uint16_t length)
{
    if (addr == NULL) {
        printf("ERROR: addr in dag_root_raw_print null\n");
        return;
    }
    if (data == NULL) {
        printf("ERROR: data in dag_root_raw_print null\n");
        return;
    }

    if (length != 11 && length != 24) {
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

    if (length == 11) {
        printf("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02XFFFFFFFFFFFFFFFFFFFFFFFFFF",
               data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9]);
    }

    if (length == 24) {
        printf("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
               data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],data[8],data[9],
               data[10],data[11],data[12],data[13],data[14],data[15],data[16],data[17],data[18],data[19],
               data[20],data[21],data[22]);
    }

    printf("RAWEND   \n");
}

/*---------------------------------------------------------------------------*/
/*
void uart_packet_dump(uint8_t *uart_command_buf) {
    if (uart_command_buf == NULL) {
        printf("ERROR: uart_command_buf in uart_packet_dump null\n");
        return;
    }

    printf("\nUART->6LP: ");
    for (int i = 0; i < UART_DATA_LENGTH; i++)
    {
        printf("%02X", uart_command_buf[i]);
    }
    printf("\n");
}
*/
/*---------------------------------------------------------------------------*/

static int uart_data_receiver(unsigned char c)
{
    led_blink(LED_A);
    if ((uart_iterator <= MAGIC_SEQUENCE_LENGTH - 1) ||
            ((uart_iterator >= UART_DATA_LENGTH - MAGIC_SEQUENCE_LENGTH) && (uart_iterator <= UART_DATA_LENGTH - 1)))
    {
        if (c != uart_magic_sequence[uart_iterator])
        {
            uart_iterator = 0;
            return 1;
        }
    }
    uart_command_buf[uart_iterator] = c;

    if (uart_iterator < UART_DATA_LENGTH-1)
    {
        uart_iterator++;
    }
    else
    {
        uart_iterator = 0;

        if (uart_command_buf[6] == UART_PROTOCOL_VERSION_V1)
        {
                for (int i = 0; i <= 15; i++)
                {
                   command_message.destination_address.u8[i] = uart_command_buf[i+7];
                }
                command_message.ability_number = uart_command_buf[24];
                command_message.ability_state = uart_command_buf[25];
                command_message.ability_target = uart_command_buf[23];
                command_message.ready_to_send = 1;
        }
        else
        {
            printf("USER: Incompatible protocol version!\n");
        }
        //uart_packet_dump(uart_command_buf);
    }

    return 1;
}

/*---------------------------------------------------------------------------*/

static void udp_data_receiver(struct simple_udp_connection *connection,
         const uip_ipaddr_t *sender_addr, //TODO: fd00 or fe80?
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
    led_on(LED_A);

    dag_root_raw_print(sender_addr, data, datalen);

    if (data[0] == PROTOCOL_VERSION_V1 && data[2] == DATA_TYPE_JOIN )
    {
        send_confirmation_packet(sender_addr);
    }

    led_off(LED_A);
}

/*---------------------------------------------------------------------------*/

static uip_ipaddr_t *set_global_address(void)
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

static void create_rpl_dag(uip_ipaddr_t *ipaddr)
{
    if (ipaddr == NULL) {
        printf("ERROR: ipaddr in create_rpl_dag null\n");
        return;
    }
  struct uip_ds6_addr *root_if = uip_ds6_addr_lookup(ipaddr);
  if(root_if != NULL)
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

PROCESS_THREAD(send_command_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer send_command_process_timer;
  PROCESS_PAUSE();

  while(1) {
      etimer_set(&send_command_process_timer, UART_DATA_POLL_INTERVAL);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&send_command_process_timer));
      if (command_message.ready_to_send == 1)
      {
            disable_interrupts();
            send_command_packet(&command_message);
            command_message.ready_to_send = 0;
            enable_interrupts();
      }
  }

  PROCESS_END();
}


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(rpl_root_process, ev, data)
{
  static uip_ipaddr_t *ipaddr = NULL;

  PROCESS_BEGIN();

  printf("Unwired RLP root and UDP data receiver. HELL-IN-CODE free. I hope. \n");

  /* if you do not execute "cleanall" target, rpl-root can build in "leaf" configuration. Diagnostic message */
  if (RPL_CONF_LEAF_ONLY == 1)
  {
      printf("\nWARNING: leaf mode on rpl-root!\n");
  }

  /* Set MESH-mode for dc-power rpl-root(not leaf-mode) */
  rpl_set_mode(RPL_MODE_MESH);

  /* set local address */
  ipaddr = set_global_address();

  /* make local address as rpl-root */
  create_rpl_dag(ipaddr);

  /* register udp-connection, set incoming upd-data handler(udp_data_receiver) */
  simple_udp_register(&udp_connection, UDP_DATA_PORT, NULL, UDP_DATA_PORT, udp_data_receiver);

  /* set incoming uart-data handler(uart_data_receiver) */
  cc26xx_uart_set_input(&uart_data_receiver);

  /* blink-blink LED */
  led_blink(LED_A);
  led_blink(LED_A);

  /* start flag "data for udp ready" poller process */
  process_start(&send_command_process, NULL);

  while(1) {
    PROCESS_WAIT_EVENT();
    if(ev == sensors_event) {
        if(data == &button_e_sensor_click) {
            printf("Initiating global repair\n");
            rpl_repair_root(RPL_DEFAULT_INSTANCE);
        }
        if (data == &button_e_sensor_long_click)
        {
            led_on(LED_A);
            printf("SYSTEM: Button E long click, reboot\n");
            watchdog_reboot();
        }
    }
  }
  PROCESS_END();
}
