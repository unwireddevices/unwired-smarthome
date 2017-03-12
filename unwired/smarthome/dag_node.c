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
 *         DAG-node service for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 *
 */
/*---------------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"

#include "clock.h"

#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ip/uip-debug.h"
#include "net/link-stats.h"

#include "dev/leds.h"
#include "sys/clock.h"
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "dev/watchdog.h"
#include "dev/cc26xx-uart.h"
#include "board-peripherals.h"
#include "board.h"
#include "ti-lib.h"

#include "../flash-common.h"
#include "xxf_types_helper.h"
#include "../ud_binary_protocol.h"
#include "dag_node.h"

#ifdef IF_UD_BUTTON
#  include "button.h"
#endif

#ifdef IF_UD_RELAY
#  include "relay.h"
#endif

#ifdef IF_UD_DIMMER
#  include "dimmer.h"
#endif

#include "../fake_headers.h" //no move up! not "krasivo"!

#define SHORT_STATUS_INTERVAL           (10 * 60 * CLOCK_SECOND)
#define LONG_STATUS_INTERVAL            (60 * 60 * CLOCK_SECOND)//(60 * 60 * CLOCK_SECOND)
#define ROOT_FIND_INTERVAL                    (2 * CLOCK_SECOND)
#define ROOT_FIND_LIMIT_TIME                 (60 * CLOCK_SECOND)
#define RADIO_OFF_DELAY                     (0.5 * CLOCK_SECOND)

#define MODE_NORMAL                             0x01
#define MODE_NOTROOT                            0x02
#define MODE_JOIN_PROGRESS                      0x03

#define FALSE                                   0x00
#define TRUE                                    0x01

#define TIMER                                   0x01
#define NOW                                     0x02

#define MAX_NON_ANSWERED_PINGS                  3

/*---------------------------------------------------------------------------*/

/* struct for simple_udp_send */
struct simple_udp_connection udp_connection;

volatile uint8_t node_mode;

volatile uint8_t node_rpl_maintenance = FALSE;

volatile uint8_t non_answered_packet = 0;
volatile uip_ip6addr_t root_addr;
static struct command_data message_for_main_process;

static struct etimer maintenance_timer;
static struct ctimer net_off_delay_timer;

static void net_off(uint8_t mode);
/*---------------------------------------------------------------------------*/

PROCESS(dag_node_process, "DAG-node process");
PROCESS(dag_node_button_process, "DAG-node button process");
PROCESS(root_find_process, "Root find process");
PROCESS(status_send_process, "Status send process");
PROCESS(net_off_process, "Radio off delay process");
PROCESS(maintenance_process, "Maintenance process");
PROCESS(rpl_maintenance_process, "RPL maintenance process");

/*---------------------------------------------------------------------------*/

static void net_on()
{
   if (CLASS == CLASS_B)
   {
      NETSTACK_MAC.on();
      uip_ds_6_interval_set(CLOCK_SECOND/2);
      printf("DAG Node: Radio ON\n");
   }
}

/*---------------------------------------------------------------------------*/

static void timer_net_off_event(void *ptr){
   net_off(TIMER);
}

/*---------------------------------------------------------------------------*/

