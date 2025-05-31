#ifndef __I2C_DS1307_H
#define __I2C_DS1307_H


#include "stm32f4xx.h"   // STM32F4xx HAL library
#include <stdint.h>      // Standard integer types

/**
  * @brief I2C peripheral used for DS1307 communication
  * @note Change this define if using different I2C peripheral
  */
#define I2Cx I2C1

/**
  * @brief  Initializes I2C peripheral for DS1307 communication
  * @note   Configures GPIO pins and I2C timing for 100kHz operation
  * @retval None
  */
void I2C_DS1307_Init(void);

/**
  * @brief  Generates I2C start condition
  * @note   Must be called before any data transmission/reception
  * @retval None
  */
void I2C_DS1307_Start(void);

/**
  * @brief  Generates I2C stop condition
  * @note   Must be called to end communication
  * @retval None
  */
void I2C_DS1307_Stop(void);

/**
  * @brief  Sends slave address on I2C bus
  * @param  address: 7-bit device address with R/W bit (LSB)
  * @note   For DS1307, write address is 0xD0, read address is 0xD1
  * @retval None
  */
void I2C_DS1307_Address(uint8_t address);

/**
  * @brief  Writes single byte to I2C bus
  * @param  data: Byte to be transmitted
  * @retval None
  */
void I2C_DS1307_Write(uint8_t data);

/**
  * @brief  Reads single byte from I2C bus
  * @param  ack: Whether to send ACK (1) or NACK (0) after reception
  * @note   Send ACK for all bytes except last one in multi-byte read
  * @retval Received byte
  */
uint8_t I2C_DS1307_Read(uint8_t ack);

#endif /* __I2C_DS1307_H */
