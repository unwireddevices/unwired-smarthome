#include <stdint.h>
#include "ti-lib.h"

#include "ota.h"

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

  /* Make sure the external flash is in the lower power mode */
  ext_flash_init();

  /* Re-enable interrupt if initially enabled. */
  if(!int_disabled) {
    ti_lib_int_master_enable();
  }

  ti_lib_ioc_pin_type_gpio_output(LED_IOID);
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


void
print_uart(char *str)
{
   for (int i=0; str[i] != '\0'; i++)
   {
      ti_lib_uart_char_put(UART0_BASE, str[i]);
   }
}

int
main(void)
{
   initialize_peripherals();
   initialize_uart();

   print_uart("Bootloader:\t start...\n");
   ti_lib_gpio_set_dio(LED_IOID);
   for (volatile int i = 0; i < 2000000; i++) { }


   ti_lib_gpio_clear_dio(LED_IOID);
   print_uart("Bootloader:\t jump to main image\n\n");
   jump_to_image( (CURRENT_FIRMWARE<<12) );
/*


  #if CLEAR_OTA_SLOTS
  erase_ota_image( 1 );
  erase_ota_image( 2 );
  erase_ota_image( 3 );
  #endif

  #if BURN_GOLDEN_IMAGE
  backup_golden_image();
  #endif

  //  (1) Get the metadata of whatever firmware is currently installed
  OTAMetadata_t current_firmware;
  get_current_metadata( &current_firmware );

  //  (2) Verify the current firmware! (Recompute the CRC over the internal flash image)
  verify_current_firmware( &current_firmware );

  //  (3) Are there any newer firmware images in ext-flash?
  uint8_t newest_ota_slot = find_newest_ota_image();
  OTAMetadata_t newest_firmware;
  while( get_ota_slot_metadata( newest_ota_slot, &newest_firmware ) );

  //  (4) Is the current image valid?
  if ( validate_ota_metadata( &current_firmware ) ) {
    if ( ( newest_ota_slot > 0 ) && (newest_firmware.version > current_firmware.version) ) {
      //  If there's a newer firmware image than the current firmware, install
      //  the newer version!
      update_firmware( newest_ota_slot );
      ti_lib_sys_ctrl_system_reset(); // reboot
    } else {
      //  If our image is valid, and there's nothing newer, then boot the firmware.
      jump_to_image( (CURRENT_FIRMWARE<<12) );
    }
  } else {
    //  If our image is not valid, install the newest valid image we have.
    //  Note: This can be the Golden Image, when newest_ota_slot = 0.
    update_firmware( newest_ota_slot );
    ti_lib_sys_ctrl_system_reset(); // reboot
  }

*/

  //  main() *should* never return - we should have rebooted or branched
  //  to other code by now.
  return 0;

}
