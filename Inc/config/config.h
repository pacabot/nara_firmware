/* config.h */

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <math.h>

/**************************************************************************************/
/***************                 STM32 definitions                 ********************/
/**************************************************************************************/
#define ADC_VOLTAGE				(3300)                //STM32 ADC peripheral reference is 3300mV
#define ADC_COEFF 				((ADC_VOLTAGE) / (4095))  //ADC value/mV

/**************************************************************************************/
/***************                 Times definitions                 ********************/
/**************************************************************************************/
#define HI_TIME_FREQ			    (40000.00) 	            //use for master inner loop
#define LOW_TIME_FREQ               (100.00)                //low inner loop
#define CONTROL_TIME_FREQ           (1000.00)               //use for control inner loop
#define TELEMETERS_TIME_FREQ        ((50.00) * (6.00))     //each telemeter use 1/10 of TELEMETERS_TIME_FREQ
#define GYRO_TIME_FREQ			    (2000.00)               //for adc injected freq
#define LINESENSORS_TIME_FREQ	    (1000.00)               //line sensor measure frequency
#define MOTORS_FREQ				    (23000.00)	            //motor pwm freq
#define FLOORSENSORS_PWM_FREQ		(20000.00)	            //emitter led pwm frequency

/**************************************************************************************/
/***************                 Gyro definitions                  ********************/
/**************************************************************************************/

/**************************************************************************************/
/***************                 Temperature STM32                 ********************/
/**************************************************************************************/
#define STM32_VREFINT			(3300.00)	//1210.00 if use Vrefint
#define STM32_T_SENSITIVITY		(2.50)	//The temperature coefficient is ~2.5 mV/�C at 25�C
#define STM32_T_V25				(760.00)	//Voltage at 25 °C
#define STM32_T_COEFF_A			((STM32_VREFINT) / ((4095.00) * (STM32_T_SENSITIVITY)))
#define STM32_T_COEFF_B			((25.00) - ((STM32_T_V25) / (STM32_T_SENSITIVITY)))

/**************************************************************************************/
/***************                    	VBAT                       ********************/
/**************************************************************************************/

/**************************************************************************************/
/***************                     Telemeters                    ********************/
/**************************************************************************************/
#define VL53L0X_LOG_ENABLE          /.enable tof telemeters logging
#define TRACE_UART              	(1)
#define VL53L0x_TRACE

/**************************************************************************************/
/***************                   Floor sensors                   ********************/
/**************************************************************************************/
#define FLOOR_SENSOR_ADC_THESHOLD	(1500)

/**************************************************************************************/
/***************                       Battery                     ********************/
/**************************************************************************************/
#define BATTERY_CELL_NUMBER					(4.00)	//4S
#define BATTERY_LOWER_VOLTAGE_NO_LOAD		((3000) * (BATTERY_CELL_NUMBER))	//https://learn.sparkfun.com/tutorials/battery-technologies/lithium-polymer
#define BATTERY_UPPER_VOLTAGE_NO_LOAD		((3700) * (BATTERY_CELL_NUMBER))
#define BATTERY_LOWER_VOLTAGE_OFFSET		((-)0.10) * (BATTERY_CELL_NUMBER))	//-0.1V/A
#define BATTERY_COEFF_A						(((BATTERY_UPPER_VOLTAGE_NO_LOAD) - (BATTERY_LOWER_VOLTAGE_NO_LOAD)) / (100.00))
#define BATTERY_COEFF_B						((BATTERY_LOWER_VOLTAGE_NO_LOAD) / (BATTERY_COEFF_A))

/**************************************************************************************/
/***************                 Mechanical Constants              ********************/
/**************************************************************************************/
#define WHEEL_DIAMETER			(53.00) 	    //Wheel diameter in millimeters
#define WHEELS_DISTANCE			(56.00)	        //Distance between right and left center wheels
#define	GEAR_RATIO				(32.00 / 14.00)	//wheel gear teeth per motor gear teeth
#define MOTOR_GEAR_RATIO        (23)            //motors gear ratio
#define ENCODER_RESOLUTION  	(6.00)	        //Number steps per revolution (HALLs)

