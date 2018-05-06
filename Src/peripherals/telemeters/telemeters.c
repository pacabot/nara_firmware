/**
 ******************************************************************************
 * File Name          : main.c
 * Description        : Main program body
 ******************************************************************************
 *
 * COPYRIGHT(c) 2015 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* USER CODE BEGIN Includes */
#include <string.h>
#include <limits.h>

#include "peripherals/telemeters/vl53l0x_api.h"
#include "peripherals/telemeters/telemeters.h"

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;

/** Time the initial 53L0 message is shown at power up */
#define ModeChangeDispTime  500

/** @}  */ /* config group */

#ifndef MIN
#   define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef ARRAY_SIZE
#   define ARRAY_SIZE(x) (sizeof((x))/sizeof((x)[0]))
#endif

/**
 * @defgroup ErrCode Errors code shown on display
 * @{
 */
#define ERR_DETECT             -1
#define ERR_DEMO_RANGE_ONE     1
#define ERR_DEMO_RANGE_MULTI   2


/** }@} */ /* defgroup ErrCode */


/* Private variables ---------------------------------------------------------*/

const char TxtRangeValue[]  = "rng";
const char TxtBarGraph[]    = "bar";

typedef enum
{
	LONG_RANGE      = 0, /*!< Long range mode */
	HIGH_SPEED      = 1, /*!< High speed mode */
	HIGH_ACCURACY   = 2, /*!< High accuracy mode */
} RangingConfig_e;

char *RangingConfigTxt[3] = {"LR", "HS", "HA"};

/**
 * Global ranging struct
 */
VL53L0X_RangingMeasurementData_t RangingMeasurementData;


/** leaky factor for filtered range
 *
 * r(n) = averaged_r(n-1)*leaky +r(n)(1-leaky)
 *
 * */
int LeakyFactorFix8 = (int)( 0.6 *256);
/** How many device detect set by @a DetectSensors()*/
int nDevPresent = 0;
/** bit is index in VL53L0XDevs that is not necessary the dev id of the BSP */
int nDevMask;


VL53L0X_Dev_t VL53L0XDevs[] = {
		{.Id=VL53L0x_DEV_LEFT_SIDE, 	.DevLetter='s', .I2cHandle=&hi2c3, .I2cDevAddr=0x52},
		{.Id=VL53L0x_DEV_LEFT_DIAG, 	.DevLetter='d', .I2cHandle=&hi2c3, .I2cDevAddr=0x52},
		{.Id=VL53L0x_DEV_LEFT_FRONT, 	.DevLetter='f', .I2cHandle=&hi2c3, .I2cDevAddr=0x52},
		{.Id=VL53L0x_DEV_RIGHT_SIDE, 	.DevLetter='s', .I2cHandle=&hi2c2, .I2cDevAddr=0x52},
		{.Id=VL53L0x_DEV_RIGHT_DIAG, 	.DevLetter='d', .I2cHandle=&hi2c2, .I2cDevAddr=0x52},
		{.Id=VL53L0x_DEV_RIGHT_FRONT, 	.DevLetter='f', .I2cHandle=&hi2c2, .I2cDevAddr=0x52},
};

/** range low (and high) in @a RangeToLetter()
 *
 * used for displaying  multiple sensor as bar graph
 */
int RangeLow = 100;

/** range medium in @a RangeToLetter()
 *
 * used for displaying  multiple sensor as bar graph
 */
int RangeMedium = 300;

/* Private function prototypes -----------------------------------------------*/
void ResetAndDetectSensor(int SetDisplay);

#define debug_printf    trace_printf
char WelcomeMsg[]="Hi I am Ranging VL53L0X \n";

/**
 * Handle Error
 *
 * Set err on display and loop forever
 * @param err Error case code
 */
void HandleError(int err)
{
	char msg[16];
	sprintf(msg,"Er%d", err);
	printf(msg);
	printf("\n");
	while(1){};
}

