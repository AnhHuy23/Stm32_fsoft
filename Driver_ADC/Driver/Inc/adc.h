/*
 * adc.h
 *
 *  Created on: May 12, 2025
 *      Author: proha
 */

#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
extern "C" {
#endif


#include "stm32f4xx.h"
#include <stdint.h>

/* ADC Resolution (CR1 Register, RES bits) */
typedef enum {
	/* 00: 12-bit (15 ADCCLK cycles) */
    ADC_Resolution_12BIT = 0x00,
	/* 01: 10-bit (13 ADCCLK cycles) */
    ADC_Resolution_10BIT = 0x01,
	/* 10: 8-bit (11 ADCCLK cycles) */
    ADC_Resolution_8BIT  = 0x02,
	/* 11: 6-bit (9 ADCCLK cycles) */
    ADC_Resolution_6BIT  = 0x03
} ADC_Resolution;

/* ADC Data Alignment (CR2 Register, ALIGN bit) */
typedef enum {
	/* 0: Right alignment */
    ADC_ALIGN_RIGHT = 0x00,
	/* 1: Left alignment */
    ADC_ALIGN_LEFT  = 0x01
} ADC_DataAlign;

/* ADC Common Prescaler (CCR Register, ADCPRE bits) - Affects all ADCs */
typedef enum {
    ADC_CLK_PRESCALER_DIV2 = 0x00, // 00: PCLK2 divided by 2
    ADC_CLK_PRESCALER_DIV4 = 0x01, // 01: PCLK2 divided by 4
    ADC_CLK_PRESCALER_DIV6 = 0x02, // 10: PCLK2 divided by 6
    ADC_CLK_PRESCALER_DIV8 = 0x03  // 11: PCLK2 divided by 8
} ADC_ClockPrescaler;

/*
 * ADC Sampling Time (SMPR1/SMPR2 Registers, SMPx bits)
 * These values correspond to the bit patterns for the SMPx[2:0] bits
*/
typedef enum {
    ADC_SampleTime_3CYCLES   = 0x00, /*000: 3 cycles*/
    ADC_SampleTime_15CYCLES  = 0x01, /*001: 15 cycles*/
    ADC_SampleTime_28CYCLES  = 0x02, // 010: 28 cycles
    ADC_SampleTime_56CYCLES  = 0x03, // 011: 56 cycles
    ADC_SampleTime_84CYCLES  = 0x04, // 100: 84 cycles
    ADC_SampleTime_112CYCLES = 0x05, // 101: 112 cycles
    ADC_SampleTime_144CYCLES = 0x06, // 110: 144 cycles
    ADC_SampleTime_480CYCLES = 0x07  // 111: 480 cycles
} ADC_SampleTime;



#define USE_ADC1
/* #define USE_ADC2 */
/* #define USE_ADC3 */

/* Define ADC_DRIVER_INSTANCE based on the USE_ADCx selection */
#ifdef USE_ADC1
    #define ADC_DRIVER_INSTANCE ADC1
    #define ADC_IRQn            ADC_IRQn
    #ifndef RCC_APB2ENR_ADC1EN
        #error "RCC_APB2ENR_ADC1EN is not defined for this target. Check device header/family."
    #endif
#elif defined(USE_ADC2)
    #define ADC_DRIVER_INSTANCE ADC2
    #define ADC_IRQn            ADC_IRQn
     #ifndef RCC_APB2ENR_ADC2EN
        #error "RCC_APB2ENR_ADC2EN is not defined for this target. Check device header/family."
    #endif
#elif defined(USE_ADC3)
    #define ADC_DRIVER_INSTANCE ADC3
    #define ADC_IRQn            ADC_IRQn
     #ifndef RCC_APB2ENR_ADC3EN
        #error "RCC_APB2ENR_ADC3EN is not defined for this target. Check device header/family."
    #endif
#else
    #error "Please define USE_ADC1, USE_ADC2, or USE_ADC3 in adc_reg_driver.h"
#endif

#ifndef ADC_DRIVER_INSTANCE
    #error "ADC_DRIVER_INSTANCE could not be determined based on USE_ADCx definition."
#endif


/**
 * @brief Enables the clock for the specified ADC peripheral (defined by ADC_DRIVER_INSTANCE).
 * @note Call this before ADC_Init. Assumes APB2 clock source. Adjust if needed.
 * @param None
 * @retval None
 */
void ADC_ClockEnable(void);

/**
 * @brief Disables the clock for the specified ADC peripheral.
 * @param None
 * @retval None
 */
void ADC_ClockDisable(void);


/**
 * @brief Initializes the ADC peripheral defined by ADC_DRIVER_INSTANCE.
 * @note Assumes ADC clock and GPIO clocks/configuration are already done.
 * @param resolution: ADC resolution (ADC_Resolution enum).
 * @param alignment: Data alignment (ADC_DataAlign enum).
 * @param prescaler: ADC clock prescaler (applied commonly to all ADCs).
 * @retval None
 */
void ADC_Init(ADC_Resolution resolution, ADC_DataAlign alignment, ADC_ClockPrescaler prescaler);


/**
 * @brief De-initializes the ADC peripheral (resets registers).
 * @note Does NOT disable the peripheral clock. Call ADC_ClockDisable separately if needed.
 * @param None
 * @retval None
 */
void ADC_DeInit(void);

/**
 * @brief Configures a single channel for regular conversion sequence.
 * @note This simple version configures sequence length = 1, rank 1 = channel.
 * For multi-channel scan mode, SQR registers need more complex setup.
 * @param channel: ADC channel number (0-18, check datasheet for availability. e.g., 0 for PA0/ADC123_IN0).
 * @param sampleTime: Sampling time for this channel (ADC_SampleTime enum).
 * @retval None
 */
void ADC_ConfigChannel(uint8_t channel, ADC_SampleTime sampleTime);

/**
 * @brief Enables the ADC peripheral (sets the ADON bit).
 * @note Must be called after configuration and before starting conversion.
 * A small delay might be needed after enabling before the ADC is stable.
 * @param None
 * @retval None
 */
void ADC_Enable(void);

/**
 * @brief Disables the ADC peripheral (clears the ADON bit).
 * @note Ensure no conversion is ongoing before disabling.
 * @param None
 * @retval None
 */
void ADC_Disable(void);


/**
 * @brief Starts a single ADC conversion using software trigger.
 * @note Assumes Scan mode is disabled and Continuous mode is disabled.
 * ADC must be enabled first.
 * @param None
 * @retval None
 */
void ADC_StartConversionSW(void);

/**
 * @brief Polls the End Of Conversion (EOC) flag.
 * @param timeout: A simple counter-based timeout (number of checks, not ms).
 * Use 0 for infinite polling (blocking).
 * @retval 1 if EOC flag is set, 0 if timeout occurred before EOC.
 */
uint8_t ADC_PollForConversion(volatile uint32_t timeout);

/**
 * @brief Reads the converted data from the ADC data register.
 * @note Reading DR usually clears the EOC flag automatically in single conversion mode.
 * @param None
 * @retval The 16-bit converted value (or 32-bit if needed, F4 DR is 16-bit wide effectively).
 */
uint16_t ADC_ReadValue(void);


/**
 * @brief Enables the End Of Conversion (EOC) interrupt.
 * @note You MUST also configure and enable the corresponding ADC interrupt line
 * in the NVIC (Nested Vectored Interrupt Controller) separately.
 * @param None
 * @retval None
 */
void ADC_EnableEOCInterrupt(void);

/**
 * @brief Disables the End Of Conversion (EOC) interrupt.
 * @param None
 * @retval None
 */
void ADC_DisableEOCInterrupt(void);

/**
 * @brief Checks if a specific ADC status flag is set.
 * @param flag: The status flag mask (e.g., ADC_SR_EOC, ADC_SR_OVR).
 * @retval 1 if the flag is set, 0 otherwise.
 */
uint8_t ADC_GetFlagStatus(uint32_t flag);

/**
 * @brief Clears a specific ADC status flag.
 * @note EOC is often cleared by reading DR. Check RM for others (e.g., OVR).
 * @param flag: The status flag mask (e.g., ADC_SR_OVR).
 * @retval None
 */
void ADC_ClearFlag(uint32_t flag);

#ifdef __cplusplus
}
#endif

#endif // __ADC_H

