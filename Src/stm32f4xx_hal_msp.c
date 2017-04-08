/**
  ******************************************************************************
  * @file    UART/UART_Printf/Src/stm32f4xx_hal_msp.c
  * @author  MCD Application Team
  * @version V1.2.2
  * @date    01-July-2015
  * @brief   HAL MSP module.    
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "hcitrcfg.h"    

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @defgroup HAL_MSP
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
  * @{
  */
    

/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;
#ifdef FIRMWARE  
  if (huart->Instance == EVAL_COM1) {
#endif // FIRMWARE
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    EVAL_COM1_TX_GPIO_CLK_ENABLE();
    EVAL_COM1_RX_GPIO_CLK_ENABLE();
    
    /* Enable USARTx clock */
    EVAL_COM1_CLK_ENABLE(); 
    
    /*##-2- Configure peripheral GPIO ##########################################*/  
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = EVAL_COM1_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = EVAL_COM1_TX_AF;
    
    HAL_GPIO_Init(EVAL_COM1_TX_GPIO_PORT, &GPIO_InitStruct);
      
    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = EVAL_COM1_RX_PIN;
    GPIO_InitStruct.Alternate = EVAL_COM1_RX_AF;
      
    HAL_GPIO_Init(EVAL_COM1_RX_GPIO_PORT, &GPIO_InitStruct);
#ifdef FIRMWARE    
    HAL_NVIC_SetPriority(EVAL_COM1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EVAL_COM1_IRQn);

  } else if (huart->Instance == HCITR_UART_BASE) {
    static DMA_HandleTypeDef *hdma_tx;
    static DMA_HandleTypeDef *hdma_rx;
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO clock */
    HCITR_UART_TX_GPIO_CLK_ENABLE();
    HCITR_UART_RX_GPIO_CLK_ENABLE();
    /* Enable USARTx clock */
    HCITR_UART_CLK_ENABLE(); 
    /* Enable DMA2 clock */
    HCITR_DMA_CLK_ENABLE();   

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = HCITR_TXD_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = HCITR_UART_TX_GPIO_AF;

    HAL_GPIO_Init(HCITR_TXD_GPIO_PORT, &GPIO_InitStruct);
      
    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = HCITR_RXD_PIN;
    GPIO_InitStruct.Alternate = HCITR_UART_RX_GPIO_AF;
      
    HAL_GPIO_Init(HCITR_RXD_GPIO_PORT, &GPIO_InitStruct);
      
    /* UART RTS GPIO pin configuration  */
    GPIO_InitStruct.Pin = HCITR_RTS_PIN;
    GPIO_InitStruct.Alternate = HCITR_UART_RTS_GPIO_AF;
      
    HAL_GPIO_Init(HCITR_RTS_GPIO_PORT, &GPIO_InitStruct);
      
    /* UART RX GPIO pin configuration  */
    GPIO_InitStruct.Pin = HCITR_CTS_PIN;
    GPIO_InitStruct.Alternate = HCITR_UART_CTS_GPIO_AF;
      
    HAL_GPIO_Init(HCITR_CTS_GPIO_PORT, &GPIO_InitStruct);
 

    /*##-3- Configure the DMA streams ##########################################*/
    /* Configure the DMA handler for Transmission process */
    hdma_tx = huart->hdmatx;
    hdma_tx->Instance                 = HCITR_TXD_DMA_STREAM;
    hdma_tx->Parent                   = huart;

    hdma_tx->Init.Channel             = HCITR_TXD_DMA_CHANNEL;
    hdma_tx->Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx->Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx->Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx->Init.Mode                = DMA_NORMAL;
    hdma_tx->Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    hdma_tx->Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_tx->Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_tx->Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_tx->Init.PeriphBurst         = DMA_PBURST_SINGLE;

    HAL_DMA_Init(hdma_tx);   

      
    /* Configure the DMA handler for reception process */
    hdma_rx = huart->hdmarx;
    hdma_rx->Instance                 = HCITR_RXD_DMA_STREAM;
    hdma_rx->Parent                   = huart;

    hdma_rx->Init.Channel             = HCITR_RXD_DMA_CHANNEL;
    hdma_rx->Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx->Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx->Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx->Init.Mode                = DMA_NORMAL;
    hdma_rx->Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    hdma_rx->Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_rx->Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_1QUARTERFULL;
    hdma_rx->Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_rx->Init.PeriphBurst         = DMA_PBURST_SINGLE; 

    HAL_DMA_Init(hdma_rx);
      
      
    /*##-4- Configure the NVIC for DMA #########################################*/
    /* NVIC configuration for DMA transfer complete interrupt (HCITR_TXD_IRQ) */
    HAL_NVIC_SetPriority(HCITR_TXD_IRQ, HCITR_IRQ_INTERRUPT_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(HCITR_TXD_IRQ);
      
    /* NVIC configuration for DMA transfer complete interrupt (HCITR_RXD_IRQ) */
    HAL_NVIC_SetPriority(HCITR_RXD_IRQ, HCITR_IRQ_INTERRUPT_PRIORITY, 0);   
    HAL_NVIC_EnableIRQ(HCITR_RXD_IRQ);

  }
#endif // FIRMWARE
}

