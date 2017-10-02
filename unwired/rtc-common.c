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
int16_t epoch_msec_offset = 0;

uint32_t local_time_req_send_s = 0;
int16_t local_time_req_send_ms = 0;

/*---------------------------------------------------------------------------*/

void set_local_time_req_send()
{
   local_time_req_send_s = clock_seconds();
   local_time_req_send_ms = clock_mseconds();
}

/*---------------------------------------------------------------------------*/

uint16_t calculate_transit_time()
{
   uint32_t local_time_res_recieved_s = clock_seconds();
   uint16_t local_time_res_recieved_ms = clock_mseconds();

   uint32_t transit_time_s = local_time_res_recieved_s - local_time_req_send_s;
   int32_t transit_time_ms = local_time_res_recieved_ms - local_time_req_send_ms;

   int32_t full_transit_time_ms = (transit_time_s * 1000) + transit_time_ms;

   return (uint16_t)full_transit_time_ms;
}

/*---------------------------------------------------------------------------*/

void set_epoch_time(uint32_t epoch)
{
   uint32_t current_uptime = clock_seconds();
   if (epoch > current_uptime)
   {
      epoch_sec_offset = epoch - current_uptime;
   }
}

/*---------------------------------------------------------------------------*/

uint32_t get_epoch_time()
{
   uint32_t current_uptime = clock_seconds();
   return current_uptime + epoch_sec_offset;
}

/*---------------------------------------------------------------------------*/

void set_epoch_msec_time(uint16_t msec)
{
   uint16_t current_msec = clock_mseconds();
   epoch_msec_offset = msec - current_msec; //тут таится какая-то фигня!
}

/*---------------------------------------------------------------------------*/

uint16_t get_epoch_msec_time()
{
   uint16_t current_msec = clock_mseconds();
   return current_msec + epoch_msec_offset;
}

/*---------------------------------------------------------------------------*/

