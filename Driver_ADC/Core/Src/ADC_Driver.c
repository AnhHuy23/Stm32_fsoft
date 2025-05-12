/*
 * ADC_Driver.c
 *
 *  Created on: May 12, 2025
 *      Author: by me
 */

#include "ADC_Driver.h"

void ADC_Init(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin)
{
    /* Enable ADC1 clock */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* Enable GPIO clock */
    if (GPIOx == GPIOA)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    else if (GPIOx == GPIOB)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    else if (GPIOx == GPIOC)
        RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    uint32_t position;
    uint32_t ioposition = 0x00U;
    uint32_t iocurrent = 0x00U;
    uint32_t channel = 0;

    for (position = 0U; position < 16U; position++)
    {
        ioposition = 0x01U << position;
        iocurrent = (uint32_t)(GPIO_Pin) & ioposition;

        if (iocurrent == ioposition)
        {
            /* Configure GPIO pin as analog mode */
            GPIOx->MODER |= (0x3U << (position * 2U));
            GPIOx->PUPDR &= ~(0x3U << (position * 2U)); // No pull-up, pull-down

            channel = position; // Save channel number
        }
    }

    /* Configure ADC */
    ADC1->CR1 &= ~ADC_CR1_SCAN; // Disable scan mode
    ADC1->SQR1 &= ~ADC_SQR1_L;  // Set number of conversions to 1

    if (channel <= 9)
        ADC1->SMPR2 |= (0x7U << (channel * 3)); // Maximum sampling time
    else
        ADC1->SMPR1 |= (0x7U << ((channel - 10) * 3));

    ADC1->SQR3 = channel; // Set first conversion channel
    ADC1->CR2 |= ADC_CR2_CONT; // Continuous mode
    ADC1->CR2 |= ADC_CR2_ADON; // Enable ADC
}


void ADC_DeInit(GPIO_TypeDef *GPIOx, uint32_t GPIO_Pin)
{
	/*Disable ADC1 Clock*/
	RCC->APB2RSTR |= RCC_APB2RSTR_ADC1RST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_ADC1RST;

	/* Disable GPIO clock */
	if (GPIOx == GPIOA)
		RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOAEN;
	else if (GPIOx == GPIOB)
		RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOBEN;
	else if (GPIOx == GPIOC)
		RCC->AHB1ENR &= ~RCC_AHB1ENR_GPIOCEN;

	uint32_t position;
	uint32_t ioposition = 0x00U;
	uint32_t iocurrent = 0x00U;
	uint32_t tmp = 0x00U;

	for(position = 0U; position < 16U; position++) {
	    /* Get the IO position */
	    ioposition = 0x01U << position;
	    /* Get the current IO position */
	    iocurrent = (GPIO_Pin) & ioposition;

	    if(iocurrent == ioposition)
	    {

	      /*------------------------- GPIO Mode Configuration --------------------*/
	      /* Configure IO Direction in Input Floating Mode */
	      GPIOx->MODER &= ~(0x3U << (position * 2U));

	      /* Deactivate the Pull-up and Pull-down resistor for the current IO */
	      GPIOx->PUPDR &= ~(0x3U << (position * 2U));

	      /* Configure the default value IO Output Type */
	      GPIOx->OTYPER  &= ~(0x1U << position) ;

	      /* Configure the default value for IO Speed */
	      GPIOx->OSPEEDR &= ~(0x3U << (position * 2U));
	    }
	}
	/* Disable ADC */
	ADC1->CR2 &= ~ADC_CR2_ADON; // Disable ADC
}

void ADC_Start(void)
{
	ADC1->CR2 |= ADC_CR2_ADON; // Start ADC conversion
	ADC1->CR2 |= ADC_CR2_SWSTART; // Start conversion
}

uint32_t ADC_Read(void)
{
	while (!(ADC1->SR & ADC_SR_EOC)); // Wait for conversion to complete
	return ADC1->DR; // Read conversion result
}



