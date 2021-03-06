/**************************************************************************/
/*!
 @file    positionControl.c
 @author  PLF (PACABOT)
 @date
 @version  0.0
 */
/**************************************************************************/
/* STM32 hal library declarations */
#include "stm32h7xx_hal.h"

/* General declarations */
#include <arm_math.h>
#include <config/basetypes.h>
#include <config/config.h>
#include <math.h>
#include <string.h>

/* Peripheral declarations */
#include <peripherals/times_base/times_base.h>

/* Middleware declarations */
#include <middleware/controls/mainControl/mainControl.h>
#include <middleware/controls/pidController/pidController.h>

/* Declarations for this module */
#include <middleware/controls/mainControl/positionControl.h>
#include <peripherals/encoders/encoder.h>

#define MAX_POSITION_ERROR     90.00 //Degrees
#define POSITION_ERROR_LIMITER 30.00 //Degrees

typedef struct
{
    double angle_consign;	    //total angle
    double anglar_acc_per_loop;
    double angle_per_loop;
    double max_speed;
    int nb_loop;
    int sign;
} position_params_struct;

typedef struct
{

    double current_angular_speed;
    int nb_loop;
    double position_command;
    double position_error;
    double position_consign;
    double current_angle;
    double current_angle_consign;   //differential distance (mm) since the control start
    double old_angle;				//effective distance at the previous call
    char end_control;
    enum enablePositionCtrl enablePositionCtrl;
    enum positionType positionType;
    pid_control_struct position_pid;
} position_control_struct;

/* App definitions */

/* Macros */

/* Static functions */

/* extern variables */

/* global variables */
static position_control_struct position_control;
static position_params_struct position_params;
static arm_pid_instance_f32 gyro_pid_instance;

int positionControlInit(void)
{
    memset(&position_control, 0, sizeof(position_control_struct));
    memset(&position_params, 0, sizeof(position_params_struct));
    positionProfileCompute(0, 0, 0);

    gyro_pid_instance.Kp = 60;//150;
    gyro_pid_instance.Ki = 0;//0.01;
    gyro_pid_instance.Kd = 500;//0.01 * CONTROL_TIME_FREQ;

    //    gyro_pid_instance.Kp = zhonxCalib_data->pid_gyro.Kp;
    //    gyro_pid_instance.Ki = zhonxCalib_data->pid_gyro.Ki / CONTROL_TIME_FREQ;
    //    gyro_pid_instance.Kd = zhonxCalib_data->pid_gyro.Kd * CONTROL_TIME_FREQ;

    position_control.position_pid.instance = &gyro_pid_instance;

    position_control.positionType = ENCODERS;
    position_params.sign = 1;

    pidControllerInit(position_control.position_pid.instance);

    return POSITION_CONTROL_E_SUCCESS;
}

char positionControlHasMoveEnded(void)
{
    return position_control.end_control;
}

double positionControlGetCurrentAngle(void)
{
    return position_control.current_angle;
}

double positionControlGetPositionCommand(void)
{
    return position_control.position_command;
}

char positionControlSetPositionType(enum positionType position_type)
{
    position_control.positionType = position_type;
    return POSITION_CONTROL_E_SUCCESS;
}

char positionControlEnablePositionCtrl(enum enablePositionCtrl enable_position_ctrl)
{
    position_control.enablePositionCtrl = enable_position_ctrl;
    return POSITION_CONTROL_E_SUCCESS;
}

double positionControlSetSign(double sign)
{
    return position_params.sign = sign;
}

