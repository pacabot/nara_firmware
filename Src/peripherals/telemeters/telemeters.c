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

/**
 * @defgroup Configuration Static configuration
 * @{
 */
#define HAVE_ALARM_DEMO 0

/** Time the initial 53L0 message is shown at power up */
#define WelcomeTime 660

/** Time the initial 53L0 message is shown at power up */
#define ModeChangeDispTime  500

/**
 * Time considered as  a "long push" on push button
 */
#define PressBPSwicthTime   1000

/** @}  */ /* config group */

#ifndef MIN
#   define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC

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
#if HAVE_ALARM_DEMO
const char TxtAlarm[]       = "Alr";
#endif


typedef enum {
	LONG_RANGE      = 0, /*!< Long range mode */
	HIGH_SPEED      = 1, /*!< High speed mode */
	HIGH_ACCURACY   = 2, /*!< High accuracy mode */
} RangingConfig_e;
char *RangingConfigTxt[3] = {"LR", "HS", "HA"};

typedef enum {
	RANGE_VALUE     = 0, /*!< Range displayed in cm */
	BAR_GRAPH       = 1, /*!< Range displayed as a bar graph : one bar per sensor */
} DemoMode_e;
char *DemoModeTxt[2] = {"rng", "bar"};

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
int nDevPresent=0;
/** bit is index in VL53L0XDevs that is not necessary the dev id of the BSP */
int nDevMask;


VL53L0X_Dev_t VL53L0XDevs[]={
		{.Id=VL53L0x_DEV_LEFT, .DevLetter='l', .I2cHandle=&hi2c2, .I2cDevAddr=0x52},
		{.Id=VL53L0x_DEV_CENTER, .DevLetter='c', .I2cHandle=&hi2c2, .I2cDevAddr=0x52},
		{.Id=VL53L0x_DEV_RIGHT, .DevLetter='r', .I2cHandle=&hi2c2, .I2cDevAddr=0x52},
};

/** range low (and high) in @a RangeToLetter()
 *
 * used for displaying  multiple sensor as bar graph
 */
int RangeLow=100;

/** range medium in @a RangeToLetter()
 *
 * used for displaying  multiple sensor as bar graph
 */
int RangeMedium=300;

/** Timer
 *
 * Used to get time stamp for UART logging
 */
TIM_HandleTypeDef htim5;

///* TIM5 init function */
//void MX_TIM5_Init(void)
//{
//
//  TIM_MasterConfigTypeDef sMasterConfig;
//  TIM_OC_InitTypeDef sConfigOC;
//
//  htim5.Instance = TIM5;
//  htim5.Init.Prescaler = 83;
//  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
//  htim5.Init.Period = 0xFFFFFFFF;
//  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//  HAL_TIM_OC_Init(&htim5);
//
//  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
//  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//  HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig);
//
//  sConfigOC.OCMode = TIM_OCMODE_TIMING;
//  sConfigOC.Pulse = 0;
//  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
//  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
//  HAL_TIM_OC_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1);
//
//}

void TimeStamp_Init(){
	//    MX_TIM5_Init();
}

void TimeStamp_Reset(){
	HAL_TIM_Base_Start(&htim5);
	htim5.Instance->CNT=0;
}

uint32_t TimeStamp_Get(){
	return htim5.Instance->CNT;
}

/* Private function prototypes -----------------------------------------------*/
void ResetAndDetectSensor(int SetDisplay);

#define debug_printf    trace_printf
char WelcomeMsg[]="Hi I am Ranging VL53L0X \n"; //PRE

#if HAVE_ALARM_DEMO
volatile int IntrCount;
volatile int LastIntrPin;
volatile int LastIntrId;
volatile int IntrCounts[3];
#endif

#define BSP_BP_PORT GPIOC
#define BSP_BP_PIN  GPIO_PIN_13

int BSP_GetPushButton(void){
	GPIO_PinState state ;
	state = HAL_GPIO_ReadPin(BSP_BP_PORT, BSP_BP_PIN);
	return state;
}

#if HAVE_ALARM_DEMO
/**
 * Interrupt handler called each time an interrupt is produced by the ranging sensor (in ALARM mode)
 * @param err
 */
