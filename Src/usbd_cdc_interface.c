/**
  ******************************************************************************
  * @file    USB_Device/CDC_Standalone/Src/usbd_cdc_interface.c
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    14-August-2015
  * @brief   Source file for USBD CDC interface
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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#ifdef USE_USB_FS
#define CDC_RX_DATA_SIZE  64
#endif
#ifdef USE_USB_HS
#define CDC_RX_DATA_SIZE  512
#endif
//#define APP_TX_DATA_SIZE  64
#define CDC_RX_BUFF_NB  100     // if we go below 70, the delay between 512 bytes block transfert increase significally.

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

USBD_CDC_LineCodingTypeDef LineCoding =
  {
    115200, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
  };

typedef struct _S_USB_RX_BUFF
  {
#if CDC_RX_DATA_SIZE < 256
  uint8_t Len;
#else
  uint16_t Len;
#endif
  uint8_t RxBuff[CDC_RX_DATA_SIZE];
}S_USB_RX_BUFF;

// FIFO of Rx Buffers
uint8_t CDC_RxTail;
uint8_t CDC_RxHead;
S_USB_RX_BUFF CDC_RxBuff[CDC_RX_BUFF_NB];
volatile uint8_t CDC_RxFifoFull;
extern int BT_WriteABuffer(const char * lpBuf, DWORD dwToWrite);
uint8_t CDC_NeedReset = FALSE;

//uint8_t UserRxBuffer[APP_RX_DATA_SIZE];/* Received Data over USB are stored in this buffer */
//uint8_t UserTxBuffer[APP_TX_DATA_SIZE];/* Received Data over UART (CDC interface) are stored in this buffer */

/* USB handler declaration */
USBD_HandleTypeDef  USBD_Device;

/* USB handler declaration */
extern USBD_HandleTypeDef  USBD_Device;
extern int (*pWriteABuffer)(const char*, DWORD);

/* Private function prototypes -----------------------------------------------*/
static int8_t CDC_Itf_Init(void);
static int8_t CDC_Itf_DeInit(void);
static int8_t CDC_Itf_Control(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Itf_Receive(uint8_t* pbuf, uint32_t *Len);
static void CDC_RxThread(void const *argument);
int CDC_WriteABuffer(const char * lpBuf, DWORD dwToWrite);

uint32_t CDC_RxCharInt=0;
uint32_t CDC_RxCharThread=0;

extern int HandleASuccessfulRead(char *lpBuf, DWORD dwRead );

USBD_CDC_ItfTypeDef USBD_CDC_fops = 
{
  CDC_Itf_Init,
  CDC_Itf_DeInit,
  CDC_Itf_Control,
  CDC_Itf_Receive
};

osSemaphoreId CDC_RxSemaphore;

bool CDC_Started = FALSE;

void CDC_Start(void)
{
  if(CDC_Started == FALSE)
  {
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
    USBD_Start(&USBD_Device);
    CDC_Started = TRUE;
    printf("Start CDC.\r\n");
  }
}

void CDC_Stop(void)
{
  if(CDC_Started)
  {
    USBD_Stop(&USBD_Device);
    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
    CDC_Started = FALSE;
    printf("Stop CDC\r\n");
  }
}


void CDC_Init(void)
{
  osSemaphoreDef(CDC_SEM);
  CDC_RxSemaphore = osSemaphoreCreate(osSemaphore(CDC_SEM) , 1);
  
  osThreadDef(CDC_Thread, CDC_RxThread, osPriorityBelowNormal/*osPriorityNormal*/, 0, 8 * configMINIMAL_STACK_SIZE);
  osThreadCreate(osThread(CDC_Thread), NULL);
  
  /* Init Device Library */
  USBD_Init(&USBD_Device, &VCP_Desc, 0);
  
  /* Add Supported Class */
  USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS);
  
  /* Add CDC Interface Class */
  USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops);
  
  /* Start Device Process */
  USBD_Start(&USBD_Device);
  CDC_Started = TRUE;
  
  // STOP USB in order to reduce power.
  //USBD_Stop(&USBD_Device);
  CDC_Stop();
  //__HAL_RCC_USB_OTG_FS_CLK_DISABLE();
}

void CDC_Reset(void)
{
  memset(CDC_RxBuff,0,sizeof(CDC_RxBuff));
  osSemaphoreRelease(CDC_RxSemaphore);
  
  USBD_LL_Reset(&USBD_Device);
  
  /* Start Device Process */
  USBD_Start(&USBD_Device);
  
  printf("CDC_Reset\r\n");
}


