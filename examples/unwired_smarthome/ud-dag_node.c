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
#include "dev/cc26xx-uart.h"

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
#  include "ud-button.h"
#endif

#ifdef IF_UD_RELAY
#  include "ud-relay.h"
#endif

#include "fake_headers.h" //no move up! not "krasivo"!

#define DEBUG_INTERVAL                    (60 * CLOCK_SECOND)
//#define DEBUG_INTERVAL                      (2 * CLOCK_SECOND)
#define SHORT_PING_INTERVAL                (5 * CLOCK_SECOND)
#define LONG_PING_INTERVAL                (50 * CLOCK_SECOND)
#define STATUS_SEND_INTERVAL         (10 * 60 * CLOCK_SECOND)

#define MAX_NON_ANSWERED_PINGS              5

#define DPRINT  printf(">>%s:%"PRIu16"\n", __FILE__, __LINE__);


/*---------------------------------------------------------------------------*/

unsigned char uart_char;
int start = 0;
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
    DPRINT
	led_on(LED_A); DPRINT
	printf("DEBUG: UDP packer: %02x,%02x,%02x from ", data[0],data[1],data[2]); DPRINT
	uip_debug_ipaddr_print(sender_addr); DPRINT
	printf("\n"); DPRINT

	if (data[0] == PROTOCOL_VERSION_V1 && data[1] == CURRENT_DEVICE_VERSION)
	{
	    DPRINT
		switch ( data[2] )
		{
				case DATA_TYPE_CONFIRM:
				    DPRINT
					printf("DAG Node: DAG join packet confirmation received, DAG active\n");
					led_off(LED_A); DPRINT
					dag_active = 1; DPRINT
					root_addr = *sender_addr; DPRINT
					non_answered_ping = 0; DPRINT
					if (process_is_running(&status_send_process) == 0)
					{
					    DPRINT
						process_start(&status_send_process, NULL); DPRINT
					}

					break;
				case DATA_TYPE_COMMAND:
					printf("DAG Node: Command packet received\n"); DPRINT
					static struct command_data message_for_main_process; DPRINT
					message_for_main_process.ability_target = data[3]; DPRINT
					message_for_main_process.ability_number = data[4]; DPRINT
					message_for_main_process.ability_state = data[5]; DPRINT
					process_post(&main_process, PROCESS_EVENT_CONTINUE, &message_for_main_process); DPRINT
					break;
				default:
				    DPRINT
					printf("DAG NODE: Incompatible data type(%02x)!\n", data[2]); DPRINT
					break;
		} /* switch */
	    DPRINT
	}
	else
	{
	    DPRINT
		printf("DAG NODE: Incompatible device or protocol version!\n"); DPRINT
	}

	led_off(LED_A); DPRINT
}

/*---------------------------------------------------------------------------*/

void
print_debug_data(void)
{
    DPRINT
	printf("\n");
	printf( "SYSTEM: uptime: %" PRIu32 " s\n", clock_seconds() ); DPRINT
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
    DPRINT
	uint8_t *uptime_uint8_t = (uint8_t *)&uptime; DPRINT
	uint8_t *rssi_parent_uint8_t = (int8_t *)&rssi_parent; DPRINT

	uint8_t length = 23; DPRINT
	uint8_t buf[length]; DPRINT
	buf[0] = PROTOCOL_VERSION_V1;
	buf[1] = CURRENT_DEVICE_VERSION;
	buf[2] = DATA_TYPE_STATUS;
	buf[3] = ( (uint8_t *)parent_addr )[8];
	buf[4] = ( (uint8_t *)parent_addr )[9];
	buf[5] = ( (uint8_t *)parent_addr )[10];
	buf[6] = ( (uint8_t *)parent_addr )[11];
	buf[7] = ( (uint8_t *)parent_addr )[12];
	buf[8] = ( (uint8_t *)parent_addr )[13];
	buf[9] = ( (uint8_t *)parent_addr )[14];
	buf[10] = ( (uint8_t *)parent_addr )[15];
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
	buf[22] = DATA_RESERVED; DPRINT

	simple_udp_sendto(connection, buf, length + 1, dest_addr); DPRINT
}


/*---------------------------------------------------------------------------*/

void
send_join_packet(const uip_ip6addr_t *dest_addr, struct simple_udp_connection *connection)
{
    DPRINT
	uint8_t length = 10; DPRINT
	uint8_t buf[length]; DPRINT
	buf[0] = PROTOCOL_VERSION_V1;
	buf[1] = CURRENT_DEVICE_VERSION;
	buf[2] = DATA_TYPE_JOIN;
	buf[3] = CURRENT_DEVICE_GROUP;
	buf[4] = CURRENT_DEVICE_SLEEP_TYPE;
	buf[5] = CURRENT_ABILITY_1BYTE;       //TODO: заменить на нормальную схему со сдвигами
	buf[6] = CURRENT_ABILITY_2BYTE;
	buf[7] = CURRENT_ABILITY_3BYTE;
	buf[8] = CURRENT_ABILITY_4BYTE;
	buf[9] = DATA_RESERVED; DPRINT
	simple_udp_sendto(connection, buf, length + 1, dest_addr); DPRINT
}


/*---------------------------------------------------------------------------*/

static void
dag_root_find(void)
{
	rpl_dag_t *dag = NULL; DPRINT
	uip_ip6addr_t addr; DPRINT

	uip_ds6_addr_t *addr_desc = uip_ds6_get_global(ADDR_PREFERRED); DPRINT
	if (addr_desc != NULL)
	{
	    DPRINT
		dag = rpl_get_any_dag(); DPRINT
		if (dag)
		{
		    DPRINT
			led_blink(LED_A); DPRINT
			if (&dag->dag_id)
			{
			    DPRINT
				if (dag_active == 0)
				{
					uip_ip6addr_copy(&addr, &dag->dag_id); DPRINT

					printf("DAG node: send join packet to rpl root"); DPRINT
					uip_debug_ip6addr_print(&addr); DPRINT
					printf("\n"); DPRINT
					send_join_packet(&addr, &udp_connection); DPRINT
					if (non_answered_ping < 100)
					{
					    DPRINT
						non_answered_ping++; DPRINT
					}
				}
			}
			else
			{
			    DPRINT
				//printf("RPL: address destination: none \n");
				dag_active = 0; DPRINT
			}
		}
	}

	if (non_answered_ping > MAX_NON_ANSWERED_PINGS)
	{
	    DPRINT
		dag_active = 0; DPRINT
	}
}


/*---------------------------------------------------------------------------*/

