/*!
DC1812A-A
LTC2943: Multicell Battery Gas Gauge with Temperature, Voltage and Current Measurement

DC1812A-C
LTC2943-1: Multicell Battery Gas Gauge with Temperature, Voltage and Current Measurement

@verbatim

  Setup:
     Apply a voltage source to the V_BATT terminal and a load at the V_CHRG/LD terminal.
     Refer to the Demo Manual Guide for a detailed setup description. Ensure that JP1 is
     set to AL# position.

  Explanation of Commands:

     1 - Automatic Mode - Scans Voltage, Current, Temperature and charge every 60ms.
         Scanning interval has been increased to 1s for ease of reading on the Serial Prompt.
         Displays an alert if it is set in the status register at the time of scan.

     2 - Scan Mode - Scans Voltage, Current, Temperature and charge every 10s.
         Displays an alert if it is set in the status register at the time of scan.

     3 - Manual Mode - Provides a SnapShot of the Voltage, Current, Temperature and Accumulated Charge.
         After the initial SnapShot, the part goes into sleep mode where it only counts charge.
         Displays an alert if it is set in the status register at the time of scan.

     4 - Sleep Mode - The ADC portion of the LTC2943 is shutdown. In this mode only charge is accumulated.

     5 - Shutdown Mode - The LTC2943 goes into low power mode.

     6 - Settings - Allows user to set alert thresholds, set prescalar, set AL#/CC# pin mode and change the temperature and charge units.

NOTES
 Setup:
 Set the terminal baud rate to 115200 and select the newline terminator.
 Requires a power supply.
 Refer to demo manual DC1812A-A.

USER INPUT DATA FORMAT:
 decimal : 1024
 hex     : 0x400
 octal   : 02000  (leading 0 "zero")
 binary  : B10000000000
 float   : 1024.0

@endverbatim

http://www.linear.com/product/LTC2943
http://www.linear.com/product/LTC2943-1

http://www.linear.com/product/LTC2943#demoboards
http://www.linear.com/product/LTC2943-1#demoboards

REVISION HISTORY
$Revision: 5179 $
$Date: 2016-05-31 16:28:15 -0700 (Tue, 31 May 2016) $

Copyright (c) 2013, Linear Technology Corp.(LTC)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of Linear Technology Corp.

The Linear Technology Linduino is not affiliated with the official Arduino team.
However, the Linduino is only possible because of the Arduino team's commitment
to the open-source community.  Please, visit http://www.arduino.cc and
http://store.arduino.cc , and consider a purchase that will help fund their
ongoing work.
 */

/*! @file
    @ingroup LTC2943
 */

#include "stm32h7xx_hal.h"
#include "stdio.h"
#include "stdbool.h"
#include "math.h"

/* Declarations for this module */
#include "peripherals/fuelGauge/LTC2943.h"

// Function Declaration
static void print_title();                 // Print the title block
static void print_prompt();                // Print the Prompt

int8_t menu_1_automatic_mode(int8_t mAh_or_Coulombs,
                             int8_t celcius_or_kelvin ,
                             uint16_t prescalar_mode,
                             uint16_t prescalarValue,
                             uint16_t alcc_mode);

int8_t menu_2_scan_mode(int8_t mAh_or_Coulombs ,
                        int8_t celcius_or_kelvin ,
                        uint16_t prescalar_mode,
                        uint16_t prescalarValue,
                        uint16_t alcc_mode);

int8_t menu_3_manual_mode(int8_t mAh_or_Coulombs ,
                          int8_t celcius_or_kelvin ,
                          uint16_t prescalar_mode,
                          uint16_t prescalarValue,
                          uint16_t alcc_mode);

int8_t menu_4_sleep_mode(int8_t mAh_or_Coulombs ,
                         uint16_t prescalar_mode,
                         uint16_t prescalarValue,
                         uint16_t alcc_mode);

int8_t menu_5_shutdown_mode();

int8_t menu_6_settings(uint8_t *mAh_or_Coulombs,
                       uint8_t *celcius_or_kelvin,
                       uint16_t *prescalar_mode,
                       uint16_t *prescalarValue,
                       uint16_t *alcc_mode);

int8_t menu_6_settings_menu_1_set_alert_thresholds();

int8_t menu_6_alert_menu_1_set_charge_thresholds();

int8_t menu_6_alert_menu_2_set_voltage_thresholds();

int8_t menu_6_alert_menu_3_set_current_thresholds();

int8_t menu_6_alert_menu_4_set_temperature_thresholds();

int8_t menu_6_settings_menu_2_get_alerts_thresholds();

int8_t menu_6_settings_menu_3_set_prescalar_values(uint16_t *prescalar_mode,
                                                   uint16_t *prescalarValue);

uint8_t menu_6_alert_menu_4_set_allcc_state(uint16_t *alcc_mode);

uint8_t menu_6_alert_menu_5_set_units(uint8_t *mAh_or_Coulombs,
                                      uint8_t *celcius_or_kelvin);

bool isBitSet(uint8_t value,
              uint8_t position);

void checkAlerts(uint8_t status_code);

#define AUTOMATIC_MODE_DISPLAY_DELAY    1000                    //!< The HAL_Delay between readings in automatic mode
#define SCAN_MODE_DISPLAY_DELAY         1000                   //!< The HAL_Delay between readings in scan mode

// Change resistor value to 50 mOhm (0.05) for DC1812AC (LTC2943-1)
static float resistor = .100;                               //!< resistor value on demo board

static uint16_t prescalarValue = 1024;

extern UART_HandleTypeDef huart2;

// Error string
static const char ack_error[] = "Error: No Acknowledge. Check I2C Address."; //!< Error message

union uint8_float
{
    float value;
    uint8_t data[4];
};

