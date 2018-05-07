/**************************************************************************/
/*!
 @file    lineSensor.c
 @author   PLF Pacabot.com
 @date     01 December 2014
 @version  0.10
 */
/**************************************************************************/
/* STM32 hal library declarations */
#include "stm32h7xx_hal.h"

/* General declarations */
#include "config/basetypes.h"
#include "config/config.h"
#include "config/errors.h"

#include "stdbool.h"
#include <arm_math.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/* Middleware declarations */

/* Peripheral declarations */

/* Declarations for this module */
#include "peripherals/lineSensors/lineSensors.h"

/* Definition for ADCx's Channel */
#define RX_LEFT_EXT                 ADC_CHANNEL_3   //ADC3
#define RX_LEFT                     ADC_CHANNEL_4   //ADC2
#define RX_RIGHT                    ADC_CHANNEL_13  //ADC2
#define RX_RIGHT_EXT                ADC_CHANNEL_12  //ADC3

/* Types definitions */
typedef struct
{
    uint32_t adc_value;
    uint32_t ref_adc_value;
} lineSensors_state;

typedef struct
{
    lineSensors_state left_ext;
    lineSensors_state left;
    lineSensors_state right;
    lineSensors_state right_ext;
    char active_ADC1;
    char active_ADC2;
    char emitter_state;
    char active_state;
    char selector;
} lineSensors_struct;

volatile lineSensors_struct lineSensors;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

ADC_InjectionConfTypeDef sConfigInjected;

GPIO_InitTypeDef GPIO_InitStruct;
/**
 * @brief  Period elapsed callback in non blocking mode
 * @param  htim : TIM handle
 * @retval None
 */

/**************************************************************************/
/*!
 RANK 1		INP18  	ADC 1 				LEFT_EXT
 RANK 1		INP19  	ADC 2 				LEFT
 RANK 2		INP11 	ADC 2 				RIGHT
 RANK 2		INP10 	ADC 1 				RIGHT_EXT
 */
/**************************************************************************/
void lineSensorsInit(void)
{
    HAL_ADC_Stop_IT(&hadc1);
    HAL_ADC_Stop_IT(&hadc2);

    memset((lineSensors_struct*) &lineSensors, 0, sizeof(lineSensors_struct));
}

void lineSensorsStart(void)
{
    lineSensors.active_state = TRUE;
    HAL_ADCEx_InjectedStart_IT(&hadc1);
    HAL_ADCEx_InjectedStart_IT(&hadc2);
}

void lineSensorsStop(void)
{
    lineSensors.active_state = FALSE;
    HAL_GPIO_WritePin(LINE_EN_GPIO_Port, LINE_EN_Pin, RESET);
    HAL_ADCEx_InjectedStop_IT(&hadc1);
    HAL_ADCEx_InjectedStop_IT(&hadc2);
}

double getLineSensorAdc(enum linesensorName linesensor_name)
{
    switch (linesensor_name)
    {
        case LINESENSOR_EXT_L:
            return lineSensors.left_ext.adc_value;
        case LINESENSOR_L:
            return lineSensors.left.adc_value;
        case LINESENSOR_R:
            return lineSensors.right.adc_value;
        case LINESENSOR_EXT_R:
            return lineSensors.right_ext.adc_value;
        default :
            return 0.00;
    }
}

void lineSensors_IT(void)
{
    if (lineSensors.active_state == FALSE)
        return;

    lineSensors.selector++;

    if (lineSensors.selector > 1)
    {
        lineSensors.selector = 0;
    }

    switch (lineSensors.selector)
    {
        case 0:
            HAL_GPIO_WritePin(LINE_EN_GPIO_Port, LINE_EN_Pin, SET);
            lineSensors.emitter_state = TRUE;
            break;
        case 1:
            lineSensors.active_ADC1 = TRUE;
            lineSensors.active_ADC2 = TRUE;
            HAL_ADCEx_InjectedStart_IT(&hadc1);
            HAL_ADCEx_InjectedStart_IT(&hadc2);
            break;
    }
}

void lineSensors_ADC_IT(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1)
    {
        lineSensors.left_ext.adc_value = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
        lineSensors.right_ext.adc_value = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);
        lineSensors.active_ADC1 = FALSE;
    }
    if (hadc == &hadc2)
    {
        lineSensors.right.adc_value = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1);
        lineSensors.left.adc_value = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2);
        lineSensors.active_ADC2 = FALSE;
    }
    if (lineSensors.active_ADC1 == FALSE && lineSensors.active_ADC2 == FALSE)
    {
        HAL_GPIO_WritePin(LINE_EN_GPIO_Port, LINE_EN_Pin, RESET);
        lineSensors.emitter_state = FALSE;
    }
}

void lineSensorsTest(void)
{
    lineSensorsInit();
    lineSensorsStart();

    while(1)
    {
    printf("L_EXT = %d  %d", (uint16_t)getLineSensorAdc(LINESENSOR_EXT_L), (int)lineSensors.left_ext.ref_adc_value);
    printf("L     = %d  %d", (uint16_t)getLineSensorAdc(LINESENSOR_L), (int)lineSensors.left.ref_adc_value);
    printf("R     = %d  %d", (uint16_t)getLineSensorAdc(LINESENSOR_R), (int)lineSensors.right.ref_adc_value);
    printf("R_EXT = %d  %d", (uint16_t)getLineSensorAdc(LINESENSOR_EXT_R), (int)lineSensors.right_ext.ref_adc_value);
    }

    lineSensorsStop();
}

