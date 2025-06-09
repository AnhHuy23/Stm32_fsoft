#include "stm32f4xx.h"
#include "gpio.h"
#include "adc.h"
#include "ds3231.h"
#include "i2c_driver.h"
#include "lcd_parallel.h"
#include "labview_comm.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* Soil Moisture Sensor */
#define SOIL_SENSOR_PORT        GPIOA
#define SOIL_SENSOR_PIN         0
#define SOIL_SENSOR_ADC_CH      0

/* Relay */
#define RELAY_PORT              GPIOA
#define RELAY_PIN               1

/* Button */
#define MODE_BUTTON_PORT        GPIOA
#define MODE_BUTTON_PIN         12
#define UP_BUTTON_PORT          GPIOA
#define UP_BUTTON_PIN           15
#define DOWN_BUTTON_PORT        GPIOB
#define DOWN_BUTTON_PIN         5
#define LEFT_BUTTON_PORT        GPIOB
#define LEFT_BUTTON_PIN         3
#define RIGHT_BUTTON_PORT       GPIOB
#define RIGHT_BUTTON_PIN        4

/* Constant and Threshold */
#define MOISTURE_THRESHOLD_LOW      30
#define MOISTURE_THRESHOLD_OPTIMAL  50
#define ADC_MAX_VALUE               4095

/**
* Enum for System Mode
* AUTO_MODE: Automatic watering based on soil moisture
* MANUAL_MODE: Manual watering with user-defined start and stop times
*/
typedef enum {
    AUTO_MODE, MANUAL_MODE
} SystemMode;

/**
 * Enum for Manual Display State
 */
typedef enum {
    DISPLAY_MANUAL_NORMAL,
    SETTING_START_HOUR,
    SETTING_START_MINUTE,
    SETTING_STOP_HOUR,
    SETTING_STOP_MINUTE
} ManualDisplayState;

SystemMode current_mode = AUTO_MODE;
ManualDisplayState manual_ui_state = DISPLAY_MANUAL_NORMAL;

/* Define the structure for time */
ds3231_time_t current_time;
ds3231_time_t manual_start_time = {0, 0, 8, 0, 0, 0, 0};
ds3231_time_t manual_stop_time  = {0, 0, 9, 0, 0, 0, 0};

/* ADC buffer for soil moisture sensor readings */
uint16_t adc_buffer[20];

/* Variables for soil moisture readings */
uint16_t soil_moisture_raw = 0;

/* Soil moisture percentage */
uint8_t soil_moisture_percent = 0;

/* Pump status */
uint8_t pump_status = 0; /* 0 = OFF, 1 = ON */

/* Static Function */
static uint16_t Time_ToMinutes(const ds3231_time_t* time_struct);

/* Function Prototypes */
void SystemClock_Config(void);
void Error_Handler(void);

void GPIO_pinsConfig(void);
void ADC_peripheralConfig(void);

uint8_t readButtonDebounced(GPIO_TypeDef *GPIOx, uint8_t pin);
void handleButtonInputs(void);

void processAutoMode(void);
void processManualMode(void);
void updateLCD(void);

uint8_t ADC_convertToMoisturePercentage(uint16_t adc_value);
void controlPump(uint8_t state);

int main(void) {
    HAL_Init();
    SystemClock_Config();
    Delay_Init(); // Khởi tạo SysTick cho delay

    GPIO_pinsConfig();

    LCD_Init();
    LCD_Clear();
    LCD_Write("System Init...");


    ADC_peripheralConfig();
    LabVIEW_UART_Init();

    I2C_Init();

    LCD_Clear();

    while (1) {
        /* Read and process data */
        LabVIEW_UART_ProcessData();
        /* Handle button inputs */
        handleButtonInputs();
        /* Get real time from DS3231 */
        DS3231_getFullTime(&current_time);
        /* Read soil moisture sensor */
        ADC_startConversionSW(ADC1);
        while (!ADC_DMA_getFlagStatus(ADC1, ADC_DMA_FLAG_TC));
        ADC_DMA_clearFlag(ADC1, ADC_DMA_FLAG_TC);
        uint32_t sum = 0;
        for (int i = 0; i < (sizeof(adc_buffer) / sizeof(uint16_t)); i++) {
            sum += adc_buffer[i];
        }
        soil_moisture_raw = sum / (sizeof(adc_buffer) / sizeof(uint16_t));
        soil_moisture_percent = ADC_convertToMoisturePercentage(
                soil_moisture_raw);

        /* Process based on current mode */
        if (current_mode == AUTO_MODE) {
            processAutoMode();
        } else {
            processManualMode();

        }
        /* Send state signal to labview */
        LabVIEW_Send_Value(soil_moisture_raw,
                current_mode == AUTO_MODE ? 0 : 1, pump_status);
        /* Update LCD display */
        updateLCD();
        delay_ms(100);
    }
}


