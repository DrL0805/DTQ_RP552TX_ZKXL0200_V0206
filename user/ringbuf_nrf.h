#ifndef __RINGBUF_NRF_H
#define	__RINGBUF_NRF_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "nrf_delay.h"
 
#define 	RINGBUF_STATUS_EMPTY_nRF       			(1)
#define 	RINGBUF_STATUS_USEING_nRF         		(2)
#define 	RINGBUF_STATUS_FULL_nRF           		(3) 

// 2.4G·¢ËÍ
#define		RINGBUF_MAX_NUM_nRF		(10)
#define		RINGBUF_LENGTH_nRF		(256)

typedef struct
{
	uint8_t		length;
	uint8_t 	Channal;					// Æµµã
	uint8_t		data[RINGBUF_LENGTH_nRF];
}nRF_RINGBUF_STORE_T;

uint8_t RINGBUF_GetStatus_nRF(void);
void RINGBUF_WriteData_nRF(uint8_t *buff, uint8_t buff_len, uint8_t Channal);
void RINGBUF_ReadData_nRF(uint8_t *buff, uint8_t *buff_len, uint8_t* Channal);
void RINGBUF_UseRate_nRF(void);


#endif 