static float read_float()
{
    uint8_t read_byte[11] = {0};
    uint32_t digit_cnt = 0;
    uint32_t int_val = 0;
    float float_val = 0;
    int8_t sign = 1;
    HAL_StatusTypeDef ret;

    while (HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY);

    for (digit_cnt = 0; digit_cnt < 5; digit_cnt++)
    {
        ret = HAL_UART_Receive(&huart2, &read_byte[digit_cnt], 1, 1000);
        if (ret == HAL_TIMEOUT)
        {
            return 0x0;
        }
        if ((read_byte[digit_cnt] == '\n') || (read_byte[digit_cnt] == '\r') || (read_byte[digit_cnt] == '.'))
        {
            break;
        }
        if (read_byte[digit_cnt] == '-')
        {
            read_byte[digit_cnt] = 0;
            sign = -1;
            continue;
        }
        read_byte[digit_cnt] -= '0';

        //         Ensure that the received digit is valid
        if ((read_byte[digit_cnt] < 0) || (read_byte[digit_cnt] > 9))
        {
            continue;
        }
    }

    for (int i = 0; i < digit_cnt; i++)
    {
        int_val += (read_byte[i]) * pow(10, digit_cnt - i - 1);
    }

    float_val = (float)int_val;

    for (digit_cnt = 0; digit_cnt < 5; digit_cnt++)
    {
        ret = HAL_UART_Receive(&huart2, &read_byte[digit_cnt + 4], 1, 1000);
        if (ret == HAL_TIMEOUT)
        {
            return 0x0;
        }
        if ((read_byte[digit_cnt + 4] == '\n') || (read_byte[digit_cnt + 4] == '\r'))
        {
            break;
        }
        read_byte[digit_cnt + 4] -= '0';

        //         Ensure that the received digit is valid
        if ((read_byte[digit_cnt + 4] < 0) || (read_byte[digit_cnt + 4] > 9))
        {
            continue;
        }
    }

    for (int i = 0; i < digit_cnt; i++)
    {
//        float float_test1 = ((float)read_byte[i + 4]);
//        float float_test2 = (float)(pow(10, i + 1));

        float_val += ((float)read_byte[i + 4]) / (float)(pow(10, i + 1));
    }

    return float_val * (float)sign;
}

static int32_t read_int(void)
{
    uint8_t read_byte[11] = {0};
    uint32_t digit_cnt = 0;
    int32_t int_val = 0;
    int8_t sign = 1;
    HAL_StatusTypeDef ret;

    //    while (HAL_UART_GetState(&huart2) != HAL_UART_STATE_READY);

    for (digit_cnt = 0; digit_cnt < 10; digit_cnt++)
    {
        ret = HAL_UART_Receive(&huart2, &read_byte[digit_cnt], 1, 1000);
        if (ret == HAL_TIMEOUT)
        {
            __HAL_UART_FLUSH_DRREGISTER(&huart2);
            return 0x0;
        }
        if ((read_byte[digit_cnt] == '\n') || (read_byte[digit_cnt] == '\r'))
        {
            break;
        }
        if (read_byte[digit_cnt] == '-')
        {
            read_byte[digit_cnt] = 0;
            sign = -1;
            continue;
        }
        read_byte[digit_cnt] -= '0';

        //         Ensure that the received digit is valid
        if ((read_byte[digit_cnt] < 0) || (read_byte[digit_cnt] > 9))
        {
            continue;
        }
    }

    for (int i = 0; i < digit_cnt; i++)
    {
        int_val += (read_byte[i]) * pow(10, digit_cnt - i - 1);
    }
    return int_val * sign;
}

void ltc2943Test()
{
    int8_t ack = 0;                                   //! I2C acknowledge indicator
    static uint32_t user_command = 0;                  //! The user input command
    static uint8_t mAh_or_Coulombs = 0;
    static uint8_t celcius_or_kelvin = 0;
    static uint16_t prescalar_mode = LTC2943_PRESCALAR_M_1024;
    static uint16_t alcc_mode = LTC2943_ALERT_MODE;
    int8_t LTC2943_mode;

    print_title();
    LTC2943_mode = LTC2943_AUTOMATIC_MODE|prescalar_mode|alcc_mode ;                                    //! Set the control mode of the LTC2943 to manual mode as well as set prescalar and AL#/CC# pin values.
    ack |= LTC2943_write(LTC2943_I2C_ADDRESS, LTC2943_CONTROL_REG, LTC2943_mode);
    //    LTC2943_write_16_bits(LTC2943_I2C_ADDRESS, LTC2943_ACCUM_CHARGE_MSB_REG, LTC2943_mAh_to_code(2000, resistor, prescalarValue));

    while(1)
    {
        //__HAL_UART_FLUSH_DRREGISTER(&huart2);
        //        while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) == RESET);
        printf("*************************");
        printf("\n\rMain Menu\n");
        printf("*************************");
        print_prompt();
        while (user_command > 6 || user_command < 1)
        {
            user_command = read_int();               //! Read user input command
        }
        printf("%d\n\n", (int)user_command);

        ack = 0;
        switch (user_command)                     //! Prints the appropriate submenu
        {
            case 1:
                ack |= menu_1_automatic_mode(mAh_or_Coulombs, celcius_or_kelvin, prescalar_mode, prescalarValue, alcc_mode);  //! Automatic Mode
                break;
            case 2:
                ack |= menu_2_scan_mode(mAh_or_Coulombs, celcius_or_kelvin, prescalar_mode, prescalarValue, alcc_mode);      //! Scan Mode
                break;
            case 3:
                ack |= menu_3_manual_mode(mAh_or_Coulombs, celcius_or_kelvin, prescalar_mode, prescalarValue, alcc_mode);    //! Manual Mode
                break;
            case 4:
                ack |= menu_4_sleep_mode(mAh_or_Coulombs, prescalar_mode, prescalarValue, alcc_mode);                        //! Sleep Mode
                break;
            case 5:
                ack |= menu_5_shutdown_mode();                                                                                //! Shutdown Mode
                break;
            case 6:
                ack |= menu_6_settings(&mAh_or_Coulombs, &celcius_or_kelvin, &prescalar_mode, &prescalarValue, &alcc_mode);  //! Settings Mode
                break;
        }
        if (ack != 0)                                                       //! If ack is not recieved print an error.
            printf("%d\n", (int)ack_error);
        printf("*************************");
        user_command = 0;
    }
}

// Function Definitions
//! Print the title block
static void print_title()
{
    printf("\n*****************************************************************\n");
    printf("* DC1812A Demonstration Program                                 *\n");
    printf("*                                                               *\n");
    printf("* This program communicates with the LTC2943 Multicell Coulomb  *\n");
    printf("* Counter found on the DC1812A demo board.                      *\n");
    printf("* Set the baud rate to 115200 and select the newline terminator.*\n");
    printf("*                                                               *\n");
    printf("*****************************************************************\n");
}

