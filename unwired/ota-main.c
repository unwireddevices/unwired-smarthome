/** @file   ota.c
 *  @brief  OTA Image R/W and Verification
 *  @author Mark Solters <msolters@gmail.com>
 *
 *  Notes:
 *  uint16_t crc16() function is Copyright Texas Instruments
 */

#include "ota-main.h"
#include "string.h"
#include "xxf_types_helper.h"
#include "dev/watchdog.h"
#include <stdbool.h>

#define DPRINT //printf("\t\t\t\t\t>ota.c:%"PRIu16"\n", __LINE__);watchdog_periodic();


uint8_t ota_images[3] = OTA_ADDRESSES;

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

/**
 */
/*******************************************************************************/

uint8_t
spi_test()
{
   int status = verify_ota_slot(1);

   if (status == -1)
   {
      return SPI_EXT_FLASH_NON_ACTIVE;
   }
   else
   {
      return SPI_EXT_FLASH_ACTIVE;
   }

}

/**
 *    OTA Metadata Functions
 */
/*******************************************************************************
 * @fn      print_metadata
 *
 * @brief   Print formatted OTA image metadata to UART.
 *
 */
void
print_metadata( OTAMetadata_t *metadata )
{
  PRINTF("Firmware Size: %"PRIX32"\n", metadata->size);DPRINT;
  PRINTF("Firmware Version: %"PRIX16"\n", metadata->version);DPRINT;
  PRINTF("Firmware UUID: %"PRIX32"\n", metadata->uuid);DPRINT;
  PRINTF("Firmware CRC: %"PRIX16"\n", metadata->crc);DPRINT;
  PRINTF("Firmware CRC shadow: %"PRIX16"\n", metadata->crc_shadow);DPRINT;

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
  FlashRead( (uint8_t *)ota_slot_metadata, (CURRENT_FIRMWARE<<12), OTA_METADATA_LENGTH );DPRINT;

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
  uint32_t ota_image_address;DPRINT;
  if ( ota_slot ) {
    //  If ota_slot >= 1, it means we're looking for an OTA download
    ota_image_address = ota_images[ (ota_slot-1) ];DPRINT;
  } else {
    //  If ota_slot = 0, we're looking for the Golden Image
    ota_image_address = GOLDEN_IMAGE;DPRINT;
  }
  ota_image_address <<= 12;DPRINT;

  int eeprom_access = ext_flash_open();DPRINT;

  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not access EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;
  }

  eeprom_access = ext_flash_read( ota_image_address, OTA_METADATA_LENGTH, (uint8_t *)ota_slot_metadata);DPRINT;

  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not read EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;
  }

  ext_flash_close();DPRINT;

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
    ota_image_address = ota_images[ (ota_slot-1) ];DPRINT;
  } else {
    //  If ota_slot = 0, we're looking for the Golden Image
    ota_image_address = GOLDEN_IMAGE;
  }
  ota_image_address <<= 12;

/*
  //  (2) Get the first page of the OTA image, which contains the metadata.
  uint8_t page_data[ FLASH_PAGE_SIZE ];DPRINT;
*/

  int eeprom_access = ext_flash_open();DPRINT;
  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not access EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;
  }

/*
  eeprom_access = ext_flash_read(ota_image_address, FLASH_PAGE_SIZE, (uint8_t *)&page_data);DPRINT;
  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not read EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;
  }


  //  (3) Overwrite the metadata section of the first page
  memcpy( page_data, (uint8_t *)ota_slot_metadata, OTA_METADATA_LENGTH );DPRINT;
*/

  //  (4) Update the ext-flash with the new page data.
  eeprom_access = ext_flash_erase( ota_image_address, FLASH_PAGE_SIZE );DPRINT;
  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not erase EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;
  }
/*
  eeprom_access = ext_flash_write( ota_image_address, FLASH_PAGE_SIZE, page_data );DPRINT;
  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not write to EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;
  }
*/

  ext_flash_close();DPRINT;

  return 0;DPRINT;
}

/*******************************************************************************
 * @fn      backup_golden_image
 *
 * @brief   Copy the current firmware into OTA slot 0 as the "Golden Image"
 *
 * @return  0 for success or error code
 */