int positionControlLoop(void)
{
//    if (position_control.positionType == GYRO)                        //todo implement gyroscope
//    {
//        if (position_params.sign > 0)
//            position_control.current_angle = gyroGetAngle();
//        else
//            position_control.current_angle = -1.00 * gyroGetAngle();
//    }
//    else //use encoders
//    {
        ledPowerErrorBlink(300, 300, 2);
        if (position_params.sign > 0)
            position_control.current_angle = 180.00 * (encoderGetDist(ENCODER_L) - encoderGetDist(ENCODER_R)) / (PI * WHEELS_DISTANCE);
        else
            position_control.current_angle = 180.00 * (encoderGetDist(ENCODER_R) - encoderGetDist(ENCODER_L)) / (PI * WHEELS_DISTANCE);
//    }

    if (mainControlGetMoveType() == ROTATE_IN_PLACE)
    {
        position_control.nb_loop++;
        //        if (position_control.nb_loop < (position_params.nb_loop / 2))
        if (position_control.current_angle_consign < (position_params.angle_consign / 2.00))
        {
            position_control.current_angular_speed += position_params.anglar_acc_per_loop;
            position_control.current_angle_consign += position_control.current_angular_speed / CONTROL_TIME_FREQ;
        }
        else if (position_control.current_angle_consign < position_params.angle_consign)
        {
            position_control.current_angular_speed -= position_params.anglar_acc_per_loop;
            if (position_control.current_angular_speed > 0)
            {
                position_control.current_angle_consign += position_control.current_angular_speed / CONTROL_TIME_FREQ;
            }
            else
            {
                position_control.current_angle_consign = position_params.angle_consign;
            }
        }
        else
        {
            position_control.end_control = TRUE;
        }
    }
    else
    {
        if (position_control.nb_loop < position_params.nb_loop)
        {
            position_control.nb_loop++;
            position_control.current_angle_consign += position_params.angle_per_loop;
        }
        else
        {
            position_control.end_control = TRUE;
        }
    }
//    else if(mainControlGetMoveType() == CLOTHOID)
//    {
//        position_control.nb_loop++;
//
//        if (position_control.nb_loop < 1000)
//        {
//            position_control.current_angular_speed += 1.00/100.00 * 6.00;// position_params.anglar_acc_per_loop;
//            position_control.current_angle_consign += position_control.current_angular_speed / CONTROL_TIME_FREQ;
//        }
//        else if (position_control.nb_loop < 2000)
//        {
////            position_control.current_angular_speed += 1.00/100.00;// position_params.anglar_acc_per_loop;
//            position_control.current_angle_consign += position_control.current_angular_speed / CONTROL_TIME_FREQ;
//        }
//        else if (position_control.nb_loop < 3000)
//        {
//            position_control.current_angular_speed -= 1.00/100.00 * 6.00;// position_params.anglar_acc_per_loop;
//            position_control.current_angle_consign += position_control.current_angular_speed / CONTROL_TIME_FREQ;
//        }
//        else
//            position_control.end_control = TRUE;
//    }

    position_control.position_error = position_control.current_angle_consign - position_control.current_angle;//for distance control
    if (fabs(position_control.position_error) > MAX_POSITION_ERROR)
    {
        printf("Position control ERROR, error overflow = %d\n", (int)position_control.position_error * (int)position_params.sign);
        ledPowerErrorBlink(1000, 150, 5);
        return POSITION_CONTROL_E_ERROR;
    }

    if (fabs(position_control.position_error) > POSITION_ERROR_LIMITER)
    {
        if (position_control.position_error > 0)
            position_control.position_error = POSITION_ERROR_LIMITER;
        else
            position_control.position_error = -POSITION_ERROR_LIMITER;
    }

    position_control.position_command = (pidController(position_control.position_pid.instance,
                                                       position_control.position_error)) * (double) position_params.sign;

    position_control.old_angle = position_control.current_angle;

//	static int i = 0;
//	i++;
//	if (!(i % 500))
//	{
//		printf("Position cmd = %d\n", (int)position_control.position_command);
//	}

    return POSITION_CONTROL_E_SUCCESS;
}

/**************************************************************************/
/*!
 ***BASICS FORMULAS***

       ___   _________
      / 2 x / Acc x d
 t = v_____v__________	//without initial speed
            Acc

             __________________
     - Vi + / Vi²+ 2 x Acc x d
 t = ______v___________________	//with initial speed
                 Acc

         V²
 d =   -----
       2.Acc

      1
 d = --- Acc x t²
      2

               1
 d = Vi x t + --- Acc x t²
               2

 Vf = Vi + Acc x t 	//instantaneous speed

     Vi + Vf
 V = -------			//average speed
        2

        2.d
 Acc = -----
         t²

 d = V x t

 V = Vi + (Acc x t)
                                   ________________________
      / 1 \             / 1 \	  /
 v =  |---| x t x Acc + |---| x  V t² x Acc² - 4 x Acc x d	//v = f(t,d,Acc)
      \ 2 /			    \ 2 /

        -2(t.Vi-d)
 Acc = ------------
            t²

         2(t.Vi-d)
 Dcc = ------------
             t²
 */
/**************************************************************************/
double positionProfileCompute(double angle, double loop_time, double max_turn_speed)
{
    position_control.current_angular_speed = 0;
    position_control.nb_loop = 0;
    position_control.position_command = 0;
    position_control.position_error = 0;
    position_control.current_angle = 0;
    position_control.current_angle_consign = 0;
    position_control.position_consign = 0;
    position_control.end_control = FALSE;
    pidControllerReset(position_control.position_pid.instance);

    if (lround(angle) == 0)
    {
        position_control.end_control = TRUE;
        position_params.nb_loop = 0;
        position_params.angle_consign = 0;
        return (0);
    }
    if (lround(loop_time) == 0)
    {
        loop_time = (2.00 * sqrt(angle / MAX_TURN_ACCEL)) * CONTROL_TIME_FREQ;
        //loop_time = 2.00 * (sqrt(2.00) * (sqrt((angle / 2.00) * MAX_TURN_ACCEL))) / (MAX_TURN_ACCEL) * CONTROL_TIME_FREQ;
        //       loop_time = (angle / max_turn_speed) * CONTROL_TIME_FREQ;
    }

    position_params.angle_consign = angle;
    position_params.nb_loop = (int)loop_time;
    position_params.anglar_acc_per_loop = ((2.00 * (angle / 2.00)) / (pow(position_params.nb_loop / 2.00, 2))) * CONTROL_TIME_FREQ;
    position_params.angle_per_loop = angle / (double)position_params.nb_loop;

    return (position_params.nb_loop);
}
