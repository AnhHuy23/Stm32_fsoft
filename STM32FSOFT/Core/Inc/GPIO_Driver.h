#ifndef GPIO_H
#define GPIO_H

#include "stm32f4xx.h"

// GPIO configuration enums
typedef enum {
    DRIVER_GPIO_MODE_INPUT = 0x00,
    DRIVER_GPIO_MODE_OUTPUT = 0x01,
    DRIVER_GPIO_MODE_AF = 0x02,
    DRIVER_GPIO_MODE_ANALOG = 0x03
} GPIOMode;

typedef enum {
    DRIVER_GPIO_OUTPUT_PP = 0x00,
    DRIVER_GPIO_OUTPUT_OD = 0x01
} GPIOOutputType;

typedef enum {
    DRIVER_GPIO_NO_PULL = 0x00,
    DRIVER_GPIO_PULL_UP = 0x01,
    DRIVER_GPIO_PULL_DOWN = 0x02
} GPIOPull;

typedef enum {
    DRIVER_GPIO_SPEED_LOW = 0x00,
    DRIVER_GPIO_SPEED_MEDIUM = 0x01,
    DRIVER_GPIO_SPEED_FAST = 0x02,
    DRIVER_GPIO_SPEED_HIGH = 0x03
} GPIOSpeed;

typedef enum {
    DRIVER_GPIO_AF0 = 0x00,
    DRIVER_GPIO_AF1, DRIVER_GPIO_AF2, DRIVER_GPIO_AF3,
    DRIVER_GPIO_AF4, DRIVER_GPIO_AF5, DRIVER_GPIO_AF6, DRIVER_GPIO_AF7,
    DRIVER_GPIO_AF8, DRIVER_GPIO_AF9, DRIVER_GPIO_AF10, DRIVER_GPIO_AF11,
    DRIVER_GPIO_AF12, DRIVER_GPIO_AF13, DRIVER_GPIO_AF14, DRIVER_GPIO_AF15
} GPIOAltFunction;

typedef struct {
    GPIO_TypeDef *port;
    uint8_t pin;
    GPIOMode mode;
    GPIOOutputType otype;
    GPIOSpeed speed;
    GPIOPull pull;
    GPIOAltFunction af;
} GPIOConfig;

void gpio_init(GPIOConfig *cfg);
void gpio_write(GPIO_TypeDef *port, uint8_t pin, uint8_t value);
uint8_t gpio_read(GPIO_TypeDef *port, uint8_t pin);
void gpio_toggle(GPIO_TypeDef *port, uint8_t pin);
void gpio_set_af(GPIO_TypeDef *port, uint8_t pin, GPIOAltFunction af);

#endif // GPIO_H
