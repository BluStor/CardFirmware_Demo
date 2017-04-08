/**
  ******************************************************************************
  * @file    LwIP/LwIP_IAP/Inc/flash_if.h 
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    14-August-2015
  * @brief   Header for flash_if.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H
#define __FLASH_IF_H

/* Includes ------------------------------------------------------------------*/


/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported variable----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
//uint32_t FLASH_If_Write32(__IO uint32_t* Address, uint32_t* Data, uint16_t DataLength);
uint32_t FLASH_If_Write8(__IO uint32_t* Address, uint8_t* Data, uint32_t DataLength);
int8_t FLASH_If_Erase(uint32_t StartSector);
void FLASH_If_Unlock(void);
void FLASH_If_Lock(void);
void FLASH_If_ReadParam(void);
void FLASH_If_SaveParam(void);
unsigned short slow_crc16(unsigned short sum, unsigned char *p, unsigned int len);
#endif /* __FLASH_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
