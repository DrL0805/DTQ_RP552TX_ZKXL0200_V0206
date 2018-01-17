#ifndef __FLASH_H
#define __FLASH_H	 

// Includes -----------------------------------------------------------
//#include "nrf_gpio.h"
#include "main.h"

// Defines ------------------------------------------------------------

/*
	Flash�洢��ַ���䣬Ϊ��д���㣬ÿ����Ϣ����ռ1�֣�4Byte����
	0x3FC00~0x3FC03������ַ+0��������UID			
	0x3FC04~0x3FC07������ַ+4��������UID			
	0x3FC08~0x3FC0B������ַ+8��������������		
	0x3FC0C~0x3FC0F������ַ+12������������Ƶ��
	0x3FC10~0x3FC13������ַ+16������������Ƶ��
	
	
*/

// ��ַƫ�ƣ���λΪword��4Byte��
#define		OFFSET_ADDR_JSQ_UID				(0)
#define		OFFSET_ADDR_DTQ_UID				(OFFSET_ADDR_JSQ_UID+1)
#define		OFFSET_ADDR_DTQ_SERIAL			(OFFSET_ADDR_DTQ_UID+1)
#define		OFFSET_ADDR_TX_CHANNAL			(OFFSET_ADDR_DTQ_SERIAL+1)
#define		OFFSET_ADDR_RX_CHANNAL			(OFFSET_ADDR_TX_CHANNAL+1)




// Typedefs -----------------------------------------------------------
typedef struct 
{
//	uint32_t 		PageSize;			// == 2014
//	uint32_t		PageNum;			// == 256
	uint32_t*		UserAddr;			// �洢�û���Ϣ�ĵ�ַ��Ϊ51822���һҳflash����ַΪ0x3FC00����1K
}FLASH_PARAMETERS_T;

// Functions ----------------------------------------------------------
extern void FLASH_Init(void);
extern void FLASH_WriteAppData(void);
extern void FLASH_ReadAppData(void);
extern void flash_page_erase(uint32_t * page_address);
extern void flash_word_write(uint32_t * address, uint32_t value);
// Globals ------------------------------------------------------------
extern FLASH_PARAMETERS_T 		FLASH;

		 				    
#endif














