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
 * \addtogroup srf06-common-peripherals
 * @{
 *
 * \file
 * Header file for the SmartRF06EB + CC13xx/CC26xxEM Button Driver
 */
/*---------------------------------------------------------------------------*/
#ifndef BUTTON_SENSOR_H_
#define BUTTON_SENSOR_H_
/*---------------------------------------------------------------------------*/
#include "lib/sensors.h"
/*---------------------------------------------------------------------------*/
#define BUTTON_SENSOR "Button"
/*---------------------------------------------------------------------------*/
#define BUTTON_SENSOR_VALUE_STATE    0
#define BUTTON_SENSOR_VALUE_DURATION 1

#define BUTTON_SENSOR_VALUE_RELEASED 0
#define BUTTON_SENSOR_VALUE_PRESSED  1

PROCESS_NAME(button_sensor_short_process);
PROCESS_NAME(button_sensor_long_process);

/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor button_a_sensor_click;
extern const struct sensors_sensor button_a_sensor_long_click;
extern const struct sensors_sensor button_a_sensor_change_on;
extern const struct sensors_sensor button_a_sensor_change_off;

extern const struct sensors_sensor button_b_sensor_click;
extern const struct sensors_sensor button_b_sensor_long_click;
extern const struct sensors_sensor button_b_sensor_change_on;
extern const struct sensors_sensor button_b_sensor_change_off;

extern const struct sensors_sensor button_c_sensor_click;
extern const struct sensors_sensor button_c_sensor_long_click;
extern const struct sensors_sensor button_c_sensor_change_on;
extern const struct sensors_sensor button_c_sensor_change_off;

extern const struct sensors_sensor button_d_sensor_click;
extern const struct sensors_sensor button_d_sensor_long_click;
extern const struct sensors_sensor button_d_sensor_change_on;
extern const struct sensors_sensor button_d_sensor_change_off;

extern const struct sensors_sensor button_e_sensor_click;
extern const struct sensors_sensor button_e_sensor_long_click;
extern const struct sensors_sensor button_e_sensor_change_on;
extern const struct sensors_sensor button_e_sensor_change_off;

/*---------------------------------------------------------------------------*/
#endif /* BUTTON_SENSOR_H_ */
/*---------------------------------------------------------------------------*/
/** @} */