PROCESS_THREAD(dag_node_button_process, ev, data)
{
	PROCESS_BEGIN();
	DPRINT
	PROCESS_PAUSE();
	DPRINT
	while (1)
	{
	    DPRINT
		PROCESS_YIELD(); DPRINT

		if (ev == sensors_event)
		{
		    DPRINT
			if (data == &button_e_sensor_click)
			{
			    DPRINT
				//printf("DAG Node: Local repair activated\n"); DPRINT
				//rpl_dag_t *dag = rpl_get_any_dag(); DPRINT
				//rpl_local_repair(dag->instance); DPRINT
			    process_start(&dag_node_process, NULL); DPRINT
			}

			if (data == &button_e_sensor_long_click)
			{
			    DPRINT
				led_on(LED_A); DPRINT
				printf("SYSTEM: Button E long click, reboot\n"); DPRINT
				watchdog_reboot(); DPRINT
			}
		}
	}

	PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(status_send_process, ev, data)
{
	PROCESS_BEGIN(); DPRINT
	static struct etimer status_send_timer; DPRINT
	const rpl_dag_t *dag = NULL; DPRINT
	PROCESS_PAUSE(); DPRINT

	while (1)
	{
	    DPRINT
		dag = rpl_get_any_dag();DPRINT

		if (dag)
		{
		    DPRINT
			const uip_ipaddr_t *ipaddr_parent = rpl_get_parent_ipaddr(dag->preferred_parent); DPRINT
			const struct link_stats *stat_parent = rpl_get_parent_link_stats(dag->preferred_parent); DPRINT
			uint8_t temp = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP); DPRINT
			uint8_t voltage = ( (batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT) * 125) >> 5 ) / VOLTAGE_PRESCALER; DPRINT
			send_status_packet(&root_addr, &udp_connection, ipaddr_parent, clock_seconds(), stat_parent->rssi, temp, voltage); DPRINT
		}
		DPRINT
		etimer_set( &status_send_timer, status_send_interval + (random_rand() % status_send_interval) ); DPRINT
		PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&status_send_timer) ); DPRINT
	}
	DPRINT
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(root_ping_process, ev, data)
{
	PROCESS_BEGIN(); DPRINT

	static struct etimer ping_timer; DPRINT
	PROCESS_PAUSE(); DPRINT

	while (1)
	{
	    DPRINT
		etimer_set( &ping_timer, ping_interval + (random_rand() % ping_interval) ); DPRINT
		PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&ping_timer) ); DPRINT

		dag_root_find(); DPRINT

		if (dag_active == 0 && ping_interval != SHORT_PING_INTERVAL && non_answered_ping < 20)
		{
		    DPRINT
			ping_interval = SHORT_PING_INTERVAL; DPRINT
			uip_ds_6_interval_set(CLOCK_SECOND / 5); DPRINT
			printf( "DAG Node: Change timer to SHORT interval, DS6 interval: %" PRIu32 " ticks\n", uip_ds_6_interval_get() ); DPRINT
		}
		DPRINT
		if ( (dag_active == 1 && ping_interval != LONG_PING_INTERVAL) || non_answered_ping > 20 )
		{
		    DPRINT
			ping_interval = LONG_PING_INTERVAL; DPRINT
			uip_ds_6_interval_set(CLOCK_SECOND); DPRINT
			printf( "DAG Node: Change timer to LONG interval, DS6 interval: %" PRIu32 " ticks\n", uip_ds_6_interval_get() ); DPRINT
		}
		DPRINT
		if (non_answered_ping > 30)
		{
		    DPRINT
			printf("DAG Node: Not answer root, reboot\n"); DPRINT
			watchdog_reboot(); DPRINT
		}
		DPRINT
		if (non_answered_ping > 1)
		{
		    DPRINT
			printf("DAG Node: Non-answer ping count: %u\n", non_answered_ping); DPRINT
		}
	}
	DPRINT
	PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(dag_node_process, ev, data)
{
	PROCESS_BEGIN(); DPRINT
	static struct etimer debug_timer; DPRINT
	simple_udp_register(&udp_connection, UDP_DATA_PORT, NULL, UDP_DATA_PORT, udp_receiver); DPRINT

	PROCESS_PAUSE(); DPRINT

	if (RPL_CONF_LEAF_ONLY == 1)
	{
	    DPRINT
		rpl_set_mode(RPL_MODE_LEAF); DPRINT
	}
	else
	{
	    DPRINT
		rpl_set_mode(RPL_MODE_MESH); DPRINT
	}

	printf("DAG Node: started, %s mode\n", rpl_get_mode() == RPL_MODE_LEAF ? "leaf" : "no-leaf"); DPRINT

	//process_start(&dag_node_button_process, NULL); DPRINT
	process_start(&root_ping_process, NULL); DPRINT

	SENSORS_ACTIVATE(batmon_sensor); DPRINT

	led_on(LED_A); DPRINT

	while (1)
	{
	    DPRINT
		etimer_set( &debug_timer, debug_interval + (random_rand() % debug_interval) ); DPRINT
		PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&debug_timer) ); DPRINT
		print_debug_data(); DPRINT
	}
	DPRINT
	PROCESS_END();
}
