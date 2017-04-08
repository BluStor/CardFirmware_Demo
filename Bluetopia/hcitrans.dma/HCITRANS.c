/*****< hcitrans.c >***********************************************************/
/*      Copyright 2012 - 2013 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HCITRANS - HCI Transport Layer for use with Bluetopia.                    */
/*                                                                            */
/*  Author:  Marcus Funk                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/08/12  M. Funk        Initial creation.                               */
/******************************************************************************/

#include "BTPSKRNL.h"       /* Bluetooth Kernel Prototypes/Constants.         */
#include "HCITRANS.h"       /* HCI Transport Prototypes/Constants.            */
#include "HCITRCFG.h"       /* HCI Transport configuration.                   */

#include "FreeRTOS.h"       /* FreeRTOS Kernal Prototypes/Constants.          */
#include "task.h"           /* FreeRTOS Task Prototypes/Constants.            */
#include "semphr.h"         /* FreeRTOS Semaphore Prototypes/Constants.       */
#include "portmacro.h"


   /* The following define the size of the buffers used by HCITRANS.    */
#define INPUT_BUFFER_SIZE        1536
#define OUTPUT_BUFFER_SIZE       1056

   /* The following defines the DMA length used for reading data from   */
   /* the UART. Larger values will require less processor intervention  */
   /* but will also result in more wasted space in the buffer (as the   */
   /* buffer will wrap when the space at the end is less than           */
   /* INPUT_DMA_SIZE) and possibly slower reaction times (as the        */
   /* interrupt will not occur until half of INPUT_DMA_SIZE bytes have  */
   /* been received).                                                   */
#define INPUT_DMA_SIZE           128

   /* The following defines the maximum DMA length used for writing data*/
   /* to the UART. Larger values will require less processor            */
   /* intervention but may result in longer blocking times in           */
   /* HCITR_COMWrite as the data being written by a DMA transfer will   */
   /* not be credited back to the buffer until the DMA transaction      */
   /* is completed.                                                     */
#define OUTPUT_MAXIMUM_DMA_SIZE  32

#define ClearReset()             HAL_GPIO_WritePin(HCITR_RESET_GPIO_PORT, HCITR_RESET_PIN, GPIO_PIN_SET)
#define SetReset()               HAL_GPIO_WritePin(HCITR_RESET_GPIO_PORT, HCITR_RESET_PIN, GPIO_PIN_RESET)

#ifdef STM32F415xx
#define BT_HCI_OE_Enable()       HAL_GPIO_WritePin(HCITR_BT_HCI_OE_PORT, BT_HCI_OE_PIN, GPIO_PIN_SET)
#define BT_HCI_OE_Disable()      HAL_GPIO_WritePin(HCITR_BT_HCI_OE_PORT, BT_HCI_OE_PIN, GPIO_PIN_RESET)

//#define VSUP_BT_On()           HAL_GPIO_WritePin(HCITR_VSUP_BT_EN_PORT, VSUP_BT_EN_PIN, GPIO_PIN_SET)    already done in pmic.c
//#define VSUP_BT_Off()          HAL_GPIO_WritePin(HCITR_VSUP_BT_EN_PORT, VSUP_BT_EN_PIN, GPIO_PIN_RESET)
#endif // STM32F415xx

extern int BTActivity;

#define DisableInterrupts()      portENTER_CRITICAL()
#define EnableInterrupts()       portEXIT_CRITICAL()

#define INTERRUPT_PRIORITY       configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY

#define TRANSPORT_ID             1

#define DEBUG_PRINT              BTPS_OutputMessage

typedef enum
{
   hssNormal,
   hssSuspendWait,
   hssSuspendWaitInterrupted,
   hssSuspended
} SuspendState_t;

typedef struct _tagUartContext_t
{
#ifdef HCITR_ENABLE_DEBUG_LOGGING

   Mutex_t                  DebugMutex;
   Boolean_t                DebugEnabled;

#endif

   volatile ThreadHandle_t  ReceiveThreadHandle;
   xSemaphoreHandle         DataReceivedEvent;

   SuspendState_t           SuspendState;

   HCITR_COMDataCallback_t  COMDataCallbackFunction;
   unsigned long            COMDataCallbackParameter;

   unsigned short           RxInIndex;
   unsigned short           RxOutIndex;
   volatile unsigned short  RxBufferSize;
   volatile unsigned short  RxBytesUsed;
   volatile Boolean_t       RxDMAInProgress;
   unsigned char            RxBuffer[INPUT_BUFFER_SIZE];

   unsigned short           TxInIndex;
   unsigned short           TxOutIndex;
   volatile unsigned short  TxBytesFree;
   volatile unsigned short  TxPreviousDMALength;
   unsigned char            TxBuffer[OUTPUT_BUFFER_SIZE];
} UartContext_t;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

UART_HandleTypeDef HCITRUartHandle;
DMA_HandleTypeDef HCITRDmaTxHandle;
DMA_HandleTypeDef HCITRDmaRxHandle;

#ifdef SUPPORT_TRANSPORT_SUSPEND

   /* CTS external interrupt handler.                                   */
//static BTPSCONST EXTI_InitTypeDef CTS_ExtIntConfiguration = {HCITR_CTS_IRQ_LINE, EXTI_Mode_Interrupt, EXTI_Trigger_Falling, ENABLE};

#endif

static UartContext_t              UartContext;
static int                        HCITransportOpen        = 0;


   /* Local Function Prototypes.                                        */