/* Private functions ---------------------------------------------------------*/

/**
  * @brief  CDC_Itf_Init
  *         Initializes the CDC media low layer, called when usb cable is
  *         inserted. (called from an interruption)
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Init(void)
{
  //CDC_RxLen=0;
  CDC_RxFifoFull=0;
  CDC_RxTail = 0;
  CDC_RxHead = 0;
  memset(CDC_RxBuff,0,sizeof(CDC_RxBuff));
  
  printf("CDC insert cable.\r\n");
  
  USBD_CDC_SetTxBuffer(&USBD_Device, /*UserTxBuffer*/NULL, 0);
  USBD_CDC_SetRxBuffer(&USBD_Device, CDC_RxBuff[CDC_RxHead].RxBuff);
  
  return (USBD_OK);
}

/**
  * @brief  CDC_Itf_DeInit
  *         DeInitializes the CDC media low layer, called when USB cable is
  *         removed. (called from an interruption).
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_DeInit(void)
{
  pWriteABuffer = BT_WriteABuffer;
  printf("CDC remove cable.\r\n");
  CDCOpened = FALSE;
  CDC_NeedReset = TRUE;
  //eMMC_TurnOff = TRUE;
  return (USBD_OK);
}

/**
  * @brief  CDC_Itf_Control
  *         Manage the CDC class requests
  * @param  Cmd: Command code            
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Itf_Control (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{ 
  USBD_SetupReqTypedef * req = (USBD_SetupReqTypedef *)pbuf;   // for No Data request    
  
  printf("CDC_Itf_Control(0x%02X)\r\n",cmd);
  
  switch (cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:
    /* Add your code here */
    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:
    /* Add your code here */
    break;

  case CDC_SET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_GET_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_CLEAR_COMM_FEATURE:
    /* Add your code here */
    break;

  case CDC_SET_LINE_CODING:
    LineCoding.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |\
                            (pbuf[2] << 16) | (pbuf[3] << 24));
    LineCoding.format     = pbuf[4];
    LineCoding.paritytype = pbuf[5];
    LineCoding.datatype   = pbuf[6];
    break;

  case CDC_GET_LINE_CODING:
    pbuf[0] = (uint8_t)(LineCoding.bitrate);
    pbuf[1] = (uint8_t)(LineCoding.bitrate >> 8);
    pbuf[2] = (uint8_t)(LineCoding.bitrate >> 16);
    pbuf[3] = (uint8_t)(LineCoding.bitrate >> 24);
    pbuf[4] = LineCoding.format;
    pbuf[5] = LineCoding.paritytype;
    pbuf[6] = LineCoding.datatype;     
    break;

  case CDC_SET_CONTROL_LINE_STATE:
    /* Add your code here */
    if (req->wValue & CDC_DTR_MASK) {
        //
        // host set DTR to '1'
        //
        CDCOpened = TRUE;
        printf("\r\nCDC Open.\r\n");
        eMMC_TurnOn = TRUE;
    } else {
        //
        // host reset DTR to '0'
        //
        CDCOpened = FALSE;
        printf("CDC Close.\r\n");
        //eMMC_TurnOff = TRUE;
    }

    break;

  case CDC_SEND_BREAK:
     /* Add your code here */
    break;    
    
  default:
    break;
  }
  
  return (USBD_OK);
}

/**
  * @brief  CDC_Itf_DataRx
  *         Data received over USB OUT endpoint are sent over CDC interface 
  *         through this function.
  * @param  Buf: Buffer of data to be transmitted
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
    static int8_t CDC_Itf_Receive(uint8_t* Buf, uint32_t *Len)
{  
  //BSP_LED_On(LED3);
  
  CDC_RxBuff[CDC_RxHead].Len = *Len;
  CDC_RxCharInt += *Len;
  
  CDC_RxHead++;
  if(CDC_RxHead >= CDC_RX_BUFF_NB)
  {
    CDC_RxHead=0;
  }
  
  if(CDC_RxBuff[CDC_RxHead].Len == 0)
  {
    // next buffer is empty... start a new transfert.
    //BSP_LED_Off(LED3);
    USBD_CDC_SetRxBuffer(&USBD_Device, CDC_RxBuff[CDC_RxHead].RxBuff);
    USBD_CDC_ReceivePacket(&USBD_Device);         // indicate that we have consummed USB data and that we are ready to receive new one.
    }else
    {
    CDC_RxFifoFull=1;
  }
  osSemaphoreRelease(CDC_RxSemaphore);          // indicate to CDC_RxThread that data are available.
  
  return (USBD_OK);
}

#define INVALID_PARAMETERS_ERROR                   (-6)  /* Denotes that an   */
                                                         /* error occurred due*/
                                                         /* to the fact that  */
                                                         /* one or more of the*/
                                                         /* required          */
                                                         /* parameters were   */
                                                         /* invalid.          */

