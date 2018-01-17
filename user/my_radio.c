#include "my_radio.h"

//#define RADIO_DEBUG

#ifdef RADIO_DEBUG
#define radio_debug  printf   
#else  
#define radio_debug(...)                    
#endif 

nrf_esb_payload_t        tx_payload;

RADIO_PARAMETERS_T 		RADIO;

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
			if(0 == get_tx_fifo_count())
			{
				nrf_gpio_pin_clear(TX_PIN_NUMBER_1);
				
				SE2431L_SleepMode();
				TIMER_TxOvertimeStop();				
				RADIO.HardTxBusyFlg = false;
			}
            break;
        case NRF_ESB_EVENT_TX_FAILED:
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
            break;
    }
}



uint32_t my_tx_esb_init(void)
{
    uint32_t err_code;
    uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
    uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };
	
    nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
    nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
    nrf_esb_config.retransmit_delay         = 600;
    nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_2MBPS;
    nrf_esb_config.event_handler            = nrf_esb_event_handler;
    nrf_esb_config.mode                     = NRF_ESB_MODE_PTX;
	nrf_esb_config.selective_auto_ack       = true;	//lj 理解，等于true时，每次发送的时候可选择回不回复ACK
	nrf_esb_config.payload_length           = 250;
	
	err_code = nrf_esb_set_rf_channel(RADIO.TxChannal);		
	VERIFY_SUCCESS(err_code);
	
	err_code = nrf_esb_set_tx_power(NRF_ESB_TX_POWER_4DBM);
	VERIFY_SUCCESS(err_code);
	
    err_code = nrf_esb_set_base_address_0(base_addr_0);
    VERIFY_SUCCESS(err_code);
	
    err_code = nrf_esb_set_prefixes(addr_prefix, 8);
    VERIFY_SUCCESS(err_code);
	
    err_code = nrf_esb_init(&nrf_esb_config);
    VERIFY_SUCCESS(err_code);
    return err_code;
}


void RADIO_Init(void)
{
	ret_code_t err_code;

	RADIO.TxChannal = NRF_DEFAULT_TX_CHANNAL;
	
    err_code = my_tx_esb_init();
    APP_ERROR_CHECK(err_code);	
	
	tx_payload.noack  = true;
	tx_payload.pipe   = NRF_PIPE;
}

// 发送一个ACK
void RADIO_SendAck(uint8_t* UidBuf, uint8_t UidNum, uint32_t TxChannal)
{
	/*
		答题器2.4G链路层数据格式
		0：包头0x61
		1：源ID，
		5：目标ID，
		9：设备类型，答题器 = 0x11
		10：协议版本 = 0x21
		11：帧号 = 0
		12：包号 = 0
		13：扩展字节长度 = 0
		14：包长，广播包长
		----------包内容--------------
			15：广播命令类型 = 0x52
			16：广播命令长度 = 
			---------命令内容---------
				17：UidNum
				19：UidBuf
		------------------------------
		XX：校验
		XX：包尾0x21
	*/
	uint8_t TmpAckBuf[256], TmpAckLen;
	
	TmpAckLen = 17 + 3 + UidNum * 4;
	
	TmpAckBuf[0] = 0x61;
	memset(TmpAckBuf+1, 0x00, 8);			// 目标ID 源ID都为0
	TmpAckBuf[9] = 0x01;
	TmpAckBuf[10] = 0x20;
	TmpAckBuf[11] = 0x00;
	TmpAckBuf[12] = 0x00;
	TmpAckBuf[13] = 0x00;
	TmpAckBuf[14] = 3 + UidNum*4;			// PackLen
	TmpAckBuf[15] = 0x52;					// DataType = ACK													
	TmpAckBuf[16] = 1 + UidNum*4;			// DataLen
	TmpAckBuf[17] = UidNum;				
	memcpy(TmpAckBuf+18, UidBuf, UidNum*4);
	TmpAckBuf[TmpAckLen - 2] = XOR_Cal(TmpAckBuf+1, TmpAckLen - 3);		
	TmpAckBuf[TmpAckLen - 1] = 0x21;												

	if(RINGBUF_GetStatus_nRF() != RINGBUF_STATUS_FULL_nRF)
	{
		RINGBUF_WriteData_nRF(TmpAckBuf, TmpAckLen, TxChannal);													
	}	
}


void RADIO_SendHandler(void)
{
	uint8_t TmpChannal;
	
	// 如果RADIO硬件资源不被占用，则发送RingBuffer里的数据
	if(!RADIO.HardTxBusyFlg)
	{
		if((RINGBUF_GetStatus_nRF() != RINGBUF_STATUS_EMPTY_nRF))
		{
			RINGBUF_ReadData_nRF(tx_payload.data, &tx_payload.length, &TmpChannal);

//			printf("Channal:%02X, Len:%02X \r\n", TmpChannal, tx_payload.length);
//			DEBUG_UART_N(tx_payload.data, tx_payload.length);
			
			// 通过%02X格式打印数据会导致程序卡死，其他打印方式则不会
//			DEBUG_UART_1("%02X \r\n",tx_payload.length);
			
//			SE2431L_TxMode();
			switch(RADIO.TxPower)					
			{
				case 1:
					nrf_esb_set_tx_power(NRF_ESB_TX_POWER_0DBM);
					SE2431L_BypassMode();
					break;
				case 2:
					nrf_esb_set_tx_power(NRF_ESB_TX_POWER_4DBM);
					SE2431L_BypassMode();								
					break;
				case 3:
					nrf_esb_set_tx_power(NRF_ESB_TX_POWER_NEG4DBM);
					SE2431L_TxMode();									
					break;
				case 4:
					nrf_esb_set_tx_power(NRF_ESB_TX_POWER_0DBM);
					SE2431L_TxMode();									
					break;
				case 5:
					nrf_esb_set_tx_power(NRF_ESB_TX_POWER_4DBM);
					SE2431L_TxMode();									
					break;
			}			
			nrf_esb_set_rf_channel(TmpChannal);
			
			RADIO.HardTxBusyFlg = true;
			
			nrf_gpio_pin_set(TX_PIN_NUMBER_1);
			nrf_esb_write_payload(&tx_payload);
			
			TIMER_TxOvertimeStart();
			
		}		
	}
}









