#include "my_timer.h"

//#define TIMER_DEBUG
#ifdef TIMER_DEBUG
#define timer_debug  printf   
#else  
#define timer_debug(...)                    
#endif 

//定时器宏定义
#define APP_TIMER_PRESCALER     0
#define APP_TIMER_OP_QUEUE_SIZE 5

#define TEMP_TIMEOUT_INTERVAL     			APP_TIMER_TICKS(10000, 	APP_TIMER_PRESCALER)
#define NRF_TRANSMIT_TIMEOUT_INTERVAL     	APP_TIMER_TICKS(50, 	APP_TIMER_PRESCALER)
#define SPI_OVERTIME_TIMEOUT_INTERVAL     	APP_TIMER_TICKS(100, 	APP_TIMER_PRESCALER)
#define TX_OVERTIME_TIMEOUT_INTERVAL     	APP_TIMER_TICKS(10, 	APP_TIMER_PRESCALER)
#define TX_PRE_OVERTIME_TIMEOUT_INTERVAL    APP_TIMER_TICKS(150, 	APP_TIMER_PRESCALER)
#define WATCH_DOG_TIMEOUT_INTERVAL     		APP_TIMER_TICKS(500, 	APP_TIMER_PRESCALER)


APP_TIMER_DEF(temp_timer_id);
APP_TIMER_DEF(spi_overtime_timer_id);	
APP_TIMER_DEF(nrf_transmit_timer_id);
APP_TIMER_DEF(tx_overtime_timer_id);
APP_TIMER_DEF(tx_pre_overtime_timer_id);
APP_TIMER_DEF(watch_dog_timer_id);	

void timers_init(void)
{
	uint32_t err_code;
	
	APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
	
	err_code = app_timer_create(&temp_timer_id,APP_TIMER_MODE_REPEATED,rtc_calibrate_timeout_handler);
	APP_ERROR_CHECK(err_code);
	
	// 循环的定时器
	err_code = app_timer_create(&nrf_transmit_timer_id,APP_TIMER_MODE_REPEATED,nrf_transmit_timer_handler);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_create(&watch_dog_timer_id,APP_TIMER_MODE_REPEATED,watch_dog_timer_handler);
	APP_ERROR_CHECK(err_code);
	
	// 仅一次的定时器
	err_code = app_timer_create(&spi_overtime_timer_id,APP_TIMER_MODE_SINGLE_SHOT,spi_overtime_timer_handler);
	APP_ERROR_CHECK(err_code);
	
	err_code = app_timer_create(&tx_overtime_timer_id,APP_TIMER_MODE_SINGLE_SHOT,TIMER_TxOvertimeHandler);
	APP_ERROR_CHECK(err_code);	
	
	err_code = app_timer_create(&tx_pre_overtime_timer_id,APP_TIMER_MODE_SINGLE_SHOT,TIMER_TxPreOvertimeHandler);
	APP_ERROR_CHECK(err_code);		
}

void rtc_calibrate_timeout_start(void)
{
	uint32_t err_code;
	err_code = app_timer_start(temp_timer_id,TEMP_TIMEOUT_INTERVAL,NULL);
	APP_ERROR_CHECK(err_code);
}

void rtc_calibrate_timeout_stop(void)
{
	uint32_t err_code;
	err_code = app_timer_stop(temp_timer_id);
	APP_ERROR_CHECK(err_code);
}

void rtc_calibrate_timeout_handler(void * p_context)
{
	//timer_debug("timer_debug:rtc_calibrate_timeout_handler \r\n");
	
    NRF_CLOCK->EVENTS_DONE = 0;		
    NRF_CLOCK->TASKS_CAL = 1;
	
}




void nrf_transmit_timeout_start(uint8_t timer_ms)
{
	uint32_t err_code;
	err_code = app_timer_start(nrf_transmit_timer_id,APP_TIMER_TICKS(timer_ms,APP_TIMER_PRESCALER),NULL);
	APP_ERROR_CHECK(err_code);
}

void nrf_transmit_timeout_stop(void)
{
	uint32_t err_code;
	err_code = app_timer_stop(nrf_transmit_timer_id);
	APP_ERROR_CHECK(err_code);
}

void nrf_transmit_timer_handler(void * p_context)
{
}


void spi_overtime_timer_start(void)
{
	uint32_t err_code;
	err_code = app_timer_start(spi_overtime_timer_id,SPI_OVERTIME_TIMEOUT_INTERVAL,NULL);
	APP_ERROR_CHECK(err_code);
}

void spi_overtime_timer_stop(void)
{
	uint32_t err_code;
	err_code = app_timer_stop(spi_overtime_timer_id);
	APP_ERROR_CHECK(err_code);
}

void spi_overtime_timer_handler(void * p_context)
{
	
}

void TIMER_TxOvertimeStart(void)
{
	uint32_t err_code;
	err_code = app_timer_start(tx_overtime_timer_id,TX_OVERTIME_TIMEOUT_INTERVAL,NULL);
	APP_ERROR_CHECK(err_code);
}

void TIMER_TxOvertimeStop(void)
{
	uint32_t err_code;
	err_code = app_timer_stop(tx_overtime_timer_id);
	APP_ERROR_CHECK(err_code);
}

void TIMER_TxOvertimeHandler(void * p_context)
{

}

void TIMER_TxPreOvertimeStart(void)
{
	uint32_t err_code;
	err_code = app_timer_start(tx_pre_overtime_timer_id,TX_PRE_OVERTIME_TIMEOUT_INTERVAL,NULL);
	APP_ERROR_CHECK(err_code);
}

void TIMER_TxPreOvertimeStop(void)
{
	uint32_t err_code;
	err_code = app_timer_stop(tx_pre_overtime_timer_id);
	APP_ERROR_CHECK(err_code);
}

void TIMER_TxPreOvertimeHandler(void * p_context)
{
	RADIO.BusyFlg = false;
}

void watch_dog_timeout_start(void)
{
	uint32_t err_code;
	err_code = app_timer_start(watch_dog_timer_id,WATCH_DOG_TIMEOUT_INTERVAL,NULL);
	APP_ERROR_CHECK(err_code);
}

void watch_dog_timeout_stop(void)
{
	uint32_t err_code;
	err_code = app_timer_stop(watch_dog_timer_id);
	APP_ERROR_CHECK(err_code);
}

void watch_dog_timer_handler(void * p_context)
{
//	TIMER_DEBUG("watch_dog_timer_handler \r\n");
	WDT.FeedFlg = true;
}

