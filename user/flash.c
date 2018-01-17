// Revision record ----------------------------------------------------
/*
	NRF51系列的flash分成CODESIZE页，每页占CODEPAGESIZE字节
	整个flash空间（代码空间）共CODEPAGESIZE * CODESIZE 字节
	
	答题器所用nrf51822共有256KByte，其中
	CODESIZE = 256
	CODEPAGESIZE = 1024
	
	答题器代码flash用不完，所以可以取最后一页的1KByte空间用于存储想要的信息
	
	注意：
		1）flash读写单位为word（4Byte），我尝试每次写入1Byte后，会出现硬件错误	
*/


// Includes -----------------------------------------------------------
#include "flash.h"

// Functions ----------------------------------------------------------
void flash_page_erase(uint32_t * page_address);
void flash_word_write(uint32_t * address, uint32_t value);

// Globals ------------------------------------------------------------
FLASH_PARAMETERS_T 		FLASH;

// Locals -------------------------------------------------------------

void FLASH_Init(void)
{
	// 使用51822最后一页flash来存储用户数据
	FLASH.UserAddr = (uint32_t *)(NRF_FICR->CODEPAGESIZE * (NRF_FICR->CODESIZE - 1));	
	
	FLASH_ReadAppData();
}


// 向Flash中写入应用层（配对）数据
// 注：Flash读写单位是字（4Byte）
// 以小端模式存放，uid0~3= 0x12 0x34 0x56 0x78，则存放方式为：0x78563412
void FLASH_WriteAppData(void)
{
	uint32_t TmpWord;
	
	// FLASH每次写入前必须先擦除
	flash_page_erase(FLASH.UserAddr);	

	// 答题器接收频点
	TmpWord = RADIO.TxChannal & 0x7F;
	flash_word_write(FLASH.UserAddr+0, TmpWord);
	
	// 发送功率
	TmpWord = RADIO.TxPower;
	flash_word_write(FLASH.UserAddr+1, TmpWord);	
}

// 从Flash中读取（更新）应用层数据
void FLASH_ReadAppData(void)
{
	uint32_t TmpWord;
	
	// 接收频点
	TmpWord = *(FLASH.UserAddr+0);
	if((uint8_t)(TmpWord) < 125)
	{
		RADIO.TxChannal = (uint8_t)(TmpWord);
	}
	else
	{
		RADIO.TxChannal = 4;
	}
	
	// 发送功率
	TmpWord = *(FLASH.UserAddr+1);
	if(TmpWord >= 1 &&  TmpWord <= 5)
		RADIO.TxPower = (uint8_t)TmpWord;
	else
		RADIO.TxPower = 2;
}



/** @brief Function for erasing a page in flash.
 *
 * @param page_address Address of the first word in the page to be erased.
 */
void flash_page_erase(uint32_t * page_address)
{
    // Turn on flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Erase page:
    NRF_NVMC->ERASEPAGE = (uint32_t)page_address;

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
}


/** @brief Function for filling a page in flash with a value.
 *
 * @param[in] address Address of the first word in the page to be filled.
 * @param[in] value Value to be written to flash.
 */
void flash_word_write(uint32_t * address, uint32_t value)
{
    // Turn on flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);
	
    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    *address = value;

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
}







 
 

 
 
