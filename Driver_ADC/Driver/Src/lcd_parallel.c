/*
 * lcd_parallel.c
 *
 *  Created on: May 14, 2025
 *      Author: proha
 */
#include "lcd_parallel.h"
#include <stdint.h>

char display_settings;

static void delay(uint32_t count) {
	for (volatile uint32_t i = 0; i < count * 1000; i++);
}


/**
 * @brief  Send a falling edge to the LCD
 * @param  None
 * @retval None
 * This function generates a falling edge on the Enable pin of the LCD.
 */
static void fallingEdge(void) {
	LCD_DATA_PORT->ODR &= ~(1 << E_Pin); // Set Enable pin low
	LCD_DATA_PORT->ODR |= (1 << E_Pin);  // Set Enable pin high
	LCD_DATA_PORT->ODR &= ~(1 << E_Pin); // Set Enable pin low
	delay(1); // Small delay for the LCD to process
}

#ifndef LCD8Bit
	static void LCD_SendData4Bit(char data) {
		LCD_DATA_PORT->ODR &= ~((1 << DATA5_Pin) | (1 << DATA6_Pin) | (1 << DATA7_Pin) | (1 << DATA8_Pin));

		/* Set data pin 5 */
		if (data & 0x01) LCD_DATA_PORT->ODR |= (1 << DATA5_Pin);
		/* Set data pin 6 */
		if (data & 0x02) LCD_DATA_PORT->ODR |= (1 << DATA6_Pin);
		/* Set data pin 7 */
		if (data & 0x04) LCD_DATA_PORT->ODR |= (1 << DATA7_Pin);
		/* Set data pin 8 */
		if (data & 0x08) LCD_DATA_PORT->ODR |= (1 << DATA8_Pin);

		fallingEdge();
	}
#endif

/**
 * @brief  Send a command to the LCD
 * @param  command: Command to send
 * @retval None
 * Sends a control instruction (e.g., clear, set cursor, shift)
	to the LCD by splitting it into two 4-bit transmissions.
 */
static void LCD_SendCommand(char command) {
	/* Clear RS pin for command */
	LCD_DATA_PORT->ODR &= ~(1 << RS_Pin);
	/* Send upper nibble*/
	LCD_SendData4Bit(command >> 4);
	/* Send lower nibble */
	LCD_SendData4Bit(command); // Send lower nibble
}

/**
 * @brief  Send data to the LCD
 * @param  data: Data to send
 * @retval None
 * Sends a character to be displayed on the LCD.
 */
static void LCD_SendData(char data) {
	LCD_DATA_PORT->ODR |= (1 << RS_Pin); // Set RS pin for data
	LCD_SendData4Bit(data >> 4); // Send upper nibble
	LCD_SendData4Bit(data); // Send lower nibble
}

/**
 * @brief  Turn off display without clearing data
 * @param  None
 * @retval None
 * Sends command to disable display (Display OFF).
 */
void LCD_Clear(void) {
	LCD_SendCommand(LCD_CMD_CLEAR_DISPLAY);
	delay(5); // Wait for the command to complete
}

/**
 * @brief  Put a character on the LCD
 * @param  c: Character to display
 * @retval None
 * This function sends a character to the LCD.
 */
void LCD_Put(char c) {
	LCD_SendData(c);
}

/**
 * @brief  Write a string to the LCD
 * @param  str: Pointer to the string to display
 * @retval None
 * This function sends a string to the LCD.
 */
void LCD_Write(char *str) {
	while (*str) {
		LCD_SendData(*str++);
	}
}

/**
 * @brief  Initialize the LCD in 4-bit mode
 * @param  None
 * @retval None
 * Sends the required startup sequence and configuration commands to prepare the LCD for operation.
 */
void LCD_Init(void) {
	/* Clear RS pin for command */
	LCD_DATA_PORT->ODR &= ~(1 << RS_Pin);
	/* Clear Enable pin */
	LCD_DATA_PORT->ODR &= ~(1 << E_Pin);
	delay(50);

	#ifdef LCD_CMD_4BIT_MODE
		display_settings = LCD_CMD_4BIT_MODE | LCD_CMD_2LINE_MODE | LCD_CMD_5x8_DOTS;
		LCD_SendData4Bit(0x03);
		delay(5);
		LCD_SendData4Bit(0x03);
		delay(5);
		LCD_SendData4Bit(0x03);
		delay(2);
		LCD_SendData4Bit(0x02);
		delay(2);
	#endif
	LCD_SendCommand(LCD_CMD_FUNCTION_SET | display_settings);
	display_settings |= LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF;
	LCD_SendCommand(LCD_CMD_DISPLAY_CONTROL	 | display_settings);
	delay(2);

	LCD_Clear();
	display_settings |= LCD_CMD_SET_ENTRY_LEFT | LCD_CMD_SET_ENTRY_NO_SHIFT;
	LCD_SendCommand(LCD_CMD_ENTRY_MODE_SET | display_settings);
	delay(2);
}

/**
 * @brief  Set cursor to specified column and row
 * @param  x: Column position (0–15)
 * @param  y: Row position (0: top row, 1: bottom row)
 * @retval None
 * Moves the LCD cursor to the given (x, y) position.
 */
void LCD_SetCursor(char x, char y) {
	uint8_t address;
	if (y == 0) {
		/* First line */
		address = 0x00 + x;
	} else {
		/* Second line */
		address = 0x40 + x;
	}
	/* Set DDRAM address */
	LCD_SendCommand(LCD_CMD_SET_DDRAM_ADDR | address);
}

/**
 * @brief  Turn on the cursor
 * @param  None
 * @retval None
 * This function turns on the cursor on the LCD.
 */
void LCD_CursorOn(void) {
	LCD_SendCommand(LCD_CMD_DISPLAY_CONTROL | 0x04 | 0x02);
}

/**
 * @brief  Turn on the blinking cursor
 * @param  None
 * @retval None
 * This function turns on the blinking cursor on the LCD.
 */
void LCD_BlinkOn(void) {
	LCD_SendCommand(LCD_CMD_DISPLAY_CONTROL | 0x04 | 0x01);
}

/**
  * @brief Clears the cursor and the blink bits.
  * @param None
  * @retval None
  */
void LCD_ClearDisplay(void) {
	LCD_SendCommand(0x08 | 0x04 | 0x00);
}

/**
 * @brief  Set display settings
 * @param  settings: Display settings (ON/OFF, cursor, blink)
 * @retval None
 * This function sets the display settings for the LCD.
 */
void LCD_SetDisplaySettings(LCD_Display_Settings settings) {
	LCD_SendCommand(LCD_CMD_DISPLAY_CONTROL | (settings & 0x07));
}




