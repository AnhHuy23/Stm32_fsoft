#include "ds1307.h"
#include "i2c_ds1307.h"

#define DS1307_ADDRESS       0x68      /* 7-bit I2C address of DS1307 */
#define DS1307_WRITE_ADDRESS (DS1307_ADDRESS << 1)       /* Write address (LSB=0) */

/**
  * @brief  Convert Binary-Coded Decimal to Decimal
  * @param  val: BCD value to convert
  * @retval Decimal value
  * @note   Converts a byte containing BCD to normal decimal number
  */
static uint8_t BCD2DEC(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0F);
}

/**
  * @brief  Convert Decimal to Binary-Coded Decimal
  * @param  val: Decimal value to convert
  * @retval BCD encoded value
  * @note   Converts a normal decimal number to BCD format
  */
static uint8_t DEC2BCD(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

/**
  * @brief  Initialize DS1307 RTC module
  * @note   Initializes I2C peripheral used for communication with DS1307
  * @retval None
  */
void DS1307_Init(void) {
    I2C_DS1307_Init();
}

/**
  * @brief  Set time and date on DS1307 RTC
  * @param  time: Pointer to RTC_Time structure containing time to set
  * @note   Time values should be in valid ranges:
  *         - Seconds: 0-59
  *         - Minutes: 0-59
  *         - Hours: 0-23 (24h format)
  *         - Day of week: 1-7 (1=Sunday)
  *         - Day: 1-31
  *         - Month: 1-12
  *         - Year: 00-99 (2000-2099)
  * @retval None
  */
void DS1307_SetTime(RTC_Time *time) {
    /* Start I2C communication and select register 0 (seconds register) */
    I2C_DS1307_Start();
    I2C_DS1307_Address(DS1307_WRITE_ADDRESS);
    I2C_DS1307_Write(0x00);

    /* Write all time/date registers in sequence */
    I2C_DS1307_Write(DEC2BCD(time->seconds));
    I2C_DS1307_Write(DEC2BCD(time->minutes));
    I2C_DS1307_Write(DEC2BCD(time->hours));
    I2C_DS1307_Write(DEC2BCD(time->day_of_week));
    I2C_DS1307_Write(DEC2BCD(time->day));
    I2C_DS1307_Write(DEC2BCD(time->month));
    I2C_DS1307_Write(DEC2BCD(time->year));

    // End communication
    I2C_DS1307_Stop();
}

/**
  * @brief  Get current time and date from DS1307 RTC
  * @param  time: Pointer to RTC_Time structure to store retrieved time
  * @note   The structure will be populated with current RTC values:
  *         - Time is in 24-hour format
  *         - Day of week ranges from 1-7 (1=Sunday)
  *         - Year represents 2000-2099 (00-99)
  * @retval None
  */
void DS1307_GetTime(RTC_Time *time) {
    /* Start I2C communication and select register 0 (seconds register) */
    I2C_DS1307_Start();
    I2C_DS1307_Address(DS1307_WRITE_ADDRESS);
    I2C_DS1307_Write(0x00);

    /* Restart in read mode */
    I2C_DS1307_Start();
    I2C_DS1307_Address(DS1307_READ_ADDRESS);

    /* Read all time/date registers in sequence */
    /* Send ACK for all bytes except last one */
    time->seconds = BCD2DEC(I2C_DS1307_Read(1));
    time->minutes = BCD2DEC(I2C_DS1307_Read(1));
    time->hours   = BCD2DEC(I2C_DS1307_Read(1));
    time->day_of_week = BCD2DEC(I2C_DS1307_Read(1));
    time->day     = BCD2DEC(I2C_DS1307_Read(1));
    time->month   = BCD2DEC(I2C_DS1307_Read(1));
    time->year    = BCD2DEC(I2C_DS1307_Read(0)); /* NACK after last byte */

    /* End communication */
    I2C_DS1307_Stop();
}