/**
 * @brief GPIO Pins Configuration
 * This function configures the GPIO pins for various peripherals
 * including the soil moisture sensor, relay, buttons, and LCD.
 */
void GPIO_pinsConfig(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    /* Soil Sensor PA0 - Analog Input */
    gpio_config_t soil_sensor = { .port = SOIL_SENSOR_PORT, .pin =
    SOIL_SENSOR_PIN, .mode = GPIO_DRIVER_MODE_ANALOG,
            .otype = GPIO_DRIVER_OUTPUT_PUSH_PULL,
            .speed = GPIO_DRIVER_SPEED_LOW,
            .pull = GPIO_DRIVER_NO_PULL,
            };
    GPIO_Init(&soil_sensor);

    /* LCD Pins PA5-PA10 - Output Push-Pull */
    gpio_config_t lcd_pins[] = { { GPIOB, 1, GPIO_DRIVER_MODE_OUTPUT, GPIO_DRIVER_OUTPUT_PUSH_PULL,
            GPIO_DRIVER_SPEED_MEDIUM, GPIO_DRIVER_NO_PULL },
            { GPIOB, 0, GPIO_DRIVER_MODE_OUTPUT, GPIO_DRIVER_OUTPUT_PUSH_PULL, GPIO_DRIVER_SPEED_MEDIUM,
                    GPIO_DRIVER_NO_PULL },
            { GPIOA, 7, GPIO_DRIVER_MODE_OUTPUT, GPIO_DRIVER_OUTPUT_PUSH_PULL, GPIO_DRIVER_SPEED_MEDIUM,
                    GPIO_DRIVER_NO_PULL },
            { GPIOA, 6, GPIO_DRIVER_MODE_OUTPUT, GPIO_DRIVER_OUTPUT_PUSH_PULL, GPIO_DRIVER_SPEED_MEDIUM,
                    GPIO_DRIVER_NO_PULL },
            { GPIOB, 10, GPIO_DRIVER_MODE_OUTPUT, GPIO_DRIVER_OUTPUT_PUSH_PULL, GPIO_DRIVER_SPEED_MEDIUM,
                    GPIO_DRIVER_NO_PULL },
            { GPIOB, 2, GPIO_DRIVER_MODE_OUTPUT, GPIO_DRIVER_OUTPUT_PUSH_PULL, GPIO_DRIVER_SPEED_MEDIUM,
                    GPIO_DRIVER_NO_PULL }
    };
    for (int i = 0; i < sizeof(lcd_pins) / sizeof(gpio_config_t); i++) {
        GPIO_Init(&lcd_pins[i]);
    }

    /* PB15 (Relay) - Output Push-Pull */
    gpio_config_t relay = { .port = RELAY_PORT, .pin = RELAY_PIN, .mode =
            GPIO_DRIVER_MODE_OUTPUT, .otype = GPIO_DRIVER_OUTPUT_PUSH_PULL, .speed =
                    GPIO_DRIVER_SPEED_MEDIUM, .pull = GPIO_DRIVER_NO_PULL };
    GPIO_Init(&relay);
    GPIO_Write(RELAY_PORT, RELAY_PIN, 0);

    /* Buttons PB0-PB4 - Input with Pull-up */
    gpio_config_t buttons[] = {
            { MODE_BUTTON_PORT, MODE_BUTTON_PIN, GPIO_DRIVER_MODE_INPUT,
                    GPIO_DRIVER_OUTPUT_PUSH_PULL, GPIO_DRIVER_SPEED_LOW, GPIO_DRIVER_PULL_UP },
            { UP_BUTTON_PORT, UP_BUTTON_PIN, GPIO_DRIVER_MODE_INPUT,
                    GPIO_DRIVER_OUTPUT_PUSH_PULL, GPIO_DRIVER_SPEED_LOW, GPIO_DRIVER_PULL_UP },
            { DOWN_BUTTON_PORT, DOWN_BUTTON_PIN, GPIO_DRIVER_MODE_INPUT,
                    GPIO_DRIVER_OUTPUT_PUSH_PULL, GPIO_DRIVER_SPEED_LOW, GPIO_DRIVER_PULL_UP },
            { LEFT_BUTTON_PORT, LEFT_BUTTON_PIN, GPIO_DRIVER_MODE_INPUT,
                    GPIO_DRIVER_OUTPUT_PUSH_PULL, GPIO_DRIVER_SPEED_LOW, GPIO_DRIVER_PULL_UP },
            { RIGHT_BUTTON_PORT,RIGHT_BUTTON_PIN, GPIO_DRIVER_MODE_INPUT,
                    GPIO_DRIVER_OUTPUT_PUSH_PULL, GPIO_DRIVER_SPEED_LOW, GPIO_DRIVER_PULL_UP } };
    for (int i = 0; i < sizeof(buttons) / sizeof(gpio_config_t); i++) {
        GPIO_Init(&buttons[i]);
    }
}

