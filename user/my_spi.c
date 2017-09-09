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

/**@brief Function for initializing buffers.
 *
 * @param[in] p_tx_buf  Pointer to a transmit buffer.
 * @param[in] p_rx_buf  Pointer to a receive  buffer.
 * @param[in] len       Buffers length.
 */
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
	uint32_t i = 0;
	
	uint32_t TmpChannal = 0;
	
	switch(event.evt_type)
	{
		case NRF_DRV_SPIS_BUFFERS_SET_DONE:
			break;
		case NRF_DRV_SPIS_XFER_DONE:

			if( (0x86              == m_rx_buf[0])       &&
				(0x76              == m_rx_buf[event.rx_amount - 1]) &&
				m_rx_buf[event.rx_amount - 2] == XOR_Cal(m_rx_buf+1, event.rx_amount - 3) )
			{
				SPI.RX.Head 		= m_rx_buf[0];
				SPI.RX.DevId 		= m_rx_buf[1];
				SPI.RX.CmdType 		= m_rx_buf[2];
				SPI.RX.CmdLen 		= m_rx_buf[3];
				memcpy(SPI.RX.CmdData, m_rx_buf+4, SPI.RX.CmdLen);
				SPI.RX.Xor 			= m_rx_buf[4+SPI.RX.CmdLen];
				SPI.RX.End 			= m_rx_buf[5+SPI.RX.CmdLen];

				switch(SPI.RX.CmdType)
				{
					case SPI_CMD_SET_CHANNAL:					
						if((SPI.RX.CmdData[0]&0x7F) <= 125)
						{
							RADIO.TxChannal = SPI.RX.CmdData[0];
							RADIO.TxPower = SPI.RX.CmdData[1];
							do
							{
								nrf_esb_set_rf_channel(RADIO.TxChannal);	
								nrf_esb_get_rf_channel(&TmpChannal);	
							}while((TmpChannal != RADIO.TxChannal) && (++i < 0x0FFF));													
							spi_slave_tx_buffers_init(SPI_CMD_SET_CHANNAL);	
							spi_trigger_irq();				
						}					
						break;
					case SPI_CMD_GET_STATE:						
						spi_slave_tx_buffers_init(SPI_CMD_GET_STATE);	
						break;
					case SPI_CMD_SEND_24G_DATA:	
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
						switch(SPI.RX.CmdData[0])
						{
							case 0x01:
							case 0x02:
								if(get_ringbuf_status() != BUFF_FULL)
								{
									ringbuf_write_data(m_rx_buf,event.rx_amount);
									spi_slave_tx_buffers_init(SPI_CMD_SEND_24G_DATA);														
								}
								break;
							case 0x03:
								RADIO_SendAck(SPI.RX.CmdData+20, 1, SPI.RX.CmdData[1]);
								break;							
							case 0x04:
								RADIO_SendData(SPI.RX.CmdData+2, SPI.RX.CmdLen - 2, SPI.RX.CmdData[1]);
								break;
							default:
								break;
						}
						spi_slave_tx_buffers_init(SPI_CMD_SEND_24G_DATA);
						spi_trigger_irq();			
						break;
					default:
						break;
				}					
			}
			// SPIS每次收发完数据后需要重新SET下
			nrf_drv_spis_buffers_set(&spis,m_tx_buf,TX_BUF_SIZE,m_rx_buf,RX_BUF_SIZE);
			break;
		case NRF_DRV_SPIS_EVT_TYPE_MAX:
			
			break;
		default:
			break;
	}
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

void spi_rx_data_handler(void)
{
	uint32_t TmpChannal, i;
	uint8_t tmp_ringbuf_len = 0;
	uint8_t tmp_ringbuf_buf[255];
//	SPI_PARAMETERS_T		TmpSpi;
	
	// 从缓冲区中读取收到的SPI数据
	ringbuf_read_data(tmp_ringbuf_buf,&tmp_ringbuf_len);
	
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
	RADIO.TX.Len = tmp_ringbuf_buf[3] - 2;
	memcpy(RADIO.TX.Data, tmp_ringbuf_buf+6, RADIO.TX.Len);

	RADIO.TxChannal  = tmp_ringbuf_buf[5];
	
	// 设置发送的频点
	do
	{
		nrf_esb_set_rf_channel(RADIO.TxChannal);	
		nrf_esb_get_rf_channel(&TmpChannal);	
	}while((TmpChannal != RADIO.TxChannal) && (++i < 0x0FFF));		
	
	switch(tmp_ringbuf_buf[4])
	{
		case 0x01:	// 有效数据，需发前导帧
			RADIO.BusyFlg = true;
			RADIO.PreCnt = 0;
			nrf_transmit_timeout_start(1);
			break;
		case 0x02:	// 有效数据，无需发前导帧 
			RADIO.BusyFlg = true;
			RADIO.PreCnt = NRF_PRE_TX_NUMBER;
			nrf_transmit_timeout_start(1);
			break;
		case 0x03:	// ACK
			RADIO.BusyFlg = true;
			RADIO.PreCnt = NRF_PRE_TX_NUMBER;
			nrf_transmit_timeout_start(1);
			break;
		default:
			break;
	}	
}








