/**************************************************************************/
/*!
 @file    bldc.c
 @author  PLF (PACABOT)
 @date    29 April 2018
 */
/**************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "config/basetypes.h"

/* Application declarations */

/* Middleware declarations */

/* Peripheral declarations */

/* Declarations for this module */
#include "peripherals/motors/bldc.h"

/* Private typedef -----------------------------------------------------------*/
struct motor
{
    /* Timer handler Structure declaration */
    TIM_HandleTypeDef* htim_PWM;
    TIM_HandleTypeDef* htim_HALL;
    uint32_t TIM_TS_ITR;
    /* Timer Output Compare Configuration Structure declaration */
    TIM_OC_InitTypeDef sPWMConfig_1;
    TIM_OC_InitTypeDef sPWMConfig_2;
    TIM_OC_InitTypeDef sPWMConfig_3;
    /* HALL sensors gpio */
    GPIO_TypeDef* HALL1_GPIO_Port;
    GPIO_TypeDef* HALL2_GPIO_Port;
    GPIO_TypeDef* HALL3_GPIO_Port;
    uint16_t HALL1_Pin;
    uint16_t HALL2_Pin;
    uint16_t HALL3_Pin;
    uint8_t direction;
};

/* Timer Output Compare Configuration Structure declaration */

/* Timer Break Configuration Structure declaration */
TIM_BreakDeadTimeConfigTypeDef sBreakConfig;
/* Private define ------------------------------------------------------------*/
#define MOT_L_TIM_ITR   TIM_TS_ITR2
#define MOT_R_TIM_ITR   TIM_TS_ITR1

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static const uint8_t BLDC_BRIDGE_STATE_BACKWARD[8][6] =   // Motor step backward
{
        { FALSE, FALSE,      FALSE, FALSE,      FALSE, FALSE },  //000
        { FALSE, FALSE,      TRUE , FALSE,      FALSE, TRUE  },
        { TRUE , FALSE,      FALSE, TRUE ,      FALSE, FALSE },
        { TRUE , FALSE,      FALSE, FALSE,      FALSE, TRUE  },
        { FALSE, TRUE ,      FALSE, FALSE,      TRUE , FALSE },
        { FALSE, TRUE ,      TRUE , FALSE,      FALSE, FALSE },
        { FALSE, FALSE,      FALSE, TRUE ,      TRUE , FALSE },
        { FALSE, FALSE,      FALSE, FALSE,      FALSE, FALSE },  //111
};

static const uint8_t BLDC_BRIDGE_STATE_FORWARD[8][6] =   // Motor step forward
{
        { FALSE, FALSE,     FALSE, FALSE,       FALSE, FALSE },  //000
        { FALSE, FALSE,     FALSE, TRUE ,       TRUE , FALSE },
        { FALSE, TRUE ,     TRUE , FALSE,       FALSE, FALSE },
        { FALSE, TRUE ,     FALSE, FALSE,       TRUE , FALSE },
        { TRUE , FALSE,     FALSE, FALSE,       FALSE, TRUE  },
        { TRUE , FALSE,     FALSE, TRUE ,       FALSE, FALSE },
        { FALSE, FALSE,     TRUE , FALSE,       FALSE, TRUE  },
        { FALSE, FALSE,     FALSE, FALSE,       FALSE, FALSE },  //111
};

static struct motor left_motor = {0};
static struct motor right_motor = {0};

/* External variables --------------------------------------------------------*/
/* Timer handler declaration */
extern TIM_HandleTypeDef    htim1;  // Right Motor
extern TIM_HandleTypeDef    htim2;  // Right Halls sensors
extern TIM_HandleTypeDef    htim4;  // Left Halls sensors
extern TIM_HandleTypeDef    htim8;  // Left Motor

/* Private function prototypes -----------------------------------------------*/
static __INLINE void BLDCMotorPrepareCommutation(struct motor *mot);

