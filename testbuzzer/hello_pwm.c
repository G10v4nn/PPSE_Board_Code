/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Output PWM signals on pins 0 and 1
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

int main() {
    stdio_init_all();
    /// \tag::setup_pwm[]

    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_dir(21,true);
    gpio_set_function(21, GPIO_FUNC_PWM);
    // gpio_set_function(21, GPIO_FUNC_PWM);
    // pwm_set_gpio_level(21, 15);
    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice_num = pwm_gpio_to_slice_num(21);
    pwm_set_clkdiv_mode(slice_num,0);
    pwm_set_clkdiv(slice_num,100000.0);
    // Set period of 4 cycles (0 to 3 inclusive)
    pwm_set_wrap(slice_num, 4);
    // Set channel A output high for one cycle before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
    // Set initial B output high for three cycles before dropping
    pwm_set_chan_level(slice_num, PWM_CHAN_B, 1);
    // Set the PWM running
    pwm_set_enabled(slice_num, true);
    /// \end::setup_pwm[]

    // Note we could also use pwm_set_gpio_level(gpio, x) which looks up the
    // correct slice and channel for a given GPIO.
    while(true){

        printf("done\n");
        sleep_ms(1000);
    }
}
