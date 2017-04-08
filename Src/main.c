/**
  ******************************************************************************
  * @file    FatFs/FatFs_uSD_RTOS/Src/main.c 
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    14-August-2015
  * @brief   Main program body
  *          This sample code shows how to use FatFs with uSD card drive in RTOS
  *          mode.
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
#include "pmic.h"   
#include "slog.h"
   
extern void Reset_Handler(void);

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Versions of bootloader and firmware ---------------------------------------*/
// Bootloader version will stay at the same place in 32k and 48k version.
#ifdef BOOTLOADER
// fix bootloader version in flash so firmware can read it.
const uint8_t BootloaderMajeur @ VERSION_BOOTLOADER_MAJEUR_ADDRESS = VERSION_BOOTLOADER_MAJEUR;
const uint8_t BootloaderMineur @ VERSION_BOOTLOADER_MINEUR_ADDRESS = VERSION_BOOTLOADER_MINEUR;
#else
// pointeur used by firmware to read bootloader version.
const uint8_t* pBootloaderMajeur = (uint8_t*)(VERSION_BOOTLOADER_MAJEUR_ADDRESS);   
const uint8_t* pBootloaderMineur = (uint8_t*)(VERSION_BOOTLOADER_MINEUR_ADDRESS);
#endif    

/* Firmware header -----------------------------------------------------------*/
#ifdef FIRMWARE
// Firmware header, used by bootloader to validate than a valid firmware is present in FLASH.
#ifdef BOOT_48K
// This header is used with the new 48k bootloader.
const U_FIRMWARE_HEADER FirmwareHeader_48K @ USER_FIRMWARE_FIRST_PAGE_ADDRESS_48K = {FIRMWARE_SIGNATURE_48K,
                                                                             USER_FIRMWARE_FIRST_PAGE_ADDRESS_48K + 0x200,
                                                                             0x080FFFFF,
                                                                             VERSION_FIRMWARE_MAJEUR,
                                                                             VERSION_FIRMWARE_MINEUR
                                                                             };
#ifdef HYBRID
// On hybrid firmware, we have a second header that is compatible with 32k bootloader.
const U_FIRMWARE_HEADER FirmwareHeader_HYBRID @ USER_SAVE_PARAM_ADDRESS_48K = {FIRMWARE_SIGNATURE_32K,
                                                                             USER_SAVE_PARAM_ADDRESS_48K + 0x200,
                                                                             0x080FFFFF,
                                                                             VERSION_FIRMWARE_MAJEUR,
                                                                             VERSION_FIRMWARE_MINEUR
                                                                             };
// We also need a special interrupt vector table located as in a 32k bootloader
// so bootloader can jump in firmware. We only need to have the address of the reset vector.

//const void (*pReset_Handler)(void) @ (USER_SAVE_PARAM_ADDRESS_48K + 0x200 + 4) = (const void (*)(void))Reset_Handler;
const uint32_t pReset_Handler[2] @(USER_SAVE_PARAM_ADDRESS_48K + 0x200) = {0x20018c78,0x08043A41};//((uint32_t)Reset_Handler);


#endif // HYBRID
#else // BOOT 32k
const U_FIRMWARE_HEADER FirmwareHeader_32K @ USER_FIRMWARE_FIRST_PAGE_ADDRESS_32K = {FIRMWARE_SIGNATURE_32K,
                                                                             USER_FIRMWARE_FIRST_PAGE_ADDRESS_32K + 0x200,
                                                                             0x080FFFFF,
                                                                             VERSION_FIRMWARE_MAJEUR,
                                                                             VERSION_FIRMWARE_MINEUR
                                                                             };
#endif // BOOT_48K
#endif // FIRMWARE
  








/* Saved param in FLASH ------------------------------------------------------*/
sFLASH_PARAM RamParam;

#ifndef HYBRID
#ifndef FIRMWARE_BINARY

