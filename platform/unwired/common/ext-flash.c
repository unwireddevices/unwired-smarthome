/*
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*---------------------------------------------------------------------------*/
/**
 * \addtogroup sensortag-cc26xx-ext-flash
 * @{
 *
 * \file
 *  Driver for the LaunchPad Flash and the Sensortag WinBond W25X20CL/W25X40CL
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "ext-flash.h"
#include "ti-lib.h"
#include "board-spi.h"
#include "dev/watchdog.h"


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "xxf_types_helper.h"

/*---------------------------------------------------------------------------*/
/* Instruction codes */
#define BLS_CODE_WRITE_ENABLE     0x06 /**< Write Enable */
#define BLS_CODE_WRITE_DISABLE    0x04 /**< Write Disable */
#define BLS_CODE_MDID             0x9F /**< Manufacturer Device ID */
#define BLS_CODE_READ_STATUS      0x05 /**< Read Status Register */
#define BLS_CODE_WRITE_STATUS     0x01 /**< Read Status Register */
#define BLS_CODE_READ             0x03 /**< Read Data */
#define BLS_CODE_FAST_READ        0x0B /**< Read Data */
#define BLS_CODE_PROGRAM          0x02 /**< Page Program */
#define BLS_CODE_SECTOR_ERASE     0xD8 /**< Sector Erase */
#define BLS_CODE_BULK_ERASE       BLS_CODE_ERASE_ALL /**< ALL Erase */
#define BLS_CODE_POWERDOWN        0xB9 /**< Power down */
#define BLS_CODE_RPD              0xAB /**< Release Power-Down */
/*---------------------------------------------------------------------------*/
/* Erase instructions */

#define BLS_CODE_ERASE_512K       0xD8
#define BLS_CODE_ERASE_ALL        0xC7 /**< Mass Erase */
/*---------------------------------------------------------------------------*/
/* Bitmasks of the status register */

#define BLS_STATUS_SRWD_BM        0x80
#define BLS_STATUS_BP_BM          0x0C
#define BLS_STATUS_WEL_BM         0x02
#define BLS_STATUS_WIP_BM         0x01

#define BLS_STATUS_BIT_BUSY       0x01 /**< Busy bit of the status register */
/*---------------------------------------------------------------------------*/
/* Part specific constants */
#define BLS_NUMONYX_MID           0x20
#define BLS_DEVICE_ID_NU_MP25P40  0x20
#define BLS_NU_4MBIT              0x13

#define BLS_PROGRAM_PAGE_SIZE      256
#define BLS_ERASE_SECTOR_SIZE     4096
/*---------------------------------------------------------------------------*/
#define VERIFY_PART_ERROR           -1
#define VERIFY_PART_POWERED_DOWN     0
#define VERIFY_PART_OK               1

#define DPRINT //printf(">ext-flash.c:%"PRIu16"\n", __LINE__);watchdog_periodic();

/*---------------------------------------------------------------------------*/
/**
 * Clear external flash CSN line
 */
static void
select_on_bus(void)
{
  ti_lib_gpio_clear_dio(BOARD_IOID_FLASH_CS);DPRINT;
}
/*---------------------------------------------------------------------------*/
/**
 * Set external flash CSN line
 */
