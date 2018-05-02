/**************************************************************************/
/*!
 @file    bldc.h
 @author  PLF (PACABOT)
 @date    10 January 2018
 */
/**************************************************************************/

#ifndef __BLDC_H
#define __BLDC_H

/* Module Identifier */
#include "config/module_id.h"

/* Error codes */
#define BLDC_DRIVER_E_SUCCESS  0
#define BLDC_DRIVER_E_ERROR    MAKE_ERROR(BLDC_DRIVER_MODULE_ID, 1)

/* Types definitions */
enum motorDirection
{
    MOT_CW, MOT_CCW
};

enum motorType
{
    MOT_L, MOT_R
};

/**
 * @brief  BLDC Init
 * @param  None
 * @retval None
 */
void motorsInit(void);

/**
 * @brief  Commutation event callback in non blocking mode
 * @param  motorType : MOT_L or MOT_R, Direction : CW or CCW
 * @retval None
 */
void motorSetDir(enum motorType type, enum motorDirection dir);

/**
 * @brief  Commutation event callback in non blocking mode
 * @param  motorType : MOT_L or MOT_R, Duty cycle : 0 to 4096
 * @retval None
 */
void motorSet(enum motorType type, uint16_t duty_cycle);

/**
 * @brief  Brake all motors
 * @param  None
 * @retval None
 */

void motorsBrake(void);
void motorsDriverSleep(int isOn);
void motors_test(void);

#endif