#define CDC_TX_BUFF_SIZE        (512+5)
char CDC_TxBuff[CDC_TX_BUFF_SIZE];
USBD_CDC_HandleTypeDef *pCDCClassData = NULL;
volatile int Stop=1;
int CDC_WriteABuffer(const char * lpBuf, DWORD dwToWrite)
{
   int  ret_val = 0;
   uint32_t TimeoutStart = HAL_GetTick();
   Stop = 1;
   
   if(CDCOpened == FALSE)
   {
     ret_val = INVALID_PARAMETERS_ERROR;
     printf("CDC_WriteABuffer - CDC closed! %d\r\n", dwToWrite);
     goto end;
   }
   
   //printf("CDC_WriteABuffer(%d)\r\n",dwToWrite);
   
   if(dwToWrite > CDC_TX_BUFF_SIZE)
   {
     ret_val = INVALID_PARAMETERS_ERROR;
     printf("CDC_WriteABuffer - Request to write is to big! %d\r\n", dwToWrite);
     goto end;
   }
   
   pCDCClassData = (USBD_CDC_HandleTypeDef*) USBD_Device.pClassData;
   //USBD_CDC_HandleTypeDef   *hcdc = (USBD_CDC_HandleTypeDef*) pdev->pClassData;
   
   // wait until current CDC_TxBuff is completly transmitted.
   while((((USBD_CDC_HandleTypeDef*) USBD_Device.pClassData)->TxState) && (HAL_GetTick()-TimeoutStart < 1000) && CDCOpened);
   
   if(((USBD_CDC_HandleTypeDef*) USBD_Device.pClassData)->TxState)
   {
     // Timeout, communication is broken
     ret_val = -1;
     printf("CDC_WriteABuffer - TimeOut\r\n");
     goto end;
   }
   
   if(CDCOpened == FALSE)
   {
     ret_val = INVALID_PARAMETERS_ERROR;
     printf("CDC_WriteABuffer - CDC closed! %d\r\n", dwToWrite);
     goto end;
   }
   
   memset(CDC_TxBuff,0,sizeof(CDC_TxBuff));
   memcpy(CDC_TxBuff, lpBuf, dwToWrite);
   
   USBD_CDC_SetTxBuffer(&USBD_Device, (unsigned char*)CDC_TxBuff, dwToWrite);
   while(USBD_CDC_TransmitPacket(&USBD_Device) == USBD_BUSY);
end:
   return(ret_val);
}


static void CDC_RxThread(void const *argument)
{
  for(;;)
  {
    if (CDC_RxSemaphore != NULL)
    {
      /* Try to obtain the semaphore */
      if(osSemaphoreWait(CDC_RxSemaphore , osWaitForever) == osOK)
      {
        while(CDC_RxBuff[CDC_RxTail].Len)
        {
          CDC_RxCharThread += CDC_RxBuff[CDC_RxTail].Len;
          //printf("CDC_Rx %d char\r\n",CDC_RxBuff[CDC_RxTail].Len);
          if (pWriteABuffer == CDC_WriteABuffer) {
            HandleASuccessfulRead((char*)CDC_RxBuff[CDC_RxTail].RxBuff, CDC_RxBuff[CDC_RxTail].Len);      // pass received Data to FTP server.
          }

          CDC_RxBuff[CDC_RxTail].Len = 0;   // mark buffer as free.
          
          CDC_RxTail++;
          if(CDC_RxTail >= CDC_RX_BUFF_NB)
          {
            CDC_RxTail=0;
          }
          
          if(CDC_RxFifoFull)
          {
            CDC_RxFifoFull=0;
            USBD_CDC_SetRxBuffer(&USBD_Device, CDC_RxBuff[CDC_RxHead].RxBuff);
            //BSP_LED_Off(LED3);
            USBD_CDC_ReceivePacket(&USBD_Device);         // indicate that we have consummed USB data and that we are ready to receive new one.
          }
        }
      }
    }
  }
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
