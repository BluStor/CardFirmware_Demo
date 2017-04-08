/**
  ******************************************************************************
  * @file    LwIP/LwIP_IAP/Src/flash_if.c 
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    14-August-2015
  * @brief   This file provides high level routines to manage internal Flash 
  *          programming (erase and write). 
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */
void FLASH_If_Unlock(void)
{ 
   HAL_FLASH_Unlock(); 
}

/**
  * @brief  Locks Flash for write access
  * @param  None
  * @retval None
  */
void FLASH_If_Lock(void)
{ 
   HAL_FLASH_Lock(); 
}


/**
  * @brief  This function does an erase of all user flash area
  * @param  StartSector: start of user flash area
  * @retval 0: user flash area successfully erased
  *         1: error occurred
  */
int8_t FLASH_If_Erase(uint32_t StartSector)
{
  uint32_t FlashAddress;
  FLASH_EraseInitTypeDef FLASH_EraseInitStruct;
  uint32_t sectornb = 0;
  
  FlashAddress = StartSector;

  // db??? to investigate
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | \
                           FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_RDERR);
  /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
     be done by word */ 

  FLASH_EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
//#ifdef VDD_1_8
  FLASH_EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_1;
//#else
//  FLASH_EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
//#endif // VDD_1_8
  
  switch(FlashAddress)
  {
#ifdef BOOT_48K
  case USER_BOOTLOADER_FIRST_PAGE_ADDRESS_48K:          // erase sector 0,1 and 2.
    FLASH_EraseInitStruct.Sector = FLASH_SECTOR_0;
    FLASH_EraseInitStruct.NbSectors = 3;  
    break;
  case USER_SAVE_PARAM_ADDRESS_48K:                     // erase sector 3
    FLASH_EraseInitStruct.Sector = FLASH_SECTOR_3;
    FLASH_EraseInitStruct.NbSectors = 1;  
    break;
  case USER_FIRMWARE_FIRST_PAGE_ADDRESS_48K:            // erase sector 4 to 11
    FLASH_EraseInitStruct.Sector = FLASH_SECTOR_4;
    FLASH_EraseInitStruct.NbSectors = 8;  
    break;
#else    
  case USER_SAVE_PARAM_ADDRESS_32K:                     // erase sector 2
    FLASH_EraseInitStruct.Sector = FLASH_SECTOR_2;
    FLASH_EraseInitStruct.NbSectors = 1;  
    break;
  case USER_FIRMWARE_FIRST_PAGE_ADDRESS_32K:            // erase sector 3 to 11
    FLASH_EraseInitStruct.Sector = FLASH_SECTOR_3;
    FLASH_EraseInitStruct.NbSectors = 9;  
    break;
#endif // BOOT_48K
  default:
    return (1);
  }

  if (HAL_FLASHEx_Erase(&FLASH_EraseInitStruct, &sectornb) != HAL_OK)
    return (1);
  
  return (0);
}
/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  FlashAddress: start address for writing data buffer
  * @param  Data: pointer on data buffer
  * @param  DataLength: length of data buffer (unit is 32-bit word)   
  * @retval 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
//uint32_t FLASH_If_Write32(__IO uint32_t* FlashAddress, uint32_t* Data ,uint16_t DataLength)
//{
//  uint32_t i = 0;

  // db??? to investigate
//  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | \
//                           FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_RDERR);
  
//  for (i = 0; (i < DataLength) && (*FlashAddress <= (USER_FIRMWARE_END_ADDRESS-4)); i++)
//  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
//    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, *FlashAddress,  *(uint32_t*)(Data+i)) == HAL_OK)
//    {
     /* Check the written value */
//      if (*(uint32_t*)*FlashAddress != *(uint32_t*)(Data+i))
//      {
        /* Flash content doesn't match SRAM content */
//        return(2);
//      }
      /* Increment FLASH destination address */
//      *FlashAddress += 4;
//    }
//    else
//    {
      /* Error occurred while writing data in Flash memory */
//      return (1);
//    }
//  }

