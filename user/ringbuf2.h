#ifndef __RINGBUF2_H
#define	__RINGBUF2_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "nrf_delay.h"
 
#define 	RINGBUF_STATUS_EMPTY       			(1)
#define 	RINGBUF_STATUS_USEING         		(2)
#define 	RINGBUF_STATUS_FULL           		(3) 
 
// SPI
#define		RINGBUF_MAX_NUM_SPI		(10)
#define		RINGBUF_LENGTH_SPI		(255)

// 2.4G·¢ËÍ
#define		RINGBUF_MAX_NUM_NRF		(10)
#define		RINGBUF_LENGTH_NRF		(255)

typedef struct
{
	uint8_t		length;
	uint8_t		data[RINGBUF_LENGTH_SPI];		
}SPI_RINGBUF_STORE_T;

uint8_t RINGBUF_GetStatus(void);
void RINGBUF_WriteData(uint8_t *buff, uint8_t buff_len);
void RINGBUF_ReadData(uint8_t *buff, uint8_t *buff_len);
void RINGBUF_UseRate(void);


#endif 

