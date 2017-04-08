/*****< pmic.h >************************************************************/
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
#ifndef __PMIC_H__
#define __PMIC_H__

#include "stm324xg_eval.h"
#include "stm32f4xx_hal_adc.h"
#include "cmsis_os.h"
#include "main.h"
#include "ftpd\ftpd.h"

#include "BTTypes.h"

#ifdef FIRMWARE
#ifdef FCC_TESTS
#include "FCCTest.h"
#else
#include "SPPTask.h"
#endif
#include "JSMN\jsmn.h"
#endif // FIRMWARE

/* I2C handler declaration */
extern I2C_HandleTypeDef I2cHandle;

#ifndef STM32F407xx // eval board
#define TARGET_BOARD                    1
#endif

//#define PMIC_DEBUG                      1       // Print debug traces

#ifdef TARGET_BOARD
#define eMMC_VCCQ_EN_PORT               GPIOD
#define eMMC_VCCQ_EN_PIN                GPIO_PIN_7
#define eMMC_VCCQ_EN_CLK_ENA()          __HAL_RCC_GPIOD_CLK_ENABLE()

#define VSUP_BT_EN_PORT                 GPIOD
#define VSUP_BT_EN_PIN                  GPIO_PIN_4
#define VSUP_BT_EN_CLK_ENA()            __HAL_RCC_GPIOD_CLK_ENABLE()

#define BT_SHUTD_N_PORT                 GPIOE
#define BT_SHUTD_N_PIN                  GPIO_PIN_12
#define BT_SHUTD_N_CLK_ENA()            __HAL_RCC_GPIOE_CLK_ENABLE()

#define BT_HCI_OE_PORT                  GPIOE
#define BT_HCI_OE_PIN                   GPIO_PIN_9
#define BT_HCI_OE_CLK_ENA()             __HAL_RCC_GPIOE_CLK_ENABLE()

#define ADC_BAT                         ADC3
#define ADC_BAT_CLK_ENABLE()               __HAL_RCC_ADC3_CLK_ENABLE();
#define ADC_BAT_CHANNEL_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOA_CLK_ENABLE()
     
#define ADC_BAT_FORCE_RESET()              __HAL_RCC_ADC_FORCE_RESET()
#define ADC_BAT_RELEASE_RESET()            __HAL_RCC_ADC_RELEASE_RESET()

/* Definition for ADCx Channel Pin */
#define ADC_BAT_CHANNEL_PIN                GPIO_PIN_0
#define ADC_BAT_CHANNEL_GPIO_PORT          GPIOA 

/* Definition for ADCx's Channel */
#define ADC_BAT_CHANNEL                    ADC_CHANNEL_0

#define VBAT_EN_PORT                    GPIOC
#define VBAT_EN_PIN                     GPIO_PIN_13
#define VBAT_EN_CLK_ENA()               __HAL_RCC_GPIOC_CLK_ENABLE()


#else // TARGET_BOARD
#define BT_SHUTD_N_PORT                 GPIOD
#define BT_SHUTD_N_PIN                  GPIO_PIN_3
#define BT_SHUTD_N_CLK_ENA()            __HAL_RCC_GPIOD_CLK_ENABLE()
#endif // TARGET_BOARD

#define LED_OFF                         0x0000
#define LED_ON                          0x03ff
#define LED_SHORT_BLINK                 0x0001
#define LED_DOUBLE_BLINK                0x0005
#define LED_TRIPLE_BLINK                0x0015
#define LED_LONG_BLINK                  0x001f
#define LED_SELF_TEST                   0x0144
#define LED_ERROR_STATUS                0x001F
#define LED_ERROR_CHARGE                0x03E0

#define LED_1_BLINK                     0x0001
#define LED_2_BLINK                     0x0005
#define LED_3_BLINK                     0x0015
#define LED_4_BLINK                     0x0055
#define LED_5_BLINK                     0x0155

#define LED_BLINK_NUMBER                10


#define PMIC_LED_NUMBER                 2

#ifdef STM32F415xx
#define PMIC_LED_CHARGE                 LED_RED
#define PMIC_LED_STATUS                 LED_YELLOW
#else
#if 1
#define PMIC_LED_CHARGE                 LED_RED
#define PMIC_LED_STATUS                 LED_YELLOW
#else
#define PMIC_LED_CHARGE                 LED3
#define PMIC_LED_STATUS                 LED1
#endif
#endif // STM32F415xx

#define PMIC_LED_CHARGE_INDEX           0
#define PMIC_LED_STATUS_INDEX           1

#define FTP_ABORT_TIMEOUT              2000    // Timeout waiting for FTP abort (in ms)
#define FTP_INACTIVITY_TO_DEFAULT      20       // Timeout to detect FTP inactivity (in seconds)
#define FTP_INACTIVITY_TO_MIN          10
#define FTP_INACTIVITY_TO_MAX          86400
#define FTP_AUTHENTICATION_TO_DEFAULT  0       // Timeout to detect FTP inactivity (in seconds)
#define FTP_AUTHENTICATION_TO_MIN      0
#define FTP_AUTHENTICATION_TO_MAX      86400


#define PMIC_SHORT_PRESS_COUNT          1       // Count to detect a short press, each count represent 100ms
#define PMIC_MEDIUM_PRESS_COUNT         20      // Count to detect a medium press, each count represent 100ms
#define PMIC_FACTORY_RESET_COUNT        80      // Count to detect a factory reset (Charger must be present)

