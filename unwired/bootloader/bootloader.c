#include <stdint.h>
#include "ti-lib.h"
#include <stdbool.h>

#include "ota-bootloader.h"

#define UART_TX_IOID     IOID_3
#define UART_SPEED       115200
#define LED_IOID         IOID_22

static void
power_domains_on(void) {
  /* Turn on the PERIPH PD */
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_PERIPH);

  /* Wait for domains to power on */
  while((ti_lib_prcm_power_domain_status(PRCM_DOMAIN_PERIPH)
        != PRCM_DOMAIN_POWER_ON));
}

void
initialize_peripherals() {
  /* Disable global interrupts */
  bool int_disabled = ti_lib_int_master_disable();

  power_domains_on();

  /* Enable GPIO peripheral */
  ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_GPIO);

  /* Apply settings and wait for them to take effect */
  ti_lib_prcm_load_set();
  while(!ti_lib_prcm_load_get());

  /* Re-enable interrupt if initially enabled. */
  if(!int_disabled) {
    ti_lib_int_master_enable();
  }

}


void
initialize_uart()
{
   static int (*input_handler)(unsigned char c);
   uint32_t ctl_val = UART_CTL_UARTEN | UART_CTL_TXE;

   ti_lib_prcm_power_domain_on(PRCM_DOMAIN_SERIAL);
   while(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_SERIAL) != PRCM_DOMAIN_POWER_ON);
   ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_UART0);
   ti_lib_prcm_load_set();
   while(!ti_lib_prcm_load_get());
   ti_lib_uart_disable(UART0_BASE);
   ti_lib_ioc_pin_type_gpio_output(UART_TX_IOID);
   ti_lib_gpio_set_dio(UART_TX_IOID);
   ti_lib_ioc_pin_type_uart(UART0_BASE, IOID_UNUSED, UART_TX_IOID, IOID_UNUSED, IOID_UNUSED);
   ti_lib_uart_config_set_exp_clk(UART0_BASE, ti_lib_sys_ctrl_clock_get(), UART_SPEED,
                                  (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
   ti_lib_uart_fifo_level_set(UART0_BASE, UART_FIFO_TX7_8, UART_FIFO_RX4_8);
   HWREG(UART0_BASE + UART_O_LCRH) |= UART_LCRH_FEN;
   if(input_handler) { ctl_val += UART_CTL_RXE; }
   HWREG(UART0_BASE + UART_O_CTL) = ctl_val;
}

int
main(void)
{
   initialize_peripherals();
   initialize_uart();
   print_uart("\n\n\n\n");

   ti_lib_ioc_pin_type_gpio_output(LED_IOID);
   ti_lib_gpio_set_dio(LED_IOID);

   bool spi_status = ext_flash_init();
   if (spi_status == false)
   {
      print_uart_bl("SPI flash not found, jump to main image\n");
      ti_lib_gpio_clear_dio(LED_IOID);
      jump_to_image( (CURRENT_FIRMWARE<<12) );
   }

   uint8_t fw_flag = read_fw_flag();

   OTAMetadata_t current_firmware;
   get_current_metadata( &current_firmware );
   int8_t verify_result_int = verify_current_firmware( &current_firmware );
   int8_t verify_result_ota_1 = verify_ota_slot( 1 );
   int8_t verify_result_ota_0 = verify_ota_slot( 0 );

   print_uart_bl("FW flag: ");
   print_uart_byte(fw_flag);
   print_uart("\n");

   print_uart_bl("Internal firmware ");
   if (verify_result_int == CORRECT_CRC)
      print_uart("correct CRC\n");
   if (verify_result_int == NON_CORRECT_CRC)
      print_uart("non-correct CRC\n");

   print_uart_bl("OTA slot 0 ");
   if (verify_result_ota_0 == NON_CORRECT_CRC)
      print_uart("non-correct CRC\n");
   if (verify_result_ota_0 == NON_READ_FLASH)
      print_uart("non-read flash\n");
   if (verify_result_ota_0 == CORRECT_CRC)
      print_uart("correct CRC\n");

   print_uart_bl("OTA slot 1 ");
   if (verify_result_ota_1 == NON_CORRECT_CRC)
      print_uart("non-correct CRC\n");
   if (verify_result_ota_1 == NON_READ_FLASH)
      print_uart("non-read flash\n");
   if (verify_result_ota_1 == CORRECT_CRC)
      print_uart("correct CRC\n");

   ti_lib_gpio_clear_dio(LED_IOID);

   /* Сброс флага после процесса обновления и прыжок на основную программу */
   if (fw_flag == FW_FLAG_PING_OK) //
   {
      print_uart_bl("OTA Update ok(PING_OK), change flag, jump to main image\n");
      write_fw_flag(FW_FLAG_NON_UPDATE);
      jump_to_image( (CURRENT_FIRMWARE<<12) );
   }

   /* Gрыжок на основную программу, если процесса обновления нет */
   if (fw_flag == FW_FLAG_NON_UPDATE) //
   {
      print_uart_bl("Jump to main image(FW_FLAG_NON_UPDATE)\n\n");
      jump_to_image( (CURRENT_FIRMWARE<<12) );
   }

   /* Прыжок на основную программу после неудачного обновления */
   if (fw_flag == FW_FLAG_ERROR_GI_LOADED)
   {
      print_uart_bl("Jump to main image(ERROR_GI_LOAD)\n\n");
      jump_to_image( (CURRENT_FIRMWARE<<12) );
   }

   /* Прыжок на основную программу после перезагрузки после обновления */
   if (fw_flag == FW_FLAG_NEW_IMG_INT_RST)
   {
      write_fw_flag(FW_FLAG_NEW_IMG_INT);
      print_uart_bl("Jump to main image(NEW_IMG_INT)\n\n");
      jump_to_image( (CURRENT_FIRMWARE<<12) );
   }

   /* Шьем Golden Image, если у нас нет флага подтверждения работы */
   if (fw_flag == FW_FLAG_NEW_IMG_INT)
   {
      print_uart_bl("Update error, set ERROR_GI_LOAD\n\n");
      write_fw_flag(FW_FLAG_ERROR_GI_LOADED);
      print_uart_bl("Flash golden image\n");
      update_firmware( 0 );
      //print_uart_bl("Need reboot\n");
      ti_lib_sys_ctrl_system_reset();
   }

   /* Шьем обновление, если у нас флаг новой прошивки */
   if (fw_flag == FW_FLAG_NEW_IMG_EXT)
   {
      print_uart_bl("Flash OTA image(NEW_IMG_EXT)\n");
      update_firmware( 1 );
      print_uart_bl("Set flag to FW_FLAG_NEW_IMG_INT_RST\n");
      write_fw_flag(FW_FLAG_NEW_IMG_INT_RST);
      //print_uart_bl("Need reboot\n");
      ti_lib_sys_ctrl_system_reset();
   }

  //  main() *should* never return - we should have rebooted or branched
  //  to other code by now.
  return 0;

}
