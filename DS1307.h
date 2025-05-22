#ifndef DS1307_H
#define DS1307_H

#include "stm32f4xx.h" 
#include <stdint.h>

#define DS1307_I2C_ADDR 0x68 /* 7-bit I2C address of DS1307 */

/**
 * @brief Time structure for RTC
 */
typedef struct {
    uint8_t seconds; /* Seconds value 0-59 */
    uint8_t minutes; /* Minutes value 0-59 */
    uint8_t hours;   /* Hours value 0-23 (24-hour format) */
} RTC_Time;

/**
 * @brief Initialize DS1307 interface
 * @param i2c Pointer to I2C peripheral (I2C1, I2C2, etc.)
 */
void DS1307_Init(I2C_TypeDef *i2c);

/**
 * @brief Read current time from RTC
 * @param time Pointer to RTC_Time structure to store result
 */
void DS1307_ReadTime(RTC_Time *time);

/**
 * @brief Write new time to RTC
 * @param time Pointer to RTC_Time structure containing new time
 */
void DS1307_WriteTime(RTC_Time *time);

#endif 
