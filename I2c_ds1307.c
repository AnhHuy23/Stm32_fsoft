#include "i2c_ds1307.h"

#define I2C_GPIO_RCC_ENR RCC_AHB1ENR_GPIOBEN  /* GPIOB clock enable bit */
#define I2C_RCC_ENR      RCC_APB1ENR_I2C1EN    /* I2C1 clock enable bit */
#define I2C_PORT         GPIOB                 /* I2C GPIO port */
#define I2C_SCL_PIN      8                     /* SCL pin number */
#define I2C_SDA_PIN      9                     /* SDA pin number */

/**
 * @brief  Initialize I2C interface for DS1307 RTC
 * @note   Configures GPIO pins and I2C peripheral for communication with DS1307
 * @param  None
 * @retval None
 */
void I2C_DS1307_Init(void) {
    /* Enable clocks for GPIO and I2C peripherals */
    RCC->AHB1ENR |= I2C_GPIO_RCC_ENR;
    RCC->APB1ENR |= I2C_RCC_ENR;

    /* Configure GPIO pins for I2C alternate function */
    I2C_PORT->MODER &= ~((3 << (I2C_SCL_PIN * 2)) | (3 << (I2C_SDA_PIN * 2)));
    I2C_PORT->MODER |=  (2 << (I2C_SCL_PIN * 2)) | (2 << (I2C_SDA_PIN * 2));

    /*  Set alternate function 4 (I2C) for pins */
    I2C_PORT->AFR[1] |= (4 << ((I2C_SCL_PIN - 8) * 4)) | (4 << ((I2C_SDA_PIN - 8) * 4));

    /* Set pins as open-drain */
    I2C_PORT->OTYPER |= (1 << I2C_SCL_PIN) | (1 << I2C_SDA_PIN);

    /* Set high speed for pins */
    I2C_PORT->OSPEEDR |= (3 << (I2C_SCL_PIN * 2)) | (3 << (I2C_SDA_PIN * 2));

    /* Enable pull-up resistors */
    I2C_PORT->PUPDR |= (1 << (I2C_SCL_PIN * 2)) | (1 << (I2C_SDA_PIN * 2));

    /* Configure I2C peripheral */
    I2Cx->CR1 &= ~I2C_CR1_PE;       /* Disable I2C before configuration */
    I2Cx->CR2 = 16;                 /* Set peripheral clock to 16 MHz */
    I2Cx->CCR = 80;                 /* Set clock control for 100 kHz mode */
    I2Cx->TRISE = 17;               /* Set maximum rise time */
    I2Cx->CR1 |= I2C_CR1_PE;        /* Enable I2C peripheral */
}

/**
 * @brief  Generate I2C start condition
 * @note   Waits until start condition is generated
 * @param  None
 * @retval None
 */
void I2C_DS1307_Start(void) {
    I2Cx->CR1 |= I2C_CR1_START;     /* Generate start condition */
    while (!(I2Cx->SR1 & I2C_SR1_SB)); /* Wait for start bit flag */
}

/**
 * @brief  Send slave address
 * @note   Waits until address is sent and acknowledged
 * @param  address: 7-bit slave address with R/W bit (LSB)
 * @retval None
 */
void I2C_DS1307_Address(uint8_t address) {
    I2Cx->DR = address;             /* Send address */
    while (!(I2Cx->SR1 & I2C_SR1_ADDR)); /* Wait for address flag */
    (void)I2Cx->SR2;                /* Clear ADDR flag by reading SR2 */
}

/**
 * @brief  Write a byte to I2C bus
 * @note   Waits until data register is empty and byte is transmitted
 * @param  data: Byte to be transmitted
 * @retval None
 */
void I2C_DS1307_Write(uint8_t data) {
    while (!(I2Cx->SR1 & I2C_SR1_TXE)); /* Wait for data register empty */
    I2Cx->DR = data;                 /* Write data to data register */
    while (!(I2Cx->SR1 & I2C_SR1_BTF)); /* Wait for byte transfer finished */
}

/**
 * @brief  Read a byte from I2C bus
 * @note   Configures ACK/NACK based on parameter and waits until data is received
 * @param  ack: 1 to send ACK, 0 to send NACK after reception
 * @retval Received byte
 */
uint8_t I2C_DS1307_Read(uint8_t ack) {
    if (ack)
        I2Cx->CR1 |= I2C_CR1_ACK;  /* Enable ACK */
    else
        I2Cx->CR1 &= ~I2C_CR1_ACK;  /* Disable ACK */

    while (!(I2Cx->SR1 & I2C_SR1_RXNE)); /* Wait for data received */
    return I2Cx->DR;                /* Return received data */
}

/**
 * @brief  Generate I2C stop condition
 * @note   Ends the I2C communication
 * @param  None
 * @retval None
 */
void I2C_DS1307_Stop(void) {
    I2Cx->CR1 |= I2C_CR1_STOP;      /* Generate stop condition */
}