/**
  * @brief UART MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  if (huart->Instance == EVAL_COM1) {
    /*##-1- Reset peripherals ##################################################*/
    EVAL_COM1_FORCE_RESET();
    EVAL_COM1_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks #################################*/
    /* Configure UART Tx as alternate function  */
    HAL_GPIO_DeInit(EVAL_COM1_TX_GPIO_PORT, EVAL_COM1_TX_PIN);
    /* Configure UART Rx as alternate function  */
    HAL_GPIO_DeInit(EVAL_COM1_RX_GPIO_PORT, EVAL_COM1_RX_PIN);
  
  } else if (huart->Instance == HCITR_UART_BASE) {

    static DMA_HandleTypeDef hdma_tx;
    static DMA_HandleTypeDef hdma_rx;
    /*##-2- Disable peripherals and GPIO Clocks #################################*/
    /* Configure UART Tx as alternate function  */
    HAL_GPIO_DeInit(HCITR_TXD_GPIO_PORT, HCITR_TXD_PIN);
    /* Configure UART Rx as alternate function  */
    HAL_GPIO_DeInit(HCITR_RXD_GPIO_PORT, HCITR_RXD_PIN);
    /* Configure UART RTS as alternate function  */
    HAL_GPIO_DeInit(HCITR_RTS_GPIO_PORT, HCITR_RTS_PIN);
    /* Configure UART CTS as alternate function  */
    HAL_GPIO_DeInit(HCITR_CTS_GPIO_PORT, HCITR_CTS_PIN);

    /*##-3- Disable the DMA Streams ############################################*/
    /* De-Initialize the DMA Stream associate to transmission process */
    hdma_tx.Instance                 = HCITR_TXD_DMA_STREAM;
    hdma_rx.Instance                 = HCITR_RXD_DMA_STREAM;
    
    HAL_DMA_DeInit(&hdma_tx); 
    /* De-Initialize the DMA Stream associate to reception process */
    HAL_DMA_DeInit(&hdma_rx);
    
    /*##-4- Disable the NVIC for DMA ###########################################*/
    HAL_NVIC_DisableIRQ(HCITR_TXD_IRQ);
    HAL_NVIC_DisableIRQ(HCITR_RXD_IRQ);
    
  }  
}

extern void HCITR_UART_TxCpltCallback(UART_HandleTypeDef *huart);
extern void HCITR_UART_RxCpltCallback(UART_HandleTypeDef *huart);
extern void HCITR_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);
#ifdef CONSOLE_SUPPORT
extern void HAL_Console_RxCpltCallback(UART_HandleTypeDef *huart);
#endif // CONSOLE_SUPPORT

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == EVAL_COM1) {
  } else if (huart->Instance == HCITR_UART_BASE) {
    HCITR_UART_TxCpltCallback(huart);
  }
  
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == EVAL_COM1) {
#ifdef CONSOLE_SUPPORT
    HAL_Console_RxCpltCallback(huart);
#endif // CONSOLE_SUPPORT
  } else if (huart->Instance == HCITR_UART_BASE) {
    HCITR_UART_RxCpltCallback(huart);
  }
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == EVAL_COM1) {
  } else if (huart->Instance == HCITR_UART_BASE) {
    HCITR_UART_RxHalfCpltCallback(huart);
  }
}