#define STEPS_PER_WHEEL_REV		((ENCODER_RESOLUTION) * (GEAR_RATIO) * (MOTOR_GEAR_RATIO))	//Number steps per wheel revolution
#define MM_PER_WHEEL_REV		((M_PI) * (WHEEL_DIAMETER))		//Number of millimeters per wheel revolution
#define STEPS_PER_MM			((STEPS_PER_WHEEL_REV) / (MM_PER_WHEEL_REV))	//Number of steps per millimeter

/**************************************************************************************/
/***************                  Robot Dimensions                 ********************/
/**************************************************************************************/
#define NARA_WIDTH				(98.00)
#define NARA_LENGHT				(98.00)
#define NARA_HEIGHT				(30)
#define NARA_CENTER_BACK_DIST   (28)
#define NARA_CENTER_FRONT_DIST  (NARA_LENGHT) - (NARA_CENTER_BACK_DIST))

/**************************************************************************************/
/***************                   Dohyo Properties                 *******************/
/**************************************************************************************/
#define WHITE_BAND_THICKNESS	(12.00)
#define DOHYO_DIAMETER		    (770)

/**************************************************************************************/
/***************                     Sumo speed                    ********************/
/**************************************************************************************/

#define RUN1_SPEED_ROTATION         (500)
#define RUN1_MIN_SPEED_TRANSLATION  (500)
#define RUN1_MAX_SPEED_TRANSLATION  (1500)

#define RUN2_SPEED_ROTATION         (600)
#define RUN2_MIN_SPEED_TRANSLATION  (600)
#define RUN2_MAX_SPEED_TRANSLATION  (3000)

#define SCAN_SPEED_ROTATION         (400)
#define SCAN_MIN_SPEED_TRANSLATION  (400)
#define SCAN_MAX_SPEED_TRANSLATION  (800)

#define SAFE_SPEED_ROTATION         (400)
#define SAFE_SPEED_TRANSLATION      (400)

#define RETURN_START_CELL

/**************************************************************************************/
/***************                 Physical Constants                ********************/
/**************************************************************************************/
#define MAX_SPEED               (1500.00)   //mm/s
#define MAX_ACCEL               (800.00)   //mm/s/s

#define MAX_CURVE_ACCEL         (500.00)   //mm/s
#define MAX_CURVE_SPEED         (500.00)   //mm/s

#define MAX_ROTATE_ACCEL        (500.00)   //°/s/s perhaps wrong scale //todo verify correct scale
#define MAX_TURN_ACCEL          (500.00)   //°/s/s perhaps wrong scale //todo verify correct scale

/**************************************************************************************/
/***************                 Motors Constants                  ********************/
/**************************************************************************************/

#define PWM_RATIO_COEFF_A		((-0.50) / (6000.00))	//compute pwm ratio for limit motor voltage
#define PWM_RATIO_COEFF_B		(1.50)			    //PWM_RATIO_COEFF_A * battery voltage + PWM_RATIO_COEFF_B = TRUE MOTOR PWM

#define MOTORS_PERIOD			(4095)  //PWM scale 1000 = 100%

/*Calculations For Motor*/
#define MOTORS_INDUCTANCE       (0.000065)    //H
#define MOTORS_RESISTANCE       (4.30)
#define MOTORS_PWM_FREQUENCY    (((((MOTORS_RESISTANCE) / (MOTORS_INDUCTANCE)) / ((2.00) * (PI))) * 2)

/**************************************************************************************/
/***************                  Flash Constants                  ********************/
/**************************************************************************************/

/**************************************************************************************/
/***************                     bluetooth                     ********************/
/**************************************************************************************/
//#define DISABLE_BLUETOOTH

/**************************************************************************************/
/***************                   Misc Constants                  ********************/
/**************************************************************************************/
// Define this variable to enable Command Line mode
// Note: If not defined, Hexadecimal Command mode is used by default
#define CONFIG_USE_CMDLINE
/**************************************************************************************/
/***************               sleep and kill params               ********************/
/**************************************************************************************/
#define KILL_WEN_UNUSED

#endif // __CONFIG_H__

