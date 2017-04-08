/*****< pmic.c >************************************************************/
/*                                                                            */
/*  PMIC - Thread and support for PMIC.                                       */
/*                                                                            */
/*  Author:                                                                   */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/25/15                 Initial creation.                               */
/******************************************************************************/
#include "pmic.h"
#include "gapapi.h"
#include "slog.h"

#ifdef firmware
#include "SPPTask.h"
#endif

extern int (*pWriteABuffer)(const char*, DWORD);
extern int BT_WriteABuffer(const char * lpBuf, DWORD dwToWrite);
extern int CDC_WriteABuffer(const char * lpBuf, DWORD dwToWrite);
extern void FtpServerReset(void);
extern uint8_t CDC_NeedReset;
extern void CDC_Reset(void);
extern void BT_EnableCTSInt(Boolean_t Suspend);

extern void AdvertiseLockStatus(int locked);
extern FRESULT FindValidTemplate(void);

/* Thread declaration */
osThreadId FTPTaskHandle;
osThreadId SPPTaskHandle;

/* BLE timeout variables */
int BluetoothStackID;
uint32_t BLEConnectTime;
BD_ADDR_t BD_ADDR;

// used to track when we are turning from sleep mode
// so we can check if user is holding down the button
uint8_t returnFromSleep = FALSE;   

/* Local variables */
#ifdef FIRMWARE

static uint16_t PMICLedCtrl[PMIC_LED_NUMBER];
static Led_TypeDef  PMICBSPLed[PMIC_LED_NUMBER] = {PMIC_LED_CHARGE, PMIC_LED_STATUS};
static int CDCOpenedTrk = FALSE;
static uint16_t exti_flag = 0;

ADC_HandleTypeDef AdcHandle;
RTC_HandleTypeDef RtcHandle;

osSemaphoreId PMIC_Semaphore = NULL;
osSemaphoreId RTC_Semaphore = NULL;
int RTC_Ready = FALSE;

uint32_t TimeFromLastConnect;

int BatteryPercent;
int LastBatteryPercent = 0;
int PMIC_Is_Charging = FALSE;

#endif // FIRMWARE

uint8_t GotoStop = FALSE;

/* Functions */
#ifdef FIRMWARE
static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
	if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
			strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
		return 0;
	}
	return -1;
}
#endif // FIRMWARE

#ifdef FIRMWARE
HAL_StatusTypeDef RTC_InitTime(void)
{
  HAL_StatusTypeDef stat;
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  int32_t sem;
  
  RTC_AlarmTypeDef sAlarm;
  RTC_TimeTypeDef sAlarmTime;
  long sec, hr, min, t;
  
  sem = osSemaphoreWait(RTC_Semaphore , 100);
  if (sem != osOK) {
    return HAL_TIMEOUT;
  }
  
  slogf(LOG_DEST_BOTH, "[RTC_InitTime] real-time clock reset");

  sDate.Year = 0x00;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x01;
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  
  stat = HAL_RTC_SetDate(&RtcHandle, &sDate, RTC_FORMAT_BCD);
  if (stat == HAL_OK) {
    sTime.Hours = 0x00;
    sTime.Minutes = 0x00;
    sTime.Seconds = 0x00;
    sTime.TimeFormat = RTC_HOURFORMAT12_AM;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    
    stat = HAL_RTC_SetTime(&RtcHandle, &sTime, RTC_FORMAT_BCD);
    if (stat != HAL_OK) {
      slogf(LOG_DEST_BOTH, "[RTC_InitTime] HAL_RTC_SetTime error");
      RTC_Ready = FALSE;
    } else {
       RTC_Ready = TRUE;
    }
  } else {
    slogf(LOG_DEST_BOTH, "[RTC_InitTime] HAL_RTC_SetDate error");
    RTC_Ready = FALSE;
  }
  
  // Clear the EXTI Line 17 Pending bit (Connected internally to RTC Alarm) 
  EXTI->PR |= 0x20000;
  
  // Enable alarm if authentication timeout is enabled and card is unlocked and card has valid templates
  if ((Settings.FTP_AuthenticationTimeout > 0) && (!FTPLocked)) {
  
          sec = Settings.FTP_AuthenticationTimeout;
          hr = sec/3600;
          t = sec%3600;
          min = t/60;
          sec = t%60;
          
          sAlarmTime.Hours = (uint8_t)hr;
          sAlarmTime.Minutes = (uint8_t)min;
          sAlarmTime.Seconds = (uint8_t)sec;

          sAlarmTime.SubSeconds = 0x00;
          sAlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
          sAlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
          sAlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
          
          sAlarm.AlarmTime = sAlarmTime;
          //sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES | RTC_ALARMMASK_SECONDS;
          sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY & RTC_ALARMMASK_HOURS & RTC_ALARMMASK_MINUTES & RTC_ALARMMASK_SECONDS;
          //sAlarm.AlarmMask = RTC_ALARMMASK_MINUTES;
          sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
          sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
          sAlarm.AlarmDateWeekDay = 0x01;
          sAlarm.Alarm = RTC_CR_ALRAE;
            
          if (HAL_RTC_SetAlarm_IT(&RtcHandle, &sAlarm, RTC_FORMAT_BCD) != HAL_OK) {
            slogf(LOG_DEST_BOTH, "[RTC_InitTime] HAL_RTC_SetAlarm_IT error");
          }
          
          // RTC WakeUpTimer Interrupt Configuration: EXTI configuration 
          __HAL_RTC_EXTI_ENABLE_IT(RTC_EXTI_LINE_WAKEUPTIMER_EVENT); 
          EXTI->RTSR |= RTC_EXTI_LINE_WAKEUPTIMER_EVENT;
  
  }
  
  osSemaphoreRelease(RTC_Semaphore);
  
  return (stat);
}

void get_date(RTC_DateTypeDef * sDate, RTC_TimeTypeDef * sTime)
{
  HAL_StatusTypeDef stat;
  int32_t sem;
  
  // Set initial values to zero in case RTC is not ready
  sDate->Year = 0x00;
  sDate->Month = 0x00;
  sDate->Date = 0x00;
  sDate->WeekDay = 0x00;

  sTime->Hours = 0x00;
  sTime->Minutes = 0x00;
  sTime->Seconds = 0x00;
  sTime->SubSeconds = 0x00;
  sTime->SecondFraction = 0x00;
  
  if (!RTC_Ready) return;
      
  sem = osSemaphoreWait(RTC_Semaphore , 100);
  if (sem != osOK) {
    return;
  }
    
  stat = HAL_RTC_GetTime(&RtcHandle, sTime, RTC_FORMAT_BIN);
  if (stat == HAL_OK) {
    stat = HAL_RTC_GetDate(&RtcHandle, sDate, RTC_FORMAT_BIN);
    if (stat == HAL_OK) {
      
      // We now have number of elapsed seconds since RTC was last reset
       
    } else {
      slogf(LOG_DEST_BOTH, "[slog_get_date] error: %d", stat);
    }
  } else {
    slogf(LOG_DEST_BOTH, "[slog_get_date] error: %d\r\n", stat);
  }
    
  osSemaphoreRelease(RTC_Semaphore);
  
}

HAL_StatusTypeDef RTC_GetElapsedTime(uint32_t * seconds)
{
  HAL_StatusTypeDef stat;
  RTC_TimeTypeDef sTime;
  RTC_DateTypeDef sDate;
  uint32_t value = -1;
  int32_t sem;
  
      
  sem = osSemaphoreWait(RTC_Semaphore , 100);
  if (sem != osOK) {
    return HAL_TIMEOUT;
  }
  
  stat = HAL_RTC_GetTime(&RtcHandle, &sTime, RTC_FORMAT_BIN);
  if (stat == HAL_OK) {
    stat = HAL_RTC_GetDate(&RtcHandle, &sDate, RTC_FORMAT_BIN);
    if (stat == HAL_OK) {
      if (sDate.Month == 1) {
        value = ( ((sDate.Date-1) * 86400 ) + (sTime.Hours * 3600) + (sTime.Minutes * 60) + sTime.Seconds);
        *seconds = value;
      } else {
        
        // Can't correctly calculate the elapsed time after a month
        slogf(LOG_DEST_BOTH, "[RTC_GetElapsedTime] RTC_GetElapsedTime - more than a month has elpased, resetting clock");
        RTC_InitTime();
        *seconds = 0;
      }
    } else {
      slogf(LOG_DEST_BOTH, "[RTC_GetElapsedTime] HAL_RTC_GetDate error: %d", stat);
    }
  } else {
    slogf(LOG_DEST_BOTH, "[RTC_GetElapsedTime] HAL_RTC_GetTime error: %d", stat);
  }
  
  osSemaphoreRelease(RTC_Semaphore);
  
  return (stat);
}

void PMICGetDeviceSettings(void)
{
  jsmn_parser jp;
  jsmntok_t jt[DEV_SETTINGS_MAX_TOKEN];
  int res;
  int i;
  char buf[256];
  char param[32];
  unsigned long value;
  FIL fp;
  UINT read;
  
  memset(buf, 0, sizeof(buf));
  
  do {
    res = f_open(&fp, FTP_DEVICE_SETTINGS, FA_READ | FA_OPEN_EXISTING);
    if (res == FR_OK) {
      res = f_read(&fp, (void *) buf, sizeof(buf), &read);
      f_close(&fp);
    } else {
      slogf(LOG_DEST_BOTH, "[PMICGetDeviceSettings] No user settings found, using default device settings");
      break;
    }
    
    jsmn_init(&jp);
    
    res = jsmn_parse(&jp, buf, read, jt, sizeof(jt)/sizeof(jt[0]));
    if (res < 0) {
      slogf(LOG_DEST_BOTH, "[PMICGetDeviceSettings] JSMN failed to parse buf: %d", res);
      break;
    }
    
    if (res < 1 || jt[0].type != JSMN_OBJECT) {
      slogf(LOG_DEST_BOTH, "[PMICGetDeviceSettings] JSMN object expected");
      break;
    }
    
    for (i=1; i<res; i++) {
      if (jsoneq(buf, &jt[i], "inactivity_to") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = (int) strtoul(param, NULL, 0);
          if (value < FTP_INACTIVITY_TO_MIN) {
            value = FTP_INACTIVITY_TO_MIN;
          } else if (value > FTP_INACTIVITY_TO_MAX) {
            value = FTP_INACTIVITY_TO_MAX;
          }
          Settings.FTP_InactivityTimeout = (uint32_t) value;
        }
        i++;
      } else if (jsoneq(buf, &jt[i], "bt_baud") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = strtoul(param, NULL, 0);
          if ((value == 921600) || (value == 1843200)
              || (value == 2764800) || (value == 3686400)){
            Settings.BTBaudRate = value;
          }
        }
        i++;
      } else if (jsoneq(buf, &jt[i], "disc_time") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = strtoul(param, NULL, 0);
          if (value < BT_DISC_TIME_MIN) {
            value = BT_DISC_TIME_MIN;
          } else if (value > BT_DISC_TIME_MAX) {
            value = BT_DISC_TIME_MAX;
          }
          Settings.BT_DiscoveryTime = value;
        }
        i++;
      } else if (jsoneq(buf, &jt[i], "face_threshold") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = strtoul(param, NULL, 0);
          if (value < FACE_THRESHOLD_MIN) {
            value = FACE_THRESHOLD_MIN;
          } else if (value > FACE_THRESHOLD_MAX) {
            value = FACE_THRESHOLD_MAX;
          }
          Settings.Face_MatchThreshold = value;
        }
        i++;
      } else if (jsoneq(buf, &jt[i], "authentication_to") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = strtoul(param, NULL, 0);
          if (value < FTP_AUTHENTICATION_TO_MIN) {
            value = FTP_AUTHENTICATION_TO_MIN;
          } else if (value > FTP_AUTHENTICATION_TO_MAX) {
            value = FTP_AUTHENTICATION_TO_MAX;
          }
          Settings.FTP_AuthenticationTimeout = value;
        }
        i++;
      } else if (jsoneq(buf, &jt[i], "disconnect_on_vusb") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = strtoul(param, NULL, 0);
          if (value != 0) {
            Settings.BT_DisconnectOnVUSB = 1;
          } else {
            Settings.BT_DisconnectOnVUSB = 0;
          }
        }
        i++;
      } else if (jsoneq(buf, &jt[i], "advertise_interval_min") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = strtoul(param, NULL, 0);
          if (value < ADV_INTERVAL_MIN_RANGE_MIN) {
            value = ADV_INTERVAL_MIN_RANGE_MIN;
          } else if (value > ADV_INTERVAL_MIN_RANGE_MAX) {
            value = ADV_INTERVAL_MIN_RANGE_MAX;
          }
          Settings.Advertising_Interval_Min = value;
        }
        i++;
      } else if (jsoneq(buf, &jt[i], "advertise_interval_max") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = strtoul(param, NULL, 0);
          if (value < ADV_INTERVAL_MAX_RANGE_MIN) {
            value = ADV_INTERVAL_MAX_RANGE_MIN;
          } else if (value > ADV_INTERVAL_MAX_RANGE_MAX) {
            value = ADV_INTERVAL_MAX_RANGE_MAX;
          }
          Settings.Advertising_Interval_Max = value;
        }
        i++;
      } else if (jsoneq(buf, &jt[i], "spp_send_timeout") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = strtoul(param, NULL, 0);
          if (value < SPP_TIMEOUT_MIN) {
            value = SPP_TIMEOUT_MIN;
          } else if (value > SPP_TIMEOUT_MAX) {
            value = SPP_TIMEOUT_MAX;
          }
          Settings.SPP_Send_Timeout = value;
        }
        i++;
      } else if (jsoneq(buf, &jt[i], "spp_receive_timeout") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = strtoul(param, NULL, 0);
          if (value < SPP_TIMEOUT_MIN) {
            value = SPP_TIMEOUT_MIN;
          } else if (value > SPP_TIMEOUT_MAX) {
            value = SPP_TIMEOUT_MAX;
          }
          Settings.SPP_Receive_Timeout = value;
        }
        i++;
      } else if (jsoneq(buf, &jt[i], "advertise_enabled") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = strtoul(param, NULL, 0);
          if (value != 0) {
            Settings.Advertising_Enabled = 1;
          } else {
            Settings.Advertising_Enabled = 0;
          }
        }
        i++;
      }
    }
  } while (0);
  
  // Reset the clock and authentiation timeout alarm
  RTC_InitTime();
  
  slogf(LOG_DEST_CONSOLE,"");
  slogf(LOG_DEST_BOTH,"Loaded device settings:");
  slogf(LOG_DEST_CONSOLE,"");
  slogf(LOG_DEST_BOTH,"FTP_InactivityTimeout: %d sec", Settings.FTP_InactivityTimeout);
  slogf(LOG_DEST_BOTH,"BTBaudRate: %u bps", Settings.BTBaudRate);
  slogf(LOG_DEST_BOTH,"BT_DiscoveryTime: %u sec", Settings.BT_DiscoveryTime);
  slogf(LOG_DEST_BOTH,"FTP_AuthenticationTimeout: %u sec", Settings.FTP_AuthenticationTimeout);
  slogf(LOG_DEST_BOTH,"BT_DisconnectOnVUSB: %u", Settings.BT_DisconnectOnVUSB);
  slogf(LOG_DEST_BOTH,"Face_MatchThreshold: %d ms", Settings.Face_MatchThreshold);
  slogf(LOG_DEST_BOTH,"Advertising_Interval_Min: %d ms", Settings.Advertising_Interval_Min);
  slogf(LOG_DEST_BOTH,"SPP_Send_Timeout: %d ms", Settings.SPP_Send_Timeout);
  slogf(LOG_DEST_BOTH,"SPP_Receive_Timeout: %d ms", Settings.SPP_Receive_Timeout);
  slogf(LOG_DEST_BOTH,"Advertising_Enabled: %d", Settings.Advertising_Enabled);
  slogf(LOG_DEST_CONSOLE,"");
  
}
#endif // FIRMWARE

#ifdef FIRMWARE
#ifdef TARGET_BOARD

static int OnKeyStatus = 0;
int OnKeyChange=0;
static void PMICStatMon(void)
{
  int OnKeyChange=0;
  static int OnKeyCount = 0;
  uint8_t Value;
  HAL_StatusTypeDef status;
      
  do {
    // Charge status update
    status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_CHG_STAT1, 
                              I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
    if (status != HAL_OK) {
      break;
    }
    if (Value & PMIC_I2C_CHG_STAT1_EOC) {
      PMICStatus |= PMIC_STAT_EOC;
    } else {
      PMICStatus &= ~PMIC_STAT_EOC;
    }
    status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_CHG_STAT2, 
                              I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
    if (status != HAL_OK) {
      break;
    }
    if (Value & PMIC_I2C_CHG_STAT2_CHDET) {
      PMICStatus |= PMIC_STAT_CHDET;
      // If charger is present, reset PMIC longpress timer to allow time for factory reset button delay
      HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_RST_CTRL, 
                       I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      Value |= PMIC_I2C_RST_CTRL_ON_INPUT;
      HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_RST_CTRL, 
                        I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
    } else {
      PMICStatus &= ~PMIC_STAT_CHDET;
    }
      
    status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_INT1_STAT, 
                              I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
    if (status != HAL_OK) {
      break;
    }
    if (Value & PMIC_I2C_INT1_STAT_LOBAT) {
      PMICStatus |= PMIC_STAT_LOW_BAT;
      slogf(LOG_DEST_BOTH, "[PMICGetDeviceSettings] PMIC_STAT_LOW_BAT");
    }
    // On key management
    if (Value & PMIC_I2C_INT1_STAT_ONKEY) {
      // On key has been changed
      OnKeyChange = 1;
      status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_RST_CTRL, 
                            I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        break;
      }
      if (Value & PMIC_I2C_RST_CTRL_ON_INPUT) {
        OnKeyStatus = 1;
      } else {
        OnKeyStatus = 0;
      }
        
    }
    if (OnKeyChange) {
      // Key has been changed
      if (!OnKeyStatus) {
        // Key has been released
        if (OnKeyCount >= PMIC_MEDIUM_PRESS_COUNT) {
          PMICStatus |= PMIC_STAT_MEDIUM_PRESS_END;
        } else if (OnKeyCount >= PMIC_SHORT_PRESS_COUNT) {
          PMICStatus |= PMIC_STAT_SHORT_PRESS_END;
        }
        OnKeyCount = 0;
      }
    } else {
      // Key hasn't been changed
      if (OnKeyStatus) {
        // Key is still pressed
        OnKeyCount++;
        if ((OnKeyCount == PMIC_FACTORY_RESET_COUNT) && (PMICStatus & PMIC_STAT_CHDET)) {
          PMICStatus |= PMIC_STAT_KEY_FACTORY_RESET;
          printf("FACTORY RESET\r\n");
        } else if (OnKeyCount == PMIC_MEDIUM_PRESS_COUNT) {
          PMICStatus |= PMIC_STAT_MEDIUM_PRESS_BEGIN;
        } else if (OnKeyCount == PMIC_SHORT_PRESS_COUNT) {
          PMICStatus |= PMIC_STAT_SHORT_PRESS_BEGIN;
        }
      } else {
        // Key is not pressed
        OnKeyCount = 0;
      }
    }
  } while (0);
}


#define ADC_FILTER_SIZE         8
static uint32_t AdcFilter[ADC_FILTER_SIZE];
static uint8_t AdcFilterIndex = 0;

static int Adc_Bat_Init(void)
{
  int res;
  ADC_ChannelConfTypeDef sConfig;
  GPIO_InitTypeDef GPIO_InitStruct;
  
  // init filter with invalid values
  for (res=0;res<ADC_FILTER_SIZE;res++) {
    AdcFilter[res] = -1;
  }
    
  ADC_BAT_CHANNEL_GPIO_CLK_ENABLE();
  VBAT_EN_CLK_ENA();
  
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = VBAT_EN_PIN;
  HAL_GPIO_Init(VBAT_EN_PORT, &GPIO_InitStruct);
  
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = ADC_BAT_CHANNEL_PIN;
  HAL_GPIO_Init(ADC_BAT_CHANNEL_GPIO_PORT, &GPIO_InitStruct);

  HAL_GPIO_WritePin(VBAT_EN_PORT, VBAT_EN_PIN, GPIO_PIN_SET); 

  ADC_BAT_CLK_ENABLE();
  ADC_BAT_CHANNEL_GPIO_CLK_ENABLE();
  
  // Configure the ADC peripheral
  AdcHandle.Instance          = ADC_BAT;
  
  AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV2;
  AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
  AdcHandle.Init.ScanConvMode          = DISABLE;
  AdcHandle.Init.ContinuousConvMode    = DISABLE;
  AdcHandle.Init.DiscontinuousConvMode = DISABLE;
  AdcHandle.Init.NbrOfDiscConversion   = 0;
  AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
  AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  AdcHandle.Init.NbrOfConversion       = 1;
  AdcHandle.Init.DMAContinuousRequests = DISABLE;
  AdcHandle.Init.EOCSelection          = DISABLE;
      
  res = HAL_ADC_Init(&AdcHandle);
  if(res == HAL_OK)
  {
    // Configure ADC regular channel
    sConfig.Channel      = ADC_BAT_CHANNEL;
    sConfig.Rank         = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    sConfig.Offset       = 0;
    
    res = HAL_ADC_ConfigChannel(&AdcHandle, &sConfig);
  }
  
  return (res);
}

int ADC_Bat_GetVal(int * val)
{
  int res, temp;
  uint8_t i;
  HAL_GPIO_WritePin(VBAT_EN_PORT, VBAT_EN_PIN, GPIO_PIN_SET);
  osDelay(1);
  res = HAL_ADC_Start(&AdcHandle);
  if (res == HAL_OK) {
    res = HAL_ADC_PollForConversion(&AdcHandle, 10);
    if (res == HAL_OK) {
      AdcFilter[AdcFilterIndex++] = HAL_ADC_GetValue(&AdcHandle);
      if (AdcFilterIndex >= ADC_FILTER_SIZE) {
        AdcFilterIndex = 0;
      }
      temp = 0;
      for (i=0;i<ADC_FILTER_SIZE;i++) {
        if (AdcFilter[i] != -1) {
          temp += AdcFilter[i];
        } else {
          break;
        }
      }
      temp = temp / i;
        
        
      if (temp > ADC_BAT_LEVEL_100) {
        *val = 100;
      } else if (temp > ADC_BAT_LEVEL_75) {
        *val = 75;
      } else if (temp > ADC_BAT_LEVEL_50) {
        *val = 50;
      } else if (temp > ADC_BAT_LEVEL_25) {
        *val = 25;
      } else {
        *val = 0;
      }
    }
  }
  HAL_ADC_Stop(&AdcHandle);
  HAL_GPIO_WritePin(VBAT_EN_PORT, VBAT_EN_PIN, GPIO_PIN_RESET);
  
  return (res);
}

int ADC_Bat_GetPercent()
{
  int res = -1;
  int temp;
  uint8_t i;
  
  // Only read the battery level if not currently charging
  if (!(PMICStatus & PMIC_STAT_CHDET)) {
  
          HAL_GPIO_WritePin(VBAT_EN_PORT, VBAT_EN_PIN, GPIO_PIN_SET);
          osDelay(1);
          res = HAL_ADC_Start(&AdcHandle);
          if (res == HAL_OK) {
            res = HAL_ADC_PollForConversion(&AdcHandle, 10);
            if (res == HAL_OK) {
              AdcFilter[AdcFilterIndex++] = HAL_ADC_GetValue(&AdcHandle);
              if (AdcFilterIndex >= ADC_FILTER_SIZE) {
                AdcFilterIndex = 0;
              }
              temp = 0;
              for (i=0;i<ADC_FILTER_SIZE;i++) {
                if (AdcFilter[i] != -1) {
                  temp += AdcFilter[i];
                } else {
                  break;
                }
              }
              temp = temp / i;      
              
              // These are the formulas for converting ADC value to battery %
              // (4.0V / 7.5) * 4095 = 2184 = 100%
              // (3.0V / 7.5) * 4095 = 1638 = Board Off (0%)
                    
              res =  (int)((((double)temp - 1638.0) / (2184.0 - 1638.0)) * 100);    
              if (res > 100) res = 100;   
              if (res < 0) res = -1;
              
            }
          }
          
          HAL_ADC_Stop(&AdcHandle);
          HAL_GPIO_WritePin(VBAT_EN_PORT, VBAT_EN_PIN, GPIO_PIN_RESET);
    
    // End of charge detected? If so, report battery as 100% charged
    } else if (PMICStatus & PMIC_STAT_EOC) { 
      res = 100;
  }
  
  return (res);
}

#else // TARGET_BOARD
static int OnKeyStatus = 0;

static void PMICStatMon(void)
{
  PMICStatus |= PMIC_STAT_EOC;
  PMICStatus |= PMIC_STAT_CHDET;
}

static int Adc_Bat_Init(void)
{
  return (HAL_OK);
}

int ADC_Bat_GetVal(int * val)
{
  *val = 100;
  return (HAL_OK);
}
  


#endif // TARGET_BOARD

#endif // FIRMWARE

#ifdef FIRMWARE
static void PMICLed(void)
{
  static int led_seq = 0;
  int led;
  for (led=0; led<PMIC_LED_NUMBER;led++) {
    if (PMICLedCtrl[led] & (1 << led_seq)) {
      BSP_LED_On(PMICBSPLed[led]);
    } else {
      BSP_LED_Off(PMICBSPLed[led]);
    }
  }
  led_seq++;
  if (led_seq >= LED_BLINK_NUMBER) {
    led_seq = 0;
  }
}

// Under normal operation (not asleep) the yellow LED
// should never be fully off
static void PMICCheckCardState(void) {
 
  if ((PMICLedCtrl[PMIC_LED_STATUS_INDEX] == LED_OFF) && !returnFromSleep) {
    
      int x = 0;
      int cnt = 0;
      
      printf("ERROR - PMICCheckCardState()\r\n");
      
      BSP_LED_Off(LED_YELLOW);
      BSP_LED_On(LED_RED);
  
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
  
}

#endif // FIRMWARE

void PMIC_2_8V(uint8_t _State)
{
  // 2.8V is provided by LDO2 of PMIC.
  uint8_t Value;
  
  Value = _State?PMIC_I2C_LDO2_2_8V:0;
  HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_LDO2_VOLT, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
}

HAL_StatusTypeDef I2C_Err=HAL_OK;
void PMIC_1_8V(uint8_t _State)
{
  // 2.8V is provided by LDO2 of PMIC.
  uint8_t Value;
  
  Value = _State?PMIC_I2C_LDO1_1_8V:0;
  I2C_Err = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_LDO1_VOLT, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
  if(I2C_Err != HAL_OK)
  {
    Error_Handler();
  }
}

void PMIC_VMCU(uint8_t _Value)
{
  static uint8_t ReadValue;
  
  I2C_Err = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_SD1_VOLT,
                             I2C_MEMADD_SIZE_8BIT, &_Value, 1, PMIC_I2C_TIMEOUT);
  if(I2C_Err != HAL_OK)
  {
    Error_Handler();
  }
  
  // we read writen value because when HAL_I2C_Mem_Write() return, the last byte
  // is still in the I2C controler when the function return and when we put
  // the MCU in STOP mode it stay there and VMCU is not changed to the desired value.
  // By doing a read we are sure than the PMIC recived the command and Vmcu is
  // at the desired value. 
  I2C_Err = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_SD1_VOLT, 
                             I2C_MEMADD_SIZE_8BIT, &ReadValue, 1, PMIC_I2C_TIMEOUT);
  
  if(I2C_Err != HAL_OK)
  {
    Error_Handler();
  }
  
  if(_Value != ReadValue)
  {
    Error_Handler();
  }

}

// This function is used in bootloader so we can't use printf() inside.
#ifdef TARGET_BOARD
int PMICInit(void)
{
  int res = 1;
  uint8_t Value;
  HAL_StatusTypeDef status;
  GPIO_InitTypeDef GPIO_InitStruct;
  
  if(HAL_I2C_IsDeviceReady(&I2cHandle, PMIC_I2C_ADDR, 5, 100)==HAL_OK)
  {
    do {
      // Verify PMIC ID
      status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_ASIC_ID1, 
                                I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status == HAL_OK) {
        //printf("ID: 0x%x\r\n", Value);
        if (Value != PMIC_I2C_ASIC_ID1_VAL) {
          // printf("%s Bad ID\r\n", __func__);
          break;
        }
      } else {
        //printf("%s Error reading ID\r\n", __func__);
        break;
      }
      
      GPIO_InitStruct.Pin = eMMC_VCCQ_EN_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(eMMC_VCCQ_EN_PORT, &GPIO_InitStruct);
      HAL_GPIO_WritePin(eMMC_VCCQ_EN_PORT, eMMC_VCCQ_EN_PIN, GPIO_PIN_RESET); 
        
#ifdef FIRMWARE
      GPIO_InitStruct.Pin = VSUP_BT_EN_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(VSUP_BT_EN_PORT, &GPIO_InitStruct);
      HAL_GPIO_WritePin(VSUP_BT_EN_PORT, VSUP_BT_EN_PIN, GPIO_PIN_RESET); 

      BT_HCI_OE_CLK_ENA();
      GPIO_InitStruct.Pin = BT_HCI_OE_PIN;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Pull = GPIO_PULLUP;
      HAL_GPIO_Init(BT_HCI_OE_PORT, &GPIO_InitStruct);
      HAL_GPIO_WritePin(BT_HCI_OE_PORT, BT_HCI_OE_PIN, GPIO_PIN_RESET); 

      BT_SHUTD_N_CLK_ENA();
      GPIO_InitStruct.Pin       = BT_SHUTD_N_PIN;
      GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_OD;
      GPIO_InitStruct.Pull      = GPIO_NOPULL;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
      HAL_GPIO_Init(BT_SHUTD_N_PORT, &GPIO_InitStruct);
      HAL_GPIO_WritePin(BT_SHUTD_N_PORT, BT_SHUTD_N_PIN, GPIO_PIN_RESET); 


      // PMIC_GPIO1 (PA4) is used to generate interrupt from PMIC
      __HAL_RCC_GPIOA_CLK_ENABLE();
      GPIO_InitStruct.Pin       = GPIO_PIN_4;
      GPIO_InitStruct.Mode      = GPIO_MODE_IT_RISING;
      GPIO_InitStruct.Pull      = GPIO_NOPULL;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
      
      /* set Button EXTI Interrupt to the lowest priority */
      HAL_NVIC_SetPriority((IRQn_Type)(EXTI4_IRQn), 0x0F, 0x0);
      
      __HAL_RCC_GPIOA_CLK_ENABLE();      
      GPIO_InitStruct.Pin       = GPIO_PIN_2;
      GPIO_InitStruct.Mode      = GPIO_MODE_IT_FALLING;
      GPIO_InitStruct.Pull      = GPIO_NOPULL;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
      
      /* set Button EXTI Interrupt to the lowest priority */
      HAL_NVIC_SetPriority((IRQn_Type)(EXTI2_IRQn), 0x0F, 0x0);   
      
      Value = PMIC_I2C_MODE_OUTPUT + PMIC_I2C_IOSF_INTERRUPT_OUTPUT;
      // Program PMIC_GPIO1 to generate an interruption.
      status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_GPIO1_CONTROL, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        break;
      }
     

      // Unmask interrupts for selected source
      status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_INT1_MASK, 
                                I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error reading PMIC_I2C_INT1_MASK\r\n", __func__);
        break;
      }
      Value &= ~(PMIC_I2C_INT1_STAT_ONKEY | PMIC_I2C_INT1_STAT_LOBAT | PMIC_I2C_INT1_CHARGE_DETECT);
      status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_INT1_MASK, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error writing PMIC_I2C_INT1_MASK\r\n", __func__);
        break;
      }
      
      // Read int status to clear all flags
      status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_INT1_STAT, 
                              I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error reading PMIC_I2C_INT1_STAT\r\n", __func__);
        break;
      }
      
      
      
      
#endif // FIRMWARE

      // Set SD1 Voltage and controls
      status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_SD1_CTRL1, 
                              I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error reading PMIC_I2C_SD1_CTRL1\r\n", __func__);
        break;
      }
      Value |= PMIC_I2C_SD1_CTRL1_SD1_FSEL;
      status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_SD1_CTRL1, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error writing PMIC_I2C_SD1_CTRL1\r\n", __func__);
        break;
      }
      
      Value = PMIC_I2C_SD1_3_0V;
      status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_SD1_VOLT,
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error writing PMIC_I2C_SD1_VOLT\r\n", __func__);
        break;
      }
      
      // Set LDO1 voltage (Bluetooth)
#ifdef FIRMWARE      
      Value = PMIC_I2C_LDO1_1_8V;
#else
      Value = PMIC_I2C_LDO1_OFF;
#endif
      status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_LDO1_VOLT, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error writing PMIC_I2C_LDO1_VOLT\r\n", __func__);
        break;
      }
      // Set LDO2 voltage (eMMC)
      Value = PMIC_I2C_LDO2_2_8V;
      status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_LDO2_VOLT, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error writing PMIC_I2C_LDO2_VOLT\r\n", __func__);
        break;
      }
      // Enable supplies
      HAL_GPIO_WritePin(eMMC_VCCQ_EN_PORT, eMMC_VCCQ_EN_PIN, GPIO_PIN_SET);
#ifdef FIRMWARE
      HAL_GPIO_WritePin(VSUP_BT_EN_PORT, VSUP_BT_EN_PIN, GPIO_PIN_SET); 
#endif // FIRMWARE
      
      // Set Battery voltage monitor
#ifdef FIRMWARE
      Value = PMIC_I2C_BAT_VOLT_MON_F2_9_R3_1;
      status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_BAT_VOLT_MON, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error writing PMIC_I2C_LDO2_VOLT\r\n", __func__);
        break;
      }
#endif	// FIRMWARE
      
      // Set charge controls
      Value = PMIC_I2C_CHG_VCHOFF_SETUP;
      status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_CHG_VOLTAGE_CTRL, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error writing PMIC_I2C_CHG_VCHOFF_SETUP\r\n", __func__);
        break;
      }
      Value = PMIC_I2C_CHG_CURR_CTRL_SETUP;
      status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_CHG_CURR_CTRL, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error writing PMIC_I2C_LDO2_VOLT\r\n", __func__);
        break;
      }
     
      Value = PMIC_I2C_CHG_SUP_SETUP;
      status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_CHG_SUP, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error writing PMIC_I2C_LDO2_VOLT\r\n", __func__);
        break;
      }
     
      Value = PMIC_I2C_CHG_CTRL_SETUP;
      status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_CHG_CTRL, 
                                 I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      if (status != HAL_OK) {
        //printf("%s Error writing PMIC_I2C_LDO2_VOLT\r\n", __func__);
        break;
      }
      
      res = 0;
    } while (0);
  }
  return res;
}
#else // TARGET_BOARD
int PMICInit(void)
{
  int res = 1;
  
#ifdef FIRMWARE
  GPIO_InitTypeDef GPIO_InitStruct;
  BT_SHUTD_N_CLK_ENA();
  GPIO_InitStruct.Pin       = BT_SHUTD_N_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  HAL_GPIO_Init(BT_SHUTD_N_PORT, &GPIO_InitStruct);
  HAL_GPIO_WritePin(BT_SHUTD_N_PORT, BT_SHUTD_N_PIN, GPIO_PIN_RESET); 

#endif // FIRMWARE
      
  res = 0;
  
  return res;
}
#endif // TARGET_BOARD

#ifdef FIRMWARE
// We only allow external BLE connections for a short period of time so this
// method allows us to track those connection and forecfully disconnect when necessary
void StartBLEConnetionTimer(unsigned int stackID, BD_ADDR_t bdAddr)
{        
        // Get the current elapsed seconds
        RTC_GetElapsedTime(&BLEConnectTime);
        
        slogf(LOG_DEST_CONSOLE,"[StartBLEConnetionTimer] connection timer started, time: %d seconds\r\n",BLEConnectTime);
        
        
        BluetoothStackID = stackID;
        BD_ADDR = bdAddr;
}

// Cancel the BLE connection timer
void StopBLEConnetionTimer(void)
{
       BluetoothStackID = 0;
       slogf(LOG_DEST_CONSOLE,"[StopBLEConnetionTimer] connection timer stopped\r\n");
}

// Check if we need to terminate the BLE connection
void CheckBLEConnectionTimer(void)
{
       uint32_t now;
       uint32_t elapsed;
                 
       // Is timer currently enabled?
      if (BluetoothStackID != 0) 
      {     
         // Get the current elapsed seconds
         RTC_GetElapsedTime(&now);
        
         // How many seconds since timer started?
         elapsed = now - BLEConnectTime;
       
         if (elapsed >= BLE_CONNECTION_TIMEOUT) {
           
                GAP_LE_Disconnect(BluetoothStackID, BD_ADDR);
                slogf(LOG_DEST_BOTH,"[CheckBLEConnectionTimer] closing connection, timeout: %d seconds\r\n", elapsed);
         }
         
      }
    
}
#endif // FIRMWARE

// This function is used in bootloader so we can't use printf() inside.
int PMICPreInit(void)
{
  int res = 1;
  int timeout = 10;
  HAL_StatusTypeDef status = HAL_ERROR;
  uint8_t Value;
  
  while ((status != HAL_OK) && (timeout-- > 0))
  {
    status = HAL_I2C_IsDeviceReady(&I2cHandle, PMIC_I2C_ADDR, 5, 100);
    if (status != HAL_OK) {
      HAL_Delay(10);
    }
  }
  
  if (status == HAL_OK) {
    Value = PMIC_I2C_SD1_3_0V;
    status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_SD1_VOLT, I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
    if (status == HAL_OK) {
      res = 0;
    } 
  } 
  return (res);
}

int PMICSetBoardOff(void)
{
  int res = 1;
  uint8_t Value;
  HAL_StatusTypeDef status;

  // Read ResetControl register
  status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_RST_CTRL, 
                            I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
  if (status == HAL_OK) {
    // Set power_off
    Value |= PMIC_I2C_RST_CTRL_PWR_OFF;
    status = HAL_I2C_Mem_Write(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_RST_CTRL, 
                               I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
    if (status == HAL_OK) {
      printf("Board off\r\n");
      // Wait for off from PMIC
      while(1) {
        HAL_Delay(100);
      }
    }
  }
    
  return (res);
}


#ifdef FIRMWARE
int PMICStartFactoryReset(void)
{
  int res = 1;
    
  RamParam.DoSelfTest = 1;
  RamParam.Format = 1;
  FLASH_If_SaveParam();
  HAL_NVIC_SystemReset();
  
  // Should not exit from above function
  
  return (res);
}
#endif // FIRMWARE

#ifdef PMIC_DEBUG
static void PMICGetChargeStatus(void)
{
  uint8_t Value;
  HAL_StatusTypeDef status;

  // Read Charge status
  status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_CHG_STAT1, 
                            I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
  if (status == HAL_OK) {
    printf("PMIC_I2C_CHG_STAT1: 0x%x ", Value);
  }
  
  status = HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_CHG_STAT2, 
                            I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
  if (status == HAL_OK) {
    printf("PMIC_I2C_CHG_STAT2: 0x%x\r", Value);
  }

}

#endif


FRESULT CheckBluStorMMC() {
  
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
    res = FR_OK;
  } while (0);
  
  return (res);
  
}

  
   /* The following function is the main user interface thread.  It     */
   /* opens the Bluetooth Stack and then drives the main user interface.*/
#ifndef FCC_TESTS
#ifdef FIRMWARE

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  exti_flag |= GPIO_Pin;
}

void PMICThread(void const *argument)
{
  int res;
  int state = 1;
  //int FTPActivityTracking = -1;
  int BTActivityTracking = -1;
  uint32_t AbortTimeout;
  uint32_t OffTimeout;
  int SelfTestTimeOut=100;      // 10 seconds
  uint8_t Value;

#ifdef PMIC_DEBUG
  int sec_count = 0;
#endif    
   
  osSemaphoreDef(PMIC_SEM);
  PMIC_Semaphore = osSemaphoreCreate(osSemaphore(PMIC_SEM), 1);
  
  osSemaphoreDef(RTC_SEM);
  RTC_Semaphore = osSemaphoreCreate(osSemaphore(RTC_SEM), 1);
  
  RtcHandle.Instance = RTC;
  
  /*##-1- Configure the RTC peripheral #######################################*/
  /* Configure RTC prescaler and RTC data registers */
  /* RTC configured as follow:
      - Hour Format    = Format 12
      - Asynch Prediv  = Value according to source clock
      - Synch Prediv   = Value according to source clock
      - OutPut         = Output Disable
      - OutPutPolarity = High Polarity
      - OutPutType     = Open Drain */ 
  RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RtcHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RtcHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  
  if(HAL_RTC_Init(&RtcHandle) != HAL_OK)
  {
    /* Initialization Error */
    slogf(LOG_DEST_BOTH,"RTC init error"); // db???
  } else {
    slogf(LOG_DEST_BOTH,"[PMICThread] RTC initialized");
  }
  
  
  if(RamParam.DoSelfTest) {
    slogf(LOG_DEST_BOTH,"Begin SelfTest...");
    PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_SELF_TEST;
    PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_SELF_TEST;
  } else {
    PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_OFF;
    PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_SHORT_BLINK;
  }
  res = PMICInit();

  if (res == FR_OK) 
  {
    SelfTest.PMIC = SELF_TEST_SUCCESS;

    // Start FTP server task (this task initializes FATFS)
    osThreadDef(FTP_Thread, FTPThread, osPriorityNormal, 0, 16 * configMINIMAL_STACK_SIZE);
    FTPTaskHandle = osThreadCreate(osThread(FTP_Thread), NULL);
    
    // Flash while formatting and wait for eMMC to be ready
    if (RamParam.Format != 0) {
      
      AbortTimeout = 100;
      while ((SelfTest.eMMC != SELF_TEST_SUCCESS) && (AbortTimeout--)) {
        // If formatting wait before start Bluetooth task because it will use eMMC
        PMICLed();
        osDelay(100);
      }
      
    } else {
        
        // Normal start-up, wait for eMMC to be ready
        AbortTimeout = 100;
        while ((SelfTest.eMMC != SELF_TEST_SUCCESS) && (AbortTimeout--)) {
          osDelay(100);
        }
    }
    
    // Settings cannot be read before since eMMC is not ready
    PMICGetDeviceSettings();
    
    /* Determine correct initial FTPLocked status */
    if (FindValidTemplate() == FR_OK) {
      slogf(LOG_DEST_BOTH,"[PMICThread] Initial lock status: LOCKED");
      FTPLocked = TRUE;
    } else {
      slogf(LOG_DEST_BOTH,"[PMICThread] Initial lock status: UNLOCKED");
      FTPLocked = FALSE;
    }
    
    // Start SPP (Bluetooth task)
    osThreadDef(SPP_Thread, SPPThread, osPriorityRealtime, 0, 8 * configMINIMAL_STACK_SIZE);
    SPPTaskHandle = osThreadCreate(osThread(SPP_Thread), NULL);
  }
  
  if(RamParam.DoSelfTest)
  {
    res=1; 
    // wait that Bluetooth task finishes to display information
    AbortTimeout = (3 * 10);
    while ((SelfTest.SerialBT != SELF_TEST_SUCCESS) && (AbortTimeout--)) {
      PMICLed();
      osDelay(100);
    }

    
    // Those tests are not presently implemented... 
    SelfTest.NFC = SELF_TEST_SUCCESS;
    
    while(SelfTestTimeOut)
    {
      if(SelfTest.eMMC == SELF_TEST_SUCCESS && SelfTest.NFC == SELF_TEST_SUCCESS && SelfTest.PMIC == SELF_TEST_SUCCESS && SelfTest.SerialBT == SELF_TEST_SUCCESS)        
      {
        res = FR_OK;
        RamParam.DoSelfTest = 0;
        FLASH_If_SaveParam();   
        break;
      }
      PMICLed();
      osDelay(100);
      SelfTestTimeOut--;
    }
    
    slogf(LOG_DEST_BOTH,"Self Test Result:");
    slogf(LOG_DEST_BOTH,"eMMC = %s.",eTestResultStr[SelfTest.eMMC]);
    slogf(LOG_DEST_BOTH,"PMIC = %s.",eTestResultStr[SelfTest.PMIC]);
    slogf(LOG_DEST_BOTH,"NFC = %s.", eTestResultStr[SelfTest.NFC]);
    slogf(LOG_DEST_BOTH,"Bluetooth serial port = %s.",eTestResultStr[SelfTest.SerialBT]);
  }

  
  if(res != FR_OK)
  {
    slogf(LOG_DEST_BOTH,"SelfTest failed.");
    while (1) 
    {
      PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_ERROR_STATUS;
      PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_ERROR_CHARGE;
      PMICLed();
      osDelay(100);
    }
  }
  
  Adc_Bat_Init();
  
  osDelay(50);
  
  PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_OFF;
  PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_SHORT_BLINK;
  
  slogf(LOG_DEST_CONSOLE,"");
  slogf(LOG_DEST_BOTH, "Free Heap Size: %u", xPortGetFreeHeapSize());
  slogf(LOG_DEST_BOTH, "Minimum Free Heap Size: %u", xPortGetMinimumEverFreeHeapSize());
  BatteryPercent = ADC_Bat_GetPercent();
  slogf(LOG_DEST_BOTH,"Battery level: %d%%", BatteryPercent);
  slogf(LOG_DEST_CONSOLE,"");
  
  // Initialize the PMCI_Is_Charging flag
  if ((!PMIC_Is_Charging) && (PMICStatus & PMIC_STAT_CHDET)) {
    PMIC_Is_Charging = TRUE;
  }
  
  // Make sure mandatory directory structure exists
  CheckBluStorMMC();  
 
  // PMIC loop
  while (1) {
    
    // Check if authentication has timedout even though we are currently powered on but
    // don't timeout if there is an active SPP connection
    RTC_GetElapsedTime(&TimeFromLastConnect);
    if ((Settings.FTP_AuthenticationTimeout > 0) && (TimeFromLastConnect >= Settings.FTP_AuthenticationTimeout) && (!FTPLocked) && (!SPPOpened)) {    
         slogf(LOG_DEST_BOTH,"Authentication elapsed: %d > %d", TimeFromLastConnect, Settings.FTP_AuthenticationTimeout);
         FTPLocked = TRUE;
         AdvertiseLockStatus(FTPLocked);
         RTC_InitTime(); 
    }
    
    // Check BLE Connection timeout
    CheckBLEConnectionTimer();
    
    /*
    if(CDC_NeedReset)
    {
      CDC_NeedReset=FALSE;
      CDC_Reset();
    }
    */
    
    if(eMMC_TurnOn)
    {
      eMMC_PowerOn();
      eMMC_TurnOn = FALSE;
    }
    
    if(eMMC_TurnOff)
    {
      eMMC_PowerOff();
      eMMC_TurnOff = FALSE;
    }
    
    PMICStatMon();
    
    if(GotoStop && OnKeyStatus==0)
    {
      GotoStop = FALSE;                                 // ok
      slogf(LOG_DEST_BOTH, "Entering STOP mode");               // ok
      
      //DisconnectLE();
      
      // Close any active BT connections
      if (!CloseBluetooth()) {
        // Allow BT radio to finish closing connections
        osDelay(10000);
        
      }
      
      if(!SPP_Sleep())
      {
         osDelay(500);          // allow time for advertising to turn on
      }
      
      // Stop task scheduling
      vTaskSuspendAll();
      CDC_Stop();
      eMMC_PowerOff();                                  // ok
      
      BSP_LED_Off(PMIC_LED_CHARGE);                     // ok
      BSP_LED_Off(PMIC_LED_STATUS);                     // ok
#if TARGET_BOARD      
      HAL_GPIO_WritePin(VBAT_EN_PORT, VBAT_EN_PIN, GPIO_PIN_RESET);     // ok
#endif
      SystemClockHSI_Config();                          // ok
      HAL_I2C_DeInit (&I2cHandle);                      // ok
      I2C_Init();                                       // ok
      PMIC_VMCU(PMIC_I2C_SD1_1_8V);                     // ok
      
#if 0 // For current test without BT
      PMIC_1_8V(0);     // first command not accepted.
      PMIC_1_8V(0);     
      HAL_GPIO_WritePin(BT_SHUTD_N_PORT, BT_SHUTD_N_PIN, GPIO_PIN_RESET);
#endif 
      Gpio_Init();                                      // Ok
      HAL_I2C_DeInit (&I2cHandle);                      // ok
      
      // clear EXTI int Flag.
      if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_4) != RESET)
      {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_4);
      }
      
      if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET)
      {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
      }
      
      // enable external int for PMIC_GPIO1
      HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI4_IRQn));
      HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI2_IRQn));
      BT_EnableCTSInt(1);
      
      // enter stop mode
      HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
      
      BT_EnableCTSInt(0);  
      HAL_NVIC_DisableIRQ((IRQn_Type)(EXTI4_IRQn));
      HAL_NVIC_DisableIRQ((IRQn_Type)(EXTI2_IRQn));

      I2C_Init();                                       // ok
      PMIC_VMCU(PMIC_I2C_SD1_3_0V);                     // ok
      HAL_I2C_DeInit (&I2cHandle);                      // ok
      SystemClock_Config();     // return to normal operation clock configuration.      ok
      I2C_Init();                                       // ok
      eMMC_PowerOff();                                  // ok  
      FTPActivity++;
      BTActivity++;
      state = 1;
      FTPAbort = FALSE;
      FTPAborted = FALSE;
      // Clear PMIC interrupt status
      HAL_I2C_Mem_Read(&I2cHandle, PMIC_I2C_ADDR, PMIC_I2C_INT1_STAT, 
                              I2C_MEMADD_SIZE_8BIT, &Value, 1, PMIC_I2C_TIMEOUT);
      
      // If Low battery interrupt
      if (Value & PMIC_I2C_INT1_STAT_LOBAT) {
        // Shutdown the board
        PMICSetBoardOff();
      }
      
      // Always power the memory back on after returning from sleep
      eMMC_PowerOn();
      
      // Resume task scheduling
      xTaskResumeAll();
      
      slogf(LOG_DEST_BOTH, "Return from STOP mode");
            
      slogf(LOG_DEST_CONSOLE,"");
      slogf(LOG_DEST_BOTH,"Free Heap Size: %u", xPortGetFreeHeapSize());
      slogf(LOG_DEST_BOTH,"Minimum Free Heap Size: %u", xPortGetMinimumEverFreeHeapSize());
      BatteryPercent = ADC_Bat_GetPercent();
      slogf(LOG_DEST_BOTH,"Battery level: %d%%", BatteryPercent);
      slogf(LOG_DEST_CONSOLE,"");

      // Notify PMICStatMon() that we just returned from sleep mode
      // so we can check the button status
      returnFromSleep = TRUE;
      
      RTC_GetElapsedTime(&TimeFromLastConnect);
      slogf(LOG_DEST_BOTH,"Time elapsed: %d seconds\r\n", TimeFromLastConnect);
      if ((Settings.FTP_AuthenticationTimeout > 0) && (TimeFromLastConnect >= Settings.FTP_AuthenticationTimeout) && (!FTPLocked)) {
         slogf(LOG_DEST_BOTH,"Authentication elapsed: %d > %d\r\n", TimeFromLastConnect, Settings.FTP_AuthenticationTimeout);
         FTPLocked = TRUE;
         AdvertiseLockStatus(FTPLocked);
      } 
      
      // Reset the clock and authentiation timeout alarm
      RTC_InitTime();
      
      if(exti_flag & GPIO_PIN_2) {
        exti_flag = 0;
        Set_SPP_Event(SPP_EVT_BUTTON_WAKEUP);
        slogf(LOG_DEST_BOTH,"Button wakeup");

      } else if(exti_flag & GPIO_PIN_4) {
        slogf(LOG_DEST_BOTH,"PMIC wakeup");
      } else {
        slogf(LOG_DEST_BOTH,"Other wakeup source");
      }
      
      continue;
    
    // We're not going to sleep
    } else {
      
      // Check for bad card state and reset if necessary
      PMICCheckCardState();
      
    }
    
    /*
    if(PMICStatus & PMIC_STAT_CHDET)
    {
      if(CDC_Started == FALSE)
      {
        CDC_Start();
        CDC_Reset();
      }
    }else
    {
      if(CDC_Started)
      {
        CDC_Stop();
      }
    }
    */
    
    if (PMICStatus & PMIC_STAT_KEY_FACTORY_RESET) {
      // Start factory reset
      PMICStartFactoryReset();
    }
    
    // If SPP is open, we will prevent the card from going to sleep, otherwise
    // if we switch devices while alseep, the BT radio and firmware will be
    // confused 
    if (SPPOpened || !BTPaired) {
            
      //if ((FTPActivityTracking != FTPActivity) || (PMICStatus & PMIC_STAT_CHDET)) {
        //FTPActivityTracking = FTPActivity;
        // May not need this anymore -- probably has no impact since we never sleep
        // while SPP is open
        OffTimeout = (osKernelSysTick() + (Settings.FTP_InactivityTimeout * 1000));
      //}  
      
    } else {
      if ( (BTActivityTracking != BTActivity) || (PMICStatus & PMIC_STAT_CHDET) || (SPP_Pairing_Mode()) ) {
        BTActivityTracking = BTActivity;
        OffTimeout = (osKernelSysTick() + 10000); // 10 second timeout before sleep
      }
    }
    if ((osKernelSysTick() >= OffTimeout) || (PMICStatus & PMIC_STAT_LOW_BAT)) {
      // Simulate off key
      PMICStatus |= PMIC_STAT_SHORT_PRESS_END;
    }
    // On key management
    if (PMICStatus & PMIC_STAT_KEY_PRESS_MSK) {
      if (PMICStatus & PMIC_STAT_SHORT_PRESS_BEGIN) {
        PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_ON;
      } else if (PMICStatus & PMIC_STAT_SHORT_PRESS_END) {
        
        if (state == 0) {
          if (BTDiscMode == FALSE) {
            PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_SHORT_BLINK;
          } else {
            PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_TRIPLE_BLINK;
          }
          state = 1;
          FTPAbort = FALSE;
          FTPAborted = FALSE;
          
        } else {
          
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_OFF;
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_OFF;
          state = 0;
          FTPAbort = TRUE;
          FTPAborted = FALSE;
          osSemaphoreRelease(FTP_RxFifoEmpty);
          osSemaphoreRelease(FTP_RxDataReady);
          AbortTimeout = (osKernelSysTick() + FTP_ABORT_TIMEOUT);
          
          //wait for abort response from FTP server                        
          while ((!FTPAborted) && (osKernelSysTick() < AbortTimeout)) {
            BSP_LED_Toggle(PMIC_LED_STATUS);
            osDelay(100);
          }
          
          if(PMICStatus & PMIC_STAT_LOW_BAT)
          {
            // Power Off board (not effective for development setup)
            PMICSetBoardOff();
            // Should not exit from above function
            while(1) {
              osDelay(100);
            }
          }else
          {
            GotoStop = TRUE;
            PMICStatus = 0;
          }
        }
      } else if (PMICStatus & PMIC_STAT_MEDIUM_PRESS_BEGIN) {
        PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_TRIPLE_BLINK;
        Set_SPP_Event(SPP_EVT_BUTTON_MEDIUM_PRESS);
      }
      PMICStatus &= ~PMIC_STAT_KEY_PRESS_MSK;
    }
    if (CDCOpenedTrk != CDCOpened) {
      // USB port status has changed
      if (CDCOpened) {
        // Port has been opened
        FTPAbort = TRUE;
        FTPAborted = FALSE;
        osSemaphoreRelease(FTP_RxFifoEmpty);
        osSemaphoreRelease(FTP_RxDataReady);
        AbortTimeout = (osKernelSysTick() + FTP_ABORT_TIMEOUT);
        //wait for abort response from FTP server                        
        while ((!FTPAborted) && (osKernelSysTick() < AbortTimeout)) {
          osDelay(10);
        }
        // Lock FTP server
        FTPLocked = TRUE;
        FTPAbort = FALSE;
        osDelay(10);

        pWriteABuffer = CDC_WriteABuffer;
        FtpServerReset();
      } else {
        // Port has been closed
        FTPAbort = TRUE;
        FTPAborted = FALSE;
        osSemaphoreRelease(FTP_RxFifoEmpty);
        osSemaphoreRelease(FTP_RxDataReady);
        AbortTimeout = (osKernelSysTick() + FTP_ABORT_TIMEOUT);
        //wait for abort response from FTP server                        
        while ((!FTPAborted) && (osKernelSysTick() < AbortTimeout)) {
          osDelay(10);
        }
        
        // Lock FTP server
        FTPLocked = TRUE;
        FTPAbort = FALSE;
        osDelay(10);
        pWriteABuffer = BT_WriteABuffer;
      }
      CDCOpenedTrk = CDCOpened;
    }

    if ((state) && (PMICLedCtrl[PMIC_LED_STATUS_INDEX] != LED_ON)) {
      if (BTDiscMode == FALSE) {
        PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_SHORT_BLINK;
      } else {
        PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_TRIPLE_BLINK;
      }
    }
    
    // Did the charger just come on? Then save the last battery level
    // to determine if card is already at 100% so we don't flash the red LED
    if ((!PMIC_Is_Charging) && (PMICStatus & PMIC_STAT_CHDET)) {
        PMIC_Is_Charging = TRUE;   
        LastBatteryPercent = BatteryPercent;
        
    // Reset PMIC_Is_Charging if we're no longer charging
    } else if ((PMIC_Is_Charging) && !(PMICStatus & PMIC_STAT_CHDET)) {
        PMIC_Is_Charging = FALSE;
    } else {
        BatteryPercent = ADC_Bat_GetPercent();
    }
    
    if (PMICStatus & PMIC_STAT_CHDET) {
      if ((PMICStatus & PMIC_STAT_EOC) || (LastBatteryPercent == 100)) {
        PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_ON;
      } else {
        PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_LONG_BLINK;
      }
    } else {
      PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_OFF;
    }
      
    PMICLed();
        
    osSemaphoreWait(PMIC_Semaphore , 100); 
    
 
#ifdef PMIC_DEBUG    
    sec_count++;
    if (sec_count >= 9) {
      PMICGetChargeStatus();
      sec_count = 0;
    }
#endif    
  }
}
#endif // FIRMWARE
#else // FCC_TESTS

int FCCTestEnable=0;
int FCCTestNumber=1;

static void GetFCCTestNumber(void)
{
  jsmn_parser jp;
  jsmntok_t jt[DEV_SETTINGS_MAX_TOKEN];
  int res;
  int i;
  char buf[256];
  char param[32];
  unsigned long value;
  FIL fp;
  UINT read;
  
  memset(buf, 0, sizeof(buf));
  
  do {
  res = f_open(&fp, "/device/fcctstno", FA_READ | FA_OPEN_EXISTING);
  if (res == FR_OK) {
      res = f_read(&fp, (void *) buf, sizeof(buf), &read);
    f_close(&fp);
    } else {
      printf("Use default device settings\r\n");
      break;
    }
    
    jsmn_init(&jp);
    
    res = jsmn_parse(&jp, buf, read, jt, sizeof(jt)/sizeof(jt[0]));
    if (res < 0) {
      printf("JSMN failed to parse buf: %d\r\n", res);
      break;
    }
    
    if (res < 1 || jt[0].type != JSMN_OBJECT) {
      printf("JSMN object expected\r\n");
      break;
    }
    
    for (i=1; i<res; i++) {
      if (jsoneq(buf, &jt[i], "test") == 0) {
        memset(param, 0, sizeof(param));
        if ((jt[i+1].end - jt[i+1].start) <= sizeof(param)) {
          strncpy(param, (buf + jt[i+1].start), (jt[i+1].end - jt[i+1].start));
          value = (int) strtoul(param, NULL, 0);
          if (value > 23) {
            value = 1;
          }
          FCCTestNumber = (int) value;
        }
        i++;
      }
    }
  } while (0);
  
  Display(("FCCTestNumber: %d\r\n", FCCTestNumber));
}

