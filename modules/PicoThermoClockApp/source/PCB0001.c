/*!
 * \file
 * \brief PCB0001 pin assignment file
 * \author Dawid Babula
 * \email dbabula@adventurous.pl
 *
 * \par Copyright (C) Dawid Babula, 2021
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

#include "PCB0001.h"

// sonarcloud-disable S103
const GPIO_config_t gpio_config[18] PROGMEM =
{
    [GPIO_CHANNEL_COLON] =          { .port = GPIO_PORTB, .pin = 0U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_DISPLAY1] =       { .port = GPIO_PORTB, .pin = 1U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_DISPLAY2] =       { .port = GPIO_PORTB, .pin = 2U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_DISPLAY3] =       { .port = GPIO_PORTB, .pin = 3U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_DISPLAY0] =       { .port = GPIO_PORTB, .pin = 4U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_SEGMENTG] =       { .port = GPIO_PORTB, .pin = 5U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_SEGMENTC] =       { .port = GPIO_PORTC, .pin = 0U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_SEGMENTD] =       { .port = GPIO_PORTC, .pin = 1U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_SEGMENTE] =       { .port = GPIO_PORTC, .pin = 2U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_SEGMENTA] =       { .port = GPIO_PORTC, .pin = 3U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_SEGMENTF] =       { .port = GPIO_PORTC, .pin = 4U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_SEGMENTB] =       { .port = GPIO_PORTC, .pin = 5U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_INPUT_PLUS] =     { .port = GPIO_PORTD, .pin = 2U, .mode = GPIO_INPUT_FLOATING,   .init_value = false },
    [GPIO_CHANNEL_INPUT_MINUS] =    { .port = GPIO_PORTD, .pin = 3U, .mode = GPIO_INPUT_FLOATING,   .init_value = false },
    [GPIO_CHANNEL_1WIRE]        =   { .port = GPIO_PORTD, .pin = 4U, .mode = GPIO_INPUT_FLOATING,   .init_value = false },
    [GPIO_CHANNEL_RTC_CLK]      =   { .port = GPIO_PORTD, .pin = 5U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_RTC_IO]       =   { .port = GPIO_PORTD, .pin = 6U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
    [GPIO_CHANNEL_RTC_CE]       =   { .port = GPIO_PORTD, .pin = 7U, .mode = GPIO_OUTPUT_PUSH_PULL, .init_value = false },
};
// sonarcloud-enable S103

const SSD_MGR_config_t ssd_config PROGMEM =
{
    .is_segments_inverted = false,
    .is_displays_inverted = true
};

const uint8_t displays_config[] PROGMEM =
{
    [0] = GPIO_CHANNEL_DISPLAY0,
    [1] = GPIO_CHANNEL_DISPLAY1,
    [2] = GPIO_CHANNEL_DISPLAY2,
    [3] = GPIO_CHANNEL_DISPLAY3,
};

const WIRE_MGR_config_t wire_mgr_config PROGMEM =
{
    .is_crc = true,
    .is_fake_allowed = true,
    .resolution = WIRE_9BIT_RESOLUTION,
};

const uint8_t input_mgr_config[2] PROGMEM =
{
    [0] = GPIO_CHANNEL_INPUT_MINUS,
    [1] = GPIO_CHANNEL_INPUT_PLUS,
};

