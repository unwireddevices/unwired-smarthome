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
 *         RTC functions for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 *
 */
/*---------------------------------------------------------------------------*/

#include "contiki.h"
#include "contiki-lib.h"

#include <string.h>
#include <stdio.h>
#include "ti-lib.h"
#include "sys/clock.h"

#include "rtc-common.h"
#include "system-common.h"
#include "dag_node.h"

#include "ud_binary_protocol.h"
#include "xxf_types_helper.h"

/*---------------------------------------------------------------------------*/

#define TIME_SYNC_INTERVAL        (60 * 60 * CLOCK_SECOND)

uint32_t epoch_sec_offset = 0;
uint16_t epoch_msec_offset = 0;

PROCESS(time_sync_process, "Time sync process");

/*---------------------------------------------------------------------------*/

/*
 * Пакет DATA_TYPE_SET_TIME_RESPONSE состоит из следующих данных:
 * struct udbp-set-time-responce-packet
      {
         uint8_t protocol_version;
         uint8_t device_version;
         uint8_t DATA_TYPE_SET_TIME; //Тип — пакеты синхронизации времени
         uint8_t DATA_TYPE_SET_TIME_RESPONSE; //Подтип — ответ на запрос
         uint32_t send_responce_time_s; //Локальное время координатора(с) в момент отправки ответа
         uint16_t send_responce_time_ms; //Локальное время координатора(мс) в момент отправки ответа
         uint32_t send_responce_time_s; //Локальное время ноды(с) в момент отправки запроса
         uint16_t send_responce_time_ms; //Локальное время ноды(мс) в момент отправки запроса
      };
 *
 */
/*---------------------------------------------------------------------------*/
void time_data_handler(const uint8_t *data, uint16_t datalen)
{
   time_data_t root_time;
   time_data_t local_time_req;
   time_data_t local_time_res;

   for (uint8_t i = 0; i < 4; i++)
      root_time.seconds_u8[i] = data[i+4];
   for (uint8_t i = 0; i < 2; i++)
      root_time.milliseconds_u8[i] = data[i+8];

   for (uint8_t i = 0; i < 4; i++)
      local_time_req.seconds_u8[i] = data[i+10];
   for (uint8_t i = 0; i < 2; i++)
      local_time_req.milliseconds_u8[i] = data[i+14];

   local_time_res = get_epoch_time();

   uint16_t half_transit_time = calculate_transit_time(local_time_req, local_time_res);

   //printf("TIME SYNC: responce recieved: %" PRIu32 " sec, %" PRIu16 " ms, half transit time: %" PRIu16 " ms\n", root_time.seconds, root_time.milliseconds, half_transit_time);

   if (half_transit_time > 3000)
      return;

   while (half_transit_time > 1000)
   {
      half_transit_time = half_transit_time - 1000;
      root_time.seconds--;
   }

   if (half_transit_time > root_time.milliseconds)
   {
      root_time.seconds--;
      root_time.milliseconds = root_time.milliseconds + 1000;
   }
   root_time.milliseconds = root_time.milliseconds - half_transit_time;

   set_epoch_time(root_time);
   u8_i16_t time_diff_ms;
   time_diff_ms.i16 = calculate_diff_time(root_time, local_time_res);

   //printf("Time sync: local time: %" PRIu32 " sec, %" PRIu16 " ms\n", local_time_res.seconds, local_time_res.milliseconds);
   printf("Time sync: new time: %" PRIu32 ",%03" PRIu16 " s, sync error: %" PRIi16 " ms\n", root_time.seconds, root_time.milliseconds, time_diff_ms.i16 == 32767 ? 0 : time_diff_ms.i16);
   send_message_packet(DEVICE_MESSAGE_TIMESYNC_STATUS, time_diff_ms.u8[0], time_diff_ms.u8[1]);
}



/*---------------------------------------------------------------------------*/

