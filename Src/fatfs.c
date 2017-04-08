/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
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

#include "main.h"
#include "pmic.h"
#include "slog.h"
    
// Those value were originally defined in ff.c.
#define MBR_Table			446		/* MBR: Partition table offset (2) */
#define	SZ_PTE				16		/* MBR: Size of a partition table entry */    

char SD_Path0[4] = {0,0,0,0};
char SD_Path1[4] = {0,0,0,0};
FATFS SDFatFs[2];        /* File system object for SD card logical drive */

PARTITION VolToPart[] = {{0,1},         /* Logical drive 0 ==> Physical drive 0, 1st partition */
                         {0,2}};        /* Logical drive 1 ==> Physical drive 0, 2nd partition */

//DWORD plist[] = {50, 50, 0, 0};  /* Divide drive into two partitions of equal size*/
//DWORD plist[] = {3818304, 36288, 0, 0};  /* Divide drive into two partitions of equal size*/
//DWORD plist[] = {3850560, 4032, 0, 0};  /* Divide drive into two partitions of equal size*/
DWORD plist[] = {-1,-1,0,0};

uint8_t SD_SecondPartition = FALSE;

uint8_t eMMC_Powered = FALSE;
uint8_t eMMC_Ready = FALSE;


uint8_t eMMC_TurnOn = FALSE;
uint8_t eMMC_TurnOff = FALSE;

extern SD_HandleTypeDef uSdHandle;
FRESULT eMMC_PowerOff(void)
{
  if(eMMC_Powered)
  {
    disk_deinitialize(0);
    eMMC_VCCQ_Disable();
    PMIC_2_8V(FALSE);
    eMMC_Powered = FALSE;
    printf("eMMC_PowerOff()\r\n");
  }
  
  return FR_OK;
}

FRESULT eMMC_PowerOn(void)
{
  FRESULT res = FR_OK;
  
  if(eMMC_Powered == FALSE)
  {
    eMMC_VCCQ_Enable();
    PMIC_2_8V(TRUE);
    if(disk_initialize(0)==RES_OK)
    {
      eMMC_Powered = TRUE;
      printf("eMMC_PowerOn()\r\n");
    }else
    {
      eMMC_VCCQ_Disable();
      PMIC_2_8V(FALSE);
      res = FR_DISK_ERR;
    }
  }
  
  return res;
}


FRESULT FormateMMC(void)
{
  FRESULT res;
  
  res = f_mkfs((TCHAR const*)SD_Path0, 0, 0);
  
  return (res);
}

FRESULT FormatBlustorMMC(void)
{
  FRESULT res;

  // Prepare disk tree structure
  do {
    res = f_mkdir(DEVICE_PATH);
    if ((res != FR_OK) && (res != FR_EXIST)) {
      printf("Failed to create %s\r\n", DEVICE_PATH);
      break;
    }
    res = f_mkdir(VAULT_DATA_PATH);
    if ((res != FR_OK) && (res != FR_EXIST)) {
      printf("Failed to create %s\r\n", VAULT_DATA_PATH);
      break;
    }
    res = f_mkdir(PASSWORD_VAULT_PATH);
    if ((res != FR_OK) && (res != FR_EXIST)) {
      printf("Failed to create %s\r\n", PASSWORD_VAULT_PATH);
      break;
    }
    res = f_mkdir(AUTH_PATH);
    if ((res != FR_OK) && (res != FR_EXIST)) {
      printf("Failed to create %s\r\n", AUTH_PATH);
      break;
    }
    res = f_mkdir(AUTH_FACE_PATH);
    if ((res != FR_OK) && (res != FR_EXIST)) {
      printf("Failed to create %s\r\n", AUTH_FACE_PATH);
      break;
    }
    res = f_mkdir(AUTH_RECOVERY_CODE_PATH);
    if ((res != FR_OK) && (res != FR_EXIST)) {
      printf("Failed to create %s\r\n", AUTH_RECOVERY_CODE_PATH);
      break;
    }
    if(SD_SecondPartition)
    {
      res = f_mkdir(LICENSE_PATH);
      if ((res != FR_OK) && (res != FR_EXIST)) {
        printf("Failed to create %s\r\n", LICENSE_PATH);
        break;
      } 
    }
    res = FR_OK;
  } while (0);
  
  eMMC_Ready = TRUE;
  
  return (res);
}



// #define EMMC_WRITE_TEST      1
#ifdef EMMC_WRITE_TEST
#define EMMC_WRITE_TEST_SIZE    (100 * 1024 * 1024)
#define EMMC_WRITE_TEST_PRINT   (1024 * 256) // must be power of 2
uint8_t TmpBuff[8192];
void Test_eMMC(void)
{
  uint8_t retSD;        /* Return value for SD */
  FIL MyFile;           /* File object */
  UINT byteswritten;
  uint32_t y;
  
  printf("\r\nWrite file #1.\r\n");
  retSD = f_open(&MyFile, "/data/TstFile1.TXT", FA_CREATE_ALWAYS | FA_WRITE);
  if (retSD == FR_OK) {
   for(y=0;y<EMMC_WRITE_TEST_SIZE;y+=sizeof(TmpBuff))
   {
     if ((y & (EMMC_WRITE_TEST_PRINT - 1)) == 0) {
      printf("W:%dKB           \r", y >> 10);
     }
     retSD = f_write(&MyFile, TmpBuff, sizeof(TmpBuff),&byteswritten); 
     if(byteswritten != sizeof(TmpBuff))
     {
       printf("\nError writing to file!\r\n");
       break;
     }
   }
   printf("\n");
   retSD = f_close(&MyFile);
  } else {
   printf("Error opening file\r\n");
  }
}
#endif // EMMC_WRITE_TEST

BYTE SD_WorkingBuf[_MAX_SS];

uint8_t MX_FATFS_Init(void) 
{
  uint8_t retSD;        /* Return value for SD */
  FIL MyFile;           /* File object */
  const uint8_t WriteBuff[] = SELFTEST_eMMC_WRITE_BUFFER;
  uint8_t ReadBuff[sizeof(SELFTEST_eMMC_WRITE_BUFFER)];
  uint32_t BytesCount;
  int x;
  
  SelfTest.eMMC = SELF_TEST_MOUNT_eMMC;
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SD_Path0);    
  if(retSD != FR_OK)
  {
    slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] FATFS_LinkDriver() failed SP_Path0.");
    goto exit;
  }

  retSD = FATFS_LinkDriver(&SD_Driver, SD_Path1);    
  if(retSD != FR_OK)
  {
    slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] FATFS_LinkDriver() failed SP_Path1.");
    goto exit;
  }
  
  // check if second partion is present... read MBR.
  if(disk_initialize(0)==RES_OK)
  {
    eMMC_Powered = TRUE;
  }
  
  if(disk_read(0, SD_WorkingBuf, 0, 1) != RES_OK)
  {
    SelfTest.eMMC = SELF_TEST_READ_MBR;
    slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] failed to read MBR.");
    goto exit;
  }
  
  if(SD_WorkingBuf[MBR_Table + SZ_PTE + 4] != 0)
  {
    SD_SecondPartition = TRUE;
  }else
  {
    printf("Second partition not present.\r\n");
  }

  // Only done one first boot or factory reset
  if(RamParam.Format)      
  {
    
    // Repation and re-format second partition on initial boot
    // and factory reset
    
    // if second partition not present, partition drive 0.
    //if(SD_SecondPartition == FALSE)
    //{
      retSD = f_fdisk(0, plist, SD_WorkingBuf);
      if(retSD != FR_OK)
      {
        SelfTest.eMMC = SELF_TEST_FDISK;
        slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] f_fdisk failed on second partition.");
        goto exit;
      }
      
      slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] f_fdisk completed for second partition.");
      RamParam.Format |= FORMAT_SECOND_PARTITION;
      SD_SecondPartition = TRUE;
    //}
    
  }

  /*##-2- Register the file system object to the FatFs module ##############*/
  retSD = f_mount(&SDFatFs[0], (TCHAR const*)SD_Path0, 1);
  if((retSD != FR_OK) && (retSD != FR_NO_FILESYSTEM))
  {
    slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] f_mount failed on partition 0.");
    goto exit;
  }
  
  if(SD_SecondPartition)
  {
    retSD = f_mount(&SDFatFs[1], (TCHAR const*)SD_Path1, 1);
    if((retSD != FR_OK) && (retSD != FR_NO_FILESYSTEM))
    {
      slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] f_mount failed on parition 1.");
      goto exit;
    }
  } else {
      slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] partition 1 missing.");
      goto exit;
  }
  
  /*##-3- Create a FAT file system (format) on the logical drive #########*/
  /* WARNING: Formatting the uSD card will delete all content on the device */
  if(RamParam.Format)      // Only done one first boot for production purpose.
  {
    if(RamParam.Format & FORMAT_FIRST_PARTITION)
    {
      retSD = f_mkfs((TCHAR const*)SD_Path0, 0, 0);
      if(retSD != FR_OK)
      {
        SelfTest.eMMC = SELF_TEST_FORMAT_eMMC;
        slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] f_mkfs failed on parition 0.");
        goto exit;
      } else {
        slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] partition 0 successfully formated.");
      }
    }
    
    if(RamParam.Format & FORMAT_SECOND_PARTITION)
    {
      retSD = f_mkfs((TCHAR const*)SD_Path1, 0, 0);
      if(retSD != FR_OK)
      {
        SelfTest.eMMC = SELF_TEST_FORMAT_eMMC;
        slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] f_mkfs failed on partition 1.");
        goto exit;
      } else {
        slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] partition 1 successfully formated.");
      }
    }
    
    RamParam.Format=0;
    FLASH_If_SaveParam();
    
  }
    
  // Prepare BluStor Disk structure
  retSD = FormatBlustorMMC();
  if(retSD != FR_OK)
  {
    SelfTest.eMMC = SELF_TEST_FORMAT_eMMC;
    slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] FormatBlustorMMC() failed.");
    goto exit;
  }
  
  // do a read write test on the eMMC.
  SelfTest.eMMC = SELF_TEST_RW_eMMC;
  retSD = f_open(&MyFile, SELFTEST_RW_PATH, FA_CREATE_ALWAYS | FA_WRITE);
  if(retSD != FR_OK)
  {
    slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] f_open failed.");
    goto exit;
  }
  
  retSD = f_write(&MyFile, WriteBuff, sizeof(WriteBuff), (void *)&BytesCount);
  f_close(&MyFile);
  if(retSD != FR_OK)
  {
    slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] f_write failed.");
    goto exit;
  }
  
  retSD = f_open(&MyFile, SELFTEST_RW_PATH, FA_READ);
  if(retSD != FR_OK)
  {
    slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] f_open failed.");
    goto exit;
  }
  
  retSD = f_read(&MyFile, ReadBuff, sizeof(ReadBuff), (void *)&BytesCount);
  f_close(&MyFile);
  if(retSD != FR_OK)
  {
    slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] f_read failed.");
    goto exit;
  }
  
  for(x=0;x<sizeof(WriteBuff);x++)
  {
    if(ReadBuff[x]!=WriteBuff[x])
    {
      slogf(LOG_DEST_BOTH, "[MX_FATFS_Init] read/write comparison failed.");
      goto exit;
    }
  }

#ifdef EMMC_WRITE_TEST
  Test_eMMC();
#endif  
  
  SelfTest.eMMC = SELF_TEST_SUCCESS;
exit:
  slogf(LOG_DEST_BOTH, "FATFS init: %s!",retSD == FR_OK?"SUCCESS":"FAIL");
  
  if (retSD == FR_OK) eMMC_Ready = TRUE;
  
  /* USER CODE END Init */
  return(retSD);
}

/**
  * @brief  Gets Time from RTC 
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
