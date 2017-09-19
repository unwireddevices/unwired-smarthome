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
 *         Internal flash write-read functions for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
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

#include "int-flash-common.h"
#include "driverlib/flash.h"
#include "driverlib/vims.h"
#include "ud_binary_protocol.h"
#include "xxf_types_helper.h"



void user_flash_update_byte(uint8_t offset, uint8_t data)
{
   uint32_t old_vims_state = ti_lib_vims_mode_get(VIMS_BASE);
   ti_lib_vims_mode_set(VIMS_BASE, VIMS_MODE_DISABLED);

   uint8_t buffer[USER_FLASH_LENGTH];
   flash_read(buffer, START_USER_FLASH, USER_FLASH_LENGTH);
   buffer[offset] = data;

   ti_lib_flash_sector_erase(START_USER_FLASH);
   ti_lib_flash_program(buffer, START_USER_FLASH, USER_FLASH_LENGTH);

   ti_lib_vims_mode_set(VIMS_BASE, old_vims_state);
}

/*---------------------------------------------------------------------------*/

uint8_t user_flash_read_byte(uint8_t offset)
{
   uint32_t old_vims_state = ti_lib_vims_mode_get(VIMS_BASE);
   ti_lib_vims_mode_set(VIMS_BASE, VIMS_MODE_DISABLED);
   uint32_t address = offset + START_USER_FLASH;
   uint8_t data = *(uint8_t *)address;
   ti_lib_vims_mode_set(VIMS_BASE, old_vims_state);
   return data;
}

/*---------------------------------------------------------------------------*/

void flash_read(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count)
{
   uint32_t old_vims_state = ti_lib_vims_mode_get(VIMS_BASE);
   ti_lib_vims_mode_set(VIMS_BASE, VIMS_MODE_DISABLED);
   uint8_t *pui8ReadAddress = (uint8_t *)ui32Address;
   while (ui32Count--)
   {
      *pui8DataBuffer++ = *pui8ReadAddress++;
   }
   ti_lib_vims_mode_set(VIMS_BASE, old_vims_state);
}

/*---------------------------------------------------------------------------*/

uint32_t flash_write(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count)
{
   uint32_t old_vims_state = ti_lib_vims_mode_get(VIMS_BASE);
   ti_lib_vims_mode_set(VIMS_BASE, VIMS_MODE_DISABLED);
   uint32_t write_status = ti_lib_flash_program(pui8DataBuffer, ui32Address, ui32Count);
   ti_lib_vims_mode_set(VIMS_BASE, old_vims_state);
   return write_status;
}