static void SetBaudRate(USART_TypeDef *UartBase, unsigned int BaudRate);
static void SetSuspendGPIO(Boolean_t Suspend);
static void StartTxDMATransfer(void);
static void StartRxDMATransfer(void);
static Boolean_t ProcessRxDMA(void);

   /* The following function will reconfigure the BAUD rate without     */
   /* reconfiguring the entire port.  This function is also potentially */
   /* more accurate than the method used in the ST standard peripheral  */
   /* libraries.                                                        */
static void SetBaudRate(USART_TypeDef *UartBase, unsigned int BaudRate)
{
   unsigned int      SourceFrequency;
   unsigned int      Divider;
   unsigned int      TempDiv;

#if((HCITR_UART == 1) || (HCITR_UART == 6))

   SourceFrequency = HAL_RCC_GetPCLK2Freq();

#else

   SourceFrequency = HAL_RCC_GetPCLK1Freq();

#endif

   /* The following calculation will yield the integer divider          */
   /* concatenated with the fractional divider.  If 16-bit oversampling */
   /* is used, the least significant 4 bits will be the fraction divider*/
   /* and if 8-bit oversampling is used, the least significant 3 bits   */
   /* will be the fraction divider.                                     */
   Divider = (((SourceFrequency * 2) / BaudRate) + 1) / 2;

   /* The integer divider must be between 1 and 4095 so if the divider  */
   /* value is less than 16, 8-bit oversampling must be used.           */
   if(Divider < 16)
   {
      /* When 8-bit oversampling is used, bits [2..0] of the BRR        */
      /* register represent the fractional divider and bits [15..4]     */
      /* represent the integer divider.  Because of this, the integer   */
      /* divider needs to be shifted left one bit before the baud rate  */
      /* control register is updated.                                   */
      TempDiv = Divider & 0x7;
      Divider = (Divider & ~0x07) << 1;
      Divider |= TempDiv;

      /* Disable the UART while updating the baud rate.                 */
      UartBase->CR1 &= ~USART_CR1_UE;
      UartBase->CR1 |= USART_CR1_OVER8;
   }
   else
   {
      /* For lower frequencies, 16bit oversampling may be used.         */

      /* Disable the UART while updating the baud rate.                 */
      UartBase->CR1 &= ~USART_CR1_UE;
      UartBase->CR1 &= ~USART_CR1_OVER8;
   }

   UartBase->BRR = Divider;
   UartBase->CR1 |= USART_CR1_UE;
}

   /* The following function is responsible for setting the CTS and RTS */
   /* GPIO to the correct state when entering and leaving suspend mode. */
   /* The first parameter specifies if the GPIO should be set for the   */
   /* suspend mode (TRUE) or not-suspended mode (FALSE).                */
static void SetSuspendGPIO(Boolean_t Suspend)
{
#ifdef SUPPORT_TRANSPORT_SUSPEND
   
   //EXTI_InitTypeDef ExtIntConfiguration;
   GPIO_InitTypeDef GPIO_InitStruct;
   __HAL_RCC_GPIOD_CLK_ENABLE();
   if(Suspend)
   {
      /* Set CTS as a GPIO input.                                       */
      //ConfigureGPIO(HCITR_CTS_GPIO_PORT, HCITR_CTS_PIN, GPIO_Mode_IN);
      GPIO_InitStruct.Pin       = HCITR_CTS_PIN;
      GPIO_InitStruct.Mode      = GPIO_MODE_IT_FALLING;
      GPIO_InitStruct.Pull      = GPIO_NOPULL;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
      HAL_GPIO_Init(HCITR_CTS_GPIO_PORT, &GPIO_InitStruct);

      /* Enable the external interrupt on the CTS pin.                  */
      //EXTI_Init((EXTI_InitTypeDef *)&CTS_ExtIntConfiguration);
      NVIC_SetPriority(HCITR_CTS_IRQ, INTERRUPT_PRIORITY);
      NVIC_EnableIRQ(HCITR_CTS_IRQ);
      


      /* Set RTS as an output and pull high.                            */
//      ConfigureGPIO(HCITR_RTS_GPIO_PORT, HCITR_RTS_PIN, GPIO_Mode_OUT);
      GPIO_InitStruct.Pin       = HCITR_RTS_PIN;
      GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull      = GPIO_NOPULL;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(HCITR_RTS_GPIO_PORT, &GPIO_InitStruct);
      HAL_GPIO_WritePin(HCITR_RTS_GPIO_PORT, HCITR_RTS_PIN, GPIO_PIN_SET);
   }
   else
   {
      /* Disable the CTS interrupt.                                     */
      //BTPS_MemCopy(&ExtIntConfiguration, &CTS_ExtIntConfiguration, sizeof(EXTI_InitTypeDef));
      //ExtIntConfiguration.EXTI_LineCmd = DISABLE;
      //EXTI_Init(&ExtIntConfiguration);
      NVIC_DisableIRQ(HCITR_CTS_IRQ);

      /* Set the CTS and RTS pins back to the default functionality.    */
      //ConfigureGPIO(HCITR_CTS_GPIO_PORT, HCITR_CTS_PIN, GPIO_Mode_AF);
      GPIO_InitStruct.Pin       = HCITR_RTS_PIN;
      GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull      = GPIO_PULLUP;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
      GPIO_InitStruct.Alternate = HCITR_UART_RTS_GPIO_AF;
      HAL_GPIO_Init(HCITR_RTS_GPIO_PORT, &GPIO_InitStruct);
      //ConfigureGPIO(HCITR_RTS_GPIO_PORT, HCITR_RTS_PIN, GPIO_Mode_AF);
      GPIO_InitStruct.Pin = HCITR_CTS_PIN;
      GPIO_InitStruct.Alternate = HCITR_UART_CTS_GPIO_AF;
      HAL_GPIO_Init(HCITR_CTS_GPIO_PORT, &GPIO_InitStruct);
   }

#endif
}

