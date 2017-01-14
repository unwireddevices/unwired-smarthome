/*
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
 *         RPL-node service for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 *         
 */
 /*---------------------------------------------------------------------------*/

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"

#include "dev/leds.h"
#include "sys/clock.h"
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"
#include "cc26xx/board.h"
#include "net/ip/uip-debug.h"

#include <string.h>
#include <stdio.h>
#include "simple-udp.h"

#include "ud-dag_node.h"
#include "net/link-stats.h"

#include "xxf_types_helper.h"

#include "ti-lib.h"
#include "ud_binary_protocol.h"
#include "dev/watchdog.h"

#ifdef IF_UD_BUTTON
    #include "ud-button.h"
#endif

#ifdef IF_UD_RELAY
    #include "ud-relay.h"
#endif

#include "fake_headers.h" //no move up! not "krasivo"!

#define DEBUG_INTERVAL                    (60 * CLOCK_SECOND)
#define SHORT_PING_INTERVAL                (5 * CLOCK_SECOND)
#define LONG_PING_INTERVAL                (50 * CLOCK_SECOND)
#define STATUS_SEND_INTERVAL           (10*60 * CLOCK_SECOND)

#define MAX_NON_ANSWERED_PINGS              5

/*---------------------------------------------------------------------------*/

struct simple_udp_connection udp_connection; //struct for simple_udp_send
uint8_t dag_active = 0; //set to 1, if rpl root found and answer to join packet
uint8_t non_answered_ping = 0;
uip_ip6addr_t root_addr;
clock_time_t debug_interval = DEBUG_INTERVAL;
clock_time_t ping_interval = SHORT_PING_INTERVAL;
clock_time_t status_send_interval = STATUS_SEND_INTERVAL;

/*---------------------------------------------------------------------------*/

SENSORS(&button_a_sensor_click, &button_a_sensor_long_click,
        &button_b_sensor_click, &button_b_sensor_long_click,
        &button_c_sensor_click, &button_c_sensor_long_click,
        &button_d_sensor_click, &button_d_sensor_long_click,
        &button_e_sensor_click, &button_e_sensor_long_click); //register button sensors

PROCESS(dag_node_process, "DAG-node process");
PROCESS(dag_node_button_process, "DAG-node button process");
PROCESS(root_ping_process, "Root ping process");
PROCESS(status_send_process, "Status send process");

/*---------------------------------------------------------------------------*/

