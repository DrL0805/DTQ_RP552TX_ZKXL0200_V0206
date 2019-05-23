#include "main.h"

//#define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define main_debug  printf   
#else  
#define main_debug(...)                    
#endif 

/*
	TIMER0作为发送前导帧的精确定时器
*/
void TIMER0_Init(void)
{
	NRF_TIMER0->INTENSET |= 0x01 << 16;		// 使能COMPARE[0]中断
	NRF_TIMER0->MODE = 0;					// TIMER模式
	NRF_TIMER0->BITMODE = 3;				// 32位宽
	NRF_TIMER0->PRESCALER = 9;				// 分频系数
	
	NVIC_ClearPendingIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn, 1);
    NVIC_EnableIRQ(TIMER0_IRQn);		
}

void TIMER0_Start(uint32_t number_of_ms)
{
	NRF_TIMER0->TASKS_CLEAR=1;	
	
	// 2^9分频后，TIMER计数频率为31250Hz，每32us tick一次，即1ms tick31.25次，以下方式为了取整数
    NRF_TIMER0->CC[0]          = number_of_ms * 31;
    NRF_TIMER0->CC[0]         += number_of_ms / 4; 
	
	NRF_TIMER0->TASKS_START = 1;
}

void TIMER0_Stop(void)
{
	NRF_TIMER0->TASKS_CLEAR = 1;	// 清COUNTER值，从零开始计数
	NRF_TIMER0->TASKS_STOP = 1;		// 停止定时器
}

void TIMER0_IRQHandler(void)
{
	NRF_TIMER0->EVENTS_COMPARE[0] = 0;			// EVENT触发后一定要清空，否则会一直触发EVENT事件
	TIMER0_Stop();				
	
	if(RADIO.PreCnt++ < NRF_PRE_TX_NUMBER)
	{
		TIMER0_Start(1);
		RADIO.TxPreFlg = true;
	}
	else
	{
		RADIO.TxPreDataFlg = true;
	}
}

void DEBUG_FUN(void)
{
	nrf_gpio_pin_set(TX_PIN_NUMBER_1);
	nrf_delay_ms(5);
	nrf_gpio_pin_clear(TX_PIN_NUMBER_1);
	nrf_delay_ms(5);
}


int main (void)
{
	clocks_start();
	
	nrf_gpio_cfg_output(TX_PIN_NUMBER_1);	// 示波器查看
	debug_uart_init();						// 别忘答题器和接收器串口脚不一样
	
	timers_init();			// 普通定时器
	TIMER0_Init();			// 发送前导帧专用定时器 
	
	SE2431L_GpioInit();
	FLASH_Init();
	
	spi_gpio_init();
	my_spi_slave_init();
	
	RADIO_Init();

//	WDT_Init();
	
	rtc_calibrate_timeout_start();			//RTC校准定时器

//	DEBUG_UART_1("sys init ok \r\n");	
	
	while(true)
	{
		SPI_DataHandler();
		RADIO_SendHandler();
		
//		DEBUG_FUN();
//		WDT_FeedDog();	
	}
}


/******************************************************************************
  @函数:XOR_Cal
  @描述:
  @输入:* dat 异或的数组，及第几位开始
		length，数组中需要异或的长度
  @输出:数组异或的结果
  @调用:
******************************************************************************/
uint8_t XOR_Cal(uint8_t * dat,uint16_t length)
{
	uint8_t temp_xor;
	uint16_t i;

	temp_xor = *dat;
	for(i = 1;i < length; i++)
	{
		temp_xor = temp_xor ^ *(dat+i);
	}
	return temp_xor;
}

/******************************************************************************
  @函数:gpio_default_init
  @描述:51822的gpio配置为默认(省功耗)状态
  @输入:
  @输出:
  @调用:
******************************************************************************/
void gpio_default_init(void)
{
    uint32_t i = 0;
    for(i = 0; i< 32 ; ++i ) {
        NRF_GPIO->PIN_CNF[i] = (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos)
                               | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)
                               | (GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos)
                               | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos)
                               | (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos);
    }
}

/******************************************************************************
  @函数:clocks_start
  @描述:开启外部16M时钟
  @输入:
  @输出:
  @调用:
******************************************************************************/
void clocks_start(void)
{
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;

    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
	
	
	 /* Start low frequency crystal oscillator for app_timer(used by bsp)*/
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;

    while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
    {
        // Do nothing.
    }	
	
    NRF_CLOCK->EVENTS_DONE = 0;					//RTC校准
    NRF_CLOCK->TASKS_CAL = 1;
//    while (NRF_CLOCK->EVENTS_DONE == 0)
//    {
//        // Do nothing.
//    }
	
}


/******************************************************************************
  @函数:get_random_number
  @描述:获取0~255之间的随机数
  @输入:
  @输出:
  @调用:此函用于收到接收器下发数据后，随机延时0`255ms后再返回软件ACK，
		 减少多答题器同时回复ACK产生的数据冲突
******************************************************************************/
uint8_t get_random_number(void)
{
    NRF_RNG->TASKS_START = 1; // start the RNG peripheral.

	// Wait until the value ready event is generated.
	while (NRF_RNG->EVENTS_VALRDY == 0)
	{
		// Do nothing.
	}
	NRF_RNG->EVENTS_VALRDY = 0;		 // Clear the VALRDY EVENT.
	
	return (uint8_t)NRF_RNG->VALUE;
}
