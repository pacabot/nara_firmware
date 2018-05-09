/**************************************************************************/
/*!
 @file    TimeBase.c
 @author  PLF (PACABOT)
 @date    29 April 2018
 */
/**************************************************************************/
/* STM32 hal library declarations */
#include "stm32h7xx_hal.h"

/* General declarations */
#include "config/basetypes.h"
#include "config/config.h"
#include "config/errors.h"

/* Middleware declarations */
#include "middleware/controls/pidController/pidController.h"

/* Peripheral declarations */
#include "peripherals/tone/tone.h"
#include "peripherals/callback/user_it_callback.h"

/* Declarations for this module */
#include "peripherals/times_base/times_base.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile int32_t Blink[3] = { 500, 10, 0 };

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim7;

/**
 * @brief  Period elapsed callback in non blocking mode
 * @param  htim : TIM handle
 * @retval None
 */
void timesBaseInit(void)
{
//    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
//    TIM_OC_InitTypeDef sConfigOC;
    uint32_t uwPrescalerValue = 0;

    /*##-1- Configure the TIM peripheral #######################################*/
    /* -----------------------------------------------------------------------
     In this example TIM3 input clock (TIM3CLK) is set to 2 * APB1 clock (PCLK1),
     since APB1 prescaler is different from 1.
     TIM3CLK = 2 * PCLK1
     PCLK1 = HCLK / 4
     => TIM3CLK = HCLK / 2 = SystemCoreClock /2
     To get TIM3 counter clock at 10 KHz, the Prescaler is computed as following:
     Prescaler = (TIM3CLK / TIM3 counter clock) - 1
     Prescaler = ((SystemCoreClock /2) /10 KHz) - 1
     ----------------------------------------------------------------------- */

    /* Compute the prescaler value to have TIM7 counter clock equal to 1 KHz */
    uwPrescalerValue = (uint32_t) ((SystemCoreClock / 2) / (HI_TIME_FREQ * 2));

    htim7.Instance = TIM7;
    htim7.Init.Prescaler = uwPrescalerValue;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.Period = 2 - 1;
    htim7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim7);

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig);

    HAL_TIM_Base_Start_IT(&htim7);

//    /*## Configure the TIM peripheral for ADC123 injected trigger ####################*/
//    /* -----------------------------------------------------------------------
//     Use TIM5 for start Injected conversion on ADC1 (gyro rate).
//     Use TIM5 for start Injected conversion on ADC2 (not use).
//     Use TIM5 for start Injected conversion on ADC3 (not use).
//     ----------------------------------------------------------------------- */
//
//    /* Compute the prescaler value to have TIM5 counter clock equal to 4 KHz */
//    uwPrescalerValue = (uint32_t) ((SystemCoreClock / 2) / (GYRO_TIME_FREQ * 100));
//
//    htim5.Instance = TIM5;
//    htim5.Init.Prescaler = uwPrescalerValue;
//    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
//    htim5.Init.Period = 100 - 1;
//    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//    //    htim5.Init.RepetitionCounter = 0x0;
//    HAL_TIM_Base_Init(&htim5);
//
//    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
//    HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig);
//
//    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE; //TIM_TRGO_UPDATE see adc.c => ADC1 injected section
//    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//    HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig);
//
//    //    HAL_TIM_Base_Start_IT(&htim5);
//    HAL_TIM_Base_Start(&htim5);
//
//    /*## Configure the TIM peripheral for ADC1 regular trigger ####################*/
//    /* -----------------------------------------------------------------------
//     Use TIM4 for start Regular conversion on ADC1 (vbat, gyro_temp, internal_temps, internal vbat).
//     ----------------------------------------------------------------------- */
//
//    /* Compute the prescaler value to have TIM4 counter clock equal to 1 Hz */
//    uwPrescalerValue = (uint32_t) ((SystemCoreClock / 2) / (MULTIMMETER_TIME_FREQ * 10000));
//
//    htim4.Instance = TIM4;
//    htim4.Init.Prescaler = uwPrescalerValue;
//    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
//    htim4.Init.Period = 10000 - 1;
//    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//    HAL_TIM_Base_Init(&htim4);
//
//    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
//    HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig);
//
//    HAL_TIM_OC_Init(&htim4);
//
//    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
//    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//    HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig);
//
//    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
//    sConfigOC.Pulse = 1;
//    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
//    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
//    HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4);
//
//    HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_4);
}

char timeOut(unsigned char second, int loop_nb)
{
    static int old_tick;
    if (loop_nb == 0)
    {
        old_tick = HAL_GetTick();
    }
    if (((HAL_GetTick() - old_tick) / 1000) < second)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void ledPowerBlink(unsigned int off_time, unsigned int on_time)
{
    Blink[0] = (off_time / 10);
    Blink[1] = (on_time / 10);
    Blink[2] = 1;
}

void ledPowerErrorBlink(unsigned int off_time, unsigned int on_off_time, unsigned char repeat)
{
    Blink[0] = (off_time / 10);
    Blink[1] = (on_off_time / 10);
    Blink[2] = repeat;
}

void ledBlink_IT(void)
{
    static int cnt_led = 0;
    static int cnt_repeat = 0;

    if (Blink[0] == 0 && Blink[1] == 0)
    {
        return;
    }

    cnt_led++;
    if (cnt_led <= (Blink[0]))
    {
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, SET);
    }
    else if (cnt_led <= (Blink[0] + (Blink[1]) * (1 + cnt_repeat)))
    {
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, RESET);
    }
    //repeat blinking
    else if (cnt_led <= (Blink[0] + Blink[1] + (Blink[1]) * (1 + cnt_repeat)))
    {
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, SET);
    }
    else
    {
        cnt_repeat += 2;
    }

    if (cnt_led >= (Blink[0] + Blink[1]) + ((Blink[2] - 1) * 2 * Blink[1]))
    {
        cnt_led = 0;
        cnt_repeat = 0;
    }
}