//! Print the Prompt
static void print_prompt()
{
    printf("\n1-Automatic Mode\n");
    printf("2-Scan Mode\n");
    printf("3-Manual Mode\n");
    printf("4-Sleep Mode\n");
    printf("5-Shutdown Mode\n");
    printf("6-Settings\n");
    printf("Enter a command: ");
}

//! Automatic Mode.
int8_t menu_1_automatic_mode(int8_t mAh_or_Coulombs, int8_t celcius_or_kelvin ,uint16_t prescalar_mode, uint16_t prescalarValue, uint16_t alcc_mode)
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
{
//    static uint32_t user_command = 0;                  //! The user input command
//    int8_t LTC2943_mode;
    int8_t ack = 0;
//    LTC2943_mode = LTC2943_AUTOMATIC_MODE|prescalar_mode|alcc_mode ;               //! Set the control register of the LTC2943 to automatic mode as well as set prescalar and AL#/CC# pin values.
    //    ack |= LTC2943_write(LTC2943_I2C_ADDRESS, LTC2943_CONTROL_REG, LTC2943_mode);   //! Writes the set mode to the LTC2943 control register

    printf("*************************\n");

    uint8_t status_code;
//	uint8_t hightemp_code, lowtemp_code;
    uint16_t charge_code, current_code, voltage_code, temperature_code;

    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_ACCUM_CHARGE_MSB_REG, &charge_code);     //! Read MSB and LSB Accumulated Charge Registers for 16 bit charge code
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_VOLTAGE_MSB_REG, &voltage_code);         //! Read MSB and LSB Voltage Registers for 16 bit voltage code
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CURRENT_MSB_REG, &current_code);         //! Read MSB and LSB Current Registers for 16 bit current code
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_TEMPERATURE_MSB_REG, &temperature_code); //! Read MSB and LSB Temperature Registers for 16 bit temperature code
    ack |= LTC2943_read(LTC2943_I2C_ADDRESS, LTC2943_STATUS_REG, &status_code);                       //! Read Status Register for 8 bit status code


    float charge, current, voltage, temperature;
    if (mAh_or_Coulombs)
    {
        charge = LTC2943_code_to_coulombs(charge_code, resistor, prescalarValue); //! Convert charge code to Coulombs if Coulomb units are desired.
        printf("Coulombs: ");
        printf("%.3f", charge);
        printf(" C\n");
    }
    else
    {
        charge = LTC2943_code_to_mAh(charge_code, resistor, prescalarValue);      //! Convert charge code to mAh if mAh units are desired.
        printf("mAh: ");
        printf("%.3f", charge);
        printf(" mAh\n");
    }


    current = LTC2943_code_to_current(current_code, resistor);                //! Convert current code to Amperes
    voltage = LTC2943_code_to_voltage(voltage_code);                          //! Convert voltage code to Volts

    printf("Current ");
    printf("%.4f", current);
    printf(" A\n");

    printf("Voltage ");
    printf("%.3f", voltage);
    printf(" V\n");


    if (celcius_or_kelvin)
    {
        temperature = LTC2943_code_to_kelvin_temperature(temperature_code);   //! Convert temperature code to kelvin
        printf("Temperature ");
        printf("%.3f", temperature);
        printf(" K\n");
    }
    else
    {
        temperature = LTC2943_code_to_celcius_temperature(temperature_code);  //! Convert temperature code to celcius
        printf("Temperature ");
        printf("%.3f", temperature);
        printf(" C\n");
    }

    checkAlerts(status_code);                                                   //! Check status code for Alerts. If an Alert has been set, print out appropriate message in the Serial Prompt.

    HAL_Delay(AUTOMATIC_MODE_DISPLAY_DELAY);                                    //! HAL_Delay for 1s before next polling
    return ack;
}

//! Scan Mode
int8_t menu_2_scan_mode(int8_t mAh_or_Coulombs , int8_t celcius_or_kelvin ,uint16_t prescalar_mode,uint16_t prescalarValue, uint16_t alcc_mode)
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int8_t LTC2943_mode;
    int8_t ack = 0;
    LTC2943_mode = LTC2943_SCAN_MODE|prescalar_mode|alcc_mode ;                           //! Set the control mode of the LTC2943 to scan mode as well as set prescalar and AL#/CC# pin values.
    ack |= LTC2943_write(LTC2943_I2C_ADDRESS, LTC2943_CONTROL_REG, LTC2943_mode);          //! Writes the set mode to the LTC2943 control register

    printf("*************************\n");

    uint8_t status_code;
    uint16_t charge_code, current_code, voltage_code, temperature_code;


    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_ACCUM_CHARGE_MSB_REG, &charge_code);         //! Read MSB and LSB Accumulated Charge Registers for 16 bit charge code
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_VOLTAGE_MSB_REG, &voltage_code);             //! Read MSB and LSB Voltage Registers for 16 bit voltage code
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CURRENT_MSB_REG, &current_code);             //! Read MSB and LSB Current Registers for 16 bit current code
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_TEMPERATURE_MSB_REG, &temperature_code);     //! Read MSB and LSB Temperature Registers for 16 bit temperature code
    ack |= LTC2943_read(LTC2943_I2C_ADDRESS, LTC2943_STATUS_REG, &status_code);                           //! Read Status Registers for 8 bit status code

    float charge, current, voltage, temperature;
    if (mAh_or_Coulombs)
    {
        charge = LTC2943_code_to_coulombs(charge_code, resistor, prescalarValue);                             //! Convert charge code to Coulombs if Coulomb units are desired.
        printf("Coulombs: ");
        printf("%.3f", charge);
        printf(" C\n");
    }
    else
    {
        charge = LTC2943_code_to_mAh(charge_code, resistor, prescalarValue);                                  //! Convert charge code to mAh if mAh units are desired.
        printf("mAh: ");
        printf("%.3f", charge);
        printf(" mAh\n");
    }


    current = LTC2943_code_to_current(current_code, resistor);                                           //! Convert current code to Amperes
    voltage = LTC2943_code_to_voltage(voltage_code);                                                     //! Convert voltage code to Volts


    printf("Current ");
    printf("%.4f", current);
    printf(" A\n");

    printf("Voltage ");
    printf("%.3f", voltage);
    printf(" V\n");


    if (celcius_or_kelvin)
    {
        temperature = LTC2943_code_to_kelvin_temperature(temperature_code);                             //! Convert temperature code to Kelvin if Kelvin units are desired.
        printf("Temperature ");
        printf("%.3f", temperature);
        printf(" K\n");
    }
    else
    {
        temperature = LTC2943_code_to_celcius_temperature(temperature_code);                           //! Convert temperature code to Celcius if Celcius units are desired.
        printf("Temperature ");
        printf("%.3f", temperature);
        printf(" C\n");
    }
    checkAlerts(status_code);                                                                          //! Check status code for Alerts. If an Alert has been set, print out appropriate message in the Serial Prompt

    HAL_Delay(SCAN_MODE_DISPLAY_DELAY);
    return ack;
}

