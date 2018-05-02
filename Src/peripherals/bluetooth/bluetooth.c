/**************************************************************************/
/*!
 @file    bluetooth.c
 @author  PLF (PACABOT)
 @date    29 April 2018
 */
/**************************************************************************/
/* STM32 hal library declarations */
#include "stm32h7xx_hal.h"

/* General declarations */
#include "config/basetypes.h"
#include "config/config.h"
#include "config/errors.h"

#include <stdarg.h>
#include <string.h>

/* Application declarations */

/* Middleware declarations */

/* Peripheral declarations */

/* Declarations for this module */
#include "peripherals/bluetooth/bluetooth.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define BLUETOOTH_BUFFER_SIZE 512

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
// Array of possible baud rates
//static const int baudrates[] = { 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800,
//        //921600,
//        -0x7FFFFFFF             // Indicates the end of the array
//};
//static int BTBaudrate = 115200; // This variable is used to change the baudrate
//static presetParam BTpresetBaudRate = { (void *) &BTBaudrate, (void *) baudrates, bluetoothSetBaudrate };

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
static inline int bluetoothInit_IT(void)
{
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);

    return HAL_OK;
}

static inline int bluetoothDeInit_IT(void)
{
    __HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);

    huart1.gState = HAL_UART_STATE_READY;

    return HAL_OK;
}

/* Public functions -----------------------------------------------------------*/
void bluetoothInit(void)
{
    // Allow Remote Escape Sequence
    bluetoothCmd("AT+AB Config RmtEscapeSequence = true");

    // Reset the bluetooth peripheral
    bluetoothCmd("AT+AB Reset");

    // Enable USART Interrupts
    bluetoothEnable();
}

void bluetoothEnable(void)
{
    // Enable bluetooth Interrupts
    bluetoothInit_IT();
}

void bluetoothDisable(void)
{
    // Disable bluetooth interrupts
    bluetoothDeInit_IT();
}

int bluetoothSend(unsigned char *data, int length)
{
    return HAL_UART_Transmit_DMA(&huart1, data, length);
}

int bluetoothReceive(unsigned char *data, int length)
{
    return HAL_UART_Receive_DMA(&huart1, data, length);
}

void bluetoothPrintf(const char *format, ...)
{
#if !defined DISABLE_BLUETOOTH
    static char buffer[BLUETOOTH_BUFFER_SIZE];
    va_list va_args;

    va_start(va_args, format);
    vsnprintf(buffer, BLUETOOTH_BUFFER_SIZE, format, va_args);
    va_end(va_args);

    bluetoothSend((unsigned char *) buffer, strlen(buffer));
#endif
}

void bluetoothWaitReady(void)
{
#if !defined DISABLE_BLUETOOTH
    // Wait until UART becomes ready
    while (HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY);
#endif
}

char *bluetoothCmd(const char *cmd)
{
    HAL_StatusTypeDef rv;
    static char response[255];
    char command[50];
    char *p_response = response;

    memset(response, 0, sizeof(response));

    // Disable RXNE interrupts
    bluetoothDeInit_IT();

    strcpy(command, cmd);
    strcat(command, "\r\n");

    HAL_UART_Transmit(&huart1, (uint8_t *)command, strlen(command), 5000);

    // Wait until end of reception
    do
    {
        rv = HAL_UART_Receive(&huart1, (uint8_t *)p_response++, 1, 200);
    }
    while (rv != HAL_TIMEOUT);

    // Put a NULL character at the end of the response string
    *p_response = '\0';

    bluetoothInit_IT();

    return response;
}

int bluetoothSetBaudrate(int baudrate, void *param)
{
    int rv;
    char cmd[40];

    UNUSED(param);

    // Send command to Bluetooth module
    sprintf(cmd, "AT+AB ChangeBaud %i", baudrate);
    bluetoothCmd(cmd);

    // Set baudrate of CPU USART
    __HAL_UART_DISABLE(&huart1);

    huart1.Init.BaudRate = baudrate;
    //    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    //    huart1.Init.StopBits = UART_STOPBITS_1;
    //    huart1.Init.Parity = UART_PARITY_NONE;
    //    huart1.Init.Mode = UART_MODE_TX_RX;
    //    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    //    huart1.Init.OverSampling = UART_OVERSAMPLING_8;
    rv = HAL_UART_Init(&huart1);

    __HAL_UART_ENABLE(&huart1);

    // TODO: Check whether the following code is required
    // bluetoothInit_IT();

    if (rv == HAL_OK)
    {
        return BLUETOOTH_DRIVER_E_SUCCESS;
    }
    return rv;
}

int isBluetoothEvent(char *evnt)
{
    if (strncmp(evnt, "AT-AB ", 6) == 0)
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
 * TEST FUNCTIONS
 *****************************************************************************/

void bluetoothTest(void)
{
    bluetoothPrintf("hello NARA, nb send = %d\r\n", 12345);
}
