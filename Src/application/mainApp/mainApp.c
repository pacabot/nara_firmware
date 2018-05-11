/**************************************************************************/
/*!
 @file    mainApp.c
 @author  PLF (PACABOT)
 @date    10 May 2018
 */
/**************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "config/basetypes.h"
#include "config/config.h"

/* Application declarations */

/* Middleware declarations */
#include "middleware/moves/basicMoves/basicMoves.h"
#include "middleware/controls/mainControl/mainControl.h"
/* Peripheral declarations */
#include "peripherals/telemeters/telemeters.h"
#include "peripherals/tone/tone.h"
#include "peripherals/motors/bldc.h"
#include "peripherals/times_base/times_base.h"
#include "peripherals/encoders/encoder.h"
#include "peripherals/floorSensors/floorSensors.h"

/* Declarations for this module */
#include "application/mainApp/mainApp.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void mainPeripheralsInits(void);
static void waitStart(void);

/* Private functions ---------------------------------------------------------*/
static void mainPeripheralsInits(void)
{
	telemetersInit();
	encodersInit();
	motorsInit();
	floorSensorsInit();
	mainControlInit();
}

static void waitStart(void)
{
	double dist = 2000;
	while (dist > 50.00)
	{
		dist = getTelemeterDist(TELEMETER_FR);
		if (dist < 20)
			dist = 2000.00;
		HAL_Delay(2);
	}

	tone(D3, 100);

	while (dist < 100)
	{
		dist = getTelemeterDist(TELEMETER_FR);
		HAL_Delay(2);
	}

	toneItMode(D5, 500);
	//	tone(D5, 100);
	HAL_Delay(1000);
}

static char whiteBandIsPresent(void)
{
	if (getFloorSensorAdc(FLOORSENSOR_EXT_L) < FLOOR_SENSOR_ADC_THESHOLD)
	{
		if (getFloorSensorAdc(FLOORSENSOR_L) < FLOOR_SENSOR_ADC_THESHOLD)
		{
			if (getFloorSensorAdc(FLOORSENSOR_EXT_R) < FLOOR_SENSOR_ADC_THESHOLD)
				return FLOORSENSOR_ALL;
			return FLOORSENSOR_BOTH_L;
		}
		return FLOORSENSOR_EXT_L;
	}
	if (getFloorSensorAdc(FLOORSENSOR_EXT_R) < FLOOR_SENSOR_ADC_THESHOLD)
	{
		if (getFloorSensorAdc(FLOORSENSOR_R) < FLOOR_SENSOR_ADC_THESHOLD)
			return FLOORSENSOR_BOTH_R;
		return FLOORSENSOR_EXT_R;
	}
	return FLOORSENSOR_NONE;
}

/* Public functions ----------------------------------------------------------*/
/**
 * @brief  mainApp
 * @param  None
 * @retval None
 */
void mainApp(void)
{
	mainPeripheralsInits();
//	floorSensorsTest();
//	telemeters_Test();
	waitStart();

	while(1)
	{
		printf ("Withe line is present = %d\n", (int)whiteBandIsPresent());
		HAL_Delay(100);
	}


	basicMove(0, 0, 0, 0);
	basicMove(0, 10, 100, 100);
	while (hasMoveEnded() != TRUE);
	basicMove(0, 0, 0, 0);
	//	basicMove(0, 1000, 1000, 0);
	while(1);


	//	while(1)
	//	{
	//		printf ("Withe line is present = %d\n", (int)whiteBandIsPresent());
	//		HAL_Delay(100);
	//	}
	//	telemeters_Test();
	//	floorSensorsTest();

}
