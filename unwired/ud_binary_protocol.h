/*---------------------------------------------------------------------------*/
/**
 * \file
 *         Defines file for Unwired Devices Binary Protocol(UDBP)
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 */
/*---------------------------------------------------------------------------*/

/* Inter-device protocol versions */
#define PROTOCOL_VERSION_V1                                     0x01
#define PROTOCOL_VERSION_V2                                     0x02
#define DEVICE_VERSION_V1                                       0x01

/* Data types */
#define DATA_TYPE_JOIN                                          0x01 //Запрос на включение в сеть
#define DATA_TYPE_SENSOR_DATA                                   0x02 //Данные с датчиков устройства
#define DATA_TYPE_JOIN_CONFIRM                                  0x03 //Подтверждение запроса на включение в сеть
#define DATA_TYPE_PONG                                          0x04 //Подтверждение доставки пакета
#define DATA_TYPE_COMMAND                                       0x05 //Команды возможностям устройства
#define DATA_TYPE_STATUS                                        0x06 //Пакет со статусными данными
#define DATA_TYPE_GET_STATUS                                    0x07 //Запрос статуса(не реализовано)
#define DATA_TYPE_SETTINGS                                      0x08 //Команда настройки параметров
#define DATA_TYPE_MESSAGE                                       0x09 //Сообщения
#define DATA_TYPE_SET_TIME                                      0x0A //Команда установки времени(не реализовано)
#define DATA_TYPE_SET_SCHEDULE                                  0x0B //Команда установки расписания(не реализовано)
#define DATA_TYPE_FIRMWARE                                      0x0C //Данные для OTA
#define DATA_TYPE_UART                                          0x0D //Команда с данными UART
#define DATA_TYPE_FIRMWARE_CMD                                  0x0E //Команды OTA

/* Reserved data */
#define DATA_RESERVED                                           0xFF
#define DATA_NONE                                               DATA_RESERVED

/* Devices groups */
/* Показывает только общее назначение устройства */
#define DEVICE_GROUP_BUTTON_SWITCH                              0x00
#define DEVICE_GROUP_SENSORS                                    0x01
#define DEVICE_GROUP_MOTION_SENSOR                              0x02
#define DEVICE_GROUP_OPEN_SENSORS                               0x03
#define DEVICE_GROUP_METERS                                     0x04
#define DEVICE_GROUP_RELAY                                      0x05
#define DEVICE_GROUP_DIMMER                                     0x06
#define DEVICE_GROUP_LIGHT                                      0x07
#define DEVICE_GROUP_RGB_LIGHT                                  0x08
#define DEVICE_GROUP_BRIDGE_CONVERTER                           0x09
#define DEVICE_GROUP_OTHER                                      0xFF

/* Devices ability's */
/* Определяет наличие в устройстве конкретных возможностей */
#define DEVICE_ABILITY_NONE                                     0

#define DEVICE_ABILITY_BUTTON                                   1
#define DEVICE_ABILITY_TEMPERATURE                              2
#define DEVICE_ABILITY_HUMIDITY                                 3
#define DEVICE_ABILITY_PRESSURE                                 4
#define DEVICE_ABILITY_LIGHT_SENSOR                             5
#define DEVICE_ABILITY_NOISE_SENSOR                             6
#define DEVICE_ABILITY_MOTION_SENSOR                            7
#define DEVICE_ABILITY_RESERVED1                                8

#define DEVICE_ABILITY_C02_SENSOR                               9
#define DEVICE_ABILITY_CO_SENSOR                                10
#define DEVICE_ABILITY_GAS_SENSOR                               11
#define DEVICE_ABILITY_POWER_METER                              12
#define DEVICE_ABILITY_RADIATION_METER                          13
#define DEVICE_ABILITY_RESERVED3                                14
#define DEVICE_ABILITY_RESERVED4                                15
#define DEVICE_ABILITY_RESERVED5                                16

#define DEVICE_ABILITY_RELAY                                    17
#define DEVICE_ABILITY_DIMMER                                   18
#define DEVICE_ABILITY_RESERVED6                                19
#define DEVICE_ABILITY_RESERVED7                                20
#define DEVICE_ABILITY_RESERVED8                                21
#define DEVICE_ABILITY_RESERVED9                                22
#define DEVICE_ABILITY_RESERVED10                               23
#define DEVICE_ABILITY_LED                                      24

#define DEVICE_ABILITY_OTA                                      25 //использовать для определения поддержки OTA
#define DEVICE_ABILITY_RESERVED12                               26
#define DEVICE_ABILITY_RESERVED13                               27
#define DEVICE_ABILITY_RESERVED14                               28
#define DEVICE_ABILITY_UART_BRIGE                               29
#define DEVICE_ABILITY_0_10V_ANALOG                             30
#define DEVICE_ABILITY_RS485_BRIGE                              31
#define DEVICE_ABILITY_DALI_BRIGE                               32

/* Devices sleep types */
#define DEVICE_SLEEP_TYPE_NORMAL                                0x01
#define DEVICE_SLEEP_TYPE_LEAF                                  0x02

/* DEVICE_ABILITY_BUTTON events */
#define DEVICE_ABILITY_BUTTON_EVENT_CLICK                       0x01
#define DEVICE_ABILITY_BUTTON_EVENT_LONG_CLICK                  0x02
#define DEVICE_ABILITY_BUTTON_EVENT_ON                          0x03
#define DEVICE_ABILITY_BUTTON_EVENT_OFF                         0x04

/* DEVICE_ABILITY_MOTION_SENSOR events */
#define DEVICE_ABILITY_MOTION_SENSOR_EVENT_MOTION               0x01
#define DEVICE_ABILITY_MOTION_SENSOR_EVENT_NO_MOTION            0x02

