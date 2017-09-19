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
#define      NRF_DEFAULT_TX_CHANNAL				(4)
#define      NRF_DEFAULT_TX_POWER				(NRF_ESB_TX_POWER_4DBM)
#define		 NRF_LINK_DATA_LEN				(17)		// ��·�㶨�����ݳ���


#define 	NRF_RX_DEV_ID						(0x01)
#define 	NRF_TX_DEV_ID						(0x02)


typedef enum
{
	RADIO_TYPE_USE_NEED_PRE = 1,			// ��Ч������Ҫ����ǰ��֡
	RADIO_TYPE_USE_NEEDLESS_PRE = 2,		// ��Ч��������ǰ��֡
	RADIO_TYPE_INSTANT_ACK = 3,				// �����ȷ��͵�ACK
	RADIO_TYPE_INSTANT_USE = 4				// �����ȷ��͵���Ч����
}RADIO_LINK_DATA_TYPE;		// ��·����������

typedef struct
{
	uint8_t			Len;			// ACK����
	uint8_t			Data[252];		// ���������UID��ÿ��ռ4�ֽ�
}TX_PARAMETERS_T;


typedef struct
{
	uint8_t 				TxChannal;
	uint8_t					TxPower;
	
	bool					BusyFlg;	

	bool					HardTxBusyFlg;		// Ӳ��������Դ����ռ�ñ�־
		
	uint8_t					PreCnt;				// ����ǰ��֡����
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




