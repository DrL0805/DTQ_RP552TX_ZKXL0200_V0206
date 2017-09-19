#ifndef __MY_RADIO_H
#define	__MY_RADIO_H


#include "main.h"

#define 	 NRF_MAX_NUMBER_OF_RETRANSMITS		(3)					//最大重发次数
#define 	 NRF_PIPE							(0)					//2.4G通道、
#define 	 NRF_DATA_HEAD						(0x61)
#define      NRF_DATA_END                   	(0x21)
#define      NRF_DATA_RESERVE					(0xFF)			  	//保留位默认值
#define 	 NRF_PRE_TX_NUMBER					(110)				//前导帧发送次数
#define 	 NRF_ENHANCE_TX_NUM					(2)					// 加强帧发送次数（总发送次数）
#define      NRF_DEFAULT_TX_CHANNAL				(4)
#define      NRF_DEFAULT_TX_POWER				(NRF_ESB_TX_POWER_4DBM)
#define		 NRF_LINK_DATA_LEN				(17)		// 链路层定义数据长度


#define 	NRF_RX_DEV_ID						(0x01)
#define 	NRF_TX_DEV_ID						(0x02)


typedef enum
{
	RADIO_TYPE_USE_NEED_PRE = 1,			// 有效数据需要发送前导帧
	RADIO_TYPE_USE_NEEDLESS_PRE = 2,		// 有效数据无需前导帧
	RADIO_TYPE_INSTANT_ACK = 3,				// 需优先发送的ACK
	RADIO_TYPE_INSTANT_USE = 4				// 需优先发送的有效数据
}RADIO_LINK_DATA_TYPE;		// 链路层数据类型

typedef struct
{
	uint8_t			Len;			// ACK数量
	uint8_t			Data[252];		// 保存答题器UID，每个占4字节
}TX_PARAMETERS_T;


typedef struct
{
	uint8_t 				TxChannal;
	uint8_t					TxPower;
	
	bool					BusyFlg;	

	bool					HardTxBusyFlg;		// 硬件发送资源正在占用标志
		
	uint8_t					PreCnt;				// 发送前导帧计数
	TX_PARAMETERS_T			TX;
}RADIO_PARAMETERS_T;

extern nrf_esb_payload_t        tx_payload;
extern nrf_esb_payload_t        ack_payload;
extern RADIO_PARAMETERS_T 		RADIO;

extern uint32_t my_tx_esb_init(void);
extern void RADIO_Init(void);
extern void RADIO_SendAck(uint8_t* UidBuf, uint8_t UidNum, uint32_t TxChannal);
void RADIO_SendHandler(void);
#endif 