/**
  * @brief I2C MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           //- DMA configuration for transmission request by peripheral
  *           //- NVIC configuration for DMA interrupt request enable
  *           - nb. I2C is only used in polling mode, no DMA, no interruption!
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  static DMA_HandleTypeDef hdma_tx;
  static DMA_HandleTypeDef hdma_rx;

  GPIO_InitTypeDef  GPIO_InitStruct;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  EVAL_I2Cx_SCL_GPIO_CLK_ENABLE();
  EVAL_I2Cx_SDA_GPIO_CLK_ENABLE();
  /* Enable I2Cx clock */
  EVAL_I2Cx_CLK_ENABLE();
  /* Enable DMAx clock */
  //EVAL_DMAx_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* I2C SCL GPIO pin configuration  */
  GPIO_InitStruct.Pin       = EVAL_I2Cx_SCL_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = EVAL_I2Cx_SCL_AF;

  HAL_GPIO_Init(EVAL_I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

  /* I2C SDA GPIO pin configuration  */
  GPIO_InitStruct.Pin = EVAL_I2Cx_SDA_PIN;
  GPIO_InitStruct.Alternate = EVAL_I2Cx_SDA_AF;

  HAL_GPIO_Init(EVAL_I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

  /*##-3- Configure the DMA ##################################################*/
  /* Configure the DMA handler for Transmission process */
  //hdma_tx.Instance                 = I2Cx_DMA_INSTANCE_TX;

  //hdma_tx.Init.Channel             = I2Cx_TX_DMA_CHANNEL;
  //hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  //hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  //hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
  //hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  //hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  //hdma_tx.Init.Mode                = DMA_NORMAL;
  //hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
  //hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  //hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  //hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
  //hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;

  //HAL_DMA_Init(&hdma_tx);

  /* Associate the initialized DMA handle to the the I2C handle */
  //__HAL_LINKDMA(hi2c, hdmatx, hdma_tx);

  /* Configure the DMA handler for Transmission process */
  //hdma_rx.Instance                 = I2Cx_DMA_INSTANCE_RX;

  //hdma_rx.Init.Channel             = I2Cx_RX_DMA_CHANNEL;
  //hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  //hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
  //hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
  //hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  //hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  //hdma_rx.Init.Mode                = DMA_NORMAL;
  //hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
  //hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  //hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  //hdma_rx.Init.MemBurst            = DMA_MBURST_INC4;
  //hdma_rx.Init.PeriphBurst         = DMA_PBURST_INC4;

  //HAL_DMA_Init(&hdma_rx);

  /* Associate the initialized DMA handle to the the I2C handle */
  //__HAL_LINKDMA(hi2c, hdmarx, hdma_rx);

  /*##-4- Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt (I2C1_TX) */
  //HAL_NVIC_SetPriority(I2Cx_DMA_TX_IRQn, 0, 0);
  //HAL_NVIC_EnableIRQ(I2Cx_DMA_TX_IRQn);

  /* NVIC configuration for DMA transfer complete interrupt (I2C1_RX) */
  //HAL_NVIC_SetPriority(I2Cx_DMA_RX_IRQn, 0, 0);
  //HAL_NVIC_EnableIRQ(I2Cx_DMA_RX_IRQn);
  
  /*##-5- Configure I2C interruption */
  /* Set priority and enable I2Cx event Interrupt */
  HAL_NVIC_SetPriority(EVAL_I2Cx_EV_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EVAL_I2Cx_EV_IRQn);
  
  /* Set priority and enable I2Cx error Interrupt */
  HAL_NVIC_SetPriority(EVAL_I2Cx_ER_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EVAL_I2Cx_ER_IRQn);
}

/**
  * @brief I2C MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{

  static DMA_HandleTypeDef hdma_tx;
  static DMA_HandleTypeDef hdma_rx;

  /*##-1- Reset peripherals ##################################################*/
  EVAL_I2Cx_FORCE_RESET();
  EVAL_I2Cx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks ################################*/
  /* Configure I2C Tx as alternate function  */
  HAL_GPIO_DeInit(EVAL_I2Cx_SCL_GPIO_PORT, EVAL_I2Cx_SCL_PIN);
  /* Configure I2C Rx as alternate function  */
  HAL_GPIO_DeInit(EVAL_I2Cx_SDA_GPIO_PORT, EVAL_I2Cx_SDA_PIN);

  /*##-3- Disable the DMA handles ############################################*/
  /* De-Initialize the DMA handle associated to transmission process */
  //HAL_DMA_DeInit(&hdma_tx);
  /* De-Initialize the DMA handle associated to reception process */
  //HAL_DMA_DeInit(&hdma_rx);

  /*##-4- Disable the NVIC for DMA ###########################################*/
  //HAL_NVIC_DisableIRQ(I2Cx_DMA_TX_IRQn);
  //HAL_NVIC_DisableIRQ(I2Cx_DMA_RX_IRQn);
}
/**
  * @}
  */

/**
  * @brief RTC MSP Initialization 
  *        This function configures the hardware resources used in this example
  * @param hrtc: RTC handle pointer
  * 
  * @note  Care must be taken when HAL_RCCEx_PeriphCLKConfig() is used to select 
  *        the RTC clock source; in this case the Backup domain will be reset in  
  *        order to modify the RTC Clock source, as consequence RTC registers (including 
  *        the backup registers) and RCC_BDCR register are set to their reset values.
  *             
  * @retval None
  */
void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
  RCC_OscInitTypeDef        RCC_OscInitStruct;
  RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
  
  /*##-1- Configue LSE as RTC clock soucre ###################################*/ 
#ifdef RTC_CLOCK_SOURCE_LSE
  
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  { 
    Error_Handler();
  }
  
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  { 
    Error_Handler();
  }
#elif defined (RTC_CLOCK_SOURCE_LSI)  
  RCC_OscInitStruct.OscillatorType =  RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  { 
    Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if(HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  { 
    Error_Handler();
  }
#else
#error Please select the RTC Clock source inside the main.h file
#endif /*RTC_CLOCK_SOURCE_LSE*/
  
  /*##-2- Enable RTC peripheral Clocks #######################################*/ 
  /* Enable RTC Clock */ 
  __HAL_RCC_RTC_ENABLE(); 

}

/**
  * @brief RTC MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hrtc: RTC handle pointer
  * @retval None
  */
void HAL_RTC_MspDeInit(RTC_HandleTypeDef *hrtc)
{
  /*##-1- Reset peripherals ##################################################*/
   __HAL_RCC_RTC_DISABLE();
   
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
