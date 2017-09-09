#ifndef __MY_RADIO_H
#define	__MY_RADIO_H


#include "main.h"

#define 	 NRF_MAX_NUMBER_OF_RETRANSMITS		(3)					//����ط�����
#define 	 NRF_PIPE							(0)					//2.4Gͨ����
#define 	 NRF_DATA_HEAD						(0x61)
#define      NRF_DATA_END                   	(0x21)
#define      NRF_DATA_RESERVE					(0xFF)			  	//����λĬ��ֵ
#define 	 NRF_PRE_TX_NUMBER					(110)				//ǰ��֡���ʹ���
#define 	 NRF_ENHANCE_TX_NUM					(2)					// ��ǿ֡���ʹ������ܷ��ʹ�����
#define      NRF_DEFAULT_TX_POWER				(NRF_ESB_TX_POWER_4DBM)
#define		 NRF_LINK_DATA_LEN				(17)		// ��·�㶨�����ݳ���


#define 	NRF_RX_DEV_ID						(0x01)
#define 	NRF_TX_DEV_ID						(0x02)


typedef struct
{
	uint8_t			Num;				// ACK����
	uint8_t			Uid[40];			// ���������UID��ÿ��ռ4�ֽ�
	
	uint8_t 		SrcId[4];			// ԴID
	uint8_t			DstId[4];			// Ŀ��ID
}ACK_PARAMETERS_T;

typedef struct
{
	uint8_t			Len;			// ACK����
	uint8_t			Data[252];		// ���������UID��ÿ��ռ4�ֽ�
}TX_PARAMETERS_T;



typedef struct
{	
	uint8_t Len;					//Head~End��������Ч���ݳ���
	
	uint8_t Head;					//��ͷ
	uint8_t DstId[4];				//Ŀ��UID
	uint8_t SrcId[4];				//ԴUID
	uint8_t DevId;					//�豸UID
	uint8_t ProVer;					//Э��汾��
	uint8_t SeqNum;					//֡��
	uint8_t PackNum;				//����
	uint8_t ExtendLen;				//��չ�ֽڳ���
	uint8_t ExtendData[5];			//��չ�ֽ�����
	uint8_t	PackLen;				//����
	uint8_t PackData[240];			//�����ݣ��洢Ӧ�ò������
	uint8_t Xor;					//��������ͷ��β����������У����
	uint8_t End;					//��β
}NRF_LINK_LAYER_T;

typedef struct
{
	uint8_t 				TxChannal;
	uint8_t					TxPower;
	
	bool					BusyFlg;
	
	uint8_t					PreCnt;			// ����ǰ��֡����
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




