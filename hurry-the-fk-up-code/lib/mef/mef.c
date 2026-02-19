#include "mef.h"
#include "rgb.h"
#include "buzzer.h"
#include "uart_debug.h"
#include "button.h"

#define TURN_DURATION_MS 120000
#define TURN_TICKS (TURN_DURATION_MS / 10)

typedef enum
{
    STATE_IDLE = 0,
    STATE_INIT,
    STATE_TURN_IN_PROCESS,
    STATE_TURN_TIME_FINISHED,
} mef_states_t;

typedef struct
{
    uint16_t elapsed_ticks;
    mef_states_t current_state;
    uint8_t last_button_state;
    uint16_t alarm_tick;
    uint8_t buzzer_duration_ticks;
    uint8_t one_minute_beep_done;
} mef_data_t;

static mef_data_t mef_state = {0, STATE_IDLE, BUTTON_RELEASED, 0, 0, 0};

static void alarm_pattern(void)
{
    static const struct
    {
        uint8_t on_ticks;
        uint8_t off_ticks;
    } pattern[] = {
        {5, 5},   // short beep
        {5, 5},   // short beep
        {10, 10}, // long beep
        {5, 5},   // short beep
        {5, 5},   // short beep
        {10, 20}  // long beep with longer pause
    };

    static const uint8_t pattern_len = 6;
    uint8_t phase = (mef_state.alarm_tick / 1) % (pattern_len * 40);

    for (uint8_t i = 0; i < pattern_len; i++)
    {
        if (phase < pattern[i].on_ticks)
        {
            buzzer_sound();
            return;
        }
        phase -= pattern[i].on_ticks;

        if (phase < pattern[i].off_ticks)
        {
            buzzer_silence();
            return;
        }
        phase -= pattern[i].off_ticks;
    }
}

static void fade_rgb(void)
{
    uint8_t red = 0;
    uint8_t green = 255;
    uint8_t blue = 0;

    uint16_t quarter_ticks = TURN_TICKS / 4;

    if (mef_state.elapsed_ticks < quarter_ticks)
    {
        // Green to Yellow: R goes 0->255, G stays 255
        uint32_t progress = ((uint32_t)mef_state.elapsed_ticks * 255U) / quarter_ticks;
        red = (uint8_t)progress;
        green = 255;
    }
    else if (mef_state.elapsed_ticks < 2 * quarter_ticks)
    {
        // Yellow to Orange: R stays 255, G goes 255->165
        uint32_t step = mef_state.elapsed_ticks - quarter_ticks;
        uint32_t progress = (step * 90U) / quarter_ticks;
        red = 255;
        green = 255 - (uint8_t)progress;
    }
    else if (mef_state.elapsed_ticks < 3 * quarter_ticks)
    {
        // Orange to Red: R stays 255, G goes 165->0
        uint32_t step = mef_state.elapsed_ticks - 2 * quarter_ticks;
        uint32_t progress = (step * 165U) / quarter_ticks;
        red = 255;
        green = 165 - (uint8_t)progress;
    }
    else
    {
        // Red: stay at 255, 0, 0
        red = 255;
        green = 0;
    }

    led_set_value(RED, red);
    led_set_value(GREEN, green);
    led_set_value(BLUE, blue);
}

void mef_init(void)
{
    uart_debug_init();
    led_setup();
    buzzer_setup();
    button_setup();
    button_update();

    uart_debug_printf("System initialized. Press button to start fade.\n");

    mef_state.current_state = STATE_IDLE;
    mef_state.elapsed_ticks = 0;
    mef_state.last_button_state = BUTTON_RELEASED;
    mef_state.alarm_tick = 0;
    mef_state.buzzer_duration_ticks = 0;
    mef_state.one_minute_beep_done = 0;
}

void mef_update_10ms(void)
{
    button_update();
    uint8_t current_button = button_get_state();
    uint8_t button_pressed = (mef_state.last_button_state == BUTTON_RELEASED) &&
                             (current_button == BUTTON_PRESSED);
    mef_state.last_button_state = current_button;

    // Handle buzzer duration countdown
    if (mef_state.buzzer_duration_ticks > 0)
    {
        buzzer_sound();
        mef_state.buzzer_duration_ticks--;
    }
    else
    {
        buzzer_silence();
    }

    switch (mef_state.current_state)
    {
    case STATE_IDLE:
        if (button_pressed)
        {
            mef_state.current_state = STATE_TURN_IN_PROCESS;
            mef_state.elapsed_ticks = 0;
            mef_state.buzzer_duration_ticks = 10;
            mef_state.one_minute_beep_done = 0;
            uart_debug_printf("Turn started!\n");
        }
        break;

    case STATE_TURN_IN_PROCESS:
        fade_rgb();
        mef_state.elapsed_ticks++;

        // Check for 1-minute mark (6000 ticks = 60 seconds)
        if (mef_state.elapsed_ticks == 6000 && !mef_state.one_minute_beep_done)
        {
            mef_state.buzzer_duration_ticks = 10;
            mef_state.one_minute_beep_done = 1;
            uart_debug_printf("One minute has passed!\n");
        }

        if (button_pressed)
        {
            mef_state.current_state = STATE_TURN_TIME_FINISHED;
            mef_state.alarm_tick = 0;
            uart_debug_printf("Turn ended early! Alarm activated.\n");
        }
        else if (mef_state.elapsed_ticks >= TURN_TICKS)
        {
            mef_state.current_state = STATE_TURN_TIME_FINISHED;
            mef_state.alarm_tick = 0;
            uart_debug_printf("Turn finished! Alarm activated.\n");
        }
        break;

    case STATE_TURN_TIME_FINISHED:
        fade_rgb();
        alarm_pattern();
        mef_state.alarm_tick++;

        if (button_pressed)
        {
            mef_state.current_state = STATE_IDLE;
            mef_state.elapsed_ticks = 0;
            mef_state.alarm_tick = 0;
            mef_state.one_minute_beep_done = 0;
            mef_state.buzzer_duration_ticks = 0;
            led_set_value(RED, 0);
            led_set_value(GREEN, 255);
            led_set_value(BLUE, 0);
            uart_debug_printf("Restarting...\n");
        }
        break;

    default:
        break;
    }
}