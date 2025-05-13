/*
 * adc.c
 *
 *  Created on: May 12, 2025
 *      Author: proha
 */

#include "adc.h"

// --- Helper Macros ---
#define ADC_CR1_RES_Pos             (24U) // Position of RES bits in CR1
#define ADC_CCR_ADCPRE_Pos          (16U) // Position of ADCPRE bits in CCR
#define ADC_SMPR_SMP_CLEAR_Mask     (0x07UL) // Mask to clear SMP bits (3 bits)
#define ADC_SQR3_SQ1_CLEAR_Mask     (0x1FUL) // Mask to clear SQ1 bits (5 bits)


static void simple_delay(volatile uint32_t count) {
    while(count--);
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Enables the clock for the selected ADC peripheral.
 */
void ADC_ClockEnable(void) {
    /* Use #ifdef based on the USE_ADCx selection from the header */
    #ifdef USE_ADC1
        RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
        (void)RCC->APB2ENR;
    #elif defined(USE_ADC2)
        RCC->APB2ENR |= RCC_APB2ENR_ADC2EN;
        (void)RCC->APB2ENR;
    #elif defined(USE_ADC3)
        RCC->APB2ENR |= RCC_APB2ENR_ADC3EN;
        (void)RCC->APB2ENR;
    #endif
}


/**
 * @brief Disables the clock for the selected ADC peripheral.
 */
void ADC_ClockDisable(void) {
    /* Use #ifdef based on the USE_ADCx selection from the header */
    #ifdef USE_ADC1
        RCC->APB2ENR &= ~RCC_APB2ENR_ADC1EN;
    #elif defined(USE_ADC2)
        RCC->APB2ENR &= ~RCC_APB2ENR_ADC2EN;
    #elif defined(USE_ADC3)
        RCC->APB2ENR &= ~RCC_APB2ENR_ADC3EN;
    #endif
}

/**
 * @brief Initializes the ADC peripheral defined by ADC_DRIVER_INSTANCE.
 */
void ADC_Init(ADC_Resolution resolution, ADC_DataAlign alignment, ADC_ClockPrescaler prescaler) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE; // Use the defined instance

    /* Ensure ADC is disabled before configuration */
    if (adc->CR2 & ADC_CR2_ADON) {
        adc->CR2 &= ~ADC_CR2_ADON;
        simple_delay(100);
    }

    /* 1. Configure Common ADC settings (CCR) */
    /* Reset prescaler bits */
    ADC->CCR &= ~ADC_CCR_ADCPRE;
    /* Set new prescaler */
    ADC->CCR |= (uint32_t)prescaler << ADC_CCR_ADCPRE_Pos;
    /* Configure other CCR settings if needed (e.g., DMA, Multi-mode, Vbat/Temp sensor - default off) */
    ADC->CCR &= ~(ADC_CCR_MULTI | ADC_CCR_DMA |
    		ADC_CCR_DDS | ADC_CCR_DELAY | ADC_CCR_TSVREFE | ADC_CCR_VBATE);


    /* 2. Configure ADC Control Register 1 (CR1) */
    /* Reset relevant bits: Clear RES, SCAN, DISCEN, OVRIE, EOCIE */
    adc->CR1 &= ~(ADC_CR1_RES | ADC_CR1_SCAN | ADC_CR1_DISCEN | ADC_CR1_OVRIE | ADC_CR1_EOCIE);
    /* Set resolution */
    adc->CR1 |= ((uint32_t)resolution << ADC_CR1_RES_Pos);


    /* 3. Configure ADC Control Register 2 (CR2) */
    /* Reset relevant bits */
    adc->CR2 &= ~(ADC_CR2_ALIGN | ADC_CR2_CONT | ADC_CR2_DMA |
    			ADC_CR2_DDS | ADC_CR2_EOCS | ADC_CR2_EXTEN | ADC_CR2_EXTSEL | ADC_CR2_SWSTART);
    /* Set data alignment */
    if (alignment == ADC_ALIGN_LEFT) {
        adc->CR2 |= ADC_CR2_ALIGN;
    }

    /* EOCS = 0: EOC flag is set at the end of single channel conversion (needed for basic polling/IT) */
    adc->CR2 &= ~ADC_CR2_EOCS;
    /* CONT = 0: Single conversion mode */
    adc->CR2 &= ~ADC_CR2_CONT;


    /* 4. Configure Sequence Length (SQR1) */
    /* Set sequence length to 1 (for single channel conversion) */
    adc->SQR1 &= ~ADC_SQR1_L;


    /* 5. Clear Status Register */
    adc->SR = 0; // Clear all status flags
}


/**
 * @brief De-initializes the ADC peripheral (resets registers).
 */
void ADC_DeInit(void) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE;

    /* Disable ADC first */
    if (adc->CR2 & ADC_CR2_ADON) {
       ADC_Disable();
    }

    /*--- Reset ADC Registers (Consult RM for specific reset values if needed, often 0) ---*/
    adc->CR1 = 0x00000000;
    adc->CR2 = 0x00000000;
    adc->SMPR1 = 0x00000000;
    adc->SMPR2 = 0x00000000;
    adc->JOFR1 = 0x00000000;
    adc->JOFR2 = 0x00000000;
    adc->JOFR3 = 0x00000000;
    adc->JOFR4 = 0x00000000;
    adc->HTR = 0x00000FFF;
    adc->LTR = 0x00000000;
    adc->SQR1 = 0x00000000;
    adc->SQR2 = 0x00000000;
    adc->SQR3 = 0x00000000;
    adc->JSQR = 0x00000000;
    adc->SR = 0x00000000;
}