#define PMIC_STAT_SHORT_PRESS_BEGIN     0x01
#define PMIC_STAT_SHORT_PRESS_END       0x02
#define PMIC_STAT_MEDIUM_PRESS_BEGIN    0x04
#define PMIC_STAT_MEDIUM_PRESS_END      0x08
#define PMIC_STAT_KEY_PRESS_MSK         0x0f
#define PMIC_STAT_KEY_FACTORY_RESET     0x10
#define PMIC_STAT_LOW_BAT               0x20
#define PMIC_STAT_EOC                   0x40
#define PMIC_STAT_CHDET                 0x80

#define PMIC_I2C_ADDR                   0x80
#define PMIC_I2C_TIMEOUT                100

#define PMIC_I2C_SD1_VOLT               0x01
#define PMIC_I2C_SD1_3_0V               0x78
#define PMIC_I2C_SD1_1_8V               0x50

#define PMIC_I2C_LDO1_VOLT              0x02
#define PMIC_I2C_LDO1_1_8V              0xcc
#define PMIC_I2C_LDO1_OFF               0x00

#define PMIC_I2C_GPIO2_CTRL             0x0a
#define PMIC_I2C_GPIO2_CTRL_OUT_INT     0x11

#define PMIC_I2C_LDO2_VOLT              0x03
#define PMIC_I2C_LDO2_2_8V              0xe0

#define PMIC_I2C_GPIO1_CONTROL          0x09
#define PMIC_I2C_GPIO2_CONTROL          0x0A
#define PMIC_I2C_MODE_OUTPUT            (0x01<<4)
#define PMIC_I2C_IOSF_INTERRUPT_OUTPUT  0x01


#define PMIC_I2C_SD1_CTRL1              0x30
#define PMIC_I2C_SD1_CTRL1_SD1_FSEL     0x10

#define PMIC_I2C_BAT_VOLT_MON           0x32
#define PMIC_I2C_BAT_VOLT_MON_F2_9_R3_1 0x0a
#define PMIC_I2C_BAT_VOLT_MON_F3_1_R3_3 0x14

#define PMIC_I2C_RST_CTRL               0x36
#define PMIC_I2C_RST_CTRL_ON_INPUT      0x04
#define PMIC_I2C_RST_CTRL_PWR_OFF       0x02

#define PMIC_I2C_INT1_MASK              0x74

#define PMIC_I2C_INT1_STAT              0x77
#define PMIC_I2C_INT1_CHARGE_DETECT     0x10
#define PMIC_I2C_INT1_STAT_ONKEY        0x20
#define PMIC_I2C_INT1_STAT_LOBAT        0x80

#define PMIC_I2C_CHG_CTRL               0x80
#define PMIC_I2C_CHG_CTRL_SETUP         0xb8    // low current range, bat_charging_enable, usb_current: 470ma

#define PMIC_I2C_CHG_VOLTAGE_CTRL       0x81
#define PMIC_I2C_CHG_VCHOFF_SETUP       0x5A    

#define PMIC_I2C_CHG_CURR_CTRL          0x82
#define PMIC_I2C_CHG_CURR_CTRL_SETUP    0x39    // 47ma trickle, 118ma constant current mode

#define PMIC_I2C_CHG_SUP                0x85
#ifdef TARGET_BOARD
#define PMIC_I2C_CHG_SUP_SETUP          0xc7    // ntc_high_on:1, ntc_low_on:1, ntc_10k:0, ntc_mode:1, ntc_input:1, ntc_beta:3
#else
#define PMIC_I2C_CHG_SUP_SETUP          0x00    // ntc_high_on:0, ntc_low_on:0, ntc_10k:0, ntc_mode:0, ntc_input:0, ntc_beta:0
#endif
#define PMIC_I2C_CHG_STAT1              0x86
#define PMIC_I2C_CHG_STAT1_EOC          0x10

#define PMIC_I2C_CHG_STAT2              0x87
#define PMIC_I2C_CHG_STAT2_CHDET        0x04

#define PMIC_I2C_ASIC_ID1               0x90
#define PMIC_I2C_ASIC_ID1_VAL           0x11

// Battery level. Determined by this formula ((Batt_Volt/7.5V) * 4095)
// Example for 3.6V -> ((3.6 / 7.5) * 4095) = 1965.6 -> 1966
#define ADC_BAT_LEVEL_100               1966    // >= 3.6V
#define ADC_BAT_LEVEL_75                1911    // >= 3.5V
#define ADC_BAT_LEVEL_50                1856    // >= 3.4V
#define ADC_BAT_LEVEL_25                1802    // >= 3.3V (low batt) 3.1V (power off)

// Minimum battery level to allow firmware updaes without USB attached
#define ADC_MIN_BAT_FIRMWWARE_UPDATE    50

#define BLE_CONNECTION_TIMEOUT          10    // Allow BLE client to remain connected to card for up to 10 seconds

static uint16_t PMICLedCtrl[PMIC_LED_NUMBER];

void PMICThread(void const *argument);
int PMICPreInit(void);
int PMICInit(void);
int PMICSetBoardOff(void);
int PMICStartFactoryReset(void);
void PMIC_2_8V(uint8_t _State);
int ADC_Bat_GetVal(int * val);
int ADC_Bat_GetPercent();
HAL_StatusTypeDef RTC_InitTime(void);
HAL_StatusTypeDef RTC_GetElapsedTime(uint32_t * seconds);
void get_date(RTC_DateTypeDef * sDate, RTC_TimeTypeDef * sTime);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

void StartBLEConnetionTimer(unsigned int stackID, BD_ADDR_t bdAddr);
void StopBLEConnetionTimer(void);

#endif // __PMIC_H__