int
backup_golden_image()
{
  PRINTF("Backup golden image to ext flash\n");
  while( erase_ota_image( 0 ) );
  uint32_t page;
  uint32_t chunk_size = 0x100;
  for ( page=0; page < 25; page++ )
  {
    uint32_t chunk = 0;
    uint8_t firmware_page[chunk_size];
    while (chunk < 0x1000)
    {
       uint32_t int_flash_address = ((CURRENT_FIRMWARE + page) << 12) + chunk;
       uint32_t ext_flash_address = ((GOLDEN_IMAGE + page) << 12) + chunk;
       PRINTF("Write GM page %"PRIu32": read offset 0x%"PRIX32", write offset 0x%"PRIX32", block 0x%"PRIX32"\n", page, int_flash_address, ext_flash_address, chunk_size);
       FlashRead(firmware_page, int_flash_address, chunk_size);
       while(store_firmware_data(ext_flash_address, firmware_page, chunk_size));
       chunk = chunk + chunk_size;
    }
  }

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
  PRINTF("Recomputing CRC16 on internal flash image within range [0x2000, 0x1B000).\n");DPRINT;
  //  (1) Determine the external flash address corresponding to the OTA slot
  uint32_t firmware_address = 0x2000 + OTA_METADATA_SPACE;DPRINT;
  uint32_t firmware_end_address = firmware_address + (current_firmware_metadata->size);DPRINT;

  //  (3) Compute the CRC16 over the entire image
  uint16_t imageCRC = 0;DPRINT;

  //  (4) Read the firmware image, one word at a time
  int idx;DPRINT;
  while (firmware_address < firmware_end_address) {
    uint8_t _word[4];

    FlashRead( _word, firmware_address, 4 );
    for (idx = 0; idx < 4; idx++)
    {
      //PRINTF("%#x ", _word[idx]);DPRINT;
      imageCRC = crc16(imageCRC, _word[idx]);
    }
    firmware_address += 4; // move 4 bytes forward
    //PRINTF("\t=>%#x\n", imageCRC);DPRINT;
  }

  //  (5) Compute two more CRC iterations using value of 0
  imageCRC = crc16(imageCRC, 0);DPRINT;
  imageCRC = crc16(imageCRC, 0);DPRINT;

  PRINTF("CRC Calculated: %#x\n", imageCRC);DPRINT;
  PRINTF("CRC in image: %#x\n", current_firmware_metadata->crc);DPRINT;

  //  (6) Update the CRC shadow with our newly calculated value
  current_firmware_metadata->crc_shadow = imageCRC;DPRINT;

  //  (4) Finally, update Metadata stored in ext-flash
  while(  FlashProgram( (uint8_t *)current_firmware_metadata, (CURRENT_FIRMWARE<<12), OTA_METADATA_LENGTH )
  != FAPI_STATUS_SUCCESS );DPRINT;

  return 0;
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
  uint32_t ota_image_address;DPRINT;
  if ( ota_slot ) {
    //  If ota_slot >= 1, it means we're looking for an OTA download
    ota_image_address = ota_images[ (ota_slot-1) ];DPRINT;
  } else {
    //  If ota_slot = 0, we're looking for the Golden Image
    ota_image_address = GOLDEN_IMAGE;DPRINT;
  }
  ota_image_address <<= 12;DPRINT;

  //  (2) Read the metadata of the corresponding OTA slot
  OTAMetadata_t ota_metadata;DPRINT;
  while( get_ota_slot_metadata( ota_slot, &ota_metadata ) );DPRINT;
  //print_metadata( &ota_metadata );DPRINT;

  //  (3) Compute the CRC16 over the entire image
  uint16_t imageCRC = 0;DPRINT;
  ota_image_address += OTA_METADATA_SPACE; // this is where the OTA binary starts
  uint32_t ota_image_end_address = ota_image_address + ota_metadata.size;DPRINT;

  int eeprom_access = ext_flash_open();DPRINT;
  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not access EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;DPRINT;
  }

  //  (4) Read the firmware image, one word at a time
  int idx;
  while (ota_image_address < ota_image_end_address) {
    uint8_t _word[4];

    eeprom_access = ext_flash_read(ota_image_address, 4, _word);
    if(!eeprom_access) {
      PRINTF("[OTA]: Error - Could not read EEPROM.(%"PRIu16")\n", __LINE__);
      ext_flash_close();
      return -1;
    }

    for (idx = 0; idx < 4; idx++)
    {
      //PRINTF("%#x ", _word[idx]);DPRINT;
      imageCRC = crc16(imageCRC, _word[idx]);
    }
    ota_image_address += 4; // move 4 bytes forward
    //PRINTF("\t=>%#x\n", imageCRC);DPRINT;
  }

  //  (5) Compute two more CRC iterations using value of 0
  imageCRC = crc16(imageCRC, 0);DPRINT;
  imageCRC = crc16(imageCRC, 0);DPRINT;

  ext_flash_close();DPRINT;

  PRINTF("CRC Calculated: %#x\n", imageCRC);DPRINT;

  //  (6) Update the CRC shadow with our newly calculated value

  if (ota_metadata.crc_shadow != imageCRC){
     return -2;
  }

  //  (4) Finally, update Metadata stored in ext-flash
  //while( overwrite_ota_slot_metadata( ota_slot, &ota_metadata ) );DPRINT;

  return 0;DPRINT;
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
  bool erased = true;DPRINT;
  uint8_t *metadata_ptr = (uint8_t *)metadata;DPRINT;
  int b = OTA_METADATA_LENGTH;DPRINT;
  while (b--) {
    //PRINTF("%u: %u, ", b, *(uint8_t *)metadata);DPRINT;
    if ( *metadata_ptr++ != 0xff ) {
      //  We encountered a non-erased byte.  There's some non-trivial data here.
      erased = false;DPRINT;
      break;DPRINT;
    }
  }

  //  If the OTA slot is erased, it's not valid!  No more work to do here.
  if (erased) {
    return false;DPRINT;
  }

  //  (2) Check the CRC entries to validate the OTA data itself.
  if ( (metadata->crc) == (metadata->crc_shadow) ) {
    return true;DPRINT;
  }

  //  If we get this far, all metadata bytes were cleared (0xff)
  return false;DPRINT;
}

