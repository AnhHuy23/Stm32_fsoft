#ifndef RELAY_DRIVER_H
#define RELAY_DRIVER_H

#include "stm32f4xx.h"
#include "gpio.h"

/**
 * @brief Relay state definitions
 */
typedef enum {
    RELAY_DRIVER_STATE_OFF = 0, /**< Relay OFF state */
    RELAY_DRIVER_STATE_ON  = 1  /**< Relay ON state */
} relay_driver_state_t;

/**
 * @brief Relay driver configuration structure
 */
typedef struct {
    GPIO_TypeDef *port;        /**< GPIO port (GPIOA, GPIOB, etc.) */
    uint8_t pin;               /**< Pin number (0-15) */
    relay_driver_state_t state; /**< Current relay state */
} relay_driver_t;

/**
 * @brief Initialize relay driver
 * @param relay Pointer to relay driver instance
 * @param port GPIO port
 * @param pin Pin number
 */
void RELAY_init(relay_driver_t *relay, GPIO_TypeDef *port, uint8_t pin);

/**
 * @brief Set relay state
 * @param relay Pointer to relay driver instance
 * @param state New state (RELAY_DRIVER_STATE_OFF or RELAY_DRIVER_STATE_ON)
 */
void RELAY_setState(relay_driver_t *relay, relay_driver_state_t state);

/**
 * @brief Toggle relay state
 * @param relay Pointer to relay driver instance
 */
void RELAY_toggle(relay_driver_t *relay);

/**
 * @brief Get current relay state
 * @param relay Pointer to relay driver instance
 * @return Current relay state
 */
relay_driver_state_t RELAY_getState(const relay_driver_t *relay);

#endif /* RELAY_DRIVER_H */
