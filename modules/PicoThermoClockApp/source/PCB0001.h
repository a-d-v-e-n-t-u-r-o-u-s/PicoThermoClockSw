/*!
 * \file
 * \brief PCB0000 pin assignment file
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
#ifndef PCB0001_H
#define PCB0001_H

#include "gpio.h"
#include "ssd_mgr.h"
#include "1wire_mgr.h"
#include "input_mgr.h"
#include <avr/pgmspace.h>
/*!
 *
 * \addtogroup hardware
 * \ingroup MiniThermo
 * \brief Configures pin assignment for PCB0000
 */
#define F_CPU                   (16000000UL)

#define GPIO_CHANNEL_COLON          (0U)
#define GPIO_CHANNEL_DISPLAY1       (1U)
#define GPIO_CHANNEL_DISPLAY2       (2U)
#define GPIO_CHANNEL_DISPLAY3       (3U)
#define GPIO_CHANNEL_DISPLAY0       (4U)
#define GPIO_CHANNEL_SEGMENTG       (5U)
#define GPIO_CHANNEL_SEGMENTC       (6U)
#define GPIO_CHANNEL_SEGMENTD       (7U)
#define GPIO_CHANNEL_SEGMENTE       (8U)
#define GPIO_CHANNEL_SEGMENTA       (9U)
#define GPIO_CHANNEL_SEGMENTF       (10U)
#define GPIO_CHANNEL_SEGMENTB       (11U)
#define GPIO_CHANNEL_INPUT_PLUS     (12U)
#define GPIO_CHANNEL_INPUT_MINUS    (13U)
#define GPIO_CHANNEL_1WIRE          (14U)
#define GPIO_CHANNEL_RTC_CLK        (15U)
#define GPIO_CHANNEL_RTC_IO         (16U)
#define GPIO_CHANNEL_RTC_CE         (17U)

extern const GPIO_config_t gpio_config[18] PROGMEM;
extern const SSD_MGR_config_t ssd_config PROGMEM;
extern const uint8_t displays_config[4] PROGMEM;
extern const WIRE_MGR_config_t wire_mgr_config PROGMEM;
extern const uint8_t input_mgr_config[2] PROGMEM;

#define INPUT_MINUS_ID          (0U)
#define INPUT_PLUS_ID           (1U)
/*@{*/

/*@}*/
#endif /* end of PCB0000_H */
