#ifndef __I2C_LCD_H__
#define __I2C_LCD_H__

#include "stm32f4xx.h"

/* LCD Constants */
#define LCD_I2C_ADDRESS    (0x27 << 1)  /* I2C address shifted for write */
#define LCD_BACKLIGHT      0x08
#define ENABLE             0x04
#define REGISTER_SELECT    0x01

/* LCD Commands */
#define LCD_CLEAR_DISPLAY     0x01
#define LCD_RETURN_HOME       0x02
#define LCD_ENTRY_MODE_SET    0x04
#define LCD_DISPLAY_CONTROL   0x08
#define LCD_FUNCTION_SET      0x20
#define LCD_SET_DDRAM_ADDR    0x80

#define LCD_2LINE             0x08
#define LCD_5x8DOTS           0x00
#define LCD_4BIT_MODE         0x00
#define LCD_DISPLAY_ON        0x04

void lcd_init(void);
void lcd_send_cmd(char cmd);
void lcd_send_data(char data);
void lcd_send_string(char *str);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_clear(void);
void lcd_send_byte(char data, uint8_t rs);

#endif
