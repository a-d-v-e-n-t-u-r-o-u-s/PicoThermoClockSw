/*!
 * \file
 * \brief Main function
 * \author Dawid Babula
 * \email dbabula@adventurous.pl
 *
 * \par Copyright (C) Dawid Babula, 2018
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#define DEBUG_ENABLED DEBUG_MAIN_ENABLED
#define DEBUG_LEVEL DEBUG_MAIN_LEVEL
#define DEBUG_APP_ID "MAIN"

#include <stdint.h>
#include <stddef.h>
#include "system.h"
#include "usart.h"
#include "debug.h"
#include "ssd_mgr.h"
#include "system_timer.h"
#include "gpio.h"
#include "1wire.h"
#include "1wire_mgr.h"
#include "ds1302.h"
#include "app.h"
#include "PCB0001.h"
#include "input_mgr.h"
#include "stat.h"
#include "common.h"



static SSD_MGR_displays_t displays[4];

static inline void drivers_init(void)
{
    GPIO_configure(true);

    USART_config_t config =
    {
        .baudrate = USART_115200_BAUDRATE,
        .databits = USART_8_DATA_BITS,
        .parity = USART_NO_PARITY,
        .stopbits = USART_1_STOP_BITS,
    };

    USART_configure(&config);

    DEBUG_init(NULL);

    DS1302_configure();
    WIRE_configure();
}

static inline void modules_init(void)
{
    STAT_initialize();
    SSD_MGR_initialize();
    WIRE_MGR_initialize();
    INPUT_MGR_initialize();
}

int main(void)
{
    drivers_init();
    modules_init();

    SYSTEM_init();

    for(uint8_t i = 0u; i < ARRAY_SIZE(displays); i++)
    {
        SSD_MGR_display_create(&displays[i], pgm_read_byte(&displays_config[i]));
    }

    APP_initialize(displays, ARRAY_SIZE(displays));

    DEBUG(DL_INFO, "%s", "********************************\n");
    DEBUG(DL_INFO, "%s", "******* Mini Thermometer *******\n");
    DEBUG(DL_INFO, "%s", "********************************\n");

    while(true)
    {
        SYSTEM_main();
    }
}