#if 0   // 1 = development, no forced format of eMMC
#ifdef BOOT_48K
const sFLASH_PARAM FlashParam @ USER_SAVE_PARAM_ADDRESS_48K = {CMD_NONE,0,1};
#else
const sFLASH_PARAM FlashParam @ USER_SAVE_PARAM_ADDRESS_32K = {CMD_NONE,0,1};
#endif // BOOT_48K
#else
#ifdef BOOT_48K
const sFLASH_PARAM FlashParam @ USER_SAVE_PARAM_ADDRESS_48K = {CMD_NONE,1,1};
#else
const sFLASH_PARAM FlashParam @ USER_SAVE_PARAM_ADDRESS_32K = {CMD_NONE,1,1};
#endif // BOOT_48K
#endif

#endif // FIRMWARE_BINARY
#endif // HYBRID


#ifdef BOOT_48K
const sFLASH_PARAM* pFlashParam = (sFLASH_PARAM*)USER_SAVE_PARAM_ADDRESS_48K;
#else
const sFLASH_PARAM* pFlashParam = (sFLASH_PARAM*)USER_SAVE_PARAM_ADDRESS_32K;
#endif // BOOT_48K





S_SELF_TEST SelfTest = {SELF_TEST_FAILED, SELF_TEST_FAILED, SELF_TEST_FAILED, SELF_TEST_FAILED};

const char *eTestResultStr[SELF_TEST_NB] = {"SELF_TEST_FAILED",
                                            "SELF_TEST_MOUNT_eMMC",
                                            "SELF_TEST_READ_MBR",
                                            "SELF_TEST_FDISK",
                                            "SELF_TEST_FORMAT_eMMC",
                                            "SELF_TEST_RW_eMMC",
                                            "SELF_TEST_COM_INIT_BT",
                                            "SELF_TEST_STACK_INIT_BT",
                                            "SELF_TEST_SUCCESS"};

#ifdef FIRMWARE  // in bootloader, main is in bootloader.c.

#define RS232_RX_FIFO_SIZE         1024
char RS232_Rx_Fifo[RS232_RX_FIFO_SIZE];
int RS232_Rx_Fifo_Head=0;
int RS232_Rx_Fifo_Tail=0;
uint8_t RS232_Rx_Char;

/* UART handler declaration */
UART_HandleTypeDef UartHandle;

/* I2C handler declaration */
I2C_HandleTypeDef I2cHandle;




// Global variables

sDEVICE_SETTINGS Settings = { FTP_INACTIVITY_TO_DEFAULT,        // FTP_InactivityTimeout
                              FTP_AUTHENTICATION_TO_DEFAULT,    // FTP_AuthenticationTimeout
                              921600L,                          // BTBaudRate
                              BT_DISC_TIME_DEFAULT,             // BT_DiscoveryTime
                              0,                                // BT_DisconnectOnVUSB
                              FACE_THRESHOLD_DEFAULT,           // Face_MatchThreshold
                              ADV_INTERVAL_MIN_DEFAULT,         // Min BLE Advertising interval
                              ADV_INTERVAL_MAX_DEFAULT,         // Max BLE Advertising interval
                              SPP_SEND_TIMEOUT_DEFAULT,         // Max SPP send timeout
                              SPP_RECEIVE_TIMEOUT_DEFAULT,      // Max SPP receive timeout  
                              ADV_ENABLED};                     // Is BLE Advertising enabled?  

// ---- FTP management
int FTPAbort = FALSE;
int FTPAborted = FALSE;
int FTPLocked = TRUE;
int FTPActivity = 0;
// ---- Bluetooth management
int BTPaired = FALSE;
int BTDiscMode = FALSE;
int BTActivity = 0;
// ---- Charge management
uint8_t  PMICStatus = 0;
// ---- USB management
int CDCOpened = FALSE;
// ---- SPP management
int SPPOpened = FALSE;
// ---- Battery management
int BatteryLevel = 100;

