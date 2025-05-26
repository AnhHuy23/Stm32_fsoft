#include "gpio.h"

/**
 * @brief GPIO port and pin configuration for relay
 */
#define RELAY_PORT      GPIOA
#define RELAY_PIN       5

/**
 * @brief GPIO port and pin configuration for button
 */
#define BUTTON_PORT     GPIOC
#define BUTTON_PIN      13

/**
 * @brief Delay function using simple loops (blocking delay)
 *
 * This function provides a crude delay in milliseconds by looping NOPs.
 * Assumes a 16 MHz system clock.
 *
 * @param ms Number of milliseconds to delay
 */
void delay_ms(volatile uint32_t ms)
{
    while (ms--)
    {
        for (volatile uint32_t i = 0; i < 16000; i++) {
            __asm("nop");
        }
    }
}

/**
 * @brief Main function
 *
 * This program initializes a relay on PA5 and a button on PC13.
 * When the button is pressed (falling edge), the relay toggles its state.
 */
int main(void)
{
    /** Enable clocks for GPIOA and GPIOC */
    RCC->AHB1ENR |= (1 << 0);  /**< Enable GPIOA clock */
    RCC->AHB1ENR |= (1 << 2);  /**< Enable GPIOC clock */

    /**
     * @brief Configure PA5 as output (relay control)
     */
    gpio_config_t relay_config = {
        .port   = RELAY_PORT,
        .pin    = RELAY_PIN,
        .mode   = GPIO_DRIVER_MODE_OUTPUT,
        .otype  = GPIO_DRIVER_OUTPUT_PUSH_PULL,
        .pull   = GPIO_DRIVER_NO_PULL,
        .speed  = GPIO_DRIVER_SPEED_LOW,
        .af     = GPIO_DRIVER_AF0
    };
    GPIO_Init(&relay_config);
    GPIO_Write(RELAY_PORT, RELAY_PIN, 0);  /**< Initial state: relay off */

    /**
     * @brief Configure PC13 as input with pull-up (button input)
     */
    gpio_config_t button_config = {
        .port   = BUTTON_PORT,
        .pin    = BUTTON_PIN,
        .mode   = GPIO_DRIVER_MODE_INPUT,
        .otype  = GPIO_DRIVER_OUTPUT_PUSH_PULL,  /**< Required by struct, ignored in input mode */
        .pull   = GPIO_DRIVER_PULL_UP,
        .speed  = GPIO_DRIVER_SPEED_LOW,
        .af     = GPIO_DRIVER_AF0
    };
    GPIO_Init(&button_config);

    uint8_t relay_state = 0;
    uint8_t previous_button_state = 1;

    while (1)
    {
        uint8_t current_button_state = GPIO_Read(BUTTON_PORT, BUTTON_PIN);

        /**
         * @brief Detect falling edge on button press to toggle relay
         */
        if (previous_button_state == 1 && current_button_state == 0)
        {
            relay_state = !relay_state;
            GPIO_Write(RELAY_PORT, RELAY_PIN, relay_state);
            delay_ms(20);  /**< Simple debounce delay */
        }

        previous_button_state = current_button_state;
        delay_ms(10);  /**< Polling delay */
    }
}
