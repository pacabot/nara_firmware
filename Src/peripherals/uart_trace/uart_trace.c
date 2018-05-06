#include  "stm32h7xx_hal.h"

#include <stdarg.h>
#include <stdint.h>
#include "config/config.h"

#ifndef TRACE_UART
/**
 * Force TRACE_UART to 0 by default (if not defined in the project)
 */
#define TRACE_UART  0
#endif

#if TRACE_UART
extern UART_HandleTypeDef huart1;

static volatile int InUsed=0;
static char uart_buffer[256];
static uint32_t UartErrCnt=0;

/**
  * @brief  Tx Transfer completed callback
  * @param  huart: UART handle.
  * @note   This example shows a simple way to report end of DMA Tx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    // TODO check if any more to send and do it
    InUsed=0;
}

int uart_vprintf(const char *msg, va_list ap){
    int n;
    int status;
//    while( InUsed ){
//           //
//        __WFI();
//    }
//    InUsed|=1;
    n=vsnprintf(uart_buffer, sizeof(uart_buffer),  msg, ap);
//    status = HAL_UART_Transmit_DMA(&huart1, (uint8_t*)uart_buffer, n );
    status = HAL_UART_Transmit(&huart1, (uint8_t*)uart_buffer, n, 1000);

    if( status ){
        UartErrCnt++;
        InUsed=0;
    }
    return n;
}

int uart_printf(const char *msg, ...){
	va_list ap;
    int n;
//    while( InUsed ){
//        //
//        __WFI();
//    }
    va_start(ap, msg);
    n=uart_vprintf(msg, ap);
    va_end(ap);
    return n;
}

#else
#	define uart_vprintf(...) (void)0
#	define uart_printf(...)	(void)0
#endif
