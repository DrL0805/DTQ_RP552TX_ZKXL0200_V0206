#include "main.h"

//#define MAIN_DEBUG

#ifdef MAIN_DEBUG
#define main_debug  printf   
#else  
#define main_debug(...)                    
#endif 

/*
	TIMER0��Ϊ����ǰ��֡�ľ�ȷ��ʱ��
*/
void TIMER0_Init(void)
{
	NRF_TIMER0->INTENSET |= 0x01 << 16;		// ʹ��COMPARE[0]�ж�
	NRF_TIMER0->MODE = 0;					// TIMERģʽ
	NRF_TIMER0->BITMODE = 3;				// 32λ��
	NRF_TIMER0->PRESCALER = 9;				// ��Ƶϵ��
	
	NVIC_ClearPendingIRQ(TIMER0_IRQn);
    NVIC_SetPriority(TIMER0_IRQn, 1);
    NVIC_EnableIRQ(TIMER0_IRQn);		
}

void TIMER0_Start(uint32_t number_of_ms)
{
	NRF_TIMER0->TASKS_CLEAR=1;	
	
	// 2^9��Ƶ��TIMER����Ƶ��Ϊ31250Hz��ÿ32us tickһ�Σ���1ms tick31.25�Σ����·�ʽΪ��ȡ����
    NRF_TIMER0->CC[0]          = number_of_ms * 31;
    NRF_TIMER0->CC[0]         += number_of_ms / 4; 
	
	NRF_TIMER0->TASKS_START = 1;
}

void TIMER0_Stop(void)
{
	NRF_TIMER0->TASKS_CLEAR = 1;	// ��COUNTERֵ�����㿪ʼ����
	NRF_TIMER0->TASKS_STOP = 1;		// ֹͣ��ʱ��
}

void TIMER0_IRQHandler(void)
{
	NRF_TIMER0->EVENTS_COMPARE[0] = 0;			// EVENT������һ��Ҫ��գ������һֱ����EVENT�¼�
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
	
	nrf_gpio_cfg_output(TX_PIN_NUMBER_1);	// ʾ�����鿴
	debug_uart_init();						// �����������ͽ��������ڽŲ�һ��
	
	timers_init();			// ��ͨ��ʱ��
	TIMER0_Init();			// ����ǰ��֡ר�ö�ʱ�� 
	
	SE2431L_GpioInit();
	FLASH_Init();
	
	spi_gpio_init();
	my_spi_slave_init();
	
	RADIO_Init();

//	WDT_Init();
	
	rtc_calibrate_timeout_start();			//RTCУ׼��ʱ��

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
  @����:XOR_Cal
  @����:
  @����:* dat �������飬���ڼ�λ��ʼ
		length����������Ҫ���ĳ���
  @���:�������Ľ��
  @����:
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
  @����:gpio_default_init
  @����:51822��gpio����ΪĬ��(ʡ����)״̬
  @����:
  @���:
  @����:
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
  @����:clocks_start
  @����:�����ⲿ16Mʱ��
  @����:
  @���:
  @����:
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
	
    NRF_CLOCK->EVENTS_DONE = 0;					//RTCУ׼
    NRF_CLOCK->TASKS_CAL = 1;
//    while (NRF_CLOCK->EVENTS_DONE == 0)
//    {
//        // Do nothing.
//    }
	
}


/******************************************************************************
  @����:get_random_number
  @����:��ȡ0~255֮��������
  @����:
  @���:
  @����:�˺������յ��������·����ݺ������ʱ0`255ms���ٷ������ACK��
		 ���ٶ������ͬʱ�ظ�ACK���������ݳ�ͻ
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
