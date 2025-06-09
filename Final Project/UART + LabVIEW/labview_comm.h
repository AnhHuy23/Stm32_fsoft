#ifndef _LABVIEW_COMM_H_
#define _LABVIEW_COMM_H_

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/*
 * @brief  Global variables to store the last successfully parsed data from LabVIEW.
 * @note   'extern' keyword indicates that these variables are defined in another
 * source file (labview_comm.c). 'volatile' ensures that the compiler
 * does not optimize away accesses to these variables.
 */
extern volatile uint8_t parsed_hour;
extern volatile uint8_t parsed_minute;
extern volatile uint8_t parsed_second;
extern volatile uint8_t labview_pump_command;

/**
 * @brief  Initializes the UART peripheral and GPIO pins for LabVIEW communication.
 * @note   This function must be called once during system initialization.
 */
void LabVIEW_UART_Init(void);

/**
 * @brief  Sends a single character to LabVIEW via UART.
 * @param  c Character to send
 */
void LabVIEW_UART_SendChar(char c);

/**
 * @brief  Sends a string to LabVIEW via UART.
 * @param  str Pointer to the null-terminated string to send
 */
void LabVIEW_UART_SendString(const char* str);

/**
 * @brief  Checks for and processes any complete data lines received from LabVIEW.
 * @note   This function should be called repeatedly in the main application loop.
 * It is non-blocking. It handles tokenizing the buffer if multiple
 * commands are received in one batch.
 */
void LabVIEW_UART_ProcessData(void);

/**
 * @brief  Sends a value to LabVIEW.
 * @param  value The value to send (e.g., soil moisture percentage)
 * @param  mode The current mode (0 for auto, 1 for manual)
 * @param  pump_state The current state of the pump (0 for off, 1 for on)
 */
void LabVIEW_Send_Value(uint16_t value, uint8_t mode, uint8_t pump_state);

#endif // _LABVIEW_COMM_H_

