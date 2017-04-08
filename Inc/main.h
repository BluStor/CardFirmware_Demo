/**
  ******************************************************************************
  * @file    FatFs/FatFs_uSD_RTOS/Inc/main.h 
  * @author  MCD Application Team
  * @version V1.3.0
  * @date    14-August-2015
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *         http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

   
    
    
    
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32f4xx_hal.h"

#ifdef FIRMWARE
#ifdef HYBRID
#define VERSION_FIRMWARE_MAJEUR           99
#define VERSION_FIRMWARE_MINEUR           99
#else
#define VERSION_FIRMWARE_MAJEUR           7
#define VERSION_FIRMWARE_MINEUR           8
#define VERSION_FIRMWARE_PATCH            0
#define FIRMWARE_PRODUCT                  "DEMO"    
#endif
#else
#ifdef BOOT_48K
#define VERSION_BOOTLOADER_MAJEUR         2             // version majeur 2 or above with a 48k boot.
#define VERSION_BOOTLOADER_MINEUR         2
#else
#define VERSION_BOOTLOADER_MAJEUR         1             // version majeur always 1 with a 32k boot.
#define VERSION_BOOTLOADER_MINEUR         0
#endif
#endif   

#define CONSOLE_SUPPORT         1
//#define FCC_TESTS               1


/* EVAL includes component */
#include "stm324xg_eval.h"

/* FatFs includes component */
#include "ff_gen_drv.h"
#include "sd_diskio.h"	
extern FRESULT eMMC_PowerOn(void);
extern FRESULT eMMC_PowerOff(void);
extern uint8_t eMMC_TurnOn;
extern uint8_t eMMC_TurnOff;
extern uint8_t eMMC_Powered;
extern uint8_t eMMC_Ready;
extern uint8_t GotoStop;
#ifdef FIRMWARE
extern osSemaphoreId PMIC_Semaphore;
#endif
extern DSTATUS disk_deinitialize ( BYTE pdrv);

/* USB includes components */	
#ifdef FIRMWARE
#include "usbd_desc.h"
#include "usbd_cdc.h" 
#include "usbd_cdc_interface.h"
extern void CDC_Init(void);
extern void CDC_Start(void);
extern void CDC_Stop(void);
extern bool CDC_Started;

/* I2C includes components */
extern void I2C_Init(void);


/* Serial port, console*/
extern void SERIAL_Init(void);

/* Bluetooth */
#include "GAPAPI.h"

#endif // FIRMWARE

#include "fatfs.h"

/* FLASH includes component */
#include "flash_if.h"

#ifdef FIRMWARE
// NFaceMatch component
#include "NFaceMatch.h"
#endif // FIRMWARE

/* Self Test -----------------------------------------------------------------*/
typedef enum  _E_TEST_RESULT
{
  SELF_TEST_FAILED = 0,
  SELF_TEST_MOUNT_eMMC,
  SELF_TEST_READ_MBR,
  SELF_TEST_FDISK,
  SELF_TEST_FORMAT_eMMC,
  SELF_TEST_RW_eMMC,
  SELF_TEST_COM_INIT_BT,
  SELF_TEST_STACK_INIT_BT,
  SELF_TEST_SUCCESS,
  SELF_TEST_NB
}E_TEST_RESULT;
extern const char *eTestResultStr[SELF_TEST_NB];


typedef struct _S_SELF_TEST
{
  E_TEST_RESULT eMMC;
  E_TEST_RESULT PMIC;
  E_TEST_RESULT SerialBT;
  E_TEST_RESULT NFC;
}S_SELF_TEST;

extern S_SELF_TEST SelfTest;

