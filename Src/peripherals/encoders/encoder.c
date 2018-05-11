/**************************************************************************/
/*!
 @file    hall.c
 @author  PLF (PACABOT)
 @date    1 May 2018
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

/* Peripheral declarations */
#include "peripherals/motors/bldc.h"

/* Middleware declarations */

/* Declarations for this module */
#include "peripherals/encoders/encoder.h"

// Machine Definitions
typedef struct
{
	volatile double abs_dist;
	volatile double offset_dist;
	volatile double rel_dist;
	signed int mot_rev_cnt;
	uint16_t oldCommPos;
	TIM_HandleTypeDef *timer;
} encoder;

static const uint8_t HALL_STEPS[8] = {4, 6, 2, 3, 1, 5, 4, 6}; // multiple step 1 and 8 for optimize code

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;

/**************************************************************************/
/* Structure init                                                         */
/**************************************************************************/
// Global variable
volatile encoder left_encoder = { 0, 0, 0, 0, 0, &htim4 };

volatile encoder right_encoder = { 0, 0, 0, 0, 0, &htim2 };

/* Static functions */
static int encoderResetDistance(encoder *enc);
static double encoderGetDistance(encoder *enc);
static double encoderGetAbsDistance(encoder *enc);

void encodersInit(void)
{
	HAL_GPIO_WritePin(PW_HALL_EN_GPIO_Port, PW_HALL_EN_Pin, 1);

	HAL_TIMEx_HallSensor_Start_IT(&htim4);
	HAL_TIMEx_HallSensor_Start_IT(&htim2);

	left_encoder.mot_rev_cnt = 0;
	right_encoder.mot_rev_cnt = 0;
}

void encodersStop(void)
{
	HAL_GPIO_WritePin(PW_HALL_EN_GPIO_Port, PW_HALL_EN_Pin, 0);
	left_encoder.mot_rev_cnt = 0;
	right_encoder.mot_rev_cnt = 0;
}

void encoderLeft_IT(void)
{
	uint16_t newcomm_pos = 0;

	newcomm_pos = ((HAL_GPIO_ReadPin(MOT_L_HALL1_GPIO_Port, MOT_L_HALL1_Pin)) |
			(HAL_GPIO_ReadPin(MOT_L_HALL2_GPIO_Port, MOT_L_HALL2_Pin) << 1) |
			(HAL_GPIO_ReadPin(MOT_L_HALL3_GPIO_Port, MOT_L_HALL3_Pin) << 2));

	//	printf("Hall val = %d\n", newcomm_pos);

	for (int i = 1; i <= 6; i++)
	{
		if (newcomm_pos == HALL_STEPS[i])
		{
			if (left_encoder.oldCommPos == HALL_STEPS[i+1])
				left_encoder.mot_rev_cnt++;
			else if (left_encoder.oldCommPos == HALL_STEPS[i-1])
				left_encoder.mot_rev_cnt--;
			else
				printf("Hall left step error\n");
		}
	}

	left_encoder.oldCommPos = newcomm_pos;
}

void encoderRight_IT(void)
{
	uint16_t newcomm_pos = 0;

	newcomm_pos = ((HAL_GPIO_ReadPin(MOT_R_HALL1_GPIO_Port, MOT_R_HALL1_Pin)) |
			(HAL_GPIO_ReadPin(MOT_R_HALL2_GPIO_Port, MOT_R_HALL2_Pin) << 1) |
			(HAL_GPIO_ReadPin(MOT_R_HALL3_GPIO_Port, MOT_R_HALL3_Pin) << 2));

	//	printf("Hall val = %d\n", newcomm_pos);

	for (int i = 1; i <= 6; i++)
	{
		if (newcomm_pos == HALL_STEPS[i])
		{
			if (right_encoder.oldCommPos == HALL_STEPS[i+1])
				right_encoder.mot_rev_cnt--;
			else if (right_encoder.oldCommPos == HALL_STEPS[i-1])
				right_encoder.mot_rev_cnt++;
			else
				printf("Hall right step error\n");
		}
	}

	right_encoder.oldCommPos = newcomm_pos;
}

/*  encoderResetDistance
 *  set offset to current absolute distance
 *  offset in millimeters
 */
int encoderResetDistance(encoder *enc)
{
	enc->offset_dist = (double)enc->mot_rev_cnt / STEPS_PER_MM;
	return ENCODER_DRIVER_E_SUCCESS;
}

/*  encoderGetDistance
 *  return current relative distance and set absolute distance
 *  distance in millimeters
 */
double encoderGetDistance(encoder *enc)
{
	return enc->rel_dist = ((double)enc->mot_rev_cnt / STEPS_PER_MM) - (double)enc->offset_dist;
}

/*  encoderGetAbsDistance
 *  return absolute distance and set absolute distance
 *  distance in millimeters
 */
double encoderGetAbsDistance(encoder *enc)
{
	return enc->abs_dist = (double)enc->mot_rev_cnt / STEPS_PER_MM;
}

int encodersReset(void)
{
	encoderResetDistance((encoder*) &left_encoder);
	encoderResetDistance((encoder*) &right_encoder);
	return ENCODER_DRIVER_E_SUCCESS;
}

double encoderGetDist(enum encoderName encoder_name)
{
	static int i = 0;
	i++;
	if (!(i % 500))
	{
		printf("L REV CNT = %d  ", (signed int)left_encoder.mot_rev_cnt);
		printf("R REV CNT = %d\n", (signed int)right_encoder.mot_rev_cnt);
	}

	if (encoder_name == ENCODER_L)
		return encoderGetDistance((encoder*) &left_encoder);
	if (encoder_name == ENCODER_R)
		return encoderGetDistance((encoder*) &right_encoder);

	return ENCODER_DRIVER_E_ERROR;
}

double encoderGetAbsDist(enum encoderName encoder_name)
{
	if (encoder_name == ENCODER_L)
		return encoderGetAbsDistance((encoder*) &left_encoder);
	if (encoder_name == ENCODER_R)
		return encoderGetAbsDistance((encoder*) &right_encoder);

	return ENCODER_DRIVER_E_ERROR;
}

// test encoder
void encodersTest(void)
{
	encodersInit();
	encodersReset();

	while (1)
	{
		//		printf("L_DIST_REL = %.1f  ", encoderGetDist(ENCODER_L));
		//		printf("L_DIST_ABS = %.1f  ", encoderGetAbsDist(ENCODER_L));
		//		printf("R_DIST_REL = %.1f  ", encoderGetDist(ENCODER_R));
		//		printf("R_DIST_ABS = %.1f\n", encoderGetAbsDist(ENCODER_R));

		printf("L REV CNT = %d  ", (signed int)left_encoder.mot_rev_cnt);
		printf("R REV CNT = %d\n", (signed int)right_encoder.mot_rev_cnt);

		printf("-----------------------------------------------------------------------\n");
		HAL_Delay(500);
		HAL_Delay(10);
	}
}