static void
udp_receiver(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{
    led_on(LED_A);
    printf("DEBUG: UDP packer: %02x,%02x,%02x from ", data[0],data[1],data[2]);
    uip_debug_ipaddr_print(sender_addr);
    printf("\n");

    if (data[0] == PROTOCOL_VERSION_V1 && data[1] == CURRENT_DEVICE_VERSION) {
      switch ( data[2] ) {
      case DATA_TYPE_CONFIRM:
          printf("DAG Node: DAG join packet confirmation received, DAG active\n");
          led_off(LED_A);
          dag_active = 1;
          root_addr = *sender_addr;
          non_answered_ping = 0;
          break;
      case DATA_TYPE_COMMAND:
          printf("DAG Node: Command packet received\n");
          static struct command_data message_for_main_process;
          message_for_main_process.ability_target = data[3];
          message_for_main_process.ability_number = data[4];
          message_for_main_process.ability_state = data[5];
          process_post(&main_process, PROCESS_EVENT_CONTINUE, &message_for_main_process);
          break;
      default:
          printf("DAG NODE: Incompatible data type(%02x)!\n", data[2]);
          break;
      }
  }
  else  {
      printf("DAG NODE: Incompatible device or protocol version!\n");
  }

  led_off(LED_A);
}
/*---------------------------------------------------------------------------*/


void
print_debug_data(void)
{
    printf("\n");
    uint32_t secs_now = clock_seconds();
    printf("SYSTEM: uptime: %" PRIu32 " s\n", secs_now);
 /*
    rpl_dag_t *dag = rpl_get_any_dag();

    if (dag) {
        uip_ipaddr_t *ipaddr_parent = rpl_get_parent_ipaddr(dag->preferred_parent);
        printf("RPL: parent ip address: ");
        uip_debug_ipaddr_print(ipaddr_parent);
        printf("\n");

        uip_ipaddr_t dag_id_addr = dag->dag_id;
        printf("RPL: dag root ip address: ");
        uip_debug_ipaddr_print(&dag_id_addr);
        printf("\n");

        const struct link_stats *stat_parent = rpl_get_parent_link_stats(dag->preferred_parent);
        printf("RPL: parent last tx: %u sec ago\n", (unsigned)((clock_time() - stat_parent->last_tx_time) / (CLOCK_SECOND)));

        printf("RPL: parent rssi: %" PRId16 "\n", stat_parent->rssi);

        int parent_is_reachable = rpl_parent_is_reachable(dag->preferred_parent);
        printf("RPL: parent is reachable: %" PRId16 "\n", parent_is_reachable);

        uint8_t temp = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
        printf("SYSTEM: temp: %"PRIu8"C, voltage: %"PRId16"mv\n", temp, ((batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT) * 125) >> 5));
    }
*/
}

/*---------------------------------------------------------------------------*/

void
send_status_packet(const uip_ip6addr_t *dest_addr,
                   struct simple_udp_connection *connection,
                   const uip_ipaddr_t *parent_addr,
                   uint32_t uptime,
                   int16_t rssi_parent,
                   uint8_t temp,
                   uint8_t voltage)
{
    uint8_t *uptime_uint8_t = (uint8_t *)&uptime;
    uint8_t *rssi_parent_uint8_t = (int8_t *)&rssi_parent;

    uint8_t length = 23;
    uint8_t buf[length];
    buf[0] = PROTOCOL_VERSION_V1;
    buf[1] = CURRENT_DEVICE_VERSION;
    buf[2] = DATA_TYPE_STATUS;
    buf[3] = ((uint8_t *)parent_addr)[8];
    buf[4] = ((uint8_t *)parent_addr)[9];
    buf[5] = ((uint8_t *)parent_addr)[10];
    buf[6] = ((uint8_t *)parent_addr)[11];
    buf[7] = ((uint8_t *)parent_addr)[12];
    buf[8] = ((uint8_t *)parent_addr)[13];
    buf[9] = ((uint8_t *)parent_addr)[14];
    buf[10] = ((uint8_t *)parent_addr)[15];
    buf[11] = *uptime_uint8_t++;
    buf[12] = *uptime_uint8_t++;
    buf[13] = *uptime_uint8_t++;
    buf[14] = *uptime_uint8_t++;
    buf[15] = *rssi_parent_uint8_t++;
    buf[16] = *rssi_parent_uint8_t++;
    buf[17] = temp;
    buf[18] = voltage;
    buf[19] = DATA_RESERVED;
    buf[20] = DATA_RESERVED;
    buf[21] = DATA_RESERVED;
    buf[22] = DATA_RESERVED;

    simple_udp_sendto(connection, buf, length + 1, dest_addr);
}

/*---------------------------------------------------------------------------*/

void
send_join_packet(const uip_ip6addr_t *dest_addr, struct simple_udp_connection *connection)
{
    uint8_t length = 10;
    uint8_t buf[length];
    buf[0] = PROTOCOL_VERSION_V1;
    buf[1] = CURRENT_DEVICE_VERSION;
    buf[2] = DATA_TYPE_JOIN;
    buf[3] = CURRENT_DEVICE_GROUP;
    buf[4] = CURRENT_DEVICE_SLEEP_TYPE;
    buf[5] = CURRENT_ABILITY_1BYTE; //TODO: заменить на нормальную схему со сдвигами
    buf[6] = CURRENT_ABILITY_2BYTE;
    buf[7] = CURRENT_ABILITY_3BYTE;
    buf[8] = CURRENT_ABILITY_4BYTE;
    buf[9] = DATA_RESERVED;
    simple_udp_sendto(connection, buf, length + 1, dest_addr);
}

/*---------------------------------------------------------------------------*/

static void
dag_root_find(void)
{
    rpl_dag_t *dag = NULL;
    uip_ip6addr_t addr;

    uip_ds6_addr_t *addr_desc = uip_ds6_get_global(ADDR_PREFERRED);
    if (addr_desc != NULL) {
        dag = rpl_get_any_dag();
        if (dag) {
            led_blink(LED_A);
            if (&dag->dag_id) {
                if (dag_active == 0) {
                    uip_ip6addr_copy(&addr, &dag->dag_id);

                    printf("DAG node: send join packet to rpl root");
                    uip_debug_ip6addr_print(&addr);
                    printf("\n");
                    send_join_packet(&addr, &udp_connection);
                    if (non_answered_ping < 100) {
                        non_answered_ping++;
                    }
                }
            }
            else
            {
                //printf("RPL: address destination: none \n");
                dag_active = 0;
            }
        }
    }

    if (non_answered_ping > MAX_NON_ANSWERED_PINGS) {
        dag_active = 0;
    }
}


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(dag_node_button_process, ev, data)
{
  PROCESS_BEGIN();
  PROCESS_PAUSE();

  while(1) {
    PROCESS_YIELD();
    if(ev == sensors_event) {
        if(data == &button_e_sensor_click) {
            printf("DAG Node: Local repair activated\n");
            rpl_dag_t *dag = rpl_get_any_dag();
            rpl_local_repair(dag->instance);
        }
        if(data == &button_e_sensor_long_click) {
            led_on(LED_A);
            printf("SYSTEM: Button E long click, reboot\n");
            watchdog_reboot();
        }
    }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(status_send_process, ev, data)
{
  PROCESS_BEGIN();
  static struct etimer status_send_timer;
  PROCESS_PAUSE();

  while(1) {
      etimer_set(&status_send_timer, status_send_interval + (random_rand() % status_send_interval));
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&status_send_timer));

      const rpl_dag_t *dag = rpl_get_any_dag();

      if (dag) {
          const uip_ipaddr_t *ipaddr_parent = rpl_get_parent_ipaddr(dag->preferred_parent);
          const struct link_stats *stat_parent = rpl_get_parent_link_stats(dag->preferred_parent);
          uint8_t temp = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
          uint8_t voltage = ((batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT) * 125) >> 5)/VOLTAGE_PRESCALER;
          send_status_packet(&root_addr, &udp_connection, ipaddr_parent, clock_seconds(), stat_parent->rssi, temp, voltage);
      }
  }
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(root_ping_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer ping_timer;
  PROCESS_PAUSE();

  while(1) {

      etimer_set(&ping_timer, ping_interval + (random_rand() % ping_interval));
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&ping_timer));

      dag_root_find();

      if (dag_active == 0 && ping_interval != SHORT_PING_INTERVAL && non_answered_ping < 20) {
          ping_interval = SHORT_PING_INTERVAL;
          uip_ds_6_interval_set(CLOCK_SECOND/5);
          printf("DAG Node: Change timer to SHORT interval, DS6 interval: %" PRIu32 " ticks\n", uip_ds_6_interval_get());
      }

      if ((dag_active == 1 && ping_interval != LONG_PING_INTERVAL) || non_answered_ping > 20) {
          ping_interval = LONG_PING_INTERVAL;
          uip_ds_6_interval_set(CLOCK_SECOND);
          printf("DAG Node: Change timer to LONG interval, DS6 interval: %" PRIu32 " ticks\n", uip_ds_6_interval_get());
      }

      if (non_answered_ping > 30) {
          printf("DAG Node: Not answer root, reboot\n");
          watchdog_reboot();
      }

      if (non_answered_ping > 1)
          printf("DAG Node: Non-answer ping count: %u\n", non_answered_ping);
  }

  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(dag_node_process, ev, data)
{
  PROCESS_BEGIN();

  static struct etimer debug_timer;
  simple_udp_register(&udp_connection, UDP_DATA_PORT, NULL, UDP_DATA_PORT, udp_receiver);

  PROCESS_PAUSE();

  if (RPL_CONF_LEAF_ONLY == 1)
      rpl_set_mode(RPL_MODE_LEAF);
  else
      rpl_set_mode(RPL_MODE_MESH);

  printf("DAG Node: started, %s mode\n", rpl_get_mode() ==  RPL_MODE_LEAF ? "leaf" : "no-leaf");

  process_start(&dag_node_button_process, NULL);
  process_start(&root_ping_process, NULL);
  process_start(&status_send_process, NULL);

  SENSORS_ACTIVATE(batmon_sensor);

  led_on(LED_A);


  while(1) {
    etimer_set(&debug_timer, debug_interval + (random_rand() % debug_interval));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&debug_timer));
    print_debug_data();
  }


  PROCESS_END();
}