//  return (0);
//}

/**
  * @brief  This function writes a data buffer in flash (data are 8-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  FlashAddress: start address for writing data buffer
  * @param  Data: pointer on data buffer
  * @param  DataLength: length of data buffer (unit is 8-bit word)   
  * @retval 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t FLASH_If_Write8(__IO uint32_t* FlashAddress, uint8_t* Data ,uint32_t DataLength)
{
  uint32_t i = 0;

  // db??? to investigate
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | \
                           FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_RDERR);
  
  for (i = 0; (i < DataLength) && (*FlashAddress <= (USER_FIRMWARE_END_ADDRESS-1)); i++)
  {
    /* Device voltage range supposed to be [1.8V to 2.1V], the operation will
       be done by byte */ 
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, *FlashAddress,  *(uint8_t*)(Data+i)) == HAL_OK)
    {
     /* Check the written value */
      if (*(uint8_t*)*FlashAddress != *(uint8_t*)(Data+i))
      {
        /* Flash content doesn't match SRAM content */
        return(2);
      }
      /* Increment FLASH destination address */
      *FlashAddress += 1;
    }
    else
    {
      /* Error occurred while writing data in Flash memory */
      return (1);
    }
  }

  return (0);
}

void FLASH_If_ReadParam(void)
{
#if 1
  uint8_t* pFlash = (uint8_t*)pFlashParam;
  
  uint8_t* pRam = (uint8_t*)&RamParam;
  int x;
  
#ifdef HYBRID
  if(*pBootloaderMajeur == 1)
  {
    RamParam.BootCmd = CMD_NONE;
    RamParam.DoSelfTest = 0;
    RamParam.Format = 0;
    return;
  }
#endif  
  
  for(x=0;x<sizeof(RamParam);x++)
  {
    pRam[x] = pFlash[x];
  }
  
#else
#ifdef FIRMWARE  
#ifdef FIRMWARE_BINARY
  uint8_t* pFlash = (uint8_t*)pFlashParam;
#else
  uint8_t* pFlash = (uint8_t*)&FlashParam;
#endif // FIRMWARE_BINARY
#else
  uint8_t* pFlash = (uint8_t*)pFlashParam;
#endif
  uint8_t* pRam = (uint8_t*)&RamParam;
  int x;
  
  for(x=0;x<sizeof(RamParam);x++)
  {
    pRam[x] = pFlash[x];
  }
#endif
}

void FLASH_If_SaveParam(void)
{
#ifdef BOOT_48K
  uint32_t FlashWriteAddress = USER_SAVE_PARAM_ADDRESS_48K;
 
#ifdef FIRMWARE  
  if(*pBootloaderMajeur == 1)
  {
    // if we write SaveParam with an old boot we will brike the STM32.
    return;
  }
#endif
  
#else
  uint32_t FlashWriteAddress = USER_SAVE_PARAM_ADDRESS_32K;
#endif
  
  
  
  
  
  FLASH_If_Unlock();
#ifdef BOOT_48K
  FLASH_If_Erase(USER_SAVE_PARAM_ADDRESS_48K);
#else
  FLASH_If_Erase(USER_SAVE_PARAM_ADDRESS_32K);
#endif
//#ifdef VDD_1_8
  FLASH_If_Write8(&FlashWriteAddress, (uint8_t*)&RamParam, sizeof(RamParam));
//#else
//  FLASH_If_Write32(&FlashWriteAddress, (uint32_t*)&RamParam, (sizeof(RamParam)+3)/4);
//#endif // VDD_1_8
  FLASH_If_Lock();
}

unsigned short slow_crc16(unsigned short sum, unsigned char *p, unsigned int len)
{
  while (len--)
  {
    int i;
    unsigned char byte = *(p++);
    for (i = 0; i < 8; ++i)
    {
      unsigned long osum = sum;
      sum <<= 1;
      if (byte & 0x80)
        sum |= 1;
      if (osum & 0x8000)
        sum ^= 0x1021;
      byte <<= 1;
    }
  }
  return sum;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
