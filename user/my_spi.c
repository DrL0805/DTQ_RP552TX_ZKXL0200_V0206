#include "my_spi.h"

//#define SPI_DEBUG
#ifdef SPI_DEBUG
#define spi_debug  printf   
#else
#define spi_debug(...)                    
#endif

uint8_t tx_data_len;
uint8_t m_tx_buf[TX_BUF_SIZE];   /**< SPI TX buffer. */      
uint8_t m_rx_buf[RX_BUF_SIZE];   /**< SPI RX buffer. */ 

SPI_PARAMETERS_T		SPI;

static const nrf_drv_spis_t spis = NRF_DRV_SPIS_INSTANCE(1);

static void spis_event_handler(nrf_drv_spis_event_t event);

/**@brief Function for initializing buffers.
 *
 * @param[in] p_tx_buf  Pointer to a transmit buffer.
 * @param[in] p_rx_buf  Pointer to a receive  buffer.
 * @param[in] len       Buffers length.
 */
static __INLINE void spi_slave_buffers_init(uint8_t * const p_tx_buf, uint8_t * const p_rx_buf, const uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        p_tx_buf[i] = 0x00;
        p_rx_buf[i] = 0x00;
    }
}

static __INLINE void spi_slave_tx_buffers_init(SPI_CMD_TYPE spi_buf_type)
{
	switch(spi_buf_type)
	{
		case SPI_CMD_NONE:
			memset(m_tx_buf,0x00,TX_BUF_SIZE);		
			break;
		case SPI_CMD_SEND_24G_DATA:
			m_tx_buf[0] = 0x86;							// 包头
			m_tx_buf[1] = NRF_TX_DEV_ID;				// 设备ID
			m_tx_buf[2] = SPI_CMD_SEND_24G_DATA;		// CmdType
			m_tx_buf[3] = 0x01;							// CmdLen
			m_tx_buf[4] = 0x01;							// CmdData	
			m_tx_buf[5] = XOR_Cal(m_tx_buf+1,4);
			m_tx_buf[6] = 0x76;
			memset(m_tx_buf+7,0x00,TX_BUF_SIZE - 7);	// 其他缓冲区值默认为0
			break;
		case SPI_CMD_SET_CHANNAL:
			m_tx_buf[0] = 0x86;							// 包头
			m_tx_buf[1] = NRF_TX_DEV_ID;				// 设备ID
			m_tx_buf[2] = SPI_CMD_SET_CHANNAL;			// CmdType
			m_tx_buf[3] = 0x02;							// CmdLen
			m_tx_buf[4] = RADIO.TxChannal;
			m_tx_buf[5] = RADIO.TxPower;
			m_tx_buf[6] = XOR_Cal(m_tx_buf+1,5);		// XOR校验
			m_tx_buf[7] = 0x76;							// 包尾
			memset(m_tx_buf+8,0x00,TX_BUF_SIZE - 8);	// 其他缓冲区值默认为0
			break;
		case SPI_CMD_GET_STATE:
			m_tx_buf[0] = 0x86;							//包头
			m_tx_buf[1] = 0x31;							//类型
			m_tx_buf[2] = 0x08;							//DataLen
			m_tx_buf[3] = RADIO.TxChannal;				//
			m_tx_buf[4] = VERSION_LEVEL_1;				//一级版本号
			m_tx_buf[5] = VERSION_LEVEL_2;				//二级版本号
			memset(m_tx_buf+6,0x00,5);					//保留
			m_tx_buf[11] = XOR_Cal(m_tx_buf+1,10);		//XOR校验
			m_tx_buf[12] = 0x76;						//包尾
			memset(m_tx_buf+13,0x00,TX_BUF_SIZE - 13);	//其他缓冲区值默认为0
			break;
		default:
			break;
	}
}

//SPI相关引脚初始化
void spi_gpio_init(void)
{
//	nrf_gpio_cfg_output(SPIS_IRQ_PIN);
	nrf_gpio_cfg_input(SPIS_CE_PIN, NRF_GPIO_PIN_PULLUP);  
//	nrf_gpio_pin_set(SPIS_IRQ_PIN);
}