/**
 * @brief ADC Peripheral Configuration
 * This function initializes the ADC peripheral for reading soil moisture sensor values.
 */
void ADC_peripheralConfig(void) {
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    ADC_Init(ADC1, ADC_Resolution_12BIT, ADC_ALIGN_RIGHT,
            ADC_CLK_PRESCALER_DIV4);
    ADC_configChannel(ADC1, SOIL_SENSOR_ADC_CH, ADC_sampleTime_144CYCLES);
    ADC_DMA_Config(ADC1, adc_buffer, sizeof(adc_buffer) / sizeof(uint16_t),
            ADC_DMA_MODE_CIRCULAR);
    ADC_Enable(ADC1);
    ADC_DMA_Enable(ADC1);
}

/**
 * @brief Read Button with Debouncing
 * This function reads the button state with debouncing logic.
 * It waits for the button to be released before returning a pressed state.
 *
 * @param GPIOx Pointer to the GPIO port
 * @param pin Pin number of the button
 * @return 1 if button is pressed, 0 otherwise
 */
uint8_t readButtonDebounced(GPIO_TypeDef *GPIOx, uint8_t pin) {
    if (GPIO_Read(GPIOx, pin) == 0) {
        /* Wait for button to be released */
        delay_ms(50);
        /* Check if button is still pressed after delay */
        if (GPIO_Read(GPIOx, pin) == 0) {

            while (GPIO_Read(GPIOx, pin) == 0)
                ;

            return 1;
        }
    }
    return 0;
}

/**
 * @brief Handle Button Inputs
 * This function handles the button inputs for switching modes and setting manual watering times.
 */