void BT_EnableCTSInt(Boolean_t Suspend)
{
     //EXTI_InitTypeDef ExtIntConfiguration;
   GPIO_InitTypeDef GPIO_InitStruct;
   __HAL_RCC_GPIOD_CLK_ENABLE();
   if(Suspend)
   {
      /* Set CTS as a GPIO input.                                       */
      //ConfigureGPIO(HCITR_CTS_GPIO_PORT, HCITR_CTS_PIN, GPIO_Mode_IN);
      GPIO_InitStruct.Pin       = HCITR_CTS_PIN;
      GPIO_InitStruct.Mode      = GPIO_MODE_IT_FALLING;
      GPIO_InitStruct.Pull      = GPIO_NOPULL;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
      HAL_GPIO_Init(HCITR_CTS_GPIO_PORT, &GPIO_InitStruct);

      /* Enable the external interrupt on the CTS pin.                  */
      //EXTI_Init((EXTI_InitTypeDef *)&CTS_ExtIntConfiguration);
      NVIC_SetPriority(HCITR_CTS_IRQ, INTERRUPT_PRIORITY);
      NVIC_EnableIRQ(HCITR_CTS_IRQ);
      


      /* Set RTS as an output and pull high.                            */
//      ConfigureGPIO(HCITR_RTS_GPIO_PORT, HCITR_RTS_PIN, GPIO_Mode_OUT);
      GPIO_InitStruct.Pin       = HCITR_RTS_PIN;
      GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull      = GPIO_NOPULL;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(HCITR_RTS_GPIO_PORT, &GPIO_InitStruct);
      HAL_GPIO_WritePin(HCITR_RTS_GPIO_PORT, HCITR_RTS_PIN, GPIO_PIN_SET);
   }
   else
   {
      /* Disable the CTS interrupt.                                     */
      //BTPS_MemCopy(&ExtIntConfiguration, &CTS_ExtIntConfiguration, sizeof(EXTI_InitTypeDef));
      //ExtIntConfiguration.EXTI_LineCmd = DISABLE;
      //EXTI_Init(&ExtIntConfiguration);
      NVIC_DisableIRQ(HCITR_CTS_IRQ);

      /* Set the CTS and RTS pins back to the default functionality.    */
      //ConfigureGPIO(HCITR_CTS_GPIO_PORT, HCITR_CTS_PIN, GPIO_Mode_AF);
      GPIO_InitStruct.Pin       = HCITR_RTS_PIN;
      GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull      = GPIO_PULLUP;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
      GPIO_InitStruct.Alternate = HCITR_UART_RTS_GPIO_AF;
      HAL_GPIO_Init(HCITR_RTS_GPIO_PORT, &GPIO_InitStruct);
      //ConfigureGPIO(HCITR_RTS_GPIO_PORT, HCITR_RTS_PIN, GPIO_Mode_AF);
      GPIO_InitStruct.Pin = HCITR_CTS_PIN;
      GPIO_InitStruct.Alternate = HCITR_UART_CTS_GPIO_AF;
      HAL_GPIO_Init(HCITR_CTS_GPIO_PORT, &GPIO_InitStruct);
   }

}



   /* The following function starts a TX DMA transfer if one is not     */
   /* already in progress.                                              */
   /* * NOTE * Interrupts Must be disabled when calling this function as*/
   /*          it is not re-entrant.                                    */

static void StartTxDMATransfer(void)
{
   /* Confirm that a DMA is not currently in progress (true if the      */
   /* previous DMA length is 0) and that there is data waiting to be    */
   /* sent.                                                             */
   if((!(UartContext.TxPreviousDMALength)) && ((UartContext.TxPreviousDMALength = (OUTPUT_BUFFER_SIZE - UartContext.TxBytesFree)) != 0))
   {
      /* Determine the size of the transfer as the minimum of the data  */
      /* to be sent, the maximum DMA length and the amount at the end of*/
      /* the buffer.                                                    */
      if(UartContext.TxPreviousDMALength > OUTPUT_MAXIMUM_DMA_SIZE)
         UartContext.TxPreviousDMALength = OUTPUT_MAXIMUM_DMA_SIZE;

      if(UartContext.TxPreviousDMALength > (OUTPUT_BUFFER_SIZE - UartContext.TxOutIndex))
         UartContext.TxPreviousDMALength = OUTPUT_BUFFER_SIZE - UartContext.TxOutIndex;

      /* Start the DMA.                                                 */
      HAL_UART_Transmit_DMA(&HCITRUartHandle, 
                        (uint8_t *) &(UartContext.TxBuffer[UartContext.TxOutIndex]),
                        UartContext.TxPreviousDMALength);

      /* Adjust the indexes.                                            */
      UartContext.TxOutIndex  += UartContext.TxPreviousDMALength;
      if(UartContext.TxOutIndex == OUTPUT_BUFFER_SIZE)
         UartContext.TxOutIndex = 0;
   }
}

   /* The following function starts a RX DMA transfer if one is not     */
   /* already in progress.                                              */
   /* * NOTE * Interrupts Must be disabled when calling this function as*/
   /*          it is not re-entrant.                                    */

