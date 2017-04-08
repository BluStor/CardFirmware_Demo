
/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "fatfs.h"
#include <string.h>
#include "pmic.h"

/* Private typedef -----------------------------------------------------------*/
typedef  void (*pFunction)(void);

/* Private define ------------------------------------------------------------*/
#define READ_BUFF_SIZE  512

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
pFunction Jump_To_Application;
uint32_t  JumpAddress;

/* UART handler declaration */
UART_HandleTypeDef UartHandle;  /* for debug purpose only */

/* I2C handler declaration */
I2C_HandleTypeDef I2cHandle;

uint8_t retSD;          /* Return value for SD */
char SD_Path0[4];        /* SD logical drive path */
FATFS SDFatFs;          /* File system object for SD card logical drive */
FIL MyFile;             /* File object */
UINT BytesCount;
#ifdef BOOT_48K
U_FIRMWARE_HEADER* pFlashHeader = (U_FIRMWARE_HEADER*)USER_FIRMWARE_FIRST_PAGE_ADDRESS_48K; /* pointeur on the actual firmware header in flash */
#else
U_FIRMWARE_HEADER* pFlashHeader = (U_FIRMWARE_HEADER*)USER_FIRMWARE_FIRST_PAGE_ADDRESS_32K; /* pointeur on the actual firmware header in flash */
#endif
U_FIRMWARE_HEADER FileHeader;           /* header of the firmware file */
uint8_t ReadBuff[READ_BUFF_SIZE];       /* buffer used to read firmware update file. */
static __IO uint32_t FlashWriteAddress; /* Used to track where to write in FLASH */
unsigned short Crc16;
unsigned short ValidFirmware;
uint8_t* pFirmware;
uint8_t* pFile;
unsigned char zeros[2] = {0, 0};
#ifdef BOOT_48K
uint8_t FirmwareSignature[] = FIRMWARE_SIGNATURE_48K;
#else
uint8_t FirmwareSignature[] = FIRMWARE_SIGNATURE_32K;
#endif // BOOT_48K

#ifdef BOOTLOADER_BINARY
#ifdef BOOT_48K
const U_FIRMWARE_HEADER BootloaderHeader_48K @ (USER_BOOTLOADER_FIRST_PAGE_ADDRESS_48K - 0x200)  = {BOOTLOADER_SIGNATURE,
                                                                             USER_BOOTLOADER_FIRST_PAGE_ADDRESS_48K,
                                                                             0x0800BFFF,
                                                                             VERSION_BOOTLOADER_MAJEUR,
                                                                             VERSION_BOOTLOADER_MINEUR
                                                                             };
#endif
#endif






void DebugPrint(uint8_t* _pBuff)
{
  while(*_pBuff != 0)
  {
    HAL_UART_Transmit(&UartHandle, _pBuff, 1, 0xFFFF);
    _pBuff++;
  }
}



uint8_t ChkValidFirmware(char* Signature)
{
  int x;
 
  for(x=0;x<sizeof(FirmwareSignature);x++)
  {
    if(Signature[x] != FirmwareSignature[x])
    {
      return 0;
    }
  }
  
  return 1;
}