PROCESS_THREAD(time_sync_process, ev, data)
{
   PROCESS_BEGIN();

   if (ev == PROCESS_EVENT_EXIT)
      return 1;

   static struct etimer time_sync_timer;

   while (1)
   {
      send_time_sync_req_packet();
      etimer_set( &time_sync_timer, TIME_SYNC_INTERVAL + (random_rand() % CLOCK_SECOND*2) );
      PROCESS_WAIT_EVENT_UNTIL( etimer_expired(&time_sync_timer) );
   }

   PROCESS_END();
}

/*---------------------------------------------------------------------------*/

int16_t calculate_diff_time(time_data_t time_max, time_data_t time_min)
{
   int64_t diff_time_s = time_max.seconds - time_min.seconds;
   int64_t diff_time_ms = time_max.milliseconds - time_min.milliseconds;

   diff_time_ms = ((diff_time_s * 1000) + diff_time_ms);

   //printf("RTC_COMMON: diff %"PRIi64" ms\n", diff_time_ms);

   if (diff_time_s > 30 || diff_time_s < -30 || diff_time_ms > 30*1000 || diff_time_ms < -30*1000)
      return 32767;
   else
      return (int16_t)diff_time_ms;
}
/*---------------------------------------------------------------------------*/

uint16_t calculate_transit_time(time_data_t time_req, time_data_t time_res)
{
   uint32_t transit_time_s = time_res.seconds - time_req.seconds;
   uint32_t transit_time_ms = time_res.milliseconds - time_req.milliseconds;
   uint32_t half_transit_time_ms = ((transit_time_s * 1000) + transit_time_ms) / 2;
   //printf("RTC_COMMON: transit time: %"PRIu32" sec, %"PRIu32" ms\n", transit_time_s, transit_time_ms);
   return (uint16_t)half_transit_time_ms;
}

/*---------------------------------------------------------------------------*/

void set_epoch_time(time_data_t new_time)
{
   time_data_t local_time;
   local_time.seconds = rtc_s();
   local_time.milliseconds = rtc_ms();

   if ((new_time.milliseconds - local_time.milliseconds) >= 1000)
   {
      epoch_msec_offset = (new_time.milliseconds - local_time.milliseconds) - 1000;
      epoch_sec_offset = (new_time.seconds - local_time.seconds) + 1;
   }
   else if ((new_time.milliseconds - local_time.milliseconds) < 0)
   {
      epoch_msec_offset = (new_time.milliseconds - local_time.milliseconds) + 1000;
      epoch_sec_offset = (new_time.seconds - local_time.seconds) - 1;
   }
   else
   {
      epoch_msec_offset = new_time.milliseconds - local_time.milliseconds;
      epoch_sec_offset = new_time.seconds - local_time.seconds;
   }
   //printf("RTC_COMMON: set offset: %" PRIu32 " sec, %" PRIi16 " ms\n", epoch_sec_offset, epoch_msec_offset);
}

/*---------------------------------------------------------------------------*/

time_data_t get_epoch_time()
{
   time_data_t local_time;

   //printf("RTC_COMMON: get offsets: %" PRIu32 " sec, %" PRIi16 " ms\n", epoch_sec_offset, epoch_msec_offset);
   local_time.seconds = rtc_s() + epoch_sec_offset;
   local_time.milliseconds = rtc_ms() + epoch_msec_offset;

   if (local_time.milliseconds >= 1000)
   {
      local_time.seconds++;
      local_time.milliseconds = local_time.milliseconds - 1000;
   }
   else if (local_time.milliseconds < 0)
   {
      local_time.seconds--;
      local_time.milliseconds = local_time.milliseconds + 1000;
   }
   //printf("RTC_COMMON: get time: %" PRIu32 " sec, %" PRIi16 " ms\n", local_time.seconds, local_time.milliseconds);

   return local_time;
}

/*---------------------------------------------------------------------------*/