/* Flash user area definition *************************************************/   
/* 
   IMPORTANT NOTE:
   ==============
   Memory map when using 32k version:
   the Bootloader use the Sector 0 and 1 in FLASH for a maximum of 32K.
      0x0800 0000 - 0x0800 7FFF
   A Parameter block is placed in the sector 2 and can contain up t0 16k.
      0x0800 8000 - 0x0800 BFFF
   The firmware use the Sector 3 to 11 for a maximum of 976k.
      0x0800 C000 - 0x080F FFFF
   The firmware have some particularity:
      at 0x0800 C000 - 0x0800 C1FF we have an header that identify the firmware.
      this header is used by the bootloader to validate than an valid firmware
      is present in FLASH.
      at 0x0800 C200 we have the interrupt vector table.
   Actually, there is a CRC16 that is calculated only in a part of the FLASH in
   order to limit the size of the firmware file. If firmware go beyound this
   side the zone on which the CRC is calculated must be adjusted.
      0x0800 C200

   Memory map when using 48k version:
   the Bootloader use the Sector 0, 1 and 2 in FLASH for a maximum of 48K.
      0x0800 0000 - 0x0800 BFFF
   A Parameter block is placed in the sector 3 and can contain up t0 16k.
      0x0800 C000 - 0x0800 FFFF
   The firmware use the Sector 4 to 11 for a maximum of 960k.
      0x0801 0000 - 0x080F FFFF
   The firmware have some particularity:
      at 0x0801 0000 - 0x0801 01FF we have an header that identify the firmware.
      this header is used by the bootloader to validate than an valid firmware
      is present in FLASH.
      at 0x0801 0200 we have the interrupt vector table.
   Actually, there is a CRC16 that is calculated only in a part of the FLASH in
   order to limit the size of the firmware file. If firmware go beyound this
   side the zone on which the CRC is calculated must be adjusted.
      0x0800 C200

   */

#ifdef BOOT_48K
#define USER_BOOTLOADER_FIRST_PAGE_ADDRESS_48K  0x08000000
#define USER_BOOTLOADER_END_ADDRESS             0x0800BFFF
#define USER_FIRMWARE_FIRST_PAGE_ADDRESS_48K    0x08010000 /* Sector 4 */
#define USER_FIRMWARE_LAST_PAGE_ADDRESS         0x080E0000 /* Sector 11 */
#define USER_FIRMWARE_END_ADDRESS               0x080FFFFF     
#define USER_SAVE_PARAM_ADDRESS_48K             0x0800C000 /* Sector 3 */
#define FIRMWARE_SIGNATURE_48K                  "GC010-Firmware2"
#define BOOTLOADER_SIGNATURE                    "GC010-Boot"
#else
#define USER_FIRMWARE_FIRST_PAGE_ADDRESS_32K    0x0800C000 /* Sector 3 */
#define USER_FIRMWARE_LAST_PAGE_ADDRESS         0x080E0000 /* Sector 11 */
#define USER_FIRMWARE_END_ADDRESS               0x080FFFFF     
#define USER_SAVE_PARAM_ADDRESS_32K             0x08008000 /* Sector 2 */
#endif // BOOT_48K

#define FIRMWARE_SIGNATURE_32K                  "GC010-Firmware"

#define VERSION_BOOTLOADER_MAJEUR_ADDRESS       (0x08008000-2)     
#define VERSION_BOOTLOADER_MINEUR_ADDRESS       (0x08008000-1)
     
     
#define RTC_CLOCK_SOURCE_LSI
//#define RTC_CLOCK_SOURCE_LSE
     
#ifdef RTC_CLOCK_SOURCE_LSI
#define RTC_ASYNCH_PREDIV  0x7F
#define RTC_SYNCH_PREDIV   0x0130
#endif

#ifdef RTC_CLOCK_SOURCE_LSE
  #define RTC_ASYNCH_PREDIV  0x7F
  #define RTC_SYNCH_PREDIV   0x00FF
#endif


#define min(a,b)        (a>b?b:a)
#define max(a,b)        (a>b?a:b)

#pragma pack (1)
typedef struct _S_FIRMWARE_HEADER
{
  char Signature[16];
  uint32_t StartAdd;
  uint32_t EndAdd;
  uint8_t ver_majeur;
  uint8_t ver_mineur;
} S_FIRMWARE_HEADER;
#pragma pack()

