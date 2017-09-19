
/** @file   ota.c
 *  @brief  OTA Image R/W and Verification
 *  @author Mark Solters <msolters@gmail.com>
 *
 *  Notes:
 *  uint16_t crc16() function is Copyright Texas Instruments
 */

#include "ota-bootloader.h"
#include "string.h"
#include <stdbool.h>


uint8_t ota_images[3] = OTA_ADDRESSES;

/*---------------------------------------------------------------------------*/

void
print_uart(char *str)
{
   for (uint8_t i=0; str[i] != '\0'; i++)
   {
      ti_lib_uart_char_put(UART0_BASE, str[i]);
   }
}

/*---------------------------------------------------------------------------*/

void
print_uart_bl(char *str)
{
   print_uart("Bootloader:\t ");
   print_uart(str);
}

/*---------------------------------------------------------------------------*/

void
print_uart_byte(uint8_t byte)
{
   ti_lib_uart_char_put(UART0_BASE, byte);
}


/*---------------------------------------------------------------------------*/


/**
 *    OTA Flash-specific Functions
 */
/*******************************************************************************
 * @fn      FlashRead
 *
 * @brief   Read data from the CC26xx Internal Flash.
 *
 * @param   pui8DataBuffer  - Pointer to uint8_t array to store flash data in.
 *
 * @param   ui32Address     - Flash address to begin reading from.
 *
 * @param   ui32Count       - Number of bytes to read from flash, starting at
 *                            ui32Address.
 *
 */
void
FlashRead(uint8_t *pui8DataBuffer, uint32_t ui32Address, uint32_t ui32Count)
{
  uint8_t *pui8ReadAddress = (uint8_t *)ui32Address;
  while (ui32Count--) {
    *pui8DataBuffer++ = *pui8ReadAddress++;
  }
}

/*********************************************************************

 * @fn          crc16
 *
 * @brief       Run the CRC16 Polynomial calculation over the byte parameter.
 *
 * @param       crc - Running CRC calculated so far.
 * @param       val - Value on which to run the CRC16.
 *
 * @return      crc - Updated for the run.
 */
uint16_t
crc16(uint16_t crc, uint8_t val)
{
  const uint16_t poly = 0x1021;
  uint8_t cnt;

  for (cnt = 0; cnt < 8; cnt++, val <<= 1)
  {
    uint8_t msb = (crc & 0x8000) ? 1 : 0;

    crc <<= 1;

    if (val & 0x80)
    {
      crc |= 0x0001;
    }

    if (msb)
    {
      crc ^= poly;
    }
  }

  return crc;
}


/*******************************************************************************
 * @fn      get_current_metadata
 *
 * @brief   Get the metadata belonging of whatever firmware is currently
 *          installed to the internal flash.
 *
 * @return  OTAMetadata_t object read from the current firmware
 */
int
get_current_metadata( OTAMetadata_t *ota_slot_metadata  )
{
  FlashRead( (uint8_t *)ota_slot_metadata, (CURRENT_FIRMWARE<<12), OTA_METADATA_LENGTH );

  return 0;
}

/*******************************************************************************
 * @fn      get_ota_slot_metadata
 *
 * @brief   Get the metadata belonging to an OTA slot in external flash.
 *
 * @param   ota_slot    - The OTA slot to be read for metadata.
 *
 * @param   *metadata   - Pointer to the OTAMetadata_t struct where the metadata
 *                        is to be written.
 *
 * @return  0 on success or error code
 */
int
get_ota_slot_metadata( uint8_t ota_slot, OTAMetadata_t *ota_slot_metadata )
{
  //  (1) Determine the external flash address corresponding to the OTA slot
  uint32_t ota_image_address;
  if ( ota_slot ) {
    //  If ota_slot >= 1, it means we're looking for an OTA download
    ota_image_address = ota_images[ (ota_slot-1) ];
  } else {
    //  If ota_slot = 0, we're looking for the Golden Image
    ota_image_address = GOLDEN_IMAGE;
  }
  ota_image_address <<= 12;

  int eeprom_access = ext_flash_open();

  if(!eeprom_access) {
    ext_flash_close();
    return -1;
  }

  eeprom_access = ext_flash_read( ota_image_address, OTA_METADATA_LENGTH, (uint8_t *)ota_slot_metadata);

  if(!eeprom_access) {
    ext_flash_close();
    return -1;
  }

  ext_flash_close();

  return 0;
}


/*******************************************************************************
 * @fn      overwrite_ota_slot_metadata
 *
 * @brief   Write new metadata to a specific OTA slot in external flash.
 *
 * @param   ota_slot             - The OTA slot to be modified.
 *
 * @param   *ota_slot_metadata   - Pointer to the new OTAMetadata_t data.
 *
 * @return  0 on success or error code
 */