static void StartRxDMATransfer(void)
{
   unsigned BufferEnd;

   /* Determine the end of the availabe region to use for DMA as either */
   /* the output index (if the current data in the buffer wraps the end)*/
   /* or the size of the buffer.                                        */
   if(UartContext.RxOutIndex > UartContext.RxInIndex)
      BufferEnd = UartContext.RxOutIndex;
   else
      BufferEnd = INPUT_BUFFER_SIZE;

   /* Confirm that a Rx DMA is not curently in progress and there is    */
   /* space in the buffer for another DMA transfer.                     */
   if((!(UartContext.RxDMAInProgress)) && (UartContext.RxBytesUsed != UartContext.RxBufferSize) && ((BufferEnd - UartContext.RxInIndex) >= INPUT_DMA_SIZE))
   {
      HAL_UART_Receive_DMA(&HCITRUartHandle,
                           (uint8_t *) (&(UartContext.RxBuffer[UartContext.RxInIndex])),
                           INPUT_DMA_SIZE);

      UartContext.RxDMAInProgress = TRUE;
   }
}

typedef struct
{
  __IO uint32_t ISR;   /*!< DMA interrupt status register */
  __IO uint32_t Reserved0;
  __IO uint32_t IFCR;  /*!< DMA interrupt flag clear register */
} DMA_Base_Registers;

static void DMA_ClearFlag(DMA_HandleTypeDef * hdma, uint32_t flag)
{
  DMA_Base_Registers *regs;
  regs = (DMA_Base_Registers *)hdma->StreamBaseAddress;
  regs->IFCR = flag;
  
}
   /* The following function processes data on the Rx DMA stream. Before*/
   /* processing starts, the DMA will be disabled and re-enabled when   */
   /* processing is complete. This function returns TRUE if data was    */
   /* read from the DMA or FALSE otherwise.                             */
   /* * NOTE * Interrupts MUST be disabled when this function is called */
   /*          to prevent clashes with the ISR.                         */

static Boolean_t ProcessRxDMA(void)
{
   Boolean_t    ret_val;
   unsigned int Length;

   /* Check if any data is available in the DMA buffer.                 */
   if(HCITR_RXD_DMA_STREAM->NDTR != INPUT_DMA_SIZE)
   {
      /* Make sure the DMA is disabled and clear the interrupt flags.   */
      HCITRUartHandle.Instance->CR3 &= (uint32_t)(~USART_CR3_DMAR);
      //__HAL_DMA_DISABLE(&HCITRDmaRxHandle);
      //while((HCITRDmaRxHandle.Instance->CR  & DMA_SxCR_EN) != 0);
      HAL_DMA_Abort(&HCITRDmaRxHandle);
      DMA_ClearFlag(&HCITRDmaRxHandle, HCITR_RXD_DMA_FLAG_TCIF | HCITR_RXD_DMA_FLAG_HTIF);
      
      
      UartContext.RxDMAInProgress = FALSE;

      /* Note how much data is in the buffer and adjust the indexes.    */
      Length = INPUT_DMA_SIZE - (unsigned short)(HCITR_RXD_DMA_STREAM->NDTR);
      UartContext.RxBytesUsed += Length;
      UartContext.RxInIndex   += Length;

      /* Determine if the next DMA transfer will wrap the end of the    */
      /* buffer. If this is the case then set the size of the buffer to */
      /* the current index.                                             */
      if(UartContext.RxInIndex >= (INPUT_BUFFER_SIZE - INPUT_DMA_SIZE))
      {
         UartContext.RxBufferSize = UartContext.RxInIndex;
         UartContext.RxInIndex    = 0;
      }


      if (HCITRUartHandle.State == HAL_UART_STATE_BUSY_TX_RX) {
        HCITRUartHandle.State = HAL_UART_STATE_BUSY_TX;
      } else {
        HCITRUartHandle.State = HAL_UART_STATE_READY;
      }
      
      /* Re-start the DMA transfer.                                     */
      StartRxDMATransfer();
      ret_val = TRUE;
  }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* The following thread is used to process the data that has been    */
   /* received from the UART and placed in the receive buffer.          */
static void *RxThread(void *Param)
{
   unsigned int MaxLength;
   unsigned int TotalLength;

#ifdef HCITR_ENABLE_DEBUG_LOGGING

   unsigned int Index;

#endif

   /* Loop the thread until the transport has been closed.              */
   while(HCITransportOpen)
   {
      /* Wait until there is data available in the receive buffer.      */
      while(((TotalLength = UartContext.RxBytesUsed) == 0) && (HCITransportOpen))
      {
         if(xSemaphoreTake(UartContext.DataReceivedEvent, 10 / portTICK_RATE_MS) != pdPASS)
         {
            /* Failed to take the semaphore so check if the DMA is      */
            /* partially full.                                          */
            DisableInterrupts();
            ProcessRxDMA();
            EnableInterrupts();
         }
      }

      if(TotalLength)
      {
         /* Determine the maximum number of characters that we can send */
         /* before we reach the end of the buffer.  We need to process  */
         /* the smaller of the max characters or the number of          */
         /* characters that are in the buffer.                          */
         MaxLength = (UartContext.RxBufferSize - UartContext.RxOutIndex);
         if(TotalLength > MaxLength)
            TotalLength = MaxLength;

#ifdef HCITR_ENABLE_DEBUG_LOGGING

         if(UartContext.DebugEnabled)
         {
            DEBUG_PRINT(">");

            for(Index = 0; Index < Count; Index ++)
               DEBUG_PRINT(" %02X", UartContext.RxBuffer[UartContext.RxOutIndex + Index]);

            DEBUG_PRINT("\r\n");
         }

#endif

         /* Call the upper layer back with the data.                    */
         if(UartContext.COMDataCallbackFunction)
            (*UartContext.COMDataCallbackFunction)(TRANSPORT_ID, TotalLength, &(UartContext.RxBuffer[UartContext.RxOutIndex]), UartContext.COMDataCallbackParameter);

         DisableInterrupts();

         /* Adjust the Out Index and handle any looping.                */
         UartContext.RxBytesUsed -= TotalLength;
         UartContext.RxOutIndex += TotalLength;
         if(UartContext.RxOutIndex == UartContext.RxBufferSize)
         {
            UartContext.RxOutIndex = 0;
            UartContext.RxBufferSize = INPUT_BUFFER_SIZE;
         }

         /* Make sure the receive DMA is running.                          */
         StartRxDMATransfer();
         EnableInterrupts();
      }
   }

   /* Set the thread handle to NULL to indicate the thread is ready to  */
   /* terminate.                                                        */
   UartContext.ReceiveThreadHandle = NULL;

   return(NULL);
}

   /* The following function is the Interrupt Service Routine for the   */
   /* UART TXD DMA.                                                     */
   /* * NOTE * This function is mapped to the appropriate DMA in        */
   /*          HCITRCFG.h.                                              */

void HCITR_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
   UartContext.TxBytesFree         += UartContext.TxPreviousDMALength;
   UartContext.TxPreviousDMALength  = 0;
   
   if (HCITRUartHandle.State == HAL_UART_STATE_BUSY_TX_RX) {
      HCITRUartHandle.State = HAL_UART_STATE_BUSY_RX;
    } else {
      HCITRUartHandle.State = HAL_UART_STATE_READY;
    }

   /* Start the next DMA transfer.                                      */
   StartTxDMATransfer();
   BTActivity++;


}

   /* The following function is the Interrupt Service Routine for the   */
   /* UART RXD DMA.                                                     */
   /* * NOTE * This function is mapped to the appropriate DMA in        */
   /*          HCITRCFG.h.                                              */