int telemetersResetId(int DevNo, int state)
{
	int status = 0;
	switch( DevNo )
	{
	case VL53L0x_DEV_LEFT_SIDE :
		HAL_GPIO_WritePin(TOF_EN_LS_GPIO_Port, TOF_EN_LS_Pin, state);
		break;
	case VL53L0x_DEV_LEFT_DIAG :
		HAL_GPIO_WritePin(TOF_EN_LD_GPIO_Port, TOF_EN_LD_Pin, state);
		break;
	case VL53L0x_DEV_LEFT_FRONT :
		HAL_GPIO_WritePin(TOF_EN_LF_GPIO_Port, TOF_EN_LF_Pin, state);
		break;
	case VL53L0x_DEV_RIGHT_SIDE :
		HAL_GPIO_WritePin(TOF_EN_RS_GPIO_Port, TOF_EN_RS_Pin, state);
		break;
	case VL53L0x_DEV_RIGHT_DIAG :
		HAL_GPIO_WritePin(TOF_EN_RD_GPIO_Port, TOF_EN_RD_Pin, state);
		break;
	case VL53L0x_DEV_RIGHT_FRONT :
		HAL_GPIO_WritePin(TOF_EN_RF_GPIO_Port, TOF_EN_RF_Pin, state);
		break;
	default:
		VL53L0x_ErrLog("Invalid DevNo %d",DevNo);
		status = -1;
		return status;
	}
	//error with valid id
	if( status )
	{
		VL53L0x_ErrLog("expander i/o error for DevNo %d state %d ",DevNo, state);
	}

	return status;
}

/**
 * Reset all sensor then do presence detection
 *
 * All present devices are data initiated and assigned to their final I2C address
 * @return
 */