int
overwrite_ota_slot_metadata( uint8_t ota_slot, OTAMetadata_t *ota_slot_metadata )
{
  //  (1) Determine the external flash address corresponding to the OTA slot
  uint32_t ota_image_address;
  if ( ota_slot ) {
    //  If ota_slot >= 1, it means we're looking for an OTA download
    ota_image_address = ota_images[ (ota_slot-1) ];
  } else {
    //  If ota_slot = 0, we're looking for the Golden Image
    ota_image_address = GOLDEN_IMAGE;
  }
  ota_image_address <<= 12;

/*
  //  (2) Get the first page of the OTA image, which contains the metadata.
  uint8_t page_data[ FLASH_PAGE_SIZE ];
*/

  int eeprom_access = ext_flash_open();
  if(!eeprom_access) {
    ext_flash_close();
    return -1;
  }

/*
  eeprom_access = ext_flash_read(ota_image_address, FLASH_PAGE_SIZE, (uint8_t *)&page_data);
  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not read EEPROM.(%"PRIu16")\n", __LINE__);
    ext_flash_close();
    return -1;
  }


  //  (3) Overwrite the metadata section of the first page
  memcpy( page_data, (uint8_t *)ota_slot_metadata, OTA_METADATA_LENGTH );
*/

  //  (4) Update the ext-flash with the new page data.
  eeprom_access = ext_flash_erase( ota_image_address, FLASH_PAGE_SIZE );
  if(!eeprom_access) {
    ext_flash_close();
    return -1;
  }
/*
  eeprom_access = ext_flash_write( ota_image_address, FLASH_PAGE_SIZE, page_data );
  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not write to EEPROM.(%"PRIu16")\n", __LINE__);
    ext_flash_close();
    return -1;
  }
*/

  ext_flash_close();

  return 0;
}


/*******************************************************************************
 * @fn      verify_current_firmware
 *
 * @brief   Rerun the CRC16 algorithm over the contents of internal flash.
 *
 * @return  0 for success or error code
 */
int
verify_current_firmware( OTAMetadata_t *current_firmware_metadata )
{
  //  (1) Determine the external flash address corresponding to the OTA slot
  uint32_t firmware_address = 0x2000 + OTA_METADATA_SPACE;
  uint32_t firmware_end_address = firmware_address + (current_firmware_metadata->size);

  //  (3) Compute the CRC16 over the entire image
  uint16_t imageCRC = 0;

  //  (4) Read the firmware image, one word at a time
  int idx;
  while (firmware_address < firmware_end_address) {
    uint8_t _word[4];

    FlashRead( _word, firmware_address, 4 );
    for (idx = 0; idx < 4; idx++)
    {
      //PRINTF("%#x ", _word[idx]);
      imageCRC = crc16(imageCRC, _word[idx]);
    }
    firmware_address += 4; // move 4 bytes forward
    //PRINTF("\t=>%#x\n", imageCRC);
  }

  //  (5) Compute two more CRC iterations using value of 0
  imageCRC = crc16(imageCRC, 0);
  imageCRC = crc16(imageCRC, 0);

  //  (6) Update the CRC shadow with our newly calculated value
  current_firmware_metadata->crc_shadow = imageCRC;

  //  (4) Finally, update Metadata stored in ext-flash
  while(  FlashProgram( (uint8_t *)current_firmware_metadata, (CURRENT_FIRMWARE<<12), OTA_METADATA_LENGTH )
  != FAPI_STATUS_SUCCESS );

  if (current_firmware_metadata->crc_shadow == current_firmware_metadata->crc)
  {
     return CORRECT_CRC;
  }

  return NON_CORRECT_CRC;
}

/*******************************************************************************
 * @fn      verify_ota_slot
 *
 * @brief   Given an OTA slot, verify the firmware content against the metadata.
 *          If everything is fine, update the metadata to indicate this OTA slot
 *          is valid.
 *
 * @param   ota_slot  - OTA slot index to verify. (1-3)
 *
 * @return  0 for success or error code
 */