//! Manual Mode
int8_t menu_3_manual_mode(int8_t mAh_or_Coulombs ,int8_t celcius_or_kelvin ,uint16_t prescalar_mode, uint16_t prescalarValue, uint16_t alcc_mode)
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int8_t LTC2943_mode;
    int8_t ack = 0;
    LTC2943_mode = LTC2943_MANUAL_MODE|prescalar_mode|alcc_mode ;                                    //! Set the control mode of the LTC2943 to manual mode as well as set prescalar and AL#/CC# pin values.
    ack |= LTC2943_write(LTC2943_I2C_ADDRESS, LTC2943_CONTROL_REG, LTC2943_mode);                     //! Writes the set mode to the LTC2943 control register

    int staleData = 0;                                                                                //! Stale Data Check variable. When set to 1 it indicates that stale data is being read from the voltage, current and temperature registers.

    printf("*************************\n");

    uint8_t status_code;
    uint16_t charge_code, current_code, voltage_code, temperature_code;


    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_ACCUM_CHARGE_MSB_REG, &charge_code);         //! Read MSB and LSB Accumulated Charge Registers for 16 bit charge code
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_VOLTAGE_MSB_REG, &voltage_code);             //! Read MSB and LSB Voltage Registers for 16 bit voltage code
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CURRENT_MSB_REG, &current_code);             //! Read MSB and LSB Current Registers for 16 bit current code
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_TEMPERATURE_MSB_REG, &temperature_code);     //! Read MSB and LSB Temperature Registers for 16 bit temperature code
    ack |= LTC2943_read(LTC2943_I2C_ADDRESS, LTC2943_STATUS_REG, &status_code);                           //! Read Status Registers for 8 bit status code


    float charge, current, voltage, temperature;
    if (mAh_or_Coulombs)
    {
        charge = LTC2943_code_to_coulombs(charge_code, resistor, prescalarValue);                             //! Convert charge code to Coulombs if Coulomb units are desired.
        printf("Coulombs: ");
        printf("%.3f", charge);
        printf(" C\n");
    }
    else
    {
        charge = LTC2943_code_to_mAh(charge_code, resistor, prescalarValue);                                  //! Convert charge code to mAh if mAh units are desired.
        printf("mAh: ");
        printf("%.3f", charge);
        printf(" mAh\n");
    }


    current = LTC2943_code_to_current(current_code, resistor);                                            //! Convert current code to Amperes
    voltage = LTC2943_code_to_voltage(voltage_code);                                                      //! Convert voltage code to Volts


    printf("Current ");
    printf("%.4f", current);
    printf(" A");
    if (staleData) printf("     ***** Stale Data ******\n");                                     //! If Stale data is inside the register after initial snapshot, Print Stale Data message.
    else printf("\n");

    printf("Voltage ");
    printf("%.3f", voltage);
    printf(" V");
    if (staleData) printf("     ***** Stale Data ******\n");                                     //! If Stale data is inside the register after initial snapshot, Print Stale Data message.
    else printf("\n");


    if (celcius_or_kelvin)
    {
        temperature = LTC2943_code_to_kelvin_temperature(temperature_code);                               //! Convert temperature code to Kelvin if Kelvin units are desired.
        printf("Temperature ");
        printf("%.3f", temperature);
        printf(" K");
    }
    else
    {
        temperature = LTC2943_code_to_celcius_temperature(temperature_code);                              //! Convert temperature code to Celcius if Celcius units are desired.
        printf("Temperature ");
        printf("%.3f", temperature);
        printf(" C");
    }
    if (staleData) printf("   ***** Stale Data ******\n");
    else printf("\n");

    checkAlerts(status_code);                                                                             //! Check status code for Alerts. If an Alert has been set, print out appropriate message in the Serial Prompt

    staleData = 1;
    HAL_Delay(AUTOMATIC_MODE_DISPLAY_DELAY);
    return(ack);
}

//! Sleep Mode
int8_t menu_4_sleep_mode(int8_t mAh_or_Coulombs ,uint16_t prescalar_mode, uint16_t prescalarValue, uint16_t alcc_mode)
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int8_t LTC2943_mode;
    int8_t ack = 0;
    LTC2943_mode = LTC2943_SLEEP_MODE|prescalar_mode|alcc_mode ;                            //! Set the control mode of the LTC2943 to sleep mode as well as set prescalar and AL#/CC# pin values.
    ack |= LTC2943_write(LTC2943_I2C_ADDRESS, LTC2943_CONTROL_REG, LTC2943_mode);            //! Writes the set mode to the LTC2943 control register

    printf("*************************\n");
    HAL_Delay(100);
    uint8_t status_code;
    uint16_t charge_code;


    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_ACCUM_CHARGE_MSB_REG, &charge_code);     //! Read MSB and LSB Accumulated Charge Registers for 16 bit charge code
    ack |= LTC2943_read(LTC2943_I2C_ADDRESS, LTC2943_STATUS_REG, &status_code);                       //! Read Status Registers for 8 bit status code


    float charge;
    if (mAh_or_Coulombs)
    {
        charge = LTC2943_code_to_coulombs(charge_code, resistor, prescalarValue);                         //! Convert charge code to Coulombs if Coulomb units are desired.
        printf("Coulombs: ");
        printf("%.3f", charge);
        printf(" C\n");
    }
    else
    {
        charge = LTC2943_code_to_mAh(charge_code, resistor, prescalarValue);                              //! Convert charge code to mAh if mAh units are desired.
        printf("mAh: ");
        printf("%.3f", charge);
        printf(" mAh\n");
    }

    printf("Current ");
    printf("     ADC Sleep...\n");


    printf("Voltage ");
    printf("     ADC Sleep...\n");

    printf("Temperature ");
    printf(" ADC Sleep...\n");

    checkAlerts(status_code);

    HAL_Delay(AUTOMATIC_MODE_DISPLAY_DELAY);
    return(ack);
}

