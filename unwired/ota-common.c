/** @file   ota-common.c
 *  @brief  OTA common funtions and defines
 *  @author Vlad Zaytsev <vvzvlad@gmail.com>
 */

#include "string.h"
#include <stdbool.h>
#include "ota-common.h"
#include "ext-flash.h"

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

   int eeprom_access = ext_flash_open();
   if(!eeprom_access) { ext_flash_close(); return FLAG_ERROR_WRITE; }

   uint8_t data_write[ FLAG_SIZE ];
   uint8_t data_read[ FLAG_SIZE ];
   data_write[0] = value;

   eeprom_access = ext_flash_erase( FW_FLAG_ADRESS, FLASH_PAGE_SIZE );
   if(!eeprom_access) { ext_flash_close(); return FLAG_ERROR_WRITE; }

   eeprom_access = ext_flash_write( FW_FLAG_ADRESS, FLAG_SIZE, data_write);
   if(!eeprom_access) { ext_flash_close(); return FLAG_ERROR_WRITE; }

   eeprom_access = ext_flash_read(FW_FLAG_ADRESS, FLAG_SIZE, (uint8_t *)&data_read);
   if(!eeprom_access) { ext_flash_close(); return FLAG_ERROR_WRITE; }

   ext_flash_close();

   if (data_write[0] != data_read[0])
   {
      printf("FW OTA: Non-correct write flag to flash!");
      return FLAG_ERROR_WRITE;
   }

   return FLAG_OK_WRITE;
}

/*---------------------------------------------------------------------------*/

uint8_t
read_fw_flag()
{
   uint8_t page_data[ FLAG_SIZE ];

   int eeprom_access = ext_flash_open();
   if(!eeprom_access) { ext_flash_close(); return 0; }

   eeprom_access = ext_flash_read(FW_FLAG_ADRESS, FLAG_SIZE, (uint8_t *)&page_data);
   if(!eeprom_access) { ext_flash_close(); return 0; }

   ext_flash_close();

   if (page_data[0] != FW_FLAG_NON_UPDATE &&
         page_data[0] != FW_FLAG_NEW_IMG_EXT &&
         page_data[0] != FW_FLAG_NEW_IMG_INT &&
         page_data[0] != FW_FLAG_ERROR_GI_LOAD &&
         page_data[0] != FW_FLAG_NEW_IMG_INT_RST &&
         page_data[0] != FW_FLAG_PING_OK)
   {
      write_fw_flag(FW_FLAG_NON_UPDATE);
      return FW_FLAG_NON_UPDATE;
   }

   return page_data[0];
}

/*---------------------------------------------------------------------------*/


