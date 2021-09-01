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
#define DEBUG_ENABLED DEBUG_APP_ENABLED
#define DEBUG_LEVEL DEBUG_APP_LEVEL
#define DEBUG_APP_ID "APP"

#include "app.h"
#include "hardware.h"
#include "system.h"
#include "system_timer.h"
#include "1wire_mgr.h"
#include "debug.h"
#include "ds1302.h"
#include <util/delay.h>
#include <stdbool.h>
#include "input_mgr.h"

#define DISPLAY_SPLASH_VALUE        (8888u)

#define STATE_DELAY_1S              (1000u)
#define STATE_DELAY_5S              (5000u)

#define YEAR_2000_VALUE             (2000u)

#define LEFT_DISP1_IDX              (0u)
#define LEFT_DISP2_IDX              (1U)
#define LEFT_DISP3_IDX              (2u)
#define LEFT_DISP4_IDX              (3u)

#define TASK_PERIOD                 (100u)

#define EPOCH_YEAR                  (70U)
#define EPOCH_MONTH                 (1U)
#define EPOCH_DAY                   (1U)
#define EPOCH_WEEKDAY               (4U)

typedef enum
{
    IDLE,
    SPLASH_SCREEN_ON,
    SPLASH_SCREEN_WAIT,
    SET_HOURS_SCREEN,
    SET_MINUTES_SCREEN,
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
static SSD_MGR_displays_t *app_displays;
static uint8_t app_displays_size;
static uint8_t timer5s;
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
        default:
             return 0u;
     }
}

static void set_input_to_defaults(INPUT_MGR_event_t *event)
{
    event->id = UINT8_MAX;
    event->event = UINT8_MAX;
}