/* Private function prototypes -----------------------------------------------*/
void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
#ifdef FIRMWARE
#ifndef FIRMWARE_BINARY
#ifndef HYBRID
#pragma required=FlashParam
#endif
#endif
#ifdef BOOT_48K
#pragma required=FirmwareHeader_48K
#ifdef HYBRID
#pragma required=FirmwareHeader_HYBRID
#pragma required=pReset_Handler
#endif
#else
#pragma required=FirmwareHeader_32K
#endif // BOOT_48K
 
    
int main(void)
{ 
  // Initialise RamParam with value saved in FLASH.
  FLASH_If_ReadParam();
  
  // TEST TEST TEST
  //RamParam.Format = 0;

  
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
  HAL_Init();
  
  /* Configure LED1, LED2, LED3 and LED4 */
#ifdef STM32F415xx
  BSP_LED_Init(LED_RED);
  BSP_LED_Init(LED_YELLOW);
#else
#if 1
  BSP_LED_Init(LED_RED);
  BSP_LED_Init(LED_YELLOW);
#else
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED2);
  BSP_LED_Init(LED3);  
  BSP_LED_Init(LED4);
#endif
#endif // STM32F415xx
  
#ifdef STM32F415xx
  // Free pins are configured as Analog to optimize the power consumption.
  Gpio_Init();
#endif // STM32F415xx
  
  
  /*##-1- Configure the I2C peripheral #######################################*/
  I2C_Init();
  
  PMICPreInit();

  
  // Wait end of transmission
  HAL_Delay(10);
  HAL_I2C_DeInit (&I2cHandle);
  
 
  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  
  // Configure USB FS as a CDC device
  CDC_Init();
  
  SERIAL_Init();
  
  // initialize logging system
  slog_init();
  
  //printf("\r\nFirmware version = %d.%d.%d\r\n",VERSION_FIRMWARE_MAJEUR,VERSION_FIRMWARE_MINEUR, VERSION_FIRMWARE_PATCH);
  //printf("Bootloader version = %d.%d\r\n", *pBootloaderMajeur, *pBootloaderMineur);
  slogf(LOG_DEST_CONSOLE, "");
  slogf(LOG_DEST_BOTH, "Firmware version = %d.%d.%d %s",VERSION_FIRMWARE_MAJEUR,VERSION_FIRMWARE_MINEUR, VERSION_FIRMWARE_PATCH, FIRMWARE_PRODUCT );
  slogf(LOG_DEST_BOTH, "Bootloader version = %d.%d", *pBootloaderMajeur, *pBootloaderMineur);
  
  
#ifdef HYBRID
  if(pReset_Handler)
  {
    printf("HYBRID special firmware used to pass from 32k to 48k bootloader.\r\n");
  }
#endif

  /*##-1- Configure the I2C peripheral #######################################*/
  I2C_Init();
  
  
  osThreadDef(PMIC_Thread, PMICThread, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
  osThreadCreate(osThread(PMIC_Thread), NULL);

  /*##-2- Start scheduler ####################################################*/
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  for( ;; );
}
#endif // FIRMWARE
/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the EVAL_COM1 and Loop until the end of transmission */
  HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 0xFFFF); 

  return ch;
}


void I2C_Init(void)
{
  I2cHandle.Instance             = EVAL_I2Cx;

  I2cHandle.Init.ClockSpeed      = BSP_I2C_SPEED; 
  I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  I2cHandle.Init.OwnAddress1     = 0x00;
  I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2cHandle.Init.OwnAddress2     = 0x00;
  I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&I2cHandle) != HAL_OK)
  {
    /* Initialization Error */
    printf("[I2C_Init] initialization error");
    Error_Handler();
  }
}

void SERIAL_Init(void)
{
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
    
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    /* Initialization Error */
    printf("[SERIAL_Init] initialization error");
    Error_Handler(); 
  }
}

