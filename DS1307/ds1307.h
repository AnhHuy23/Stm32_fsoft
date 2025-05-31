#ifndef __DS1307_H
#define __DS1307_H

#include "stm32f4xx.h"
#include <stdint.h>

/**
  * @brief DS1307 7-bit I2C slave address
  * @note The address is 0x68 (1101000 in binary)
  */
#define DS1307_ADDRESS 0x68

/**
  * @brief DS1307 write address (LSB = 0)
  * @note Shift left 1 bit and set LSB to 0 for write operation
  */
#define DS1307_WRITE_ADDRESS (DS1307_ADDRESS << 1)

/**
  * @brief DS1307 read address (LSB = 1)
  * @note Shift left 1 bit and set LSB to 1 for read operation
  */
#define DS1307_READ_ADDRESS  ((DS1307_ADDRESS << 1) | 0x01)

/**
  * @brief  RTC Time structure definition
  * @note   Contains all time and date fields supported by DS1307
  */
typedef struct {
    uint8_t seconds;      /*Seconds (0-59) */
    uint8_t minutes;      /* Minutes (0-59) */
    uint8_t hours;        /* Hours (0-23 in 24h mode, 1-12 in 12h mode) */
    uint8_t day_of_week;  /* Day of week (1-7, where 1 = Sunday) */
    uint8_t day;          /* Day of month (1-31) */
    uint8_t month;        /* Month (1-12) */
    uint8_t year;         /* Year (00-99, represents 2000-2099) */
} RTC_Time;

/**
  * @brief  Initialize DS1307 RTC
  * @note   Configures I2C interface and checks RTC communication
  * @retval None
  */
void DS1307_Init(void);

/**
  * @brief  Set time and date on DS1307
  * @param  time: Pointer to RTC_Time structure containing time/date to set
  * @note   The structure should contain valid time/date values
  * @retval None
  */
void DS1307_SetTime(RTC_Time *time);

/**
  * @brief  Get current time and date from DS1307
  * @param  time: Pointer to RTC_Time structure to store retrieved time/date
  * @note   The function will populate the provided structure with current RTC values
  * @retval None
  */
void DS1307_GetTime(RTC_Time *time);

#endif /* __DS1307_H */