#pragma required = BootloaderMajeur
#pragma required = BootloaderMineur
#ifdef BOOTLOADER_BINARY
#ifdef BOOT_48K
#pragma required = BootloaderHeader_48K
#endif
#endif
int main()
{
  uint32_t x;
  uint8_t Value;
  int WaitUSBPower = 100;
  HAL_StatusTypeDef status;
 //  GPIO_InitTypeDef GPIO_InitStruct;
   
 // read flashed parameters
  FLASH_If_ReadParam();
  
  if(RamParam.BootCmd != CMD_UPDATE_FIRMWARE)
  {
    /* Check if a valid firmware signature is present in FLASH */
    if(ChkValidFirmware(pFlashHeader->Header.Signature))
    {
      /* Jump to user application */
#ifdef BOOT_48K
      JumpAddress = *(__IO uint32_t*) (USER_FIRMWARE_FIRST_PAGE_ADDRESS_48K + sizeof(U_FIRMWARE_HEADER) + 4);
#else
      JumpAddress = *(__IO uint32_t*) (USER_FIRMWARE_FIRST_PAGE_ADDRESS_32K + sizeof(U_FIRMWARE_HEADER) + 4);
#endif
      Jump_To_Application = (pFunction) JumpAddress;
      /* Initialize user application's Stack Pointer */
#ifdef BOOT_48K
      __set_MSP(*(__IO uint32_t*) (USER_FIRMWARE_FIRST_PAGE_ADDRESS_48K + sizeof(U_FIRMWARE_HEADER)));
#else      
      __set_MSP(*(__IO uint32_t*) (USER_FIRMWARE_FIRST_PAGE_ADDRESS_32K + sizeof(U_FIRMWARE_HEADER)));
#endif // BOOT_48K
      Jump_To_Application();
      /* do nothing */
      while(1);
    }
    else
    {
      // invalid firmware... check if a firmware is present on eMMC... it's better than nothing.
    }
  }

  
  /* STM32F4xx HAL library initialization:
     - Configure the Flash prefetch, instruction and Data caches
     - Configure the Systick to generate an interrupt each 1 msec
     - Set NVIC Group Priority to 4
     - Global MSP (MCU Support Package) initialization
   */
  HAL_Init();
 
  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  
  // Add Uart support for debug purpose
  /*##-1- Configure the UART peripheral ######################################*/
  /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
  /* UART3 configured as follow:
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - Parity = ODD parity
      - BaudRate = 9600 baud
      - Hardware flow control disabled (RTS and CTS signals) */
  UartHandle.Instance          = EVAL_COM1;
  UartHandle.Init.BaudRate     = 115200;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
  HAL_UART_Init(&UartHandle);
 
    /*##-1- Configure the I2C peripheral #######################################*/
#ifdef STM32F415xx
  I2cHandle.Instance             = EVAL_I2Cx;

  I2cHandle.Init.ClockSpeed      = BSP_I2C_SPEED; 
  I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  I2cHandle.Init.OwnAddress1     = 0x00;
  I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2cHandle.Init.OwnAddress2     = 0x00;
  I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

  HAL_I2C_Init(&I2cHandle); 

  BSP_LED_Init(LED_RED);
  BSP_LED_Init(LED_YELLOW);
  
  
  if (PMICPreInit()) 
  {
    DebugPrint("Error PMICPreInit Failed!\r\n");
#if 0 // special code for wake-on-usb problem
    DebugPrint("Wait for Off\r\n");
    WaitUSBPower = 300;
    do {
      // USB power not present make led blink.
      if(WaitUSBPower & 0x00000001)
      {
        BSP_LED_On(LED_RED);
        BSP_LED_Off(LED_YELLOW);
      }else
      {
        BSP_LED_Off(LED_RED);
        BSP_LED_On(LED_YELLOW);
      }
      HAL_Delay(100);

    } while(WaitUSBPower--);
#endif    
    goto error;
  }

  if(PMICInit())
  {
    DebugPrint("Error PMICInit Failed!\r\n");
    goto error;
  }
#endif // STM32F415xx
  
#ifdef STM32F415xx  
  BSP_LED_On(LED_RED);
  BSP_LED_On(LED_YELLOW);
  
  // USB power must be present in order to do an update
  /*
  do
  {
    status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_CHG_STAT2, 
                              I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
  
    if(status != HAL_OK)
    {
      DebugPrint("Error reading PMIC_I2C_CHG_STAT2.\r\n");
      goto error;
    }
    
    if(Value & PMIC_I2C_CHG_STAT2_CHDET)
    {
      break;
    }else
    {
      // USB power not present make led blink.
      if(WaitUSBPower & 0x00000001)
      {
        BSP_LED_On(LED_RED);
        BSP_LED_Off(LED_YELLOW);
      }else
      {
        BSP_LED_Off(LED_RED);
        BSP_LED_On(LED_YELLOW);
      }
      HAL_Delay(100);
    }
  }while(WaitUSBPower--);
  
  if(WaitUSBPower<=0)
  {
    DebugPrint("Error :  No USB Power.\r\n");
    PMICSetBoardOff();
    while(1);
  } 
  
  BSP_LED_On(LED_RED);
  BSP_LED_On(LED_YELLOW);
  */
  
#endif // STM32F415xx

  /* Enter in IAP mode */
  DebugPrint("\r\nEnter IAP.\r\n");
  
  DebugPrint("Mount File System...\r\n");
  retSD = FATFS_LinkDriver(&SD_Driver, SD_Path0);
  if(retSD != FR_OK)
  {
    DebugPrint("Error in FATFS_LinkDriver().\r\n");
    goto error;
  }
  
  retSD = f_mount(&SDFatFs, (TCHAR const*)SD_Path0, 1);        // force mount immediatly
  if(retSD != FR_OK)
  {
    DebugPrint("Error in f_mount().\r\n");
    goto error;
  }
  
  DebugPrint("Search for firmware file...\r\n");
  retSD = f_open(&MyFile, FIRMWARE_FILENAME, FA_READ);
  if(retSD != FR_OK)
  {
    DebugPrint("Error opening file.\r\n");
    goto error;
  }
  
  // we must read all file to validate it CRC.
  DebugPrint("Calculate firmware file CRC16...");
  // We compare file with current firmware in FLASH. If it is identical, firmware update is succesful.
  ValidFirmware = true;
#ifdef BOOT_48K
  pFirmware = (uint8_t *)(USER_FIRMWARE_FIRST_PAGE_ADDRESS_48K + sizeof(FileHeader));
#else
  pFirmware = (uint8_t *)(USER_FIRMWARE_FIRST_PAGE_ADDRESS_32K + sizeof(FileHeader));
#endif // BOOT_48K
  
  retSD = f_read(&MyFile, (void*)&FileHeader, sizeof(FileHeader), (UINT*)&BytesCount);
  if((retSD != FR_OK) || (BytesCount != sizeof(FileHeader)))
  {
    DebugPrint(" error reading Header!\r\n");
    goto error;
  }
  
  Crc16=0;
  do
  {
    retSD = f_read(&MyFile, ReadBuff, READ_BUFF_SIZE, (UINT*)&BytesCount);
    if(retSD != FR_OK)
    {
      DebugPrint(" error reading file!\r\n");
      goto error;
    }
    pFile = ReadBuff;
    for(x=0;x<BytesCount;x++)
    {
      if(*pFile != *pFirmware)
      {
        ValidFirmware = false;
      }
      pFile++;
      pFirmware++;
    }
    Crc16 = slow_crc16(Crc16, ReadBuff, BytesCount);
  }while((retSD == FR_OK) && (BytesCount == READ_BUFF_SIZE));
  Crc16 = slow_crc16(Crc16, zeros, 2);
  
  if(Crc16 != FileHeader.Crc.Crc16)
  {
    DebugPrint(" error bad CRC!\r\n");
    goto error;
  }
  
  DebugPrint(" good!\r\n");
  
  if(ValidFirmware)
  {
    DebugPrint("Firmware sucessfully writen in FLASH.\r\n");
    RamParam.BootCmd = CMD_NONE;
    FLASH_If_SaveParam();
    goto exit;
  }
  
  
  
  // replace file pointer at the begin of firmware... just after the file header.
  retSD = f_lseek(&MyFile, sizeof(U_FIRMWARE_HEADER));
  if(retSD != FR_OK)
  {
    DebugPrint("Error in f_lseek().\r\n");
    goto error;
  }

  /* Erase USER FLASH */
  DebugPrint("Erase User Flash...\r\n");
  FLASH_If_Unlock();
#ifdef BOOT_48K
  FLASH_If_Erase(USER_FIRMWARE_FIRST_PAGE_ADDRESS_48K);
#else
  FLASH_If_Erase(USER_FIRMWARE_FIRST_PAGE_ADDRESS_32K);
#endif // BOOT_48K
  
  DebugPrint("Write Flash\r\n");
  // Don't write Firmware header right now... 
#ifdef BOOT_48K
  FlashWriteAddress = USER_FIRMWARE_FIRST_PAGE_ADDRESS_48K + sizeof(U_FIRMWARE_HEADER);
#else
  FlashWriteAddress = USER_FIRMWARE_FIRST_PAGE_ADDRESS_32K + sizeof(U_FIRMWARE_HEADER);
#endif // BOOT_48K
  while((FlashWriteAddress < USER_FIRMWARE_END_ADDRESS)&&(retSD == FR_OK))
  {
    DebugPrint(".");
    memset(ReadBuff,0,READ_BUFF_SIZE);
    BytesCount=0;
    retSD = f_read(&MyFile, ReadBuff, READ_BUFF_SIZE, (UINT*)&BytesCount);
    if(retSD != FR_OK)
    {
      DebugPrint("Error reading file\r\n");
      goto error;
    }
       
    if(BytesCount > 0)
    {
      FLASH_If_Write8(&FlashWriteAddress, (uint8_t*)(ReadBuff),BytesCount);
    }else
    {
      DebugPrint("\r\nEOF\r\n");
    }
    
    if(BytesCount != READ_BUFF_SIZE )
    {
      // reach end of file!
      break;
    }
  }
  
  // write Firmware Header in last, that way bootloader will be able to know
  // that the firmware have been completly writen.
#ifdef BOOT_48K
  FlashWriteAddress = USER_FIRMWARE_FIRST_PAGE_ADDRESS_48K;
#else
  FlashWriteAddress = USER_FIRMWARE_FIRST_PAGE_ADDRESS_32K;
#endif // BOOT_48K
  FLASH_If_Write8(&FlashWriteAddress, (uint8_t*)(&FileHeader),sizeof(FileHeader));

  if(retSD == FR_OK)
  {
    DebugPrint("\r\nFirmware update done.\r\n");
    //RamParam.BootCmd = CMD_NONE;
    //FLASH_If_SaveParam();
  }
  retSD = f_close(&MyFile);
  
exit:  
  FLASH_If_Lock();
  for(;;)
  {
    // exit bootloader. Reset to start firmware mode.
    HAL_NVIC_SystemReset();
  }
  
error:
    DebugPrint("Error - cancel firmware update!\r\n");
    RamParam.BootCmd = CMD_NONE;
    FLASH_If_SaveParam();
    goto exit;
}