/*******************************************************************************
 * @fn      find_matching_ota_slot
 *
 * @brief   Find an OTA slot containing firmware matching the supplied firmware
 *          version number.  Will only find the first matching slot.
 *
 * @return  The OTA slot index of the matching OTA slot.  Return -1 in the event
 *          of no match.
 */
int
find_matching_ota_slot( uint16_t version )
{
  int matching_slot = -1; // Assume there is no matching OTA slot.

  //  Iterate through each of the 3 OTA download slots.
  for ( int slot=1; slot<4 ; slot++ ) {

    //  (1) Get the metadata of the current OTA download slot.
    OTAMetadata_t ota_slot_metadata;DPRINT;
    while( get_ota_slot_metadata( slot, &ota_slot_metadata ) );DPRINT;

    //  (2) Is this slot empty? If yes, skip.
    if ( validate_ota_metadata( &ota_slot_metadata ) == false ) {
      continue;DPRINT;
    }

    //  (3) Does this slot's FW version match our search parameter?
    if ( ota_slot_metadata.version == version ) {
      matching_slot = slot;DPRINT;
      break;DPRINT;
    }
  }

  if ( matching_slot == -1 ) {
    PRINTF("No OTA slot matches Firmware v%i\n", version);DPRINT;
  } else {
    PRINTF("OTA slot #%i matches Firmware v%i\n", matching_slot, version);DPRINT;
  }

  return matching_slot;DPRINT;
}

/*******************************************************************************
 * @fn      find_empty_ota_slot
 *
 * @brief   Find the first empty OTA download slot.  Failing this, find the slot
 *          with the most out-of-date firmware version.
 *
 * @return  The OTA slot index of the empty/oldest OTA slot.  This will never be
 *          0 because the Golden Image should never be erased.
 */
