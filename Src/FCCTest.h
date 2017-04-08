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
#include "BTPSKRNL.h"            /* BTPS Kernel Header.                       */
#include "BTVSAPI.h"             /* Vendror Specific header.                  */
#include "HCITRANS.h"


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
// #define VENDOR_BAUD_RATE                                    1843200L

void FCCTestsThread(void const *argument);
int InitializeApplication(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization);
int HAL_ConsoleWrite(UART_HandleTypeDef *UartHandle, int Length, char *Buffer);
int HAL_ConsoleRead(int Length, char *Buffer);
void HAL_ConfigureConsole(UART_HandleTypeDef *UartHandle);

Boolean_t ProcessCommandLine(char *String);


#endif