char RS232_Get_Rx_Fifo(char* _pData)
{
  if(RS232_Rx_Fifo_Head == RS232_Rx_Fifo_Tail)
  {
    return false;	// fifo empty
  }

  *_pData = RS232_Rx_Fifo[RS232_Rx_Fifo_Tail];
  //RS232_Rx_Fifo[RS232_Rx_Fifo_Tail]=0xFF;
  RS232_Rx_Fifo_Tail++;
  return true;
}

uint8_t RS232_Put_Rx_Fifo(char _Data)
{
  if(RS232_Rx_Fifo_Head+1 != RS232_Rx_Fifo_Tail)
  {
    RS232_Rx_Fifo[RS232_Rx_Fifo_Head]=_Data;
    RS232_Rx_Fifo_Head++;
    return true;
  }
  return false;
}

#endif // FIRMWARE

#ifdef VDD_1_8 

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 108000000
  *            HCLK(Hz)                       = 108000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 26000000
  *            PLL_M                          = 26
  *            PLL_N                          = 432
  *            PLL_P                          = 4
  *            PLL_Q                          = 9
  *            VDD(V)                         = 1.8
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 26;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

#else // VDD = 3.3V

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
#ifdef STM32F415xx
  RCC_OscInitStruct.PLL.PLLM = 26;
#else
  RCC_OscInitStruct.PLL.PLLM = 25;
#endif // STM32F415xx
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);     // a latency of 6WSallow us to use a Vmcu as low as 2.4V

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}
void SystemClockHSI_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 10;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
#endif // VDD_1_8
/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
#ifdef FIRMWARE
void Error_Handler(void)
{
  
  int x  =0;
  int cnt = 0;
  
  /* Go to infinite loop when Hard Fault exception occurs */
  printf("[main.c] ERROR - Error_Handler()\r\n");
  
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
#endif

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  int x=0;
  /* Go to infinite loop when Usage Fault exception occurs */
  printf("ERROR - assert_failed()\r\n");
  printf("%s, line %d\r\n",file,line);
  while (1)
  {
    x=0;
    while(x++<100000);
    BSP_LED_On(PMIC_LED_CHARGE);
    BSP_LED_Off(PMIC_LED_STATUS);
    x=0;
    while(x++<100000)
    BSP_LED_Off(PMIC_LED_CHARGE);
    BSP_LED_On(PMIC_LED_STATUS);
  }
}
#endif

void Gpio_Init(void)
{
#ifdef STM32F415xx
  GPIO_InitTypeDef GPIO_InitStruct;
  
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  
  // Port A: VBAT_DIV, ON_KEY_N_MCU, PMIC_GPIO1, USB HS (ULPI), NFC_OE, and unused pins  
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_3 //| GPIO_PIN_4 | GPIO_PIN_2
                        | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  // Port B : USB HS (ULPI), BOOT1, RESET_N and unused pins  
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6
                      | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11
                      | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  // Port C: USB HS (ULPI), 32.768kHz and unused pins  
  __HAL_RCC_GPIOC_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6 | GPIO_PIN_7
                      | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11
                      | GPIO_PIN_12 | GPIO_PIN_14| GPIO_PIN_15;
  GPIO_InitStruct.Pin |= GPIO_PIN_1 |GPIO_PIN_4 | GPIO_PIN_5;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  
  // Port D : unused pins  
  __HAL_RCC_GPIOD_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 //| GPIO_PIN_5 | GPIO_PIN_6
                      | GPIO_PIN_10 | GPIO_PIN_14;
  GPIO_InitStruct.Pin |= GPIO_PIN_3 | GPIO_PIN_13;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  // Port E : NFC_FD, PMIC_XIRQ and unused pins  
  __HAL_RCC_GPIOE_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15; // GPIO_PIN_13
  GPIO_InitStruct.Pin |= GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  __HAL_RCC_GPIOF_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
  
  __HAL_RCC_GPIOG_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
  
  
  
  
  // Port H :
  __HAL_RCC_GPIOH_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Pin |= GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
  
  // Port I : unused pins  
  __HAL_RCC_GPIOI_CLK_ENABLE();
  GPIO_InitStruct.Pin = GPIO_PIN_0 /*| GPIO_PIN_1*/;
  GPIO_InitStruct.Pin |= GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
  
#if 0  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  
  GPIO_InitStruct.Pin = GPIO_PIN_13;                            // VBAT_EN
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); 

  GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_7;                // VSUP_BT_EN, eMMC_VCCQ_EN
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_4 | GPIO_PIN_7, GPIO_PIN_RESET); 

  GPIO_InitStruct.Pin = GPIO_PIN_9;                             // BT_HCI_OE
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_9, GPIO_PIN_RESET); 

  GPIO_InitStruct.Pin = GPIO_PIN_1;                             // RST_eMMC_N
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_9, GPIO_PIN_RESET); 
  
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
  __HAL_RCC_GPIOD_CLK_DISABLE();
  __HAL_RCC_GPIOE_CLK_DISABLE();
  __HAL_RCC_GPIOI_CLK_DISABLE();