int
find_empty_ota_slot()
{
  //  Iterate through each of the 3 OTA download slots.
  for ( int slot=1; slot<4 ; slot++ ) {

    //  (1) Get the metadata of the current OTA download slot.
    OTAMetadata_t ota_slot_metadata;DPRINT;
    while( get_ota_slot_metadata( slot, &ota_slot_metadata ) );DPRINT;

    //  (2) Is this slot invalid? If yes, let's treat it as empty.
    if ( validate_ota_metadata( &ota_slot_metadata ) == false ) {
      return slot;DPRINT;
    }
  }

  PRINTF("Could not find any empty OTA slots!\nSearching for oldest OTA slot...\n");DPRINT;
  //  If execution goes this far, no empty slot was found.  Now, we look for
  //  the oldest OTA slot instead.
  return find_oldest_ota_image();DPRINT;
}

/*******************************************************************************
 * @fn      find_oldest_ota_image
 *
 * @brief   Find the OTA slot containing the most out-of-date firmware version.
 *          OTA slots are in external flash.
 *
 * @return  The OTA slot index of the oldest firmware version.  The oldest
 *          possible OTA slot is 0, which is the Golden Image.
 */
int
find_oldest_ota_image()
{
  //  By construction, the oldest firmware should be the Golden Image, v0
  int oldest_ota_slot = 0;DPRINT;
  uint16_t oldest_firmware_version = 0;DPRINT;

  //  Iterate through each of the 3 OTA download slots.
  for ( int slot=1; slot<4; slot++ ) {
    //  (1) Get the metadata of the current OTA download slot.
    OTAMetadata_t ota_slot_metadata;DPRINT;
    while( get_ota_slot_metadata( slot, &ota_slot_metadata ) );DPRINT;

    //  (3) Is this slot populated? If not, skip.
    if ( validate_ota_metadata( &ota_slot_metadata) == false ) {
      continue;DPRINT;
    }

    //  (4) Is this the oldest non-Golden Image image we've found thus far?
    if ( oldest_firmware_version ) {
      if ( ota_slot_metadata.version < oldest_firmware_version ) {
        oldest_ota_slot = slot;DPRINT;
        oldest_firmware_version = ota_slot_metadata.version;DPRINT;
      }
    } else {
      oldest_ota_slot = slot;DPRINT;
      oldest_firmware_version = ota_slot_metadata.version;DPRINT;
    }
  }

  PRINTF("Oldest OTA slot: #%u; Firmware v%u\n", oldest_ota_slot, oldest_firmware_version);DPRINT;

  return oldest_ota_slot;DPRINT;
}

/*******************************************************************************
 * @fn      find_newest_ota_image
 *
 * @brief   Find the OTA slot containing the most recent firmware version.
 *          OTA slots are in external flash.
 *
 * @return  The OTA slot index of the newest firmware version.
 */
