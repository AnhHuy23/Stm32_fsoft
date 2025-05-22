#include "button_driver.h"
#include "gpio.h"

/**
 * @brief Initializes button hardware interface and debounce parameters
 * @param button Pointer to button driver instance
 * @param port GPIO port (GPIOA, GPIOB, etc.)
 * @param pin Pin number (0-15)
 * @param debounce_ms Minimum stable time required in milliseconds
 */
void BUTTON_init(button_driver_t *button, GPIO_TypeDef *port, uint8_t pin, uint32_t debounce_ms)
{
    /* GPIO configuration for button input */
    const gpio_config_t gpio_cfg = {
        .port = port,
        .pin = pin,
        .mode = GPIO_DRIVER_MODE_INPUT,
        .pull = GPIO_DRIVER_PULL_UP,       /* Internal pull-up resistor */
        .speed = GPIO_DRIVER_SPEED_LOW,     /* Reduced speed for noise immunity */
        .otype = GPIO_DRIVER_OUTPUT_PUSH_PULL, /* Unused for input mode */
        .af = GPIO_DRIVER_AF0              /* No alternate function */
    };
    GPIO_init(&gpio_cfg);

    /* Enable hardware debounce filter if supported */
    if (port == GPIOA || port == GPIOB || port == GPIOC) {
        port->PUPDR |= (0x2 << (pin * 2)); /* Enable hardware filtering */
    }

    /* Initialize button driver instance */
    button->port = port;
    button->pin = pin;
    button->state = BUTTON_DRIVER_STATE_RELEASED;
    button->last_debounce_time = 0;
    button->debounce_delay = debounce_ms;
}

/**
 * @brief Reads and processes debounced button state
 * @param button Pointer to initialized button driver instance
 * @return Current stable button state (PRESSED/RELEASED)
 *
 * @note Implements 3-stage debounce:
 *       1. Physical state detection
 *       2. Debounce timeout verification
 *       3. Stable state confirmation
 */
button_driver_state_t BUTTON_read(button_driver_t *button)
{
    uint32_t current_time = HAL_GetTick(); /* Get current system time */

    /* Stage 1: Read raw physical state */
    uint8_t current_pin_state = GPIO_read(button->port, button->pin) == 0 ?
                              BUTTON_DRIVER_STATE_PRESSED :
                              BUTTON_DRIVER_STATE_RELEASED;

    /* Stage 2: Detect state transitions */
    if (current_pin_state != button->state) {
        button->last_debounce_time = current_time;
        button->state = BUTTON_DRIVER_STATE_BOUNCING;
    }

    /* Stage 3: Verify debounce period elapsed */
    if ((button->state == BUTTON_DRIVER_STATE_BOUNCING) &&
        ((current_time - button->last_debounce_time) > button->debounce_delay))
    {
        /* Final state confirmation */
        uint8_t verified_state = GPIO_read(button->port, button->pin) == 0 ?
                               BUTTON_DRIVER_STATE_PRESSED :
                               BUTTON_DRIVER_STATE_RELEASED;

        button->state = verified_state;
    }

    return button->state;
}
