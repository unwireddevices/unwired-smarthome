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

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "net/ip/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/mac/contikimac/contikimac.h"

#include "dev/leds.h"
#include "sys/clock.h"
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"
#include "board.h"
#include "net/ip/uip-debug.h"
#include "dev/cc26xx-uart.h"

#include <string.h>
#include <stdio.h>
#include "simple-udp.h"

#include "net/link-stats.h"
#include "ti-lib.h"
#include "clock.h"
#include "dev/watchdog.h"


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

#define DEBUG_INTERVAL                    (60 * CLOCK_SECOND)
//#define DEBUG_INTERVAL                      (2 * CLOCK_SECOND)
#define SHORT_PING_INTERVAL                (5 * CLOCK_SECOND)
#define LONG_PING_INTERVAL                (50 * CLOCK_SECOND)
#define STATUS_SEND_INTERVAL         (10 * 60 * CLOCK_SECOND)

#define MAX_NON_ANSWERED_PINGS              5

/*---------------------------------------------------------------------------*/

/* struct for simple_udp_send */
struct simple_udp_connection udp_connection;

/* set to 1, if rpl root found and answer to join packet */
volatile uint8_t dag_active = 0;

volatile uint8_t non_answered_ping = 0;
volatile uip_ip6addr_t root_addr;
static struct command_data message_for_main_process;

volatile clock_time_t debug_interval = DEBUG_INTERVAL;
volatile clock_time_t ping_interval = SHORT_PING_INTERVAL;
volatile clock_time_t status_send_interval = STATUS_SEND_INTERVAL;