void handleButtonInputs(void) {
    /* Switch between Auto and Manual Mode */
    if (readButtonDebounced(MODE_BUTTON_PORT, MODE_BUTTON_PIN)) {
        current_mode = (current_mode == AUTO_MODE) ? MANUAL_MODE : AUTO_MODE;
        manual_ui_state = DISPLAY_MANUAL_NORMAL;
        LCD_Clear();
    }

    /* Handle Manual Mode UI State */
    if (current_mode == MANUAL_MODE) {
        ds3231_time_t temp_time;  // Temporary time structure for validation

        switch (manual_ui_state) {
        case DISPLAY_MANUAL_NORMAL:
            if (readButtonDebounced(RIGHT_BUTTON_PORT, RIGHT_BUTTON_PIN)) {
                manual_ui_state = SETTING_START_HOUR;
            }
            break;

        case SETTING_START_HOUR:
            if (readButtonDebounced(UP_BUTTON_PORT, UP_BUTTON_PIN)) {
                manual_start_time.hours = (manual_start_time.hours + 1) % 24;
                if (Time_ToMinutes(&manual_start_time)
                        >= Time_ToMinutes(&manual_stop_time)) {
                    manual_stop_time = manual_start_time;
                    manual_stop_time.minutes = (manual_stop_time.minutes + 1)
                            % 60;
                    if (manual_stop_time.minutes == 0) {
                        manual_stop_time.hours = (manual_stop_time.hours + 1)
                                % 24;
                    }
                }
            }
            if (readButtonDebounced(DOWN_BUTTON_PORT, DOWN_BUTTON_PIN)) {
                manual_start_time.hours = (manual_start_time.hours + 23) % 24;
            }
            if (readButtonDebounced(RIGHT_BUTTON_PORT, RIGHT_BUTTON_PIN))
                manual_ui_state = SETTING_START_MINUTE;
            if (readButtonDebounced(LEFT_BUTTON_PORT, LEFT_BUTTON_PIN))
                manual_ui_state = DISPLAY_MANUAL_NORMAL;
            break;

        case SETTING_START_MINUTE:
            if (readButtonDebounced(UP_BUTTON_PORT, UP_BUTTON_PIN)) {
                manual_start_time.minutes = (manual_start_time.minutes + 1)
                        % 60;
                if (manual_start_time.minutes == 0)
                    manual_start_time.hours = (manual_start_time.hours + 1)
                            % 24;
                if (Time_ToMinutes(&manual_start_time)
                        >= Time_ToMinutes(&manual_stop_time)) {
                    manual_stop_time = manual_start_time;
                    manual_stop_time.minutes = (manual_stop_time.minutes + 1)
                            % 60;
                    if (manual_stop_time.minutes == 0)
                        manual_stop_time.hours = (manual_stop_time.hours + 1)
                                % 24;
                }
            }
            if (readButtonDebounced(DOWN_BUTTON_PORT, DOWN_BUTTON_PIN)) {
                if (manual_start_time.minutes == 0)
                    manual_start_time.hours = (manual_start_time.hours + 23)
                            % 24;
                manual_start_time.minutes = (manual_start_time.minutes + 59)
                        % 60;
            }
            if (readButtonDebounced(RIGHT_BUTTON_PORT, RIGHT_BUTTON_PIN))
                manual_ui_state = SETTING_STOP_HOUR;
            if (readButtonDebounced(LEFT_BUTTON_PORT, LEFT_BUTTON_PIN))
                manual_ui_state = SETTING_START_HOUR;
            break;

        case SETTING_STOP_HOUR:
            if (readButtonDebounced(UP_BUTTON_PORT, UP_BUTTON_PIN)) {
                manual_stop_time.hours = (manual_stop_time.hours + 1) % 24;
            }
            if (readButtonDebounced(DOWN_BUTTON_PORT, DOWN_BUTTON_PIN)) {
                temp_time = manual_stop_time;
                temp_time.hours = (temp_time.hours + 23) % 24;
                if (Time_ToMinutes(&temp_time)
                        > Time_ToMinutes(&manual_start_time)) {
                    manual_stop_time.hours = temp_time.hours;
                }
            }
            if (readButtonDebounced(RIGHT_BUTTON_PORT, RIGHT_BUTTON_PIN))
                manual_ui_state = SETTING_STOP_MINUTE;
            if (readButtonDebounced(LEFT_BUTTON_PORT, LEFT_BUTTON_PIN))
                manual_ui_state = SETTING_START_MINUTE;
            break;

        case SETTING_STOP_MINUTE:
            if (readButtonDebounced(UP_BUTTON_PORT, UP_BUTTON_PIN)) {
                manual_stop_time.minutes = (manual_stop_time.minutes + 1) % 60;
                if (manual_stop_time.minutes == 0)
                    manual_stop_time.hours = (manual_stop_time.hours + 1) % 24;
            }
            if (readButtonDebounced(DOWN_BUTTON_PORT, DOWN_BUTTON_PIN)) {
                temp_time = manual_stop_time;
                if (temp_time.minutes == 0)
                    temp_time.hours = (temp_time.hours + 23) % 24;
                temp_time.minutes = (temp_time.minutes + 59) % 60;
                if (Time_ToMinutes(&temp_time)
                        > Time_ToMinutes(&manual_start_time)) {
                    manual_stop_time = temp_time;
                }
            }
            if (readButtonDebounced(RIGHT_BUTTON_PORT, RIGHT_BUTTON_PIN))
                manual_ui_state = DISPLAY_MANUAL_NORMAL;
            if (readButtonDebounced(LEFT_BUTTON_PORT, LEFT_BUTTON_PIN))
                manual_ui_state = SETTING_STOP_HOUR;
            break;
        }
    }
}

/**
 * @brief Convert ADC value to Soil Moisture Percentage
 * This function converts the raw ADC value from the soil moisture sensor
 * to a percentage value.
 *
 * @param adc_value The raw ADC value
 * @return uint8_t Moisture percentage (0-100)
 */
uint8_t ADC_convertToMoisturePercentage(uint16_t adc_value) {
    if (adc_value >= ADC_MAX_VALUE)
        return 0;
    uint32_t percent = (((uint32_t) ADC_MAX_VALUE - adc_value) * 100)
            / ADC_MAX_VALUE;
    return (uint8_t) percent;
}

/**
 * @brief Control Pump
 * This function controls the state of the pump by activating or deactivating
 * the relay connected to the pump.
 * @param state 1 to turn on the pump, 0 to turn it off
 */
void controlPump(uint8_t state) {
    if (state) {
        GPIO_Write(RELAY_PORT, RELAY_PIN, 1);
        pump_status = 1;
    } else {
        GPIO_Write(RELAY_PORT, RELAY_PIN, 0);
        pump_status = 0;
    }
}

/**
 * @brief Convert Time Structure to Minutes
 * This function converts a ds3231_time_t structure to the total number of minutes
 * since midnight.
 *
 * @param time_struct Pointer to the ds3231_time_t structure
 * @return uint16_t Total minutes since midnight
 */
