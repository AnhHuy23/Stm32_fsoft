#include "relay_driver.h"
#include "gpio.h"

void RELAY_init(relay_driver_t *relay, GPIO_TypeDef *port, uint8_t pin)
{
    /* Initialize GPIO pin */
    const gpio_config_t gpio_cfg = {
        .port = port,
        .pin = pin,
        .mode = GPIO_DRIVER_MODE_OUTPUT,
        .otype = GPIO_DRIVER_OUTPUT_PUSH_PULL,
        .speed = GPIO_DRIVER_SPEED_HIGH,
        .pull = GPIO_DRIVER_NO_PULL,
        .af = GPIO_DRIVER_AF0
    };
    GPIO_init(&gpio_cfg);

    /* Initialize relay structure */
    relay->port = port;
    relay->pin = pin;
    relay->state = RELAY_DRIVER_STATE_OFF;

    /* Set initial state */
    GPIO_write(port, pin, RELAY_DRIVER_STATE_OFF);
}

void RELAY_setState(relay_driver_t *relay, relay_driver_state_t state)
{
    GPIO_write(relay->port, relay->pin, state);
    relay->state = state;
}

void RELAY_toggle(relay_driver_t *relay)
{
    const relay_driver_state_t new_state =
        (relay->state == RELAY_DRIVER_STATE_OFF) ?
        RELAY_DRIVER_STATE_ON : RELAY_DRIVER_STATE_OFF;

    RELAY_setState(relay, new_state);
}

relay_driver_state_t RELAY_getState(const relay_driver_t *relay)
{
    return relay->state;
}
