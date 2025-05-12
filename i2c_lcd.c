#include "i2c_lcd.h"
#include "stm32f4xx_hal_i2c.h"


static I2C_HandleTypeDef *lcd_i2c;
static uint8_t backlight = LCD_BACKLIGHT;

void lcd_send_byte(char data, uint8_t rs) {
    uint8_t upper = data & 0xF0;
    uint8_t lower = (data << 4) & 0xF0;
    uint8_t data_arr[4];

    data_arr[0] = upper | backlight | rs | ENABLE;
    data_arr[1] = upper | backlight | rs;
    data_arr[2] = lower | backlight | rs | ENABLE;
    data_arr[3] = lower | backlight | rs;

    HAL_I2C_Master_Transmit(lcd_i2c, LCD_I2C_ADDRESS, data_arr, 4, HAL_MAX_DELAY);
    HAL_Delay(1);
}

void lcd_send_cmd(char cmd) {
    lcd_send_byte(cmd, 0);
}

void lcd_send_data(char data) {
    lcd_send_byte(data, REGISTER_SELECT);
}

void lcd_init(I2C_HandleTypeDef *hi2c) {
    lcd_i2c = hi2c;
    HAL_Delay(50);

    // Gửi chuỗi khởi tạo LCD ở chế độ 4-bit
    lcd_send_cmd(0x30);
    HAL_Delay(5);
    lcd_send_cmd(0x30);
    HAL_Delay(1);
    lcd_send_cmd(0x30);
    HAL_Delay(10);
    lcd_send_cmd(0x20); // 4-bit mode

    lcd_send_cmd(LCD_FUNCTION_SET | LCD_2LINE | LCD_5x8DOTS | LCD_4BIT_MODE);
    lcd_send_cmd(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON);
    lcd_send_cmd(LCD_CLEAR_DISPLAY);
    HAL_Delay(2);
    lcd_send_cmd(LCD_ENTRY_MODE_SET | 0x02); // cursor moves to right
}

void lcd_send_string(char *str) {
    while (*str) {
        lcd_send_data(*str++);
    }
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t addr = 0x00;

    switch (row) {
        case 0:
            addr = 0x00 + col;
            break;
        case 1:
            addr = 0x40 + col;
            break;
    }

    lcd_send_cmd(LCD_SET_DDRAM_ADDR | addr);
}

void lcd_clear(void) {
    lcd_send_cmd(LCD_CLEAR_DISPLAY);
    HAL_Delay(2);
}
