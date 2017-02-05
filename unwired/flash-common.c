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
 *         Flash write-read functions for Unwired Devices mesh smart house system(UDMSHS %) <- this is smile
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

#include "flash-common.h"
#include "driverlib/flash.h"
#include "ud_binary_protocol.h"


/*---------------------------------------------------------------------------*/

void flash_write_power_status(uint8_t channel_num, uint8_t channel_state)
{
    uint8_t write_data[1];
    write_data[0] = channel_state;
    if (channel_num == POWER_1_CH)
    {
        flash_write(write_data, POWER_1_CH_STATUS_ADDRESS, 0x1);
    }
    if (channel_num == POWER_2_CH)
    {
        flash_write(write_data, POWER_1_CH_STATUS_ADDRESS, 0x1);
    }
}

/*---------------------------------------------------------------------------*/

uint8_t flash_read_power_status(uint8_t channel_num)
{
    uint8_t read_len = 1;
    uint8_t read_buffer[read_len];
    uint32_t start_address;
    if (channel_num == POWER_1_CH)
    {
        start_address = POWER_1_CH_STATUS_ADDRESS;
    }

    if (channel_num == POWER_2_CH)
    {
        start_address = POWER_2_CH_STATUS_ADDRESS;
    }

    if (channel_num != POWER_1_CH && channel_num != POWER_2_CH)
    {
        return 0;
    }

    flash_read(read_buffer, start_address, read_len);

    return read_buffer[1];
}

/*---------------------------------------------------------------------------*/

void flash_read(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count)
{
    if (pui8DataBuffer != NULL && ui32Address+ui32Count < MAX_USER_FLASH && ui32Address > MIN_USER_FLASH)
    {
        uint8_t *pui8ReadAddress = (uint8_t *)ui32Address;
        while (ui32Count--) {
          *pui8DataBuffer++ = *pui8ReadAddress++;
        }
    }
}

/*---------------------------------------------------------------------------*/

void flash_write(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count)
{
    if (pui8DataBuffer != NULL && ui32Address+ui32Count < MAX_USER_FLASH && ui32Address > MIN_USER_FLASH)
    {
        FlashProgram(pui8DataBuffer, ui32Address, ui32Count);
    }
}
