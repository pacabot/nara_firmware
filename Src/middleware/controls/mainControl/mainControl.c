/**************************************************************************/
/*!
 @file    mainControl.c
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
#include "middleware/controls/mainControl/speedControl.h"
#include "middleware/controls/mainControl/transfertFunction.h"

/* Peripheral declarations */
#include "peripherals/times_base/times_base.h"
#include "peripherals/bluetooth/bluetooth.h"

/* Middleware declarations */
#include "middleware/controls/pidController/pidController.h"

typedef struct
{
    char wall_follow_state;
    char line_follow_state;
    char position_state;
    char speed_state;
} control_params_struct;

static enum mainControlMoveType moveType;
static control_params_struct control_params;
static pid_loop_struct pid_loop;

int mainControlInit(void)
{
    pid_loop.start_state = FALSE;

    speedControlInit();
    positionControlInit();
    transfertFunctionInit();

    positionControlSetPositionType(ENCODERS);
    mainControlSetFollowType(NO_FOLLOW);

    mainControlSetMoveType(STRAIGHT);

    control_params.wall_follow_state = 0;
    control_params.line_follow_state = 0;

    return MAIN_CONTROL_E_SUCCESS;
}

int mainControlStopPidLoop(void)
{
    pid_loop.start_state = FALSE;
    return MAIN_CONTROL_E_SUCCESS;
}

int mainControlSartPidLoop(void)
{
    pid_loop.start_state = TRUE;
    return MAIN_CONTROL_E_SUCCESS;
}

int mainControl_IT(void)
{
    int rv;
    if (pid_loop.start_state == FALSE)
    {
        return MAIN_CONTROL_E_SUCCESS;
    }

    rv = positionControlLoop();
    if (rv != POSITION_CONTROL_E_SUCCESS)
        return rv;
    rv = speedControlLoop();
    if (rv != SPEED_CONTROL_E_SUCCESS)
        return rv;
    rv = transfertFunctionLoop();
    if (rv != TRANSFERT_FUNCTION_E_SUCCESS)
        return rv;
    return MAIN_CONTROL_E_SUCCESS;
}

int mainControlSetFollowType(enum mainControlFollowType followType)
{
    switch (followType)
    {
        case LINE_FOLLOW:
            control_params.wall_follow_state = FALSE;
            control_params.line_follow_state = TRUE;
            return MAIN_CONTROL_E_SUCCESS;
        case WALL_FOLLOW:
            control_params.line_follow_state = FALSE;
            control_params.wall_follow_state = TRUE;
            return MAIN_CONTROL_E_SUCCESS;
        case NO_FOLLOW:
            control_params.line_follow_state = FALSE;
            control_params.wall_follow_state = FALSE;
            return MAIN_CONTROL_E_SUCCESS;
    }

    return MAIN_CONTROL_E_ERROR;
}

enum mainControlFollowType mainControlGetFollowType()
{
    if (control_params.wall_follow_state == TRUE)
        return WALL_FOLLOW;
    else if (control_params.line_follow_state == TRUE)
        return LINE_FOLLOW;
    else
        return NO_FOLLOW;
}

enum mainControlMoveType mainControlGetMoveType()
{
    return moveType;
}

int mainControlSetMoveType(enum mainControlMoveType move_type)
{
    moveType = move_type;
    return MAIN_CONTROL_E_ERROR;
}

char hasMoveEnded(void)
{
    if ((positionControlHasMoveEnded() == TRUE && speedControlHasMoveEnded() == TRUE) ||
            pid_loop.start_state == FALSE)
    {
        pid_loop.start_state = FALSE;
        //        motorsDriverSleep(ON);
        return TRUE;
    }
    return FALSE;
}
