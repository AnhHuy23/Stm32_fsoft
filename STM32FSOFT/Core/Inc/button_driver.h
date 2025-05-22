#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include "stm32f4xx.h"
#include "gpio.h"

/**
 * @brief Button state definitions with debounce
 */
typedef enum {
    BUTTON_DRIVER_STATE_RELEASED = 0, /**< Button released (stable state) */
    BUTTON_DRIVER_STATE_PRESSED  = 1, /**< Button pressed (stable state) */
    BUTTON_DRIVER_STATE_BOUNCING = 2  /**< Button in debouncing phase */
} button_driver_state_t;

/**
 * @brief Button driver configuration structure
 */
typedef struct {
    GPIO_TypeDef *port;          /**< GPIO port */
    uint8_t pin;                 /**< Pin number */
    button_driver_state_t state;  /**< Current stable state */
    uint32_t last_debounce_time;  /**< Last debounce timestamp */
    uint32_t debounce_delay;      /**< Debounce delay in ms (default: 50ms) */
} button_driver_t;

/**
 * @brief Initialize button driver with hardware debounce
 * @param button Pointer to button instance
 * @param port GPIO port
 * @param pin Pin number
 * @param debounce_ms Debounce time in milliseconds
 */
void BUTTON_init(button_driver_t *button, GPIO_TypeDef *port, uint8_t pin, uint32_t debounce_ms);

/**
 * @brief Read debounced button state
 * @param button Pointer to button instance
 * @return Stable button state (PRESSED/RELEASED)
 */
button_driver_state_t BUTTON_read(button_driver_t *button);

#endif /* BUTTON_DRIVER_H */
