/**************************************************************************/
/*!
 @file    mainApp.h
 @author  PLF (PACABOT)
 @date    10 May 2018
 */
/**************************************************************************/

#ifndef __MAINAPP_H
#define __MAINAPP_H

/* Module Identifier */
#include "config/module_id.h"

/* Error codes */
#define MAINAPP_ID_E_SUCCESS  0
#define MAINAPP_ID_E_ERROR    MAKE_ERROR(MAINAPP_ID, 1)

void mainApp(void);

#endif


