/**
 * @file main.c
 * @brief Simple GPIO example for STM32F401CCU6 using the provided GPIO driver.
 * This example toggles an LED based on the state of a button.
 */

#include "stm32f4xx.h"
#include "GPIO_Driver.h"

/* Define the LED pin and port */
#define LED_PIN   5  /* Typically PA5 on many STM32 boards */
#define LED_PORT  GPIOA

/* Define the button pin and port */
#define BUTTON_PIN   0 /* Assuming the button is connected to PC0 */
#define BUTTON_PORT  GPIOC

/**
 * @brief Main function. Initializes GPIO pins and controls an LED based on button input.
 *
 * @return int Should return 0 if execution is successful.
 */
int main(void) {
    /* Configure the LED pin as an output */
    gpio_driver_config_t led_config;
    led_config.port  = LED_PORT;
    led_config.pin   = LED_PIN;
    led_config.mode  = GPIO_DRIVER_MODE_OUTPUT;
    led_config.otype = GPIO_DRIVER_OUTPUT_PUSH_PULL;
    led_config.speed = GPIO_DRIVER_SPEED_LOW;
    led_config.pull  = GPIO_DRIVER_NO_PULL; /* No internal pull-up/down needed for output */
    gpio_driver_init(&led_config);

    /* Configure the button pin as an input with pull-down resistor */
    gpio_driver_config_t button_config;
    button_config.port  = BUTTON_PORT;
    button_config.pin   = BUTTON_PIN;
    button_config.mode  = GPIO_DRIVER_MODE_INPUT;
    button_config.pull  = GPIO_DRIVER_PULL_DOWN; /* Use pull-down to ensure a defined LOW state when the button is not pressed */
    gpio_driver_init(&button_config);

    /* Main loop */
    while (1) {
        /* Read the state of the button */
        uint8_t button_state = gpio_driver_read(BUTTON_PORT, BUTTON_PIN);

        /* If the button is pressed (HIGH state), turn the LED on */
        if (button_state) {
            gpio_driver_write(LED_PORT, LED_PIN, 1); /* Set LED pin HIGH */
        } else {
            /* If the button is not pressed (LOW state), turn the LED off */
            gpio_driver_write(LED_PORT, LED_PIN, 0); /* Set LED pin LOW */
        }

        /* Simple delay loop for visualization (not recommended for real-time applications) */
        for (volatile uint32_t i = 0; i < 100000; i++);
    }
}
