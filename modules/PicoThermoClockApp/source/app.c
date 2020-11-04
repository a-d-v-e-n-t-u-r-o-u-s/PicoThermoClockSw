/*!
 * \file
 * \brief Application implementation file
 * \author Dawid Babula
 * \email dbabula@adventurous.pl
 *
 * \par Copyright (C) Dawid Babula, 2020
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
#include "app.h"
#include "system.h"
#include "system_timer.h"
#include "1wire_mgr.h"
#include "debug.h"
#include "ds1302.h"
#include <util/delay.h>
#include <stdbool.h>
#include "PCB0001.h"
#include "input_mgr.h"

typedef enum
{
    IDLE,
    SPLASH_SCREEN_ON,
    SPLASH_SCREEN_WAIT,
    SET_YEAR_SCREEN,
    SET_MONTH_SCREEN,
    SET_DAY_SCREEN,
    SET_TIME_SCREEN,
    TIME_SCREEN,
    TEMP_SCREEN,
} APP_state_t;

typedef enum
{
    INVALID,
    MINUS_RELEASE,
    MINUS_LONG_PRESS,
    PLUS_LONG_PRESS,
    PLUS_RELEASE,
    DOUBLE_PRESS,
} APP_event_t;

static uint32_t tick;

static APP_state_t state;
static APP_state_t old_state;
static INPUT_MGR_event_t new_input;
static INPUT_MGR_event_t old_input;
static bool is_colon_active;
static SSD_MGR_displays_t *app_displays;
static uint8_t app_displays_size;

static DS1302_datetime_t datetime;

static uint8_t get_digit(uint16_t value, uint8_t position)
{
     switch(position)
     {
         case 0:
             return value%10u;
         case 1:
             return (value/10u)%10u;
         case 2:
             return (value/100u)%10u;
         case 3:
             return (value/1000u)%10u;
     }

     return 0U;
}

static void set_input_to_defaults(INPUT_MGR_event_t *event)
{
    event->id = UINT8_MAX;
    event->event = UINT8_MAX;
}

static void set_datetime_to_defaults(DS1302_datetime_t *data)
{
    data->year = 0u;
    data->month = 12u;
    data->date = 1u;
    data->weekday = 7u;
    data->hours = 12u;
    data->min = 0u;
    data->secs = 0u;
    data->format = 0u;
}

static uint16_t get_converted_fraction(uint16_t value)
{
    uint16_t ret = 0;
    if((value & (1 << 3)) != 0)
    {
        ret += 50;
    }
    if((value & (1 << 2)) != 0)
    {
        ret += 25;
    }

    if((value & (1 << 1)) != 0)
    {
        ret += 13;
    }

    if((value & (1 << 0)) != 0)
    {
        ret += 7;
    }

    return ret;
}

static void callback(void)
{
    static uint32_t colon_tick;
    static bool is_colon;

    if(is_colon_active)
    {
        if(SYSTEM_timer_tick_difference(colon_tick, SYSTEM_timer_get_tick()) > 1000)
        {
            colon_tick = SYSTEM_timer_get_tick();
            if(is_colon)
            {
                GPIO_write_pin(COLON_PORT, COLON_PIN, 0U);
                is_colon = false;
            }
            else
            {
                GPIO_write_pin(COLON_PORT, COLON_PIN, 1U);
                is_colon = true;
            }
        }
    }
}

static void set_to_display(uint16_t value)
{
    for(uint8_t i = 0u; i < app_displays_size; i++)
    {
        uint8_t digit = get_digit(value, i);
        SSD_MGR_display_set(&app_displays[i], digit);
    }
}

static void set_temp_to_display(uint8_t temp)
{
    SSD_MGR_display_set(&app_displays[0], SSD_CHAR_C);
    uint8_t digit = get_digit(temp, 0);
    SSD_MGR_display_set(&app_displays[1], digit);
    digit = get_digit(temp, 1);
    SSD_MGR_display_set(&app_displays[2], digit);
    SSD_MGR_display_set(&app_displays[3], SSD_BLANK);
}

APP_event_t get_app_event(void)
{
    APP_event_t ret = INVALID;

    if(INPUT_MGR_get_event(&new_input) == 0)
    {
        DEBUG_output("Ev: [%d] \n",new_input.event);

        if(old_input.event == BUTTON_SHORT_PRESSED &&
                new_input.event == BUTTON_SHORT_PRESSED)
        {
            set_input_to_defaults(&new_input);
            ret = DOUBLE_PRESS;
        }
        else if(old_input.id == INPUT_MINUS_ID &&
                old_input.event == BUTTON_SHORT_PRESSED &&
                new_input.id == INPUT_MINUS_ID &&
                new_input.event == BUTTON_RELEASED)
        {
            ret = MINUS_RELEASE;
        }
        else if(old_input.id == INPUT_PLUS_ID &&
                old_input.event == BUTTON_SHORT_PRESSED &&
                new_input.id == INPUT_PLUS_ID &&
                new_input.event == BUTTON_RELEASED)
        {
            ret = PLUS_RELEASE;
        }


        old_input = new_input;
    }

    if(new_input.id == INPUT_MINUS_ID &&
            new_input.event == BUTTON_LONG_PRESSED)
    {
        ret = MINUS_LONG_PRESS;
    }

    if(new_input.id == INPUT_PLUS_ID &&
            new_input.event == BUTTON_LONG_PRESSED)
    {
        ret = PLUS_LONG_PRESS;
    }

    return ret;
}

static APP_state_t handle_splash_screen_on(void)
{
    datetime.year = 0U;
    datetime.month = 12u;
    datetime.date = 30u;
    datetime.weekday = 7u;
    datetime.hours = 12u;
    datetime.min = 0u;
    GPIO_write_pin(COLON_PORT, COLON_PIN, 1U);
    set_to_display(8888u);
    tick = SYSTEM_timer_get_tick();
    return SPLASH_SCREEN_WAIT;
}

static APP_state_t handle_splash_screen_wait(void)
{
    if(SYSTEM_timer_tick_difference(tick, SYSTEM_timer_get_tick()) > 5000)
    {
        GPIO_write_pin(COLON_PORT, COLON_PIN, 0U);
        return SET_YEAR_SCREEN;
    }
    else
    {
        return SPLASH_SCREEN_WAIT;
    }
}

static APP_state_t handle_set_year_screen(APP_event_t event)
{
    APP_state_t ret = SET_YEAR_SCREEN;

    switch(event)
    {
        case MINUS_LONG_PRESS:
        case MINUS_RELEASE:
            if(datetime.year == 0U)
            {
                datetime.year = 99U;
            }
            else
            {
                datetime.year--;
            }
            break;
        case PLUS_LONG_PRESS:
        case PLUS_RELEASE:
            datetime.year++;
            datetime.year %=100u;
            break;
        case DOUBLE_PRESS:
            ret = SET_MONTH_SCREEN;
            break;
        case INVALID:
        default:
            break;
    }

    set_to_display(datetime.year + 2000u);
    return ret;
}

static APP_state_t handle_set_month_screen(APP_event_t event)
{
    APP_state_t ret = SET_MONTH_SCREEN;

    switch(event)
    {
        case MINUS_LONG_PRESS:
        case MINUS_RELEASE:
            if(datetime.month == 1)
            {
                datetime.month = 12u;
            }
            else
            {
                datetime.month--;
            }
            break;
        case PLUS_LONG_PRESS:
        case PLUS_RELEASE:
            if(datetime.month == 12)
            {
                datetime.month = 1;
            }
            else
            {
                datetime.month++;
            }
            break;
        case DOUBLE_PRESS:
            ret = SET_DAY_SCREEN;
            break;
        case INVALID:
        default:
            break;
    }

    set_to_display(datetime.month);
    return ret;
}

static APP_state_t handle_set_day_screen(APP_event_t event)
{
    APP_state_t ret = SET_DAY_SCREEN;

    switch(event)
    {
        case MINUS_LONG_PRESS:
        case MINUS_RELEASE:
            if(datetime.date == 1)
            {
                datetime.date = 30u;
            }
            else
            {
                datetime.date--;
            }
            break;
        case PLUS_LONG_PRESS:
        case PLUS_RELEASE:
            if(datetime.date == 30)
            {
                datetime.date = 1;
            }
            else
            {
                datetime.date++;
            }
            break;
        case DOUBLE_PRESS:
            ret = SET_TIME_SCREEN;
            break;
        case INVALID:
        default:
            break;
    }

    set_to_display(datetime.date);
    return ret;
}

static APP_state_t handle_set_time_screen(APP_event_t event)
{
    APP_state_t ret = SET_TIME_SCREEN;

    switch(event)
    {
        case MINUS_LONG_PRESS:
        case MINUS_RELEASE:
            datetime.hours++;
            datetime.hours %= 24u;
            break;
        case PLUS_LONG_PRESS:
        case PLUS_RELEASE:
            datetime.min++;
            datetime.min %= 60u;
            break;
        case DOUBLE_PRESS:
            ret = TIME_SCREEN;
            break;
        case INVALID:
        default:
            break;
    }

    datetime.secs = 0u;

    uint16_t const to_display = datetime.hours*100U + datetime.min;

    DS1302_set_write_protection(false);
    DS1302_set(&datetime);
    set_to_display(to_display);
    tick = SYSTEM_timer_get_tick();
    is_colon_active = true;
    return ret;
}

static uint8_t timer5s;

static APP_state_t handle_time_screen(void)
{
    if(SYSTEM_timer_tick_difference(tick, SYSTEM_timer_get_tick()) > 1000)
    {
        uint8_t ss = DS1302_get_seconds();
        uint8_t mm = DS1302_get_minutes();
        uint8_t hh = DS1302_get_hours();

        DEBUG_output("%d:%d:%d\n",hh, mm, ss);
        set_to_display(hh*100 + mm);
        tick = SYSTEM_timer_get_tick();
        timer5s++;
    }

    if(timer5s > 5U)
    {
        timer5s = 0u;
        tick = SYSTEM_timer_get_tick();
        is_colon_active = false;
        GPIO_write_pin(COLON_PORT, COLON_PIN, 0U);
        return TEMP_SCREEN;
    }
    else
    {
        return TIME_SCREEN;
    }
}

static APP_state_t handle_temp_screen(void)
{
    if(SYSTEM_timer_tick_difference(tick, SYSTEM_timer_get_tick()) > 1000)
    {
        uint16_t temperature = WIRE_MGR_get_temperature();
        uint16_t fraction = get_converted_fraction(temperature % 16);
        uint16_t integer = (temperature / 16);
        uint16_t display = integer * 100;
        display += fraction;
        DEBUG_output("Temp %d,%d \n", integer, fraction);
        //set_to_display(display);
        set_temp_to_display(integer);
        tick = SYSTEM_timer_get_tick();
        timer5s++;
    }

    if(timer5s > 5U)
    {
        timer5s = 0;
        tick = SYSTEM_timer_get_tick();
        is_colon_active = true;
        return TIME_SCREEN;
    }
    else
    {
        return TEMP_SCREEN;
    }
}


static void app_main(void)
{
    if(old_state != state)
    {
        DEBUG_output("Old [%d] -> New [%d]\n", old_state, state);
        old_state = state;
    }

    APP_event_t app_event = get_app_event();

    switch(state)
    {
        case IDLE:
            state = SPLASH_SCREEN_ON;
            break;
        case SPLASH_SCREEN_ON:
            state = handle_splash_screen_on();
            break;
        case SPLASH_SCREEN_WAIT:
            state = handle_splash_screen_wait();
            break;
        case SET_YEAR_SCREEN:
            state = handle_set_year_screen(app_event);
            break;
        case SET_MONTH_SCREEN:
            state = handle_set_month_screen(app_event);
            break;
        case SET_DAY_SCREEN:
            state = handle_set_day_screen(app_event);
            break;
        case SET_TIME_SCREEN:
            state = handle_set_time_screen(app_event);
            break;
        case TIME_SCREEN:
            state = handle_time_screen();
            break;
        case TEMP_SCREEN:
            state = handle_temp_screen();
            break;
    }
}

int8_t APP_initialize(SSD_MGR_displays_t *displays, uint8_t size)
{
    if(SYSTEM_register_task(app_main, 100u) != 0)
    {
        return -1;
    }

    SYSTEM_timer_register(callback);
    set_input_to_defaults(&old_input);
    set_datetime_to_defaults(&datetime);
    old_state = SET_YEAR_SCREEN;
    app_displays = displays;
    app_displays_size = size;
    return 0;
}
