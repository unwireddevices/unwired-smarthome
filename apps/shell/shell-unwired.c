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
 *         Shell for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 */
/*---------------------------------------------------------------------------*/

#include "contiki.h"
#include "contiki-lib.h"
#include "contiki-net.h"
#include "sys/cc.h"
#include "shell-unwired.h"

#include <stdio.h>
#include <string.h>

#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ip/uip-debug.h"
#include "net/link-stats.h"
#include "core/lib/sensors.h"
#include "batmon-sensor.h"

#include "rtc-common.h"
#include "dag_node.h"

#include "xxf_types_helper.h"
#include "ud_binary_protocol.h"

/*---------------------------------------------------------------------------*/
PROCESS(unwired_shell_time_process, "time");
SHELL_COMMAND(unwired_shell_time_command, "time", "time: show the current node time in unix epoch", &unwired_shell_time_process);

PROCESS(unwired_shell_uptime_process, "uptime");
SHELL_COMMAND(unwired_shell_uptime_command, "uptime", "uptime: show the current node uptime", &unwired_shell_uptime_process);

PROCESS(unwired_shell_timesync_process, "timesync");
SHELL_COMMAND(unwired_shell_timesync_command, "timesync", "timesync: sync time now", &unwired_shell_timesync_process);

PROCESS(unwired_shell_status_process, "status");
SHELL_COMMAND(unwired_shell_status_command, "status", "status: show node status", &unwired_shell_status_process);

PROCESS(unwired_shell_set_channel_process, "set_channel");
SHELL_COMMAND(unwired_shell_set_channel_command, "set_channel", "set_channel: change radio channel", &unwired_shell_set_channel_process);

PROCESS(unwired_shell_test_process, "test");
SHELL_COMMAND(unwired_shell_test_command, "test", "test: test func", &unwired_shell_test_process);
/*---------------------------------------------------------------------------*/

uint8_t parse_args(char *args_string, char **args, uint8_t max_args)
{
   uint8_t i = 0;
   args[i] = strtok(args_string," ");
   while (args[i] != NULL && i != max_args)
   {
      i++;
      args[i] = strtok(NULL," ");
   }
   return i;
}

typedef enum str2int_errno_t {
   STR2INT_SUCCESS,
   STR2INT_OVERFLOW,
   STR2INT_UNDERFLOW,
   STR2INT_INCONVERTIBLE
} str2int_errno_t;


uint8_t str2uint(char *s)
{
   int temp = 0;
   for (uint8_t i = 0; i < 5; i++)
   {
      if (s[i] >= 0x30 && s[i] <= 0x39)
      {
         temp = temp + (s[i] & 0x0F);
         temp = temp * 10;
      }
      else
         break;
   }
   temp = temp / 10;
   if (temp > 0xFF)
      temp = 0;
   return (uint8_t)temp;
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(unwired_shell_uptime_process, ev, data)
{
  PROCESS_BEGIN();
  printf( "Uptime: %" PRIu32 " sec, %" PRIu16 " ms\n", rtc_s(), rtc_ms());
  printf("\n\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unwired_shell_time_process, ev, data)
{
  PROCESS_BEGIN();
  time_data_t time = get_epoch_time();
  printf( "RTC: %" PRIu32 " sec, %" PRIu16 " ms\n", time.seconds, time.milliseconds);
  printf("\n\n");
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unwired_shell_status_process, ev, data)
{
   PROCESS_BEGIN();
   rpl_dag_t *dag = rpl_get_any_dag();

   if (dag)
   {
      uip_ipaddr_t *ipaddr_parent = rpl_get_parent_ipaddr(dag->preferred_parent);
      printf("STATUS: rpl parent ip address: ");
      uip_debug_ipaddr_print(ipaddr_parent);
      printf("\n");

      uip_ipaddr_t dag_id_addr = dag->dag_id;
      printf("STATUS: rpl dag root ip address: ");
      uip_debug_ipaddr_print(&dag_id_addr);
      printf("\n");

      const struct link_stats *stat_parent = rpl_get_parent_link_stats(dag->preferred_parent);
      printf("STATUS: rpl parent last tx: %u sec ago\n", (unsigned)((clock_time() - stat_parent->last_tx_time) / (CLOCK_SECOND)));

      printf("STATUS: rpl parent rssi: %" PRId16 "\n", stat_parent->rssi);

      int parent_is_reachable = rpl_parent_is_reachable(dag->preferred_parent);
      printf("STATUS: rpl parent is reachable: %" PRId16 "\n", parent_is_reachable);

   }
   uint8_t temp = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
   printf("STATUS: temp: %"PRIu8"C, voltage: %"PRId16"mv\n", temp, ((batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT) * 125) >> 5));

   printf("\n\n");
   PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unwired_shell_timesync_process, ev, data)
{
  PROCESS_BEGIN();
  send_time_sync_req_packet();
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unwired_shell_set_channel_process, ev, data)
{
   uint8_t max_args = 1;
   uint8_t argc = 0;
   char *args[max_args];
   const char *nextptr;
   uint8_t channel = 0;

   PROCESS_BEGIN();

   argc = parse_args(data, args, max_args);
   if (argc == 0)
   {
      printf("No arg!\n");
      PROCESS_EXIT();
   }

   channel = shell_strtolong(args[0], &nextptr);
   //printf("Channel text: %"CHAR"\n", args[0]);
   //volatile uint8_t channel = str2uint(args[0]);


   printf("Number channel: %"PRIu8"\n", channel);



   PROCESS_END();
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(unwired_shell_test_process, ev, data)
{
   uint8_t max_args = 1;
   uint8_t argc;
   char *args[max_args];
   PROCESS_BEGIN();

   argc = parse_args(data, args, max_args);
   printf ("argc: %"PRIu8"\n", argc);

   for (uint8_t i = 0; i < argc; i++)
   {
      printf ("%s\n",args[i]);
   }

   PROCESS_END();
}
/*---------------------------------------------------------------------------*/
void unwired_shell_init(void)
{
  shell_register_command(&unwired_shell_time_command);
  shell_register_command(&unwired_shell_uptime_command);
  shell_register_command(&unwired_shell_timesync_command);
  shell_register_command(&unwired_shell_status_command);
  shell_register_command(&unwired_shell_test_command);
  shell_register_command(&unwired_shell_set_channel_command);

}
/*---------------------------------------------------------------------------*/