void HCITR_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
   signed portBASE_TYPE xHigherPriorityTaskWoken;
   Boolean_t bRet;

   bRet = ProcessRxDMA();
   if(bRet)
   {
      /* Signal the reception of some data.                             */
      xHigherPriorityTaskWoken = pdFALSE;
      xSemaphoreGiveFromISR(UartContext.DataReceivedEvent, &xHigherPriorityTaskWoken);
      portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
      BTActivity++;
   }
}

void HCITR_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
   signed portBASE_TYPE xHigherPriorityTaskWoken;
   Boolean_t bRet;

   bRet = ProcessRxDMA();
   if(bRet)
   {
      /* Signal the reception of some data.                             */
      xHigherPriorityTaskWoken = pdFALSE;
      xSemaphoreGiveFromISR(UartContext.DataReceivedEvent, &xHigherPriorityTaskWoken);
      portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
      BTActivity++;
   }
}



   /* The following function handles the GPIO interrupt from the        */
   /* Bluetooth CTS line.  will make sure that the transmit interrupt is*/
   /* enabled when the flow control line is low.                        */
void HCITR_CTS_IRQ_HANDLER(void)
{
#ifdef SUPPORT_TRANSPORT_SUSPEND
   if(UartContext.SuspendState == hssSuspended)
   {
      /* Resume the UART.                                               */
      //EnableUartPeriphClock();
      HCITR_UART_CLK_ENABLE();
      HCITR_DMA_CLK_ENABLE();
      SetSuspendGPIO(FALSE);
      UartContext.SuspendState = hssNormal;
   }
   else
   {
      if(UartContext.SuspendState == hssSuspendWait)
      {
         /* Indicate the suspend is interrupted.                        */
         UartContext.SuspendState = hssSuspendWaitInterrupted;
      }
   }

   /* Enable the UART transmit interrupt if there is data in the buffer.*/
   if(UartContext.TxBytesFree != OUTPUT_BUFFER_SIZE)
   {
      //USART_ITConfig(HCITR_UART_BASE, USART_IT_TXE, ENABLE);
      StartTxDMATransfer();
   }
#endif
   //EXTI_ClearFlag(CTS_ExtIntConfiguration.EXTI_Line);
   if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_11) != RESET)
   {
     BTActivity++;
     __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);
   }
}



   /* The following function is responsible for opening the HCI         */
   /* Transport layer that will be used by Bluetopia to send and receive*/
   /* COM (Serial) data.  This function must be successfully issued in  */
   /* order for Bluetopia to function.  This function accepts as its    */
   /* parameter the HCI COM Transport COM Information that is to be used*/
   /* to open the port.  The final two parameters specify the HCI       */
   /* Transport Data Callback and Callback Parameter (respectively) that*/
   /* is to be called when data is received from the UART.  A successful*/
   /* call to this function will return a non-zero, positive value which*/
   /* specifies the HCITransportID that is used with the remaining      */
   /* transport functions in this module.  This function returns a      */
   /* negative return value to signify an error.                        */
