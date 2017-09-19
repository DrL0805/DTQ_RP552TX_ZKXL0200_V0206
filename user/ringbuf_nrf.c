#include "ringbuf_nrf.h"

uint8_t pRead_nRF = 0;
uint8_t pWrite_nRF = 0;
uint8_t BufStatus_nRF = RINGBUF_STATUS_EMPTY_nRF;

nRF_RINGBUF_STORE_T		nRF_Ringbuff[RINGBUF_MAX_NUM_nRF]; 	


uint8_t RINGBUF_GetStatus_nRF(void)
{
	return BufStatus_nRF;
}
	
void RINGBUF_WriteData_nRF(uint8_t *buff, uint8_t buff_len, uint8_t Channal)
{
	nRF_Ringbuff[pWrite_nRF].length = buff_len;
	nRF_Ringbuff[pWrite_nRF].Channal = Channal;
	memcpy(nRF_Ringbuff[pWrite_nRF].data, buff, buff_len);
	
	if(pWrite_nRF < (RINGBUF_MAX_NUM_nRF - 1))
		pWrite_nRF++;
	else
		pWrite_nRF = 0;
	
	if(pWrite_nRF < (RINGBUF_MAX_NUM_nRF - 1))
	{
		if((pWrite_nRF+1) == pRead_nRF)	//写的下一个是读，表示已满
			BufStatus_nRF = RINGBUF_STATUS_FULL_nRF;
		else
			BufStatus_nRF = RINGBUF_STATUS_USEING_nRF;
	}
	else		
	{
		if(pRead_nRF == 0)				//写的下一个是读，表示已满
			BufStatus_nRF = RINGBUF_STATUS_FULL_nRF;
		else
			BufStatus_nRF = RINGBUF_STATUS_USEING_nRF;
	}
}

void RINGBUF_ReadData_nRF(uint8_t *buff, uint8_t *buff_len, uint8_t* Channal)
{

	*buff_len = nRF_Ringbuff[pRead_nRF].length;
	*Channal = nRF_Ringbuff[pRead_nRF].Channal;
	memcpy(buff, nRF_Ringbuff[pRead_nRF].data, *buff_len);
	
	if(pRead_nRF < (RINGBUF_MAX_NUM_nRF - 1))
		pRead_nRF++;
	else
		pRead_nRF = 0;
	
	if(pRead_nRF == pWrite_nRF)			//写等于读，表示空
		BufStatus_nRF = RINGBUF_STATUS_EMPTY_nRF;
	else
		BufStatus_nRF = RINGBUF_STATUS_USEING_nRF;
}

//获取当前ringbuf使用情况
void RINGBUF_UseRate_nRF(void)
{
	uint8_t ringbuf_use_num = 0;
	
	if((pRead_nRF == pWrite_nRF) || (pRead_nRF < pWrite_nRF))
	{
		ringbuf_use_num = pWrite_nRF - pRead_nRF;
	}
	else
	{
		ringbuf_use_num = RINGBUF_MAX_NUM_nRF - pRead_nRF + pWrite_nRF;
	}
	printf("%d \r\n",ringbuf_use_num);
}