static uint16_t Time_ToMinutes(const ds3231_time_t* time_struct) {
    return time_struct->hours * 60 + time_struct->minutes;
}

/**
 * @brief Process Auto Mode
 * This function reads the soil moisture sensor in auto mode,
 * calculates the moisture percentage, and controls the pump accordingly.
 */
void processAutoMode(void) {
    if (soil_moisture_percent < MOISTURE_THRESHOLD_LOW) {
        controlPump(1);
    } else if (soil_moisture_percent >= MOISTURE_THRESHOLD_OPTIMAL) {
        controlPump(0);
    }
}

/**
 * @brief Process Manual Mode
 * This function checks the current time against the manual start and stop times,
 * and controls the pump accordingly.
 * It handles the case where the start time is before or after the stop time
 * to allow for overnight watering.
 */
void processManualMode(void) {
    /* Check if manual mode is in normal state */
    if (manual_ui_state != DISPLAY_MANUAL_NORMAL) {
        return;
    }

    /* Calculate the current time in minutes of the day */
    uint16_t current_minutes_of_day = current_time.hours * 60
            + current_time.minutes;
    uint16_t start_minutes_of_day = manual_start_time.hours * 60
            + manual_start_time.minutes;
    uint16_t stop_minutes_of_day = manual_stop_time.hours * 60
            + manual_stop_time.minutes;

    /* Handle the case where start time is after stop time */
    if (start_minutes_of_day <= stop_minutes_of_day) {
        if (current_minutes_of_day >= start_minutes_of_day
                && current_minutes_of_day < stop_minutes_of_day) {
            controlPump(1);
        } else {
            controlPump(0);
        }
    } else {
        if (current_minutes_of_day >= start_minutes_of_day
                || current_minutes_of_day < stop_minutes_of_day) {
            controlPump(1);
        } else {
            controlPump(0);
        }
    }
}

/**
 * @brief Update LCD Display
 * This function updates the LCD display with the current mode,
 * current time, soil moisture percentage, and pump status.
 * It formats the display based on whether the system is in auto or manual mode.
 */
void updateLCD(void) {
    char line1[21];
    char line2[21];
    memset(line1, 0, sizeof(line1));
    memset(line2, 0, sizeof(line2));

    if (current_mode == AUTO_MODE) {
        sprintf(line1, "AUTO %02d:%02d:%02d", current_time.hours,
                current_time.minutes, current_time.seconds);
        sprintf(line2, "Moist:%3d%% P:%s", soil_moisture_percent,
                pump_status ? "ON" : "OFF");
    } else {
        switch (manual_ui_state) {
        case DISPLAY_MANUAL_NORMAL:
            sprintf(line1, "MANUAL %02d:%02d:%02d", current_time.hours,
                    current_time.minutes, current_time.seconds);
            sprintf(line2, "S:%02d:%02d E:%02d:%02d P:%s",
                    manual_start_time.hours, manual_start_time.minutes,
                    manual_stop_time.hours, manual_stop_time.minutes,
                    pump_status ? "ON" : "OFF");
            break;
        case SETTING_START_HOUR:
            sprintf(line1, "Set Start HH:MM");
            sprintf(line2, ">%02d_:%02d S:%02d:%02d", manual_start_time.hours,
                    manual_start_time.minutes, manual_stop_time.hours,
                    manual_stop_time.minutes);
            break;
        case SETTING_START_MINUTE:
            sprintf(line1, "Set Start HH:MM");
            sprintf(line2, " %02d:%02d< S:%02d:%02d", manual_start_time.hours,
                    manual_start_time.minutes, manual_stop_time.hours,
                    manual_stop_time.minutes);
            break;
        case SETTING_STOP_HOUR:
            sprintf(line1, "Set Stop HH:MM");
            sprintf(line2, "S:%02d:%02d >%02d_:%02d", manual_start_time.hours,
                    manual_start_time.minutes, manual_stop_time.hours,
                    manual_stop_time.minutes);
            break;
        case SETTING_STOP_MINUTE:
            sprintf(line1, "Set Stop HH:MM");
            sprintf(line2, "S:%02d:%02d  %02d:%02d<", manual_start_time.hours,
                    manual_start_time.minutes, manual_stop_time.hours,
                    manual_stop_time.minutes);
            break;
        }
    }

    LCD_setCursor(0, 0);
    LCD_Write(line1);

    for (int i = strlen(line1); i < 16; i++)
        LCD_Put(' ');

    LCD_setCursor(0, 1); // Dòng thứ 2, cột 0
    LCD_Write(line2);
    for (int i = strlen(line2); i < 16; i++)
        LCD_Put(' ');

}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 168;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
            | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