static void callback(void)
{
    if(tick != 0U)
    {
        tick--;
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

static uint8_t increment_over_range(uint8_t type, uint8_t value)
{
    const uint8_t max = DS1302_get_range_maximum(type);
    const uint8_t min = DS1302_get_range_minimum(type);
    uint8_t tmp = value;

    if(tmp == max)
    {
        tmp = min;
    }
    else
    {
        tmp++;
    }

    return tmp;
}

static uint8_t decrement_over_range(uint8_t type, uint8_t value)
{
    const uint8_t max = DS1302_get_range_maximum(type);
    const uint8_t min = DS1302_get_range_minimum(type);
    uint8_t tmp = value;

    if(tmp == min)
    {
        tmp = max;
    }
    else
    {
        tmp--;
    }

    return tmp;
}


APP_event_t get_app_event(void)
{
    APP_event_t ret = INVALID;

    if(INPUT_MGR_get_event(&new_input) == 0)
    {
        DEBUG(DL_VERBOSE, "Ev: [%d] \n",new_input.event);

        if((old_input.event == BUTTON_SHORT_PRESSED) &&
                (new_input.event == BUTTON_SHORT_PRESSED))
        {
            set_input_to_defaults(&new_input);
            ret = DOUBLE_PRESS;
        }
        else if((old_input.id == INPUT_MINUS_ID) &&
                (old_input.event == BUTTON_SHORT_PRESSED) &&
                (new_input.id == INPUT_MINUS_ID) &&
                (new_input.event == BUTTON_RELEASED))
        {
            ret = MINUS_RELEASE;
        }
        else if((old_input.id == INPUT_PLUS_ID) &&
                (old_input.event == BUTTON_SHORT_PRESSED) &&
                (new_input.id == INPUT_PLUS_ID) &&
                (new_input.event == BUTTON_RELEASED))
        {
            ret = PLUS_RELEASE;
        }
        else
        {
            /* ignore rest of cases */
        }


        old_input = new_input;
    }

    if((new_input.id == INPUT_MINUS_ID) &&
            (new_input.event == BUTTON_LONG_PRESSED))
    {
        ret = MINUS_LONG_PRESS;
    }

    if((new_input.id == INPUT_PLUS_ID) &&
            (new_input.event == BUTTON_LONG_PRESSED))
    {
        ret = PLUS_LONG_PRESS;
    }

    return ret;
}

static APP_state_t handle_splash_screen_on(void)
{
    GPIO_write_pin(GPIO_CHANNEL_COLON, true);
    set_to_display(DISPLAY_SPLASH_VALUE);
    tick = STATE_DELAY_5S;
    return SPLASH_SCREEN_WAIT;
}

static APP_state_t handle_splash_screen_wait(void)
{
    if(tick != 0)
    {
        return SPLASH_SCREEN_WAIT;
    }

    DS1302_get(&datetime);
    return TIME_SCREEN;
}

static APP_state_t handle_set_hours_screen(APP_event_t event)
{
    APP_state_t ret = SET_HOURS_SCREEN;

    SSD_MGR_display_blink(&app_displays[LEFT_DISP4_IDX], true);
    SSD_MGR_display_blink(&app_displays[LEFT_DISP3_IDX], true);

    switch(event)
    {
        case MINUS_LONG_PRESS:
            SSD_MGR_display_blink(&app_displays[LEFT_DISP4_IDX], false);
            SSD_MGR_display_blink(&app_displays[LEFT_DISP3_IDX], false);
            /* fallthrough */
        case MINUS_RELEASE:
            datetime.hours = decrement_over_range(DS1302_HOURS, datetime.hours);
            break;
        case PLUS_LONG_PRESS:
            SSD_MGR_display_blink(&app_displays[LEFT_DISP4_IDX], false);
            SSD_MGR_display_blink(&app_displays[LEFT_DISP3_IDX], false);
            /* fallthrough */
        case PLUS_RELEASE:
            datetime.hours = increment_over_range(DS1302_HOURS, datetime.hours);
            break;
        case DOUBLE_PRESS:
            ret = SET_MINUTES_SCREEN;
            SSD_MGR_display_blink(&app_displays[LEFT_DISP4_IDX], false);
            SSD_MGR_display_blink(&app_displays[LEFT_DISP3_IDX], false);
            tick = STATE_DELAY_1S;
            break;
        default:
            break;
    }

    uint16_t const to_display = (datetime.hours*100U) + datetime.min;
    set_to_display(to_display);
    return ret;
}

static APP_state_t handle_set_minutes_screen(APP_event_t event)
{
    APP_state_t ret = SET_MINUTES_SCREEN;

    SSD_MGR_display_blink(&app_displays[LEFT_DISP1_IDX], true);
    SSD_MGR_display_blink(&app_displays[LEFT_DISP2_IDX], true);

    switch(event)
    {
        case MINUS_LONG_PRESS:
            SSD_MGR_display_blink(&app_displays[LEFT_DISP1_IDX], false);
            SSD_MGR_display_blink(&app_displays[LEFT_DISP2_IDX], false);
            /* fallthrough */
        case MINUS_RELEASE:
            datetime.min = decrement_over_range(DS1302_MINUTES, datetime.min);
            break;
        case PLUS_LONG_PRESS:
            SSD_MGR_display_blink(&app_displays[LEFT_DISP1_IDX], false);
            SSD_MGR_display_blink(&app_displays[LEFT_DISP2_IDX], false);
            /* fallthrough */
        case PLUS_RELEASE:
            datetime.min = increment_over_range(DS1302_MINUTES, datetime.min);
            break;
        case DOUBLE_PRESS:
            ret = TIME_SCREEN;
            datetime.secs = 0U;
            datetime.year = EPOCH_YEAR;
            datetime.month = EPOCH_MONTH;
            datetime.date = EPOCH_DAY;
            datetime.weekday = EPOCH_WEEKDAY;
            DS1302_set_write_protection(false);
            DS1302_set(&datetime);
            SSD_MGR_display_blink(&app_displays[LEFT_DISP1_IDX], false);
            SSD_MGR_display_blink(&app_displays[LEFT_DISP2_IDX], false);
            tick = STATE_DELAY_1S;
            break;
        default:
            break;
    }

    uint16_t const to_display = datetime.hours*100U + datetime.min;
    set_to_display(to_display);
    return ret;
}


static APP_state_t handle_time_screen(APP_event_t event)
{
    if(event == DOUBLE_PRESS)
    {
        GPIO_write_pin(GPIO_CHANNEL_COLON, false);
        return SET_HOURS_SCREEN;
    }

    if(tick != 0)
    {
        return TIME_SCREEN;
    }

    uint8_t mm = DS1302_get_minutes();
    uint8_t hh = DS1302_get_hours();

    DEBUG(DL_ERROR, "%d:%d:%d\n",hh, mm, DS1302_get_seconds());
    set_to_display(hh*100 + mm);
    tick = STATE_DELAY_1S;
    GPIO_toggle_pin(GPIO_CHANNEL_COLON);
    timer5s++;

    if(timer5s > 20U)
    {
        timer5s = 0u;
        tick = STATE_DELAY_1S;
        return TEMP_SCREEN;
    }
    else
    {
        return TIME_SCREEN;
    }
}

static APP_state_t handle_temp_screen(APP_event_t event)
{
    if(event == DOUBLE_PRESS)
    {
        GPIO_write_pin(GPIO_CHANNEL_COLON, false);
        return SET_HOURS_SCREEN;
    }

    if(tick != 0)
    {
        return TEMP_SCREEN;
    }

    uint16_t temperature;

    if(WIRE_MGR_get_temperature(&temperature))
    {
        int8_t temp = (int8_t)(temperature >> 4u);
        const bool is_round = ((temperature & ( 1 << 3u)) != 0);
        const bool is_negative = (temp < 0);

        GPIO_write_pin(GPIO_CHANNEL_COLON, false);

        if(is_round)
        {
            temp++;
        }

        uint8_t temp_abs = is_negative ? -temp : temp;
        SSD_MGR_display_set(&app_displays[LEFT_DISP1_IDX], SSD_CHAR_C);
        uint8_t digit = get_digit(temp_abs, 0);
        SSD_MGR_display_set(&app_displays[LEFT_DISP2_IDX], digit);
        digit = get_digit(temp_abs, 1);
        SSD_MGR_display_set(&app_displays[LEFT_DISP3_IDX], digit);

        if(!is_negative)
        {
            SSD_MGR_display_set(&app_displays[LEFT_DISP4_IDX], SSD_BLANK);
        }
        else
        {
            SSD_MGR_display_set(&app_displays[LEFT_DISP4_IDX], SSD_SYMBOL_MINUS);
        }
    }
    else
    {
        SSD_MGR_display_set(&app_displays[LEFT_DISP4_IDX], SSD_BLANK);
        SSD_MGR_display_set(&app_displays[LEFT_DISP3_IDX], SSD_CHAR_E);
        SSD_MGR_display_set(&app_displays[LEFT_DISP2_IDX], SSD_CHAR_r);
        SSD_MGR_display_set(&app_displays[LEFT_DISP1_IDX], SSD_CHAR_r);
    }


    tick = STATE_DELAY_1S;
    timer5s++;

    if(timer5s > 5U)
    {
        timer5s = 0;
        tick = STATE_DELAY_1S;
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
        DEBUG(DL_VERBOSE, "Old [%d] -> New [%d]\n", old_state, state);
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
        case SET_HOURS_SCREEN:
            state = handle_set_hours_screen(app_event);
            break;
        case SET_MINUTES_SCREEN:
            state = handle_set_minutes_screen(app_event);
            break;
        case TIME_SCREEN:
            state = handle_time_screen(app_event);
            break;
        case TEMP_SCREEN:
            state = handle_temp_screen(app_event);
            break;
        default:
            ASSERT(false);
            break;
    }
}

void APP_initialize(SSD_MGR_displays_t *displays, uint8_t size)
{
    SYSTEM_register_task(app_main, TASK_PERIOD);
    SYSTEM_timer_register(callback);
    set_input_to_defaults(&old_input);
    old_state = SET_HOURS_SCREEN;
    app_displays = displays;
    app_displays_size = size;
}
