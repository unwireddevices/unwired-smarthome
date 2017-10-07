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
/** @file   ota-common.c
 *  @brief  OTA common funtions and defines
 *  @author Vlad Zaytsev <vvzvlad@gmail.com>
 */

#include "string.h"
#include <stdbool.h>
#include "ota-common.h"
#include "int-flash-common.h"
#include "xxf_types_helper.h"

uint8_t
write_fw_flag(uint8_t value)
{
   if (value != FW_FLAG_NON_UPDATE &&
       value != FW_FLAG_NEW_IMG_EXT &&
       value != FW_FLAG_NEW_IMG_INT &&
       value != FW_FLAG_ERROR_GI_LOAD &&
       value != FW_FLAG_NEW_IMG_INT_RST &&
       value != FW_FLAG_PING_OK)
      return FLAG_ERROR_WRITE;

   user_flash_update_byte(OTA_STATUS_FLAG, value);
   uint8_t new_value = user_flash_read_byte(OTA_STATUS_FLAG);
   if (new_value == value)
      return FLAG_OK_WRITE;
   else
      return FLAG_ERROR_WRITE;
}

/*---------------------------------------------------------------------------*/

uint8_t
read_fw_flag()
{
   uint8_t flag_value = user_flash_read_byte(OTA_STATUS_FLAG);

   if (flag_value != FW_FLAG_NON_UPDATE &&
       flag_value != FW_FLAG_NEW_IMG_EXT &&
       flag_value != FW_FLAG_NEW_IMG_INT &&
       flag_value != FW_FLAG_ERROR_GI_LOAD &&
       flag_value != FW_FLAG_NEW_IMG_INT_RST &&
       flag_value != FW_FLAG_PING_OK)
   {
      write_fw_flag(FW_FLAG_NON_UPDATE);
      return FW_FLAG_NON_UPDATE;
   }

   return flag_value;
}

/*---------------------------------------------------------------------------*/
