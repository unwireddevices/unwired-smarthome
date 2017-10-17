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

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "net/rpl/rpl.h"
#include "net/rpl/rpl-private.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ip/uip-debug.h"
#include "net/link-stats.h"
#include "core/lib/sensors.h"
#include "batmon-sensor.h"
#include "ti-lib.h"
#include "net/mac/frame802154.h"

#include "rtc-common.h"
#include "dag_node.h"

#include "xxf_types_helper.h"
#include "ud_binary_protocol.h"

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
PROCESS(unwired_shell_time_process, "time");
SHELL_COMMAND(unwired_shell_time_command, "time", "time: show the current node time in unix epoch", &unwired_shell_time_process);

PROCESS(unwired_shell_uptime_process, "uptime");
SHELL_COMMAND(unwired_shell_uptime_command, "uptime", "uptime: show the current node uptime", &unwired_shell_uptime_process);

PROCESS(unwired_shell_timesync_process, "timesync");
SHELL_COMMAND(unwired_shell_timesync_command, "timesync", "timesync: sync time now", &unwired_shell_timesync_process);

PROCESS(unwired_shell_status_process, "status");
SHELL_COMMAND(unwired_shell_status_command, "status", "status: show node status", &unwired_shell_status_process);

PROCESS(unwired_shell_panid_process, "panid");
SHELL_COMMAND(unwired_shell_panid_command, "panid", "panid <set/get> <panid(ABCD)>: set/get panid", &unwired_shell_panid_process);

PROCESS(unwired_shell_channel_process, "channel");
SHELL_COMMAND(unwired_shell_channel_command, "channel", "channel <set/get> <num>: set/get radio channel", &unwired_shell_channel_process);

PROCESS(unwired_shell_test_process, "test");
SHELL_COMMAND(unwired_shell_test_command, "test", "test: test func", &unwired_shell_test_process);

//команда включения бутлоадера
/*---------------------------------------------------------------------------*/

static uint8_t parse_args(char *args_string, char **args, uint8_t max_args)
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

typedef enum str2int_errno_t{
   STR2INT_SUCCESS,
   STR2INT_OVERFLOW,
   STR2INT_UNDERFLOW,
   STR2INT_INCONVERTIBLE
} str2int_errno_t;

static str2int_errno_t hex_str2uint8(uint8_t *out, char *s) {
   char *end;
   if (s[0] == '\0' || isspace((unsigned char) s[0]))
       return STR2INT_INCONVERTIBLE;
   errno = 0;
   long l = strtol(s, &end, 16);
   /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
   if (l > 255 || (errno == ERANGE && l == LONG_MAX))
       return STR2INT_OVERFLOW;
   if (l < 0 || (errno == ERANGE && l == LONG_MIN))
       return STR2INT_UNDERFLOW;
   if (*end != '\0')
       return STR2INT_INCONVERTIBLE;
   *out = (uint8_t)l;
   return STR2INT_SUCCESS;
}

static str2int_errno_t hex_str2uint16(uint16_t *out, char *s) {
   char *end;
   if (s[0] == '\0' || isspace((unsigned char) s[0]))
       return STR2INT_INCONVERTIBLE;
   errno = 0;
   long l = strtol(s, &end, 16);
   /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
   if (l > 0xFFFF || (errno == ERANGE && l == LONG_MAX))
       return STR2INT_OVERFLOW;
   if (l < 0 || (errno == ERANGE && l == LONG_MIN))
       return STR2INT_UNDERFLOW;
   if (*end != '\0')
       return STR2INT_INCONVERTIBLE;
   *out = (uint16_t)l;
   return STR2INT_SUCCESS;
}