/* DEVICE_ABILITY_LED commands */
#define DEVICE_ABILITY_LED_COMMAND_OFF                          0x00
#define DEVICE_ABILITY_LED_COMMAND_ON                           0x01
#define DEVICE_ABILITY_LED_COMMAND_BLINK                        0x02

/* DEVICE_ABILITY_RELAY commands and targets */
#define DEVICE_ABILITY_RELAY_COMMAND_OFF                        0x00
#define DEVICE_ABILITY_RELAY_COMMAND_ON                         0x01
#define DEVICE_ABILITY_RELAY_COMMAND_TOGGLE                     0x80
#define DEVICE_ABILITY_RELAY_COMMAND_BLINK                      0x81

#define DEVICE_ABILITY_RELAY_1_SETTINGS_START_STATE             0x01
#define DEVICE_ABILITY_RELAY_2_SETTINGS_START_STATE             0x02

#define DEVICE_ABILITY_RELAY_1                                  0x01
#define DEVICE_ABILITY_RELAY_2                                  0x02

/* DEVICE_ABILITY_DIMMER commands, settings and targets */
#define DEVICE_ABILITY_DIMMER_COMMAND_OFF                       0x00
#define DEVICE_ABILITY_DIMMER_COMMAND_ON                        0x64 //100 in decimal
#define DEVICE_ABILITY_DIMMER_COMMAND_TOGGLE                    0x80
#define DEVICE_ABILITY_DIMMER_COMMAND_BLINK                     0x81

#define DEVICE_ABILITY_DIMMER_SETTINGS_FADE_TIME                0x01
#define DEVICE_ABILITY_DIMMER_SETTINGS_RISE_TIME                0x02
#define DEVICE_ABILITY_DIMMER_1_SETTINGS_START_STATE            0x03
#define DEVICE_ABILITY_DIMMER_2_SETTINGS_START_STATE            0x04

#define DEVICE_ABILITY_DIMMER_1                                 0x01
#define DEVICE_ABILITY_DIMMER_2                                 0x02

/* DEVICE_ABILITY_0_10V_ANALOG commands and targets */
#define DEVICE_ABILITY_0_10V_ANALOG_COMMAND_OFF                 0x00
#define DEVICE_ABILITY_0_10V_ANALOG_COMMAND_ON                  0x64 //100 in decimal
#define DEVICE_ABILITY_0_10V_ANALOG_COMMAND_TOGGLE              0x80
#define DEVICE_ABILITY_0_10V_ANALOG_COMMAND_BLINK               0x81

#define DEVICE_ABILITY_0_10V_ANALOG_SETTINGS_FADE_TIME          0x01
#define DEVICE_ABILITY_0_10V_ANALOG_SETTINGS_RISE_TIME          0x02
#define DEVICE_ABILITY_0_10V_ANALOG_SETTINGS_START_STATE        0x03

#define DEVICE_ABILITY_0_10V_ANALOG_CHANNEL_1                   0x01

/* DATA_TYPE_FIRMWARE commands  */
#define DATA_TYPE_FIRMWARE_COMMAND_NEW_FW                       0x01 //Сообщение о наличии новой прошивки
#define DATA_TYPE_FIRMWARE_COMMAND_CHANK_REQ                    0x02 //Запрос пакета с частью прошивки
#define DATA_TYPE_FIRMWARE_COMMAND_REBOOT                       0x03 //Перезагрузка. Серьезно, без этого комментария не понятно?
#define DATA_TYPE_FIRMWARE_COMMAND_CLEAN_GI                     0x04 //Стереть образ GI. Будет записан после перезагрузки.


#define FIRMWARE_PAYLOAD_LENGTH                                 600
#define FIRMWARE_PAYLOAD_OFFSET                                 7

/* Devices messages */
#define DEVICE_MESSAGE_HIGH_TEMPERATYRE                         0x01
#define DEVICE_MESSAGE_LOW_VOLTAGE                              0x02
#define DEVICE_MESSAGE_HIGH_CURRENT                             0x03
#define DEVICE_MESSAGE_LOW_POWER                                0x04
#define DEVICE_MESSAGE_ERROR_ON_RELAY                           0x05
#define DEVICE_MESSAGE_ERROR_OFF_RELAY                          0x06
#define DEVICE_MESSAGE_OTA_SPI_NOTACTIVE                        0x07
#define DEVICE_MESSAGE_OTA_NOT_DELIVERED_CHUNK                  0x08
#define DEVICE_MESSAGE_OTA_NONCORRECT_CRC                       0x09
#define DEVICE_MESSAGE_OTA_BAD_GOLDEN_IMAGE                     0x0A
#define DEVICE_MESSAGE_OTA_SPI_ERASE_IN_PROGRESS                0x0B
#define DEVICE_MESSAGE_OTA_UPDATE_SUCCESS                       0x0C
#define DEVICE_MESSAGE_OTA_NONCORRECT_UUID                      0x0D

/* UART Binary data */
#define UART_PROTOCOL_VERSION_V1                                0x01
#define UART_PROTOCOL_VERSION_V2                                0x02
#define UART_PROTOCOL_VERSION_V3                                0x03
#define MAX_UART_DATA_LENGTH                                    1500
#define MAGIC_SEQUENCE_LENGTH                                   6
#define MAGIC_SEQUENCE                                          0x01,0x16,0x16,0x16,0x16,0x10

#define PACKET_HEADER_LENGTH                                    10

/* DEVICE_ROOT_LOCAL commands */
#define LOCAL_ROOT_COMMAND_REBOOT                               0x00
#define LOCAL_ROOT_COMMAND_BOOTLOADER_ACTIVATE                  0x01

/* Defines */

#define UDP_DATA_PORT                                           4004

#define VOLTAGE_PRESCALER                                       16
