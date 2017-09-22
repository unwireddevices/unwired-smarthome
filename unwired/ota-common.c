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
