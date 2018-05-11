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
extern TIM_HandleTypeDef htim3;

TIM_OC_InitTypeDef sConfig;

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

    /* Compute the prescaler value */
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

    /* Compute the prescaler value */
    uwPrescalerValue = (uint32_t) ((SystemCoreClock / 2) / (FLOORSENSORS_PWM_FREQ * 2));

    htim3.Instance = TIM3;

    htim3.Init.Prescaler         = uwPrescalerValue;
    htim3.Init.Period            = 100;
    htim3.Init.ClockDivision     = 0;
    htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim3.Init.RepetitionCounter = 0;
    if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
    {
      /* Initialization Error */
      Error_Handler();
    }

    /*##-2- Configure the PWM channels #########################################*/
    /* Common configuration for all channels */
    sConfig.OCMode       = TIM_OCMODE_PWM1;
    sConfig.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfig.OCFastMode   = TIM_OCFAST_DISABLE;
    sConfig.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sConfig.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    sConfig.OCIdleState  = TIM_OCIDLESTATE_RESET;

    /* Set the pulse value for channel 1 */
    sConfig.Pulse = 50;
    if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfig, TIM_CHANNEL_1) != HAL_OK)
    {
      /* Configuration Error */
      Error_Handler();
    }
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