/* Private functions ---------------------------------------------------------*/
/**
 * @brief  This is called from HALL timer interrupt handler
   remember:
     if hall a hall edge is detected
     first the motor commutation event is done
     next this routine is called which has to prepare the next motor step
     (which FET must be switched on or off)
   active freewhelling is used to minimize power loss

 * @param  mot : motor structure pointer
 * @retval None
 */
static __INLINE void BLDCMotorPrepareCommutation(struct motor* mot)
{
    // next bridge step calculated by HallSensor inputs
    // if there was an hall event without changing the hall position,
    // do nothing.
    //
    // In principle, on every hall event you can go to the next motor
    // step but i had sometimes problems that the motor was running
    // on an harmonic wave (??) when the motor was without load

    uint8_t BH1, BL1, BH2, BL2, BH3, BL3;

    uint16_t newcomm_pos = ((HAL_GPIO_ReadPin(mot->HALL1_GPIO_Port, mot->HALL1_Pin)) |
            (HAL_GPIO_ReadPin(mot->HALL2_GPIO_Port, mot->HALL2_Pin) << 1) |
            (HAL_GPIO_ReadPin(mot->HALL3_GPIO_Port, mot->HALL3_Pin) << 2));

    uint16_t comm_pos = 0;

    if (newcomm_pos == comm_pos) return;

    comm_pos = newcomm_pos;

    if(mot->direction == MOT_CW)
    {
        BH1 = BLDC_BRIDGE_STATE_FORWARD[comm_pos][0];
        BL1 = BLDC_BRIDGE_STATE_FORWARD[comm_pos][1];

        BH2 = BLDC_BRIDGE_STATE_FORWARD[comm_pos][2];
        BL2 = BLDC_BRIDGE_STATE_FORWARD[comm_pos][3];

        BH3 = BLDC_BRIDGE_STATE_FORWARD[comm_pos][4];
        BL3 = BLDC_BRIDGE_STATE_FORWARD[comm_pos][5];
    }
    else if(mot->direction == MOT_CCW)
    {
        BH1 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][0];
        BL1 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][1];

        BH2 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][2];
        BL2 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][3];

        BH3 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][4];
        BL3 = BLDC_BRIDGE_STATE_BACKWARD[comm_pos][5];
    }
    else
    	return;

    // **** this is with active freewheeling ****

    // Bridge FETs for Motor Phase U
    if (BH1)
    {
        // PWM at low side FET of bridge U
        // active freewheeling at high side FET of bridge U
        // if low side FET is in PWM off mode then the hide side FET
        // is ON for active freewheeling. This mode needs correct definition
        // of dead time otherwise we have shoot-through problems

        mot->sPWMConfig_1.OCMode = TIM_OCMODE_PWM1;
        HAL_TIM_PWM_ConfigChannel(mot->htim_PWM, &mot->sPWMConfig_1, TIM_CHANNEL_1);

        HAL_TIM_PWM_Start(mot->htim_PWM, TIM_CHANNEL_1);
        HAL_TIMEx_OCN_Start(mot->htim_PWM, TIM_CHANNEL_1);

    } else
    {
        // Low side FET: OFF
        HAL_TIM_OC_Stop(mot->htim_PWM, TIM_CHANNEL_1);

        if (BL1){
            // High side FET: ON
            mot->sPWMConfig_1.OCMode = TIM_OCMODE_FORCED_ACTIVE;
            HAL_TIM_PWM_ConfigChannel(mot->htim_PWM, &mot->sPWMConfig_1, TIM_CHANNEL_1);
            HAL_TIMEx_OCN_Start(mot->htim_PWM, TIM_CHANNEL_1);

        } else {

            // High side FET: OFF
            HAL_TIMEx_OCN_Stop(mot->htim_PWM, TIM_CHANNEL_1);
        }
    }

    // Bridge FETs for Motor Phase V
    if (BH2)
    {
        mot->sPWMConfig_2.OCMode = TIM_OCMODE_PWM1;
        HAL_TIM_PWM_ConfigChannel(mot->htim_PWM, &mot->sPWMConfig_2, TIM_CHANNEL_2);

        HAL_TIM_PWM_Start(mot->htim_PWM, TIM_CHANNEL_2);
        HAL_TIMEx_OCN_Start(mot->htim_PWM, TIM_CHANNEL_2);
    } else
    {
        HAL_TIM_OC_Stop(mot->htim_PWM, TIM_CHANNEL_2);

        if (BL2)
        {
            mot->sPWMConfig_2.OCMode = TIM_OCMODE_FORCED_ACTIVE;
            HAL_TIM_PWM_ConfigChannel(mot->htim_PWM, &mot->sPWMConfig_2, TIM_CHANNEL_2);

            HAL_TIMEx_OCN_Start(mot->htim_PWM, TIM_CHANNEL_2);
        } else
        {
            HAL_TIMEx_OCN_Stop(mot->htim_PWM, TIM_CHANNEL_2);
        }
    }

    // Bridge FETs for Motor Phase W

    if (BH3) {
        mot->sPWMConfig_3.OCMode = TIM_OCMODE_PWM1;
        HAL_TIM_PWM_ConfigChannel(mot->htim_PWM, &mot->sPWMConfig_3, TIM_CHANNEL_3);

        HAL_TIM_PWM_Start(mot->htim_PWM, TIM_CHANNEL_3);
        HAL_TIMEx_OCN_Start(mot->htim_PWM, TIM_CHANNEL_3);
    } else
    {
        HAL_TIM_OC_Stop(mot->htim_PWM, TIM_CHANNEL_3);

        if (BL3)
        {
            mot->sPWMConfig_3.OCMode = TIM_OCMODE_FORCED_ACTIVE;
            HAL_TIM_PWM_ConfigChannel(mot->htim_PWM, &mot->sPWMConfig_3, TIM_CHANNEL_3);

            HAL_TIMEx_OCN_Start(mot->htim_PWM, TIM_CHANNEL_3);
        } else
        {
            HAL_TIMEx_OCN_Stop(mot->htim_PWM, TIM_CHANNEL_3);
        }
    }
}

