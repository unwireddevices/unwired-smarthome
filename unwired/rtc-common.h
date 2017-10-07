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
/**
 * \file
 *         Header file for rtc functions
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "ud_binary_protocol.h"

PROCESS_NAME(time_sync_process);

/*---------------------------------------------------------------------------*/

typedef struct time_data_t
{
   union {
      uint32_t seconds;
      uint8_t seconds_u8[4];
   };
   union {
      uint16_t milliseconds;
      uint8_t milliseconds_u8[2];
   };
} time_data_t;

/*---------------------------------------------------------------------------*/

void time_data_handler(const uint8_t *data, uint16_t datalen);




int16_t calculate_diff_time(time_data_t time_max, time_data_t time_min);

/*
 * Функция, рассчитывающее время следования пакета до координатора и обратно.
 * Возвращает полное(туда и обратно) время следования в миллисекундах.
 */
uint16_t calculate_transit_time(time_data_t time_req, time_data_t time_res);

/*
 * Получить значение глобального времени в секундах эпохи Unix.
 * Возвращает uptime(локальное время), если
 * время не было синхронизировано функцией set_epoch_time
 * Рассчитывается как локальное время со сдвигом относительно глобального.
 */
time_data_t get_epoch_time();

/*
 * Установить значение глобального времени(секунды и миллисекунды).
 */
void set_epoch_time(time_data_t time);