void my_spi_slave_init(void)
{
    nrf_drv_spis_config_t spis_config = NRF_DRV_SPIS_DEFAULT_CONFIG(1);              
    spis_config.miso_pin = SPIS_MISO_PIN;                                         
    spis_config.mosi_pin = SPIS_MOSI_PIN;                                         
    spis_config.sck_pin = SPIS_SCK_PIN;
    spis_config.csn_pin	= SPIS_CSN_PIN;
    spis_config.mode = NRF_DRV_SPIS_MODE_0;
    spis_config.bit_order = NRF_DRV_SPIS_BIT_ORDER_MSB_FIRST;
    spis_config.def = NRF_DRV_SPIS_DEFAULT_DEF;
    spis_config.orc = NRF_DRV_SPIS_DEFAULT_ORC;
	
    APP_ERROR_CHECK(nrf_drv_spis_init(&spis, &spis_config, spis_event_handler));
	
	spi_slave_buffers_init(m_tx_buf, m_rx_buf, (uint16_t)TX_BUF_SIZE);
	APP_ERROR_CHECK(nrf_drv_spis_buffers_set(&spis,m_tx_buf,TX_BUF_SIZE,m_rx_buf,RX_BUF_SIZE));
}

/* 产生低电平脉冲，通知stm32中断读取SPI数据 */
void spi_trigger_irq(void)
{
	static uint8_t SpiPinInitFlg = false;

	if(false == SpiPinInitFlg)
	{
		SpiPinInitFlg = true;
		nrf_gpio_cfg_output(SPIS_IRQ_PIN);
	}

	nrf_gpio_pin_clear(SPIS_IRQ_PIN);
	nrf_delay_us(1);
	nrf_gpio_pin_set(SPIS_IRQ_PIN);	
}

void spis_event_handler(nrf_drv_spis_event_t event)
{
	uint8_t TmpPos;
	
	switch(event.evt_type)
	{
		case NRF_DRV_SPIS_BUFFERS_SET_DONE:
			break;
		case NRF_DRV_SPIS_XFER_DONE:
			//包头、包尾、数据类型、XOR校验
			if( (0x86              == m_rx_buf[SPI_LINK_OFFSET_HEAD])       &&
				(NRF_TX_DEV_ID     == m_rx_buf[SPI_LINK_OFFSET_DEV_ID])       &&
				(0x76              == m_rx_buf[event.rx_amount - 1]) &&				
				m_rx_buf[event.rx_amount - 2] == XOR_Cal(m_rx_buf+1, event.rx_amount - 3) )
			{
				if((SPI.SpiEnterPos - SPI.SpiExitPos) < SPI_MAX_CACHE_SIZE)
				{
					TmpPos = SPI.SpiEnterPos % SPI_MAX_CACHE_SIZE;
					SPI.SpiEnterPos++;
//					SBQ_DEBUG_PIN_TOG();
					SPI.DATA[TmpPos].State = SPI_DATA_NEW;
					SPI.DATA[TmpPos].DataLen = event.rx_amount;
					memcpy(SPI.DATA[TmpPos].Data, m_rx_buf, event.rx_amount);
				}
				else
				{
					// 开辟的SPI缓存过小
				}
			}
			
			// SPIS每次收发完数据后需要重新SET下，才能接受新的数据
			nrf_drv_spis_buffers_set(&spis,m_tx_buf,TX_BUF_SIZE,m_rx_buf,RX_BUF_SIZE);			
			break;
		case NRF_DRV_SPIS_EVT_TYPE_MAX:
			break;
		default:
			break;
	}
}