/* Public functions ----------------------------------------------------------*/
/**
 * @brief  BLDC Init
 * @param  None
 * @retval None
 */
void motorsInit(void)
{
    TIM_OC_InitTypeDef sPWMConfig1, sPWMConfig2, sPWMConfig3;

    left_motor.htim_PWM     = &htim8;
    left_motor.htim_HALL    = &htim4;

    right_motor.htim_PWM    = &htim1;
    right_motor.htim_HALL   = &htim2;

    left_motor.HALL1_GPIO_Port  = MOT_L_HALL1_GPIO_Port;
    left_motor.HALL2_GPIO_Port  = MOT_L_HALL2_GPIO_Port;
    left_motor.HALL3_GPIO_Port  = MOT_L_HALL3_GPIO_Port;
    left_motor.HALL1_Pin        = MOT_L_HALL1_Pin;
    left_motor.HALL2_Pin        = MOT_L_HALL2_Pin;
    left_motor.HALL3_Pin        = MOT_L_HALL3_Pin;
    left_motor.TIM_TS_ITR       = MOT_L_TIM_ITR;
    left_motor.direction       = 0;

    right_motor.HALL1_GPIO_Port = MOT_R_HALL1_GPIO_Port;
    right_motor.HALL2_GPIO_Port = MOT_R_HALL2_GPIO_Port;
    right_motor.HALL3_GPIO_Port = MOT_R_HALL3_GPIO_Port;
    right_motor.HALL1_Pin       = MOT_R_HALL1_Pin;
    right_motor.HALL2_Pin       = MOT_R_HALL2_Pin;
    right_motor.HALL3_Pin       = MOT_R_HALL3_Pin;
    right_motor.TIM_TS_ITR      = MOT_R_TIM_ITR;
    right_motor.direction       = 0;


    /*##-1- Configure the output channels ######################################*/
    /* Common configuration for all channels */
    sPWMConfig1.OCMode       = TIM_OCMODE_TIMING;
    sPWMConfig1.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sPWMConfig1.OCNPolarity  = TIM_OCNPOLARITY_HIGH;
    sPWMConfig1.OCIdleState  = TIM_OCIDLESTATE_SET;
    sPWMConfig1.OCNIdleState = TIM_OCNIDLESTATE_SET;
    sPWMConfig1.OCFastMode   = TIM_OCFAST_DISABLE;

    /* Set the pulse value for channel 1 */
    sPWMConfig1.Pulse = 4095;

    /* Set the pulse value for channel 2 */
    sPWMConfig2 = sPWMConfig1;
    sPWMConfig2.Pulse = sPWMConfig1.Pulse; //2047

    /* Set the pulse value for channel 3 */
    sPWMConfig3 = sPWMConfig1;
    sPWMConfig3.Pulse = sPWMConfig1.Pulse; //511

    right_motor.sPWMConfig_1    = sPWMConfig1;
    right_motor.sPWMConfig_2    = sPWMConfig2;
    right_motor.sPWMConfig_3    = sPWMConfig3;

    left_motor.sPWMConfig_1     = sPWMConfig1;
    left_motor.sPWMConfig_2     = sPWMConfig2;
    left_motor.sPWMConfig_3     = sPWMConfig3;

    /*##-2- Configure the commutation event: software event ####################*/
    HAL_TIMEx_ConfigCommutationEvent_IT(left_motor.htim_PWM, left_motor.TIM_TS_ITR, TIM_COMMUTATION_TRGI);
    HAL_TIMEx_ConfigCommutationEvent_IT(right_motor.htim_PWM, right_motor.TIM_TS_ITR, TIM_COMMUTATION_TRGI);

    HAL_TIMEx_HallSensor_Start(left_motor.htim_HALL);
    HAL_TIMEx_HallSensor_Start(right_motor.htim_HALL);
}

