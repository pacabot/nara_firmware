/**************************************************************************/
/*!
 @file     telemeter.h
 @author   PLF Pacabot.com
 @date     03 August 2014
 @version  0.10
 */
/**************************************************************************/
#ifndef __TELEMETERS_H__
#define __TELEMETERS_H__

/* Module Identifier */
#include "config/module_id.h"

/* Error codes */
#define TELEMETERS_DRIVER_E_SUCCESS  0
#define TELEMETERS_DRIVER_E_ERROR    MAKE_ERROR(TELEMETERS_DRIVER_MODULE_ID, 1)

#define TELEMETERS_CAL_E_SUCCESS  0
#define TELEMETERS_CAL_E_ERROR    MAKE_ERROR(TELEMETERSCAL_DRIVER_MODULE_ID, 1)

#define TELEMETER_PROFILE_ARRAY_LENGTH 		250
#define MEASURED_DISTANCE	                250
#define NUMBER_OF_MILLIMETER_BY_LOOP (MEASURED_DISTANCE/TELEMETER_PROFILE_ARRAY_LENGTH)

#if ((DISTANCE_MEASURED) % (TELEMETER_PROFILE_ARRAY_LENGTH)) != 0
#error you must put a multiple of NUMBER_OF_CELL in DISTANCE_MEASURED
#endif

enum telemeterName
{
    TELEMETER_FL, TELEMETER_DL, TELEMETER_SL, TELEMETER_DR, TELEMETER_FR, TELEMETER_SR
};

/* Exported functions for telemeter.c */
int telemeters_Test(void);

#endif //__TELEMETERS_H__
