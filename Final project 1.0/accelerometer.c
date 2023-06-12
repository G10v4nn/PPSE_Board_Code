#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include "datatypes.h"
#include "accelerometer.h"

const uint8_t ADDRESS = 0x1C;

//hardware registers

const uint8_t REG_X_MSB = 0x01;
const uint8_t REG_X_LSB = 0x02;
const uint8_t REG_Y_MSB = 0x03;
const uint8_t REG_Y_LSB = 0x04;
const uint8_t REG_Z_MSB = 0x05;
const uint8_t REG_Z_LSB = 0x06;
const uint8_t REG_DATA_CFG = 0x0E;
const uint8_t REG_CTRL_REG1 = 0x2A;

// Set the range and precision for the data 
const uint8_t range_config = 0x02; // 0x00 for ±2g, 0x01 for ±4g, 0x02 for ±8g
const float count = 1024; // 4096 for ±2g, 2048 for ±4g, 1024 for ±8g

uint8_t buf[2];

float mma8451_convert_accel(uint16_t raw_accel) {
    float acceleration;
    // Acceleration is read as a multiple of g (gravitational acceleration on the Earth's surface)
    // Check if acceleration < 0 and convert to decimal accordingly
    if ((raw_accel & 0x2000) == 0x2000) {
        raw_accel &= 0x1FFF;
        acceleration = (-8192 + (float) raw_accel) / count;
    } else {
        acceleration = (float) raw_accel / count;
    }
    acceleration *= 9.81f;
    return acceleration;
}

#ifdef i2c_default
void mma8451_set_state(uint8_t state) {
    buf[0] = REG_CTRL_REG1;
    buf[1] = state; // Set RST bit to 1
    i2c_write_blocking(i2c_default, ADDRESS, buf, 2, false);
}
#endif

void init_accelerometer(){
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
#warning i2c/mma8451_i2c example requires a board with I2C pins
    puts("Default I2C pins were not defined");
#else
    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    // Enable standby mode
    mma8451_set_state(0x00);

    // Edit configuration while in standby mode
    buf[0] = REG_DATA_CFG;
    buf[1] = range_config;
    i2c_write_blocking(i2c_default, ADDRESS, buf, 2, false);

    // Enable active mode
    mma8451_set_state(0x01);
#endif
}

void get_accelerometer(struct Data_storage *Data){
    float x_acceleration;
    float y_acceleration;
    float z_acceleration;

    // Start reading acceleration registers for 2 bytes
    i2c_write_blocking(i2c_default, ADDRESS, &REG_X_MSB, 1, true);
    i2c_read_blocking(i2c_default, ADDRESS, buf, 2, false);
    x_acceleration = mma8451_convert_accel(buf[0] << 6 | buf[1] >> 2);
    Data->x_acceleration = x_acceleration;

    i2c_write_blocking(i2c_default, ADDRESS, &REG_Y_MSB, 1, true);
    i2c_read_blocking(i2c_default, ADDRESS, buf, 2, false);
    y_acceleration = mma8451_convert_accel(buf[0] << 6 | buf[1] >> 2);
    Data->y_acceleration = y_acceleration; 

    i2c_write_blocking(i2c_default, ADDRESS, &REG_Z_MSB, 1, true);
    i2c_read_blocking(i2c_default, ADDRESS, buf, 2, false);
    z_acceleration = mma8451_convert_accel(buf[0] << 6 | buf[1] >> 2);
    Data->z_acceleration = z_acceleration;
}