static str2int_errno_t dec_str2uint8(uint8_t *out, char *s) {
    char *end;
    if (s[0] == '\0' || isspace((unsigned char) s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;
    long l = strtol(s, &end, 10);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > 255 || (errno == ERANGE && l == LONG_MAX))
        return STR2INT_OVERFLOW;
    if (l < 0 || (errno == ERANGE && l == LONG_MIN))
        return STR2INT_UNDERFLOW;
    if (*end != '\0')
        return STR2INT_INCONVERTIBLE;
    *out = (uint8_t)l;
    return STR2INT_SUCCESS;
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(unwired_shell_uptime_process, ev, data)
{
  PROCESS_BEGIN();
  printf( "Uptime: %" PRIu32 " sec, %" PRIu16 " ms\n", rtc_s(), rtc_ms());
  printf("\n");
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(unwired_shell_time_process, ev, data)
{
  PROCESS_BEGIN();
  time_data_t time = get_epoch_time();
  printf( "RTC: %" PRIu32 " sec, %" PRIu16 " ms\n", time.seconds, time.milliseconds);
  printf("\n");
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
   uint8_t temp = (uint8_t)batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
   printf("STATUS: temp: %"PRIu8"C, voltage: %"PRId16"mv\n", temp, ((batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT) * 125) >> 5));

   printf("\n");
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

PROCESS_THREAD(unwired_shell_channel_process, ev, data)
{
   uint8_t max_args = 2;
   char *args[max_args+1]; //necessary to allocate on one pointer more
   uint8_t argc = 0;

   PROCESS_BEGIN();

   argc = parse_args(data, args, max_args);
   if (argc < 1)
   {
      printf("Channel: No args! Use \"channel <set/get> <num>\"\n");
      printf("\n");
      PROCESS_EXIT();
   }

   if (!strncmp(args[0], "get", 3))
   {
      radio_value_t channel = 0;
      NETSTACK_RADIO.get_value(RADIO_PARAM_CHANNEL, &channel);

      if (ti_lib_chipinfo_chip_family_is_cc26xx())
      {
         uint32_t freq_mhz = (2405 + 5 * (channel - 11));
         printf("Channel get: Current radio-channel: %"PRIint" (%"PRIu32" MHz)\n", (int)channel, freq_mhz);
      }

      if (ti_lib_chipinfo_chip_family_is_cc13xx())
      {
         uint32_t freq_khz = 863125 + (channel * 200);
         printf("Channel get: Current radio-channel: %"PRIint" (%"PRIu32" kHz)\n", (int)channel, freq_khz);
      }
   }

   if (!strncmp(args[0], "set", 3))
   {
      uint8_t channel = 0;
      str2int_errno_t status = dec_str2uint8(&channel, args[1]);

      uint8_t cc1310_max_channel = 33;
      uint8_t cc1310_min_channel = 0;

      uint8_t cc2650_min_channel = 11;
      uint8_t cc2650_max_channel = 26;

      if (status != STR2INT_SUCCESS)
      {
         printf("Channel set error: Incorrect channel number arg\n");
         printf("\n");
         PROCESS_EXIT();
      }


      if (ti_lib_chipinfo_chip_family_is_cc26xx())
      {
         if (channel > cc2650_max_channel || channel < cc2650_min_channel)
            printf("Channel set error: Select a channel in the range %"PRIu8"-%"PRIu8"\n", cc2650_min_channel, cc2650_max_channel);
         else
         {
            uint32_t freq_mhz = (2405 + 5 * (channel - 11));
            printf("Channel: Set new radio-channel: %"PRIu8" (%"PRIu32" MHz)\n", channel, freq_mhz);
            NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel);
         }
      }

      if (ti_lib_chipinfo_chip_family_is_cc13xx())
      {
         if (channel > cc1310_max_channel || channel < cc1310_min_channel)
            printf("Channel set error: Select a channel in the range %"PRIu8"-%"PRIu8"\n", cc1310_min_channel, cc1310_max_channel);
         else if (channel == 30 || channel == 29)
         {
            uint32_t freq_khz = 863125 + (channel * 200);
            printf("Channel: Set new radio-channel: %"PRIu8" (%"PRIu32" kHz)\n", channel, freq_khz);
            NETSTACK_RADIO.set_value(RADIO_PARAM_CHANNEL, channel);
         }
         else
         {
            printf("Channel set error: Сhannel %" PRIu8 " is not available in the current region(only 29/30 ch). ¯\\_(ツ)_/¯\n", channel);
            printf("\n");
            PROCESS_EXIT();
         }
      }
   }

   printf("\n");
   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(unwired_shell_panid_process, ev, data)
{
   uint8_t max_args = 2;
   char *args[max_args+1]; //necessary to allocate on one pointer more
   uint8_t argc = 0;

   PROCESS_BEGIN();

   argc = parse_args(data, args, max_args);
   if (argc < 1)
   {
      printf("PAN ID: No args! Use \"panid <set/get> <panid(ABCD)>\", value in hex\n");
      PROCESS_EXIT();
   }

   if (!strncmp(args[0], "get", 3))
   {
      if (ti_lib_chipinfo_chip_family_is_cc26xx())
      {
         radio_value_t panid = 0;
         NETSTACK_RADIO.get_value(RADIO_PARAM_PAN_ID, &panid);
         printf("PAN ID: Current ID %"PRIXX16"\n", panid);
      }
      else
      {
         printf("PAN ID: Not support in cc1310\n");
      }

   }

   if (!strncmp(args[0], "set", 3))
   {
      if (ti_lib_chipinfo_chip_family_is_cc26xx())
      {
         uint16_t panid = 0;
         str2int_errno_t status = hex_str2uint16(&panid, args[0]);
         if (status != STR2INT_SUCCESS)
         {
            printf("PAN ID set error: Incorrect id value\n");
            printf("\n");
            PROCESS_EXIT();
         }
         NETSTACK_RADIO.set_value(RADIO_PARAM_PAN_ID, panid);
         printf("PAN ID: Set new ID %"PRIXX16"\n", panid);
      }
      else
      {
         frame802154_set_pan_id(0xAAA);
         //printf("PAN ID: Not support in cc1310\n");
      }
   }


   if (!strncmp(args[0], "aaa", 3))
   {
      frame802154_set_pan_id(0xAAA);
   }

   printf("\n");
   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

PROCESS_THREAD(unwired_shell_test_process, ev, data)
{
   uint8_t max_args = 1;
   uint8_t argc;
   char *args[max_args+1];
   PROCESS_BEGIN();

   argc = parse_args(data, args, max_args);
   uint16_t value = 0;
   str2int_errno_t status = hex_str2uint16(&value, args[0]);
   printf("Hex convert: %"PRIu16"(%"PRIXX16"), %"PRIint"\n", value, value, (int)status);

   printf("\n");
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
  shell_register_command(&unwired_shell_channel_command);
  shell_register_command(&unwired_shell_panid_command);
}

/*---------------------------------------------------------------------------*/