void VL53L0A1_EXTI_Callback(int DevNo, int GPIO_Pin){
	IntrCount++;
	LastIntrPin=GPIO_Pin;
	LastIntrId=DevNo;

	if( DevNo< ARRAY_SIZE(IntrCounts)  ){
		IntrCounts[DevNo]++;
	}
}
#endif


/**
 * Handle Error
 *
 * Set err on display and loop forever
 * @param err Error case code
 */
void HandleError(int err){
	char msg[16];
	sprintf(msg,"Er%d", err);
	printf(msg);
	printf("\n");
	while(1){};
}

int telemetersResetId(int DevNo, int state) {
    int status = 0;
    switch( DevNo ){
    case VL53L0x_DEV_CENTER :
    case 'c' :
//        CurIOVal.bytes[3]&=~0x80; /* bit 15 expender 1  => byte #3 */
//        if( state )
//            CurIOVal.bytes[3]|=0x80; /* bit 15 expender 1  => byte #3 */
//        status= _ExpanderWR(I2cExpAddr1, GPSR+1, &CurIOVal.bytes[3], 1);
        break;
    case VL53L0x_DEV_LEFT :
    case 'l' :
//        CurIOVal.bytes[1]&=~0x40; /* bit 14 expender 0 => byte #1*/
//        if( state )
//            CurIOVal.bytes[1]|=0x40; /* bit 14 expender 0 => byte #1*/
//        status= _ExpanderWR(I2cExpAddr0, GPSR+1, &CurIOVal.bytes[1], 1);
        HAL_GPIO_WritePin(TOF_EN_RF_GPIO_Port, TOF_EN_RF_Pin, state);
        break;
    case 'r' :
    case VL53L0x_DEV_RIGHT :
//        CurIOVal.bytes[1]&=~0x80; /* bit 15 expender 0  => byte #1 */
//        if( state )
//            CurIOVal.bytes[1]|=0x80; /* bit 15 expender 0 => byte #1*/
//        status= _ExpanderWR(I2cExpAddr0, GPSR+1, &CurIOVal.bytes[1], 1);
        break;
    default:
        VL53L0x_ErrLog("Invalid DevNo %d",DevNo);
        status = -1;
        goto done;
    }
//error with valid id
    if( status ){
        VL53L0x_ErrLog("expander i/o error for DevNo %d state %d ",DevNo, state);
    }
done:
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
	for (i = 0; i < 1; i++)
		status = telemetersResetId(i, 0);

	/* detect all sensors (even on-board)*/
	for (i = 0; i < 1; i++) {
		VL53L0X_Dev_t *pDev;
		pDev = &VL53L0XDevs[i];
		pDev->I2cDevAddr = 0x52;
		pDev->Present = 0;
		status = telemetersResetId( pDev->Id, 1);
		HAL_Delay(2);
		FinalAddress=0x52+(i+1)*2;

		do {
			/* Set I2C standard mode (400 KHz) before doing the first register access */
			if (status == VL53L0X_ERROR_NONE)
				status = VL53L0X_WrByte(pDev, 0x88, 0x00);

			/* Try to read one register using default 0x52 address */
			status = VL53L0X_RdWord(pDev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
			if (status) {
				debug_printf("#%d Read id fail\n", i);
				break;
			}
			if (Id == 0xEEAA) {
				/* Sensor is found => Change its I2C address to final one */
				status = VL53L0X_SetDeviceAddress(pDev,FinalAddress);
				if (status != 0) {
					debug_printf("#i VL53L0X_SetDeviceAddress fail\n", i);
					break;
				}
				pDev->I2cDevAddr = FinalAddress;
				/* Check all is OK with the new I2C address and initialize the sensor */
				status = VL53L0X_RdWord(pDev, VL53L0X_REG_IDENTIFICATION_MODEL_ID, &Id);
				if (status != 0) {
					debug_printf("#i VL53L0X_RdWord fail\n", i);
					break;
				}

				status = VL53L0X_DataInit(pDev);
				if( status == 0 ){
					pDev->Present = 1;
				}
				else{
					debug_printf("VL53L0X_DataInit %d fail\n", i);
					break;
				}
				trace_printf("VL53L0X %d Present and initiated to final 0x%x\n", pDev->Id, pDev->I2cDevAddr);
				nDevPresent++;
				nDevMask |= 1 << i;
				pDev->Present = 1;
			}
			else {
				debug_printf("#%d unknown ID %x\n", i, Id);
				status = 1;
			}
		} while (0);
		/* if fail r can't use for any reason then put the  device back to reset */
		if (status) {
			telemetersResetId(i, 0);
		}
	}
	/* Display detected sensor(s) */
	if( SetDisplay ){
		for(i=0; i<3; i++){
			if( VL53L0XDevs[i].Present ){
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

	for( i=0; i<3; i++){
		if( VL53L0XDevs[i].Present){
			status=VL53L0X_StaticInit(&VL53L0XDevs[i]);
			if( status ){
				debug_printf("VL53L0X_StaticInit %d failed\n",i);
			}

			status = VL53L0X_PerformRefCalibration(&VL53L0XDevs[i], &VhvSettings, &PhaseCal);
			if( status ){
				debug_printf("VL53L0X_PerformRefCalibration failed\n");
			}

			status = VL53L0X_PerformRefSpadManagement(&VL53L0XDevs[i], &refSpadCount, &isApertureSpads);
			if( status ){
				debug_printf("VL53L0X_PerformRefSpadManagement failed\n");
			}

			status = VL53L0X_SetDeviceMode(&VL53L0XDevs[i], VL53L0X_DEVICEMODE_SINGLE_RANGING); // Setup in single ranging mode
			if( status ){
				debug_printf("VL53L0X_SetDeviceMode failed\n");
			}

			status = VL53L0X_SetLimitCheckEnable(&VL53L0XDevs[i], VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1); // Enable Sigma limit
			if( status ){
				debug_printf("VL53L0X_SetLimitCheckEnable failed\n");
			}

			status = VL53L0X_SetLimitCheckEnable(&VL53L0XDevs[i], VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1); // Enable Signa limit
			if( status ){
				debug_printf("VL53L0X_SetLimitCheckEnable failed\n");
			}
			/* Ranging configuration */
			switch(rangingConfig) {
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

char RangeToLetter(VL53L0X_Dev_t *pDev, VL53L0X_RangingMeasurementData_t *pRange){
	char c;
	if( pRange->RangeStatus == 0 ){
		if( pDev->LeakyRange < RangeLow ){
			c='_';
		}
		else if( pDev->LeakyRange < RangeMedium ){
			c='=';
		}
		else {
			c = '~';
		}

	}
	else{
		c='-';
	}
	return c;
}

/* Store new ranging data into the device structure, apply leaky integrator if needed */
void Sensor_SetNewRange(VL53L0X_Dev_t *pDev, VL53L0X_RangingMeasurementData_t *pRange){
	if( pRange->RangeStatus == 0 ){
		if( pDev->LeakyFirst ){
			pDev->LeakyFirst = 0;
			pDev->LeakyRange = pRange->RangeMilliMeter;
		}
		else{
			pDev->LeakyRange = (pDev->LeakyRange*LeakyFactorFix8 + (256-LeakyFactorFix8)*pRange->RangeMilliMeter)>>8;
		}
	}
	else{
		pDev->LeakyFirst = 1;
	}
}

/**
 * Implement the ranging demo with all modes managed through the blue button (short and long press)
 * This function implements a while loop until the blue button is pressed
 * @param UseSensorsMask Mask of any sensors to use if not only one present
 * @param rangingConfig Ranging configuration to be used (same for all sensors)
 */
int RangeDemo(int UseSensorsMask, RangingConfig_e rangingConfig){
	int over=0;
	int status;
	char StrDisplay[5];
	char c;
	int i;
	int nSensorToUse;
	int SingleSensorNo=0;

	/* Setup all sensors in Single Shot mode */
	SetupSingleShot(rangingConfig);

	/* Which sensor to use ? */
	for(i=0, nSensorToUse=0; i<3; i++){
		if(( UseSensorsMask& (1<<i) ) && VL53L0XDevs[i-1].Present ){
			nSensorToUse++;
			if( nSensorToUse==1 )
				SingleSensorNo=i-1;
		}
	}
	if( nSensorToUse == 0 ){
		return -1;
	}

	/* Start ranging until blue button is pressed */
	//    do{
	//        if( nSensorToUse >1 ){
	//            /* Multiple devices */
	//            strcpy(StrDisplay, "    ");
	//            for( i=0; i<3; i++){
	//                if( ! VL53L0XDevs[i].Present  || (UseSensorsMask & (1<<i))==0 )
	//                    continue;
	//                /* Call All-In-One blocking API function */
	//                status = VL53L0X_PerformSingleRangingMeasurement(&VL53L0XDevs[i],&RangingMeasurementData);
	//                if( status ){
	//                    HandleError(ERR_DEMO_RANGE_MULTI);
	//                }
	//                /* Push data logging to UART */
	//                trace_printf("%d,%u,%d,%d,%d\n", VL53L0XDevs[i].Id, TimeStamp_Get(), RangingMeasurementData.RangeStatus, RangingMeasurementData.RangeMilliMeter, RangingMeasurementData.SignalRateRtnMegaCps);
	//                /* Store new ranging distance */
	//                Sensor_SetNewRange(&VL53L0XDevs[i],&RangingMeasurementData);
	//                /* Translate distance in bar graph (multiple device) */
	//                c = RangeToLetter(&VL53L0XDevs[i],&RangingMeasurementData);
	//                StrDisplay[i+1]=c;
	//            }
	//        }
	//        else{
	/* only one sensor */
	/* Call All-In-One blocking API function */
	status = VL53L0X_PerformSingleRangingMeasurement(&VL53L0XDevs[SingleSensorNo],&RangingMeasurementData);
	if( status ==0 ){
		/* Push data logging to UART */
		trace_printf("%d,%u,%d,%d,%d\n", VL53L0XDevs[SingleSensorNo].Id, TimeStamp_Get(), RangingMeasurementData.RangeStatus, RangingMeasurementData.RangeMilliMeter, RangingMeasurementData.SignalRateRtnMegaCps);
		Sensor_SetNewRange(&VL53L0XDevs[SingleSensorNo],&RangingMeasurementData);
		/* Display distance in cm */
		if( RangingMeasurementData.RangeStatus == 0 ){
			sprintf(StrDisplay, "%3dc",(int)VL53L0XDevs[SingleSensorNo].LeakyRange/10);
		}
		else{
			sprintf(StrDisplay, "----");
			StrDisplay[0]=VL53L0XDevs[SingleSensorNo].DevLetter;
		}
	}
	else{
		HandleError(ERR_DEMO_RANGE_ONE);
	}
	//        }
	//        VL53L0x_SetDisplayString(StrDisplay);
	printf(StrDisplay);
	printf("\n");
	/* Check blue button */
	//        if( !BSP_GetPushButton() ){
	//            over=1;
	//            break;
	//        }
	//    }while( !over);
	//    /* Wait button to be un-pressed to decide if it is a short or long press */
	//    status=PusbButton_WaitUnPress();
	return status;
}

#if HAVE_ALARM_DEMO
struct AlrmMode_t {
	const int VL53L0X_Mode;
	const char *Name;
	uint32_t ThreshLow;
	uint32_t ThreshHigh;
};

struct AlrmMode_t AlarmModes[]={
		{ .VL53L0X_Mode = VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_LOW , .Name="Lo" , .ThreshLow=300<<16 ,  .ThreshHigh=0<<16  },
		{ .VL53L0X_Mode = VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_HIGH, .Name= "hi", .ThreshLow=0<<16   ,  .ThreshHigh=300<<16},
		{ .VL53L0X_Mode = VL53L0X_GPIOFUNCTIONALITY_THRESHOLD_CROSSED_OUT , .Name="out", .ThreshLow=300<<16 ,  .ThreshHigh=400<<16},
};

VL53L0X_Error WaitStopCompleted(VL53L0X_DEV Dev) {
	VL53L0X_Error Status = VL53L0X_ERROR_NONE;
	uint32_t StopCompleted=0;
	uint32_t LoopNb;

	// Wait until it finished
	// use timeout to avoid deadlock
	if (Status == VL53L0X_ERROR_NONE) {
		LoopNb = 0;
		do {
			Status = VL53L0X_GetStopCompletedStatus(Dev, &StopCompleted);
			if ((StopCompleted == 0x00) || Status != VL53L0X_ERROR_NONE) {
				break;
			}
			LoopNb = LoopNb + 1;
			VL53L0X_PollingDelay(Dev);
		} while (LoopNb < VL53L0X_DEFAULT_MAX_LOOP);

		if (LoopNb >= VL53L0X_DEFAULT_MAX_LOOP) {
			Status = VL53L0X_ERROR_TIME_OUT;
		}

	}

	return Status;
}


void AlarmDemo(void){
	VL53L0X_Dev_t *pDev;
	uint8_t VhvSettings;
	uint8_t PhaseCal;
	uint32_t refSpadCount;
	uint8_t isApertureSpads;
	VL53L0X_RangingMeasurementData_t RangingMeasurementData;
	int status;
	int Over=0;
	int Mode=0;
	char StrDisplay[5]="----";

	/* Only center device is used */
	pDev=&VL53L0XDevs[1];


	/* configure BSP/MCU center sensor interrupt */
	VL53L0A1_EXTI_IOConfigure(VL53L0x_DEV_CENTER, 0, 0);
	VL53L0x_SetIntrStateId(1, VL53L0x_DEV_CENTER);

	/* Initialize the device in continuous ranging mode */
	VL53L0X_StaticInit(pDev);
	VL53L0X_PerformRefCalibration(pDev, &VhvSettings, &PhaseCal);
	VL53L0X_PerformRefSpadManagement(pDev, &refSpadCount, &isApertureSpads);
	VL53L0X_SetInterMeasurementPeriodMilliSeconds(pDev, 250);
	VL53L0X_SetDeviceMode(pDev, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING);

	do{
		/* set sensor interrupt mode */
		VL53L0X_StopMeasurement(pDev);           // it is safer to do this while sensor is stopped
		VL53L0X_SetInterruptThresholds(pDev, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING ,  AlarmModes[Mode].ThreshLow ,  AlarmModes[Mode].ThreshHigh);
		status = VL53L0X_SetGpioConfig(pDev, 0, VL53L0X_DEVICEMODE_CONTINUOUS_RANGING, AlarmModes[Mode].VL53L0X_Mode, VL53L0X_INTERRUPTPOLARITY_HIGH);
		status = VL53L0X_ClearInterruptMask(pDev, -1); // clear interrupt pending if any

		/* Start continuous ranging */
		VL53L0X_StartMeasurement(pDev);
		IntrCounts[1]=0;

		/* Check for interrupt */
		do{
			__WFI();
			/* Interrupt received */
			if( IntrCounts[1] !=0 ){
				/* Reset interrupt counter */
				IntrCounts[1]=0;
				/* Get ranging data and display distance*/
				VL53L0X_GetRangingMeasurementData(pDev, &RangingMeasurementData);
				sprintf(StrDisplay, "%3dc",(int)RangingMeasurementData.RangeMilliMeter/10);
				/* Clear interrupt */
				status = VL53L0X_ClearInterruptMask(pDev, -1);
				/* keep display for at least 100ms otherwise user may never see it on display*/
				VL53L0x_SetDisplayString(StrDisplay);
				HAL_Delay(100);
			}
			else{
				/* No interrupt received => Display alarm mode */
				VL53L0x_SetDisplayString(AlarmModes[Mode].Name);
			}
			/* Check blue button */
			if( !BSP_GetPushButton() ){
				break;
			}
		}while(1);
		/* Wait button to be released to decide if it is a short or long press */
		status=PusbButton_WaitUnPress();
		/* Long press => stop this demo */
		if( status )
			Over =1;
		/* Short press => change alarm mode */
		Mode=(Mode+1)%ARRAY_SIZE(AlarmModes);
	}while( !Over );

	/* Stop continuous ranging */
	VL53L0X_StopMeasurement(pDev);

	/* Ensure device is ready for other commands */
	WaitStopCompleted(pDev);

	/* Disable configuration of BSP/MCU center sensor interrupt */
	VL53L0x_SetIntrStateId(0, VL53L0x_DEV_CENTER);
	VL53L0A1_EXTI_IOUnconfigure(VL53L0x_DEV_CENTER);
}
#endif

void ResetAndDetectSensor(int SetDisplay){
	int nSensor;
	nSensor = DetectSensors(SetDisplay);
	/* at least one sensor and if one it must be the built-in one  */
	if( (nSensor <=0) ||  (nSensor ==1 && VL53L0XDevs[0].Present==0) ){
		HandleError(ERR_DETECT);
	}
}

/* USER CODE END 0 */

int telemeters_Test(void)
{

	/* USER CODE BEGIN 1 */
	int ExitWithLongPress;
	RangingConfig_e RangingConfig = LONG_RANGE;
	DemoMode_e DemoMode = RANGE_VALUE;
	int UseSensorsMask = 1<<VL53L0x_DEV_CENTER;
	/* USER CODE END 1 */

	/* MCU Configuration----------------------------------------------------------*/

	/* Initialize timestamping for UART logging */
	TimeStamp_Init();

	/* USER CODE BEGIN 2 */
	//  VL53L0x_Init();
	HAL_GPIO_WritePin(TOF_EN_RF_GPIO_Port, TOF_EN_RF_Pin, 0);
	printf(WelcomeMsg);
	//  VL53L0x_SetDisplayString("53L0");
	printf("53L0");
	printf("\n");
	//  HAL_Delay(WelcomeTime);
	ResetAndDetectSensor(1);

	/* Set VL53L0X API trace level */
	VL53L0X_trace_config(NULL, TRACE_MODULE_NONE, TRACE_LEVEL_NONE, TRACE_FUNCTION_NONE); // No Trace
	//VL53L0X_trace_config(NULL,TRACE_MODULE_ALL, TRACE_LEVEL_ALL, TRACE_FUNCTION_ALL); // Full trace


	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */
		/* Display demo mode */
		//      VL53L0x_SetDisplayString(DemoModeTxt[DemoMode]);
		printf(DemoModeTxt[DemoMode]);
		printf("\n");
		HAL_Delay(ModeChangeDispTime);

		/* Display Ranging config */
		//      VL53L0x_SetDisplayString(RangingConfigTxt[RangingConfig]);
		printf(RangingConfigTxt[RangingConfig]);
		printf("\n");

		HAL_Delay(ModeChangeDispTime);

		/* Reset and Detect all sensors */
		//      ResetAndDetectSensor(1);

		/* Reset Timestamping */
		TimeStamp_Reset();

#if HAVE_ALARM_DEMO
		VL53L0x_SetDisplayString(TxtAlarm);
		HAL_Delay(ModeChangeDispTime);
		ResetAndDetectSensor(0);
		AlarmDemo();
#else

		/* Start Ranging demo */
		while(1)
		{
			ExitWithLongPress = RangeDemo(UseSensorsMask, RangingConfig);
		}

		/* Blue button has been pressed (long or short press) */
		if(ExitWithLongPress){
			/* Long press : change demo mode if multiple sensors present*/
			if( nDevPresent >1 ){
				/* If more than one sensor is present then toggle demo mode */
				DemoMode = (DemoMode == RANGE_VALUE) ? BAR_GRAPH : RANGE_VALUE;
				UseSensorsMask = (DemoMode == BAR_GRAPH) ? 0x7 : 1<<VL53L0x_DEV_CENTER;
			}
		} else {
			/* Short press : change ranging config */
			RangingConfig = (RangingConfig == LONG_RANGE) ? HIGH_SPEED : ((RangingConfig == HIGH_SPEED) ? HIGH_ACCURACY : LONG_RANGE);
		}
#endif
		/* USER CODE BEGIN 3 */

	}
	/* USER CODE END 3 */

}
