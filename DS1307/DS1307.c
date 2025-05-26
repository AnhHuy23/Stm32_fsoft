#include "DS1307.h"

static I2C_TypeDef *I2Cx;  /* Pointer to I2C peripheral */

/**
 * @brief Convert BCD to decimal
 * @param bcd BCD value to convert
 * @return Decimal value
 */
static uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

/**
 * @brief Convert decimal to BCD
 * @param dec Decimal value to convert
 * @return BCD value
 */
static uint8_t dec_to_bcd(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

void DS1307_Init(I2C_TypeDef *i2c) {
    I2Cx = i2c;
}

void DS1307_ReadTime(RTC_Time *time) {
    uint8_t data[3];
    uint8_t reg_addr = 0x00; /* Start reading from seconds register */
    
    /* Send register address */
    while (I2Cx->SR2 & I2C_SR2_BUSY); /* Wait until bus free */
    I2Cx->CR1 |= I2C_CR1_START;      /* Generate START */
    while (!(I2Cx->SR1 & I2C_SR1_SB)); /* Wait for START generated */
    (void)I2Cx->SR1;                 /* Clear SB flag */
    I2Cx->DR = (DS1307_I2C_ADDR << 1) | 0; /* Address + Write */
    while (!(I2Cx->SR1 & I2C_SR1_ADDR));   /* Wait for address sent */
    (void)I2Cx->SR1; (void)I2Cx->SR2;      /* Clear flags */
    I2Cx->DR = reg_addr;              /* Send register address */
    while (!(I2Cx->SR1 & I2C_SR1_TXE));    /* Wait for byte transmitted */
    
    /* Read time data (3 bytes: sec, min, hour) */
    I2Cx->CR1 |= I2C_CR1_START;       /* Repeated START */
    while (!(I2Cx->SR1 & I2C_SR1_SB)); /* Wait for START generated */
    (void)I2Cx->SR1;
    I2Cx->DR = (DS1307_I2C_ADDR << 1) | 1; /* Address + Read */
    while (!(I2Cx->SR1 & I2C_SR1_ADDR));
    (void)I2Cx->SR1; (void)I2Cx->SR2;
    
    /* Read with NACK on last byte */
    for (uint8_t i = 0; i < 3; i++) {
        if (i == 2) I2Cx->CR1 &= ~I2C_CR1_ACK; /* No ACK for last byte */
        while (!(I2Cx->SR1 & I2C_SR1_RXNE));   /* Wait for data received */
        data[i] = I2Cx->DR;
    }
    
    I2Cx->CR1 |= I2C_CR1_STOP;        /* Generate STOP */
    
    /* Convert and store time values */
    time->seconds = bcd_to_dec(data[0] & 0x7F); /* Mask CH bit */
    time->minutes = bcd_to_dec(data[1]);
    time->hours = bcd_to_dec(data[2] & 0x3F);   /* Mask 24/12 hour bit */
}

void DS1307_WriteTime(RTC_Time *time) {
    uint8_t data[4];
    data[0] = 0x00; /* Register address (seconds) */
    data[1] = dec_to_bcd(time->seconds) & 0x7F; /* Ensure CH bit is 0 */
    data[2] = dec_to_bcd(time->minutes);
    data[3] = dec_to_bcd(time->hours);
    
    /* Send data */
    while (I2Cx->SR2 & I2C_SR2_BUSY); /* Wait until bus free */
    I2Cx->CR1 |= I2C_CR1_START;   /* Generate START */
    while (!(I2Cx->SR1 & I2C_SR1_SB)); /* Wait for START generated */
    (void)I2Cx->SR1;
    I2Cx->DR = (DS1307_I2C_ADDR << 1) | 0; /* Address + Write */
    while (!(I2Cx->SR1 & I2C_SR1_ADDR));   /* Wait for address sent */
    (void)I2Cx->SR1; (void)I2Cx->SR2;      /* Clear flags */
    
    /* Send 4 bytes */
    for (uint8_t i = 0; i < 4; i++) {
        while (!(I2Cx->SR1 & I2C_SR1_TXE)); /* Wait for DR empty */
        I2Cx->DR = data[i];
    }
    
    while (!(I2Cx->SR1 & I2C_SR1_BTF));    /* Wait for transfer complete */
    I2Cx->CR1 |= I2C_CR1_STOP;             /* Generate STOP */
}