/*---------------------------------------------------------------------------*/

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
             const uint8_t *data, //TODO: make "parse" function(data[0] -> data.protocol_version)
             uint16_t datalen)
{
    led_on(LED_A);


	if (data[0] == PROTOCOL_VERSION_V1 && data[1] == CURRENT_DEVICE_VERSION)
	{
	    if (data[2] == DATA_TYPE_CONFIRM)
	    {
            printf("DAG Node: DAG join packet confirmation received, DAG active\n");
            led_off(LED_A);
            dag_active = 1;
            uip_ipaddr_copy(&root_addr, sender_addr);
            non_answered_ping = 0;
            if (process_is_running(&status_send_process) == 0)
            {
                process_start(&status_send_process, NULL);
            }
	    }

        if (data[2] == DATA_TYPE_COMMAND)
        {
            printf("DAG Node: Command packet received\n");
            message_for_main_process.ability_target = data[3];
            message_for_main_process.ability_number = data[4];
            message_for_main_process.ability_state = data[5];
            process_post(&main_process, PROCESS_EVENT_CONTINUE, &message_for_main_process);
        }

        if (data[2] != DATA_TYPE_COMMAND && data[2] != DATA_TYPE_CONFIRM)
        {
            printf("DAG Node: Incompatible data type UDP packer from");
            uip_debug_ip6addr_print(sender_addr);
            printf("(%02x%02x%02x)\n", data[0],data[1],data[2]);
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

    uip_ip6addr_t addr;
    uip_ip6addr_copy(&addr, &root_addr);

    printf("DAG node: send sensor-event message to DAG-root node:");
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
    simple_udp_sendto(&udp_connection, udp_buffer, lenght + 1, &addr);
}

/*---------------------------------------------------------------------------*/

void
send_status_packet(const uip_ipaddr_t *parent_addr,
                   uint32_t uptime,
                   int16_t rssi_parent,
                   uint8_t temp,
                   uint8_t voltage)
{
    if (parent_addr == NULL){
        return;
    }

	uint8_t *uptime_uint8_t = (uint8_t *)&uptime;
	int8_t *rssi_parent_uint8_t = (int8_t *)&rssi_parent;
	uip_ip6addr_t addr;
	uip_ip6addr_copy(&addr, &root_addr);


    printf("DAG node: Send status packet to DAG-root node:");
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

	simple_udp_sendto(&udp_connection, udp_buffer, length + 1, &addr);
}


/*---------------------------------------------------------------------------*/

void
send_join_packet(const uip_ip6addr_t *dest_addr)
{

    uip_ip6addr_t addr;
    uip_ip6addr_copy(&addr, dest_addr);

	printf("DAG node: Send join packet to DAG-root node:");
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
set_activity_slow(void)
{
    if (RPL_CONF_LEAF_ONLY == 1)
    {
        uip_ds_6_interval_set(CLOCK_SECOND*2);
        printf( "DAG Node: new DS6 interval: %" PRIu32 " ticks\n", uip_ds_6_interval_get() );
        set_rdc_channel_check_rate_slow();
        printf( "DAG Node: new RDC check rate: %" PRIu8 " Hz\n", get_rdc_channel_check_rate() );
    }

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
        {
            if (dag_active == 0)
            {
                send_join_packet(&dag->dag_id);
                non_answered_ping++;
            }
        }
        else
        {
            dag_active = 0;
        }

        if (dag != NULL && rpl_parent_is_reachable(dag->preferred_parent) == 0)
        {
            dag_active = 0;
        }
	}

	if (non_answered_ping > MAX_NON_ANSWERED_PINGS)
	{
		dag_active = 0;
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
			if (data == &button_e_sensor_click)
			{
				printf("DAG Node: Local repair activated\n");
				rpl_dag_t *dag = rpl_get_any_dag();
				rpl_local_repair(dag->instance);
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

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(status_send_process, ev, data)
{
	PROCESS_BEGIN();
	static struct etimer status_send_timer;
	const rpl_dag_t *dag = NULL;
	PROCESS_PAUSE();

	while (1)
	{
		dag = rpl_get_any_dag();

		if (dag)
		{
			const uip_ipaddr_t *ipaddr_parent = rpl_get_parent_ipaddr(dag->preferred_parent);
			const struct link_stats *stat_parent = rpl_get_parent_link_stats(dag->preferred_parent);
			uint8_t temp = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
			uint8_t voltage = ( (batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT) * 125) >> 5 ) / VOLTAGE_PRESCALER;
			if (ipaddr_parent != NULL && stat_parent != NULL)
			{
                send_status_packet(ipaddr_parent, clock_seconds(), stat_parent->rssi, temp, voltage);
			}
		}

		etimer_set( &status_send_timer, status_send_interval + (random_rand() % status_send_interval) );
		PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&status_send_timer) );
	}

	PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(root_ping_process, ev, data)
{
	PROCESS_BEGIN();

	static struct etimer ping_timer;
	PROCESS_PAUSE();

	while (1)
	{
		etimer_set( &ping_timer, ping_interval + (random_rand() % ping_interval) );
		PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&ping_timer) );

		dag_root_find();

		if ((dag_active == 1 && ping_interval != LONG_PING_INTERVAL) || non_answered_ping > 20 )
		{
			ping_interval = LONG_PING_INTERVAL;
            printf("DAG Node: Change timer to LONG interval\n");
			set_activity_slow();
		}

		if (non_answered_ping > 30)
		{
			printf("DAG Node: Not answer root, reboot\n");
			watchdog_reboot();
		}

		if (non_answered_ping > 1)
		{
			printf("DAG Node: Non-answer ping count: %u\n", non_answered_ping);
		}
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
	{
		rpl_set_mode(RPL_MODE_LEAF);
	}
	else
	{
		rpl_set_mode(RPL_MODE_MESH);
	}

	set_rdc_channel_check_rate_fast();
	printf("DAG Node: started, %s mode\n", rpl_get_mode() == RPL_MODE_LEAF ? "leaf" : "no-leaf");
    printf("DAG Node: DS6 interval: %" PRIu32 " ticks\n", uip_ds_6_interval_get() );
    printf("DAG Node: RDC check rate: %" PRIu8 " Hz\n", get_rdc_channel_check_rate() );

	process_start(&dag_node_button_process, NULL);
	process_start(&root_ping_process, NULL);

	SENSORS_ACTIVATE(batmon_sensor);

	led_on(LED_A);

	while (1)
	{
		etimer_set( &debug_timer, debug_interval + (random_rand() % debug_interval) );
		PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&debug_timer) );
		print_debug_data();
	}

	PROCESS_END();
}