static void net_off(uint8_t mode)
{
   if (node_rpl_maintenance == TRUE)
   {
      ctimer_set(&net_off_delay_timer, CLOCK_SECOND * 2, timer_net_off_event, NULL);
      return;
   }

   if (CLASS == CLASS_B)
   {
      uip_ds_6_interval_set(CLOCK_SECOND * 20);

      if (mode == TIMER)
      {
         if (process_is_running(&net_off_process) == 1)
            process_exit(&net_off_process);

         process_start(&net_off_process, NULL);
      }

      if (mode == NOW)
      {
         if (process_is_running(&net_off_process) == 1)
            process_exit(&net_off_process);

         printf("DAG Node: Radio OFF immediately\n");
         NETSTACK_MAC.off(0);
      }
   }

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
   led_on(LED_A);

   if (data[0] == PROTOCOL_VERSION_V1 && data[1] == CURRENT_DEVICE_VERSION)
   {
      if (data[2] == DATA_TYPE_JOIN_CONFIRM)
      {
         printf("DAG Node: DAG active, join packet confirmation received, mode set to MODE_NORMAL\n");
         led_off(LED_A);
         uip_ipaddr_copy(&root_addr, sender_addr);
         node_mode = MODE_NORMAL;
         etimer_set(&maintenance_timer, 0);
         net_off(NOW);
      }

      if (data[2] == DATA_TYPE_COMMAND || data[2] == DATA_TYPE_SETTINGS)
      {
         printf("DAG Node: Command/settings packet received\n");
         message_for_main_process.data_type = data[2];
         message_for_main_process.ability_target = data[3];
         message_for_main_process.ability_number = data[4];
         message_for_main_process.ability_state = data[5];
         process_post(&main_process, PROCESS_EVENT_CONTINUE, &message_for_main_process);
      }

      if (data[2] == DATA_TYPE_PONG)
      {
         printf("DAG Node: Pong packet received, non-answered packet counter reset\n");
         net_off(NOW);
         non_answered_packet = 0;
      }

      if (data[2] == DATA_TYPE_FIRMWARE)
      {
         printf("DAG Node: DATA_TYPE_FIRMWARE packet received:");
         for (int i = 0; i < datalen; i++)
         {
             printf("%"PRIXX8, data[i]);
         }
         printf("\n");
      }

      if (data[2] != DATA_TYPE_COMMAND &&
            data[2] != DATA_TYPE_JOIN_CONFIRM &&
            data[2] != DATA_TYPE_SETTINGS &&
            data[2] != DATA_TYPE_FIRMWARE &&
            data[2] != DATA_TYPE_PONG)
      {
         printf("DAG Node: Incompatible data type UDP packer from");
         uip_debug_ip6addr_print(sender_addr);
         printf(", data type: 0x%02x\n", data[2]);
      }

   }
   else
   {
      printf("DAG Node: Incompatible device or protocol version UDP packer from");
      uip_debug_ip6addr_print(sender_addr);
      printf("(%02x%02x%02x)\n", data[0],data[1],data[2]);
   }

   led_off(LED_A);
}

/*---------------------------------------------------------------------------*/

