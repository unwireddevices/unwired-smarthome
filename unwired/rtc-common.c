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

#include "ud_binary_protocol.h"
#include "xxf_types_helper.h"

/*---------------------------------------------------------------------------*/

uint32_t epoch_sec_offset = 0;
uint16_t epoch_msec_offset = 0;

/*---------------------------------------------------------------------------*/

int16_t calculate_diff_time(time_data_t time_max, time_data_t time_min)
{
   int64_t diff_time_s = time_max.seconds - time_min.seconds;
   int64_t diff_time_ms = time_max.milliseconds - time_min.milliseconds;

   diff_time_ms = ((diff_time_s * 1000) + diff_time_ms);

   //printf("RTC_COMMON: diff %"PRIi64" ms\n", diff_time_ms);

   if (diff_time_s > 30 || diff_time_s < -30 || diff_time_ms > 30*1000 || diff_time_ms < -30*1000)
      return 32768;
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

void set_epoch_time(time_data_t time)
{
   uint32_t current_uptime = clock_seconds();
   uint16_t current_msec = clock_mseconds();

   if ((time.milliseconds - current_msec) > 1000)
   {
      epoch_msec_offset = (time.milliseconds - current_msec) - 1000;
      epoch_sec_offset = (time.seconds - current_uptime) + 1;
   }
   else if ((time.milliseconds - current_msec) < 0)
   {
      epoch_msec_offset = (time.milliseconds - current_msec) + 1000;
      epoch_sec_offset = (time.seconds - current_uptime) - 1;
   }
   //printf("RTC_COMMON: set offset: %" PRIu32 " sec, %" PRIi16 " ms\n", epoch_sec_offset, epoch_msec_offset);
}

/*---------------------------------------------------------------------------*/

time_data_t get_epoch_time()
{
   time_data_t time;

   //printf("RTC_COMMON: get offsets: %" PRIu32 " sec, %" PRIi16 " ms\n", epoch_sec_offset, epoch_msec_offset);


   time.seconds = clock_seconds() + epoch_sec_offset;
   time.milliseconds = clock_mseconds() + epoch_msec_offset;

   if (time.milliseconds > 1000)
   {
      time.seconds++;
      time.milliseconds = time.milliseconds - 1000;
   }
   else if (time.milliseconds < 0)
   {
      time.seconds--;
      time.milliseconds = time.milliseconds + 1000;
   }

   //printf("RTC_COMMON: get time: %" PRIu32 " sec, %" PRIi16 " ms\n", time.seconds, time.milliseconds);

   return time;
}

/*---------------------------------------------------------------------------*/

