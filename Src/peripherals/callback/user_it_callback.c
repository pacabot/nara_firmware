/**************************************************************************/
/*!
 @file    user_it_callback.c
 @author  PLF (PACABOT)
 @date    29 April 2018
 */
/**************************************************************************/
/* STM32 hal library declarations */
#include <peripherals/floorSensors/floorSensors.h>
#include "stm32h7xx_hal.h"

/* General declarations */
#include "config/basetypes.h"
#include "config/config.h"
#include "config/errors.h"

/* Application declarations */

/* Middleware declarations */
#include "middleware/controls/mainControl/mainControl.h"

/* Peripheral declarations */
#include "peripherals/times_base/times_base.h"
#include "peripherals/tone/tone.h"
#include "peripherals/telemeters/telemeters.h"
#include "peripherals/encoders/encoder.h"

#include "peripherals/callback/user_it_callback.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim4;
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

/* TIM callback --------------------------------------------------------------*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    static uint32_t cnt = 0;
    static uint32_t rv = MAIN_CONTROL_E_SUCCESS;
    if (htim == &htim7) //high time freq
    {
        cnt++;
        if (cnt % (int)(HI_TIME_FREQ / TELEMETERS_TIME_FREQ) == 0)
        {
            telemeters_IT();
        }
        if (cnt % (int)(HI_TIME_FREQ / CONTROL_TIME_FREQ) == 0)
        {
            if (rv == MAIN_CONTROL_E_SUCCESS)
                rv = mainControl_IT();
            else
            {
//                telemetersStop();
//                motorsDriverSleep(ON);
//                motorsBrake();
                HAL_TIM_Base_Stop_IT(&htim7);
                tone(A4, 4000);
            }
        }
        if (cnt % (int)(HI_TIME_FREQ / LINESENSORS_TIME_FREQ) == 0)
        {
            floorSensors_IT();
        }
        if (cnt % (int)(HI_TIME_FREQ / LOW_TIME_FREQ) == 0)
        {
            tone_IT();
            //sleep_mode_IT();
            ledBlink_IT();
        }
    }
//    if (htim == &htim4)
//    {
//        encoderLeft_IT();
//    }
//    if (htim == &htim2)
//    {
//        encoderRight_IT();
//    }
}

/* ADC callback --------------------------------------------------------------*/
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1 || hadc == &hadc2)
    {
        floorSensors_ADC_IT(hadc);
    }
}
