/** @file   ota-common.c
 *  @brief  OTA common funtions and defines
 *  @author Vlad Zaytsev <vvzvlad@gmail.com>
 */

#include "string.h"
#include <stdbool.h>
#include "ota-common.h"

#include "driverlib/flash.h"
#include "driverlib/vims.h"

/*---------------------------------------------------------------------------*/

void user_flash_update_byte(uint8_t offset, uint8_t data)
{
   uint32_t old_vims_state = ti_lib_vims_mode_get(VIMS_BASE);
   ti_lib_vims_mode_set(VIMS_BASE, VIMS_MODE_DISABLED);

   uint8_t buffer[USER_FLASH_LENGTH];
   int_flash_read(buffer, START_USER_FLASH, USER_FLASH_LENGTH);
   buffer[offset] = data;

   ti_lib_vims_mode_set(VIMS_BASE, old_vims_state);

   ti_lib_flash_sector_erase(START_USER_FLASH);
   ti_lib_flash_program(buffer, START_USER_FLASH, USER_FLASH_LENGTH);
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

void int_flash_read(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count)
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

uint32_t int_flash_write(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count)
{
   uint32_t write_status = ti_lib_flash_program(pui8DataBuffer, ui32Address, ui32Count);
   return write_status;
}


/*---------------------------------------------------------------------------*/

uint8_t write_fw_flag(uint8_t value)
{
   if (value != FW_FLAG_NON_UPDATE &&
         value != FW_FLAG_NEW_IMG_EXT &&
         value != FW_FLAG_NEW_IMG_INT &&
         value != FW_FLAG_ERROR_GI_LOADED &&
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

uint8_t read_fw_flag()
{
   uint8_t flag_value = user_flash_read_byte(OTA_STATUS_FLAG);

   if (flag_value != FW_FLAG_NON_UPDATE &&
         flag_value != FW_FLAG_NEW_IMG_EXT &&
         flag_value != FW_FLAG_NEW_IMG_INT &&
         flag_value != FW_FLAG_ERROR_GI_LOADED &&
         flag_value != FW_FLAG_NEW_IMG_INT_RST &&
         flag_value != FW_FLAG_PING_OK)
   {
      write_fw_flag(FW_FLAG_NON_UPDATE);
      return FW_FLAG_NON_UPDATE;
   }

   return flag_value;
}

/*---------------------------------------------------------------------------*/


