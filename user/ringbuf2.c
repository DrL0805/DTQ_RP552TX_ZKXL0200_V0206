#include "ringbuf2.h"

uint8_t pRead_Spi = 0;
uint8_t pWrite_Spi = 0;
uint8_t BufStatus_Spi = RINGBUF_STATUS_EMPTY;

SPI_RINGBUF_STORE_T		store_buff[RINGBUF_MAX_NUM_SPI]; 	


uint8_t RINGBUF_GetStatus(void)
{
	return BufStatus_Spi;
}
	
void RINGBUF_WriteData(uint8_t *buff, uint8_t buff_len)
{
	store_buff[pWrite_Spi].length = buff_len;
	memcpy(store_buff[pWrite_Spi].data, buff, buff_len);
	
	if(pWrite_Spi < (RINGBUF_MAX_NUM_SPI - 1))
		pWrite_Spi++;
	else
		pWrite_Spi = 0;
	
	if(pWrite_Spi < (RINGBUF_MAX_NUM_SPI - 1))
	{
		if((pWrite_Spi+1) == pRead_Spi)	//写的下一个是读，表示已满
			BufStatus_Spi = RINGBUF_STATUS_FULL;
		else
			BufStatus_Spi = RINGBUF_STATUS_USEING;
	}
	else		
	{
		if(pRead_Spi == 0)				//写的下一个是读，表示已满
			BufStatus_Spi = RINGBUF_STATUS_FULL;
		else
			BufStatus_Spi = RINGBUF_STATUS_USEING;
	}
}

void RINGBUF_ReadData(uint8_t *buff, uint8_t *buff_len)
{
//	uint8_t temp_buff_len;
//	uint8_t temp_buff[RINGBUF_LENGTH_SPI];
	*buff_len = store_buff[pRead_Spi].length;
	memcpy(buff, store_buff[pRead_Spi].data, *buff_len);
	
	if(pRead_Spi < (RINGBUF_MAX_NUM_SPI - 1))
		pRead_Spi++;
	else
		pRead_Spi = 0;
	
	if(pRead_Spi == pWrite_Spi)			//写等于读，表示空
		BufStatus_Spi = RINGBUF_STATUS_EMPTY;
	else
		BufStatus_Spi = RINGBUF_STATUS_USEING;
}

//获取当前ringbuf使用情况
void RINGBUF_UseRate(void)
{
	uint8_t ringbuf_use_num = 0;
	
	if((pRead_Spi == pWrite_Spi) || (pRead_Spi < pWrite_Spi))
	{
		ringbuf_use_num = pWrite_Spi - pRead_Spi;
	}
	else
	{
		ringbuf_use_num = RINGBUF_MAX_NUM_SPI - pRead_Spi + pWrite_Spi;
	}
	printf("%d \r\n",ringbuf_use_num);
}

