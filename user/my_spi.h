#ifndef _MY_SPI_H
#define _MY_SPI_H


#include "main.h"

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
#define SPI_LINK_OFFSET_HEAD		(0)
#define SPI_LINK_OFFSET_DEV_ID		(1)
#define SPI_LINK_OFFSET_CMD_TYPE	(2)
#define SPI_LINK_OFFSET_CMD_LEN		(3)
#define SPI_LINK_OFFSET_CMD_DATA	(4)

// SPI开辟缓存的长度和数量
#define SPI_MAX_BUFFER_SIZE (255)		
#define SPI_MAX_CACHE_SIZE	(10)

#define TX_BUF_SIZE   (255)           	  /**< SPI TX buffer size. */      
#define RX_BUF_SIZE   TX_BUF_SIZE       /**< SPI RX buffer size. */  

#define SPIS_IRQ_PIN   0     
#define SPIS_CE_PIN    1
#define SPIS_MOSI_PIN  2    // SPI MOSI signal. 
#define SPIS_MISO_PIN  3    // SPI MISO signal. 
#define SPIS_SCK_PIN   4    // SPI SCK signal.
#define SPIS_CSN_PIN   5    // SPI CSN signal. 


typedef enum
{
	SPI_CMD_NONE = 0x00,
	SPI_CMD_SET_CHANNAL = 0x20,
	SPI_CMD_GET_STATE = 0x21,
	SPI_CMD_GET_24G_DATA = 0x22,
	SPI_CMD_SEND_24G_DATA = 0x23
}SPI_CMD_TYPE;

typedef struct
{
	uint8_t			Head;
	uint8_t 		DevId;
	uint8_t			CmdType;
	uint8_t			CmdLen;
	uint8_t			CmdData[255];
	uint8_t			Xor;
	uint8_t			End;
}SPI_CMD_FORMAT_T;

typedef enum
{
	SPI_DATA_NULL,		// 初始化状态
	SPI_DATA_NEW,		// 新的SPI数据
	SPI_DATA_24G,		// 需要2.4G层处理的数据
	SPI_DATA_INVALID	// 处理完成的数据，可存入新的数据
}SPI_DATA_HANDLER_STATE;

typedef struct 
{
	uint8_t 				Data[SPI_MAX_BUFFER_SIZE];		// SPI层原始数据
	uint8_t 				DataLen;						
	SPI_DATA_HANDLER_STATE   State;			// 数据处理状态：
											// 		0：未处理过
											// 		1：SPI层处理完成，需2.4G发送层处理
											//		3：废弃
}SPI_DATA_T;

typedef struct
{
	uint32_t SpiEnterPos;	// SPI层处理位置	
	uint32_t SpiExitPos;
	uint32_t _24gEnterPos;	// 2.4G层处理位置
	uint32_t _24gExitPos;	
	
	SPI_DATA_T				DATA[SPI_MAX_CACHE_SIZE];
	
	SPI_CMD_FORMAT_T		RX;
	bool					SpiTriggerIrqFlg;
}SPI_PARAMETERS_T;


extern uint8_t tx_data_len;
extern uint8_t m_tx_buf[TX_BUF_SIZE];   /**< SPI TX buffer. */      
extern uint8_t m_rx_buf[RX_BUF_SIZE];   /**< SPI RX buffer. */ 

extern SPI_PARAMETERS_T		SPI;


void my_spi_slave_init(void);
void spi_gpio_init(void);
void spi_trigger_irq(void);
void SPI_DataHandler(void);
	
#endif