/**
 * @brief  Commutation event callback in non blocking mode
 * @param  motorType : MOT_L or MOT_R, Direction : CW or CCW
 * @retval None
 */
void motorSetDir(enum motorType type, enum motorDirection dir)
{
    if (type == MOT_L)
    {
    if (dir == MOT_CW)
        left_motor.direction = MOT_CW;
    else if (dir == MOT_CCW)
        left_motor.direction = MOT_CCW;
    }
    else if (type == MOT_R)
    {
        if (dir == MOT_CW)
            right_motor.direction = MOT_CW;
        else if (dir == MOT_CCW)
            right_motor.direction = MOT_CCW;
    }
}

/**
 * @brief  Commutation event callback in non blocking mode
 * @param  motorType : MOT_L or MOT_R, Duty cycle : 0 to 4096
 * @retval None
 */
void motorSet(enum motorType type, uint16_t duty_cycle)
{
    if (duty_cycle > 4096)
        return;

    if (type == MOT_L)
    {
        left_motor.sPWMConfig_1.Pulse = duty_cycle;   //Set the pulse value for channel 1
        left_motor.sPWMConfig_2.Pulse = duty_cycle;   //Set the pulse value for channel 2
        left_motor.sPWMConfig_3.Pulse = duty_cycle;   //Set the pulse value for channel 3
    }
    else if (type == MOT_R)
    {
        right_motor.sPWMConfig_1.Pulse = duty_cycle;
        right_motor.sPWMConfig_2.Pulse = duty_cycle;
        right_motor.sPWMConfig_3.Pulse = duty_cycle;
    }
}

//void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
//{
//    if (htim == &htim2)
//    {
//        BLDCMotorPrepareCommutation();
//    }
//}

/**
 * @brief  Commutation event callback in non blocking mode
 * @param  htim : Timer handle
 * @retval None
 */
void HAL_TIMEx_CommutationCallback(TIM_HandleTypeDef *htim)
{
    if (htim == left_motor.htim_PWM)
    {
        BLDCMotorPrepareCommutation(&left_motor);
    }
    if (htim == right_motor.htim_PWM)
    {
        BLDCMotorPrepareCommutation(&right_motor);
    }
}

/**
 * @brief  Brake all motors
 * @param  None
 * @retval None
 */
void motorsBrake(void)
{

}
void motorsDriverSleep(int isOn)
{

}

void motors_test(void)
{

}
