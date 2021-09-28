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
#include <avr/eeprom.h>
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

#define FAHRENHEIT_NUMERATOR        (9U)
#define FAHRENHEIT_DENOMINATOR      (5U)
#define FAHRENHEIT_OFFSET           (32)

#define MIN_TEMPERATURE             (-200)
#define MAX_TEMPERATURE             (200)

typedef enum
{
    IDLE,
    SPLASH_SCREEN_ON,
    SPLASH_SCREEN_WAIT,
    SET_TEMP_MODE_SCREEN,
    SET_TIME_MODE_SCREEN,
    SET_AM_PM_SCREEN,
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
static uint8_t EEMEM is_fahrenheit_eeprom = false;
static bool is_fahrenheit;

static const DS1302_datetime_t default_datetime =
{
    .year = EPOCH_YEAR,
    .month = EPOCH_MONTH,
    .date = EPOCH_DAY,
    .weekday = EPOCH_WEEKDAY,
    .hours = 12U,
    .min = 0U,
    .secs = 0U,
    .is_12h_mode = false,
    .is_pm = false,
};

static inline bool is_temperature_in_range(int16_t temperature, uint8_t scaling_factor)
{
    const int16_t min = MIN_TEMPERATURE*scaling_factor;
    const int16_t max = MAX_TEMPERATURE*scaling_factor;

    if((temperature >= min) && (temperature <= max))
    {
        return true;
    }

    return false;
}

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

static void set_blinking(uint8_t start, uint8_t end, bool value)
{
    for(uint8_t i = start; i < end; i++)
    {
        SSD_MGR_display_blink(&app_displays[i], value);
    }
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

    uint8_t tmp = eeprom_read_byte(&is_fahrenheit_eeprom);

    DS1302_get(&datetime);

    if((tmp != 0U) && (tmp != 1U))
    {
        return SET_TEMP_MODE_SCREEN;
    }

    is_fahrenheit = (bool)tmp;
    return TIME_SCREEN;
}

static APP_state_t handle_set_temp_mode_screen(APP_event_t event)
{
    APP_state_t ret = SET_TEMP_MODE_SCREEN;

    GPIO_write_pin(GPIO_CHANNEL_COLON, false);

    set_blinking(LEFT_DISP1_IDX, LEFT_DISP3_IDX, true);

    switch(event)
    {
        case MINUS_RELEASE:
        case PLUS_RELEASE:
            is_fahrenheit = !is_fahrenheit;
            break;
        case DOUBLE_PRESS:
            eeprom_write_byte(&is_fahrenheit_eeprom, (uint8_t)is_fahrenheit);
            set_blinking(LEFT_DISP1_IDX, LEFT_DISP3_IDX, false);
            tick = STATE_DELAY_1S;
            ret =  SET_TIME_MODE_SCREEN;
            break;
        default:
            break;
    }

    SSD_MGR_display_set(&app_displays[LEFT_DISP4_IDX], SSD_BLANK);
    SSD_MGR_display_set(&app_displays[LEFT_DISP3_IDX], is_fahrenheit ? SSD_DIGIT_3 : SSD_BLANK);
    SSD_MGR_display_set(&app_displays[LEFT_DISP2_IDX], is_fahrenheit ? SSD_DIGIT_2 : SSD_DIGIT_0);
    SSD_MGR_display_set(&app_displays[LEFT_DISP1_IDX], is_fahrenheit ? SSD_CHAR_F : SSD_CHAR_C);

    return ret;
}

static APP_state_t handle_set_time_mode_screen(APP_event_t event)
{
    APP_state_t ret = SET_TIME_MODE_SCREEN;

    set_blinking(LEFT_DISP1_IDX, LEFT_DISP3_IDX, true);

    switch(event)
    {
        case MINUS_RELEASE:
        case PLUS_RELEASE:
            datetime.is_12h_mode = !datetime.is_12h_mode;
            break;
        case DOUBLE_PRESS:
            ret = datetime.is_12h_mode ? SET_AM_PM_SCREEN : SET_HOURS_SCREEN;
            set_blinking(LEFT_DISP1_IDX, LEFT_DISP3_IDX, false);
            tick = STATE_DELAY_1S;
            break;
        default:
            break;
    }

    const uint8_t format = (datetime.is_12h_mode) ? 12U : 24U;

    SSD_MGR_display_set(&app_displays[LEFT_DISP4_IDX], SSD_BLANK);
    SSD_MGR_display_set(&app_displays[LEFT_DISP3_IDX], get_digit(format,1U));
    SSD_MGR_display_set(&app_displays[LEFT_DISP2_IDX], get_digit(format,0U));
    SSD_MGR_display_set(&app_displays[LEFT_DISP1_IDX], SSD_CHAR_h);

    return ret;
}

static APP_state_t handle_set_hours_screen(APP_event_t event)
{
    APP_state_t ret = SET_HOURS_SCREEN;

    set_blinking(LEFT_DISP3_IDX, LEFT_DISP4_IDX, true);

    switch(event)
    {
        case MINUS_LONG_PRESS:
            set_blinking(LEFT_DISP3_IDX, LEFT_DISP4_IDX, false);
            /* fallthrough */
        case MINUS_RELEASE:
            {
                const uint8_t type =
                    (datetime.is_12h_mode) ? DS1302_HOURS_12H : DS1302_HOURS_24H;
                datetime.hours = decrement_over_range(type, datetime.hours);
            }
            break;
        case PLUS_LONG_PRESS:
            set_blinking(LEFT_DISP3_IDX, LEFT_DISP4_IDX, false);
            /* fallthrough */
        case PLUS_RELEASE:
            {
                const uint8_t type =
                    (datetime.is_12h_mode) ? DS1302_HOURS_12H : DS1302_HOURS_24H;
                datetime.hours = increment_over_range(type, datetime.hours);
            }
            break;
        case DOUBLE_PRESS:
            ret = SET_MINUTES_SCREEN;
            set_blinking(LEFT_DISP3_IDX, LEFT_DISP4_IDX, false);
            tick = STATE_DELAY_1S;
            break;
        default:
            break;
    }

    uint16_t const to_display = (datetime.hours*100U) + datetime.min;
    set_to_display(to_display);
    return ret;
}

static APP_state_t handle_set_am_pm_screen(APP_event_t event)
{
    APP_state_t ret = SET_AM_PM_SCREEN;

    set_blinking(LEFT_DISP1_IDX, LEFT_DISP3_IDX, true);

    switch(event)
    {
        case MINUS_RELEASE:
        case PLUS_RELEASE:
            datetime.is_pm = !datetime.is_pm;
            break;
        case DOUBLE_PRESS:
            ret = SET_HOURS_SCREEN;
            set_blinking(LEFT_DISP1_IDX, LEFT_DISP3_IDX, false);
            tick = STATE_DELAY_1S;
            break;
        default:
            break;
    }

    SSD_MGR_display_set(&app_displays[LEFT_DISP4_IDX], SSD_BLANK);
    SSD_MGR_display_set(&app_displays[LEFT_DISP3_IDX], SSD_BLANK);
    SSD_MGR_display_set(&app_displays[LEFT_DISP2_IDX], SSD_BLANK);
    SSD_MGR_display_set(&app_displays[LEFT_DISP1_IDX],
            datetime.is_pm ? SSD_CHAR_P : SSD_CHAR_A);

    return ret;
}

static APP_state_t handle_set_minutes_screen(APP_event_t event)
{
    APP_state_t ret = SET_MINUTES_SCREEN;

    set_blinking(LEFT_DISP1_IDX, LEFT_DISP2_IDX, true);

    switch(event)
    {
        case MINUS_LONG_PRESS:
            set_blinking(LEFT_DISP1_IDX, LEFT_DISP2_IDX, false);
            /* fallthrough */
        case MINUS_RELEASE:
            datetime.min = decrement_over_range(DS1302_MINUTES, datetime.min);
            break;
        case PLUS_LONG_PRESS:
            set_blinking(LEFT_DISP1_IDX, LEFT_DISP2_IDX, false);
            /* fallthrough */
        case PLUS_RELEASE:
            datetime.min = increment_over_range(DS1302_MINUTES, datetime.min);
            break;
        case DOUBLE_PRESS:
            ret = TIME_SCREEN;
            DEBUG(DL_ERROR, "%d:%d:%d\n",datetime.hours, datetime.min, datetime.secs);
            DS1302_set_write_protection(false);
            DS1302_set(&datetime);
            set_blinking(LEFT_DISP1_IDX, LEFT_DISP2_IDX, false);
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
        datetime = default_datetime;
        return SET_TEMP_MODE_SCREEN;
    }

    if(tick != 0)
    {
        return TIME_SCREEN;
    }

    uint8_t mm = DS1302_get_minutes();
    uint8_t hh = DS1302_get_hours(datetime.is_12h_mode);

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
        datetime = default_datetime;
        return SET_TEMP_MODE_SCREEN;
    }

    if(tick != 0)
    {
        return TEMP_SCREEN;
    }

    int16_t temperature;
    const uint8_t scaling_factor = (1U << 4U);

    if(WIRE_MGR_get_temperature(&temperature) &&
            is_temperature_in_range(temperature, scaling_factor))
    {
        const uint8_t accuracy = scaling_factor/2U;
        int16_t temperature_converted = temperature;

        if(is_fahrenheit)
        {
            const uint8_t num = FAHRENHEIT_NUMERATOR;
            const uint8_t denom = FAHRENHEIT_DENOMINATOR;

            temperature_converted =
                ((num*temperature) + (scaling_factor*denom*FAHRENHEIT_OFFSET))/denom;
        }

        const bool is_negative = (temperature_converted < 0);

        int16_t temperature_rounded = is_negative ?
            (temperature_converted - accuracy) : (temperature_converted + accuracy);
        int16_t temperature_renormalized = (temperature_rounded/scaling_factor);

        GPIO_write_pin(GPIO_CHANNEL_COLON, false);

        uint16_t temp_abs = is_negative ?
            -temperature_renormalized : temperature_renormalized;

        SSD_MGR_display_set(&app_displays[LEFT_DISP1_IDX], is_fahrenheit ? SSD_CHAR_F: SSD_CHAR_C);
        uint8_t digit = get_digit(temp_abs, 0);
        SSD_MGR_display_set(&app_displays[LEFT_DISP2_IDX], digit);
        digit = get_digit(temp_abs, 1);
        SSD_MGR_display_set(&app_displays[LEFT_DISP3_IDX], digit);
        digit = get_digit(temp_abs, 2);
        SSD_MGR_display_set(&app_displays[LEFT_DISP4_IDX], digit);

        if(is_negative)
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

    return TEMP_SCREEN;
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
        case SET_TEMP_MODE_SCREEN:
            state = handle_set_temp_mode_screen(app_event);
            break;
        case SET_TIME_MODE_SCREEN:
            state = handle_set_time_mode_screen(app_event);
            break;
        case SET_HOURS_SCREEN:
            state = handle_set_hours_screen(app_event);
            break;
        case SET_AM_PM_SCREEN:
            state = handle_set_am_pm_screen(app_event);
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
    old_state = SET_TIME_MODE_SCREEN;
    app_displays = displays;
    app_displays_size = size;
}
