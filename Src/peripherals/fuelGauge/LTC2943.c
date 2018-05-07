/*!
LTC2943: Multicell Battery Gas Gauge with Temperature, Voltage and Current Measurement.
LTC2943-1: Multicell Battery Gas Gauge with Temperature, Voltage and Current Measurement.

@verbatim

The LTC2943 measures battery charge state, battery voltage,
battery current and its own temperature in portable
product applications. The wide input voltage range allows
use with multicell batteries up to 20V. A precision coulomb
counter integrates current through a sense resistor between
the battery’s positive terminal and the load or charger.
Voltage, current and temperature are measured with an
internal 14-bit No Latency ΔΣ™ ADC. The measurements
are stored in internal registers accessible via the onboard
I2C/SMBus Interface

@endverbatim

http://www.linear.com/product/LTC2943
http://www.linear.com/product/LTC2943-1

http://www.linear.com/product/LTC2943#demoboards
http://www.linear.com/product/LTC2943-1#demoboards

REVISION HISTORY
$Revision: 5672 $
$Date: 2016-09-02 11:42:55 -0700 (Fri, 02 Sep 2016) $

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

//! @defgroup LTC2943 LTC2943: Multicell Battery Gas Gauge with Temperature, Voltage and Current Measurement

/*! @file
   @ingroup LTC2943
   Library for LTC2943 Multicell Battery Gas Gauge with Temperature, Voltage and Current Measurement
 */

#include "stm32h7xx_hal.h"
#include "stdio.h"
#include "peripherals/fuelGauge/LTC2943.h"

extern I2C_HandleTypeDef hi2c4;

// Write an 8-bit code to the LTC2943.
int8_t LTC2943_write(uint8_t i2c_address, uint8_t adc_command, uint8_t code)
// The function returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
{
    int32_t ack = 0;
    uint8_t aTxbuffer[2];

    aTxbuffer[0] = adc_command;
    aTxbuffer[1] = code;

    while (HAL_I2C_GetState(&hi2c4) != HAL_I2C_STATE_READY);
    ack = HAL_I2C_Master_Transmit(&hi2c4, i2c_address, aTxbuffer, 2, 1000);
    return(ack);
}


// Write a 16-bit code to the LTC2943.
int8_t LTC2943_write_16_bits(uint8_t i2c_address, uint8_t adc_command, uint16_t code)
// The function returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
{
    int8_t ack;
    uint8_t *p = (uint8_t *)&code;
    uint8_t tmp;

    tmp = p[0];
    p[0] = p[1];
    p[1] = tmp;

    while (HAL_I2C_GetState(&hi2c4) != HAL_I2C_STATE_READY);
    ack = HAL_I2C_Mem_Write(&hi2c4, i2c_address, (uint16_t)adc_command, 1, (uint8_t*)&code, 2, 1000);
    return(ack);
}

// Reads an 8-bit adc_code from LTC2943
int8_t LTC2943_read(uint8_t i2c_address, uint8_t adc_command, uint8_t *adc_code)
// The function returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
{
    int32_t ack;

    while (HAL_I2C_GetState(&hi2c4) != HAL_I2C_STATE_READY);
    ack = HAL_I2C_Mem_Read(&hi2c4, i2c_address, adc_command, 1, adc_code, 1, 1000);

    return(ack);
}

// Reads a 16-bit adc_code from LTC2943
int8_t LTC2943_read_16_bits(uint8_t i2c_address, uint8_t adc_command, uint16_t *adc_code)
// The function returns the state of the acknowledge bit after the I2C address write. 0=acknowledge, 1=no acknowledge.
{
    int32_t ack;
    uint8_t *p = (uint8_t *)adc_code;
    uint8_t tmp;

    while (HAL_I2C_GetState(&hi2c4) != HAL_I2C_STATE_READY);
    ack = HAL_I2C_Mem_Read(&hi2c4, i2c_address, (uint16_t)adc_command, 1, (uint8_t *)adc_code, 2, 1000);

    tmp = p[0];
    p[0] = p[1];
    p[1] = tmp;

    return(ack);
}


float LTC2943_code_to_coulombs(uint16_t adc_code, float resistor, uint16_t prescalar)
// The function converts the 16-bit RAW adc_code to Coulombs
{
    float coulomb_charge;
    coulomb_charge =  1000*(float)(adc_code*LTC2943_CHARGE_lsb*prescalar*50E-3)/(resistor*4096);
    coulomb_charge = coulomb_charge*3.6f;
    return(coulomb_charge);
}

float LTC2943_code_to_mAh(uint16_t adc_code, float resistor, uint16_t prescalar )
// The function converts the 16-bit RAW adc_code to mAh
{
    float mAh_charge;
    mAh_charge = 1000 * (float)(adc_code * LTC2943_CHARGE_lsb * prescalar * 50E-3)/(resistor * 4096);
    return(mAh_charge);
}

uint16_t LTC2943_mAh_to_code(float mAh_charge, float resistor, uint16_t prescalar)
// The function converts mAh to the 16-bit RAW adc_code
{
    uint16_t adc_code;

    //qLSB = 0.340mAh • (50mΩ/R_SENSE) • (M/4096)

    float ba = ((resistor*4096.0) / (LTC2943_CHARGE_lsb*prescalar*50E-3));
    float tot = (ba * (mAh_charge / 1000.0));
    adc_code = (uint16_t)(tot);

    return(adc_code);
}

float LTC2943_code_to_voltage(uint16_t adc_code)
// The function converts the 16-bit RAW adc_code to Volts
{
    float voltage;
    voltage = ((float)adc_code / (65535)) * LTC2943_FULLSCALE_VOLTAGE;
    return(voltage);
}

uint16_t LTC2943_voltage_to_code(float voltage)
// The function converts the Volts to 16-bit RAW adc_code
{
    uint16_t adc_code;
    adc_code = voltage * (0xFFFF) / (LTC2943_FULLSCALE_VOLTAGE);
    return(adc_code);
}

float LTC2943_code_to_current(uint16_t adc_code, float resistor)
// The function converts the 16-bit RAW adc_code to Amperes
{
    float current;
    current = (((float)adc_code-32767) / (32767)) * ((float)(LTC2943_FULLSCALE_CURRENT)/resistor);
    return(current);
}

uint16_t LTC2943_current_to_code(float current, float resistor)
{
    uint16_t adc_code;
    adc_code = resistor * current * (0x7FFF) / (LTC2943_FULLSCALE_CURRENT) + 0x7FFF;
    return(adc_code);
}

float LTC2943_code_to_kelvin_temperature(uint16_t adc_code)
// The function converts the 16-bit RAW adc_code to Kelvin
{
    float temperature;
    temperature = adc_code*((float)(LTC2943_FULLSCALE_TEMPERATURE)/65535);
    return(temperature);
}

float LTC2943_code_to_celcius_temperature(uint16_t adc_code)
// The function converts the 16-bit RAW adc_code to Celcius
{
    float temperature;
    temperature = adc_code*((float)(LTC2943_FULLSCALE_TEMPERATURE)/65535) - 273.15;
    return(temperature);
}

uint16_t LTC2943_celcius_temperature_to_code(float temperature)
{
    uint16_t adc_code;
    adc_code = (temperature + 273.15) * (0xFFFF) / (LTC2943_FULLSCALE_TEMPERATURE);
    return(adc_code);
}

// Used to set and clear bits in a control register.  bits_to_set will be bitwise OR'd with the register.
// bits_to_clear will be inverted and bitwise AND'd with the register so that every location with a 1 will result in a 0 in the register.
int8_t LTC2943_register_set_clear_bits(uint8_t i2c_address, uint8_t register_address, uint8_t bits_to_set, uint8_t bits_to_clear)
{
    uint8_t register_data;
    int8_t ack = 0;

    while (HAL_I2C_GetState(&hi2c4) != HAL_I2C_STATE_READY);
    ack |= LTC2943_read(i2c_address, register_address, &register_data);
    register_data = register_data & (~bits_to_clear);
    register_data = register_data | bits_to_set;
    while (HAL_I2C_GetState(&hi2c4) != HAL_I2C_STATE_READY);
    ack |= LTC2943_write(i2c_address, register_address, register_data);
    return(ack);
}