//! Shutdown Mode
int8_t menu_5_shutdown_mode()
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int8_t ack = 0;
    ack |= LTC2943_write(LTC2943_I2C_ADDRESS, LTC2943_CONTROL_REG, LTC2943_SHUTDOWN_MODE);                      //! Sets the LTC2943 into shutdown mode
    printf("LTC2943 Has Been ShutDown\n");
    return(ack);
}
//! Settings Menu
int8_t menu_6_settings(uint8_t *mAh_or_Coulombs, uint8_t *celcius_or_kelvin, uint16_t *prescalar_mode, uint16_t *prescalarValue, uint16_t *alcc_mode)
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int8_t ack = 0;
    int8_t user_command = 0;

    while(1)
    {
        printf("*************************\n");
        printf("1-Set Alert Thresholds\n");
        printf("2-Get Alerts Thresholds\n");
        printf("3-Set Prescalar Value\n");
        printf("4-Set AL#/CC# Pin State\n");
        printf("5-Set Units\n");
        printf("6-Exit\n");
        printf("Enter a command: ");

        while (user_command > 6 || user_command < 1)
        {
            user_command = read_int();               //! Read user input command
        }
        if (user_command != 'm')
            printf("%d\n", user_command);
        printf("\n");
        switch (user_command)
        {
            case 1:
                ack |= menu_6_settings_menu_1_set_alert_thresholds();                                          //! Settings Menu to set Alert Thresholds
                break;

            case 2:
                ack |= menu_6_settings_menu_2_get_alerts_thresholds();                                          //! Settings Menu to set Alert Thresholds
                break;

            case 3:
                ack |= menu_6_settings_menu_3_set_prescalar_values(prescalar_mode, prescalarValue);            //! Settings Menu to set Prescalar Values
                break;

            case 4:
                ack |= menu_6_alert_menu_4_set_allcc_state(alcc_mode);                                        //! Settings Menu to set AL#/CC# mode
                break;

            case 5:
                ack |= menu_6_alert_menu_5_set_units(mAh_or_Coulombs, celcius_or_kelvin);                     //! Settings Menu to set Temperature and Charge Units
                break;

            case 6:
                return(ack);
                break;
        }
        user_command = 0;
    }
}

//! Alert Threshold Menu
int8_t menu_6_settings_menu_1_set_alert_thresholds()
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int8_t ack = 0;
    int32_t user_command = 0;

    while(1)
    {
        printf("*************************\n");
        printf("1-Set Charge Thresholds\n");
        printf("2-Set Voltage Thresholds\n");
        printf("3-Set Current Thresholds\n");
        printf("4-Set Temperature Thresholds\n");
        printf("5-Exit\n");
        printf("Enter a command: ");

        while (user_command > 5 || user_command < 1)
        {
            user_command = read_int();               //! Read user input command
        }
        if (user_command != 'm')
            printf("%d\n", (int)user_command);
        printf("\n");
        switch (user_command)
        {
            case 1:
                ack |= menu_6_alert_menu_1_set_charge_thresholds();                   //! Set Max and Min Charge Thresholds. The ACR charge lsb size changes with respect to the prescalar and sense resistor value. Due to this variability, for the purpose of this demo enter values in hexadecimal.
                break;
            case 2:
                ack |= menu_6_alert_menu_2_set_voltage_thresholds();                  //! Set Max and Min Voltage Thresholds. Enter Values in Volts
                break;
            case 3:
                ack |= menu_6_alert_menu_3_set_current_thresholds();                  //! Set Max and Min Current Thresholds. Enter Values in Amperes.
                break;
            case 4:
                ack |= menu_6_alert_menu_4_set_temperature_thresholds();              //! Set Max and Min Temperature Thresholds. Enter Values in Celcius.
                break;
            case 5:
                return(ack);
                break;
        }
        user_command = 0;
    }
}

//! Set Charge Threshold Function
int8_t menu_6_alert_menu_1_set_charge_thresholds()
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int user_command = 0;
    float user_commandf = 0;
    int8_t ack = 0;
    float max_charge_threshold = 0;
    float min_charge_threshold = 0;
    uint16_t max_charge_threshold_code = 0;
    uint16_t min_charge_threshold_code = 0;

    while(1)
    {
        ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CHARGE_THRESH_HIGH_MSB_REG, &max_charge_threshold_code);
        ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CHARGE_THRESH_LOW_MSB_REG, &min_charge_threshold_code);

        printf("Actual values :\n");
        printf("    Max = %.2f mAh\n", LTC2943_code_to_mAh(max_charge_threshold_code, resistor, prescalarValue));
        printf("    Min = %.2f mAh\n\n", LTC2943_code_to_mAh(min_charge_threshold_code, resistor, prescalarValue));

        printf("1-Change max charge threshold\n");
        printf("2-Change min charge threshold\n");
        printf("3-Exit\n");
        printf("Enter a command: ");

        while (user_command > 3 || user_command < 1)
        {
            user_command = read_int();               //! Read user input command
        }
        if (user_command != 'm')
            printf("%d\n", user_command);
        printf("\n");
        switch (user_command)
        {
            case 1:
                printf("Enter Max Charge Threshold (mAh)\n");

                user_commandf = 0;
                while (user_commandf < 1)
                {
                    user_commandf = read_float();               //! Read user input command
                }
                max_charge_threshold = user_commandf;
                //! Read user entered value
                printf("Max Charge Threshold : %.2f mAh\n\n", max_charge_threshold);

                max_charge_threshold_code = LTC2943_mAh_to_code(max_charge_threshold, resistor, prescalarValue);

                ack |= LTC2943_write_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CHARGE_THRESH_HIGH_MSB_REG, max_charge_threshold_code);    //! write user entered value to HIGH threshold register
                break;
            case 2:
                printf("Enter Min Charge Threshold\n");

                user_commandf = 0;
                while (user_commandf < 1)
                {
                    user_commandf = read_float();               //! Read user input command
                }
                min_charge_threshold = user_commandf;
                //! Read user entered value
                printf("Min Charge Threshold : %.2f mAh\n\n", min_charge_threshold);

                min_charge_threshold_code = LTC2943_mAh_to_code(min_charge_threshold, resistor, prescalarValue);

                ack |= LTC2943_write_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CHARGE_THRESH_LOW_MSB_REG, min_charge_threshold_code);     //! write user entered value to HIGH threshold register
                return(ack);
                break;
            case 3:
                return(ack);
                break;
        }
        user_command = 0;
    }
}

