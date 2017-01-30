/*
 * Copyright (c) 2015, Unwired Devices- http://www.unwireddevices.com
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
 *
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
 * \addtogroup cc26xx-cc13xx
 * @{
 *
 * \file
 *  Driver for the CC26XX-CC13XX PWM
 *
 * \author
 *         Mikhail Churikov <mc@unwds.com>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "driverlib/prcm.h"
#include "driverlib/ioc.h"
#include "pwm.h"
#include "ti-lib.h"
#include "lpm.h"
#include <stdio.h>
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#define DEBUG 1
#if DEBUG
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
/*---------------------------------------------------------------------------*/
#define PWM_GPTIMER_NUM_TO_BASE(x) ((GPT0_BASE) + ((x) << 12))
/*---------------------------------------------------------------------------*/
LPM_MODULE(pwm_module, NULL, NULL, NULL, LPM_DOMAIN_PERIPH);
/*---------------------------------------------------------------------------*/
static void
power_domain_on(void)
{
  ti_lib_prcm_power_domain_on(PRCM_DOMAIN_PERIPH);
  while(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_PERIPH) !=
        PRCM_DOMAIN_POWER_ON);
}
/*---------------------------------------------------------------------------*/
static uint8_t
pwm_configured(uint8_t timer, uint32_t ab)
{
  uint8_t offset;
  uint32_t gpt_base;
  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);
  offset = (ab == PWM_TIMER_B) ? 4 : 0;
  PRINTF("PWM: CHECK - GPT_x_BASE 0x%08lX offset: 0x%08d\n", gpt_base, offset);
  PRINTF("PWM: CHECK: ADDR: 0x%08lx REG CONTENTS: 0x%08lx\n", 
      gpt_base + GPT_O_TAMR + offset,
      HWREG(gpt_base + GPT_O_TAMR + offset));

  PRINTF("PWM: CHECK - TAAMS: 0x%08lX PERIODIC: 0x%08ld\n", 
      HWREG(gpt_base + GPT_O_TAMR + offset) & GPT_TAMR_TAAMS, 
      HWREG(gpt_base + GPT_O_TAMR + offset) & GPT_TAMR_TAMR_PERIODIC);

  if((HWREG(gpt_base + GPT_O_TAMR + offset) & GPT_TAMR_TAAMS) &&
     (HWREG(gpt_base + GPT_O_TAMR + offset) & GPT_TAMR_TAMR_PERIODIC)) {
    PRINTF("PWM: CHECK - PASS!!\n");
    return 1;
  }
  PRINTF("PWM: CHECK - FAIL!!\n");
  return 0;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_enable(uint32_t freq, uint8_t duty, uint8_t timer, uint32_t ab)
{
  uint32_t interval_load, duty_count;
  uint32_t gpt_base, gpt_mode;
  uint8_t offset;

  if((freq < PWM_FREQ_MIN) || (freq > PWM_FREQ_MAX) ||
     (duty < PWM_DUTY_MIN) || (duty > PWM_DUTY_MAX) ||
     (timer > PWM_TIMER_MAX) || (timer < PWM_TIMER_MIN)) {
    PRINTF("PWM: Invalid PWM settings\n");
    return PWM_ERROR;
  }

  /* GPT0 timer A is used for clock_delay_usec() in clock.c */
  if((ab == PWM_TIMER_A) && (timer == PWM_TIMER_0)) {
    PRINTF("PWM: GPT0 (timer A) is reserved for clock_delay_usec()\n");
    return PWM_ERROR;
  }

  PRINTF("PWM: F%08luHz: %u%% on GPT%u-%lu\n", freq, duty, timer, ab);

  /* PRCM check clock on periphral domain */
  if(ti_lib_prcm_power_domain_status(PRCM_DOMAIN_PERIPH) !=
     PRCM_DOMAIN_POWER_ON) {
    power_domain_on();
    PRINTF("PWM: Power on GPT%u-%lu domain\n", timer, ab);
  }

  if (!ti_lib_prcm_peripheral_status_enabled(timer)) {
    /* PRCM Enable clock on GPT_On */
    /* PRCM_PERIPH_TIMERn directly mapped with PWM_TIMER_n */
    ti_lib_prcm_peripheral_run_enable(timer);
    ti_lib_prcm_peripheral_sleep_enable(timer);
    ti_lib_prcm_peripheral_deep_sleep_enable(timer);
    PRINTF("PWM: PRCM_PERIPH_TIMERn:0x%08X\n", timer);
    ti_lib_prcm_load_set();
    while(!ti_lib_prcm_load_get());
  }

  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);
  gpt_mode = (ab == PWM_TIMER_A) ? (GPT_TAMR_TAAMS | GPT_TAMR_TAMR_PERIODIC): 
                                   ((GPT_TBMR_TBAMS | GPT_TBMR_TBMR_PERIODIC) << 8);


  offset = (ab == PWM_TIMER_B) ? 4 : 0;
        PRINTF("PWM: BEFORE: ADDR: 0x%08lx TAMR REG CONTENTS: 0x%08lx\n",
            gpt_base + GPT_O_TAMR + 0x0,
            HWREG(gpt_base + GPT_O_TAMR + offset));
        PRINTF("PWM: BEFORE: ADDR: 0x%08lx TBMR REG CONTENTS: 0x%08lx\n",
            gpt_base + GPT_O_TAMR + 0x4,
            HWREG(gpt_base + GPT_O_TAMR + offset));

        /* Restore later, ensure GPT_CTL_TxEN and GPT_CTL_TxPWML are clear */
        //ti_lib_timer_disable(gpt_base, ab);
        PRINTF("PWM: GPT MODE: 0x%08lX\n", TIMER_CFG_SPLIT_PAIR | gpt_mode);
  PRINTF("PWM: GPT_x_BASE 0x%08lX\n", gpt_base);
  if (!((HWREG(gpt_base + GPT_O_CTL) & GPT_CTL_TAEN) |
      (HWREG(gpt_base + GPT_O_CTL) & GPT_CTL_TBEN))) {
    /*
     * Register ourself with LPM. This will keep the PERIPH PD powered on
     * during deep sleep, allowing the buzzer to keep working while the chip is
     * being power-cycled
     */
    lpm_register_module(&pwm_module);
    //HWREG(gpt_base + GPT_O_CTL) &= ~(GPT_CTL_TAEN | GPT_CTL_TBEN);
    //HWREG(gpt_base + GPT_O_CFG) = TIMER_CFG_SPLIT_PAIR >> 24;
    //ti_lib_timer_configure(gpt_base, TIMER_CFG_SPLIT_PAIR | gpt_mode);
    HWREG(gpt_base + GPT_O_CTL) &= ~(GPT_CTL_TAEN | GPT_CTL_TBEN);
    HWREG(gpt_base + GPT_O_CFG) = (TIMER_CFG_SPLIT_PAIR | gpt_mode) >> 24;
    HWREG(gpt_base + GPT_O_TAMR) = ((TIMER_CFG_SPLIT_PAIR | gpt_mode) & 0xFF) | GPT_TAMR_TAPWMIE;
    HWREG(gpt_base + GPT_O_TBMR) = (((TIMER_CFG_SPLIT_PAIR | gpt_mode) >> 8) & 0xFF) | GPT_TBMR_TBPWMIE;
    PRINTF("PWM: Disable ALL\n");
  }
  else if (ab == PWM_TIMER_A) {
    PRINTF("PWM: Disable A\n");

      /* Configure PWM split mode */
      //ti_lib_timer_configure(gpt_base, TIMER_CFG_SPLIT_PAIR | gpt_mode);
        //
        // Disable the timers.
        //
        HWREG(gpt_base + GPT_O_CTL) &= ~GPT_CTL_TAEN;

        //
        // Set the global timer configuration.
        //
        HWREG(gpt_base + GPT_O_CFG) = (TIMER_CFG_SPLIT_PAIR | gpt_mode) >> 24;

        //
        // Set the configuration of the A and B timers. Note that the B timer
        // configuration is ignored by the hardware in 32-bit modes.
        //
        HWREG(gpt_base + GPT_O_TAMR) = ((TIMER_CFG_SPLIT_PAIR | gpt_mode) & 0xFF) | GPT_TAMR_TAPWMIE;

  }
  else {
    PRINTF("PWM: Disable B\n");

      /* Configure PWM split mode */
      //ti_lib_timer_configure(gpt_base, TIMER_CFG_SPLIT_PAIR | gpt_mode);
        //
        // Disable the timers.
        //
        HWREG(gpt_base + GPT_O_CTL) &= ~GPT_CTL_TBEN;

        //
        // Set the global timer configuration.
        //
        HWREG(gpt_base + GPT_O_CFG) = (TIMER_CFG_SPLIT_PAIR | gpt_mode) >> 24;

        //
        // Set the configuration of the A and B timers. Note that the B timer
        // configuration is ignored by the hardware in 32-bit modes.
        //
        HWREG(gpt_base + GPT_O_TBMR) =
            (((TIMER_CFG_SPLIT_PAIR | gpt_mode) >> 8) & 0xFF) | GPT_TBMR_TBPWMIE;

  }

  PRINTF("PWM: AFTER: ADDR: 0x%08lx TAMR REG CONTENTS: 0x%08lx\n", 
      gpt_base + GPT_O_TAMR + 0x0,
      HWREG(gpt_base + GPT_O_TAMR + 0x0));
  PRINTF("PWM: AFTER: ADDR: 0x%08lx TBMR REG CONTENTS: 0x%08lx\n", 
      gpt_base + GPT_O_TAMR + 0x4,
      HWREG(gpt_base + GPT_O_TAMR + 0x4));
  /* Set INVERTED PWM mode on specified timer */
  ti_lib_timer_level_control(gpt_base, ab, true);

  /* If the duty cycle is zero, leave the GPTIMER configured as PWM to pass a next
   * configured check, but do nothing else */
  if(!duty) {
    return PWM_SUCCESS;
  }

  /* Get the peripheral clock and equivalent deassert count */
  interval_load = PWM_SYS_CLK / freq;
  duty_count = ((interval_load * duty) + 1) / 100;

  PRINTF("PWM: sys %uHz: %lu %lu\n", PWM_SYS_CLK, 
         interval_load, duty_count);

  /* Set the start value (period), count down */
  ti_lib_timer_load_set(gpt_base, ab, ((uint16_t *)&interval_load)[0] - 1);
  /* Set the deassert period */
  ti_lib_timer_match_set(gpt_base, ab, ((uint16_t *)&duty_count)[0] - 1);
  /* Set the prescaler if required */
  ti_lib_timer_prescale_set(gpt_base, ab, ((uint8_t *)&interval_load)[2]);
  /* Set the prescaler match if required */
  ti_lib_timer_prescale_match_set(gpt_base, ab, ((uint8_t *)&duty_count)[2]);
  /* Restore the register content */

  PRINTF("PWM: TnILR %lu ", ti_lib_timer_load_get(gpt_base, ab));
  PRINTF("TnMATCHR %lu  ", ti_lib_timer_match_get(gpt_base, ab));
  PRINTF("TnPR %lu  ", ti_lib_timer_prescale_get(gpt_base, ab));
  PRINTF("TnPMR %lu\n", ti_lib_timer_prescale_match_get(gpt_base, ab));

  PRINTF("PWM: END: ADDR: 0x%08lx TAMR REG CONTENTS: 0x%08lx\n", 
      gpt_base + GPT_O_TAMR + 0x0,
      HWREG(gpt_base + GPT_O_TAMR + 0x0));
  PRINTF("PWM: END: ADDR: 0x%08lx TBMR REG CONTENTS: 0x%08lx\n", 
      gpt_base + GPT_O_TAMR + 0x4,
      HWREG(gpt_base + GPT_O_TAMR + 0x4));
  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_stop(uint8_t timer, uint32_t ab, uint8_t pin, bool release)
{
  //uint32_t gpt_base;

  if((ab > PWM_TIMER_B) || (timer < PWM_TIMER_MIN) ||
     (timer > PWM_TIMER_MAX)) {
    PRINTF("PWM: Invalid PWM values\n");
    return PWM_ERROR;
  }

  if(!pwm_configured(timer, ab)) {
    PRINTF("PWM: GPTn not configured as PWM\n");
    return PWM_ERROR;
  }

  /* CC26xx-CC13xx has only 32 available IOIDs */
  if(pin >= NUM_IO_MAX) {
    PRINTF("PWM: Invalid pin settings\n");
    return PWM_ERROR;
  }

  ti_lib_timer_disable(PWM_GPTIMER_NUM_TO_BASE(timer), ab);

  if (release) {
    /* Configure the pin as GPIO, input */
    ti_lib_ioc_port_configure_set(pin, IOC_PORT_GPIO, IOC_STD_INPUT);
  }

  PRINTF("PWM: OFF -> Timer %u (%lu)\n", timer, ab);
  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_start(uint8_t timer, uint32_t ab, uint8_t pin)
{
  uint32_t gpt_base;
  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);

  if((ab > PWM_TIMER_B) || (timer < PWM_TIMER_MIN) ||
     (timer > PWM_TIMER_MAX)) {
    PRINTF("PWM: Invalid PWM values\n");
    return PWM_ERROR;
  }

  if(!pwm_configured(timer, ab)) {
    PRINTF("PWM: GPTn not configured as PWM\n");
    return PWM_ERROR;
  }

  /* CC26xx-CC13xx has only 32 available IOIDs */
  if(pin >= NUM_IO_MAX) {
    PRINTF("PWM: Invalid pin settings\n");
    return PWM_ERROR;
  }

  PRINTF("PWM: Enable: ADDR: 0x%08lx TAMR REG CONTENTS: 0x%08lx\n", 
      gpt_base + GPT_O_TAMR + 0x0,
      HWREG(gpt_base + GPT_O_TAMR + 0x0));
  PRINTF("PWM: Enable: ADDR: 0x%08lx TBMR REG CONTENTS: 0x%08lx\n", 
      gpt_base + GPT_O_TAMR + 0x4,
      HWREG(gpt_base + GPT_O_TAMR + 0x4));
  ti_lib_timer_enable(PWM_GPTIMER_NUM_TO_BASE(timer), ab);
  /* Map to given pin */
  ti_lib_ioc_port_configure_set(pin, IOC_PORT_MCU_PORT_EVENT0 + (2 * timer) + (ab >> 15), IOC_OUTPUT);
  PRINTF("PWM: after Enable: ADDR: 0x%08lx TAMR REG CONTENTS: 0x%08lx\n", 
      gpt_base + GPT_O_TAMR + 0x0,
      HWREG(gpt_base + GPT_O_TAMR + 0x0));
  PRINTF("PWM: after Enable: ADDR: 0x%08lx TBMR REG CONTENTS: 0x%08lx\n", 
      gpt_base + GPT_O_TAMR + 0x4,
      HWREG(gpt_base + GPT_O_TAMR + 0x4));

  PRINTF("PWM: ON -> Timer %u (%lu) IOID:%u IOC_PORT_MCU_PORT_EVENT:%08lX\n", timer, ab, pin, 
          IOC_PORT_MCU_PORT_EVENT0 + (2 * timer) + (ab >> 15));
  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_set_duty(uint8_t timer, uint32_t ab, uint32_t freq, uint8_t duty)
{
  uint32_t gpt_base;
  uint32_t interval_load, duty_count;

  if((ab > PWM_TIMER_B) || (timer < PWM_TIMER_MIN) ||
     (timer > PWM_TIMER_MAX) || (duty > 100)) {
    PRINTF("PWM: Invalid PWM values\n");
    return PWM_ERROR;
  }

  if(!pwm_configured(timer, ab)) {
    PRINTF("PWM: GPTn not configured as PWM\n");
    return PWM_ERROR;
  }

  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);

  /* Get the peripheral clock and equivalent deassert count */
  interval_load = PWM_SYS_CLK / freq;
  duty_count = ((interval_load * duty) + 1) / 100;

  PRINTF("PWM: sys %uHz: %lu %lu\n", PWM_SYS_CLK, 
         interval_load, duty_count);

  /* Set the start value (period), count down */
  ti_lib_timer_load_set(gpt_base, ab, ((uint16_t *)&interval_load)[0] - 1);
  /* Set the deassert period */
  ti_lib_timer_match_set(gpt_base, ab, ((uint16_t *)&duty_count)[0] - 1);
  /* Set the prescaler if required */
  ti_lib_timer_prescale_set(gpt_base, ab, ((uint8_t *)&interval_load)[2]);
  /* Set the prescaler match if required */
  ti_lib_timer_prescale_match_set(gpt_base, ab, ((uint8_t *)&duty_count)[2]);
  /* Restore the register content */

  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_set_direction(uint8_t timer, uint32_t ab, uint32_t dir)
{
  uint32_t gpt_base;

  if((ab > PWM_TIMER_B) || (timer < PWM_TIMER_MIN) ||
     (timer > PWM_TIMER_MAX) || (dir > PWM_SIGNAL_INVERTED+1)) {
    PRINTF("PWM: Invalid PWM values\n");
    return PWM_ERROR;
  }

  if(!pwm_configured(timer, ab)) {
    PRINTF("PWM: GPTn not configured as PWM\n");
    return PWM_ERROR;
  }

  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);
  if(dir) {
    ti_lib_timer_level_control(gpt_base, ab, true);
  } else {
    ti_lib_timer_level_control(gpt_base, ab, false);
  }

  PRINTF("PWM: Signal direction (%lu) -> Timer %u (%lu)\n", dir, timer, ab);
  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
int8_t
pwm_disable(uint8_t timer, uint32_t ab, uint8_t pin)
{
  uint32_t gpt_base;
  uint8_t offset = (ab == PWM_TIMER_B) ? 4 : 0;
  gpt_base = PWM_GPTIMER_NUM_TO_BASE(timer);

  if((ab > PWM_TIMER_B) || (timer < PWM_TIMER_MIN) ||
     (timer > PWM_TIMER_MAX)) {
    PRINTF("PWM: Invalid PWM values\n");
    return PWM_ERROR;
  }

  /* CC26xx-CC13xx has only 32 available IOIDs */
  if(pin >= NUM_IO_MAX) {
    PRINTF("PWM: Invalid pin settings\n");
    return PWM_ERROR;
  }

  if(!pwm_configured(timer, ab)) {
    PRINTF("PWM: GPTn not configured as PWM\n");
    return PWM_ERROR;
  }

  /* Stop the PWM */
  pwm_stop(timer, ab, pin, PWM_OFF_WHEN_STOP);
  /* Disable the PWM mode */
  HWREG(gpt_base + (GPT_O_TAMR + offset)) = 0;
  /* Restart the interval load and deassert values */
  HWREG(gpt_base + (GPT_O_TAILR + offset)) = 0;
  HWREG(gpt_base + (GPT_O_TAMATCHR + offset)) = 0;

  /* Configure the pin as GPIO, input */
  ti_lib_ioc_port_configure_set(pin, IOC_PORT_GPIO, IOC_STD_INPUT);

  return PWM_SUCCESS;
}
/*---------------------------------------------------------------------------*/
/** @} */
