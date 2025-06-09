#include "labview_comm.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define PCLK1_FREQ              42000000UL  /* PCLK1 frequency in Hz */
#define USART_BAUDRATE          115200UL    /* Desired baud rate for USART2 */
#define RX_BUFFER_SIZE          128

volatile uint8_t parsed_hour = 0;
volatile uint8_t parsed_minute = 0;
volatile uint8_t parsed_second = 0;
volatile uint8_t labview_pump_command = 0;

/* The UART reception buffer. 'static' makes it private to this file. */
static volatile char rx_buffer[RX_BUFFER_SIZE];
/* Current index in the reception buffer. */
static volatile uint8_t rx_index = 0;
/* Flag to indicate a complete line has been received from LabVIEW. */
static volatile uint8_t new_data_received_flag = 0;

extern void controlPump(uint8_t state);
extern void DS3231_setTime(uint8_t hh, uint8_t mm, uint8_t ss);
extern void Activate_LabVIEW_Override(void);

/* Private (Static) Function Prototypes */
static void GPIO_Init_USART2(void);
static void USART2_Init(void);
static void USART2_SendChar(char c);
static void parse_token(const char* data_str);

/**
 * @brief Initializes USART2 peripheral and corresponding GPIO pins.
 */
void LabVIEW_UART_Init(void) {
    GPIO_Init_USART2();
    USART2_Init();
}

/**
 * @brief Sends a string to LabVIEW via USART2.
 */
void LabVIEW_UART_SendString(const char* str) {
    while (*str) {
        USART2_SendChar(*str++);
    }
}

/**
 * @brief Checks for and processes any complete commands received from LabVIEW.
 */
void LabVIEW_UART_ProcessData(void) {
    if (new_data_received_flag != 1) {
        return; /* No new complete command to process */
    }

    rx_buffer[rx_index] = '\0'; /* Null-terminate the received data */

    /* Use strtok to handle multiple commands in one buffer */
    char *token = strtok((char*) rx_buffer, "\n\r");
    while (token != NULL) {
        if (strlen(token) > 0) {
            if ((strlen(token) == 1 && (token[0] == '0' || token[0] == '1'))
                    || (strchr(token, ':') != NULL
                            && (token[strlen(token) - 1] == '0'
                                    || token[strlen(token) - 1] == '1'))) {
            }
            parse_token(token);
        }
        token = strtok(NULL, "\n\r");
    }

    /* Reset for the next reception */
    rx_index = 0;
    new_data_received_flag = 0;
}

void LabVIEW_Send_Value(uint16_t value, uint8_t mode, uint8_t pump_state) {
    char buffer[20];
    uint32_t temp = value;
    if (mode == 0) { /* AUTO_MODE */
        temp = temp * 10;
    } else if (mode == 1) { /* MANUAL_MODE */
        temp = (temp * 10) + 1;
    }
    /* Append pump state */
    if (pump_state == 1) { /* Pump ON */
        temp = (temp * 10) + 1;
    }
    else { /* Pump OFF */
        temp = temp * 10;
    }
    sprintf(buffer, "%d\n", temp);
    LabVIEW_UART_SendString(buffer);
}

/**
 * @brief Configures GPIO pins PA2 (TX) and PA3 (RX) for USART2.
 */
static void GPIO_Init_USART2(void) {
    /* GPIOA clock should be enabled before calling this function */
    GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->MODER |= (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);
    GPIOA->OSPEEDR |= (3U << (2*2)) | (3U << (3*2));
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD3);
    GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0;
    GPIOA->AFR[0] &= ~((0xFU << (2*4)) | (0xFU << (3*4)));
    GPIOA->AFR[0] |= (7U << (2*4)) | (7U << (3*4));
}

/**
 * @brief Initializes the USART2 peripheral.
 */
