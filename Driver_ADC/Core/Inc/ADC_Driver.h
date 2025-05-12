/*
 * ADC_Driver.h
 *
 *  Created on: May 12, 2025
 *      Author: proha
 */

#ifndef INC_ADC_DRIVER_H_
#define INC_ADC_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"

void ADC_Init(void);
void ADC_Start(void);
uint32_t ADC_Read(void);
void ADC_DeInit(void);