//! Set Voltage Thresholds
int8_t menu_6_alert_menu_2_set_voltage_thresholds()
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int user_command = 0;
    float user_commandf = 0;
    int8_t ack = 0;
    float max_voltage_threshold = 0;
    float min_voltage_threshold = 0;
    uint16_t max_voltage_threshold_code = 0;
    uint16_t min_voltage_threshold_code = 0;

    while(1)
    {
        ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_VOLTAGE_THRESH_HIGH_MSB_REG, (uint16_t*)&max_voltage_threshold_code);
        ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_VOLTAGE_THRESH_LOW_MSB_REG, (uint16_t*)&min_voltage_threshold_code);

        printf("Actual values :\n");
        printf("    Max = %.2f V\n", LTC2943_code_to_voltage(max_voltage_threshold_code));
        printf("    Min = %.2f V\n\n", LTC2943_code_to_voltage(min_voltage_threshold_code));

        printf("1-Change max voltage threshold\n");
        printf("2-Change min voltage threshold\n");
        printf("3-Exit\n");
        printf("Enter a command: ");

        while (user_command > 3 || user_command < 1)
        {
            user_command = read_int();               //! Read user input command
        }
        if (user_command != 'm')
            printf("%d\n", user_command);
        printf("\n");
        switch (user_command)
        {
            case 1:
                printf("Enter Max Voltage Threshold (V)\n");

                user_commandf = 0;
                while (user_commandf < 0.0001)
                {
                    user_commandf = read_float();               //! Read user input command
                }
                max_voltage_threshold = user_commandf;
                printf("Max Voltage Threshold: %.2f V\n\n", max_voltage_threshold);

                max_voltage_threshold_code = LTC2943_voltage_to_code(max_voltage_threshold);                         //! Convert user entered voltage into adc code.

                ack |= LTC2943_write_16_bits(LTC2943_I2C_ADDRESS, LTC2943_VOLTAGE_THRESH_HIGH_MSB_REG, max_voltage_threshold_code);       //! Write adc code to HIGH threshold register
                break;
            case 2:
                printf("Enter Min Voltage Threshold\n");

                user_commandf = 0;
                while (user_commandf < 0.0001)
                {
                    user_commandf = read_float();               //! Read user input command
                }
                min_voltage_threshold = user_commandf;
                printf("Min Voltage Threshold: %.2f V\n\n", min_voltage_threshold);

                min_voltage_threshold_code = LTC2943_voltage_to_code(min_voltage_threshold);                         //! Convert user entered voltage into adc code.

                ack |= LTC2943_write_16_bits(LTC2943_I2C_ADDRESS, LTC2943_VOLTAGE_THRESH_LOW_MSB_REG, min_voltage_threshold_code);        //! Write adc code to LOW threshold register
                return(ack);
                break;
            case 3:
                return(ack);
                break;
        }
        user_command = 0;
    }
}
//! Set Current Thresholds
int8_t menu_6_alert_menu_3_set_current_thresholds()
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int user_command = 0;
    float user_commandf = 0;
    int8_t ack = 0;
    float max_current_threshold = 0;
    float min_current_threshold = 0;
    uint16_t max_current_threshold_code = 0;
    uint16_t min_current_threshold_code = 0;

    while(1)
    {
        ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CURRENT_THRESH_HIGH_MSB_REG, &max_current_threshold_code);
        ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CURRENT_THRESH_LOW_MSB_REG, &min_current_threshold_code);

        printf("Actual values :\n");
        printf("    Max = %.3f A\n", LTC2943_code_to_current(max_current_threshold_code, resistor));
        printf("    Min = %.3f A\n\n", LTC2943_code_to_current(min_current_threshold_code, resistor));

        printf("1-Change max current threshold\n");
        printf("2-Change min current threshold\n");
        printf("3-Exit\n");
        printf("Enter a command: ");

        while (user_command > 3 || user_command < 1)
        {
            user_command = read_int();               //! Read user input command
        }
        if (user_command != 'm')
            printf("%d\n", user_command);
        printf("\n");
        switch (user_command)
        {
            case 1:
                printf("Enter Max Current Threshold (A)\n");

                user_commandf = 0;
                while (fabs(user_commandf) < 0.00001)
                {
                    user_commandf = read_float();               //! Read user input command
                }
                max_current_threshold = user_commandf;
                printf("Max Current Threshold: %.3f A\n\n", max_current_threshold);

                max_current_threshold_code = LTC2943_current_to_code(max_current_threshold, resistor);      //! Convert user entered current into adc code.

                ack |= LTC2943_write_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CURRENT_THRESH_HIGH_MSB_REG, max_current_threshold_code);      //! Write adc code to HIGH threshold register
                break;
            case 2:
                printf("Enter Min Current Threshold\n");

                user_commandf = 0;
                while (fabs(user_commandf) < 0.00001)
                {
                    user_commandf = read_float();               //! Read user input command
                }
                min_current_threshold = user_commandf;
                printf("Min Current Threshold: %.3f A\n\n", min_current_threshold);

                min_current_threshold_code = LTC2943_current_to_code(min_current_threshold, resistor);      //! Convert user entered current into adc code.

                ack |= LTC2943_write_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CURRENT_THRESH_LOW_MSB_REG, min_current_threshold_code);       //! Write adc code to LOW threshold register
                return(ack);
                break;
            case 3:
                return(ack);
                break;
        }
        user_command = 0;
    }
}

