/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define IS_RGBW true
#define NUM_PIXELS 8

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 2
#endif

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 8) |
            ((uint32_t) (g) << 16) |
            (uint32_t) (b);
}

void pattern_snakes(uint len, uint t) {
    for (uint i = 0; i < len; ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10)
            put_pixel(urgb_u32(0xff, 0, 0));
        else if (x >= 15 && x < 25)
            put_pixel(urgb_u32(0, 0xff, 0));
        else if (x >= 30 && x < 40)
            put_pixel(urgb_u32(0, 0, 0xff));
        else
            put_pixel(0);
    }
}

void pattern_random(uint len, uint t) {
    if (t % 8)
        return;
    for (int i = 0; i < len; ++i)
        put_pixel(rand());
}

void pattern_sparkle(uint len, uint t) {
    if (t % 8)
        return;
    for (int i = 0; i < len; ++i)
        put_pixel(rand() % 16 ? 0 : 0xffffffff);
}

void pattern_greys(uint len, uint t) {
    int max = 100; // let's not draw too much current!
    t %= max;
    for (int i = 0; i < len; ++i) {
        put_pixel(t * 0x10101);
        if (++t >= max) t = 0;
    }
}

typedef void (*pattern)(uint len, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_snakes,  "Snakes!"},
        {pattern_random,  "Random data"},
        {pattern_sparkle, "Sparkles"},
        {pattern_greys,   "Greys"},
};
void set_sequential_led(uint8_t values[], int n_leds){
    int i = 0;
    int j = 0;
    int n_tx = n_leds * 3 / 4;
    if(n_tx*4 < n_leds*3){
        n_tx++;
    }
    uint8_t tmp_values [n_tx*4];
    for(i=0;i<(n_tx*4);i++){
        tmp_values[i]=0;
    }
    for(i=0;i<(n_leds*3);i++){
        tmp_values[i]=values[i];
        printf("values leds: %02x \n",tmp_values[i]);

    }
    uint32_t tx[n_tx];
    for(i=0;i<n_tx;i++){
        tx[i] = 0;
        tx[i] = tmp_values[j] << 24 | tmp_values[j+1] << 16 | tmp_values[j+2] << 8 | tmp_values[j+3] << 0;
        j = j + 4;
        printf("values array: %08x \n",tx[i]);
    }
    for(i=0; i<n_tx;i++){
        pio_sm_put_blocking(pio0, 0, tx[i]);
    }

}
int main() {
    //set_sys_clock_48();
    stdio_init_all();
    printf("WS2812 Smoke Test, using pin %d", WS2812_PIN);

    // todo get free sm
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);

    ws2812_program_init(pio, sm, offset, 13, 800000, IS_RGBW);

    // uint8_t t = 0;
    while (1) {
        // uint8_t leds_color [8*3] = {255,255,255, 255,255,255, 255,255,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0};
        uint8_t leds_color [8*3] ;//= {255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255, 255,255,255};
        int i = 0;
        for(i=0;i<10;i++){
            leds_color[0 + i*3] = 255;
            leds_color[1 + i*3] = 255;
            leds_color[2 + i*3] = 255;
        }
        set_sequential_led(leds_color,8);
        sleep_ms(1000);

        for(i=0;i<10;i++){
            leds_color[0 + i*3] = 0;
            leds_color[1 + i*3] = 0;
            leds_color[2 + i*3] = 255;
        }
        set_sequential_led(leds_color,8);
        sleep_ms(1000);

        for(i=0;i<10;i++){
            leds_color[0 + i*3] = 0;
            leds_color[1 + i*3] = 255;
            leds_color[2 + i*3] = 0;
        }
        set_sequential_led(leds_color,8);
        sleep_ms(1000);

        for(i=0;i<10;i++){
            leds_color[0 + i*3] = 255;
            leds_color[1 + i*3] = 0;
            leds_color[2 + i*3] = 0;
        }
        set_sequential_led(leds_color,8);
        sleep_ms(1000);
        // int pat = rand() % count_of(pattern_table);
        // int dir = (rand() >> 30) & 1 ? 1 : -1;
        // puts(pattern_table[pat].name);
        // puts(dir == 1 ? "(forward)" : "(backward)");
        // for (int i = 0; i < 1000; ++i) {
        //     pattern_table[pat].pat(NUM_PIXELS, t);
        //     sleep_ms(10);
        //     t += dir;
        // }
    }
}
