/**************************************************************************/
/*!
 @file    tone.c
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
/* Peripheral declarations */

/* Declarations for this module */
#include "peripherals/tone/tone.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static int tone_duration = 0;

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim5;

/* Private function prototypes -----------------------------------------------*/
static void imperialMarch(void);
//static void happyBirthday(void);

/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/**
 * @brief  Tone Init
 * @param  None
 * @retval None
 */
void toneInit(void)
{
    TIM_OC_InitTypeDef sConfigOC;

    htim5.Instance = TIM5;
    htim5.Init.Prescaler = 1800;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 1000;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim5);

    HAL_TIM_PWM_Init(&htim5);

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 50;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_4);
}

void tonesplayer(int *note, int *duration, int size, int tempo)
{
    int note_freq;
    int uwPrescalerValue;
    for (int ii = 0; ii < size; ii++)
    {
        note_freq = note[ii];

        uwPrescalerValue = (uint32_t) ((SystemCoreClock / 2) / (note_freq * 1000)) - 1;
        htim5.Instance = TIM5;
        htim5.Init.Prescaler = uwPrescalerValue;
        htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
        htim5.Init.Period = 1000;
        htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        HAL_TIM_Base_Init(&htim5);
        HAL_TIM_PWM_Init(&htim5);
        HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);

        HAL_Delay(60000 * duration[ii] / (tempo * 4) - 60);
        HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_4);
        HAL_Delay(60);
    }
}

// Arduino tone() compatible
void tone(int note, int duration)
{
    int uwPrescalerValue;
    tone_duration = 0;

    uwPrescalerValue = (uint32_t) ((SystemCoreClock / 2) / (note * 1000)) - 1;
    htim5.Instance = TIM5;
    htim5.Init.Prescaler = uwPrescalerValue;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 1000 - 1;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim5);
    HAL_TIM_PWM_Init(&htim5);
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);

    HAL_Delay(duration);
    HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_4);
}

void toneItMode(int note, int duration_ms)
{
    static int uwPrescalerValue;
    tone_duration = (duration_ms / (1000 / LOW_TIME_FREQ)) + 1;

    uwPrescalerValue = (uint32_t) ((SystemCoreClock / 2) / (note * 1000)) - 1;
    htim5.Instance = TIM5;
    htim5.Init.Prescaler = uwPrescalerValue;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 1000 - 1;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim5);
    HAL_TIM_PWM_Init(&htim5);
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);
}

void tone_IT(void)
{
    if (tone_duration > 1)
        tone_duration--;
    if (tone_duration == 1)
        HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_4);
}

void toneStart(int note)
{
    int uwPrescalerValue;
    tone_duration = 0;

    uwPrescalerValue = (uint32_t) ((SystemCoreClock / 2) / (note * 1000)) - 1;
    htim5.Instance = TIM5;
    htim5.Init.Prescaler = uwPrescalerValue;
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 1000 - 1;
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim5);
    HAL_TIM_PWM_Init(&htim5);
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_4);
}

void toneStop(void)
{
    HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_4);
}

void toneSetVolulme(int volume)
{
    TIM_OC_InitTypeDef sConfigOC;

    if (volume >= 100)
        volume = 100;

    if (volume <= 0)
        volume = 0;

    volume = volume * 20 / 100; //change scale

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = volume;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_4);
}

void toneTest(void)
{
    toneSetVolulme(100);

    //	happyBirthday();
    imperialMarch();
}

//void happyBirthday()
//{
//    tone(D3, 250 * 2);
//    tone(D3, 250 * 2);
//    tone(E3, 500 * 2);
//    tone(D3, 500 * 2);
//    tone(G3, 500 * 2);
//    tone(F3H, 1000 * 2);
//
//    tone(D3, 250 * 2);
//    tone(D3, 250 * 2);
//    tone(E3, 500 * 2);
//    tone(D3, 500 * 2);
//    tone(A3, 500 * 2);
//    tone(G3, 1000 * 2);
//
//    tone(D3, 250 * 2);
//    tone(D3, 250 * 2);
//    tone(D3, 500 * 2);
//    tone(B3, 500 * 2);
//    tone(G3, 500 * 2);
//    tone(F3H, 500 * 2);
//    tone(E3, 500 * 2);
//
//    tone(C3, 250 * 2);
//    tone(C3, 250 * 2);
//    tone(B3, 500 * 2);
//    tone(G3, 500 * 2);
//    tone(A3, 500 * 2);
//    tone(G3, 1000 * 2);
//
//    HAL_Delay(1000);
//
//    tone(C3, 200 * 2);
//    tone(B3, 200 * 2);
//    tone(A3, 1000 * 2);
//    while (1)
//    {
//        tone(A3 * 2, 1 * 2);
//        tone(A4 * 2, 1 * 2);
//    }
//}

void imperialMarch()
{

    //Play first section
    tone(a, 500);
    tone(a, 500);
    tone(a, 500);
    tone(f, 350);
    tone(cH, 150);
    tone(a, 500);
    tone(f, 350);
    tone(cH, 150);
    tone(a, 650);

    HAL_Delay(500);

    tone(eH, 500);
    tone(eH, 500);
    tone(eH, 500);
    tone(fH, 350);
    tone(cH, 150);
    tone(gS, 500);
    tone(f, 350);
    tone(cH, 150);
    tone(a, 650);

    HAL_Delay(500);

    //Play second section
    tone(aH, 500);
    tone(a, 300);
    tone(a, 150);
    tone(aH, 500);
    tone(gSH, 325);
    tone(gH, 175);
    tone(fSH, 125);
    tone(fH, 125);
    tone(fSH, 250);

    HAL_Delay(325);

    tone(aS, 250);
    tone(dSH, 500);
    tone(dH, 325);
    tone(cSH, 175);
    tone(cH, 125);
    tone(b, 125);
    tone(cH, 250);

    HAL_Delay(350);

    //Variant 1
    tone(f, 250);
    tone(gS, 500);
    tone(f, 350);
    tone(a, 125);
    tone(cH, 500);
    tone(a, 375);
    tone(cH, 125);
    tone(eH, 650);

    HAL_Delay(500);

    //Repeat second section
    tone(aH, 500);
    tone(a, 300);
    tone(a, 150);
    tone(aH, 500);
    tone(gSH, 325);
    tone(gH, 175);
    tone(fSH, 125);
    tone(fH, 125);
    tone(fSH, 250);

    HAL_Delay(325);

    tone(aS, 250);
    tone(dSH, 500);
    tone(dH, 325);
    tone(cSH, 175);
    tone(cH, 125);
    tone(b, 125);
    tone(cH, 250);

    HAL_Delay(350);

    //Variant 2
    tone(f, 250);
    tone(gS, 500);
    tone(f, 375);
    tone(cH, 125);
    tone(a, 500);
    tone(f, 375);
    tone(cH, 125);
    tone(a, 650);

    HAL_Delay(650);
}

