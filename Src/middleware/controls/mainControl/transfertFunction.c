/**************************************************************************/
/*!
 @file    transfertFunction.c
 @author  PLF (PACABOT)
 @date
 @version  0.0
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
#include "middleware/controls/mainControl/mainControl.h"
#include "middleware/controls/mainControl/positionControl.h"
#include "middleware/controls/mainControl/positionControl.h"
#include "middleware/controls/mainControl/speedControl.h"
#include "middleware/controls/mainControl/transfertFunction.h"

/* Peripheral declarations */
#include "peripherals/motors/bldc.h"
#include "peripherals/times_base/times_base.h"

/* Middleware declarations */
#include "middleware/controls/pidController/pidController.h"

transfert_function_struct transfert_function;

int transfertFunctionInit(void)
{
    transfert_function.left_motor_pwm = 0;
    transfert_function.right_motor_pwm = 0;
    transfert_function.pwm_ratio = 0;
    return TRANSFERT_FUNCTION_E_SUCCESS;
}

int transfertFunctionLimiter(void)
{
    if (transfert_function.left_motor_pwm > MOTORS_PERIOD)
    {
        transfert_function.left_motor_pwm = MOTORS_PERIOD;
        transfert_function.right_motor_pwm -= (transfert_function.left_motor_pwm - MOTORS_PERIOD);
    }

    if (transfert_function.left_motor_pwm < -MOTORS_PERIOD)
    {
        transfert_function.left_motor_pwm = -MOTORS_PERIOD;
        transfert_function.right_motor_pwm -= (transfert_function.left_motor_pwm + MOTORS_PERIOD);
    }

    if (transfert_function.right_motor_pwm > MOTORS_PERIOD)
    {
        transfert_function.right_motor_pwm = MOTORS_PERIOD;
        transfert_function.left_motor_pwm -= (transfert_function.right_motor_pwm - MOTORS_PERIOD);
    }

    if (transfert_function.right_motor_pwm < -MOTORS_PERIOD)
    {
        transfert_function.right_motor_pwm = -MOTORS_PERIOD;
        transfert_function.left_motor_pwm -= (transfert_function.right_motor_pwm + MOTORS_PERIOD);
    }

    return TRANSFERT_FUNCTION_E_SUCCESS;
}

int transfertFunctionLoop(void)
{
    //	if (multimeterGetBatVoltage() > 5000 && multimeterGetBatVoltage() < 9000)
    //		transfert_function.pwm_ratio = (PWM_RATIO_COEFF_A * multimeterGetBatVoltage() + PWM_RATIO_COEFF_B);	//compute ratio to not exceed motor voltage
    //	else
    transfert_function.pwm_ratio = (PWM_RATIO_COEFF_A * 7400.00 + PWM_RATIO_COEFF_B);	//if vbat read fail

    transfert_function.right_motor_pwm = (speedControlGetSpeedCommand()
            - (positionControlGetPositionCommand()))
                    * transfert_function.pwm_ratio;
    transfert_function.left_motor_pwm = (speedControlGetSpeedCommand()
            + (positionControlGetPositionCommand()))
                    * transfert_function.pwm_ratio;

    transfertFunctionLimiter();
    motorSet(MOT_R, transfert_function.right_motor_pwm);
    motorSet(MOT_L, transfert_function.left_motor_pwm);

    //	bluetoothPrintf("pwmR: %d \r\n", (transfert_function.right_motor_pwm));
    return TRANSFERT_FUNCTION_E_SUCCESS;
}
