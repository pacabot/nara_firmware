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

/* Private functions ---------------------------------------------------------*/
static void mainPeripheralsInits(void)
{
	encodersInit();
	motorsInit();
	floorSensorsInit();
	mainControlInit();
	telemetersInit();
	telemetersStart();
}
/* Public functions ----------------------------------------------------------*/
/**
 * @brief  mainApp
 * @param  None
 * @retval None
 */
void mainApp(void)
{
	double dist = 0.00;

	mainPeripheralsInits();
//	telemeters_Test();

	while(1)
	{
		dist = getTelemeterDist(TELEMETER_SL);
		printf("SL dist = %0.1f  ", dist);
		dist = getTelemeterDist(TELEMETER_DL);
		printf("DL dist = %0.1f  ", dist);
		dist = getTelemeterDist(TELEMETER_FL);
		printf("FL dist = %0.1f  ", dist);
		dist = getTelemeterDist(TELEMETER_SR);
		printf("SR dist = %0.1f  ", dist);
		dist = getTelemeterDist(TELEMETER_DR);
		printf("DR dist = %0.1f  ", dist);
		dist = getTelemeterDist(TELEMETER_FR);
		printf("FR dist = %0.1f  \n", dist);
		printf("-------------------------------------------------------------------------------------------------------\n");
		HAL_Delay(1000);
	}

	basicMove(0, 0, 0, 0);
	basicMove(0, 1000, 1000, 1000);
	while (hasMoveEnded() != TRUE);
	basicMove(0, 1000, 1000, 0);
	while(1);

}
