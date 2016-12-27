#include <stdint.h>

#ifndef PIN_CONFIG_H_
#define PIN_CONFIG_H_

typedef enum {
  LED,
  BUTTON,
  GPIO_IN,
  GPIO_OUT,
  GPIO_INTERRUPT,
  PWM,
  UART_TX,
  UART_TX,
  UART_CTS,
  UART_RTS,
  SPI_MISO,
  SPI_MOSI,
  SPI_SCK,
  I2C_SDA,
  I2C_SCL,
  ANALOG
} function_t;

typedef enum
{
  UNUSED,
  ACTIVE,
  ACCUIRED
} state_t;

typedef structure 
{
  function_t function;
  state_t state;

} pin_config_t;

pin_config_t pin_settings[32];

#endif /* PIN_CONFIG_H_ */
