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
//				SBQ_DEBUG_PIN_CLEAR();
				SBQ_DEBUG_PIN_TOG();
				SE2431L_SleepMode();
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

void RADIO_SetTxPower(void)
{
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
}

void RADIO_SendHandler(void)
{
	uint8_t TmpPos;
	uint32_t i;
	
	if(RADIO.TxPreFlg)	// 发送前导帧
	{
		RADIO.TxPreFlg = false;
		TmpPos = SPI._24gExitPos % SPI_MAX_CACHE_SIZE;
		
		tx_payload.length = 23;
		memcpy(tx_payload.data, &SPI.DATA[TmpPos].Data[6], 17);	// Head~ExtendLen
		tx_payload.data[17] = 3;													// 包长
		tx_payload.data[18] = 0x51;														
		tx_payload.data[19] = 0x01;
		tx_payload.data[20] = RADIO.PreCnt;
		tx_payload.data[tx_payload.length - 2] = XOR_Cal(tx_payload.data+1, tx_payload.length-3);				// 校验
		tx_payload.data[tx_payload.length - 1] = 0x21;									// 包尾	
		
		RADIO_SetTxPower();
		nrf_esb_set_rf_channel(SPI.DATA[TmpPos].Data[5]);
		nrf_esb_write_payload(&tx_payload);
//		SBQ_DEBUG_PIN_SET();
	}	
	
	if(RADIO.TxPreDataFlg)	// 发送带有前导帧的有效数据
	{
		RADIO.TxPreDataFlg = false;
		RADIO.HardTxBusyFlg = false;
		TmpPos = SPI._24gExitPos % SPI_MAX_CACHE_SIZE;
		SPI._24gExitPos++;
		
		tx_payload.length = SPI.DATA[TmpPos].Data[3]-2;
		memcpy(tx_payload.data, &SPI.DATA[TmpPos].Data[6], SPI.DATA[TmpPos].Data[3]-2);
		
		RADIO_SetTxPower();
		nrf_esb_set_rf_channel(SPI.DATA[TmpPos].Data[5]);		
		nrf_esb_write_payload(&tx_payload);
//		SBQ_DEBUG_PIN_TOG();
	}	
	
	// 如果RADIO硬件资源不被占用，且SPI层有还未发送的2.4G数据
	if(!RADIO.HardTxBusyFlg && (SPI._24gExitPos < SPI._24gEnterPos))
	{
		TmpPos = SPI._24gExitPos % SPI_MAX_CACHE_SIZE;
		
		if(SPI_DATA_24G == SPI.DATA[TmpPos].State)
		{
			SPI.DATA[TmpPos].State = SPI_DATA_INVALID;
			
			switch(SPI.DATA[TmpPos].Data[4])
			{
				case RADIO_TYPE_USE_NEED_PRE:	// 有效数据，需发前导帧
					RADIO.HardTxBusyFlg = true;
					RADIO.PreCnt = 0;
					TIMER0_Start(1);			// 开始前导帧定时器
					break;
				case RADIO_TYPE_USE_NEEDLESS_PRE:	// 有效数据，无需发前导帧 
					SPI._24gExitPos++;
				
					RADIO_SetTxPower();
					nrf_esb_set_rf_channel(SPI.DATA[TmpPos].Data[5]);
				
					tx_payload.length = SPI.DATA[TmpPos].Data[3]-2;
					memcpy(tx_payload.data, &SPI.DATA[TmpPos].Data[6], SPI.DATA[TmpPos].Data[3]-2);
					
					nrf_esb_write_payload(&tx_payload);
//					SBQ_DEBUG_PIN_TOG();
					break;
				case RADIO_TYPE_INSTANT_ACK:	// ACK
					break;
				case RADIO_TYPE_INSTANT_USE:	// 需优先发送的有效数据
					break;
				default:
					break;
			}
		}
		else
		{
			SPI._24gExitPos++;
		}
	}
}









