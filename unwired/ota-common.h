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
#define CORRECT_CRC                    0

/**
 *    NEW_FW_FLAG — Флаг статуса процесса обновления
 */
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


void
write_fw_flag(uint8_t value);

uint8_t
read_fw_flag();
