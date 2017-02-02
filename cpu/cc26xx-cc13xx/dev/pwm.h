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
 *
 * \file
 * Header file for the CC26XX-CC13XX PWM driver
 *
 * \author
 *         Vladislav Zaytsev vvzvlad@gmail.com vz@unwds.com
 */
/*---------------------------------------------------------------------------*/
#ifndef PWM_H_
#define PWM_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "driverlib/ioc.h"
#include "driverlib/timer.h"

#define PWM_SUCCESS    0
#define PWM_ERROR      (-1)
/** @} */
/*---------------------------------------------------------------------------*/
/** \name PWM recommended values respect to peripheral clock frequency
 * @{
 */
/* Roughly 244 Hz with a 16-MHz system clock, no prescaler */
#define PWM_SYS_16MHZ_NO_PRES_MIN            0xFFFF
#define PWM_SYS_16MHZ_NO_PRES_MIN_FREQ       244
/* Roughly 1 Hz with a 16-MHz system clock, to keep frequency parameter in Hz */
#define PWM_SYS_16MHZ_PRES_MIN               0x00F42400
#define PWM_SYS_16MHZ_PRES_MIN_FREQ          1
/* Yields 160 KHz at 16 MHz and allows down to 1% (integer) duty cycles */
#define PWM_SYS_16MHZ_NO_PRES_MAX            100
#define PWM_SYS_16MHZ_NO_PRES_MAX_FREQ       160000
/** @} */
/*---------------------------------------------------------------------------*/
/** \name PWM driver definitions and configuration values
 * @{
 */
#define PWM_TIMER_A                          TIMER_A
#define PWM_TIMER_B                          TIMER_B
#define PWM_TIMER_0                          0
#define PWM_TIMER_1                          1
#define PWM_TIMER_2                          2
#define PWM_TIMER_3                          3
#define PWM_TIMER_MIN                        PWM_TIMER_0
#define PWM_TIMER_MAX                        PWM_TIMER_3
#define PWM_SIGNAL_STRAIGHT                  1
#define PWM_SIGNAL_INVERTED                  0
#define PWM_OFF_WHEN_STOP                    0
#define PWM_ON_WHEN_STOP                     1
#define PWM_GPTIMER_CFG_SPLIT_MODE           0x04
#define PWM_DUTY_MAX                         100
#define PWM_DUTY_MIN                         0
#define PWM_FREQ_MIN                         PWM_SYS_16MHZ_PRES_MIN_FREQ
#define PWM_FREQ_MAX                         PWM_SYS_16MHZ_NO_PRES_MAX_FREQ
#define PWM_SYS_CLK                          48000000
#define IOC_OUTPUT                      (IOC_CURRENT_8MA    | IOC_STRENGTH_MAX | \
                                         IOC_NO_IOPULL      | IOC_SLEW_DISABLE | \
                                         IOC_HYST_DISABLE   | IOC_NO_EDGE      | \
                                         IOC_INT_DISABLE    | IOC_IOMODE_INV   | \
                                         IOC_NO_WAKE_UP     | IOC_INPUT_DISABLE )

void pwm_config(uint32_t port, uint32_t freq);
void pwm_stop();
void pwm_start();
void pwm_set_duty(uint8_t full_percent);


#endif /* PWM_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */
