/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define LED1_Pin GPIO_PIN_2
#define LED1_GPIO_Port GPIOE
#define LED2_Pin GPIO_PIN_3
#define LED2_GPIO_Port GPIOE
#define LED3_Pin GPIO_PIN_4
#define LED3_GPIO_Port GPIOE
#define LED4_Pin GPIO_PIN_5
#define LED4_GPIO_Port GPIOE
#define WKUP_Pin GPIO_PIN_13
#define WKUP_GPIO_Port GPIOC
#define FLOOR_ADC_R_EXT_Pin GPIO_PIN_0
#define FLOOR_ADC_R_EXT_GPIO_Port GPIOC
#define FLOOR_ADC_R_Pin GPIO_PIN_1
#define FLOOR_ADC_R_GPIO_Port GPIOC
#define EXT_ADC1_Pin GPIO_PIN_2
#define EXT_ADC1_GPIO_Port GPIOC
#define EXT_ADC2_Pin GPIO_PIN_3
#define EXT_ADC2_GPIO_Port GPIOC
#define MOT_R_HALL1_Pin GPIO_PIN_0
#define MOT_R_HALL1_GPIO_Port GPIOA
#define MOT_R_HALL2_Pin GPIO_PIN_1
#define MOT_R_HALL2_GPIO_Port GPIOA
#define MOT_R_HALL3_Pin GPIO_PIN_2
#define MOT_R_HALL3_GPIO_Port GPIOA
#define BEEPER_Pin GPIO_PIN_3
#define BEEPER_GPIO_Port GPIOA
#define FLOOR_ADC_L_EXT_Pin GPIO_PIN_4
#define FLOOR_ADC_L_EXT_GPIO_Port GPIOA
#define FLOOR_ADC_L_Pin GPIO_PIN_5
#define FLOOR_ADC_L_GPIO_Port GPIOA
#define MOT_R_EN1_Pin GPIO_PIN_7
#define MOT_R_EN1_GPIO_Port GPIOA
#define PW_HALL_EN_Pin GPIO_PIN_2
#define PW_HALL_EN_GPIO_Port GPIOB
#define MOT_L_EN1_Pin GPIO_PIN_8
#define MOT_L_EN1_GPIO_Port GPIOE
#define MOT_L_IN1_Pin GPIO_PIN_9
#define MOT_L_IN1_GPIO_Port GPIOE
#define MOT_L_EN2_Pin GPIO_PIN_10
#define MOT_L_EN2_GPIO_Port GPIOE
#define MOT_L_IN2_Pin GPIO_PIN_11
#define MOT_L_IN2_GPIO_Port GPIOE
#define MOT_L_EN3_Pin GPIO_PIN_12
#define MOT_L_EN3_GPIO_Port GPIOE
#define MOT_L_IN3_Pin GPIO_PIN_13
#define MOT_L_IN3_GPIO_Port GPIOE
#define TOF_R_SCL_Pin GPIO_PIN_10
#define TOF_R_SCL_GPIO_Port GPIOB
#define TOF_R_SDA_Pin GPIO_PIN_11
#define TOF_R_SDA_GPIO_Port GPIOB
#define MOT_R_EN2_Pin GPIO_PIN_14
#define MOT_R_EN2_GPIO_Port GPIOB
#define MOT_R_EN3_Pin GPIO_PIN_15
#define MOT_R_EN3_GPIO_Port GPIOB
#define FUEL_GAUGE_ALCC_Pin GPIO_PIN_11
#define FUEL_GAUGE_ALCC_GPIO_Port GPIOD
#define MOT_L_HALL1_Pin GPIO_PIN_12
#define MOT_L_HALL1_GPIO_Port GPIOD
#define MOT_L_HALL2_Pin GPIO_PIN_13
#define MOT_L_HALL2_GPIO_Port GPIOD
#define MOT_L_HALL3_Pin GPIO_PIN_14
#define MOT_L_HALL3_GPIO_Port GPIOD
#define MOT_R_IN1_Pin GPIO_PIN_6
#define MOT_R_IN1_GPIO_Port GPIOC
#define MOT_R_IN2_Pin GPIO_PIN_7
#define MOT_R_IN2_GPIO_Port GPIOC
#define MOT_R_IN3_Pin GPIO_PIN_8
#define MOT_R_IN3_GPIO_Port GPIOC
#define TOF_L_SDA_Pin GPIO_PIN_9
#define TOF_L_SDA_GPIO_Port GPIOC
#define TOF_L_SCL_Pin GPIO_PIN_8
#define TOF_L_SCL_GPIO_Port GPIOA
#define BLUETOOTH_TX_Pin GPIO_PIN_9
#define BLUETOOTH_TX_GPIO_Port GPIOA
#define BLUETOOTH_RX_Pin GPIO_PIN_10
#define BLUETOOTH_RX_GPIO_Port GPIOA
#define GYRO_CS_Pin GPIO_PIN_15
#define GYRO_CS_GPIO_Port GPIOA
#define GYRO_SCK_Pin GPIO_PIN_10
#define GYRO_SCK_GPIO_Port GPIOC
#define GYRO_MISO_Pin GPIO_PIN_11
#define GYRO_MISO_GPIO_Port GPIOC
#define GYRO_MOSI_Pin GPIO_PIN_12
#define GYRO_MOSI_GPIO_Port GPIOC
#define PW_KILL_Pin GPIO_PIN_0
#define PW_KILL_GPIO_Port GPIOD
#define TOF_EN_LS_Pin GPIO_PIN_2
#define TOF_EN_LS_GPIO_Port GPIOD
#define TOF_EN_LD_Pin GPIO_PIN_3
#define TOF_EN_LD_GPIO_Port GPIOD
#define TOF_EN_LF_Pin GPIO_PIN_4
#define TOF_EN_LF_GPIO_Port GPIOD
#define TOF_EN_RS_Pin GPIO_PIN_5
#define TOF_EN_RS_GPIO_Port GPIOD
#define TOF_EN_RD_Pin GPIO_PIN_6
#define TOF_EN_RD_GPIO_Port GPIOD
#define TOF_EN_RF_Pin GPIO_PIN_7
#define TOF_EN_RF_GPIO_Port GPIOD
#define FLOOR_EN_Pin GPIO_PIN_4
#define FLOOR_EN_GPIO_Port GPIOB
#define OLED_SCL_Pin GPIO_PIN_6
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_7
#define OLED_SDA_GPIO_Port GPIOB
#define FUEL_GAUGE_SCL_Pin GPIO_PIN_8
#define FUEL_GAUGE_SCL_GPIO_Port GPIOB
#define FUEL_GAUGE_SDA_Pin GPIO_PIN_9
#define FUEL_GAUGE_SDA_GPIO_Port GPIOB
#define USER_SW1_Pin GPIO_PIN_0
#define USER_SW1_GPIO_Port GPIOE
#define USER_SW2_Pin GPIO_PIN_1
#define USER_SW2_GPIO_Port GPIOE

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
