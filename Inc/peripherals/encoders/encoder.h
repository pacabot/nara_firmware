/**************************************************************************/
/*!
 @file    encoder.h
 @author  PLF (PACABOT)
 @date    1 May 2018
 */
/**************************************************************************/
#ifndef __ENCODERS_H__
#define __ENCODERS_H__

/* Module Identifier */
#include "config/module_id.h"

/* Error codes */
#define ENCODER_DRIVER_E_SUCCESS  0
#define ENCODER_DRIVER_E_ERROR    MAKE_ERROR(ENCODER_DRIVER_MODULE_ID, 1)

enum encoderName
{
    ENCODER_L, ENCODER_R
};

void encodersInit(void);
void encodersStop(void);
void encoderLeft_IT(void);
void encoderRight_IT(void);
int encodersReset(void);
double encoderGetDist(enum encoderName encoder_name);
double encoderGetAbsDist(enum encoderName encoder_name);
void encoderTest(void);

#endif //__ENCODER_H__
