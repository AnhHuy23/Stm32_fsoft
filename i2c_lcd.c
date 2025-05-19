#include "i2c_lcd.h"
#include "stm32f4xx_hal_i2c.h"

static I2C_HandleTypeDef *lcd_i2c;         // Pointer to I2C handle
static uint8_t backlight = LCD_BACKLIGHT;  // Backlight control flag

// Send a byte (command or data) to the LCD via I2C
void lcd_send_byte(char data, uint8_t rs) {
    uint8_t upper = data & 0xF0;               // Upper nibble
    uint8_t lower = (data << 4) & 0xF0;        // Lower nibble
    uint8_t data_arr[4];

    // Send upper nibble with enable bit set then cleared
    data_arr[0] = upper | backlight | rs | ENABLE;
    data_arr[1] = upper | backlight | rs;

    // Send lower nibble with enable bit set then cleared
    data_arr[2] = lower | backlight | rs | ENABLE;
    data_arr[3] = lower | backlight | rs;

    // Transmit the 4 bytes via I2C
    HAL_I2C_Master_Transmit(lcd_i2c, LCD_I2C_ADDRESS, data_arr, 4, HAL_MAX_DELAY);
    HAL_Delay(1);  // Small delay for LCD to process data
}

// Send a command byte to the LCD
void lcd_send_cmd(char cmd) {
    lcd_send_byte(cmd, 0);  // rs = 0 indicates command
}

// Send a data byte (character) to the LCD
void lcd_send_data(char data) {
    lcd_send_byte(data, REGISTER_SELECT);  // rs = 1 indicates data
}

// Initialize the LCD
void lcd_init(I2C_HandleTypeDef *hi2c) {
    lcd_i2c = hi2c;       // Assign I2C handle
    HAL_Delay(50);        // Wait for LCD to power up

    // Initialization sequence for 4-bit mode (as per HD44780 datasheet)
    lcd_send_cmd(0x30);
    HAL_Delay(5);
    lcd_send_cmd(0x30);
    HAL_Delay(1);
    lcd_send_cmd(0x30);
    HAL_Delay(10);
    lcd_send_cmd(0x20);   // Set to 4-bit mode

    // Function set: 4-bit, 2-line, 5x8 font
    lcd_send_cmd(LCD_FUNCTION_SET | LCD_2LINE | LCD_5x8DOTS | LCD_4BIT_MODE);

    // Display control: display ON, cursor OFF
    lcd_send_cmd(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON);

    // Clear the display
    lcd_send_cmd(LCD_CLEAR_DISPLAY);
    HAL_Delay(2);

    // Entry mode set: cursor moves right
    lcd_send_cmd(LCD_ENTRY_MODE_SET | 0x02);
}

// Send a string to the LCD
void lcd_send_string(char *str) {
    while (*str) {
        lcd_send_data(*str++);  // Send each character
    }
}

// Set the cursor to a specific row and column
void lcd_set_cursor(uint8_t row, uint8_t col) {
    uint8_t addr = 0x00;

    switch (row) {
        case 0:
            addr = 0x00 + col;  // Line 1 starts at address 0x00
            break;
        case 1:
            addr = 0x40 + col;  // Line 2 starts at address 0x40
            break;
    }

    // Set DDRAM address to position the cursor
    lcd_send_cmd(LCD_SET_DDRAM_ADDR | addr);
}

// Clear the LCD display
void lcd_clear(void) {
    lcd_send_cmd(LCD_CLEAR_DISPLAY);
    HAL_Delay(2);  // Wait for clear command to finish
}
