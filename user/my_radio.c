#include "my_radio.h"

//#define RADIO_DEBUG

#ifdef RADIO_DEBUG
#define radio_debug  printf   
#else  
#define radio_debug(...)                    
#endif 

nrf_esb_payload_t        tx_payload;
nrf_esb_payload_t        ack_payload;

RADIO_PARAMETERS_T 		RADIO;


void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
			if(0 == get_tx_fifo_count())
			{
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

uint32_t my_rx_esb_init(void)
{
	uint32_t err_code;
	uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7};
	uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };

	nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
	nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
	nrf_esb_config.retransmit_delay         = 600;
	nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_1MBPS;
	nrf_esb_config.event_handler            = nrf_esb_event_handler;
	nrf_esb_config.mode                     = NRF_ESB_MODE_PRX;
	nrf_esb_config.selective_auto_ack       = true;	
	nrf_esb_config.payload_length           = 250;

	err_code = nrf_esb_set_rf_channel(2);		//注意：答题器发送频点61接收频点21，接收器相反
	VERIFY_SUCCESS(err_code);
	
	err_code = nrf_esb_init(&nrf_esb_config);
	VERIFY_SUCCESS(err_code);

	err_code = nrf_esb_set_base_address_0(base_addr_0);
	VERIFY_SUCCESS(err_code);

	err_code = nrf_esb_set_prefixes(addr_prefix, 8);
	VERIFY_SUCCESS(err_code);

	return err_code;
}


void RADIO_Init(void)
{
	ret_code_t err_code;
	
	
	RADIO.TxChannal = 4;
	RADIO.TxPower = NRF_DEFAULT_TX_POWER;
	
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
	ack_payload.noack  = true;
	ack_payload.pipe   = NRF_PIPE;
	ack_payload.length = 17 + 3 + UidNum * 4;
	
	ack_payload.data[0] = 0x61;
	memset(ack_payload.data+1, 0x00, 8);			// 目标ID 源ID都为0
	ack_payload.data[9] = 0x01;
	ack_payload.data[10] = 0x20;
	ack_payload.data[11] = 0x00;
	ack_payload.data[12] = 0x00;
	ack_payload.data[13] = 0x00;
	ack_payload.data[14] = 3 + UidNum*4;			// PackLen
	ack_payload.data[15] = 0x52;						// DataType = ACK													
	ack_payload.data[16] = 1 + UidNum*4;			// DataLen
	ack_payload.data[17] = UidNum;				
	memcpy(ack_payload.data+18, UidBuf, UidNum*4);
	ack_payload.data[ack_payload.length - 2] = XOR_Cal(ack_payload.data+1, ack_payload.length - 3);		
	ack_payload.data[ack_payload.length - 1] = 0x21;												
	
	nrf_esb_set_rf_channel(TxChannal);
	
//	printf("A \r\n");
	
	SE2431L_TxMode();
	nrf_esb_write_payload(&ack_payload);
//	nrf_esb_write_payload(&ack_payload);
	
	
}

void RADIO_SendData(uint8_t* DataBuf, uint8_t DataLen, uint8_t TxChannal)
{
	tx_payload.noack  = true;
	tx_payload.pipe   = NRF_PIPE;
	tx_payload.length = DataLen;
	
	memcpy(tx_payload.data, DataBuf, DataLen);
	
	nrf_esb_set_rf_channel(TxChannal);	
	
	SE2431L_TxMode();
	
//	printf("%d \r\n",tx_payload.length);
	
	nrf_esb_write_payload(&tx_payload);
}










