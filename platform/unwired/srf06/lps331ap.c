/*
 * Copyright (C) 2014 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     driver_lps331ap
 * @{
 *
 * @file
 * @brief       Device driver implementation for the LPS331AP pressure sensor
 *
 * @note The current driver implementation is very basic and allows only for polling the
 *       devices temperature and pressure values. Threshold values and interrupts are not
 *       used.
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Peter Kietzmann <peter.kietzmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdint.h>

#include "board-i2c.h"
#include "lps331ap.h"
#include "lps331ap-internal.h"

/**
 * @brief default I2C bus speed for this sensor
 */
#define BUS_SPEED           I2C_SPEED_FAST

/**
 * @brief pressure divider for norming pressure output
 */
#define PRES_DIVIDER        (4096U)

/**
 * @brief temperature base value and divider for norming temperature output
 */
#define TEMP_BASE           (42.5f)
#define TEMP_DIVIDER        (480.0)

uint8_t lps_address = 0;

int lps331ap_init(lps331ap_rate_t rate)
{
    uint8_t tmp[2];

    /* Acquire exclusive access to the bus. */
    board_i2c_select(BOARD_I2C_INTERFACE_0, LPS331AP_SLAVE_ADDR);
    /* initialize underlying I2C bus */

    /* configure device, for simple operation only CTRL_REG1 needs to be touched */
    tmp[1] = LPS331AP_CTRL_REG1_DBDU | LPS331AP_CTRL_REG1_PD |
          (rate << LPS331AP_CTRL_REG1_ODR_POS);
    tmp[0] = LPS331AP_REG_CTRL_REG1;
    board_i2c_write(tmp, 2);

    return 0;
}

double lps331ap_read_temp()
{
    uint8_t tmp;
    int16_t val = 0;
    double res = TEMP_BASE;      /* reference value -> see datasheet */

    board_i2c_write_single(LPS331AP_REG_TEMP_OUT_L);
    board_i2c_read(&tmp, 1);
    val |= tmp;
    board_i2c_write_single(LPS331AP_REG_TEMP_OUT_H);
    board_i2c_read(&tmp, 1);
    val |= (tmp << 8);

    /* compute actual temperature value in °C */
    res += (double)val / TEMP_DIVIDER;

    /* return temperature in m°C */
    return res;
}

int lps331ap_read_pres()
{
    uint8_t tmp;
    int32_t val = 0;
    float res;

    board_i2c_write_single(LPS331AP_REG_PRESS_OUT_XL);
    board_i2c_read(&tmp, 1);
    val |= tmp;
    board_i2c_write_single(LPS331AP_REG_PRESS_OUT_L);
    board_i2c_read(&tmp, 1);
    val |= (tmp << 8);
    board_i2c_write_single(LPS331AP_REG_PRESS_OUT_H);
    board_i2c_read(&tmp, 1);
    val |= (tmp << 16);
    /* see if value is negative */
    if (tmp & 0x80) {
        val |= (0xff << 24);
    }

    /* compute actual pressure value in mbar */
    res = ((float)val) / PRES_DIVIDER;

    return (int)res;
}


int lps331ap_enable()
{
    uint8_t tmp[2];

    board_i2c_write_single(LPS331AP_REG_CTRL_REG1);
    board_i2c_read(tmp + 1, 1);
    if (tmp[1] == 0) {
        return -1;
    }
    tmp[1] |= (LPS331AP_CTRL_REG1_PD);
    tmp[0] = LPS331AP_REG_CTRL_REG1;
    board_i2c_write(tmp, 2);

    return 1;
}

int lps331ap_disable()
{
    uint8_t tmp[2];

    board_i2c_write_single(LPS331AP_REG_CTRL_REG1);
    if (board_i2c_read(tmp + 1, 1) != 1) {
        return -1;
    }
    tmp[1] &= ~(LPS331AP_CTRL_REG1_PD);
    tmp[0] = LPS331AP_REG_CTRL_REG1;
    board_i2c_write(tmp, 2);


    return 1;
}
