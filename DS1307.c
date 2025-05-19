#include "DS1307.h"
#include "stm32f4xx_hal_i2c.h"

static I2C_HandleTypeDef *ds_i2c;

static uint8_t BCD_To_Dec(uint8_t val) {
    return ((val >> 4) * 10 + (val & 0x0F));
}

static uint8_t Dec_To_BCD(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

void DS1307_Init(I2C_HandleTypeDef *hi2c) {
    ds_i2c = hi2c;
}

void DS1307_GetTime(RTC_Time *time) {
    uint8_t data[3];
    uint8_t reg = 0x00;
    HAL_I2C_Master_Transmit(ds_i2c, DS1307_ADDRESS, &reg, 1, HAL_MAX_DELAY);
    HAL_I2C_Master_Receive(ds_i2c, DS1307_ADDRESS, data, 3, HAL_MAX_DELAY);

    time->second = BCD_To_Dec(data[0] & 0x7F);
    time->minute = BCD_To_Dec(data[1]);
    time->hour   = BCD_To_Dec(data[2] & 0x3F);
}

void DS1307_SetTime(RTC_Time *time) {
    uint8_t data[4];
    data[0] = 0x00;
    data[1] = Dec_To_BCD(time->second);
    data[2] = Dec_To_BCD(time->minute);
    data[3] = Dec_To_BCD(time->hour);

    HAL_I2C_Master_Transmit(ds_i2c, DS1307_ADDRESS, data, 4, HAL_MAX_DELAY);
}
