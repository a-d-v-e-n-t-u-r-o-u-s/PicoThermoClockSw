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

static inline void drivers_init(void)
{
    GPIO_configure(true);

    GPIO_config_pin(COLON_PORT, COLON_PIN, GPIO_OUTPUT_PUSH_PULL);

    USART_config_t config =
    {
        .baudrate = USART_115200_BAUDRATE,
        .databits = USART_8_DATA_BITS,
        .parity = USART_NO_PARITY,
        .stopbits = USART_1_STOP_BITS,
    };

    (void) USART_configure(&config);

    DEBUG_init(NULL);

    const DS1302_config_t rtc_config =
    {
        .pins =
        {
            [0] = { RTC_CLK_PORT, RTC_CLK_PIN },
            [1] = { RTC_IO_PORT , RTC_IO_PIN  },
            [2] = { RTC_CE_PORT , RTC_CE_PIN  },
        }
    };

    if(DS1302_configure(&rtc_config))
    {
        DEBUG_output("DS1302 [fail]\n");
    }

    if(WIRE_configure(WIRE_DATA_PORT, WIRE_DATA_PIN) != 0)
    {
        DEBUG_output("WIRE [fail]\n");
    }
}

static const INPUT_MGR_config_t input_mgr_config[2] =
{
    [0] =
    {
        .id = INPUT_MINUS_ID,
        .gpio_config = { INPUT_MINUS_PORT, INPUT_MINUS_PIN },
    },
    [1] =
    {
        .id = INPUT_PLUS_ID,
        .gpio_config = { INPUT_PLUS_PORT, INPUT_PLUS_PIN },
    }
};

static inline void modules_init(void)
{
    uint8_t displays[4][2] =
    {
        [0] = { DISPLAY0_PORT, DISPLAY0_PIN },
        [1] = { DISPLAY1_PORT, DISPLAY1_PIN },
        [2] = { DISPLAY2_PORT, DISPLAY2_PIN },
        [3] = { DISPLAY3_PORT, DISPLAY3_PIN },
    };

    SSD_MGR_config_t ssd_mgr_config =
    {
        .seg_config =
        {
            [0] = { SEGMENTA_PORT, SEGMENTA_PIN },
            [1] = { SEGMENTB_PORT, SEGMENTB_PIN },
            [2] = { SEGMENTC_PORT, SEGMENTC_PIN },
            [3] = { SEGMENTD_PORT, SEGMENTD_PIN },
            [4] = { SEGMENTE_PORT, SEGMENTE_PIN },
            [5] = { SEGMENTF_PORT, SEGMENTF_PIN },
            [6] = { SEGMENTG_PORT, SEGMENTG_PIN },
            [7] = { SEGMENTDP_PORT, SEGMENTDP_PIN },
        },
        .disp_config = displays,
        .disp_config_size = 4u,
        .is_segment_mode = false,
        .is_disp_inverted_logic = true,
        .is_segment_inverted_logic = false,
    };


    if(SSD_MGR_initialize(&ssd_mgr_config) != 0)
    {
        DEBUG_output("SSD MGR [fail]\n");
    }

    if(WIRE_MGR_initialize() != 0)
    {
        DEBUG_output("WIRE MGR [fail]\n");
    }

    if(INPUT_MGR_initialize(input_mgr_config, sizeof(input_mgr_config)/sizeof(input_mgr_config[0])) != 0)
    {
        DEBUG_output("INPUT MGR [fail]\n");
    }
}

int main(void)
{
    drivers_init();
    modules_init();

    SYSTEM_init();

    if(APP_initialize() != 0)
    {
        DEBUG_output("APP [fail]\n");
    }

    DEBUG_output("********************************\n");
    DEBUG_output("******* Mini Thermometer *******\n");
    DEBUG_output("********************************\n");

    while(true)
    {
        SYSTEM_main();
    }
}
