#include "GPIO_Driver.h"

void gpio_init(GPIOConfig *cfg) {
    // Enable clock for GPIO port
    if (cfg->port == GPIOA) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (cfg->port == GPIOB) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if (cfg->port == GPIOC) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
    else if (cfg->port == GPIOD) RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    else if (cfg->port == GPIOE) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
    else if (cfg->port == GPIOH) RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;

    // Configure mode
    cfg->port->MODER &= ~(0x3 << (cfg->pin * 2));
    cfg->port->MODER |= ((cfg->mode & 0x3) << (cfg->pin * 2));

    // Configure output type
    cfg->port->OTYPER &= ~(1 << cfg->pin);
    cfg->port->OTYPER |= (cfg->otype << cfg->pin);

    // Configure speed
    cfg->port->OSPEEDR &= ~(0x3 << (cfg->pin * 2));
    cfg->port->OSPEEDR |= (cfg->speed << (cfg->pin * 2));

    // Configure pull-up/pull-down
    cfg->port->PUPDR &= ~(0x3 << (cfg->pin * 2));
    cfg->port->PUPDR |= (cfg->pull << (cfg->pin * 2));

    // Alternate function
    if (cfg->mode == DRIVER_GPIO_MODE_AF) {
        gpio_set_af(cfg->port, cfg->pin, cfg->af);
    }
}

void gpio_set_af(GPIO_TypeDef *port, uint8_t pin, GPIOAltFunction af) {
    if (pin < 8) {
        port->AFR[0] &= ~(0xF << (pin * 4));
        port->AFR[0] |= (af << (pin * 4));
    } else {
        port->AFR[1] &= ~(0xF << ((pin - 8) * 4));
        port->AFR[1] |= (af << ((pin - 8) * 4));
    }
}

void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value) {
    if (value)
        port->BSRR = (1 << pin);
    else
        port->BSRR = (1 << (pin + 16));
}

uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin) {
    return (port->IDR >> pin) & 0x01;
}

void gpio_toggle(GPIO_TypeDef *port, uint8_t pin) {
    port->ODR ^= (1 << pin);
}