#endif

#endif // STM32F415xx
}

/**
  * @brief  MCU Pre Sleep process.
  * @param  ulExpectedIdleTime: Not used
  * @retval None
  */
void PreSleepProcessing(uint32_t* ulExpectedIdleTime)
{
  /* Called by the kernel before it places the MCU into a sleep mode because
  configPRE_SLEEP_PROCESSING() is #defined to PreSleepProcessing().
  
  NOTE:  Additional actions can be taken here to get the power consumption
  even lower.  For example, peripherals can be turned	off here, and then back
  on again in the post sleep processing function.  For maximum power saving
  ensure all unused pins are in their lowest power state. */

  /* Avoid compiler warnings about the unused parameter. */
  (void) ulExpectedIdleTime;
  
  /* Disable the peripheral clock during Low Power (Sleep) mode.*/
  //__HAL_RCC_GPIOG_CLK_SLEEP_DISABLE();
  SDIO_PowerState_OFF(SDIO);            // 1.5 mA
  //__HAL_RCC_GPIOA_CLK_DISABLE();        // 0.5 mA
  __HAL_RCC_GPIOB_CLK_DISABLE();        // 0.5 mA
  __HAL_RCC_GPIOC_CLK_DISABLE();        // 0.5 mA
  //__HAL_RCC_GPIOD_CLK_DISABLE();      // pas de gain
  if(eMMC_Powered)
  {
    __HAL_RCC_GPIOE_CLK_DISABLE();
  }
}

/**
  * @brief  MCU Post Sleep process.
  * @param  ulExpectedIdleTime: Not used
  * @retval None
  */
void PostSleepProcessing(uint32_t* ulExpectedIdleTime)
{
  /* Called by the kernel when the MCU exits a sleep mode because
     configPOST_SLEEP_PROCESSING is #defined to PostSleepProcessing(). */
  
  /* Avoid compiler warnings about the unused parameter. */
  (void) ulExpectedIdleTime;
  //__HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  //__HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  if(eMMC_Powered)
  {
    SDIO_PowerState_ON(SDIO);
  }
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName )
{
 
  int x=0;
  int cnt = 0;
  
  /* Go to infinite loop when stack overflow occurs */
  printf("Stack OverFlow detected in %s\r\n",pcTaskName);
  BSP_LED_Off(LED_YELLOW);
  BSP_LED_Off(LED_RED);
  
  // Flash error code for about 15 seconds
  while (cnt++ < 40)
  {
    x=0;
    while(x++<2000000);
    BSP_LED_On(LED_YELLOW);
    
    x=0;
    while(x++<2000000);
    BSP_LED_Off(LED_YELLOW);
    
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


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

