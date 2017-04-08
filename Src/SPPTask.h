/*****< sppdemo.h >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SPPDEMO - Simple embedded application using SPP Profile.                  */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/24/11  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __SPPDEMO_H__
#define __SPPDEMO_H__

#include "SS1BTPS.h"             /* Main SS1 Bluetooth Stack Header.          */
#include "SS1BTGAT.h"            /* Main SS1 GATT Header.                     */
#include "SS1BTGAP.h"            /* Main SS1 GAP Service Header.              */
#include "SS1BTTPS.h"            /* Main SS1 TPS Service Header.              */
#include "SS1BTIAS.h"            /* Main SS1 IAS Service Header.              */
#include "SS1BTLLS.h"            /* Main SS1 LLS Service Header.              */
#include "SS1BTDIS.h"            /* Main SS1 DIS Service Header.              */
#include "SS1BTBAS.h"            /* Main SS1 BAS Service Header.              */
#include "SS1BTHIDS.h"           /* Main SS1 HIDS Service Header.             */
#include "PWVTypes.h"            /* Custom service, no API                    */
#include "BTPSKRNL.h"            /* BTPS Kernel Header.                       */
#include "BTVSAPI.h"             /* Vendror Specific header.                  */
#include "HCITRANS.h"



   /* The following type definition represents the container type which */
   /* holds the mapping between Bluetooth devices (based on the BD_ADDR)*/
   /* and the Link Key (BD_ADDR <-> Link Key Mapping).                  */
typedef struct _tagLinkKeyInfo_t
{
   BD_ADDR_t  BD_ADDR;
   Link_Key_t LinkKey;
} LinkKeyInfo_t;



#define Display(_x)                                do { BTPS_OutputMessage _x; } while(0)
   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define APPLICATION_ERROR_INVALID_PARAMETERS       (-1000)
#define APPLICATION_ERROR_UNABLE_TO_OPEN_STACK     (-1001)


 /* Bluetooth Protocol Demo Application Major Version Release Number.  */
#define DEMO_APPLICATION_MAJOR_VERSION_NUMBER      0

 /* Bluetooth Protocol Demo Application Minor Version Release Number.  */
#define DEMO_APPLICATION_MINOR_VERSION_NUMBER      3

   /* Constants used to convert numeric constants to string constants   */
   /* (used in MACRO's below).                                          */
#define VERSION_NUMBER_TO_STRING(_x)               #_x
#define VERSION_CONSTANT_TO_STRING(_y)             VERSION_NUMBER_TO_STRING(_y)

   /*  Demo Application Version String of the form "a.b"                */      
   /* where:                                                            */
   /*    a - DEMO_APPLICATION_MAJOR_VERSION_NUMBER                      */
   /*    b - DEMO_APPLICATION_MINOR_VERSION_NUMBER                      */

#define DEMO_APPLICATION_VERSION_STRING            VERSION_CONSTANT_TO_STRING(DEMO_APPLICATION_MAJOR_VERSION_NUMBER) "." VERSION_CONSTANT_TO_STRING(DEMO_APPLICATION_MINOR_VERSION_NUMBER)

 /* Define the baud rate that will be used for communication with the */
   /* Bluetooth chip.  The value of this rate will be configured in the */
   /* Bluetooth transport.                                              */
#define VENDOR_BAUD_RATE                                    921600L

#define LOWPOWER_ENABLE

 /* Define the events for the SPP thread                                */
#define SPP_EVT_NONE                    0
#define SPP_EVT_INIT_START              1
#define SPP_EVT_BUTTON_WAKEUP           2
#define SPP_EVT_LE_CONNECT              3
#define SPP_EVT_LE_PAIR_COMPLETE        4
#define SPP_EVT_PLUG_IN                 5
#define SPP_EVT_BUTTON_MEDIUM_PRESS     6
#define SPP_EVT_HIDS_CONNECT            7
#define SPP_EVT_SM_TIMEOUT              8
#define SPP_EVT_EDR_PAIR_COMPLETE       9
#define SPP_EVT_ENABLE_EDR              10
#define SPP_EVT_LE_DISCONNECT           11
#define SPP_EVT_SPP_DISCONNECT          12
#define SPP_EVT_GATT_BUFFER_EMPTY       13
#define SPP_EVT_LE_SEND_FILE            14
#define SPP_EVT_GATT_BUFFER_FULL        15

void SPPThread(void const *argument);
int InitializeApplication(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization);
int HAL_ConsoleWrite(UART_HandleTypeDef *UartHandle, int Length, char *Buffer);
int HAL_ConsoleRead(int Length, char *Buffer);
void HAL_ConfigureConsole(UART_HandleTypeDef *UartHandle);
int CloseBluetooth(void);
void ResetBluetoothKeys(void);
int DeleteLinkKey(BD_ADDR_t BD_ADDR);
int AddLinkedKey(LinkKeyInfo_t * pKeyInfo);
int DisconnectLE(void);
void SendKeystrokes(char *Keys);
void Set_SPP_Event(uint8_t event);
int8_t SPP_Sleep(void);
uint8_t SPP_Pairing_Mode(void);
int GetLinkedKey(BD_ADDR_t BTAdd, LinkKeyInfo_t* pKey);
Boolean_t ProcessCommandLine(char *String);
LinkKeyInfo_t *ReturnAllLinkedKey(int *len);

#endif