#pragma pack (1)
typedef struct _S_FIRMWARE_CRC
{
  uint8_t reserved[0x200-2];
#ifdef BOOTLOADER
  unsigned short Crc16;
#endif
} S_FIRMWARE_CRC;
#pragma pack ()

#pragma pack (1)
typedef union _U_FIRMWARE_HEADER
{
  S_FIRMWARE_HEADER Header;
  S_FIRMWARE_CRC Crc;
} U_FIRMWARE_HEADER;
#pragma pack ()

typedef enum _BOOT_CMD
{
  CMD_NONE = 1,
  CMD_UPDATE_FIRMWARE = 2,
} eBOOT_CMD;

typedef struct _FLASH_PARAM
{
  eBOOT_CMD BootCmd;
  uint8_t Format;
  uint8_t DoSelfTest;
#ifdef BOOTLOADER
  uint8_t reserved[16*1024-(sizeof(eBOOT_CMD) +2)];
#endif
} sFLASH_PARAM;

#define FORMAT_FIRST_PARTITION   0x01   // file vault partition
#define FORMAT_SECOND_PARTITION  0x02   // licence partition


//#ifdef BOOT_48K
//extern const sFLASH_PARAM FlashParam @ USER_SAVE_PARAM_ADDRESS_48K;
//#else
//extern const sFLASH_PARAM FlashParam @ USER_SAVE_PARAM_ADDRESS_32K;
//#endif // BOOT_48K

#define BT_DISC_TIME_DEFAULT            300     // seconds
#define BT_DISC_TIME_MIN                30      // seconds
#define BT_DISC_TIME_MAX                600     // seconds

typedef struct _DEVICE_SETTINGS
{
  uint32_t FTP_InactivityTimeout;       // in s.
  uint32_t FTP_AuthenticationTimeout;   // in s.
  uint32_t BTBaudRate;
  uint32_t BT_DiscoveryTime;            // in s.
  uint32_t BT_DisconnectOnVUSB;          // 0-> FALSE, 1-> TRUE
  uint32_t Face_MatchThreshold;    
  uint32_t Advertising_Interval_Min;
  uint32_t Advertising_Interval_Max;
  uint32_t SPP_Send_Timeout;
  uint32_t SPP_Receive_Timeout;
  uint32_t Advertising_Enabled;
} sDEVICE_SETTINGS;

extern sDEVICE_SETTINGS Settings;


#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define DEVICE_PATH                     "/device"
#define BT_KEY_FILENAME                 "/device/bt.key"
#define BT_LE_KEY_FILENAME              "/device/btle.key"
#define FIRMWARE_FILENAME               "/device/firmware"
#define VAULT_DATA_PATH                 "/data"
#define AUTH_PATH                       "/auth"
#define PASSWORD_VAULT_PATH             "/passwordvault"
#define AUTH_FACE_PATH                  "/auth/face"
#define AUTH_RECOVERY_CODE_PATH         "/auth/code"
#define LICENSE_PATH                    "1:/license"
#define RECOVERY_CODE_MIN_SIZE          6
#define RECOVERY_CODE_MAX_SIZE          1024
#define FTP_ROOT_PATH                   VAULT_DATA_PATH
#define FTP_DEVICE_PATH                 "/device"
#define FTP_DEVICE_BATTERY              "/device/battery"
#define FTP_DEVICE_SELFTEST             "/device/selftest"
#define FTP_DEVICE_PAIR                 "/device/pair"
#define FTP_DEVICE_FIRMWARE             "/device/firmware"
#define FTP_DEVICE_BOOTLOADER           "/device/bootld"
#define FTP_DEVICE_SETTINGS             "/device/settings"
#define FTP_DEVICE_RESET                "/device/reset"
#define FTP_VAULT_PATH                  "/apps/vault/data"
#define FTP_AUTH_PATH                   "/auth"
#define FTP_PASSWORD_VAULT_PATH         "/passwordvault"
#define FTP_AUTH_ENROLL_FACE_PATH       "/auth/enroll/face"
#define FTP_AUTH_ENROLL_CODE_PATH       "/auth/enroll/code"
#define FTP_AUTH_SIGNIN_PATH            "/auth/signin"
#define FTP_AUTH_SIGNIN_FACE_PATH       "/auth/signin/face"
#define FTP_AUTH_SIGNIN_CODE_PATH       "/auth/signin/code"
#define FTP_AUTH_SIGNOUT_PATH           "/auth/signout"
#define FTP_LICENSE_PATH                "/license"
#define TEMP_FILE                       "/file.tmp"
#define TEST_10K_PATH                   "/data/10k.txt"

