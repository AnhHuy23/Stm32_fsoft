#include "main.h"

#define RELAY_PORT GPIOA
#define RELAY_PIN  GPIO_PIN_5

#define BUTTON_PORT GPIOC
#define BUTTON_PIN  GPIO_PIN_13

void SystemClock_Config(void);
void Error_Handler(void);

int main(void)
{
    /* MCU Initialization */
    HAL_Init();
    SystemClock_Config();

    /* GPIO Initialization structures */
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Configure GPIO pin for Relay output */
    GPIO_InitStruct.Pin = RELAY_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;      /* Push-pull output */
    GPIO_InitStruct.Pull = GPIO_NOPULL;              /* No pull-up or pull-down */
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;     /* Low frequency */
    HAL_GPIO_Init(RELAY_PORT, &GPIO_InitStruct);

    /* Start with relay OFF */
    HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET);

    /* Configure GPIO pin for Button input */
    GPIO_InitStruct.Pin = BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;          /* Input mode */
    GPIO_InitStruct.Pull = GPIO_PULLUP;               /* Pull-up enabled */
    HAL_GPIO_Init(BUTTON_PORT, &GPIO_InitStruct);

    /* Variables to track button and relay state */
    uint8_t relay_state = 0;           /* 0 = OFF, 1 = ON */
    uint8_t button_prev_state = GPIO_PIN_SET;  /* Assume button not pressed (pull-up) */

    while (1)
    {
        uint8_t button_curr_state = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);

        /* Detect falling edge: button press (from released to pressed) */
        if ((button_prev_state == GPIO_PIN_SET) && (button_curr_state == GPIO_PIN_RESET))
        {
            /* Toggle relay state */
            relay_state = !relay_state;

            if (relay_state)
            {
                HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_SET);   /* Relay ON */
            }
            else
            {
                HAL_GPIO_WritePin(RELAY_PORT, RELAY_PIN, GPIO_PIN_RESET); /* Relay OFF */
            }

            HAL_Delay(20);  /* Basic debounce delay */
        }

        button_prev_state = button_curr_state;

        HAL_Delay(10);  /* Polling delay */
    }
}