void PMICThread(void const *argument)
{
  int res;
  int state = 1;
  int FTPActivityTracking = -1;
  uint32_t AbortTimeout;
  uint32_t OffTimeout;
  int SelfTestTimeOut=100;      // 10 seconds

  if(RamParam.DoSelfTest) {
    printf("Begin SelfTest...\r\n");
    PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_SELF_TEST;
    PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_SELF_TEST;
  } else {
    PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_OFF;
    PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_SHORT_BLINK;
  }
  res = PMICInit();
  
  if (res == FR_OK) 
  {
    SelfTest.PMIC = SELF_TEST_SUCCESS;

    // Start FTP server task (this task initializes FATFS)
    osThreadDef(FTP_Thread, FTPThread, osPriorityNormal, 0, 16 * configMINIMAL_STACK_SIZE);
    FTPTaskHandle = osThreadCreate(osThread(FTP_Thread), NULL);
    
    if (RamParam.Format != 0) {
      AbortTimeout = 10;
      while ((SelfTest.eMMC != SELF_TEST_SUCCESS) && (AbortTimeout--)) {
        // If formatting wait before start Bluetooth task because it will use eMMC
        PMICLed();
        osDelay(1000);
      }
    }
    
    AbortTimeout = (10);
    while ((SelfTest.eMMC != SELF_TEST_SUCCESS) && (AbortTimeout--)) {
      osDelay(1000);
    }
    
    // Start SPP (Bluetooth task)
    osThreadDef(SPP_Thread, FCCTestsThread, osPriorityRealtime, 0, 1024);
    SPPTaskHandle = osThreadCreate(osThread(SPP_Thread), NULL);
  }
  
  if(RamParam.DoSelfTest)
  {
    res=1; 
    
    // Those tests are not presently implemented... 
    SelfTest.NFC = SELF_TEST_SUCCESS;
    SelfTest.SerialBT = SELF_TEST_SUCCESS;
    
    while(SelfTestTimeOut)
    {
      if(SelfTest.eMMC == SELF_TEST_SUCCESS && SelfTest.NFC == SELF_TEST_SUCCESS && SelfTest.PMIC == SELF_TEST_SUCCESS && SelfTest.SerialBT == SELF_TEST_SUCCESS)        
      {
        res = FR_OK;
        RamParam.DoSelfTest = 0;
        FLASH_If_SaveParam();   
        break;
      }
      PMICLed();
      osDelay(100);
      SelfTestTimeOut--;
    }
  }
  
  if(res != FR_OK)
  {
    printf("SelfTest failed.\r\n");
    while (1) 
    {
      PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_ERROR_STATUS;
      PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_ERROR_CHARGE;
      PMICLed();
      osDelay(100);
    }
  }
  
  osDelay(100);
  Settings.FTP_InactivityTimeout = (20 * 60);
  
  PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_OFF;
  PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_SHORT_BLINK;
  
  GetFCCTestNumber();

  // PMIC loop
  while (1) {
    PMICStatMon();
    if (PMICStatus & PMIC_STAT_KEY_FACTORY_RESET) {
      // Start factory reset
      PMICStartFactoryReset();
    }
    if ((FTPActivityTracking != FTPActivity) || (PMICStatus & PMIC_STAT_CHDET)) {
      FTPActivityTracking = FTPActivity;
      OffTimeout = (osKernelSysTick() + (Settings.PMICOffTimeout * 1000));
    }
    if ((osKernelSysTick() >= OffTimeout) || (PMICStatus & PMIC_STAT_LOW_BAT)){
      // Simulate off key
      PMICStatus |= PMIC_STAT_SHORT_PRESS_END;
    }
    // On key management
    if (PMICStatus & PMIC_STAT_KEY_PRESS_MSK) {
      if (PMICStatus & PMIC_STAT_SHORT_PRESS_BEGIN) {
        PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_ON;
      } else if (PMICStatus & PMIC_STAT_SHORT_PRESS_END) {
        if (state == 0) {
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_SHORT_BLINK;
          state = 1;
          FTPAbort = FALSE;
          FTPAborted = FALSE;
        } else {
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_OFF;
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_OFF;
          state = 0;
          FTPAbort = TRUE;
          FTPAborted = FALSE;
          osSemaphoreRelease(FTP_RxFifoEmpty);
          osSemaphoreRelease(FTP_RxDataReady);
          AbortTimeout = (osKernelSysTick() + PMIC_ABORT_TIMEOUT);
          //wait for abort response from FTP server                        
          while ((!FTPAborted) && (osKernelSysTick() < AbortTimeout)) {
            BSP_LED_Toggle(PMIC_LED_STATUS);
            osDelay(100);
          }
          // Lock FTP server
          FTPLocked = TRUE;
          AdvertiseLockStatus(FTPLocked);
          if (FCCTestEnable) {
            FCCTestEnable = 0;
            //SetNextFCCTestNumber();
          }
          // Power Off board (not effective for development setup)
          PMICSetBoardOff();
          // Should not exit from above function
          while(1) {
            osDelay(100);
          }
        }
      } else if (PMICStatus & PMIC_STAT_MEDIUM_PRESS_BEGIN) {
        PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_SHORT_BLINK;
        if (!FCCTestEnable) {
          FCCTestEnable = 1;
        }
      }
      PMICStatus &= ~PMIC_STAT_KEY_PRESS_MSK;
    }
    if (CDCOpenedTrk != CDCOpened) {
      // USB port status has changed
      if (CDCOpened) {
        // Port has been opened
        FTPAbort = TRUE;
        FTPAborted = FALSE;
        osSemaphoreRelease(FTP_RxFifoEmpty);
        osSemaphoreRelease(FTP_RxDataReady);
        AbortTimeout = (osKernelSysTick() + PMIC_ABORT_TIMEOUT);
        //wait for abort response from FTP server                        
        while ((!FTPAborted) && (osKernelSysTick() < AbortTimeout)) {
          osDelay(10);
        }
        // Lock FTP server
        FTPLocked = TRUE;
        FTPAbort = FALSE;
        osDelay(10);

        pWriteABuffer = CDC_WriteABuffer;
        FtpServerReset();
      } else {
        // Port has been closed
        FTPAbort = TRUE;
        FTPAborted = FALSE;
        osSemaphoreRelease(FTP_RxFifoEmpty);
        osSemaphoreRelease(FTP_RxDataReady);
        AbortTimeout = (osKernelSysTick() + PMIC_ABORT_TIMEOUT);
        //wait for abort response from FTP server                        
        while ((!FTPAborted) && (osKernelSysTick() < AbortTimeout)) {
          osDelay(10);
        }
        
        // Lock FTP server
        FTPLocked = TRUE;
        FTPAbort = FALSE;
        osDelay(10);
        pWriteABuffer = BT_WriteABuffer;
      }
      CDCOpenedTrk = CDCOpened;
    }

    if (!FCCTestEnable) {
      if ((state) && (PMICLedCtrl[PMIC_LED_STATUS_INDEX] != LED_ON)) {
        PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_SHORT_BLINK;
      }
    } else {
      // FCC tests
      switch(FCCTestNumber) {
        case 1:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_1_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_OFF;
          break;
        case 2:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_2_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_OFF;
          break;
        case 3:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_3_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_OFF;
          break;
        case 4:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_4_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_OFF;
          break;
        case 5:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_5_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_OFF;
          break;
        case 6:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_1_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_1_BLINK;
          break;
        case 7:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_2_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_1_BLINK;
          break;
        case 8:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_3_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_1_BLINK;
          break;
        case 9:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_4_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_1_BLINK;
          break;
        case 10:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_5_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_1_BLINK;
          break;
        case 11:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_1_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_2_BLINK;
          break;
        case 12:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_2_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_2_BLINK;
          break;
        case 13:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_3_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_2_BLINK;
          break;
        case 14:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_4_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_2_BLINK;
          break;
        case 15:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_5_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_2_BLINK;
          break;
        case 16:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_1_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_3_BLINK;
          break;
        case 17:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_2_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_3_BLINK;
          break;
        case 18:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_3_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_3_BLINK;
          break;
        case 19:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_4_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_3_BLINK;
          break;
        case 20:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_5_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_3_BLINK;
          break;
        case 21:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_1_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_4_BLINK;
          break;
        case 22:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_2_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_4_BLINK;
          break;
        case 23:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_3_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_4_BLINK;
          break;
        case 24:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_4_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_4_BLINK;
          break;
        case 25:
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_5_BLINK;
          PMICLedCtrl[PMIC_LED_STATUS_INDEX] = LED_4_BLINK;
          break;
      }
        
    }
    if (!FCCTestEnable) {
      if (PMICStatus & PMIC_STAT_CHDET) {
        if (PMICStatus & PMIC_STAT_EOC) {
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_ON;
        } else {
          PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_LONG_BLINK;
        }
      } else {
        PMICLedCtrl[PMIC_LED_CHARGE_INDEX] = LED_OFF;
      }
    }
      
    PMICLed();
    osDelay(100);
 
  }
}
      
#endif // FCC_TESTS