int
verify_ota_slot( uint8_t ota_slot )
{
  //  (1) Determine the external flash address corresponding to the OTA slot
  uint32_t ota_image_address;
  if ( ota_slot ) {
    //  If ota_slot >= 1, it means we're looking for an OTA download
    ota_image_address = ota_images[ (ota_slot-1) ];
  } else {
    //  If ota_slot = 0, we're looking for the Golden Image
    ota_image_address = GOLDEN_IMAGE;
  }
  ota_image_address <<= 12;

  //  (2) Read the metadata of the corresponding OTA slot
  OTAMetadata_t ota_metadata;
  while( get_ota_slot_metadata( ota_slot, &ota_metadata ) );
  //print_metadata( &ota_metadata );

  //  (3) Compute the CRC16 over the entire image
  uint16_t imageCRC = 0;
  ota_image_address += OTA_METADATA_SPACE; // this is where the OTA binary starts
  uint32_t ota_image_end_address = ota_image_address + ota_metadata.size;

  int eeprom_access = ext_flash_open();
  if(!eeprom_access) {
    ext_flash_close();
    return NON_READ_FLASH;
  }

  //  (4) Read the firmware image, one word at a time
  int idx;
  while (ota_image_address < ota_image_end_address) {
    uint8_t _word[4];

    eeprom_access = ext_flash_read(ota_image_address, 4, _word);
    if(!eeprom_access) {
      ext_flash_close();
      return NON_READ_FLASH;
    }

    for (idx = 0; idx < 4; idx++)
    {
      //PRINTF("%#x ", _word[idx]);
      imageCRC = crc16(imageCRC, _word[idx]);
    }
    ota_image_address += 4; // move 4 bytes forward
    //PRINTF("\t=>%#x\n", imageCRC);
  }

  //  (5) Compute two more CRC iterations using value of 0
  imageCRC = crc16(imageCRC, 0);
  imageCRC = crc16(imageCRC, 0);

  ext_flash_close();

  //  (6) Update the CRC shadow with our newly calculated value

  if (ota_metadata.crc_shadow != imageCRC){
     return NON_CORRECT_CRC;
  }

  //  (4) Finally, update Metadata stored in ext-flash
  //while( overwrite_ota_slot_metadata( ota_slot, &ota_metadata ) );

  return CORRECT_CRC;
}

/*******************************************************************************
 * @fn      validate_ota_metadata
 *
 * @brief   Returns true only if the metadata provided indicates the OTA slot
 *          is populated and valid.
 *
 * @return  True if the OTA slot is populated and valid.  Otherwise, false.
 */
bool
validate_ota_metadata( OTAMetadata_t *metadata )
{
  //  (1) Is the OTA slot erased?
  //      First, we check to see if every byte in the metadata is 0xFF.
  //      If this is the case, this metadata is "erased" and therefore we assume
  //      the OTA slot to be empty.
  bool erased = true;
  uint8_t *metadata_ptr = (uint8_t *)metadata;
  int b = OTA_METADATA_LENGTH;
  while (b--) {
    //PRINTF("%u: %u, ", b, *(uint8_t *)metadata);
    if ( *metadata_ptr++ != 0xff ) {
      //  We encountered a non-erased byte.  There's some non-trivial data here.
      erased = false;
      break;
    }
  }

  //  If the OTA slot is erased, it's not valid!  No more work to do here.
  if (erased) {
    return false;
  }

  //  (2) Check the CRC entries to validate the OTA data itself.
  if ( (metadata->crc) == (metadata->crc_shadow) ) {
    return true;
  }

  //  If we get this far, all metadata bytes were cleared (0xff)
  return false;
}


/*******************************************************************************
 * @fn      erase_extflash_page
 *
 * @brief   Erase a page of external flash, starting at the given address.
 *
 * @param   ext_address    - The external flash address representing the start
 *                           of the page to be erased.
 *
 * @return  0 or error code
 */
int
erase_extflash_page( uint32_t ext_address )
{
  //  (1) Open the external flash
  bool eeprom_access = ext_flash_open();

  if(eeprom_access == false) {
    ext_flash_close();
    return -1;
  }
  //PRINTF("[OTA]: Erasing flash (0x%"PRIX32",0x%"PRIX16")\n", ext_address, FLASH_PAGE_SIZE);
  eeprom_access = ext_flash_erase( ext_address, FLASH_PAGE_SIZE );
  //PRINTF("[OTA]: Erased flash at (0x%"PRIX32",0x%"PRIX16")\n", ext_address, FLASH_PAGE_SIZE);

  if(eeprom_access == false) {
    ext_flash_close();
    return -1;
  }

  ext_flash_close();

  return 0;
}

/*******************************************************************************
 * @fn      erase_ota_image
 *
 * @brief   Clear an OTA slot in external flash.
 *
 * @param   ota_slot    - The OTA slot index of the firmware image to be copied.
 *                        You can erase the Golden Image (0), but be extra
 *                        careful your application could never accidentally
 *                        do this!
 *
 * @return  0 or error code
 */
int
erase_ota_image( uint8_t ota_slot )
{
  //  (1) Get page address of the ota_slot in ext-flash
  uint8_t ota_image_base_address;
  if ( ota_slot ) {
    ota_image_base_address = ota_images[ (ota_slot-1) ];
  } else {
    ota_image_base_address = GOLDEN_IMAGE;
  }

  //  (2) Erase each page in the OTA download slot!
  for (uint32_t page=0; page<25; page++) {
    while( erase_extflash_page( (( ota_image_base_address + page ) << 12) ) );
  }

  return 0;
}