void
print_debug_data(void)
{
   printf("\n");
   printf( "SYSTEM: uptime: %" PRIu32 " s\n", clock_seconds() );
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

void send_sensor_event(struct sensor_packet *packet)
{
   if (node_mode != MODE_NORMAL)
      return;

   if (packet == NULL)
      return;

   uip_ip6addr_t addr;
   uip_ip6addr_copy(&addr, &root_addr);

   printf("DAG Node: Send sensor-event message to DAG-root node:");
   uip_debug_ip6addr_print(&addr);
   printf("\n");

   uint8_t lenght = 10;
   uint8_t udp_buffer[lenght];
   udp_buffer[0] = packet->protocol_version;
   udp_buffer[1] = packet->device_version;
   udp_buffer[2] = packet->data_type;

   udp_buffer[3] = packet->number_ability;
   udp_buffer[4] = DATA_RESERVED;
   udp_buffer[5] = packet->sensor_number;
   udp_buffer[6] = packet->sensor_event;
   udp_buffer[7] = DATA_RESERVED;
   udp_buffer[8] = DATA_RESERVED;
   udp_buffer[9] = DATA_RESERVED;

   net_on();
   simple_udp_sendto(&udp_connection, udp_buffer, lenght + 1, &addr);
   net_off(TIMER);
}

/*---------------------------------------------------------------------------*/

void
send_status_packet(const uip_ipaddr_t *parent_addr,
                   uint32_t uptime,
                   int16_t rssi_parent,
                   uint8_t temp,
                   uint8_t voltage)
{
   if (parent_addr == NULL)
      return;

   uint8_t *uptime_uint8_t = (uint8_t *)&uptime;
   int8_t *rssi_parent_uint8_t = (int8_t *)&rssi_parent;
   uip_ip6addr_t addr;
   uip_ip6addr_copy(&addr, &root_addr);


   printf("DAG Node: Send status packet to DAG-root node:");
   uip_debug_ip6addr_print(&addr);
   printf("\n");

   uint8_t length = 23;
   uint8_t udp_buffer[length];
   udp_buffer[0] = PROTOCOL_VERSION_V1;
   udp_buffer[1] = CURRENT_DEVICE_VERSION;
   udp_buffer[2] = DATA_TYPE_STATUS;
   udp_buffer[3] = ( (uint8_t *)parent_addr )[8];
   udp_buffer[4] = ( (uint8_t *)parent_addr )[9];
   udp_buffer[5] = ( (uint8_t *)parent_addr )[10];
   udp_buffer[6] = ( (uint8_t *)parent_addr )[11];
   udp_buffer[7] = ( (uint8_t *)parent_addr )[12];
   udp_buffer[8] = ( (uint8_t *)parent_addr )[13];
   udp_buffer[9] = ( (uint8_t *)parent_addr )[14];
   udp_buffer[10] = ( (uint8_t *)parent_addr )[15];
   udp_buffer[11] = *uptime_uint8_t++;
   udp_buffer[12] = *uptime_uint8_t++;
   udp_buffer[13] = *uptime_uint8_t++;
   udp_buffer[14] = *uptime_uint8_t++;
   udp_buffer[15] = *rssi_parent_uint8_t++;
   udp_buffer[16] = *rssi_parent_uint8_t++;
   udp_buffer[17] = temp;
   udp_buffer[18] = voltage;
   udp_buffer[19] = DATA_RESERVED;
   udp_buffer[20] = DATA_RESERVED;
   udp_buffer[21] = DATA_RESERVED;
   udp_buffer[22] = DATA_RESERVED;

   net_on();
   simple_udp_sendto(&udp_connection, udp_buffer, length + 1, &addr);
   net_off(TIMER);
}


/*---------------------------------------------------------------------------*/

void
send_join_packet(const uip_ip6addr_t *dest_addr)
{
   if (dest_addr == NULL)
      return;

   uip_ip6addr_t addr;
   uip_ip6addr_copy(&addr, dest_addr);

   printf("DAG Node: Send join packet to DAG-root node:");
   uip_debug_ip6addr_print(&addr);
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
   simple_udp_sendto(&udp_connection, udp_buffer, length + 1, &addr);
}

/*---------------------------------------------------------------------------*/

static void
dag_root_find(void)
{
   rpl_dag_t *dag = NULL;

   if (uip_ds6_get_global(ADDR_PREFERRED) != NULL)
   {
      dag = rpl_get_any_dag();
      if (dag != NULL && &dag->dag_id)
         send_join_packet(&dag->dag_id);
   }
}


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(dag_node_button_process, ev, data)
{
   PROCESS_BEGIN();
   PROCESS_PAUSE();

   while (1)
   {
      PROCESS_YIELD();

      if (ev == sensors_event)
      {
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

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(net_off_process, ev, data)
{
   PROCESS_BEGIN();

   if (ev == PROCESS_EVENT_EXIT)
   {
      return 1;
   }
   else
   {
      static struct etimer net_off_timer;
      etimer_set( &net_off_timer, RADIO_OFF_DELAY);
      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&net_off_timer) );
      printf("DAG Node: Radio OFF on timer expired\n");
      NETSTACK_MAC.off(0);
   }

   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(maintenance_process, ev, data)
{
   PROCESS_BEGIN();
   PROCESS_PAUSE();

   while (1)
   {
      if (node_mode == MODE_NORMAL)
      {
         led_off(LED_A);
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
            led_off(LED_A);
            printf("DAG Node: Root not found, sleep\n");
            if (process_is_running(&dag_node_button_process) == 1)
               process_exit(&dag_node_button_process);

            if (process_is_running(&root_find_process) == 1)
               process_exit(&root_find_process);

            if (process_is_running(&status_send_process) == 1)
               process_exit(&status_send_process);

            if (process_is_running(&maintenance_process) == 1)
               process_exit(&maintenance_process);

            net_off(NOW);
         }

         if (CLASS == CLASS_C)
         {
            printf("DAG Node: Root not found, reboot\n");
            watchdog_reboot();
         }
      }

      if (node_mode == MODE_JOIN_PROGRESS)
      {
         net_on();
         led_on(LED_A);

         if (process_is_running(&root_find_process) == 0)
            process_start(&root_find_process, NULL);

         if (process_is_running(&status_send_process) == 1)
            process_exit(&status_send_process);
      }

      etimer_set( &maintenance_timer, SHORT_STATUS_INTERVAL);
      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&maintenance_timer) );
   }
   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(rpl_maintenance_process, ev, data)
{
   PROCESS_BEGIN();
   static struct etimer rpl_maintenance_timer;
   static uint32_t delay;
   static uip_ds6_nbr_t *nbr = NULL;
   PROCESS_PAUSE();


   while (1)
   {
      nbr = uip_ds6_nbr_lookup(uip_ds6_defrt_choose());
      if (nbr == NULL)
      {
         node_rpl_maintenance = TRUE;
      }
      else
      {
         if(nbr->state != NBR_REACHABLE)
            node_rpl_maintenance = TRUE;
         else
            node_rpl_maintenance = FALSE;
      }

      if (node_rpl_maintenance == TRUE)
         net_on();

      if (node_rpl_maintenance == FALSE)
         net_off(TIMER);


      if (node_rpl_maintenance == FALSE)
         delay = (5 * CLOCK_SECOND);
      else
         delay = (CLOCK_SECOND);

      etimer_set( &rpl_maintenance_timer, delay);
      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&rpl_maintenance_timer) );
   }
   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(status_send_process, ev, data)
{
   PROCESS_BEGIN();
   static struct etimer status_send_timer;
   const rpl_dag_t *dag = NULL;
   PROCESS_PAUSE();

   while (1)
   {
      //print_debug_data();

      dag = rpl_get_any_dag();

      if (dag != NULL && node_mode == MODE_NORMAL)
      {

         if (rpl_parent_is_reachable(dag->preferred_parent) == 0)
         {
            printf("DAG Node: Parent is not reachable\n");
            //node_mode = MODE_RPL_PROBING;
            //watchdog_reboot();
            //rpl_local_repair(dag->instance);
            //uip_ipaddr_t *ipaddr_parent = rpl_get_parent_ipaddr(dag->preferred_parent);
            //printf("RPL: parent ip address: ");
            //uip_debug_ipaddr_print(ipaddr_parent);
            //printf("\n");
         }


         const uip_ipaddr_t *ipaddr_parent = rpl_get_parent_ipaddr(dag->preferred_parent);
         const struct link_stats *stat_parent = rpl_get_parent_link_stats(dag->preferred_parent);
         uint8_t temp = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
         uint8_t voltage = ( (batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT) * 125) >> 5 ) / VOLTAGE_PRESCALER;
         if (ipaddr_parent != NULL && stat_parent != NULL)
         {
            send_status_packet(ipaddr_parent, clock_seconds(), stat_parent->rssi, temp, voltage);
         }
         non_answered_packet++;
         printf("DAG Node: Non-answered packet counter increase(status message): %"PRId8" \n", non_answered_packet);
      }

      if (CLASS == CLASS_B)
      {
         printf("DAG Node: Next status message planned on long interval\n");
         etimer_set( &status_send_timer, LONG_STATUS_INTERVAL + (random_rand() % LONG_STATUS_INTERVAL) );
      }
      if (CLASS == CLASS_C)
      {
         printf("DAG Node: Next status message planned on short interval\n");
         etimer_set( &status_send_timer, SHORT_STATUS_INTERVAL + (random_rand() % SHORT_STATUS_INTERVAL) );
      }

      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&status_send_timer) && node_mode == MODE_NORMAL );
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
   PROCESS_PAUSE();

   etimer_set( &find_root_limit_timer, ROOT_FIND_LIMIT_TIME);

   while (1)
   {
      etimer_set( &find_root_timer, ROOT_FIND_INTERVAL + (random_rand() % ROOT_FIND_INTERVAL) );
      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&find_root_timer) );

      if (node_mode == MODE_JOIN_PROGRESS)
      {
         if (etimer_expired(&find_root_limit_timer))
         {
            node_mode = MODE_NOTROOT;
            printf("DAG Node: mode set to MODE_NOTROOT\n");
            etimer_set(&maintenance_timer, 0);
         }
         dag_root_find();
      }
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

   printf("Node started, %s mode, %s class\n",
          rpl_get_mode() == RPL_MODE_LEAF ? "leaf" : "no-leaf",
          CLASS == CLASS_B ? "B(sleep)" : "C(non-sleep)");

   process_start(&dag_node_button_process, NULL);
   process_start(&maintenance_process, NULL);

   SENSORS_ACTIVATE(batmon_sensor);

   led_on(LED_A);

   PROCESS_END();
}