int DetectSensors(int SetDisplay) {
	int i;
	uint16_t Id;
	int status;
	int FinalAddress;

	char PresentMsg[5]="    ";
	/* Reset all */
	nDevPresent = 0;
	for (i = 0; i < 6; i++)
		status = telemetersResetId(i, 0);

	/* detect all sensors (even on-board)*/
	for (i = 0; i < 6; i++)
	{
		VL53L0X_Dev_t *pDev;
		pDev = &VL53L0XDevs[i];
		pDev->I2cDevAddr = 0x52;
		pDev->Present = 0;
		status = telemetersResetId( pDev->Id, 1);
		HAL_Delay(2);
		FinalAddress=0x52+(i+1)*2;

		do
		{
			/* Set I2C standard mode (400 KHz) before doing the first register access */
			if (status == VL53L0X_ERROR_NONE)
				status = VL53L0X_WrByte(pDev, 0x88, 0x00);

			/* Try to read one register using default 0x52 address */
			status = VL53L0X_RdWord(pDev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
			if (status)
			{
				debug_printf("#%d Read id fail\n", i);
				break;
			}
			if (Id == 0xEEAA)
			{
				/* Sensor is found => Change its I2C address to final one */
				status = VL53L0X_SetDeviceAddress(pDev,FinalAddress);
				if (status != 0)
				{
					debug_printf("#i VL53L0X_SetDeviceAddress fail\n", i);
					break;
				}
				pDev->I2cDevAddr = FinalAddress;
				/* Check all is OK with the new I2C address and initialize the sensor */
				status = VL53L0X_RdWord(pDev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
				if (status != 0)
				{
					debug_printf("#i VL53L0X_RdWord fail\n", i);
					break;
				}

				status = VL53L0X_DataInit(pDev);
				if( status == 0 )
				{
					pDev->Present = 1;
				}
				else
				{
					debug_printf("VL53L0X_DataInit %d fail\n", i);
					break;
				}
				trace_printf("VL53L0X %d Present and initiated to final 0x%x\n", pDev->Id, pDev->I2cDevAddr);
				nDevPresent++;
				nDevMask |= 1 << i;
				pDev->Present = 1;
			}
			else
			{
				debug_printf("#%d unknown ID %x\n", i, Id);
				status = 1;
			}
		} while (0);
		/* if fail r can't use for any reason then put the  device back to reset */
		if (status)
		{
			telemetersResetId(i, 0);
		}
	}
	/* Display detected sensor(s) */
	if( SetDisplay )
	{
		for(i=0; i < 6; i++)
		{
			if( VL53L0XDevs[i].Present )
			{
				PresentMsg[i+1]=VL53L0XDevs[i].DevLetter;
			}
		}
		PresentMsg[0]=' ';
		//        VL53L0x_SetDisplayString(PresentMsg);
		printf(PresentMsg);
		printf("\n");
		HAL_Delay(1000);
	}

	return nDevPresent;
}

/**
 *  Setup all detected sensors for single shot mode and setup ranging configuration
 */
void SetupSingleShot(RangingConfig_e rangingConfig){
	int i;
	int status;
	uint8_t VhvSettings;
	uint8_t PhaseCal;
	uint32_t refSpadCount;
	uint8_t isApertureSpads;
	FixPoint1616_t signalLimit = (FixPoint1616_t)(0.25*65536);
	FixPoint1616_t sigmaLimit = (FixPoint1616_t)(18*65536);
	uint32_t timingBudget = 33000;
	uint8_t preRangeVcselPeriod = 14;
	uint8_t finalRangeVcselPeriod = 10;

	for( i = 0; i < 6; i++)
	{
		if( VL53L0XDevs[i].Present)
		{
			status=VL53L0X_StaticInit(&VL53L0XDevs[i]);
			if( status )
			{
				debug_printf("VL53L0X_StaticInit %d failed\n",i);
			}

			status = VL53L0X_PerformRefCalibration(&VL53L0XDevs[i], &VhvSettings, &PhaseCal);
			if( status )
			{
				debug_printf("VL53L0X_PerformRefCalibration failed\n");
			}

			status = VL53L0X_PerformRefSpadManagement(&VL53L0XDevs[i], &refSpadCount, &isApertureSpads);
			if( status )
			{
				debug_printf("VL53L0X_PerformRefSpadManagement failed\n");
			}

			status = VL53L0X_SetDeviceMode(&VL53L0XDevs[i], VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
			if( status )
			{
				debug_printf("VL53L0X_SetDeviceMode failed\n");
			}

			status = VL53L0X_SetLimitCheckEnable(&VL53L0XDevs[i], VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1); // Enable Sigma limit
			if( status )
			{
				debug_printf("VL53L0X_SetLimitCheckEnable failed\n");
			}

			status = VL53L0X_SetLimitCheckEnable(&VL53L0XDevs[i], VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1); // Enable Signa limit
			if( status )
			{
				debug_printf("VL53L0X_SetLimitCheckEnable failed\n");
			}
			/* Ranging configuration */
			switch(rangingConfig)
			{
			case LONG_RANGE:
				signalLimit = (FixPoint1616_t)(0.1*65536);
				sigmaLimit = (FixPoint1616_t)(60*65536);
				timingBudget = 33000;
				preRangeVcselPeriod = 18;
				finalRangeVcselPeriod = 14;
				break;
			case HIGH_ACCURACY:
				signalLimit = (FixPoint1616_t)(0.25*65536);
				sigmaLimit = (FixPoint1616_t)(18*65536);
				timingBudget = 200000;
				preRangeVcselPeriod = 14;
				finalRangeVcselPeriod = 10;
				break;
			case HIGH_SPEED:
				signalLimit = (FixPoint1616_t)(0.25*65536);
				sigmaLimit = (FixPoint1616_t)(32*65536);
				timingBudget = 20000;
				preRangeVcselPeriod = 14;
				finalRangeVcselPeriod = 10;
				break;
			default:
				debug_printf("Not Supported");
			}

			status = VL53L0X_SetLimitCheckValue(&VL53L0XDevs[i],  VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, signalLimit);
			if( status ){
				debug_printf("VL53L0X_SetLimitCheckValue failed\n");
			}

			status = VL53L0X_SetLimitCheckValue(&VL53L0XDevs[i],  VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, sigmaLimit);
			if( status ){
				debug_printf("VL53L0X_SetLimitCheckValue failed\n");
			}

			status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&VL53L0XDevs[i],  timingBudget);
			if( status ){
				debug_printf("VL53L0X_SetMeasurementTimingBudgetMicroSeconds failed\n");
			}

			status = VL53L0X_SetVcselPulsePeriod(&VL53L0XDevs[i],  VL53L0X_VCSEL_PERIOD_PRE_RANGE, preRangeVcselPeriod);
			if( status ){
				debug_printf("VL53L0X_SetVcselPulsePeriod failed\n");
			}

			status = VL53L0X_SetVcselPulsePeriod(&VL53L0XDevs[i],  VL53L0X_VCSEL_PERIOD_FINAL_RANGE, finalRangeVcselPeriod);
			if( status ){
				debug_printf("VL53L0X_SetVcselPulsePeriod failed\n");
			}

			status = VL53L0X_PerformRefCalibration(&VL53L0XDevs[i], &VhvSettings, &PhaseCal);
			if( status ){
				debug_printf("VL53L0X_PerformRefCalibration failed\n");
			}

			VL53L0XDevs[i].LeakyFirst=1;
		}
	}
}

char RangeToLetter(VL53L0X_Dev_t *pDev, VL53L0X_RangingMeasurementData_t *pRange)
{
	char c;
	if( pRange->RangeStatus == 0 )
	{
		if( pDev->LeakyRange < RangeLow )
		{
			c='_';
		}
		else if( pDev->LeakyRange < RangeMedium )
		{
			c='=';
		}
		else
		{
			c = '~';
		}

	}
	else
	{
		c='-';
	}
	return c;
}

/* Store new ranging data into the device structure, apply leaky integrator if needed */
void Sensor_SetNewRange(VL53L0X_Dev_t *pDev, VL53L0X_RangingMeasurementData_t *pRange)
{
	if( pRange->RangeStatus == 0 )
	{
		if( pDev->LeakyFirst )
		{
			pDev->LeakyFirst = 0;
			pDev->LeakyRange = pRange->RangeMilliMeter;
		}
		else
		{
			pDev->LeakyRange = (pDev->LeakyRange*LeakyFactorFix8 + (256 - LeakyFactorFix8)*pRange->RangeMilliMeter) >> 8;
		}
	}
	else
	{
		pDev->LeakyFirst = 1;
	}
}

void ResetAndDetectSensor(int SetDisplay){
	int nSensor;
	nSensor = DetectSensors(SetDisplay);
	/* at least one sensor and if one it must be the built-in one  */
	if( (nSensor <= 0) ||  (nSensor == 1 && VL53L0XDevs[0].Present == 0) ){
		HandleError(ERR_DETECT);
	}
}

int telemeters_Test(void)
{
	int status;
	int i;
	int nSensorToUse;

	RangingConfig_e RangingConfig = HIGH_SPEED;

	/* Initialize timestamping for UART logging */
	//	TimeStamp_Init();

	/* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(TOF_EN_RS_GPIO_Port, TOF_EN_RS_Pin, 0);
	HAL_GPIO_WritePin(TOF_EN_RD_GPIO_Port, TOF_EN_RD_Pin, 0);
	HAL_GPIO_WritePin(TOF_EN_RF_GPIO_Port, TOF_EN_RF_Pin, 0);
	HAL_GPIO_WritePin(TOF_EN_LS_GPIO_Port, TOF_EN_LS_Pin, 0);
	HAL_GPIO_WritePin(TOF_EN_LD_GPIO_Port, TOF_EN_LD_Pin, 0);
	HAL_GPIO_WritePin(TOF_EN_LF_GPIO_Port, TOF_EN_LF_Pin, 0);

	printf(WelcomeMsg);

	ResetAndDetectSensor(1);

	/* Set VL53L0X API trace level */
	VL53L0X_trace_config(NULL, TRACE_MODULE_NONE, TRACE_LEVEL_NONE, TRACE_FUNCTION_NONE); // No Trace
	//VL53L0X_trace_config(NULL,TRACE_MODULE_ALL, TRACE_LEVEL_ALL, TRACE_FUNCTION_ALL); // Full trace

	HAL_Delay(ModeChangeDispTime);

	/* Display Ranging config */
	//      VL53L0x_SetDisplayString(RangingConfigTxt[RangingConfig]);
	printf(RangingConfigTxt[RangingConfig]);
	printf("\n");

	HAL_Delay(ModeChangeDispTime);

	/* Setup all sensors in Single Shot mode */
	SetupSingleShot(RangingConfig);

	/* Start Ranging demo */
	while(1)
	{

//		/* Which sensor to use ? */
//		for(i = 0, nSensorToUse = 0; i < 6; i++)
//		{
//			//		if ((UseSensorsMask & (1 << i) ) && VL53L0XDevs[i].Present)
//			if (VL53L0XDevs[i].Present)
//			{
//				nSensorToUse++;
//			}
//		}
//		if( nSensorToUse == 0 )
//		{
//			return -1;
//		}

		/* Read 6 devices */
		for (i = 0; i < 6; i++)
		{
			if (!VL53L0XDevs[i].Present)// || (UseSensorsMask & (1 << i)) == 0)
				continue;
			/* Call All-In-One blocking API function */
			status = VL53L0X_PerformSingleRangingMeasurement(&VL53L0XDevs[i],&RangingMeasurementData);
			if( status )
			{
				HandleError(ERR_DEMO_RANGE_MULTI);
			}
			/* Push data logging to UART */
//			trace_printf("%d,%d,%d,%d\n", VL53L0XDevs[i].Id, RangingMeasurementData.RangeStatus, RangingMeasurementData.RangeMilliMeter, RangingMeasurementData.SignalRateRtnMegaCps);
			/* Store new ranging distance */
			Sensor_SetNewRange(&VL53L0XDevs[i],&RangingMeasurementData);
		}
		char bargraph[100] = {0};
		for (int c = 0; c < (RangingMeasurementData.RangeMilliMeter/40) && c < 90; c++)
		{
			bargraph[c] = '|';
		}
		printf("%s\n", bargraph);

//		printf("------------------------------\n");
		HAL_Delay(1);
	}
}