int BTPSAPI HCITR_COMOpen(HCI_COMMDriverInformation_t *COMMDriverInformation, HCITR_COMDataCallback_t COMDataCallback, unsigned long CallbackParameter)
{
   int ret_val;

   /* First, make sure that the port is not already open and make sure  */
   /* that valid COMM Driver Information was specified.                 */
   if((!HCITransportOpen) && (COMMDriverInformation) && (COMDataCallback))
   {
      /* Initialize the return value for success.                       */
      ret_val               = TRANSPORT_ID;

      /* Initialize the context structure.                              */
      BTPS_MemInitialize(&UartContext, 0, sizeof(UartContext_t));

      UartContext.COMDataCallbackFunction  = COMDataCallback;
      UartContext.COMDataCallbackParameter = CallbackParameter;
      UartContext.TxBytesFree              = OUTPUT_BUFFER_SIZE;
      UartContext.RxBufferSize             = INPUT_BUFFER_SIZE;
      UartContext.SuspendState             = hssNormal;

      /* Flag that the HCI Transport is open.                           */
      HCITransportOpen = 1;

      /* Create the event that will be used to signal data has arrived. */
      vSemaphoreCreateBinary(UartContext.DataReceivedEvent);

      if(UartContext.DataReceivedEvent)
      {
         /* Make sure that the event is in the reset state.             */
         xSemaphoreTake(UartContext.DataReceivedEvent, 1);

         /* Create a thread that will process the received data.        */
         UartContext.ReceiveThreadHandle = BTPS_CreateThread(RxThread, 1600, NULL);

         if(!UartContext.ReceiveThreadHandle)
         {
            /* Failed to start the thread, delete the semaphore.        */
            vQueueDelete(UartContext.DataReceivedEvent);
            ret_val = HCITR_ERROR_UNABLE_TO_OPEN_TRANSPORT;
         }
      }
      else
         ret_val = HCITR_ERROR_UNABLE_TO_OPEN_TRANSPORT;

      /* If there was no error, then continue to setup the port.        */
      if(ret_val != HCITR_ERROR_UNABLE_TO_OPEN_TRANSPORT)
      {

          // Pin is initialized in pmic.c
          SetReset();
          
#ifdef STM32F415xx
          // Pin is initialized in pmic.c
          BT_HCI_OE_Disable();
          
#endif // STM32F415xx          
          
          
          HCITRUartHandle.Instance          = HCITR_UART_BASE;
  
          HCITRUartHandle.Init.BaudRate     = 115200;
          HCITRUartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
          HCITRUartHandle.Init.StopBits     = UART_STOPBITS_1;
          HCITRUartHandle.Init.Parity       = UART_PARITY_NONE;
          HCITRUartHandle.Init.HwFlowCtl    = UART_HWCONTROL_RTS_CTS;
          HCITRUartHandle.Init.Mode         = UART_MODE_TX_RX;
          HCITRUartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
          
          HCITRUartHandle.hdmarx = &HCITRDmaRxHandle;
          HCITRUartHandle.hdmatx = &HCITRDmaTxHandle;
    
          if(HAL_UART_Init(&HCITRUartHandle) != HAL_OK) {
            ret_val = HCITR_ERROR_UNABLE_TO_OPEN_TRANSPORT;
          }
         /* Reconfigure the baud rate to make sure it is as accurate as */
         /* possible.                                                   */
         SetBaudRate(HCITR_UART_BASE, COMMDriverInformation->BaudRate);
         
         StartRxDMATransfer();


#ifdef SUPPORT_TRANSPORT_SUSPEND

         /* Initialize the external interrupt for the CTS line          */
         //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
         //SYSCFG_EXTILineConfig(HCITR_CTS_EXTI_PORT, HCITR_CTS_PIN);
         //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, DISABLE);
         SetSuspendGPIO(FALSE);

#endif

#ifdef STM32F415xx
         // enable serial buffer
         BT_HCI_OE_Enable();
         // Turn on VSUP_BT
         //VSUP_BT_On();   already done in pmic.c
#endif // STM32F415xx
         
         /* Clear the reset.                                            */
         BTPS_Delay(10);
         ClearReset();
         BTPS_Delay(250);
      }
      else
         HCITransportOpen = 0;
   }
   else
      ret_val = HCITR_ERROR_UNABLE_TO_OPEN_TRANSPORT;

   return(ret_val);
}

   /* The following function is responsible for closing the specific HCI*/
   /* Transport layer that was opened via a successful call to the      */
   /* HCITR_COMOpen() function (specified by the first parameter).      */
   /* Bluetopia makes a call to this function whenever an either        */
   /* Bluetopia is closed, or an error occurs during initialization and */
   /* the driver has been opened (and ONLY in this case).  Once this    */
   /* function completes, the transport layer that was closed will no   */
   /* longer process received data until the transport layer is         */
   /* Re-Opened by calling the HCITR_COMOpen() function.                */
   /* * NOTE * This function *MUST* close the specified COM Port.  This */
   /*          module will then call the registered COM Data Callback   */
   /*          function with zero as the data length and NULL as the    */
   /*          data pointer.  This will signify to the HCI Driver that  */
   /*          this module is completely finished with the port and     */
   /*          information and (more importantly) that NO further data  */
   /*          callbacks will be issued.  In other words the very last  */
   /*          data callback that is issued from this module *MUST* be a*/
   /*          data callback specifying zero and NULL for the data      */
   /*          length and data buffer (respectively).                   */
