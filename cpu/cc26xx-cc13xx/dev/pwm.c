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
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
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

static uint32_t frequency;

/*---------------------------------------------------------------------------*/
LPM_MODULE(pwm_module, NULL, NULL, NULL, LPM_DOMAIN_PERIPH);

/*---------------------------------------------------------------------------*/
void pwm_config(uint32_t port, uint32_t freq)
{
    frequency = freq;
    /* Enable GPT0 clocks under active, sleep, deep sleep */
    ti_lib_prcm_peripheral_run_enable(PRCM_PERIPH_TIMER0);
    ti_lib_prcm_peripheral_sleep_enable(PRCM_PERIPH_TIMER0);
    ti_lib_prcm_peripheral_deep_sleep_enable(PRCM_PERIPH_TIMER0);
    ti_lib_prcm_load_set();
    while(!ti_lib_prcm_load_get());

    /* Drive the I/O ID with GPT0 / Timer A */
    ti_lib_ioc_port_configure_set(port, IOC_PORT_MCU_PORT_EVENT0, IOC_OUTPUT);

    /* GPT0 / Timer A: PWM, Interrupt Enable */
    HWREG(GPT0_BASE + GPT_O_TAMR) = (TIMER_CFG_A_PWM & 0xFF) | GPT_TAMR_TAPWMIE;

    ti_lib_timer_disable(GPT0_BASE, TIMER_A);
}
/*---------------------------------------------------------------------------*/
void pwm_stop()
{
    ti_lib_timer_disable(GPT0_BASE, TIMER_A);
    lpm_unregister_module(&pwm_module);
}
/*---------------------------------------------------------------------------*/
void pwm_start()
{
    lpm_register_module(&pwm_module);
    ti_lib_timer_enable(GPT0_BASE, TIMER_A);
}
/*---------------------------------------------------------------------------*/
void pwm_set_duty(uint8_t full_percent)
{
    uint32_t load = (GET_MCU_CLOCK / frequency);
    uint32_t match;

    if (full_percent > 100)
        return;

    if (full_percent == 100)
        match = load - 1;
    else
        match = (load / 100) * full_percent;

    ti_lib_timer_load_set(GPT0_BASE, TIMER_A, load);
    ti_lib_timer_match_set(GPT0_BASE, TIMER_A, match);
}

/*---------------------------------------------------------------------------*/
/** @} */
