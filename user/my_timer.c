#include "my_timer.h"

#define TIMER_DEBUG
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
#define RETURN_ACK_TIMEOUT_INTERVAL     	APP_TIMER_TICKS(5, 	APP_TIMER_PRESCALER)


APP_TIMER_DEF(temp_timer_id);
APP_TIMER_DEF(spi_overtime_timer_id);	
APP_TIMER_DEF(nrf_transmit_timer_id);
APP_TIMER_DEF(return_ack_timer_id);


void timers_init(void)
{
	uint32_t err_code;
	
	APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
	
	err_code = app_timer_create(&temp_timer_id,APP_TIMER_MODE_REPEATED,rtc_calibrate_timeout_handler);
	APP_ERROR_CHECK(err_code);
	
	// 循环的定时器
	err_code = app_timer_create(&nrf_transmit_timer_id,APP_TIMER_MODE_REPEATED,nrf_transmit_timer_handler);
	APP_ERROR_CHECK(err_code);
	
	err_code = app_timer_create(&return_ack_timer_id,APP_TIMER_MODE_REPEATED,return_ack_timer_handler);
	APP_ERROR_CHECK(err_code);	
	
	// 仅一次的定时器
	err_code = app_timer_create(&spi_overtime_timer_id,APP_TIMER_MODE_SINGLE_SHOT,spi_overtime_timer_handler);
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
	uint8_t i;
	// 如果前导帧已发满NRF_PRE_TX_NUMBER次，开始发有效数据
	if(++RADIO.PreCnt > NRF_PRE_TX_NUMBER)
	{
		nrf_transmit_timeout_stop();
		RADIO.BusyFlg = false;
		
		tx_payload.length = RADIO.TX.Len;
		memcpy(tx_payload.data, RADIO.TX.Data, tx_payload.length);								// Head~ExtendLen

		SE2431L_TxMode();
		
		for(i = 0; i < NRF_ENHANCE_TX_NUM; i++)
			nrf_esb_write_payload(&tx_payload);
	}
	else
	{
		tx_payload.length = 20;
		
		memcpy(tx_payload.data, &RADIO.TX.Data, 14);								// Head~ExtendLen
		tx_payload.data[14] = 3;														// 包长
		tx_payload.data[15] = 0x51;														
		tx_payload.data[16] = 0x01;
		tx_payload.data[17] = RADIO.PreCnt;
		tx_payload.data[tx_payload.length - 2] = XOR_Cal(tx_payload.data+1, 17);		// 校验
		tx_payload.data[tx_payload.length - 1] = 0x21;									// 包尾	
		
		SE2431L_TxMode();
		nrf_esb_write_payload(&tx_payload);
	}
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

void return_ack_timer_start(void)
{
	uint32_t err_code;
	err_code = app_timer_start(return_ack_timer_id,RETURN_ACK_TIMEOUT_INTERVAL,NULL);
	APP_ERROR_CHECK(err_code);
}

void return_ack_timer_stop(void)
{
	uint32_t err_code;
	err_code = app_timer_stop(return_ack_timer_id);
	APP_ERROR_CHECK(err_code);
}

void return_ack_timer_handler(void * p_context)
{
	if(RADIO.ACK.Num)
	{
		ack_payload.noack  = true;
		ack_payload.pipe   = NRF_PIPE;
		ack_payload.length = 17 + 3 + RADIO.ACK.Num * 4;
		
		ack_payload.data[0] = 0x61;
		memset(ack_payload.data+1, 0x00, 8);				// 目标ID 源ID都为0
		ack_payload.data[9] = 0x01;
		ack_payload.data[10] = 0x20;
		ack_payload.data[11] = 0x00;
		ack_payload.data[12] = 0x00;
		ack_payload.data[13] = 0x00;
		ack_payload.data[14] = 3 + RADIO.ACK.Num*4;			// PackLen
		ack_payload.data[15] = 0x52;						// DataType = ACK													
		ack_payload.data[16] = 1 + RADIO.ACK.Num*4;			// DataLen
		ack_payload.data[17] = RADIO.ACK.Num;				
		memcpy(ack_payload.data+18, RADIO.ACK.Uid, RADIO.ACK.Num*4);
		ack_payload.data[ack_payload.length - 2] = XOR_Cal(ack_payload.data+1, ack_payload.length - 3);		
		ack_payload.data[ack_payload.length - 1] = 0x21;												
		
		SE2431L_TxMode();
		nrf_esb_write_payload(&ack_payload);	
	
		RADIO.ACK.Num = 0;
	}
}





