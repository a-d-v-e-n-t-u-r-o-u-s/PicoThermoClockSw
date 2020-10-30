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
#include "ssd_mgr.h"
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

static bool is_minus_pressed;
static bool is_minus_long_pressed;
static bool is_plus_long_pressed;
static bool is_plus_pressed;

static uint32_t tick;
static uint16_t counter;
static uint16_t old_counter = UINT16_MAX;

static APP_state_t state;
static APP_state_t old_state = SET_YEAR_SCREEN;
static uint8_t minus_event = UINT8_MAX;
static uint8_t plus_event = UINT8_MAX;
static DS1302_datetime_t datetime =
{
    .year = 0u,
    .month = 1u,
    .date = 1u,
    .weekday = 1u,
    .hours = 12u,
    .min = 0u,
    .secs = 0u,
    .format = 0u,
};

static inline is_short_press(uint8_t event)
{
    return (event == BUTTON_SHORT_PRESSED);
}

static inline is_release(uint8_t event)
{
    return (event == BUTTON_RELEASED);
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
    /*
     *if(SYSTEM_timer_tick_difference(tick, SYSTEM_timer_get_tick()) > 5000)
     *{
     *    tick = SYSTEM_timer_get_tick();
     *    counter++;
     *}
     */
}

static APP_state_t handle_splash_screen_on(void)
{
    GPIO_write_pin(COLON_PORT, COLON_PIN, 1U);
    SSD_MGR_set(8888U);
    tick = SYSTEM_timer_get_tick();
    return SPLASH_SCREEN_WAIT;
}

static APP_state_t handle_splash_screen_wait(void)
{
    if(SYSTEM_timer_tick_difference(tick, SYSTEM_timer_get_tick()) > 5000)
    {
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

    SSD_MGR_set(datetime.year + 2000u);
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

    SSD_MGR_set(datetime.month);
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

    SSD_MGR_set(datetime.date);
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
    SSD_MGR_set(to_display);
    tick = SYSTEM_timer_get_tick();
    return ret;
}

static APP_state_t handle_time_screen(void)
{
    if(SYSTEM_timer_tick_difference(tick, SYSTEM_timer_get_tick()) > 1000)
    {
        uint8_t ss = DS1302_get_seconds();
        uint8_t mm = DS1302_get_minutes();
        uint8_t hh = DS1302_get_hours();

        DEBUG_output("%d:%d:%d\n",hh, mm, ss);
        SSD_MGR_set(hh*100 + mm);
        tick = SYSTEM_timer_get_tick();
    }

    return TIME_SCREEN;
}


static void app_main(void)
{
    if(old_state != state)
    {
        DEBUG_output("Old [%d] -> New [%d]\n", old_state, state);
        old_state = state;
    }

    INPUT_MGR_event_t event;
    APP_event_t app_event = INVALID;

    if(INPUT_MGR_get_event(&event) == 0)
    {
        switch(event.event)
        {
            case BUTTON_SHORT_PRESSED:
                if(event.id == INPUT_MINUS_ID)
                {
                    is_minus_pressed = true;

                    if(is_plus_pressed)
                    {
                        app_event = DOUBLE_PRESS;
                    }
                }
                else
                {
                    is_plus_pressed = true;
                    if(is_minus_pressed)
                    {
                        app_event = DOUBLE_PRESS;
                    }
                }
                break;
            case BUTTON_LONG_PRESSED:
                if(event.id == INPUT_MINUS_ID)
                {
                    is_minus_long_pressed = true;
                }

                if(event.id == INPUT_PLUS_ID)
                {
                    is_plus_long_pressed = true;
                }

                is_minus_pressed = false;
                is_plus_pressed = false;
                break;
            case BUTTON_RELEASED:
                if(event.id == INPUT_MINUS_ID)
                {
                    app_event = MINUS_RELEASE;
                    is_minus_pressed = false;
                    is_minus_long_pressed = false;
                }
                else
                {
                    app_event = PLUS_RELEASE;
                    is_plus_pressed = false;
                    is_plus_long_pressed = false;
                }
                break;
        }
    }

    if(is_minus_long_pressed)
    {
        app_event = MINUS_LONG_PRESS;
    }

    if(is_plus_long_pressed)
    {
        app_event = PLUS_LONG_PRESS;
    }

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
    }

#if 0
    static bool flag;

    if(false)
    {
        /*
         *hours += (counter/3600u) % 24u;
         *minutes += (counter/60u) % 60u;
         *SSD_MGR_set(hours*100 + minutes);
         *flag = true;
         */
        uint8_t ss = DS1302_get_seconds();
        uint8_t mm = DS1302_get_minutes();
        uint8_t hh = DS1302_get_hours();

        DEBUG_output("%d:%d:%d\n",hh, mm, ss);
        SSD_MGR_set(mm*100 + ss);

        if(flag)
        {
            GPIO_write_pin(COLON_PORT, COLON_PIN, 0U);
        }
        else
        {
            GPIO_write_pin(COLON_PORT, COLON_PIN, 1U);
        }
    }
    else
    {
        uint16_t temperature = WIRE_MGR_get_temperature();
        uint16_t fraction = get_converted_fraction(temperature % 16);
        uint16_t integer = (temperature / 16);
        uint16_t display = integer * 100;
        display += fraction;
        DEBUG_output("Temp %d,%d \n", integer, fraction);
        SSD_MGR_set(display);
    }

    flag = !flag;

    if(old_counter != counter)
    {
        old_counter = counter;
    }
#endif
}

int8_t APP_initialize(void)
{
    if(SYSTEM_register_task(app_main, 100u) != 0)
    {
        return -1;
    }

    SYSTEM_timer_register(callback);
    return 0;
}