static void
deselect(void)
{
  DPRINT;
  ti_lib_gpio_set_dio(BOARD_IOID_FLASH_CS);
  DPRINT;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Wait till previous erase/program operation completes.
 * \return True when successful.
 */
static bool
wait_ready(void)
{
  bool ret;DPRINT;
  const uint8_t wbuf[1] = { BLS_CODE_READ_STATUS };DPRINT;

  select_on_bus();DPRINT;

  /* Throw away all garbages */
  board_spi_flush();DPRINT;

  ret = board_spi_write(wbuf, sizeof(wbuf));DPRINT;

  if(ret == false) {
    deselect();DPRINT;
    return false;
  }

  for(;;) {
    uint8_t buf;
    /* Note that this temporary implementation is not
     * energy efficient.
     * Thread could have yielded while waiting for flash
     * erase/program to complete.
     */
    DPRINT;
    ret = board_spi_read(&buf, sizeof(buf));
    DPRINT;

    if(ret == false) {
      /* Error */
       DPRINT;
      deselect();
      DPRINT;
      return false;
    }
    if(!(buf & BLS_STATUS_BIT_BUSY)) {
      /* Now ready */
       DPRINT;
      break;
    }
    DPRINT;
  }
  DPRINT;
  deselect();DPRINT;
  return true;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Verify the flash part.
 * \retval VERIFY_PART_OK The part was identified successfully
 * \retval VERIFY_PART_ERROR There was an error communicating with the part
 * \retval VERIFY_PART_POWERED_DOWN Communication was successful, but the part
 *         was powered down
 */
static uint8_t
verify_part(void)
{
  const uint8_t wbuf[] = { BLS_CODE_MDID };DPRINT;
  uint8_t rbuf[3] = { 0x42, 0x42, 0x42 };DPRINT;
  bool ret;DPRINT;

  select_on_bus();DPRINT;

  ret = board_spi_write(wbuf, sizeof(wbuf));DPRINT;

  if(ret == false) {
    deselect();DPRINT;
    return VERIFY_PART_ERROR;DPRINT;
  }

  ret = board_spi_read(rbuf, 3);DPRINT;
  deselect();DPRINT;

  if(ret == false) {
    return VERIFY_PART_ERROR;DPRINT;
  }

  //printf("Extflash return: %"PRIXX8", %"PRIXX8", %"PRIXX8"(expected %"PRIXX8", %"PRIXX8", %"PRIXX8")\n",
  //       rbuf[0], rbuf[1], rbuf[2], BLS_NUMONYX_MID, BLS_DEVICE_ID_NU_MP25P40, BLS_NU_4MBIT);DPRINT;

  if((rbuf[0] != BLS_NUMONYX_MID) || (rbuf[1] != BLS_DEVICE_ID_NU_MP25P40) || (rbuf[2] != BLS_NU_4MBIT))
  {
    return VERIFY_PART_POWERED_DOWN;DPRINT;
  }
  return VERIFY_PART_OK;DPRINT;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Put the device in power save mode. No access to data; only
 *        the status register is accessible.
 */
static void
power_down(void)
{
  uint8_t cmd;DPRINT;
  uint8_t i;DPRINT;

  /* First, wait for the device to be ready */
  if(wait_ready() == false) {
    /* Entering here will leave the device in standby instead of powerdown */
    return;DPRINT;
  }

  cmd = BLS_CODE_POWERDOWN;DPRINT;
  select_on_bus();DPRINT;
  board_spi_write(&cmd, sizeof(cmd));DPRINT;
  deselect();DPRINT;

  i = 0;DPRINT;
  while(i < 10) {
    if(verify_part() == VERIFY_PART_POWERED_DOWN) {
      /* Device is powered down */
      return;DPRINT;
    }
    i++;DPRINT;
  }

  /* Should not be required */
  deselect();DPRINT;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief    Take device out of power save mode and prepare it for normal operation
 * \return   True if the command was written successfully
 */
static bool
power_standby(void)
{
  uint8_t cmd;DPRINT;
  bool success;DPRINT;

  cmd = BLS_CODE_RPD;DPRINT;
  select_on_bus();DPRINT;
  success = board_spi_write(&cmd, sizeof(cmd));DPRINT;

  if(success) {
    success = wait_ready() == true ? true : false;DPRINT;
  }

  deselect();DPRINT;

  return success;DPRINT;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief Enable write.
 * \return True when successful.
 */
static bool
write_enable(void)
{
  bool ret;DPRINT;
  const uint8_t wbuf[] = { BLS_CODE_WRITE_ENABLE };DPRINT;

  select_on_bus();DPRINT;
  ret = board_spi_write(wbuf, sizeof(wbuf));DPRINT;
  deselect();DPRINT;

  if(ret == false) {
    return false;DPRINT;
  }
  return true;DPRINT;
}
/*---------------------------------------------------------------------------*/
bool
ext_flash_open()
{
  board_spi_open(4000000, BOARD_IOID_SPI_CLK_FLASH);DPRINT;

  /* GPIO pin configuration */
  ti_lib_ioc_pin_type_gpio_output(BOARD_IOID_FLASH_CS);DPRINT;

  /* Default output to clear chip select */
  deselect();DPRINT;

  /* Put the part is standby mode */
  power_standby();DPRINT;

  return verify_part() == VERIFY_PART_OK ? true : false;DPRINT;
}
/*---------------------------------------------------------------------------*/
void
ext_flash_close()
{
  /* Put the part in low power mode */
  power_down();DPRINT;

  board_spi_close();DPRINT;
}
/*---------------------------------------------------------------------------*/
bool
ext_flash_read(size_t offset, size_t length, uint8_t *buf)
{
  uint8_t wbuf[4];DPRINT;
  bool ret;DPRINT;
  /* Wait till previous erase/program operation completes */
  //bool ret = wait_ready();DPRINT;
  //if(ret == false) {
  //  return false;DPRINT;
  //}

  /*
   * SPI is driven with very low frequency (1MHz < 33MHz fR spec)
   * in this implementation, hence it is not necessary to use fast read.
   */
  wbuf[0] = BLS_CODE_READ;DPRINT;
  wbuf[1] = (offset >> 16) & 0xff;DPRINT;
  wbuf[2] = (offset >> 8) & 0xff;DPRINT;
  wbuf[3] = offset & 0xff;DPRINT;

  select_on_bus();DPRINT;

  if(board_spi_write(wbuf, sizeof(wbuf)) == false) {
    /* failure */
    deselect();DPRINT;
    return false;DPRINT;
  }

  ret = board_spi_read(buf, length);DPRINT;

  deselect();DPRINT;

  return ret;DPRINT;
}
/*---------------------------------------------------------------------------*/
bool
ext_flash_write(size_t offset, size_t length, const uint8_t *buf)
{
  uint8_t wbuf[4];DPRINT;
  bool ret;DPRINT;
  size_t ilen; /* interim length per instruction */

  while(length > 0) {
    /* Wait till previous erase/program operation completes */
    ret = wait_ready();DPRINT;
    if(ret == false) {
      return false;DPRINT;
    }

    ret = write_enable();DPRINT;
    if(ret == false) {
      return false;DPRINT;
    }

    ilen = BLS_PROGRAM_PAGE_SIZE - (offset % BLS_PROGRAM_PAGE_SIZE);DPRINT;
    if(length < ilen) {
      ilen = length;DPRINT;
    }

    wbuf[0] = BLS_CODE_PROGRAM;DPRINT;
    wbuf[1] = (offset >> 16) & 0xff;DPRINT;
    wbuf[2] = (offset >> 8) & 0xff;DPRINT;
    wbuf[3] = offset & 0xff;DPRINT;

    offset += ilen;DPRINT;
    length -= ilen;DPRINT;

    /* Upto 100ns CS hold time (which is not clear
     * whether it's application only inbetween reads)
     * is not imposed here since above instructions
     * should be enough to delay
     * as much. */
    select_on_bus();DPRINT;

    if(board_spi_write(wbuf, sizeof(wbuf)) == false) {
      /* failure */
      deselect();DPRINT;
      return false;DPRINT;
    }

    if(board_spi_write(buf, ilen) == false) {
      /* failure */
      deselect();DPRINT;
      return false;DPRINT;
    }
    buf += ilen;DPRINT;
    deselect();DPRINT;
  }

  return true;DPRINT;
}
/*---------------------------------------------------------------------------*/
bool
ext_flash_erase(size_t offset, size_t length)
{
  /*
   * Note that Block erase might be more efficient when the floor map
   * is well planned for OTA, but to simplify this implementation,
   * sector erase is used blindly.
   */
  uint8_t wbuf[4];DPRINT;
  bool ret;DPRINT;
  size_t i, numsectors;DPRINT;
  size_t endoffset = offset + length - 1;DPRINT;

  offset = (offset / BLS_ERASE_SECTOR_SIZE) * BLS_ERASE_SECTOR_SIZE;DPRINT;
  numsectors = (endoffset - offset + BLS_ERASE_SECTOR_SIZE - 1) / BLS_ERASE_SECTOR_SIZE;DPRINT;

  wbuf[0] = BLS_CODE_SECTOR_ERASE;DPRINT;

  for(i = 0; i < numsectors; i++) {
    /* Wait till previous erase/program operation completes */
    ret = wait_ready();DPRINT;
    if(ret == false) {
       printf("[EXTFLASH]: Ext flash error(%"PRIu16")\n", __LINE__);DPRINT;
      return false;DPRINT;
    }

    ret = write_enable();DPRINT;
    if(ret == false) {
       printf("[EXTFLASH]: Ext flash error(%"PRIu16")\n", __LINE__);DPRINT;
      return false;DPRINT;
    }

    wbuf[1] = (offset >> 16) & 0xff;DPRINT;
    wbuf[2] = (offset >> 8) & 0xff;DPRINT;
    wbuf[3] = offset & 0xff;DPRINT;

    select_on_bus();DPRINT;

    if(board_spi_write(wbuf, sizeof(wbuf)) == false) {
      /* failure */
      deselect();DPRINT;
      printf("[EXTFLASH]: Ext flash error(%"PRIu16")\n", __LINE__);DPRINT;
      return false;DPRINT;
    }
    deselect();DPRINT;

    offset += BLS_ERASE_SECTOR_SIZE;DPRINT;
  }

  //printf("[EXTFLASH]: Ext flash true(%"PRIu16")\n", __LINE__);DPRINT;
  return true;DPRINT;
}
/*---------------------------------------------------------------------------*/
bool
ext_flash_test(void)
{
  bool ret;DPRINT;

  ret = ext_flash_open();DPRINT;
  ext_flash_close();DPRINT;

  return ret;DPRINT;
}
/*---------------------------------------------------------------------------*/
void
ext_flash_probe(void)
{
   uint8_t flash_data[4] = {0x00, 0x00, 0x00, 0x00};DPRINT;
   int eeprom_access;DPRINT;

   ext_flash_open();DPRINT;

   eeprom_access = ext_flash_read(0x00, 4, flash_data);DPRINT;
   if(!eeprom_access) {
    printf("[EXTFLASH]: Error - Could not read EEPROM\n");DPRINT;
   }
   else
   {
      printf("[EXTFLASH]: READ: ");DPRINT;
      for (int i = 0; i < 4; i++)
      {
       printf("%"PRIXX8" ", flash_data[i]);DPRINT;
      }
      printf("\n");DPRINT;
   }

   const uint8_t flash_write_data[4] = {0x42, 0x42, 0x42, 0x42};DPRINT;

   bool flash_read = ext_flash_write(0x00, 4, flash_write_data);DPRINT;

   printf("[EXTFLASH]: ext_flash_write return %s\n", flash_read == true ? "write ok" : "write error");DPRINT;






/*
   printf("\nFLASH: ");DPRINT;
   for (int i = 0; i < sizeof(rbuf); i++)
   {
      printf("%"PRIXX8"", rbuf[i]);DPRINT;
   }


   printf("\n");DPRINT;
*/

   ext_flash_close();DPRINT;

}
/*---------------------------------------------------------------------------*/
void
ext_flash_init()
{
  ext_flash_open();DPRINT;
  ext_flash_close();DPRINT;
}
/*---------------------------------------------------------------------------*/
/** @} */