static void USART2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    volatile uint32_t dummy_read = RCC->APB1ENR; (void)dummy_read;

    USART2->CR1 &= ~USART_CR1_UE;
    USART2->BRR = 0x16D; /* 0x16D corresponds to 115200 baud rate with PCLK1_FREQ of 42MHz */

    USART2->CR1 &= ~(USART_CR1_M | USART_CR1_PCE);
    USART2->CR2 &= ~USART_CR2_STOP;
    USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE);

    NVIC_SetPriority(USART2_IRQn, 0);
    NVIC_EnableIRQ(USART2_IRQn);

    USART2->CR1 |= USART_CR1_UE;
}

/**
 * @brief Sends a single character via USART2.
 */
static void USART2_SendChar(char c) {
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (c & 0xFF);
}

/**
 * @brief Parses a combined string to find time and an accompanying pump command.
 */
static void parse_token(const char* data_str) {
    int h = 0, m = 0, s = 0;
    char am_pm_str[3] = {0};
    int items_matched = 0;
    int chars_consumed = 0;
    const char* ptr_search = data_str;

    /* Search for the time pattern within the token */
    while (*ptr_search != '\0') {
        items_matched = sscanf(ptr_search, "%d:%d:%d %2s%n", &h, &m, &s, am_pm_str, &chars_consumed);
        if (items_matched == 4) {
            if ((strcmp(am_pm_str, "AM") == 0 || strcmp(am_pm_str, "PM") == 0) &&
                (h >= 1 && h <= 12) && (m >= 0 && m <= 59) && (s >= 0 && s <= 59)) {
                break; /* Found a valid time pattern */
            } else {
                items_matched = 0; /* Invalid values, continue search */
            }
        }
        ptr_search++;
    }

    if (items_matched == 4) {
        /* Time part was valid, update globals */
        parsed_hour = (uint8_t)h;
        parsed_minute = (uint8_t)m;
        parsed_second = (uint8_t)s;
        if (strcmp(am_pm_str, "PM") == 0 && parsed_hour != 12) parsed_hour += 12;
        else if (strcmp(am_pm_str, "AM") == 0 && parsed_hour == 12) parsed_hour = 0;

        /* Now, check for an accompanying pump command */
        char pump_cmd_char = ptr_search[chars_consumed];
        if (pump_cmd_char == '1') {
            labview_pump_command = 1;
        } else if (pump_cmd_char == '0') {
            labview_pump_command = 0;
        }

        /* Perform actions based on newly parsed data */
        DS3231_setTime(parsed_hour, parsed_minute, parsed_second);

    } else if (strlen(data_str) == 1) { /* Check if it was a simple pump command */
        if (data_str[0] == '1') {
            labview_pump_command = 1;
        } else if (data_str[0] == '0') {
            labview_pump_command = 0;
        }
    } else {
        /* If no valid time pattern was found, set error flag */
        new_data_received_flag = 2; /* Set error flag */
        parsed_hour = 0;
        parsed_minute = 0;
        parsed_second = 0;
        labview_pump_command = 0; /* Reset pump command */
    }
}


/**
 * @brief This function handles the USART2 global interrupt.
 */
void USART2_IRQHandler(void) {
    /* Check for Overrun Error (ORE) */
    if (USART2->SR & USART_SR_ORE) {
        volatile uint32_t temp_read = USART2->DR; /* Read DR to clear ORE */
        (void)temp_read; /* Prevent unused variable warning */
        rx_index = 0; /* Reset index on overrun error */
        new_data_received_flag = 0; /* Reset new data flag */
        return;
    }

    if (USART2->SR & USART_SR_RXNE) {
        char received_char = (char)(USART2->DR & 0xFF);
        if (new_data_received_flag == 0) {
            if (received_char == '\n' || received_char == '\r') {
                if (rx_index > 0) {
                    new_data_received_flag = 1;
                }
            } else {
                if (rx_index < RX_BUFFER_SIZE - 1) {
                    rx_buffer[rx_index++] = received_char;
                } else {
                    rx_index = 0;
                }
            }
        }
    }
}
