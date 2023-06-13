#include "pico.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "buzzer.h"
#include "pitches.h"

#include <stdio.h>
#include <stdlib.h>

#define TOP_MAX 65534

const uint8_t BUZ_PIN = 21;
uint slice_num;
uint channel_num;
uint32_t m_div = 0, m_top = 0;


void init_buzzer()
{
    gpio_set_function(BUZ_PIN, GPIO_FUNC_PWM);

    slice_num = pwm_gpio_to_slice_num(BUZ_PIN);
    channel_num = pwm_gpio_to_channel(BUZ_PIN);

}

void stop()
{
    pwm_set_enabled(slice_num, false);
}

int pwm_set_freq(int freq)
{
    // Set the frequency, making "top" as large as possible for maximum resolution.
    m_div = (uint32_t)(16 * clock_get_hz(clk_sys) / (uint32_t)freq);
    m_top = 1;
    for (;;) {
        // Try a few small prime factors to get close to the desired frequency.
        if (m_div >= 16 * 5 && m_div % 5 == 0 && m_top * 5 <= TOP_MAX) {
            m_div /= 5;
            m_top *= 5;
        }
        else if (m_div >= 16 * 3 && m_div % 3 == 0 && m_top * 3 <= TOP_MAX) {
            m_div /= 3;
            m_top *= 3;
        }
        else if (m_div >= 16 * 2 && m_top * 2 <= TOP_MAX) {
            m_div /= 2;
            m_top *= 2;
        }
        else {
            break;
        }
    }

    if (m_div < 16) {
        m_div = 0;
        m_top = 0;
        return -1; // freq too large
    }

    if (m_div >= 256 * 16) {
        m_div = 0;
        m_top = 0;
        return -2; // freq too small
    }

    return 0;
}

int set_pwm_duty(uint32_t duty)
{
    // Set duty cycle.
    uint32_t cc = duty * (m_top + 1) / 65535;

    pwm_set_chan_level(slice_num, channel_num, cc);
    pwm_set_enabled(slice_num, true);

    return 0;
}

void play(uint16_t frequency)
{
    stop();

    pwm_set_freq(frequency);
    pwm_set_wrap(slice_num, m_top);
    set_pwm_duty(32767);
}

void click_button(){
    play(NOTE_GS1);
    sleep_ms(70);
    
    stop();
}

void end_menu(){
    play(NOTE_E5);
    sleep_ms(80);

    stop();
}

void exit_nosave(){
    play(NOTE_C7);
    sleep_ms(60);
    stop();
    sleep_ms(50);
    play(NOTE_G6);
    sleep_ms(100);
    stop();
}

void exit_save(){
    play(NOTE_C7);
    sleep_ms(60);
    stop();
    sleep_ms(50);
    play(NOTE_G6);
    sleep_ms(60);
    stop();
    sleep_ms(50);
    play(NOTE_E6);
    sleep_ms(60);
    stop();
}

void end_acquisition(){
    play(NOTE_E7);
    sleep_ms(120);
    stop();
    play(NOTE_E7);
    sleep_ms(120);
    stop();
    sleep_ms(120);
    play(NOTE_E7);
    sleep_ms(120);
    stop();
    sleep_ms(120);    
    play(NOTE_C7);
    sleep_ms(120);
    stop();
    play(NOTE_E7);
    sleep_ms(120);
    stop();
    sleep_ms(120);
    play(NOTE_G7);
    sleep_ms(120);
    stop();
    sleep_ms(120);
    sleep_ms(120);
    sleep_ms(120);
    play(NOTE_G6);
    sleep_ms(120);
    stop();
    sleep_ms(120);
    sleep_ms(120);
    sleep_ms(120);
}