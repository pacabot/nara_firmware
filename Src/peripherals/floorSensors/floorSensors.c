/**************************************************************************/
/*!
 @file    floorSensors.c
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
#include <peripherals/floorSensors/floorSensors.h>

/* Definition for ADCx's Channel */
//#define RX_LEFT_EXT                 ADC_CHANNEL_3   //ADC3
//#define RX_LEFT                     ADC_CHANNEL_4   //ADC2
//#define RX_RIGHT                    ADC_CHANNEL_13  //ADC2
//#define RX_RIGHT_EXT                ADC_CHANNEL_12  //ADC3

/* Types definitions */
typedef struct
{
	uint32_t adc_value;
	uint32_t ref_adc_value;
} floorSensors_state;

typedef struct
{
	floorSensors_state left_ext;
	floorSensors_state left;
	floorSensors_state right;
	floorSensors_state right_ext;
	char active_ADC1;
	char active_ADC2;
	char emitter_state;
	char active_state;
	char selector;
} floorSensors_struct;

volatile floorSensors_struct floorSensors;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern TIM_HandleTypeDef htim3;

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
void floorSensorsInit(void)
{
	memset((floorSensors_struct*) &floorSensors, 0, sizeof(floorSensors_struct));

	HAL_ADC_Stop_IT(&hadc1);
	HAL_ADC_Stop_IT(&hadc2);

	floorSensorsStart();
}

void floorSensorsStart(void)
{
	floorSensors.active_state = TRUE;
	HAL_ADCEx_InjectedStart_IT(&hadc1);
	HAL_ADCEx_InjectedStart_IT(&hadc2);
}

void floorSensorsStop(void)
{
	floorSensors.active_state = FALSE;
	HAL_GPIO_WritePin(FLOOR_EN_GPIO_Port, FLOOR_EN_Pin, RESET);
	HAL_ADCEx_InjectedStop_IT(&hadc1);
	HAL_ADCEx_InjectedStop_IT(&hadc2);
}

double getFloorSensorAdc(enum floorSensorName floorSensor_name)
{
	switch (floorSensor_name)
	{
	case FLOORSENSOR_EXT_L:
		return floorSensors.left_ext.adc_value;
	case FLOORSENSOR_L:
		return floorSensors.left.adc_value;
	case FLOORSENSOR_R:
		return floorSensors.right.adc_value;
	case FLOORSENSOR_EXT_R:
		return floorSensors.right_ext.adc_value;
	default :
		return 0.00;
	}
}

void __INLINE floorSensors_IT(void)
{
	if (floorSensors.active_state == FALSE)
		return;

	floorSensors.selector++;

	if (floorSensors.selector > 4)
	{
		floorSensors.selector = 0;
	}

	switch (floorSensors.selector)
	{
	case 0:
//		HAL_GPIO_WritePin(FLOOR_EN_GPIO_Port, FLOOR_EN_Pin, SET);
		HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
		floorSensors.emitter_state = TRUE;
		break;
	case 1:
		floorSensors.active_ADC1 = TRUE;
		floorSensors.active_ADC2 = TRUE;
		HAL_ADCEx_InjectedStart_IT(&hadc1);
		HAL_ADCEx_InjectedStart_IT(&hadc2);
		break;
	case 3:
		HAL_GPIO_WritePin(FLOOR_EN_GPIO_Port, FLOOR_EN_Pin, RESET);
		HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
		floorSensors.emitter_state = FALSE;
		break;
	case 4:
		floorSensors.active_ADC1 = TRUE;
		floorSensors.active_ADC2 = TRUE;
		HAL_ADCEx_InjectedStart_IT(&hadc1);
		HAL_ADCEx_InjectedStart_IT(&hadc2);
		break;
	}
}

void __INLINE floorSensors_ADC_IT(ADC_HandleTypeDef *hadc)
{
	if (floorSensors.emitter_state == TRUE)
	{
		if (hadc == &hadc1)
		{
			floorSensors.left_ext.adc_value = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1) + ((4095 - floorSensors.left_ext.ref_adc_value) / 2);
			floorSensors.right_ext.adc_value = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2) + ((4095 - floorSensors.right_ext.ref_adc_value) / 2);
			floorSensors.active_ADC1 = FALSE;
		}
		if (hadc == &hadc2)
		{
			floorSensors.right.adc_value = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2) + ((4095 - floorSensors.right.ref_adc_value) / 2);
			floorSensors.left.adc_value = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1) + ((4095 - floorSensors.left.ref_adc_value) / 2);
			floorSensors.active_ADC2 = FALSE;
		}
			if (floorSensors.active_ADC1 == FALSE && floorSensors.active_ADC2 == FALSE)
			{
				HAL_GPIO_WritePin(FLOOR_EN_GPIO_Port, FLOOR_EN_Pin, RESET);
				HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
				floorSensors.emitter_state = FALSE;
			}
	} else
	{
		if (hadc == &hadc1)
		{
			floorSensors.left_ext.ref_adc_value = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
			floorSensors.right_ext.ref_adc_value = HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_2);
		}
		if (hadc == &hadc2)
		{
			floorSensors.right.ref_adc_value = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_1);
			floorSensors.left.ref_adc_value = HAL_ADCEx_InjectedGetValue(&hadc2, ADC_INJECTED_RANK_2);
		}
	}
}

void floorSensorsTest(void)
{
	floorSensorsInit();

	while(1)
	{
		printf("L_EXT = %d  L_EXT_REF = %d   ",  	(int)getFloorSensorAdc(FLOORSENSOR_EXT_L), 	(int)floorSensors.left_ext.ref_adc_value);
		printf("L = %d  L_REF = %d   ",    		(int)getFloorSensorAdc(FLOORSENSOR_L), 		(int)floorSensors.left.ref_adc_value);
		printf("R = %d  R_REF = %d   ",      		(int)getFloorSensorAdc(FLOORSENSOR_R), 		(int)floorSensors.right.ref_adc_value);
		printf("R_EXT = %d  R_EXT_REF = %d\n",	(int)getFloorSensorAdc(FLOORSENSOR_EXT_R), 	(int)floorSensors.right_ext.ref_adc_value);
		printf("------------------------------------------------------------------------------------------------------------------------\n");
		HAL_Delay(500);
	}

	floorSensorsStop();
}