void BTPSAPI HCITR_COMClose(unsigned int HCITransportID)
{
   HCITR_COMDataCallback_t COMDataCallback;

#ifdef SUPPORT_TRANSPORT_SUSPEND

   //EXTI_InitTypeDef ExtIntConfiguration;

#endif

   /* Check to make sure that the specified Transport ID is valid.      */
   if((HCITransportID == TRANSPORT_ID) && (HCITransportOpen))
   {
      /* Flag that the HCI Transport is no longer open.                 */
      HCITransportOpen = 0;

#ifdef SUPPORT_TRANSPORT_SUSPEND

      /* Disable external interrupt for the CTS line                    */
      //NVIC_DisableIRQ(HCITR_CTS_IRQ);
      //BTPS_MemCopy(&ExtIntConfiguration, &CTS_ExtIntConfiguration, sizeof(EXTI_InitTypeDef));
      //ExtIntConfiguration.EXTI_LineCmd = DISABLE;
      //EXTI_Init(&ExtIntConfiguration);
      SetSuspendGPIO(FALSE);

#endif


      HAL_UART_DeInit(&HCITRUartHandle);

      
      /* Place the Bluetooth Device in Reset.                           */
      SetReset();
      
#ifdef STM32F415xx
      // Disable serial buffer.
      BT_HCI_OE_Disable();
      //VSUP_BT_Off();     already done in pmic.c
#endif

      /* Signal the receive thread to terminate and wait for it to      */
      /* close.                                                         */
      xSemaphoreGive(UartContext.DataReceivedEvent);
      while(UartContext.ReceiveThreadHandle)
         BTPS_Delay(1);

      /* Close the semaphore.                                           */
      vQueueDelete((xQueueHandle)(UartContext.DataReceivedEvent));

      /* Note the Callback information.                                 */
      COMDataCallback   = UartContext.COMDataCallbackFunction;

      UartContext.COMDataCallbackFunction = NULL;

      /* All finished, perform the callback to let the upper layer know */
      /* that this module will no longer issue data callbacks and is    */
      /* completely cleaned up.                                         */
      if(COMDataCallback)
         (*COMDataCallback)(HCITransportID, 0, NULL, UartContext.COMDataCallbackParameter);

      UartContext.COMDataCallbackParameter = 0;
   }
}

   /* The following function is responsible for instructing the         */
   /* specified HCI Transport layer (first parameter) that was opened   */
   /* via a successful call to the HCITR_COMOpen() function to          */
   /* reconfigure itself with the specified information.                */
   /* * NOTE * This function does not close the HCI Transport specified */
   /*          by HCI Transport ID, it merely reconfigures the          */
   /*          transport.  This means that the HCI Transport specified  */
   /*          by HCI Transport ID is still valid until it is closed via*/
   /*          the HCI_COMClose() function.                             */
void BTPSAPI HCITR_COMReconfigure(unsigned int HCITransportID, HCI_Driver_Reconfigure_Data_t *DriverReconfigureData)
{
   HCI_COMMReconfigureInformation_t *ReconfigureInformation;

   /* Check to make sure that the specified Transport ID is valid.      */
   if((HCITransportID == TRANSPORT_ID) && (HCITransportOpen) && (DriverReconfigureData))
   {
      if((DriverReconfigureData->ReconfigureCommand == HCI_COMM_DRIVER_RECONFIGURE_DATA_COMMAND_CHANGE_COMM_PARAMETERS) && (DriverReconfigureData->ReconfigureData))
      {
         ReconfigureInformation = (HCI_COMMReconfigureInformation_t *)(DriverReconfigureData->ReconfigureData);

         /* Check if the baud rate needs to change.                     */
         if(ReconfigureInformation->ReconfigureFlags & HCI_COMM_RECONFIGURE_INFORMATION_RECONFIGURE_FLAGS_CHANGE_BAUDRATE)
         {
            DisableInterrupts();
            SetBaudRate(HCITR_UART_BASE, ReconfigureInformation->BaudRate);
            EnableInterrupts();
         }
      }
   }
}

   /* The following function is responsible for actually sending data   */
   /* through the opened HCI Transport layer (specified by the first    */
   /* parameter).  Bluetopia uses this function to send formatted HCI   */
   /* packets to the attached Bluetooth Device.  The second parameter to*/
   /* this function specifies the number of bytes pointed to by the     */
   /* third parameter that are to be sent to the Bluetooth Device.  This*/
   /* function returns a zero if the all data was transferred           */
   /* successfully or a negative value if an error occurred.  This      */
   /* function MUST NOT return until all of the data is sent (or an     */
   /* error condition occurs).  Bluetopia WILL NOT attempt to call this */
   /* function repeatedly if data fails to be delivered.  This function */
   /* will block until it has either buffered the specified data or sent*/
   /* all of the specified data to the Bluetooth Device.                */
   /* * NOTE * The type of data (Command, ACL, SCO, etc.) is NOT passed */
   /*          to this function because it is assumed that this         */
   /*          information is contained in the Data Stream being passed */
   /*          to this function.                                        */