/**
 * @brief Configures a single channel for regular conversion sequence.
 */
void ADC_ConfigChannel(uint8_t channel, ADC_SampleTime sampleTime) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE;
    /* 0 for SMPR2, 1 for SMPR1 */
    uint32_t smpr_index;
    uint32_t smp_bit_offset;

    /* Invalid channel for F4 */
    if (channel > 18) return;

    /* 1. Configure Sampling Time ---
     * Determine which SMPR register (SMPR1 for channels 10-18, SMPR2 for 0-9)
     * and the bit offset within the register (3 bits per channel)
    */
    if (channel < 10) {
        smpr_index = 2; // Use SMPR2
        smp_bit_offset = (channel % 10) * 3;
        /* Clear the 3 bits for the channel */
        adc->SMPR2 &= ~(ADC_SMPR_SMP_CLEAR_Mask << smp_bit_offset);
        /* Set the new sampling time */
        adc->SMPR2 |= ((uint32_t)sampleTime << smp_bit_offset);
    } else {
        smpr_index = 1; // Use SMPR1
        smp_bit_offset = (channel % 10) * 3;
        /* Clear the 3 bits for the channel */
        adc->SMPR1 &= ~(ADC_SMPR_SMP_CLEAR_Mask << smp_bit_offset);
        /* Set the new sampling time */
        adc->SMPR1 |= ((uint32_t)sampleTime << smp_bit_offset);
    }

    /* 2. Configure Regular Sequence (Rank 1)
     * Assuming sequence length is 1 (set in Init)
     * Set the first conversion in the sequence (SQ1) in SQR3 register */
    adc->SQR3 &= ~ADC_SQR3_SQ1;
    /* Set channel number (0-18) */
    adc->SQR3 |= (channel & ADC_SQR3_SQ1_CLEAR_Mask);
}


/**
 * @brief Enables the ADC peripheral (sets the ADON bit).
 */
void ADC_Enable(void) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE;
    if (!(adc->CR2 & ADC_CR2_ADON)) {
        adc->CR2 |= ADC_CR2_ADON;
        simple_delay(100);
    }
}

/**
 * @brief Disables the ADC peripheral (clears the ADON bit).
 */
void ADC_Disable(void) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE;
    if (adc->CR2 & ADC_CR2_ADON) {
        adc->CR2 &= ~ADC_CR2_ADON;
        simple_delay(10); // Small delay
    }
}

/**
 * @brief Starts a single ADC conversion using software trigger.
 */
void ADC_StartConversionSW(void) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE;
    /* EOCS=0: EOC flag set at end of conversion */
    adc->CR2 &= ~ADC_CR2_EOCS;

    /* Clear EOC flag before starting conversion */
    adc->SR &= ~ADC_SR_EOC;

    /* Set SWSTART bit to begin conversion */
    adc->CR2 |= ADC_CR2_SWSTART;
}

/**
 * @brief Polls the End Of Conversion (EOC) flag.
 */
uint8_t ADC_PollForConversion(volatile uint32_t timeout) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE;
    uint8_t timed_out = 0;

    if (timeout == 0) {
        while (!(adc->SR & ADC_SR_EOC)) {
        }
    } else {
        /* Wait with timeout */
        while (!(adc->SR & ADC_SR_EOC)) {
            if (--timeout == 0) {
                timed_out = 1;
                break;
            }
        }
    }

    /* Return 1 if EOC is set, 0 if timed out */
    return (timed_out == 0);
}

/**
 * @brief Reads the converted data from the ADC data register.
 */
uint16_t ADC_ReadValue(void) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE;
    /* Reading DR clears the EOC flag in single conversion mode when EOCS=0 */
    return (uint16_t)(adc->DR);
}

/**
 * @brief Enables the End Of Conversion (EOC) interrupt.
 */
void ADC_EnableEOCInterrupt(void) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE;
    adc->CR1 |= ADC_CR1_EOCIE;
    /* Enable NVIC interrupt for ADC */
    NVIC_EnableIRQ(ADC_IRQn);
    NVIC_SetPriority(ADC_IRQn, 1);
}

/**
 * @brief Disables the End Of Conversion (EOC) interrupt.
 */
void ADC_DisableEOCInterrupt(void) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE;
    adc->CR1 &= ~ADC_CR1_EOCIE;
    NVIC_DisableIRQ(ADC_IRQn);
}

/**
 * @brief Checks if a specific ADC status flag is set.
 */
uint8_t ADC_GetFlagStatus(uint32_t flag) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE;
    return ((adc->SR & flag) != 0);
}

/**
 * @brief Clears a specific ADC status flag.
 */
void ADC_ClearFlag(uint32_t flag) {
    ADC_TypeDef* adc = ADC_DRIVER_INSTANCE;
    /* Standard way to clear flags by writing 0 */
    adc->SR &= ~flag;
}
