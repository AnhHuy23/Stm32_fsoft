#include "i2c_lcd.h"

/* Initialize I2C1: PB6 = SCL, PB7 = SDA */
void i2c1_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;  /* Enable GPIOB */
    GPIOB->MODER |= (2 << (6 * 2)) | (2 << (7 * 2));  /* AF mode */
    GPIOB->AFR[0] |= (4 << (6 * 4)) | (4 << (7 * 4)); /* AF4 = I2C1 */

    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;   /* Enable I2C1 clock */

    I2C1->CR1 &= ~I2C_CR1_PE;             /* Disable I2C before config */
    I2C1->CR2 = 16;                       /* APB1 at 16 MHz */
    I2C1->CCR = 80;                       /* Standard mode: 100kHz */
    I2C1->TRISE = 17;                     /* Max rise time */
    I2C1->CR1 |= I2C_CR1_PE;              /* Enable I2C */
}

/* Send array of data via I2C1 */
void i2c1_write(uint8_t addr, uint8_t *data, uint8_t len) {
    while (I2C1->SR2 & I2C_SR2_BUSY);     /* Wait until bus is free */
    I2C1->CR1 |= I2C_CR1_START;           /* Generate START */

    while (!(I2C1->SR1 & I2C_SR1_SB));    /* Wait for START bit */
    (void)I2C1->SR1;
    I2C1->DR = addr;                      /* Send address */

    while (!(I2C1->SR1 & I2C_SR1_ADDR));  /* Wait for ADDR sent */
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    for (int i = 0; i < len; i++) {
        while (!(I2C1->SR1 & I2C_SR1_TXE));
        I2C1->DR = data[i];
    }

    while (!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;           /* STOP condition */
}

/* Delay bằng SysTick */
void delay_ms(uint32_t ms) {
    SysTick->LOAD = 16000 - 1;           /* 1ms delay @16MHz */
    SysTick->VAL = 0;
    SysTick->CTRL = 5;                   /* Enable SysTick */

    for (uint32_t i = 0; i < ms; i++) {
        while (!(SysTick->CTRL & (1 << 16)));  /* Wait 1ms */
    }
    SysTick->CTRL = 0;
}

/**
 * @brief  Send a byte (command or data) to the LCD
 * @param  data: 8 bit data
 * @param  rs: 0 = command, 1 = data
 */
void lcd_send_byte(char data, uint8_t rs) {
    uint8_t upper = data & 0xF0;
    uint8_t lower = (data << 4) & 0xF0;
    uint8_t data_arr[4];

    data_arr[0] = upper | LCD_BACKLIGHT | rs | ENABLE;
    data_arr[1] = upper | LCD_BACKLIGHT | rs;
    data_arr[2] = lower | LCD_BACKLIGHT | rs | ENABLE;
    data_arr[3] = lower | LCD_BACKLIGHT | rs;

    i2c1_write(LCD_I2C_ADDRESS, data_arr, 4);
    delay_ms(1);
}

/**
 * @brief  Send command to LCD
 * @param  cmd: command
 */
void lcd_send_cmd(char cmd) {
    lcd_send_byte(cmd, 0);
}

/**
 * @brief  Send character to LCD
 * @param  data: character
 */
void lcd_send_data(char data) {
    lcd_send_byte(data, REGISTER_SELECT);
}

/**
 * @brief Initialize LCD 16x2 I2C
 */
void lcd_init(void) {
    i2c1_init();
    delay_ms(50);

    lcd_send_cmd(0x30);
    delay_ms(5);
    lcd_send_cmd(0x30);
    delay_ms(1);
    lcd_send_cmd(0x30);
    delay_ms(10);
    lcd_send_cmd(0x20); /* Set 4-bit mode */

    lcd_send_cmd(LCD_FUNCTION_SET | LCD_2LINE | LCD_5x8DOTS | LCD_4BIT_MODE);
    lcd_send_cmd(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON);
    lcd_send_cmd(LCD_CLEAR_DISPLAY);
    delay_ms(2);
    lcd_send_cmd(LCD_ENTRY_MODE_SET | 0x02);
}

/**
 * @brief  Send null-terminated string to LCD
 * @param  str: character string
 */
void lcd_send_string(char *str) {
    while (*str) {
        lcd_send_data(*str++);
    }
}

/**
 * @brief  Set cursor position on LCD
 * @param  row: line (0 or 1)
 * @param  col: column (0–15)
 */
void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t addr = (row == 0) ? (0x00 + col) : (0x40 + col);
    lcd_send_cmd(LCD_SET_DDRAM_ADDR | addr);
}

/**
 * @brief Clear LCD screen
 */
void lcd_clear(void) {
    lcd_send_cmd(LCD_CLEAR_DISPLAY);
    delay_ms(2);
}
