#ifndef __DS1307_H__
#define __DS1307_H__

#include "stm32f4xx_hal.h"

#define DS1307_ADDRESS 0x68 << 1

typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} RTC_Time;

void DS1307_Init(I2C_HandleTypeDef *hi2c);
void DS1307_GetTime(RTC_Time *time);
void DS1307_SetTime(RTC_Time *time);

#endif