static int
update_firmware_page( uint32_t source_page, uint32_t destination_page )
{
  uint8_t page_data[ FLASH_PAGE_SIZE ];

  //  Erase internal flash page
  while(  FlashSectorErase( destination_page ) !=
          FAPI_STATUS_SUCCESS );

  //  Read one page from the new external flash image
  int eeprom_access = ext_flash_open();

  if(!eeprom_access) {
    ext_flash_close();
    return -1;
  }

  eeprom_access = ext_flash_read( source_page, FLASH_PAGE_SIZE, (uint8_t *)&page_data);

  if(!eeprom_access) {
    ext_flash_close();
    return -1;
  }

  ext_flash_close();

  while(  FlashProgram( page_data, destination_page, FLASH_PAGE_SIZE )
          != FAPI_STATUS_SUCCESS );

  return 0;
}

/*******************************************************************************
 * @fn      update_firmware
 *
 * @brief   Overwrite firmware located in internal flash with the firmware
 *          stored in an external flash OTA slot.
 *
 * @param   ota_slot    - The OTA slot index of the firmware image to be copied.
 *                          0 = "Golden Image" backup, aka factory restore
 *                          1, 2, 3 = OTA Download slots
 *
 * @return  0 or error code
 */
int
update_firmware( uint8_t ota_slot )
{
  //  (1) Determine the external flash address corresponding to the OTA slot
  uint32_t ota_image_address;
  if ( ota_slot ) {
    //  If ota_slot >= 1, it means we want to copy over an OTA download
    ota_image_address = ota_images[ (ota_slot-1) ];
  } else {
    //  If ota_slot = 0, it means we want to copy over the Golden Image
    ota_image_address = GOLDEN_IMAGE;
  }
  ota_image_address <<= 12;

  //  (2) Get metadata about the new version
  //OTAMetadata_t new_firmware;
  //FlashRead( (uint8_t *)&new_firmware, ota_image_address, OTA_METADATA_LENGTH );

  //  (2) Validate the new firmware (CRC)
  //  return -1 if not valid!


  //  (3) Erase internal user pages from 0x02000 to 0x27000
  //      Overwrite them with the corresponding image pages from
  //      external flash.
  //  Each firmware image is 25 pages big at most
  print_uart("Bootloader:\t Flash internal memory: ");
  for (uint8_t sector_num=0; sector_num<25; sector_num++) {
    while( update_firmware_page( (ota_image_address + (sector_num << 12)), ((sector_num+CURRENT_FIRMWARE) << 12) ) );
    print_uart(".");
  }
  print_uart("\n");

  //  (4) Reboot


  return 0;
}

/*******************************************************************************
 * @fn      store_firmware_data
 *
 * @brief   Store firmware data in external flash at the specified
 *          address.
 *
 * @param   ext_address   - External flash address to begin writing data.
 *
 * @param   data          - Pointer to the data buffer to be written.
 *                          Note: page_data can be larger than 4096 bytes, but
 *                          only the first 4096 bytes will be written!
 *
 * @return  0 or error code
 */
int
store_firmware_data( uint32_t ext_address, uint8_t *data, size_t data_length )
{
  //  (1) Erase external flash page first!
  int eeprom_access;
  eeprom_access = ext_flash_open();

  if(!eeprom_access) {
    ext_flash_close();
    return -1;
  }

  //  (2) Copy page_data into external flash chip.
  eeprom_access = ext_flash_write( ext_address, data_length, data );

  if(!eeprom_access) {
    ext_flash_close();
    return -1;
  }

  ext_flash_close();

  return 0;
}


/*******************************************************************************
 * @fn      jump_to_image
 *
 * @brief   Begin executing another firmware binary located in internal flash.
 *
 * @param   destination_address - Internal flash address of the vector table for
 *                                the firmware binary that is to be booted into.
 *                                Since this OTA lib prepends metadata to each
 *                                binary, the true VTOR start address will be
 *                                OTA_METADATA_SPACE bytes past this address.
 *
 */
void
jump_to_image(uint32_t destination_address)
{
  if ( destination_address ) {
      //Only add the metadata length offset if destination_address is NOT 0!
      //(Jumping to 0x0 is used to reboot the device)
    destination_address += OTA_METADATA_SPACE;
  }
  destination_address += OTA_RESET_VECTOR;
  __asm("LDR R0, [%[dest]]"::[dest]"r"(destination_address)); //  Load the destination address
  __asm("ORR R0, #1");                                        //  Make sure the Thumb State bit is set.
  __asm("BX R0");                                             //  Branch execution
}
