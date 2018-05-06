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

enum telemeterName
{
    TELEMETER_FL, TELEMETER_DL, TELEMETER_SL, TELEMETER_DR, TELEMETER_FR, TELEMETER_SR
};

/**
 * @defgroup VL53L0x_Debugging VL053L0A1  debugging
 * @{
 */

#ifndef  VL53L0x_TRACE
/**
 * @brief enable error output via trace
 *
 * when undefined (default) no trace no error logging is done, it is safe
 * to at least count error just to see if any errors ever occur.
 *
 * Traces formating and output is end user defined via #trace_printf
 */
#   define VL53L0x_TRACE    0
#endif // VL53L0x_TRACE


/**
 * @def VL53L0x_ErrLog(...)
 * Macro used to report error log messages with printf format
 *
 * Our testing version use externally trace_printf,
 * We trace out function names and line numbers plus any text formated with some extra arguments
 */
#ifdef VL53L0x_TRACE
#   define VL53L0x_ErrLog( msg, ...) trace_printf("[Err] %s l %d \t" msg "\n", __func__, __LINE__, ##__VA_ARGS__)
#else
#   define VL53L0x_ErrLog(...) (void)0
#endif

/**
 * 53L0X Device selector
 *
 * @note Most functions are using a device selector as input. ASCII 'c', 'l' or 'r' are also accepted.
 */
enum VL53L0x_dev_e{
    VL53L0x_DEV_LEFT =  0,    //!< left satellite device P21 header : 'l'
    VL53L0x_DEV_CENTER  =  1, //!< center (built-in) vl053 device : 'c"
    VL53L0x_DEV_RIGHT=  2     //!< Right satellite device P22 header : 'r'
};

/* Exported functions for telemeter.c */
int telemeters_Test(void);

#endif //__TELEMETERS_H__
