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
#define      NRF_DEFAULT_TX_POWER				(NRF_ESB_TX_POWER_4DBM)
#define		 NRF_LINK_DATA_LEN				(17)		// 链路层定义数据长度


#define 	NRF_RX_DEV_ID						(0x01)
#define 	NRF_TX_DEV_ID						(0x02)


typedef struct
{
	uint8_t			Num;				// ACK数量
	uint8_t			Uid[40];			// 保存答题器UID，每个占4字节
	
	uint8_t 		SrcId[4];			// 源ID
	uint8_t			DstId[4];			// 目标ID
}ACK_PARAMETERS_T;

typedef struct
{
	uint8_t			Len;			// ACK数量
	uint8_t			Data[252];		// 保存答题器UID，每个占4字节
}TX_PARAMETERS_T;



typedef struct
{	
	uint8_t Len;					//Head~End的所有有效数据长度
	
	uint8_t Head;					//包头
	uint8_t DstId[4];				//目标UID
	uint8_t SrcId[4];				//源UID
	uint8_t DevId;					//设备UID
	uint8_t ProVer;					//协议版本号
	uint8_t SeqNum;					//帧号
	uint8_t PackNum;				//包号
	uint8_t ExtendLen;				//扩展字节长度
	uint8_t ExtendData[5];			//扩展字节内容
	uint8_t	PackLen;				//包长
	uint8_t PackData[240];			//包内容，存储应用层的命令
	uint8_t Xor;					//不包括包头包尾和自身的异或校验结果
	uint8_t End;					//包尾
}NRF_LINK_LAYER_T;

typedef struct
{
	uint8_t 				TxChannal;
	uint8_t					TxPower;
	
	bool					BusyFlg;
	
	uint8_t					PreCnt;			// 发送前导帧计数
	ACK_PARAMETERS_T		ACK;
	TX_PARAMETERS_T			TX;
}RADIO_PARAMETERS_T;

extern nrf_esb_payload_t        tx_payload;
extern nrf_esb_payload_t        ack_payload;

extern RADIO_PARAMETERS_T 		RADIO;


extern uint32_t my_tx_esb_init(void);
extern uint32_t my_rx_esb_init(void);
extern void RADIO_Init(void);
extern void RADIO_SendAck(uint8_t* UidBuf, uint8_t UidNum, uint32_t TxChannal);
extern void RADIO_SendData(uint8_t* DataBuf, uint8_t DataLen, uint8_t TxChannal);
#endif 




