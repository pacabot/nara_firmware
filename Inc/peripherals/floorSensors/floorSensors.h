/**************************************************************************/
/*!
 @file     floorSensors.h
 @author   PLF Pacabot.com
 @date     09 May 2018
 */
/**************************************************************************/
#ifndef __FLOORSENSORS_H__
#define __FLOORSENSORS_H__

/* Module Identifier */
#include "config/module_id.h"
#include "stm32h7xx_hal.h"

/* Error codes */
#define FLOORSENSORS_DRIVER_E_SUCCESS  0
#define FLOORSENSORS_DRIVER_E_ERROR    MAKE_ERROR(FLOORSENSORS_DRIVER_MODULE_ID, 1)

enum floorSensorName
{
    FLOORSENSOR_EXT_L, FLOORSENSOR_L, FLOORSENSOR_R, FLOORSENSOR_EXT_R, FLOORSENSOR_BOTH_L, FLOORSENSOR_BOTH_R, FLOORSENSOR_ALL, FLOORSENSOR_NONE
};

void    floorSensorsInit(void);
void    floorSensorsStart(void);
void    floorSensorsStop(void);
double  getFloorSensorAdc(enum floorSensorName floorSensor);
void    floorSensors_IT(void);
void    floorSensors_ADC_IT(ADC_HandleTypeDef *hadc);
void    floorSensorsTest(void);

#endif