#define FTP_DEVICE_RESET "/device/reset"
#define FTP_DEVICE_FACTORYRESET "/device/factoryreset"
#define FTP_DEVICE_POWEROFF "/device/poweroff"


#define ADV_INTERVAL_MIN_DEFAULT       2000     // min ble advertising interval in ms (2000)
#define ADV_INTERVAL_MAX_DEFAULT       2500     // max ble advertising interval in ms (2500)
#define ADV_INTERVAL_MIN_RANGE_MIN     100
#define ADV_INTERVAL_MIN_RANGE_MAX     10000
#define ADV_INTERVAL_MAX_RANGE_MIN     100
#define ADV_INTERVAL_MAX_RANGE_MAX     10000
#define ADV_ENABLED                    1        // 0 = no, 1 = yes

#define SPP_SEND_TIMEOUT_DEFAULT       2500
#define SPP_RECEIVE_TIMEOUT_DEFAULT    2500
#define SPP_TIMEOUT_MIN                100
#define SPP_TIMEOUT_MAX                10000

#define FACE_THRESHOLD_DEFAULT         48      // For NeuroTechnology face match library
#define FACE_THRESHOLD_MIN             28      // For NeuroTechnology face match library
#define FACE_THRESHOLD_MAX             148      // For NeuroTechnology face match library

#define DEV_SETTINGS_MAX_TOKEN          20      // For JSMN library

#define SELFTEST_RW_PATH                "/device/SelfTest.txt"
#define SELFTEST_eMMC_WRITE_BUFFER      "BluStor eMMC SelfTest Data."

/* USB handler declaration */
#ifdef FIRMWARE
extern USBD_HandleTypeDef  USBD_Device;
#endif // FIRMWARE

#ifdef BOOTLOADER
// fix bootloader version in flash so firmware can read it.
extern const uint8_t BootloaderMajeur;
extern const uint8_t BootloaderMineur;
#else
// pointeur used by firmware to read bootloader version.
extern const uint8_t* pBootloaderMajeur;
extern const uint8_t* pBootloaderMineur;
#endif    


extern sFLASH_PARAM RamParam;
extern const sFLASH_PARAM* pFlashParam;

//#ifdef FIRMWARE
//#ifdef FIRMWARE_BINARY
// Firmware binary is for update purpose... since we didn't want to erase saved
// parameters in flash we acces to theh via a pointer.
//extern const sFLASH_PARAM* pFlashParam;
//#else
// Default parameters that will be programmed in production.
//extern const sFLASH_PARAM FlashParam;
//#endif // FIRMWARE_BINARY
//#else  // BOOTLOADER
//extern const sFLASH_PARAM* pFlashParam;
//#endif



extern void SystemClock_Config(void);
extern void SystemClockHSI_Config(void);
extern void Gpio_Init(void);
extern void Error_Handler(void);


extern int FTPAbort;
extern int FTPAborted;
extern int FTPActivity;
extern int BTActivity;
extern int FTPLocked;
extern int BTPaired;
extern int BTForcePair;
extern int BTDiscMode;
extern uint8_t  PMICStatus;
extern int CDCOpened;
extern int SPPOpened;
extern int BatteryLevel;


#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
