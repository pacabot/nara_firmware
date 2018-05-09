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

#define WELL_TURN_NB	10

// Machine Definitions
typedef struct
{
    volatile double abs_dist;
    volatile double offset_dist;
    volatile double rel_dist;
    signed int mot_rev_cnt;
    TIM_HandleTypeDef *timer;
} encoder;

extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;

/**************************************************************************/
/* Structure init                                                 */
/**************************************************************************/
// Global variable
volatile encoder left_encoder = { 0, 0, 0, 0, &htim4 };

volatile encoder right_encoder = { 0, 0, 0, 0, &htim2 };

/* Static functions */
static int encoderResetDistance(encoder *enc);
static double encoderGetDistance(encoder *enc);
static double encoderGetAbsDistance(encoder *enc);

void encodersInit(void)
{
	HAL_GPIO_WritePin(PW_HALL_EN_GPIO_Port, PW_HALL_EN_Pin, 1);
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
    switch (__HAL_TIM_DIRECTION_STATUS(&htim4))
    {
        case 1:
            left_encoder.mot_rev_cnt--;
            break;
        case 0:
            left_encoder.mot_rev_cnt++;
            break;
    }
}

void encoderRight_IT(void)
{
    switch (__HAL_TIM_DIRECTION_STATUS(&htim2))
    {
        case 1:
            right_encoder.mot_rev_cnt--;
            break;
        case 0:
            right_encoder.mot_rev_cnt++;
            break;
    }
}

/*  encoderResetDistance
 *  set offset to current absolute distance
 *  offset in millimeters
 */
int encoderResetDistance(encoder *enc)
{
    enc->offset_dist = ((((double) enc->mot_rev_cnt * ENCODER_RESOLUTION * (double) WELL_TURN_NB)
            + ((double) __HAL_TIM_GetCounter(enc->timer))) /
    STEPS_PER_MM);
    return ENCODER_DRIVER_E_SUCCESS;
}

/*  encoderGetDistance
 *  return current relative distance and set absolute distance
 *  distance in millimeters
 */
double encoderGetDistance(encoder *enc)
{
    enc->rel_dist = (((((double) enc->mot_rev_cnt * ENCODER_RESOLUTION * (double) WELL_TURN_NB)
            + ((double) __HAL_TIM_GetCounter(enc->timer))) /
    STEPS_PER_MM) - (double) enc->offset_dist);
    return enc->rel_dist;
}

/*  encoderGetAbsDistance
 *  return absolute distance and set absolute distance
 *  distance in millimeters
 */
double encoderGetAbsDistance(encoder *enc)
{
    enc->abs_dist = ((((double) enc->mot_rev_cnt * ENCODER_RESOLUTION * (double) WELL_TURN_NB)
            + ((double) __HAL_TIM_GetCounter(enc->timer))) /
    STEPS_PER_MM);
    return enc->abs_dist;
}

int encodersReset(void)
{
    encoderResetDistance((encoder*) &left_encoder);
    encoderResetDistance((encoder*) &right_encoder);
    return ENCODER_DRIVER_E_SUCCESS;
}

double encoderGetDist(enum encoderName encoder_name)
{
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
void encoderTest(void)
{
    encodersInit();
    encodersReset();
}
