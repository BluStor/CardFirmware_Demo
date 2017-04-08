/**
  ******************************************************************************
  * @file    FatFs/FatFs_uSD_RTOS/Src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    14-August-2015
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
  printf("NMI_Handler()\r\n");
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  int x  =0;
  int cnt = 0;
  
  /* Go to infinite loop when Hard Fault exception occurs */
  printf("ERROR - HardFault_Handler()\r\n");
  
  BSP_LED_Off(LED_YELLOW);
  BSP_LED_Off(LED_RED);
  
  // Flash error code for about 15 seconds
  while (cnt++ < 80)
  {
    x=0;
    while(x++<2000000);
    BSP_LED_On(LED_RED);
    
    x=0;
    while(x++<2000000);
    BSP_LED_Off(LED_RED);
  }
  
  // Attempt a system reset
  HAL_NVIC_SystemReset();
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  int x=0;
  int cnt = 0;
  
  /* Go to infinite loop when Memory Manage exception occurs */
  printf("ERROR - MemManage_Handler()\r\n");
  BSP_LED_On(LED_YELLOW);
  BSP_LED_Off(LED_RED);
  
  // Flash error code for about 15 seconds
  while (cnt++ < 80)
  {
    x=0;
    while(x++<2000000);
    BSP_LED_On(LED_RED);
    
    x=0;
    while(x++<2000000);
    BSP_LED_Off(LED_RED);
  }
  
  // Attempt a system reset
  HAL_NVIC_SystemReset();
  
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  int x=0;
  int cnt = 0;
  
  /* Go to infinite loop when Bus Fault exception occurs */
  printf("ERROR - BusFault_Handler()\r\n");
  BSP_LED_Off(LED_YELLOW);
  BSP_LED_Off(LED_RED);
  
  // Flash error code for about 15 seconds
  while (cnt++ < 80)
  {
    x=0;
    while(x++<2000000);
    BSP_LED_On(LED_RED);
    BSP_LED_On(LED_YELLOW);
    
    x=0;
    while(x++<2000000);
    BSP_LED_Off(LED_RED);
    BSP_LED_Off(LED_YELLOW);
  }
  
  // Attempt a system reset
  HAL_NVIC_SystemReset();
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  int x=0;
  int cnt = 0;
  
  /* Go to infinite loop when Usage Fault exception occurs */
  printf("ERROR - UsageFault_Handler()\r\n");
  BSP_LED_Off(LED_YELLOW);
  BSP_LED_Off(LED_RED);
  
  // Flash error code for about 15 seconds
  while (cnt++ < 80)
  {
    x=0;
    while(x++<2000000);
    BSP_LED_On(LED_YELLOW);
    
    x=0;
    while(x++<2000000);
    BSP_LED_Off(LED_YELLOW);
  }
  
  // Attempt a system reset
  HAL_NVIC_SystemReset();
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
//void SVC_Handler(void)        // already defined by RTOS as vPortSVCHandler!!!
//{
//}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
  printf("DebugMon_Handler()\r\n");
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
//void PendSV_Handler(void)     // already defined by RTOS as xPortPendSVHandler!!!
//{
//}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
#ifdef FIRMWARE
  osSystickHandler();
#endif
}
/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles USB-On-The-Go FS global interrupt request.
  * @param  None
  * @retval None
  */
#ifdef FIRMWARE
#ifdef USE_USB_FS
void OTG_FS_IRQHandler(void)
#else
void OTG_HS_IRQHandler(void)
#endif
{
  HAL_PCD_IRQHandler(&hpcd);
}
#endif // FIRMWARE
/**
  * @brief  This function handles DMA2 Stream 3 interrupt request. // SDIO
  * @param  None
  * @retval None
  */
void DMA2_Stream3_IRQHandler(void)
{
  BSP_SD_DMA_Rx_IRQHandler();
}


/**
  * @brief  This function handles DMA2 Stream 6 interrupt request. // SDIO
  * @param  None
  * @retval None
  */
void DMA2_Stream6_IRQHandler(void)
{
  BSP_SD_DMA_Tx_IRQHandler(); 
}


/**
  * @brief  This function handles SDIO interrupt request.
  * @param  None
  * @retval None
  */
void SDIO_IRQHandler(void)
{
  BSP_SD_IRQHandler();
}

extern UART_HandleTypeDef UartHandle;
#ifdef FIRMWARE
extern UART_HandleTypeDef HCITRUartHandle;


/**
  * @brief  This function handles DMA2 Stream 1 interrupt request.
  * @param  None
  * @retval None
  */
#ifdef STM32F415xx
void DMA1_Stream1_IRQHandler(void)
#else
void DMA2_Stream1_IRQHandler(void)
#endif
{
  HAL_DMA_IRQHandler(HCITRUartHandle.hdmarx);
}

/**
  * @brief  This function handles DMA2 Stream 7 interrupt request.
  * @param  None
  * @retval None
  */
#ifdef STM32F415xx
void DMA1_Stream3_IRQHandler(void)
#else
void DMA2_Stream7_IRQHandler(void)
#endif
{
  HAL_DMA_IRQHandler(HCITRUartHandle.hdmatx);
}
#endif // FIRMWARE
/**
  * @brief  This function handles USART3 interrupt request.
  * @param  None
  * @retval None
  */
#ifdef FIRMWARE
#ifdef STM32F415xx
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartHandle);
}
#else
void USART3_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UartHandle);
}
#endif
#endif // FIRMWARE

/**
  * @brief  This function handles External lines 15 to 10 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI4_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_4);
}

/**
  * @brief  This function handles External lines 2 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI2_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_2);
}
/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
