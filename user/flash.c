// Revision record ----------------------------------------------------
/*
	NRF51ϵ�е�flash�ֳ�CODESIZEҳ��ÿҳռCODEPAGESIZE�ֽ�
	����flash�ռ䣨����ռ䣩��CODEPAGESIZE * CODESIZE �ֽ�
	
	����������nrf51822����256KByte������
	CODESIZE = 256
	CODEPAGESIZE = 1024
	
	����������flash�ò��꣬���Կ���ȡ���һҳ��1KByte�ռ����ڴ洢��Ҫ����Ϣ
	
	ע�⣺
		1��flash��д��λΪword��4Byte�����ҳ���ÿ��д��1Byte�󣬻����Ӳ������	
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
	// ʹ��51822���һҳflash���洢�û�����
	FLASH.UserAddr = (uint32_t *)(NRF_FICR->CODEPAGESIZE * (NRF_FICR->CODESIZE - 1));	
	
	FLASH_ReadAppData();
}


// ��Flash��д��Ӧ�ò㣨��ԣ�����
// ע��Flash��д��λ���֣�4Byte��
// ��С��ģʽ��ţ�uid0~3= 0x12 0x34 0x56 0x78�����ŷ�ʽΪ��0x78563412
void FLASH_WriteAppData(void)
{
	uint32_t TmpWord;
	
	// FLASHÿ��д��ǰ�����Ȳ���
	flash_page_erase(FLASH.UserAddr);	

	// ����������Ƶ��
	TmpWord = RADIO.TxChannal & 0x7F;
	flash_word_write(FLASH.UserAddr+0, TmpWord);
	
	// ���͹���
	TmpWord = RADIO.TxPower;
	flash_word_write(FLASH.UserAddr+1, TmpWord);	
}

// ��Flash�ж�ȡ�����£�Ӧ�ò�����
void FLASH_ReadAppData(void)
{
	uint32_t TmpWord;
	
	// ����Ƶ��
	TmpWord = *(FLASH.UserAddr+0);
	if((uint8_t)(TmpWord) < 125)
	{
		RADIO.TxChannal = (uint8_t)(TmpWord);
	}
	else
	{
		RADIO.TxChannal = 4;
	}
	
	// ���͹���
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







 
 

 
 