int BTPSAPI HCITR_COMWrite(unsigned int HCITransportID, unsigned int Length, unsigned char *Buffer)
{
   int ret_val;
   int Count;
   int BytesFree;

#ifdef HCITR_ENABLE_DEBUG_LOGGING

   unsigned int Index;

#endif

   /* Check to make sure that the specified Transport ID is valid and   */
   /* the output buffer appears to be valid as well.                    */
   if((HCITransportID == TRANSPORT_ID) && (HCITransportOpen) && (Length) && (Buffer))
   {
      /* If the UART is suspended, resume it.                           */
      if(UartContext.SuspendState == hssSuspended)
      {
         DisableInterrupts();

         SetSuspendGPIO(FALSE);
         UartContext.SuspendState = hssNormal;

         EnableInterrupts();
      }

#ifdef HCITR_ENABLE_DEBUG_LOGGING

      if(UartContext.DebugEnabled)
      {
         DEBUG_PRINT("<");

         for(Index = 0; Index < Length; Index ++)
            DEBUG_PRINT(" %02X", Buffer[Index]);

         DEBUG_PRINT("\r\n");
      }

#endif

      /* Process all of the data.                                       */
      while(Length)
      {
         /* Wait for space in the transmit buffer.                      */
        while(!UartContext.TxBytesFree)
            BTPS_Delay(1);

         /* The data may have to be copied in 2 phases.  Calculate the  */
         /* number of character that can be placed in the buffer before */
         /* the buffer must be wrapped.                                 */
         BytesFree = UartContext.TxBytesFree;
         Count = Length;
         Count = (BytesFree < Count) ? BytesFree : Count;
         Count = ((OUTPUT_BUFFER_SIZE - UartContext.TxInIndex) < Count) ? (OUTPUT_BUFFER_SIZE - UartContext.TxInIndex) : Count;

         BTPS_MemCopy(&(UartContext.TxBuffer[UartContext.TxInIndex]), Buffer, Count);

         /* Update the number of free bytes in the buffer.  Since this  */
         /* count can also be updated in the interrupt routine, we will */
         /* have to update this with interrupts disabled.               */

         /* Adjust the index values.                                    */
         Buffer                  += Count;
         Length                  -= Count;
         UartContext.TxInIndex   += Count;
         if(UartContext.TxInIndex == OUTPUT_BUFFER_SIZE)
            UartContext.TxInIndex = 0;

         /* Update the bytes free and make sure the transmit interrupt  */
         /* is enabled.                                                 */
         DisableInterrupts();
         UartContext.TxBytesFree -= Count;
         StartTxDMATransfer();
         EnableInterrupts();
      }

      ret_val = 0;
   }
   else
      ret_val = HCITR_ERROR_WRITING_TO_PORT;

   return(ret_val);
}

   /* The following function is responsible for suspending the HCI COM  */
   /* transport.  It will block until the transmit buffers are empty and*/
   /* all data has been sent then put the transport in a suspended      */
   /* state.  This function will return a value of 0 if the suspend was */
   /* successful or a negative value if there is an error.              */
   /* * NOTE * An error will occur if the suspend operation was         */
   /*          interrupted by another event, such as data being received*/
   /*          before the transmit buffer was empty.                    */
   /* * NOTE * The calling function must lock the Bluetooth stack with a*/
   /*          call to BSC_LockBluetoothStack() before this function is */
   /*          called.                                                  */
   /* * NOTE * This function should only be called when the baseband    */
   /*          low-power protocol in use has indicated that it is safe  */
   /*          to sleep.  Also, this function must be called            */
   /*          successfully before any clocks necessary for the         */
   /*          transport to operate are disabled.                       */
int BTPSAPI HCITR_COMSuspend(unsigned int HCITransportID)
{
   int ret_val;

#ifdef SUPPORT_TRANSPORT_SUSPEND

   if(HCITransportID == TRANSPORT_ID)
   {
      /* Signal that we are waiting for a suspend operation to complete.*/
      UartContext.SuspendState = hssSuspendWait;

      /* Set the CTS interrupt.                                         */
      SetSuspendGPIO(TRUE);

      /* Wait for the UART transmit buffer and FIFO to be empty.        */
      //while(((UartContext.TxBytesFree != OUTPUT_BUFFER_SIZE) || (UartContext.TxPreviousDMALength) || (USART_GetFlagStatus(HCITR_UART_BASE, USART_FLAG_TC) != SET)) && (UartContext.SuspendState == hssSuspendWait))
      while(((UartContext.TxBytesFree != OUTPUT_BUFFER_SIZE) || (UartContext.TxPreviousDMALength) || (__HAL_UART_GET_FLAG(&HCITRUartHandle, USART_SR_TC) != SET)) && (UartContext.SuspendState == hssSuspendWait))
      {
         BTPS_Delay(1);
      }
      
      /* Confirm that no data was received in this time and suspend the */
      /* UART.                                                          */
      DisableInterrupts();

      if(UartContext.SuspendState == hssSuspendWait)
      {
         UartContext.SuspendState = hssSuspended;

         /* Disable the UART clock.                                     */
         //DisableUartPeriphClock();
         HCITR_UART_CLK_ENABLE();
         HCITR_DMA_CLK_ENABLE();

         ret_val = 0;
      }
      else
      {
         /* Data was received, abort suspending the UART.               */
         SetSuspendGPIO(FALSE);

         ret_val = HCITR_ERROR_SUSPEND_ABORTED;
      }

      EnableInterrupts();

   }
   else
      ret_val = HCITR_ERROR_INVALID_PARAMETER;

#else

   ret_val = HCITR_ERROR_INVALID_PARAMETER;

#endif

   return(ret_val);
}

   /* The following function is used to enable or disable debug logging */
   /* within HCITRANS.  The function accepts as its parameter a flag    */
   /* which indicates if debugging should be enabled.  It returns zero  */
   /* if successful or a negative value if there was an error.          */
int BTPSAPI HCITR_EnableDebugLogging(Boolean_t Enable)
{
   int ret_val;

#ifdef HCITR_ENABLE_DEBUG_LOGGING

   UartContext.DebugEnabled = Enabled;

   ret_val = 0;

#else

   ret_val = HCITR_ERROR_INVALID_PARAMETER;

#endif

   return(ret_val);
}
