#ifndef __MY_UART_H
#define	__MY_UART_H


#include "simple_uart.h"

#define DEBUG_UART

#ifdef DEBUG_UART
#define DEBUG_UART_1  printf   
#define DEBUG_UART_N  UART_PrintfN
#else  
#define DEBUG_UART_1(...)   
#define DEBUG_UART_N(...)
#endif 

	//接收器51822RX对应UART口
#define RX_PIN_NUMBER_1  9    // UART RX pin number.
#define TX_PIN_NUMBER_1  10    // UART TX pin number.
#define CTS_PIN_NUMBER_1 11   // UART Clear To Send pin number. Not used if HWFC is set to false
#define RTS_PIN_NUMBER_1 12    // Not used if HWFC is set to false
#define HWFC_1           false // UART hardware flow control

	
void debug_uart_init(void);
int fputc(int ch, FILE *f);
void UART_send_byte(uint8_t byte);
void UART_Send(uint8_t *Buffer, uint32_t Length);
uint8_t UART_Recive(void);
void UART_PrintfN(uint8_t * p_buffer, uint32_t len);

#endif /* __UART_H */
