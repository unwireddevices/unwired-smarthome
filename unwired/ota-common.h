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
/** @file   ota-common.h
 *  @brief  OTA common funtions and defines
 *  @author Vlad Zaytsev <vvzvlad@gmail.com>
 */

#include <stdio.h>

#include "ti-lib.h"
#include "driverlib/flash.h"


#define FLASH_PAGE_SIZE                0x1000

#define NON_CORRECT_CRC                -2
#define NON_READ_FLASH                 -1
#define CORRECT_CRC                     0

#define FLAG_OK_WRITE                  1
#define FLAG_ERROR_WRITE               0

/* NEW_FW_FLAG — Флаг статуса процесса обновления */
#define FW_FLAG                        0x42
#define FW_FLAG_ADRESS                 0x1000

#define FW_FLAG_NON_UPDATE             0x30 //Процесс обновления завершен или не начат("0" в ANSCII)
#define FW_FLAG_NEW_IMG_EXT            0x31 //Новый образ загружен во внешнюю память("1" в ANSCII)
#define FW_FLAG_NEW_IMG_INT_RST        0x32 //Новый образ загружен во внутреннюю память("2" в ANSCII), требуется перезагрузка
#define FW_FLAG_NEW_IMG_INT            0x33 //Новый образ загружен во внутреннюю память("3" в ANSCII), нормальный запуск
#define FW_FLAG_PING_OK                0x34 //Новый образ запущен, соединение с сервером проверено("4" в ANSCII)
#define FW_FLAG_ERROR_GI_LOAD          0x35 //Ошибка обновления, загружен Golden Image("5" в ANSCII)
#define FW_FLAG_NONE                   0xFF //На месте флага пустая память

#define FLAG_SIZE                      1

uint8_t write_fw_flag(uint8_t value);
uint8_t read_fw_flag();
