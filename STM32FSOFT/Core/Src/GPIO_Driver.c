#include "GPIO_Driver.h"

/**
 * @brief Initialize GPIO pin with specified configuration.
 *
 * @param cfg Pointer to GPIO configuration structure
 */
void gpio_driver_init(const gpio_driver_config_t *cfg)
{
    /* Enable clock for the corresponding GPIO port */
    if (cfg->port == GPIOA)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (cfg->port == GPIOB)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if (cfg->port == GPIOC)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    else if (cfg->port == GPIOD)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    else if (cfg->port == GPIOE)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    else if (cfg->port == GPIOH)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;

    /* Configure pin mode */
    cfg->port->MODER &= ~(0x3 << (cfg->pin * 2));
    cfg->port->MODER |= ((cfg->mode & 0x3) << (cfg->pin * 2));

    /* Configure output type */
    cfg->port->OTYPER &= ~(1 << cfg->pin);
    cfg->port->OTYPER |= (cfg->otype << cfg->pin);

    /* Configure output speed */
    cfg->port->OSPEEDR &= ~(0x3 << (cfg->pin * 2));
    cfg->port->OSPEEDR |= (cfg->speed << (cfg->pin * 2));

    /* Configure pull-up / pull-down resistors */
    cfg->port->PUPDR &= ~(0x3 << (cfg->pin * 2));
    cfg->port->PUPDR |= (cfg->pull << (cfg->pin * 2));

    /* Configure alternate function if needed */
    if (cfg->mode == GPIO_DRIVER_MODE_ALT_FUNCTION)
    {
        gpio_driver_set_af(cfg->port, cfg->pin, cfg->af);
    }
}

/**
 * @brief Set alternate function for a GPIO pin.
 *
 * @param port GPIO port (GPIOA~GPIOH).
 * @param pin  GPIO pin number (0~15).
 * @param af   Alternate function selection (AF0~AF15).
 */
void gpio_driver_set_af(GPIO_TypeDef *port, uint8_t pin, gpio_driver_alt_function_t af)
{
    if (pin < 8)
    {
        port->AFR[0] &= ~(0xF << (pin * 4));
        port->AFR[0] |= (af << (pin * 4));
    }
    else
    {
        port->AFR[1] &= ~(0xF << ((pin - 8) * 4));
        port->AFR[1] |= (af << ((pin - 8) * 4));
    }
}

/**
 * @brief Write a digital value to a GPIO pin.
 *
 * @param port GPIO port.
 * @param pin  GPIO pin number.
 * @param value 0 for low, non-zero for high.
 */
void gpio_driver_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value)
{
    if (value)
        port->BSRR = (1 << pin);          // Set pin
    else
        port->BSRR = (1 << (pin + 16));   // Reset pin
}

/**
 * @brief Read digital input value from a GPIO pin.
 *
 * @param port GPIO port.
 * @param pin  GPIO pin number.
 * @return uint8_t 1 if high, 0 if low.
 */
uint8_t gpio_driver_read(GPIO_TypeDef *port, uint8_t pin)
{
    return (port->IDR >> pin) & 0x01;
}

/**
 * @brief Toggle the current output state of a GPIO pin.
 *
 * @param port GPIO port.
 * @param pin  GPIO pin number.
 */
void gpio_driver_toggle(GPIO_TypeDef *port, uint8_t pin)
{
    port->ODR ^= (1 << pin);
}
