/*
 * Copyright (c) 2016, George Oikonomou - http://www.spd.gr
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
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
#ifndef TARGET_CONF_H_
#define TARGET_CONF_H_
/*---------------------------------------------------------------------------*/
#define CC26XX_UART_CONF_BAUD_RATE    460800
#define RF_BLE_CONF_ENABLED                0
#define ROM_BOOTLOADER_ENABLE              1


#undef NONCORESEC_CONF_SEC_LVL
#define NONCORESEC_CONF_SEC_LVL                    FRAME802154_SECURITY_LEVEL_NONE

#undef RPL_CONF_LEAF_ONLY
#define RPL_CONF_LEAF_ONLY                       0

#undef RPL_CONF_DIO_INTERVAL_MIN
#define RPL_CONF_DIO_INTERVAL_MIN                12 //2^X ms, 12(2^12 = 4.096s) default. The DIO interval (n) represents

#undef RPL_CONF_DIO_INTERVAL_DOUBLINGS
#define RPL_CONF_DIO_INTERVAL_DOUBLINGS          8 // RPL_CONF_DIO_INTERVAL_MIN + RPL_CONF_DIO_INTERVAL_DOUBLINGS <= 20


 #undef RPL_CONF_DEFAULT_LIFETIME
#define RPL_CONF_DEFAULT_LIFETIME                  5      //LIFETIME = DEFAULT_LIFETIME * DEFAULT_LIFETIME_UNIT
#undef RPL_CONF_DEFAULT_LIFETIME_UNIT
#define RPL_CONF_DEFAULT_LIFETIME_UNIT             60*60
#undef RPL_CONF_PROBING_INTERVAL
#define RPL_CONF_PROBING_INTERVAL                  ((RPL_CONF_DEFAULT_LIFETIME * RPL_CONF_DEFAULT_LIFETIME_UNIT) / 2 ) * CLOCK_SECOND
/*---------------------------------------------------------------------------*/
#define SENSNIFF_IO_DRIVER_H "pool/cc13xx-cc26xx-io.h"
/*---------------------------------------------------------------------------*/
#endif /* TARGET_CONF_H_ */
/*---------------------------------------------------------------------------*/