int
find_newest_ota_image()
{
  //  At first, we only assume knowledge of the Golden Image, v0
  int newest_ota_slot = 0;DPRINT;
  uint16_t newest_firmware_version = 0;DPRINT;

  //  Iterate through each of the 3 OTA download slots.
  for ( int slot=1; slot<4 ; slot++ ) {
    //  (1) Get the metadata of the current OTA download slot.
    OTAMetadata_t ota_slot_metadata;DPRINT;
    while( get_ota_slot_metadata( slot, &ota_slot_metadata ) );DPRINT;

    //  (2) Is this slot populated? If not, skip.
    if ( validate_ota_metadata( &ota_slot_metadata) == false ) {
      continue;DPRINT;
    }

    //  (3) Is this the newest non-Golden Image image we've found thus far?
    if ( ota_slot_metadata.version > newest_firmware_version ) {
      newest_ota_slot = slot;DPRINT;
      newest_firmware_version = ota_slot_metadata.version;DPRINT;
    }
  }

  PRINTF("Newest OTA slot: #%u; Firmware v%u\n", newest_ota_slot, newest_firmware_version);DPRINT;
  return newest_ota_slot;DPRINT;
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
  bool eeprom_access = ext_flash_open();DPRINT;

  if(eeprom_access == false) {
    ext_flash_close();DPRINT;
    return -1;DPRINT;
  }
  //PRINTF("[OTA]: Erasing flash (0x%"PRIX32",0x%"PRIX16")\n", ext_address, FLASH_PAGE_SIZE);DPRINT;
  eeprom_access = ext_flash_erase( ext_address, FLASH_PAGE_SIZE );DPRINT;
  //PRINTF("[OTA]: Erased flash at (0x%"PRIX32",0x%"PRIX16")\n", ext_address, FLASH_PAGE_SIZE);DPRINT;

  if(eeprom_access == false) {
    PRINTF("[OTA]: Error - Could not erase EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;DPRINT;
  }

  ext_flash_close();

  return 0;DPRINT;
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
  uint8_t ota_image_base_address;DPRINT;
  if ( ota_slot ) {
    ota_image_base_address = ota_images[ (ota_slot-1) ];DPRINT;
  } else {
    ota_image_base_address = GOLDEN_IMAGE;DPRINT;
  }
  PRINTF("[OTA]: Erasing OTA slot %u [%#x, %#x)...\n", ota_slot, (ota_image_base_address<<12), ((ota_image_base_address+25)<<12));DPRINT;

  //  (2) Erase each page in the OTA download slot!
  for (uint32_t page=0; page<25; page++) {
    PRINTF("[OTA]: Erasing page %"PRIu32" at 0x%"PRIX32"..\n", page, (( ota_image_base_address + page ) << 12));DPRINT;
    while( erase_extflash_page( (( ota_image_base_address + page ) << 12) ) );DPRINT;
    watchdog_periodic();DPRINT;
  }

  return 0;DPRINT;
}

static int
update_firmware_page( uint32_t source_page, uint32_t destination_page )
{
  uint8_t page_data[ FLASH_PAGE_SIZE ];DPRINT;

  //  Erase internal flash page
  while(  FlashSectorErase( destination_page ) !=
          FAPI_STATUS_SUCCESS );DPRINT;

  //  Read one page from the new external flash image
  int eeprom_access = ext_flash_open();DPRINT;

  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not access EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;DPRINT;
  }

  eeprom_access = ext_flash_read( source_page, FLASH_PAGE_SIZE, (uint8_t *)&page_data);DPRINT;

  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not read EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;DPRINT;
  }

  ext_flash_close();DPRINT;

  while(  FlashProgram( page_data, destination_page, FLASH_PAGE_SIZE )
          != FAPI_STATUS_SUCCESS );DPRINT;

  return 0;DPRINT;
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
  uint32_t ota_image_address;DPRINT;
  if ( ota_slot ) {
    //  If ota_slot >= 1, it means we want to copy over an OTA download
    ota_image_address = ota_images[ (ota_slot-1) ];DPRINT;
  } else {
    //  If ota_slot = 0, it means we want to copy over the Golden Image
    ota_image_address = GOLDEN_IMAGE;DPRINT;
  }
  ota_image_address <<= 12;DPRINT;

  //  (2) Get metadata about the new version
  //OTAMetadata_t new_firmware;DPRINT;
  //FlashRead( (uint8_t *)&new_firmware, ota_image_address, OTA_METADATA_LENGTH );DPRINT;

  //  (2) Validate the new firmware (CRC)
  //  return -1 if not valid!


  //  (3) Erase internal user pages from 0x02000 to 0x27000
  //      Overwrite them with the corresponding image pages from
  //      external flash.
  //  Each firmware image is 25 pages big at most
  for (uint8_t sector_num=0; sector_num<25; sector_num++) {
    while( update_firmware_page( (ota_image_address + (sector_num << 12)), ((sector_num+CURRENT_FIRMWARE) << 12) ) );DPRINT;
  }

  //  (4) Reboot


  return 0;DPRINT;
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
  int eeprom_access;DPRINT;
  eeprom_access = ext_flash_open();DPRINT;

  if(!eeprom_access) {
    PRINTF("[OTA]: Error - could not access EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;DPRINT;
  }

  //  (2) Copy page_data into external flash chip.
  eeprom_access = ext_flash_write( ext_address, data_length, data );DPRINT;
  watchdog_periodic();

  if(!eeprom_access) {
    PRINTF("[OTA]: Error - Could not write to EEPROM.(%"PRIu16")\n", __LINE__);DPRINT;
    ext_flash_close();DPRINT;
    return -1;DPRINT;
  }

  ext_flash_close();DPRINT;

  //PRINTF("[OTA]: Firmware data successfully written to 0x%"PRIX32".\n", ext_address);DPRINT;
  return 0;DPRINT;
}