//! Set Temperature Thresholds
int8_t menu_6_alert_menu_4_set_temperature_thresholds()
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int user_command = 0;
    float user_commandf = 0;
    int8_t ack = 0;
    float max_temperature_threshold = 0;
    float min_temperature_threshold = 0;
    uint16_t max_temperature_threshold_code = 0;
    uint16_t min_temperature_threshold_code = 0;

    while(1)
    {
        ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_TEMPERATURE_THRESH_HIGH_REG, &max_temperature_threshold_code);
        ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_TEMPERATURE_THRESH_LOW_REG, &min_temperature_threshold_code);

        printf("Actual values :\n");
        printf("    Max = %.1f C\n", LTC2943_code_to_celcius_temperature(max_temperature_threshold_code));
        printf("    Min = %.1f C\n\n", LTC2943_code_to_celcius_temperature(min_temperature_threshold_code));

        printf("1-Change max temperature threshold\n");
        printf("2-Change min temperature threshold\n");
        printf("3-Exit\n");
        printf("Enter a command: ");

        while (user_command > 3 || user_command < 1)
        {
            user_command = read_int();               //! Read user input command
        }
        if (user_command != 'm')
            printf("%d\n", user_command);
        printf("\n");
        switch (user_command)
        {
            case 1:
                printf("Enter Max Temperature Threshold in Celcius\n");

                user_commandf = 0;
                while (fabs(user_commandf) < 0.0001)
                {
                    user_commandf = read_float();               //! Read user input command
                }
                max_temperature_threshold = user_commandf;
                printf("Max Temperature Threshold in Celcius: %.1f C\n\n", max_temperature_threshold);

                max_temperature_threshold_code = LTC2943_celcius_temperature_to_code(max_temperature_threshold);   //! Convert user entered temperature into adc code.


                ack |= LTC2943_write_16_bits(LTC2943_I2C_ADDRESS, LTC2943_TEMPERATURE_THRESH_HIGH_REG, max_temperature_threshold_code);    //! Write adc code to HIGH threshold register
                break;
            case 2:
                printf("Enter Min Temperature Threshold in Celcius\n");

                user_commandf = 0;
                while (fabs(user_commandf) < 0.0001)
                {
                    user_commandf = read_float();               //! Read user input command
                }
                min_temperature_threshold = user_commandf;
                printf("Min Temperature Threshold in Celcius: %.1f C\n\n", min_temperature_threshold);

                min_temperature_threshold_code = LTC2943_celcius_temperature_to_code(min_temperature_threshold);  //! Convert user entered temperature into adc code.

                ack |= LTC2943_write_16_bits(LTC2943_I2C_ADDRESS, LTC2943_TEMPERATURE_THRESH_LOW_REG, min_temperature_threshold_code);    //! Write adc code to LOW threshold register
                return(ack);
                break;
            case 3:
                return(ack);
                break;
        }
        user_command = 0;
    }
}

//! Get Thresholds Menu
int8_t menu_6_settings_menu_2_get_alerts_thresholds(void)
{
//    int user_command = 0;
//    float user_commandf = 0;
    int8_t ack = 0;
    float max_charge_threshold = 0;
    float min_charge_threshold = 0;
    uint16_t max_charge_threshold_code = 0;
    uint16_t min_charge_threshold_code = 0;
//    float max_voltage_threshold = 0;
//    float min_voltage_threshold = 0;
    uint16_t max_voltage_threshold_code = 0;
    uint16_t min_voltage_threshold_code = 0;
//    float max_current_threshold = 0;
//    float min_current_threshold = 0;
    uint16_t max_current_threshold_code = 0;
    uint16_t min_current_threshold_code = 0;
//    float max_temperature_threshold = 0;
//    float min_temperature_threshold = 0;
    uint16_t max_temperature_threshold_code = 0;
    uint16_t min_temperature_threshold_code = 0;

    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CHARGE_THRESH_HIGH_MSB_REG, &max_charge_threshold_code);
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CHARGE_THRESH_LOW_MSB_REG, &min_charge_threshold_code);

    max_charge_threshold = LTC2943_code_to_mAh(max_charge_threshold_code, resistor, prescalarValue);      //! Convert charge code to mAh.
    min_charge_threshold = LTC2943_code_to_mAh(min_charge_threshold_code, resistor, prescalarValue);      //! Convert charge code to mAh.

    printf("Charge Thresholds (mAh) :\n");
    printf("    Max = %.2f mAh\n", max_charge_threshold);
    printf("    Min = %.2f mAh\n\n", min_charge_threshold);

    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_VOLTAGE_THRESH_HIGH_MSB_REG, (uint16_t*)&max_voltage_threshold_code);
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_VOLTAGE_THRESH_LOW_MSB_REG, (uint16_t*)&min_voltage_threshold_code);

    printf("Voltage Thresholds (V) :\n");
    printf("    Max = %.2f V\n", LTC2943_code_to_voltage(max_voltage_threshold_code));
    printf("    Min = %.2f V\n\n", LTC2943_code_to_voltage(min_voltage_threshold_code));

    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CURRENT_THRESH_HIGH_MSB_REG, &max_current_threshold_code);
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_CURRENT_THRESH_LOW_MSB_REG, &min_current_threshold_code);

    printf("Current Thresholds (A) :\n");
    printf("    Max = %.4f A\n", LTC2943_code_to_current(max_current_threshold_code, resistor));
    printf("    Min = %.4f A\n\n", LTC2943_code_to_current(min_current_threshold_code, resistor));

    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_TEMPERATURE_THRESH_HIGH_REG, &max_temperature_threshold_code);
    ack |= LTC2943_read_16_bits(LTC2943_I2C_ADDRESS, LTC2943_TEMPERATURE_THRESH_LOW_REG, &min_temperature_threshold_code);

    printf("Actual values :\n");
    printf("    Max = %.1f C\n", LTC2943_code_to_celcius_temperature(max_temperature_threshold_code));
    printf("    Min = %.1f C\n\n", LTC2943_code_to_celcius_temperature(min_temperature_threshold_code));
    return(ack);
}