void SPI_DataHandler(void)
{
	/*
		SPI层数据格式：
			0：		Head固定 0x86
			1：		DevId
			2：		CmdType
			3：		CmdLen
			4~之后：CmdData
				4：DataType，有效数据/ack/是否需要发送前导帧
				5：发送频点
				6~之后：需发送的2.4G数据
	*/		
	uint8_t TmpPos;
	
	if(SPI.SpiExitPos < SPI.SpiEnterPos)
	{
		TmpPos = SPI.SpiExitPos % SPI_MAX_CACHE_SIZE;
		SPI.SpiExitPos++;
//		SBQ_DEBUG_PIN_TOG();
//		UART_PrintfN(SPI.DATA[TmpPos].Data, SPI.DATA[TmpPos].DataLen);
//		UART_PrintfN(SPI.DATA[TmpPos].Data, 7);
//		simple_uart_put(SPI.DATA[TmpPos].DataLen);
		
		switch(SPI.DATA[TmpPos].Data[SPI_LINK_OFFSET_CMD_TYPE])
		{
			case SPI_CMD_SET_CHANNAL:
				if((SPI.DATA[TmpPos].Data[SPI_LINK_OFFSET_CMD_DATA]&0x7F) <= 125)			
				{
					RADIO.TxChannal = SPI.DATA[TmpPos].Data[SPI_LINK_OFFSET_CMD_DATA];		// 发送频点
					RADIO.TxPower = SPI.DATA[TmpPos].Data[SPI_LINK_OFFSET_CMD_DATA+1];		// 发送功率和发送方式
					FLASH_WriteAppData();							// 相关参数写入FLASH
					
					nrf_esb_set_rf_channel(RADIO.TxChannal);															
				}
				
				SPI.DATA[TmpPos].State = SPI_DATA_INVALID;
				
				spi_slave_tx_buffers_init(SPI_CMD_SET_CHANNAL);	
				SPI.SpiTriggerIrqFlg = true;							
				break;
			case SPI_CMD_GET_STATE:
				SPI.DATA[TmpPos].State = SPI_DATA_INVALID;
				spi_slave_tx_buffers_init(SPI_CMD_GET_STATE);
				break;
			case SPI_CMD_SEND_24G_DATA:
				switch(SPI.DATA[TmpPos].Data[SPI_LINK_OFFSET_CMD_DATA])
				{
					case RADIO_TYPE_USE_NEED_PRE:
					case RADIO_TYPE_USE_NEEDLESS_PRE:	// 有效数据，按顺序发送
						SPI.DATA[TmpPos].State = SPI_DATA_24G;		// 更新缓存数据的状态标志位，供下层（2.4G发送层）进行处理
					
						SPI._24gEnterPos = SPI.SpiExitPos;
						spi_slave_tx_buffers_init(SPI_CMD_SEND_24G_DATA);														
						break;
					case RADIO_TYPE_INSTANT_ACK:		// ACK，立即发送
						SPI.DATA[TmpPos].State = SPI_DATA_INVALID;
						
						RADIO_SetTxPower();
						nrf_esb_set_rf_channel(SPI.DATA[TmpPos].Data[SPI_LINK_OFFSET_CMD_DATA+1]);
						
						tx_payload.length = SPI.DATA[TmpPos].Data[SPI_LINK_OFFSET_CMD_LEN]-2;
						memcpy(tx_payload.data, &SPI.DATA[TmpPos].Data[SPI_LINK_OFFSET_CMD_DATA+2], tx_payload.length);

						nrf_esb_write_payload(&tx_payload);
//						SBQ_DEBUG_PIN_SET();
						break;
					case RADIO_TYPE_INSTANT_USE:		// 有效数据，立即发送
						SPI.DATA[TmpPos].State = SPI_DATA_INVALID;
						
						RADIO_SetTxPower();
						nrf_esb_set_rf_channel(SPI.DATA[TmpPos].Data[SPI_LINK_OFFSET_CMD_DATA+1]);
					
						tx_payload.length = SPI.DATA[TmpPos].Data[SPI_LINK_OFFSET_CMD_LEN]-2;
						memcpy(tx_payload.data, &SPI.DATA[TmpPos].Data[6], tx_payload.length);
						
						nrf_esb_write_payload(&tx_payload);
//						SBQ_DEBUG_PIN_SET();
						break;
					default:
						break;
				}
				spi_slave_tx_buffers_init(SPI_CMD_SEND_24G_DATA);
				SPI.SpiTriggerIrqFlg = true;
				break;
			default:
				break;
		}// switch END
		
		if(SPI.SpiTriggerIrqFlg)
		{
			SPI.SpiTriggerIrqFlg = false;
			spi_trigger_irq();			
		}		
		
	}// if END
}





