#ifndef __MY_TIMER_H
#define	__MY_TIMER_H

//#include "app_timer.h"
//#include "my_radio.h"
#include "main.h"




void timers_init(void);

void rtc_calibrate_timeout_start(void);
void rtc_calibrate_timeout_stop(void);
void rtc_calibrate_timeout_handler(void * p_context);

void nrf_transmit_timeout_start(uint8_t timer_ms);
void nrf_transmit_timeout_stop(void);
void nrf_transmit_timer_handler(void * p_context);

void spi_overtime_timer_start(void);
void spi_overtime_timer_stop(void);
void spi_overtime_timer_handler(void * p_context);

void TIMER_TxOvertimeStart(void);
void TIMER_TxOvertimeStop(void);
void TIMER_TxOvertimeHandler(void * p_context);

#endif 

