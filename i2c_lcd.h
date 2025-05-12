#ifndef __I2C_LCD_H__
#define __I2C_LCD_H__

#include "stm32f4xx_hal.h"

#define LCD_I2C_ADDRESS 0x27 << 1

/*Các lệnh lcd*/
#define LCD_CLEAR_DISPLAY     0x01
#define LCD_RETURN_HOME       0x02
#define LCD_ENTRY_MODE_SET    0x04
#define LCD_DISPLAY_CONTROL   0x08
#define LCD_CURSOR_SHIFT      0x10
#define LCD_FUNCTION_SET      0x20
#define LCD_SET_CGRAM_ADDR    0x40
#define LCD_SET_DDRAM_ADDR    0x80

/*Các flags cho hiển thị*/
#define LCD_DISPLAY_ON        0x04
#define LCD_CURSOR_ON         0x02
#define LCD_BLINK_ON          0x01

#define LCD_2LINE             0x08
#define LCD_5x8DOTS           0x00
#define LCD_4BIT_MODE         0x00

#define LCD_BACKLIGHT         0x08
#define LCD_NOBACKLIGHT       0x00

#define ENABLE  0x04
#define READ_WRITE 0x02
#define REGISTER_SELECT 0x01

void lcd_init(I2C_HandleTypeDef *hi2c);
void lcd_send_cmd(char cmd);
void lcd_send_data(char data);
void lcd_send_string(char *str);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_clear(void);

#endif
