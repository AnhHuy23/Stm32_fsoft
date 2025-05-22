#include "stm32f4xx.h"
#include "gpio.h"
#include "relay_driver.h"
#include "button_driver.h"

/* Private constants ---------------------------------------------------------*/
#define BUTTON_DEBOUNCE_DELAY_MS    50u  /* Debounce time in milliseconds */
#define BUTTON_COOLDOWN_DELAY_MS    200u /* Anti-repeat delay */
#define SYSTEM_POLLING_DELAY_MS     10u  /* Main loop delay */

/* Private variables ---------------------------------------------------------*/
static relay_driver_t water_pump;      /* Relay instance for water pump */
static button_driver_t control_button; /* Button instance */

/* Private function prototypes -----------------------------------------------*/
static void System_Init(void);

/**
  * @brief  Main program
  * @retval int
  */
int main(void)
{
    /* System initialization */
    System_Init();

    button_driver_state_t prev_button_state = BUTTON_DRIVER_STATE_RELEASED;

    /* Main super-loop */
    while (1)
    {
        button_driver_state_t current_state = BUTTON_read(&control_button);

        /* Detect complete press-release cycle */
        if ((prev_button_state == BUTTON_DRIVER_STATE_RELEASED) &&
            (current_state == BUTTON_DRIVER_STATE_PRESSED))
        {
            /* Toggle pump state */
            RELAY_toggle(&water_pump);

            /* Wait for button release */
            while (BUTTON_read(&control_button) != BUTTON_DRIVER_STATE_RELEASED)
            {
                /* Blocking wait with timeout protection */
                HAL_Delay(1);
            }

            /* Anti-repeat cooldown */
            HAL_Delay(BUTTON_COOLDOWN_DELAY_MS);
        }

        prev_button_state = current_state;
        HAL_Delay(SYSTEM_POLLING_DELAY_MS);
    }
}

/**
  * @brief  System initialization
  * @note   Configures all hardware peripherals
  */
static void System_Init(void)
{
    /* Relay initialization (PA5 - Output) */
    RELAY_init(&water_pump, GPIOA, 5u);

    /* Button initialization (PC13 - Input with debounce) */
    BUTTON_init(&control_button, GPIOC, 13u, BUTTON_DEBOUNCE_DELAY_MS);
}