//! Prescalar Menu
int8_t menu_6_settings_menu_3_set_prescalar_values(uint16_t *prescalar_mode, uint16_t *prescalarValue)
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    uint32_t user_command = 0;
    int8_t ack = 0;

    while(1)
    {
        printf("*************************\n");
        printf("1-Set Prescalar M = 1\n");
        printf("2-Set Prescalar M = 4\n");
        printf("3-Set Prescalar M = 16\n");
        printf("4-Set Prescalar M = 64\n");
        printf("5-Set Prescalar M = 256\n");
        printf("6-Set Prescalar M = 1024\n");
        printf("7-Set Prescalar M = 4096\n");
        printf("8-Exit\n");
        printf("Enter a command: ");

        while (user_command > 8 || user_command < 1)
        {
            user_command = read_int();               //! Read user input command
        }
        if (user_command != 'm')
            printf("%d\n", (int)user_command);
        printf("\n");
        switch (user_command)
        {
            case 1:
                *prescalar_mode = LTC2943_PRESCALAR_M_1;                                   //! Set Prescalar Value M = 1
                *prescalarValue = 1;
                printf("\nPrescalar Set to 1\n");
                break;
            case 2:
                *prescalar_mode = LTC2943_PRESCALAR_M_4;                                  //! Set Prescalar Value M = 4
                *prescalarValue = 4;
                printf("\nPrescalar Set to 4\n");
                break;
            case 3:
                *prescalar_mode = LTC2943_PRESCALAR_M_16;                                 //! Set Prescalar Value M = 16
                *prescalarValue = 16;
                printf("\nPrescalar Set to 16\n");
                break;
            case 4:
                *prescalar_mode = LTC2943_PRESCALAR_M_64;                                //! Set Prescalar Value M = 64
                *prescalarValue = 64;
                printf("\nPrescalar Set to 64\n");
                break;
            case 5:
                *prescalar_mode = LTC2943_PRESCALAR_M_256;                               //! Set Prescalar Value M = 256
                *prescalarValue = 256;
                printf("\nPrescalar Set to 256\n");
                break;
            case 6:
                *prescalar_mode = LTC2943_PRESCALAR_M_1024;                              //! Set Prescalar Value M = 1024
                *prescalarValue = 1024;
                \
                printf("\nPrescalar Set to 1024\n");
                break;
            case 7:
                *prescalar_mode = LTC2943_PRESCALAR_M_4096;                              //! Set Prescalar Value M = 4096
                *prescalarValue = 4096;
                printf("\nPrescalar Set to 4096\n");
                break;
            case 8:
                return(ack);
                break;
        }
        user_command = 0;
    }
}


//! AL#/CC# Pin Mode Menu
uint8_t menu_6_alert_menu_4_set_allcc_state(uint16_t *alcc_mode)
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int8_t ack = 0;
    int32_t user_command;

    while(1)
    {
        printf("*************************\n");
        printf("1-Enable Alert Mode\n");
        printf("2-Enable Charge Complete Mode\n");
        printf("3-Disable AL#/CC# Pin\n");
        printf("4-Exit\n");
        printf("Enter a command: ");

        while (user_command > 4 || user_command < 1)
        {
            user_command = read_int();               //! Read user input command
        }
        if (user_command != 'm')
            printf("%d\n", (int)user_command);
        printf("\n");
        switch (user_command)
        {
            case 1:
                *alcc_mode = LTC2943_ALERT_MODE;                         //! Set AL#/CC# mode to Alert Mode
                printf("\nAlert Mode Enabled\n");
                break;
            case 2:
                *alcc_mode = LTC2943_CHARGE_COMPLETE_MODE;               //! Set AL#/CC# mode to Charge Complete Mode
                printf("\nCharge Mode Enabled\n");
                break;
            case 3:
                *alcc_mode = LTC2943_DISABLE_ALCC_PIN;                   //! Disable AL#/CC# pin.
                printf("\nAL#/CC# Pin Disabled\n");
                break;
            case 4:
                return(ack);
                break;
        }
        user_command = 0;
    }
}

//! Set Charge and Temperature Units Menu
uint8_t menu_6_alert_menu_5_set_units(uint8_t *mAh_or_Coulombs, uint8_t *celcius_or_kelvin)
//! @return Returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge
{
    int8_t ack = 0;
    int32_t user_command;

    while(1)
    {
        printf("*************************\n");
        printf("1-Set Charge Units to mAh\n");
        printf("2-Set Charge Units to Coulombs\n");
        printf("3-Set Temperature Units to Celsius\n");
        printf("4-Set Temperature Units to Kelvin\n");
        printf("5-Exit\n");
        printf("Enter a command: ");

        while (user_command > 4 || user_command < 1)
        {
            user_command = read_int();               //! Read user input command
        }
        if (user_command != 'm')
            printf("%d\n", (int)user_command);
        printf("\n");
        switch (user_command)
        {
            case 1:
                *mAh_or_Coulombs = 0;
                printf("\nCharge Units Set to mAh\n");
                break;
            case 2:
                *mAh_or_Coulombs = 1;
                printf("\nCharge Units Set to Coulombs\n");
                break;
            case 3:
                *celcius_or_kelvin = 0;
                printf("\nTemperature Units Set to Celcius\n");
                break;
            case 4:
                *celcius_or_kelvin = 1;
                printf("\nTemperature Units Set to Kelvin\n");
                break;
            case 5:
                return(ack);
                break;
        }
        user_command = 0;
    }
}

//! Checks to see if a bit in a certain position is set.
bool isBitSet(uint8_t value, uint8_t position)
//! @return Returns the state of a bit at "position" in a byte. 1 = Set, 0 = Not Set
{
    return((1<<position)&value);
}
//! Check Alerts Function - Checks to see if an alert has been set in the status register. If an alert has been set, it prints out the appropriate message.
void checkAlerts(uint8_t status_code)
//! @return
{
    if (isBitSet(status_code,6))
    {
        printf("\n***********************\n");
        printf("Alert: ");
        printf("Current Alert\n");
        printf("***********************\n");
    }
    if (isBitSet(status_code,5))
    {
        printf("\n***********************\n");
        printf("Alert: ");
        printf("Charge Over/Under Flow Alert\n");
        printf("***********************\n");
    }
    if (isBitSet(status_code,4))
    {
        printf("\n***********************\n");
        printf("Alert: ");
        printf("Temperature Alert\n");
        printf("***********************\n");
    }
    if (isBitSet(status_code,3))
    {
        printf("\n***********************\n");
        printf("Alert: ");
        printf("Charge High Alert\n");
        printf("***********************\n");
    }
    if (isBitSet(status_code,2))
    {
        printf("\n***********************\n");
        printf("Alert: ");
        printf("Charge Low Alert\n");
        printf("***********************\n");
    }
    if (isBitSet(status_code,1))
    {
        printf("\n***********************\n");
        printf("Alert: ");
        printf("Voltage Alert\n");
        printf("***********************\n");
    }
    if (isBitSet(status_code,0))
    {
        printf("\n***********************\n");
        printf("Alert: ");
        printf("UVLO Alert\n");
        printf("***********************\n");
    }
}

