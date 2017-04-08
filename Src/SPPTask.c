/* spptask.c
  Based on sppdemo from Stonestreet One
*/

/*****< sppdemo.c >************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*      Copyright 2015 Texas Instruments Incorporated.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SPPDEMO - Simple embedded application using SPP Profile.                  */
/*                                                         f                   */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   01/24/11  T. Cook        Initial creation.                               */
/*   11/24/14  R. Malovany    Update.                                         */
/*   03/03/15  D. Horowitz    Adding Demo Application version.                */
/******************************************************************************/

#include "Main.h"                /* Application Interface Abstraction.        */
#include <stdio.h>               /* Included for sscanf.                      */
#include <ctype.h>               /* Included for isalnum.                     */
#include "pmic.h"
#include "slog.h"

#ifndef FCC_TESTS        // Do not compile for FCC tests

extern int HandleASuccessfulRead(char *lpBuf, DWORD dwRead );
extern int (*pWriteABuffer)(const char*, DWORD);
extern void FtpServerReset(void);

extern FRESULT FindValidTemplate(void);

#ifdef CONSOLE_SUPPORT
extern UART_HandleTypeDef UartHandle;
#endif

#define MAX_SUPPORTED_COMMANDS                     (40)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_NUM_OF_PARAMETERS                      (25)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

#define MAX_INQUIRY_RESULTS                        (20)  /* Denotes the max   */
                                                         /* number of inquiry */
                                                         /* results.          */

#define DEFAULT_IO_CAPABILITY          (icDisplayYesNo)  /* Denotes the       */
                                                         /* default I/O       */
                                                         /* Capability that is*/
                                                         /* used with Secure  */
                                                         /* Simple Pairing.   */

#define DEFAULT_MITM_PROTECTION                  (TRUE)  /* Denotes the       */
                                                         /* default value used*/
                                                         /* for Man in the    */
                                                         /* Middle (MITM)     */
                                                         /* protection used   */
                                                         /* with Secure Simple*/
                                                         /* Pairing.          */
#define DEFAULT_LE_IO_CAPABILITY   (licNoInputNoOutput)  /* Denotes the       */
                                                         /* default I/O       */
                                                         /* Capability that is*/
                                                         /* used with LE      */
                                                         /* Pairing.          */

#define DEFAULT_LE_MITM_PROTECTION              (TRUE)   /* Denotes the       */
                                                         /* default value used*/
                                                         /* for Man in the    */
                                                         /* Middle (MITM)     */
                                                         /* protection used   */
                                                         /* with LE Pairing.  */
#define DEFAULT_ALERT_LEVEL                        (0)
#define MAX_LE_CONNECTIONS                          (1)  /* Denotes the max   */
                                                         /* number of LE      */
                                                         /* connections that  */
                                                         /* are allowed at    */
                                                         /* the same time.    */
#define FILEPATH_LE_MAX_LENGTH                      (30) /* Maximum string    */
                                                         /* length for le file*/
                                                         /* transfer path     */

   /* The following MACRO is used to convert an ASCII character into the*/
   /* equivalent decimal value.  The MACRO converts lower case          */
   /* characters to upper case before the conversion.                   */
#define ToInt(_x)                                  (((_x) > 0x39)?(((_x) & ~0x20)-0x37):((_x)-0x30))

#if 0
const uint8_t TEST_DATA[517] =
{
  0x31, 0x32, 0x33, 0x0d, 0x0a,
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
  0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x0d, 0x0A, 
};
#else
#define TEST_DATA              "This is a test string."  /* Denotes the data  */
                                                         /* that is sent via  */
                                                         /* SPP when calling  */
                                                         /* SPP_Data_Write(). */
#endif
#define NO_COMMAND_ERROR                           (-1)  /* Denotes that no   */
                                                         /* command was       */
                                                         /* specified to the  */
                                                         /* parser.           */

#define INVALID_COMMAND_ERROR                      (-2)  /* Denotes that the  */
                                                         /* Command does not  */
                                                         /* exist for         */
                                                         /* processing.       */

#define EXIT_CODE                                  (-3)  /* Denotes that the  */
                                                         /* Command specified */
                                                         /* was the Exit      */
                                                         /* Command.          */

#define FUNCTION_ERROR                             (-4)  /* Denotes that an   */
                                                         /* error occurred in */
                                                         /* execution of the  */
                                                         /* Command Function. */

#define TO_MANY_PARAMS                             (-5)  /* Denotes that there*/
                                                         /* are more          */
                                                         /* parameters then   */
                                                         /* will fit in the   */
                                                         /* UserCommand.      */

#define INVALID_PARAMETERS_ERROR                   (-6)  /* Denotes that an   */
                                                         /* error occurred due*/
                                                         /* to the fact that  */
                                                         /* one or more of the*/
                                                         /* required          */
                                                         /* parameters were   */
                                                         /* invalid.          */

#define UNABLE_TO_INITIALIZE_STACK                 (-7)  /* Denotes that an   */
                                                         /* error occurred    */
                                                         /* while Initializing*/
                                                         /* the Bluetooth     */
                                                         /* Protocol Stack.   */

#define INVALID_STACK_ID_ERROR                     (-8)  /* Denotes that an   */
                                                         /* occurred due to   */
                                                         /* attempted         */
                                                         /* execution of a    */
                                                         /* Command when a    */
                                                         /* Bluetooth Protocol*/
                                                         /* Stack has not been*/
                                                         /* opened.           */

#define UNABLE_TO_REGISTER_SERVER                  (-9)  /* Denotes that an   */
                                                         /* error occurred    */
                                                         /* when trying to    */
                                                         /* create a Serial   */
                                                         /* Port Server.      */

#define EXIT_MODE                                  (-10) /* Flags exit from   */
                                                         /* any Mode.         */

//#define PWV_DATA_BUFFER_LENGTH  (BTPS_CONFIGURATION_GATT_MAXIMUM_SUPPORTED_MTU_SIZE)
#define PWV_DATA_BUFFER_LENGTH  (350)

                                                         /* Defines the length*/
                                                         /* of a SPPLE Data   */
                                                         /* Buffer.           */

   /* Determine the Name we will use for this compilation.              */
#define APP_DEMO_NAME                              "CYBERGATE"
#define APP_CYBERGATE_LE                           "CYBERGATELE"

   /* The following define the PnP values that are assigned in the PnP  */
   /* ID characteristic of the Device Information Service.              */
#define PNP_ID_VENDOR_ID_STONESTREET_ONE                 0x005E
#define PNP_ID_PRODUCT_ID                                0xDEAD
#define PNP_ID_PRODUCT_VERSION                           0xBEEF

   /* The following represent the possible values of UI_Mode variable.  */
#define UI_MODE_IS_PRODTOOLS   (3)
#define UI_MODE_IS_CLIENT      (2)
#define UI_MODE_IS_SERVER      (1)
#define UI_MODE_SELECT         (0)
#define UI_MODE_IS_INVALID     (-1)

   /* Following converts a Sniff Parameter in Milliseconds to frames.   */
#define MILLISECONDS_TO_BASEBAND_SLOTS(_x)   ((_x) / (0.625))

   /* Following represents operating states of the SPP thread loop      */
#define SPP_STATE_INIT                          0
#define SPP_STATE_INIT_PAIRING                  1
#define SPP_STATE_IDLE                          2
#define SPP_STATE_ADVERTISE                     3
#define SPP_STATE_PAIRING_MODE                  4
#define SPP_STATE_EDR_ENABLED                   5
#define SPP_STATE_LE_FILE_TRANSFER_HOLD         6
#define SPP_STATE_LE_FILE_TRANSFER_ACTIVE       7
#define SPP_STATE_LE_FILE_TRANSFER_START        8
#define SPP_STATE_PM_WAIT_LE                    9
#define SPP_STATE_PM_WAIT_EDR                   10

   /* Following represents defines for custom password vault service    */

   /* The following defines the PWV service that is registered with     */
   /* the GATT_Register_Service function call.                          */
   /* * NOTE * This array will be registered with GATT in the call to   */
   /*          GATT_Register_Service.                                   */
   /* The SPPLE Service Declaration UUID.                               */

static BTPSCONST GATT_Primary_Service_128_Entry_t PWV_Service_UUID =
{
   PWV_SERVICE_BLUETOOTH_UUID_CONSTANT
};

   /* The PWV Control Point Characteristic Declaration.                  */
static BTPSCONST GATT_Characteristic_Declaration_128_Entry_t PWV_Control_Point_Declaration =
{
   (GATT_CHARACTERISTIC_PROPERTIES_READ|GATT_CHARACTERISTIC_PROPERTIES_NOTIFY|GATT_CHARACTERISTIC_PROPERTIES_WRITE),
   PWV_CONTROL_POINT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT
};

   /* The PWV Control Point Characteristic Value.                        */
static BTPSCONST GATT_Characteristic_Value_128_Entry_t PWV_Control_Point_Value =
{
   PWV_CONTROL_POINT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT,
   0,
   NULL
};

   /* Client Characteristic Configuration Descriptor.                   */
static GATT_Characteristic_Descriptor_16_Entry_t Client_Characteristic_Configuration =
{
   GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_BLUETOOTH_UUID_CONSTANT,
   GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_LENGTH,
   NULL
};

   /* The PWV File Write Characteristic Declaration.                  */
static BTPSCONST GATT_Characteristic_Declaration_128_Entry_t PWV_File_Write_Declaration =
{
   (GATT_CHARACTERISTIC_PROPERTIES_WRITE),
   PWV_FILE_WRITE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT
};

   /* The PWV File Write Characteristic Value.                        */
static BTPSCONST GATT_Characteristic_Value_128_Entry_t PWV_File_Write_Value =
{
   PWV_FILE_WRITE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT,
   0,
   NULL
};

BTPSCONST GATT_Service_Attribute_Entry_t PWV_Service[] =
{
   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetPrimaryService128,            (Byte_t *)&PWV_Service_UUID},                    //0
   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration128, (Byte_t *)&PWV_Control_Point_Declaration},       //1
   {GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, aetCharacteristicValue128,       (Byte_t *)&PWV_Control_Point_Value},             //2
   {GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, aetCharacteristicDescriptor16,   (Byte_t *)&Client_Characteristic_Configuration}, //3
   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration128, (Byte_t *)&PWV_File_Write_Declaration},          //4
   {GATT_ATTRIBUTE_FLAGS_WRITABLE,          aetCharacteristicValue128,       (Byte_t *)&PWV_File_Write_Value},                //5
};
#define PWV_SERVICE_ATTRIBUTE_COUNT                    (sizeof(PWV_Service)/sizeof(GATT_Service_Attribute_Entry_t))

#define PWV_CONTROL_POINT_CHARACTERISTIC_ATTRIBUTE_OFFSET               2
#define PWV_CONTROL_POINT_CHARACTERISTIC_CCD_ATTRIBUTE_OFFSET           3
#define PWV_FILE_WRITE_CHARACTERISTIC_ATTRIBUTE_OFFSET                  5

#define PWV_ERROR_FILE_OPEN_FAIL                                        1
#define PWV_ERROR_FILE_NOT_OPEN                                         2
#define PWV_ERROR_FILE_CLOSE_FAIL                                       3
#define PWV_ERROR_FILE_WRITE_FAIL                                       4

#define PWV_CMD_ENABLE_EDR                                              1
#define PWV_CMD_SEND_FILE                                               2
#define PWV_CMD_OPEN_FILE                                               3
#define PWV_CMD_CLOSE_FILE                                              4

   /* The following type definition represents the structure which holds*/
   /* all information about the parameter, in particular the parameter  */
   /* as a string and the parameter as an unsigned int.                 */
typedef struct _tagParameter_t
{
   char     *strParam;
   SDWord_t  intParam;
} Parameter_t;

   /* The following type definition represents the structure which holds*/
   /* a list of parameters that are to be associated with a command The */
   /* NumberofParameters variable holds the value of the number of      */
   /* parameters in the list.                                           */
typedef struct _tagParameterList_t
{
   int         NumberofParameters;
   Parameter_t Params[MAX_NUM_OF_PARAMETERS];
} ParameterList_t;

   /* The following type definition represents the structure which holds*/
   /* the command and parameters to be executed.                        */
typedef struct _tagUserCommand_t
{
   char            *Command;
   ParameterList_t  Parameters;
} UserCommand_t;

   /* The following type definition represents the generic function     */
   /* pointer to be used by all commands that can be executed by the    */
   /* test program.                                                     */
typedef int (*CommandFunction_t)(ParameterList_t *TempParam);

   /* The following type definition represents the structure which holds*/
   /* information used in the interpretation and execution of Commands. */
typedef struct _tagCommandTable_t
{
   char              *CommandName;
   CommandFunction_t  CommandFunction;
} CommandTable_t;

   /* User to represent a structure to hold a BD_ADDR return from       */
   /* BD_ADDRToStr.                                                     */
typedef char BoardStr_t[15];

   /* The following structure holds status information about a send     */
   /* process.                                                          */
typedef struct _tagSend_Info_t
{
   DWord_t PendingCount;
   DWord_t BytesToSend;
   DWord_t BytesSent;
} Send_Info_t;

   /* The following structure is used to hold information of the 		*/
   /* FIRMWARE version.                                                 */
typedef struct FW_Version_t
{
   Byte_t StatusResult; 
   Byte_t HCI_VersionResult;
   Word_t HCI_RevisionResult;
   Byte_t LMP_VersionResult; 
   Word_t Manufacturer_NameResult; 
   Word_t LMP_SubversionResult;
} FW_Version;

   /* Structure used to hold all of the GAP LE Parameters.              */
typedef struct _tagGAPLE_Parameters_t
{
   GAP_LE_Connectability_Mode_t ConnectableMode;
   GAP_Discoverability_Mode_t   DiscoverabilityMode;
   GAP_LE_IO_Capability_t       IOCapability;
   Boolean_t                    MITMProtection;
   Boolean_t                    OOBDataPresent;
} GAPLE_Parameters_t;

   /* The following structure holds information on known Device         */
   /* Appearance Values.                                                */
typedef struct _tagGAPS_Device_Appearance_Mapping_t
{
   Word_t  Appearance;
   char   *String;
} GAPS_Device_Appearance_Mapping_t;

                        /* The Encryption Root Key should be generated  */
                        /* in such a way as to guarantee 128 bits of    */
                        /* entropy.                                     */
static BTPSCONST Encryption_Key_t ER = {0x28, 0xBA, 0xE1, 0x37, 0x13, 0xB2, 0x20, 0x45, 0x16, 0xB2, 0x19, 0xD0, 0x80, 0xEE, 0x4A, 0x51};

                        /* The Identity Root Key should be generated    */
                        /* in such a way as to guarantee 128 bits of    */
                        /* entropy.                                     */
static BTPSCONST Encryption_Key_t IR = {0x41, 0x09, 0xA0, 0x88, 0x09, 0x6B, 0x70, 0xC0, 0x95, 0x23, 0x3C, 0x8C, 0x48, 0xFC, 0xC9, 0xFE};

                        /* The following keys can be regerenated on the */
                        /* fly using the constant IR and ER keys and    */
                        /* are used globally, for all devices.          */
static Encryption_Key_t DHK;
static Encryption_Key_t IRK;

                        /* The following array represents the IEEE      */
                        /* 11073-20601 Regulatory Certification Data    */
                        /* List.                                        */
static Byte_t IEEE_DATA_LIST[] = {0x12, 0x34, 0x56, 0x78};

   /* The following structure represents the information we will store  */
   /* on a Discovered GAP Service.                                      */
typedef struct _tagGAPS_Client_Info_t
{
   Word_t DeviceNameHandle;
   Word_t DeviceAppearanceHandle;
} GAPS_Client_Info_t;

   /* The following structure is a container for information on         */
   /* connected devices.                                                */
typedef struct _tagConnectionInfo_t
{
   unsigned char         Flags;
   unsigned int          ConnectionID;
   unsigned int          PasskeyDigits;
   unsigned long         Passkey;
   GAP_LE_Address_Type_t AddressType;
   BD_ADDR_t             BD_ADDR;
   unsigned int          SecurityTimerID;
} ConnectionInfo_t;

#define CONNECTION_INFO_FLAGS_CONNECTION_ENCRYPTED          0x01
#define CONNECTION_INFO_FLAGS_CONNECTION_AWAITING_PASSKEY   0x02
#define CONNECTION_INFO_FLAGS_CONNECTION_VALID              0x80

   /* The following structure is used to hold all of the application    */
   /* state information.                                                */
typedef struct _tagApplicationStateInfo_t
{
   unsigned int         BluetoothStackID;
   Byte_t               Flags;
   unsigned int         GAPSInstanceID;
   unsigned int         DISInstanceID;
   unsigned int         BASInstanceID;
   unsigned int         BatteryLevel;
   ConnectionInfo_t     LEConnectionInfo;
} ApplicationStateInfo_t;

#define APPLICATION_STATE_INFO_FLAGS_LE_CONNECTED           0x01
#define APPLICATION_STATE_INFO_FLAGS_CAPS_LOCKED            0x02

/* The following structure for a Master is used to hold a list of    */
   /* information on all paired devices. For slave we will not use this */
   /* structure.                                                        */
typedef struct _tagDeviceInfo_t
{
   Byte_t                    Flags;
   Byte_t                    EncryptionKeySize;
   GAP_LE_Address_Type_t     ConnectionAddressType;
   BD_ADDR_t                 ConnectionBD_ADDR;
   Long_Term_Key_t           LTK;
   Random_Number_t           Rand;
   Word_t                    EDIV;
   GAPS_Client_Info_t        GAPSClientInfo;
   BAS_Server_Information_t  BASServerInformation;
   LLS_Client_Information_t  LLS_ClientInfo;
   IAS_Client_Information_t  IAS_ClientInfo;
   TPS_Client_Information_t  TPS_ClientInfo;
   
   // HIDS specific info
   Encryption_Key_t          IRK;
   
   // PWV specific info
   PWV_Server_Info_t         ServerInfo;
   
   struct _tagDeviceInfo_t  *NextDeviceInfoPtr;
} DeviceInfo_t;

   /* Defines the bit mask flags that may be set in the DeviceInfo_t    */
   /* structure.                                                        */
#define DEVICE_INFO_FLAGS_LTK_VALID                         0x01
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING     0x02
#define DEVICE_INFO_FLAGS_LLS_SERVICE_DISCOVERY_COMPLETE    0x04
#define DEVICE_INFO_FLAGS_IAS_SERVICE_DISCOVERY_COMPLETE    0x08
#define DEVICE_INFO_FLAGS_TPS_SERVICE_DISCOVERY_COMPLETE    0x10

   /* Defines the bitmask flags that may be set in the Flags member of  */
   /* the DeviceInfo_t structure.                                       */
#define DEVICE_INFO_FLAGS_IRK_VALID                      0x01

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static ApplicationStateInfo_t ApplicationStateInfo; /* Container for all of the        */
                                                    /* Application State Information.  */

int                        UI_Mode;                 /* Holds the UI Mode.              */

static Byte_t              PWVBuffer[PWV_DATA_BUFFER_LENGTH+1];  /* Buffer that is */
                                                    /* used for Sending/Receiving      */
                                                    /* SPPLE Service Data.             */
static uint16_t            PWVBufferIndex;

static unsigned int        BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static int                 SerialPortID;            /* Variable which contains the     */
                                                    /* Handle of the most recent       */
                                                    /* SPP Port that was opened.       */

static int                 ServerPortID;            /* Variable which contains the     */
                                                    /* Handle of the SPP Server Port   */
                                                    /* that was opened.                */

static Word_t              Connection_Handle;       /* Holds the Connection Handle of  */
                                                    /* the most recent SPP Connection. */


_Pragma("diag_suppress=Pe550")
static Boolean_t           Connected;               /* Variable which flags whether or */
                                                    /* not there is currently an active*/
                                                    /* connection.                     */
static unsigned int        NumberofValidResponses;  /* Variable which holds the number */
                                                    /* of valid inquiry results within */
                                                    /* the inquiry results array.      */
_Pragma("diag_default=Pe550")

static DWord_t             SPPServerSDPHandle;      /* Variable used to hold the Serial*/
                                                    /* Port Service Record of the      */
                                                    /* Serial Port Server SDP Service  */
                                                    /* Record.                         */


static BD_ADDR_t           CurrentRemoteBD_ADDR;    /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */

static GAP_IO_Capability_t IOCapability;            /* Variable which holds the        */
                                                    /* current I/O Capabilities that   */
                                                    /* are to be used for Secure Simple*/
                                                    /* Pairing.                        */

static Boolean_t           OOBSupport;              /* Variable which flags whether    */
                                                    /* or not Out of Band Secure Simple*/
                                                    /* Pairing exchange is supported.  */

static Boolean_t           MITMProtection;          /* Variable which flags whether or */
                                                    /* not Man in the Middle (MITM)    */
                                                    /* protection is to be requested   */
                                                    /* during a Secure Simple Pairing  */
                                                    /* procedure.                      */

static BD_ADDR_t           SelectedBD_ADDR;         /* Holds address of selected Device*/

static BD_ADDR_t           NullADDR;                /* Holds a NULL BD_ADDR for comp.  */
                                                    /* purposes.                       */

static Boolean_t           LoopbackActive;          /* Variable which flags whether or */
                                                    /* not the application is currently*/
                                                    /* operating in Loopback Mode      */
                                                    /* (TRUE) or not (FALSE).          */

static Boolean_t           DisplayRawData;          /* Variable which flags whether or */
                                                    /* not the application is to       */
                                                    /* simply display the Raw Data     */
                                                    /* when it is received (when not   */
                                                    /* operating in Loopback Mode).    */

static Boolean_t           AutomaticReadActive;     /* Variable which flags whether or */
                                                    /* not the application is to       */
                                                    /* automatically read all data     */
                                                    /* as it is received.              */


static Send_Info_t         SendInfo;                /* Variable that contains          */
                                                    /* information about a data        */
                                                    /* transfer process.               */



static GAPLE_Parameters_t  LE_Parameters;           /* Holds GAP Parameters like       */
                                                    /* Discoverability, Connectability */
                                                    /* Modes.                          */
static Boolean_t           LocalDeviceIsMaster;     /* Boolean that tells if the local */
                                                    /* device is the master of the     */
                                                    /* current connection.             */
static DeviceInfo_t       *DeviceInfoList;          /* Holds the list head for the     */
                                                    /* device info list.               */
static unsigned int        LLSInstanceID;           /* The following holds the LLS     */
                                                    /* Instance ID that is returned    */
                                                    /* from LLS_Initialize_Service().  */
static unsigned int        TPSInstanceID;           /* The following holds the TPS     */
                                                    /* Instance ID that is returned    */
                                                    /* from TPS_Initialize_Service().  */

static unsigned int        IASInstanceID;           /* The following holds the IAS     */
                                                    /* Instance ID that is returned    */
                                                    /* from IAS_Initialize_Service().  */
static unsigned int        PasswordVaultServiceID;  /* The following holds the PWV     */
                                                    /* Service ID that is returned from*/
                                                    /* GATT_Register_Service().        */
static unsigned int        GAPSInstanceID;          /* Holds the Instance ID for the   */
                                                    /* GAP Service.                    */
static unsigned int        DISInstanceID;           /* Holds the Instance ID for the   */
                                                    /* DIS Service.                    */
static unsigned int        BASInstanceID;           /* Holds the Instance ID for the   */
                                                    /* BAS Service.                    */
static unsigned int        ConnectionID;            /* Holds the Connection ID of the  */
                                                    /* currently connected device.     */
static IAS_Control_Point_Command_t   AlertLevelControlPointCommand;/* Variable which is*/
                                                    /* used to hold the Alert Level    */
                                                    /* control point command set by    */
                                                    /* remote device.                  */

static BD_ADDR_t           ConnectionBD_ADDR;       /* Holds the BD_ADDR of the        */
                                                    /* currently connected device.     */
static GAP_Encryption_Mode_t GAPEncryptionMode;    /* Holds the encryption mode of     */
                                                   /* currently connected device.      */
static int                 AdvertisingStatus = FALSE;      /* Current Advertising status */

   /* Variables which contain information used by the loopback          */
   /* functionality of this test application.                           */
static unsigned int        BufferLength;

static unsigned char       Buffer[530];

static Boolean_t           BufferFull;

static Boolean_t           FTPModeActive = TRUE;

int BT_LinkedDeviceNb = 0;

static uint8_t SPP_event = SPP_EVT_NONE;
static uint8_t SPP_state = SPP_STATE_INIT;
static uint8_t SPP_state_timer_id = 0;
static uint8_t pairing_mode = 0;
static uint8_t le_transfer_filepath[FILEPATH_LE_MAX_LENGTH];
static uint16_t le_transfer_index = 0;
static FIL fp_download;
static DeviceInfo_t *le_transfer_DeviceInfo;
static DeviceInfo_t *SPP_Paired_Device = NULL;
static uint32_t LETransferStartTime, LETransferEndTime;

#define NUM_SUPPORTED_HCI_VERSIONS              (sizeof(HCIVersionStrings)/sizeof(char *) - 1)

   /* The following string table is used to map the API I/O Capabilities*/
   /* values to an easily displayable string.                           */
static BTPSCONST char *IOCapabilitiesStrings[] =
{
   "Display Only",
   "Display Yes/No",
   "Keyboard Only",
   "No Input/Output"
} ;

#define NUM_SUPPORTED_HCI_VERSIONS              (sizeof(HCIVersionStrings)/sizeof(char *) - 1)


   /* The following is used to map from ATT Error Codes to a printable  */
   /* string.                                                           */
static char *ErrorCodeStr[] =
{
   "ATT_PROTOCOL_ERROR_CODE_NO_ERROR",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_HANDLE",
   "ATT_PROTOCOL_ERROR_CODE_READ_NOT_PERMITTED",
   "ATT_PROTOCOL_ERROR_CODE_WRITE_NOT_PERMITTED",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_PDU",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION",
   "ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_OFFSET",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION",
   "ATT_PROTOCOL_ERROR_CODE_PREPARE_QUEUE_FULL",
   "ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_FOUND",
   "ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION_KEY_SIZE",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH",
   "ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION",
   "ATT_PROTOCOL_ERROR_CODE_UNSUPPORTED_GROUP_TYPE",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_RESOURCES"
};

#define NUMBER_GATT_ERROR_CODES  (sizeof(ErrorCodeStr)/sizeof(char *))

   /* The following array is used to map Device Appearance Values to    */
   /* strings.                                                          */
static GAPS_Device_Appearance_Mapping_t AppearanceMappings[] =
{
   {GAP_DEVICE_APPEARENCE_VALUE_UNKNOWN,                        "Unknown"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_PHONE,                  "Generic Phone"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_COMPUTER,               "Generic Computer"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_WATCH,                  "Generic Watch"},
   {GAP_DEVICE_APPEARENCE_VALUE_SPORTS_WATCH,                   "Sports Watch"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_CLOCK,                  "Generic Clock"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_DISPLAY,                "Generic Display"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_GENERIC_REMOTE_CONTROL, "Generic Remote Control"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_EYE_GLASSES,            "Eye Glasses"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_TAG,                    "Generic Tag"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_KEYRING,                "Generic Keyring"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_MEDIA_PLAYER,           "Generic Media Player"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_BARCODE_SCANNER,        "Generic Barcode Scanner"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_THERMOMETER,            "Generic Thermometer"},
   {GAP_DEVICE_APPEARENCE_VALUE_THERMOMETER_EAR,                "Ear Thermometer"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_HEART_RATE_SENSOR,      "Generic Heart Rate Sensor"},
   {GAP_DEVICE_APPEARENCE_VALUE_BELT_HEART_RATE_SENSOR,         "Belt Heart Rate Sensor"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_BLOOD_PRESSURE,         "Generic Blood Pressure"},
   {GAP_DEVICE_APPEARENCE_VALUE_BLOOD_PRESSURE_ARM,             "Blood Pressure: ARM"},
   {GAP_DEVICE_APPEARENCE_VALUE_BLOOD_PRESSURE_WRIST,           "Blood Pressure: Wrist"},
   {GAP_DEVICE_APPEARENCE_VALUE_HUMAN_INTERFACE_DEVICE,         "Human Interface Device"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_KEYBOARD,                   "HID Keyboard"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_MOUSE,                      "HID Mouse"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_JOYSTICK,                   "HID Joystick"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_GAMEPAD,                    "HID Gamepad"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_DIGITIZER_TABLET,           "HID Digitizer Tablet"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_CARD_READER,                "HID Card Reader"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_DIGITAL_PEN,                "HID Digitizer Pen"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_BARCODE_SCANNER,            "HID Bardcode Scanner"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_GLUCOSE_METER,          "Generic Glucose Meter"}
};

#define NUMBER_OF_APPEARANCE_MAPPINGS     (sizeof(AppearanceMappings)/sizeof(GAPS_Device_Appearance_Mapping_t))


   /* The following defines a data sequence that will be used to        */
   /* generate message data.                                            */
static char  DataStr[]  = "a123456789b123456789c123456789d123456789e123456789f123456789g123456789h123456789";
static int   DataStrLen = (sizeof(DataStr)-1);

#ifdef CONSOLE_SUPPORT
static BD_ADDR_t           InquiryResultList[MAX_INQUIRY_RESULTS]; /* Variable which   */
                                                    /* contains the inquiry result     */
                                                    /* received from the most recently */
                                                    /* preformed inquiry.              */
static unsigned int        NumberCommands;          /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static BoardStr_t          Callback_BoardStr;       /* Holds a BD_ADDR string in the   */
                                                    /* Callbacks.                      */

static BoardStr_t          Function_BoardStr;       /* Holds a BD_ADDR string in the   */
                                                    /* various functions.              */

static CommandTable_t      CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is  */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */
   /* The following string table is used to map HCI Version information */
   /* to an easily displayable version string.                          */
static BTPSCONST char *HCIVersionStrings[] =
{
   "1.0b",
   "1.1",
   "1.2",
   "2.0",
   "2.1",
   "3.0",
   "4.0",
   "4.1",
   "Unknown (greater 4.1)"
} ;
#endif // CONSOLE_SUPPORT


/* Internal function prototypes.                                     */

// PWV prototypes
static unsigned int PWVSendData(unsigned int BluetoothStackID, DeviceInfo_t *DeviceInfo, unsigned int DataLength, Byte_t *Data);
static void BTPSAPI GATT_ServerEventCallback(unsigned int BluetoothStackID, GATT_Server_Event_Data_t *GATT_ServerEventData, unsigned long CallbackParameter);
static int RegisterPasswordVault(void);

// HIDS protoypes
static DeviceInfo_t *SearchLEDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, GAP_LE_Address_Type_t AddressType, BD_ADDR_t BD_ADDR);
static DeviceInfo_t *DeleteLEDeviceInfoEntry(DeviceInfo_t **ListHead, GAP_LE_Address_Type_t AddressType, BD_ADDR_t BD_ADDR);
static int SlaveSecurityReEstablishment(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR);

// Other protoypes
static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization);
static int CloseStack(void);
static int SetDisc(int disc_mode);
static int SetConnect(uint8_t connectable);
static int SetPairable(int pairable);
static int SetPairableLE(int pairable);
static int RegisterAuthentication(void);
//static int AddLinkedKey(LinkKeyInfo_t * pKeyInfo);
static int GetLinkedKeyNb(void);
//static int GetLinkedKey(BD_ADDR_t BTAdd, LinkKeyInfo_t* pKey);
//static int DeleteLinkKey(BD_ADDR_t BD_ADDR);
static int PINCodeResponse(ParameterList_t *TempParam);
static int OpenServer(ParameterList_t *TempParam);
static int AdvertiseLE(ParameterList_t *TempParam);
static int AdvertiseLEEnable(uint8_t whitelist);
static int AdvertiseLEDisable(void);
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR);
int BT_WriteABuffer(const char * lpBuf, DWORD dwToWrite);
static int SaveDeviceInfoList(void);
static int LoadDeviceInfoList(void);

#ifdef CONSOLE_SUPPORT
static int PassKeyResponse(ParameterList_t *TempParam);
static void UserInterface_Client(void);
static void UserInterface_Server(void);
static void UserInterface_ProdTools(void);
static void UserInterface_Selection(void);
static int DisplayCallback(int Length, char *Message);
static void ProcessCharacters(void *UserParameter);
static Boolean_t CommandLineInterpreter(char *Command);
static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr);
static void DisplayIOCapabilities(void);
static void DisplayClassOfDevice(Class_of_Device_t Class_of_Device);
static void DisplayPrompt(void);
static void DisplayUsage(char *UsageString);
static void DisplayFunctionError(char *Function,int Status);
static void DisplayFunctionSuccess(char *Function);
static void DisplayFWVersion (void);
static unsigned long StringToUnsignedInteger(char *StringInteger);
static char *StringParser(char *String);
static int CommandParser(UserCommand_t *TempCommand, char *Input);
static int CommandInterpreter(UserCommand_t *TempCommand);
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction);
static CommandFunction_t FindCommand(char *Command);
static void ClearCommands(void);
static int QueryMemory(ParameterList_t *TempParam);
static int FTPMode(ParameterList_t *TempParam);
static int FormatMMC(ParameterList_t *TempParam);
static int GetFreeMMC(ParameterList_t *TempParam);
static int StartFirmwareUpdate(ParameterList_t *TempParam);
static int GetRSSI(ParameterList_t *TempParam);
static int DisplayHelp(ParameterList_t *TempParam);
static int Inquiry(ParameterList_t *TempParam);
static int DisplayInquiryList(ParameterList_t *TempParam);
static int SetDiscoverabilityMode(ParameterList_t *TempParam);
static int SetConnectabilityMode(ParameterList_t *TempParam);
static int SetPairabilityMode(ParameterList_t *TempParam);
static int ChangeSimplePairingParameters(ParameterList_t *TempParam);
static int Pair(ParameterList_t *TempParam);
static int EndPairing(ParameterList_t *TempParam);
static int UserConfirmationResponse(ParameterList_t *TempParam);
static int GetLocalAddress(ParameterList_t *TempParam);
static int SetLocalName(ParameterList_t *TempParam);
static int GetLocalName(ParameterList_t *TempParam);
static int SetClassOfDevice(ParameterList_t *TempParam);
static int GetClassOfDevice(ParameterList_t *TempParam);
static int GetRemoteName(ParameterList_t *TempParam);
static int SniffMode(ParameterList_t *TempParam);
static int ExitSniffMode(ParameterList_t *TempParam);
static int CloseServer(ParameterList_t *TempParam);
static int OpenRemoteServer(ParameterList_t *TempParam);
static int CloseRemoteServer(ParameterList_t *TempParam);
static int Read(ParameterList_t *TempParam);
static int Write(ParameterList_t *TempParam);
static int GetConfigParams(ParameterList_t *TempParam);
static int SetConfigParams(ParameterList_t *TempParam);
static int GetQueueParams(ParameterList_t *TempParam);
static int SetQueueParams(ParameterList_t *TempParam);
static int Loopback(ParameterList_t *TempParam);
static int DisplayRawModeData(ParameterList_t *TempParam);
static int AutomaticReadMode(ParameterList_t *TempParam);
static int SetBaudRate(ParameterList_t *TempParam);
static int SendData(ParameterList_t *TempParam);
static int ServerMode(ParameterList_t *TempParam);
static int ClientMode(ParameterList_t *TempParam);
static int ProdToolMode(ParameterList_t *TempParam);
#endif // CONSOLE_SUPPORT



   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI SPP_Event_Callback(unsigned int BluetoothStackID, SPP_Event_Data_t *SPP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_LLS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI LLS_EventCallback(unsigned int BluetoothStackID, LLS_Event_Data_t *LLS_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI IAS_EventCallback(unsigned int BluetoothStackID, IAS_Event_Data_t *IAS_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI BAS_Event_Callback(unsigned int BluetoothStackID, BAS_Event_Data_t *BAS_Event_Data, unsigned long CallbackParameter);

   /* HIDS Callback function prototypes.                                */
static void BTPSAPI BSC_TimerCallback(unsigned int BluetoothStackID, unsigned int TimerID, unsigned long CallbackParameter);

void SPP_On(void)
{
  ParameterList_t parm;
  
  parm.NumberofParameters = 1;
  parm.Params[0].intParam = 1;
  OpenServer(&parm);
  SetConnect(TRUE);
}

void SPP_Off(void)
{
  SetDisc(FALSE);
  SetPairable(FALSE);
  CloseServer(NULL);
}


#if CONSOLE_SUPPORT
   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static void UserInterface_Client(void)
{
      /* Next display the available commands.                           */
      DisplayHelp(NULL);

      /* Clear the installed command.                                   */
      ClearCommands();

      AddCommand("INQUIRY", Inquiry);
      AddCommand("DISPLAYINQUIRYLIST", DisplayInquiryList);
      AddCommand("PAIR", Pair);
      AddCommand("ENDPAIRING", EndPairing);
      AddCommand("PINCODERESPONSE", PINCodeResponse);
      AddCommand("PASSKEYRESPONSE", PassKeyResponse);
      AddCommand("USERCONFIRMATIONRESPONSE", UserConfirmationResponse);
      AddCommand("SETDISCOVERABILITYMODE", SetDiscoverabilityMode);
      AddCommand("SETCONNECTABILITYMODE", SetConnectabilityMode);
      AddCommand("SETPAIRABILITYMODE", SetPairabilityMode);
      AddCommand("CHANGESIMPLEPAIRINGPARAMETERS", ChangeSimplePairingParameters);
      AddCommand("GETLOCALADDRESS", GetLocalAddress);
      AddCommand("SETLOCALNAME", SetLocalName);
      AddCommand("GETLOCALNAME", GetLocalName);
      AddCommand("SETCLASSOFDEVICE", SetClassOfDevice);
      AddCommand("GETCLASSOFDEVICE", GetClassOfDevice);
      AddCommand("GETREMOTENAME", GetRemoteName);
      AddCommand("SNIFFMODE", SniffMode);
      AddCommand("EXITSNIFFMODE", ExitSniffMode);
      AddCommand("OPEN", OpenRemoteServer);
      AddCommand("CLOSE", CloseRemoteServer);
      AddCommand("READ", Read);
      AddCommand("WRITE", Write);
      AddCommand("GETCONFIGPARAMS", GetConfigParams);
      AddCommand("SETCONFIGPARAMS", SetConfigParams);
      AddCommand("GETQUEUEPARAMS", GetQueueParams);
      AddCommand("SETQUEUEPARAMS", SetQueueParams);
      AddCommand("LOOPBACK", Loopback);
      AddCommand("DISPLAYRAWMODEDATA", DisplayRawModeData);
      AddCommand("AUTOMATICREADMODE", AutomaticReadMode);
      AddCommand("SETBAUDRATE", SetBaudRate);
      AddCommand("SEND", SendData);
      AddCommand("HELP", DisplayHelp);
      AddCommand("QUERYMEMORY", QueryMemory);
      
}

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static void UserInterface_Server(void)
{ 
      /* Next display the available commands.                           */
      DisplayHelp(NULL);

      /* Clear the installed command.                                   */
      ClearCommands();

      /* Install the commands revelant for this UI.                     */
      AddCommand("INQUIRY", Inquiry);
      AddCommand("DISPLAYINQUIRYLIST", DisplayInquiryList);
      AddCommand("PAIR", Pair);
      AddCommand("ENDPAIRING", EndPairing);
      AddCommand("PINCODERESPONSE", PINCodeResponse);
      AddCommand("PASSKEYRESPONSE", PassKeyResponse);
      AddCommand("USERCONFIRMATIONRESPONSE", UserConfirmationResponse);
      AddCommand("SETDISCOVERABILITYMODE", SetDiscoverabilityMode);
      AddCommand("SETCONNECTABILITYMODE", SetConnectabilityMode);
      AddCommand("SETPAIRABILITYMODE", SetPairabilityMode);
      AddCommand("CHANGESIMPLEPAIRINGPARAMETERS", ChangeSimplePairingParameters);
      AddCommand("GETLOCALADDRESS", GetLocalAddress);
      AddCommand("SETLOCALNAME", SetLocalName);
      AddCommand("GETLOCALNAME", GetLocalName);
      AddCommand("SETCLASSOFDEVICE", SetClassOfDevice);
      AddCommand("GETCLASSOFDEVICE", GetClassOfDevice);
      AddCommand("GETREMOTENAME", GetRemoteName);
      AddCommand("SNIFFMODE", SniffMode);
      AddCommand("EXITSNIFFMODE", ExitSniffMode);
      AddCommand("OPEN", OpenServer);
      AddCommand("CLOSE", CloseServer);
      AddCommand("READ", Read);
      AddCommand("WRITE", Write);
      AddCommand("GETCONFIGPARAMS", GetConfigParams);
      AddCommand("SETCONFIGPARAMS", SetConfigParams);
      AddCommand("GETQUEUEPARAMS", GetQueueParams);
      AddCommand("SETQUEUEPARAMS", SetQueueParams);
      AddCommand("LOOPBACK", Loopback);
      //AddCommand("DISPLAYRAWMODEDATA", DisplayRawModeData);
      //AddCommand("AUTOMATICREADMODE", AutomaticReadMode);
      AddCommand("SETBAUDRATE", SetBaudRate);
      AddCommand("SEND", SendData);
      AddCommand("HELP", DisplayHelp);    
      AddCommand("QUERYMEMORY", QueryMemory);
      AddCommand("FTPMODE", FTPMode);
      AddCommand("GETRSSI", GetRSSI);
      AddCommand("ADVERTISELE", AdvertiseLE);
}

static void UserInterface_ProdTools(void)
{ 
      /* Next display the available commands.                           */
      DisplayHelp(NULL);

      /* Clear the installed command.                                   */
      ClearCommands();

      /* Install the commands revelant for this UI.                     */
      AddCommand("FORMAT", FormatMMC);
      AddCommand("FREE", GetFreeMMC);
      AddCommand("FIRMWAREUPDATE", StartFirmwareUpdate);
      
}

   /* The following function is responsible for choosing the user       */
   /* interface to present to the user.                                 */
static void UserInterface_Selection(void)
{
   /* Next display the available commands.                              */
   DisplayHelp(NULL);

   ClearCommands();

   AddCommand("SERVER", ServerMode);
   AddCommand("CLIENT", ClientMode);
   AddCommand("PROD", ProdToolMode);
   AddCommand("HELP", DisplayHelp);
}

   /* The following function is responsible for parsing user input      */
   /* and call appropriate command function.                            */
static Boolean_t CommandLineInterpreter(char *Command)
{
   int           Result = !EXIT_CODE;
   Boolean_t     ret_val = FALSE;
   UserCommand_t TempCommand;

   /* The string input by the user contains a value, now run the string */
   /* through the Command Parser.                                       */
       if(CommandParser(&TempCommand, Command) >= 0)
         {
            Display(("\r\n"));

            /* The Command was successfully parsed run the Command.           */
            Result = CommandInterpreter(&TempCommand);
            switch(Result)
            {
               case INVALID_COMMAND_ERROR:
                  Display(("Invalid Command: %s.\r\n",TempCommand.Command));
                  break;
               case FUNCTION_ERROR:
                  Display(("Function Error.\r\n"));
                  break;
               case EXIT_CODE:
            if(ServerPortID)
               CloseServer(NULL);
            else
            {
               if(SerialPortID)
                  CloseRemoteServer(NULL);
            }

            /* Restart the User Interface Selection.                    */
            UI_Mode = UI_MODE_SELECT;
            /* Set up the Selection Interface.                          */
            UserInterface_Selection();
                  break;
            }

          /* Display a prompt.                                              */
          DisplayPrompt();

          ret_val = TRUE;
         }
       else
        {
          /* Display a prompt.                                              */
          DisplayPrompt();

          Display(("\r\nInvalid Command.\r\n"));
        }
    
   return(ret_val);
}

   /* The following function is responsible for converting number       */
   /* strings to there unsigned integer equivalent.  This function can  */
   /* handle leading and tailing white space, however it does not handle*/
   /* signed or comma delimited values.  This function takes as its     */
   /* input the string which is to be converted.  The function returns  */
   /* zero if an error occurs otherwise it returns the value parsed from*/
   /* the string passed as the input parameter.                         */
static unsigned long StringToUnsignedInteger(char *StringInteger)
{
   int          IsHex;
   unsigned int Index;
   unsigned long ret_val = 0;

   /* Before proceeding make sure that the parameter that was passed as */
   /* an input appears to be at least semi-valid.                       */
   if((StringInteger) && (BTPS_StringLength(StringInteger)))
   {
      /* Initialize the variable.                                       */
      Index = 0;

      /* Next check to see if this is a hexadecimal number.             */
      if(BTPS_StringLength(StringInteger) > 2)
      {
         if((StringInteger[0] == '0') && ((StringInteger[1] == 'x') || (StringInteger[1] == 'X')))
         {
            IsHex = 1;

            /* Increment the String passed the Hexadecimal prefix.      */
            StringInteger += 2;
         }
         else
            IsHex = 0;
      }
      else
         IsHex = 0;

      /* Process the value differently depending on whether or not a    */
      /* Hexadecimal Number has been specified.                         */
      if(!IsHex)
      {
         /* Decimal Number has been specified.                          */
         while(1)
         {
            /* First check to make sure that this is a valid decimal    */
            /* digit.                                                   */
            if((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9'))
            {
               /* This is a valid digit, add it to the value being      */
               /* built.                                                */
               ret_val += (StringInteger[Index] & 0xF);

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < BTPS_StringLength(StringInteger)) && (StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9'))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 10.                                */
                  ret_val *= 10;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }
      }
      else
      {
         /* Hexadecimal Number has been specified.                      */
         while(1)
         {
            /* First check to make sure that this is a valid Hexadecimal*/
            /* digit.                                                   */
            if(((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9')) || ((StringInteger[Index] >= 'a') && (StringInteger[Index] <= 'f')) || ((StringInteger[Index] >= 'A') && (StringInteger[Index] <= 'F')))
            {
               /* This is a valid digit, add it to the value being      */
               /* built.                                                */
               if((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9'))
                  ret_val += (StringInteger[Index] & 0xF);
               else
               {
                  if((StringInteger[Index] >= 'a') && (StringInteger[Index] <= 'f'))
                     ret_val += (StringInteger[Index] - 'a' + 10);
                  else
                     ret_val += (StringInteger[Index] - 'A' + 10);
               }

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < BTPS_StringLength(StringInteger)) && (((StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9')) || ((StringInteger[Index+1] >= 'a') && (StringInteger[Index+1] <= 'f')) || ((StringInteger[Index+1] >= 'A') && (StringInteger[Index+1] <= 'F'))))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 16.                                */
                  ret_val *= 16;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }
      }
   }

   return(ret_val);
}

   /* The following function is responsible for parsing strings into    */
   /* components.  The first parameter of this function is a pointer to */
   /* the String to be parsed.  This function will return the start of  */
   /* the string upon success and a NULL pointer on all errors.         */
static char *StringParser(char *String)
{
   int   Index;
   char *ret_val = NULL;

   /* Before proceeding make sure that the string passed in appears to  */
   /* be at least semi-valid.                                           */
   if((String) && (BTPS_StringLength(String)))
   {
      /* The string appears to be at least semi-valid.  Search for the  */
      /* first space character and replace it with a NULL terminating   */
      /* character.                                                     */
      for(Index=0, ret_val=String;Index < BTPS_StringLength(String);Index++)
      {
         /* Is this the space character.                                */
         if((String[Index] == ' ') || (String[Index] == '\r') || (String[Index] == '\n'))
         {
            /* This is the space character, replace it with a NULL      */
            /* terminating character and set the return value to the    */
            /* beginning character of the string.                       */
            String[Index] = '\0';
            break;
         }
      }
   }

   return(ret_val);
}

   /* This function is responsible for taking command strings and       */
   /* parsing them into a command, param1, and param2.  After parsing   */
   /* this string the data is stored into a UserCommand_t structure to  */
   /* be used by the interpreter.  The first parameter of this function */
   /* is the structure used to pass the parsed command string out of the*/
   /* function.  The second parameter of this function is the string    */
   /* that is parsed into the UserCommand structure.  Successful        */
   /* execution of this function is denoted by a return value of zero.  */
   /* Negative return values denote an error in the parsing of the      */
   /* string parameter.                                                 */
static int CommandParser(UserCommand_t *TempCommand, char *Input)
{
   int            ret_val;
   int            StringLength;
   char          *LastParameter;
   unsigned int   Count         = 0;

   /* Before proceeding make sure that the passed parameters appear to  */
   /* be at least semi-valid.                                           */
   if((TempCommand) && (Input) && (BTPS_StringLength(Input)))
   {
      /* First get the initial string length.                           */
      StringLength = BTPS_StringLength(Input);

      /* Retrieve the first token in the string, this should be the     */
      /* command.                                                       */
      TempCommand->Command = StringParser(Input);

      /* Flag that there are NO Parameters for this Command Parse.      */
      TempCommand->Parameters.NumberofParameters = 0;

       /* Check to see if there is a Command                            */
      if(TempCommand->Command)
      {
         /* Initialize the return value to zero to indicate success on  */
         /* commands with no parameters.                                */
         ret_val    = 0;

         /* Adjust the UserInput pointer and StringLength to remove the */
         /* Command from the data passed in before parsing the          */
         /* parameters.                                                 */
         Input        += BTPS_StringLength(TempCommand->Command) + 1;
         StringLength  = BTPS_StringLength(Input);

         /* There was an available command, now parse out the parameters*/
         while((StringLength > 0) && ((LastParameter = StringParser(Input)) != NULL))
         {
            /* There is an available parameter, now check to see if     */
            /* there is room in the UserCommand to store the parameter  */
            if(Count < (sizeof(TempCommand->Parameters.Params)/sizeof(Parameter_t)))
            {
               /* Save the parameter as a string.                       */
               TempCommand->Parameters.Params[Count].strParam = LastParameter;

               /* Save the parameter as an unsigned int intParam will   */
               /* have a value of zero if an error has occurred.        */
               TempCommand->Parameters.Params[Count].intParam = StringToUnsignedInteger(LastParameter);

               Count++;
               Input        += BTPS_StringLength(LastParameter) + 1;
               StringLength -= BTPS_StringLength(LastParameter) + 1;

               ret_val = 0;
            }
            else
            {
               /* Be sure we exit out of the Loop.                      */
               StringLength = 0;

               ret_val      = TO_MANY_PARAMS;
            }
         }

         /* Set the number of parameters in the User Command to the     */
         /* number of found parameters                                  */
         TempCommand->Parameters.NumberofParameters = Count;
      }
      else
      {
         /* No command was specified                                    */
         ret_val = NO_COMMAND_ERROR;
      }
   }
   else
   {
      /* One or more of the passed parameters appear to be invalid.     */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* This function is responsible for determining the command in which */
   /* the user entered and running the appropriate function associated  */
   /* with that command.  The first parameter of this function is a     */
   /* structure containing information about the command to be issued.  */
   /* This information includes the command name and multiple parameters*/
   /* which maybe be passed to the function to be executed.  Successful */
   /* execution of this function is denoted by a return value of zero.  */
   /* A negative return value implies that command was not found and is */
   /* invalid.                                                          */
static int CommandInterpreter(UserCommand_t *TempCommand)
{
   int               i;
   int               ret_val;
   CommandFunction_t CommandFunction;

   /* If the command is not found in the table return with an invalid   */
   /* command error                                                     */
   ret_val = INVALID_COMMAND_ERROR;

   /* Let's make sure that the data passed to us appears semi-valid.    */
   if((TempCommand) && (TempCommand->Command))
   {
      /* Now, let's make the Command string all upper case so that we   */
      /* compare against it.                                            */
      for(i=0;i<BTPS_StringLength(TempCommand->Command);i++)
      {
         if((TempCommand->Command[i] >= 'a') && (TempCommand->Command[i] <= 'z'))
            TempCommand->Command[i] -= ('a' - 'A');
      }

      /* Check to see if the command which was entered was exit.        */
      if(BTPS_MemCompare(TempCommand->Command, "QUIT", BTPS_StringLength("QUIT")) != 0)
      {
         /* The command entered is not exit so search for command in    */
         /* table.                                                      */
         if((CommandFunction = FindCommand(TempCommand->Command)) != NULL)
         {
            /* The command was found in the table so call the command.  */
            if((ret_val = ((*CommandFunction)(&TempCommand->Parameters))) == 0)
            {
               /* Return success to the caller.                         */
               ret_val = 0;
            }
            else
            {
               if((ret_val != EXIT_CODE) && (ret_val != EXIT_MODE))
                  ret_val = FUNCTION_ERROR;
            }
         }
      }
      else
      {
         /* The command entered is exit, set return value to EXIT_CODE  */
         /* and return.                                                 */
         ret_val = EXIT_CODE;
      }
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return(ret_val);
}

   /* The following function is provided to allow a means to            */
   /* programatically add Commands the Global (to this module) Command  */
   /* Table.  The Command Table is simply a mapping of Command Name     */
   /* (NULL terminated ASCII string) to a command function.  This       */
   /* function returns zero if successful, or a non-zero value if the   */
   /* command could not be added to the list.                           */
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction)
{
   int ret_val = 0;

   /* First, make sure that the parameters passed to us appear to be    */
   /* semi-valid.                                                       */
   if((CommandName) && (CommandFunction))
   {
      /* Next, make sure that we still have room in the Command Table   */
      /* to add commands.                                               */
      if(NumberCommands < MAX_SUPPORTED_COMMANDS)
      {
         /* Simply add the command data to the command table and        */
         /* increment the number of supported commands.                 */
         CommandTable[NumberCommands].CommandName       = CommandName;
         CommandTable[NumberCommands++].CommandFunction = CommandFunction;

         /* Return success to the caller.                               */
         ret_val                                        = 0;
      }
      else
         ret_val = 1;
   }
   else
      ret_val = 1;

   return(ret_val);
}

   /* The following function searches the Command Table for the         */
   /* specified Command.  If the Command is found, this function returns*/
   /* a NON-NULL Command Function Pointer.  If the command is not found */
   /* this function returns NULL.                                       */
static CommandFunction_t FindCommand(char *Command)
{
   unsigned int      Index;
   CommandFunction_t ret_val;

   /* First, make sure that the command specified is semi-valid.        */
   if(Command)
   {
      /* Now loop through each element in the table to see if there is  */
      /* a match.                                                       */
      for(Index=0, ret_val = NULL; ((Index<NumberCommands) && (!ret_val)); Index++)
      {
         if((BTPS_StringLength(CommandTable[Index].CommandName) == BTPS_StringLength(Command)) && (BTPS_MemCompare(Command, CommandTable[Index].CommandName, BTPS_StringLength(CommandTable[Index].CommandName)) == 0))
            ret_val = CommandTable[Index].CommandFunction;
      }
   }
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function is provided to allow a means to clear out  */
   /* all available commands from the command table.                    */
static void ClearCommands(void)
{
   /* Simply flag that there are no commands present in the table.      */
   NumberCommands = 0;
}

   /* The following function is responsible for converting data of type */
   /* BD_ADDR to a string.  The first parameter of this function is the */
   /* BD_ADDR to be converted to a string.  The second parameter of this*/
   /* function is a pointer to the string in which the converted BD_ADDR*/
   /* is to be stored.                                                  */
static void BD_ADDRToStr(BD_ADDR_t Board_Address, BoardStr_t BoardStr)
{
   BTPS_SprintF((char *)BoardStr, "0x%02X%02X%02X%02X%02X%02X", Board_Address.BD_ADDR5, Board_Address.BD_ADDR4, Board_Address.BD_ADDR3, Board_Address.BD_ADDR2, Board_Address.BD_ADDR1, Board_Address.BD_ADDR0);
}

   /* Displays the current I/O Capabilities.                            */
static void DisplayIOCapabilities(void)
{
   Display(("I/O Capabilities: %s, MITM: %s.\r\n", IOCapabilitiesStrings[(unsigned int)IOCapability], MITMProtection?"TRUE":"FALSE"));
}

   /* Utility function to display a Class of Device Structure.          */
static void DisplayClassOfDevice(Class_of_Device_t Class_of_Device)
{
   Display(("Class of Device: 0x%02X%02X%02X.\r\n", Class_of_Device.Class_of_Device0, Class_of_Device.Class_of_Device1, Class_of_Device.Class_of_Device2));
}

   /* Displays the correct prompt depending on the Server/Client Mode.  */
static void DisplayPrompt(void)
{
  if(UI_Mode == UI_MODE_IS_CLIENT) {
    Display(("\r\nClient> \b"));
  } else if(UI_Mode == UI_MODE_IS_SERVER) {
    Display(("\r\nServer> \b"));
  } else if(UI_Mode == UI_MODE_IS_PRODTOOLS) {
    Display(("\r\nProd> \b"));
  } else {
   Display(("\r\nChoose Mode> \b"));
  }
}

   /* Displays a usage string..                                         */
static void DisplayUsage(char *UsageString)
{
   Display(("\nUsage: %s.\r\n", UsageString));
}

   /* Displays a function error message.                                */
static void DisplayFunctionError(char *Function, int Status)
{
   Display(("\n%s Failed: %d.\r\n", Function, Status));
}

   /* Displays a function success message.                              */
static void DisplayFunctionSuccess(char *Function)
{
   Display(("\n%s success.\r\n", Function));
}
   /* The following function is for displaying The FW Version by reading*/
   /* The Local version information form the FW.                        */
static void DisplayFWVersion (void)
{
    FW_Version FW_Version_Details;
    
    /* This function retrieves the Local Version Information of the FW. */    
    HCI_Read_Local_Version_Information(BluetoothStackID, &FW_Version_Details.StatusResult, &FW_Version_Details.HCI_VersionResult, &FW_Version_Details.HCI_RevisionResult, &FW_Version_Details.LMP_VersionResult, &FW_Version_Details.Manufacturer_NameResult, &FW_Version_Details.LMP_SubversionResult);
    if (!FW_Version_Details.StatusResult)
    {
        /* This function prints The project type from firmware, Bits    */
        /* 10 to 14 (5 bits) from LMP_SubversionResult parameter.       */
        Display(("Project Type  : %d \r\n", ((FW_Version_Details.LMP_SubversionResult >> 10)) & 0x1F));
        /* This function prints The version of the firmware. The first  */
        /* number is the Major version, Bits 7 to 9 and 15 (4 bits) from*/
        /* LMP_SubversionResult parameter, the second number is the     */
        /* Minor Version, Bits 0 to 6 (7 bits) from LMP_SubversionResult*/
        /* parameter.                                                   */
        slogf(LOG_DEST_BOTH, "Bluetooth FW Version    : %d.%d", ((FW_Version_Details.LMP_SubversionResult >> 7) & 0x07) + ((FW_Version_Details.LMP_SubversionResult >> 12) & 0x08), FW_Version_Details.LMP_SubversionResult & 0x7F);
    }
    else
        /* There was an error with HCI_Read_Local_Version_Information.  */
        /* Function.                                                    */
        DisplayFunctionError("HCI_Read_Local_Version_Information", FW_Version_Details.StatusResult);
}


   /* The following function is responsible for querying the memory     */
   /* heap usage. This function will return zero on successful          */  
   /* execution and a negative value on errors.                         */
static int QueryMemory(ParameterList_t *TempParam)
{
   BTPS_MemoryStatistics_t MemoryStatistics;
   int ret_val;

   /* Get current memory buffer usage                                   */
   ret_val = BTPS_QueryMemoryUsage(&MemoryStatistics, TRUE);
   if(!ret_val)
   {
      Display(("\r\n"));
      Display(("Heap Size:                %5d bytes\r\n", MemoryStatistics.HeapSize));
      Display(("Current Memory Usage:\r\n"));
      Display(("   Used:                  %5d bytes\r\n", MemoryStatistics.CurrentHeapUsed));
      Display(("   Free:                  %5d bytes\r\n", MemoryStatistics.HeapSize - MemoryStatistics.CurrentHeapUsed));
      Display(("Maximum Memory Usage:\r\n"));
      Display(("   Used:                  %5d bytes\r\n", MemoryStatistics.MaximumHeapUsed));
      Display(("   Free:                  %5d bytes\r\n", MemoryStatistics.HeapSize - MemoryStatistics.MaximumHeapUsed));
      Display(("Framentation:\r\n"));
      Display(("   Largest Free Fragment: %5d bytes\r\n", MemoryStatistics.LargestFreeFragment));
      Display(("   Free Fragment Cound:   %5d\r\n",       MemoryStatistics.FreeFragmentCount));
   }
   else
   {
      Display(("Failed to get memory usage\r\n"));
   }

   return(ret_val);
}
#endif // CONSOLE_SUPPORT

   /* Utility function to display advertising data.                     */
static void DisplayAdvertisingData(GAP_LE_Advertising_Data_t *Advertising_Data)
{
   unsigned int Index;
   unsigned int Index2;

   /* Verify that the input parameters seem semi-valid.                 */
   if(Advertising_Data)
   {
      for(Index = 0; Index < Advertising_Data->Number_Data_Entries; Index++)
      {
         Display(("  AD Type: 0x%02X.\r\n", Advertising_Data->Data_Entries[Index].AD_Type));
         Display(("  AD Length: 0x%02X.\r\n", Advertising_Data->Data_Entries[Index].AD_Data_Length));
         if(Advertising_Data->Data_Entries[Index].AD_Data_Buffer)
         {
            Display(("  AD Data: "));
            for(Index2 = 0; Index2 < Advertising_Data->Data_Entries[Index].AD_Data_Length; Index2++)
            {
               Display(("0x%02X ", Advertising_Data->Data_Entries[Index].AD_Data_Buffer[Index2]));
            }
            Display(("\r\n"));
         }
      }
   }
}

   /* The following function displays the pairing capabilities that is  */
   /* passed into this function.                                        */
static void DisplayPairingInformation(GAP_LE_Pairing_Capabilities_t Pairing_Capabilities)
{
   /* Display the IO Capability.                                        */
   switch(Pairing_Capabilities.IO_Capability)
   {
      case licDisplayOnly:
         Display(("   IO Capability:       lcDisplayOnly.\r\n"));
         break;
      case licDisplayYesNo:
         Display(("   IO Capability:       lcDisplayYesNo.\r\n"));
         break;
      case licKeyboardOnly:
         Display(("   IO Capability:       lcKeyboardOnly.\r\n"));
         break;
      case licNoInputNoOutput:
         Display(("   IO Capability:       lcNoInputNoOutput.\r\n"));
         break;
      case licKeyboardDisplay:
         Display(("   IO Capability:       lcKeyboardDisplay.\r\n"));
         break;
   }

   Display(("   MITM:                %s.\r\n", (Pairing_Capabilities.MITM == TRUE)?"TRUE":"FALSE"));
   Display(("   Bonding Type:        %s.\r\n", (Pairing_Capabilities.Bonding_Type == lbtBonding)?"Bonding":"No Bonding"));
   Display(("   OOB:                 %s.\r\n", (Pairing_Capabilities.OOB_Present == TRUE)?"OOB":"OOB Not Present"));
   Display(("   Encryption Key Size: %d.\r\n", Pairing_Capabilities.Maximum_Encryption_Key_Size));
   Display(("   Sending Keys: \r\n"));
   Display(("      LTK:              %s.\r\n", ((Pairing_Capabilities.Sending_Keys.Encryption_Key == TRUE)?"YES":"NO")));
   Display(("      IRK:              %s.\r\n", ((Pairing_Capabilities.Sending_Keys.Identification_Key == TRUE)?"YES":"NO")));
   Display(("      CSRK:             %s.\r\n", ((Pairing_Capabilities.Sending_Keys.Signing_Key == TRUE)?"YES":"NO")));
   Display(("   Receiving Keys: \r\n"));
   Display(("      LTK:              %s.\r\n", ((Pairing_Capabilities.Receiving_Keys.Encryption_Key == TRUE)?"YES":"NO")));
   Display(("      IRK:              %s.\r\n", ((Pairing_Capabilities.Receiving_Keys.Identification_Key == TRUE)?"YES":"NO")));
   Display(("      CSRK:             %s.\r\n", ((Pairing_Capabilities.Receiving_Keys.Signing_Key == TRUE)?"YES":"NO")));
}

   /* The following function provides a mechanism to configure a        */
   /* Pairing Capabilities structure with the application's pairing     */
   /* parameters.                                                       */
static void ConfigureCapabilities(GAP_LE_Pairing_Capabilities_t *Capabilities)
{
   /* Make sure the Capabilities pointer is semi-valid.                 */
   if(Capabilities)
   {
      /* Configure the Pairing Capabilities structure.                  */
      Capabilities->Bonding_Type                    = lbtBonding;
      Capabilities->IO_Capability                   = LE_Parameters.IOCapability;
      Capabilities->MITM                            = LE_Parameters.MITMProtection;
      Capabilities->OOB_Present                     = LE_Parameters.OOBDataPresent;

      /* ** NOTE ** This application always requests that we use the    */
      /*            maximum encryption because this feature is not a    */
      /*            very good one, if we set less than the maximum we   */
      /*            will internally in GAP generate a key of the        */
      /*            maximum size (we have to do it this way) and then   */
      /*            we will zero out how ever many of the MSBs          */
      /*            necessary to get the maximum size.  Also as a slave */
      /*            we will have to use Non-Volatile Memory (per device */
      /*            we are paired to) to store the negotiated Key Size. */
      /*            By requesting the maximum (and by not storing the   */
      /*            negotiated key size if less than the maximum) we    */
      /*            allow the slave to power cycle and regenerate the   */
      /*            LTK for each device it is paired to WITHOUT storing */
      /*            any information on the individual devices we are    */
      /*            paired to.                                          */
      Capabilities->Maximum_Encryption_Key_Size        = GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;

      /* This application only demonstrates using Long Term Key's (LTK) */
      /* for encryption of a LE Link, however we could request and send */
      /* all possible keys here if we wanted to.                        */
      Capabilities->Receiving_Keys.Encryption_Key     = TRUE;
      Capabilities->Receiving_Keys.Identification_Key = TRUE;
      Capabilities->Receiving_Keys.Signing_Key        = FALSE;

      Capabilities->Sending_Keys.Encryption_Key       = TRUE;
      Capabilities->Sending_Keys.Identification_Key   = FALSE;
      Capabilities->Sending_Keys.Signing_Key          = FALSE;
   }
}


   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as parameters to this function.  This     */
   /* function will return FALSE if NO Entry was added.  This can occur */
   /* if the element passed in was deemed invalid or the actual List    */
   /* Head was invalid.                                                 */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            Connection BD_ADDR.  When this occurs, this function   */
   /*            returns NULL.                                          */
static Boolean_t CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, GAP_LE_Address_Type_t ConnectionAddressType, BD_ADDR_t ConnectionBD_ADDR)
{
   Boolean_t     ret_val = FALSE;
   DeviceInfo_t *DeviceInfoPtr;

   /* Verify that the passed in parameters seem semi-valid.             */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
   {
      /* Allocate the memory for the entry.                             */
      if((DeviceInfoPtr = BTPS_AllocateMemory(sizeof(DeviceInfo_t))) != NULL)
      {
         /* Initialize the entry.                                       */
         BTPS_MemInitialize(DeviceInfoPtr, 0, sizeof(DeviceInfo_t));
         DeviceInfoPtr->ConnectionAddressType = ConnectionAddressType;
         DeviceInfoPtr->ConnectionBD_ADDR     = ConnectionBD_ADDR;

         ret_val = BSC_AddGenericListEntry_Actual(ekBD_ADDR_t, BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead), (void *)(DeviceInfoPtr));
         if(!ret_val)
         {
            /* Failed to add to list so we should free the memory that  */
            /* we allocated for the entry.                              */
            BTPS_FreeMemory(DeviceInfoPtr);
         }
      }
   }
   
   return(ret_val);
}

   /* The following function provides a mechanism of sending a Slave    */
   /* Pairing Response to a Master's Pairing Request.                   */
static int SlavePairingRequestResponse(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR)
{
   int                                          ret_val;
   BoardStr_t                                   BoardStr;
   GAP_LE_Authentication_Response_Information_t AuthenticationResponseData;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      BD_ADDRToStr(BD_ADDR, BoardStr);
      Display(("Sending Pairing Response to %s.\r\n", BoardStr));

      /* We must be the slave if we have received a Pairing Request     */
      /* thus we will respond with our capabilities.                    */
      AuthenticationResponseData.GAP_LE_Authentication_Type = larPairingCapabilities;
      AuthenticationResponseData.Authentication_Data_Length = GAP_LE_PAIRING_CAPABILITIES_SIZE;

      /* Configure the Application Pairing Parameters.                  */
      ConfigureCapabilities(&(AuthenticationResponseData.Authentication_Data.Pairing_Capabilities));

      /* Attempt to pair to the remote device.                          */
      ret_val = GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, &AuthenticationResponseData);
      if(ret_val)
         Display(("Error - GAP_LE_Authentication_Response returned %d.\r\n", ret_val));
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function provides a mechanism of attempting to      */
   /* re-established security with a previously paired master..         */
static int SlaveSecurityReEstablishment(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR)
{
   int                           ret_val;
   GAP_LE_Security_Information_t SecurityInformation;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      /* Configure the Security Information.                            */
      SecurityInformation.Local_Device_Is_Master                              = FALSE;
      SecurityInformation.Security_Information.Slave_Information.Bonding_Type = lbtBonding;
      SecurityInformation.Security_Information.Slave_Information.MITM         = LE_Parameters.MITMProtection;

      /* Attempt to pair to the remote device.                          */
      ret_val = GAP_LE_Reestablish_Security(BluetoothStackID, BD_ADDR, &SecurityInformation, GAP_LE_Event_Callback, 0);
      if(!ret_val)
         Display(("GAP_LE_Reestablish_Security sucess.\r\n"));
      else
         Display(("Error - GAP_LE_Reestablish_Security returned %d.\r\n", ret_val));
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* responding to a request for Encryption Information to send to a   */
   /* remote device.                                                    */
static int EncryptionInformationRequestResponse(BD_ADDR_t BD_ADDR, Byte_t KeySize, GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information)
{
   int    ret_val;
   Word_t LocalDiv;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      /* Make sure the input parameters are semi-valid.                 */
      if((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (GAP_LE_Authentication_Response_Information))
      {
         Display(("   Calling GAP_LE_Generate_Long_Term_Key.\r\n"));

         /* Generate a new LTK, EDIV and Rand tuple.                    */
         ret_val = GAP_LE_Generate_Long_Term_Key(BluetoothStackID, (Encryption_Key_t *)(&DHK), (Encryption_Key_t *)(&ER), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.LTK), &LocalDiv, &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.EDIV), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Rand));
         if(!ret_val)
         {
            Display(("   Encryption Information Request Response.\r\n"));

            /* Response to the request with the LTK, EDIV and Rand      */
            /* values.                                                  */
            GAP_LE_Authentication_Response_Information->GAP_LE_Authentication_Type                                     = larEncryptionInformation;
            GAP_LE_Authentication_Response_Information->Authentication_Data_Length                                     = GAP_LE_ENCRYPTION_INFORMATION_DATA_SIZE;
            GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Encryption_Key_Size = KeySize;

            ret_val = GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, GAP_LE_Authentication_Response_Information);
            if(!ret_val)
            {
               Display(("   GAP_LE_Authentication_Response (larEncryptionInformation) success.\r\n", ret_val));
            }
            else
            {
               DisplayFunctionError("GAP_LE_Authentication_Response", ret_val);
            }
         }
         else
         {
            DisplayFunctionError("GAP_LE_Generate_Long_Term_Key", ret_val);
         }
      }
      else
      {
         Display(("Invalid Parameters.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      Display(("Stack ID Invalid.\r\n"));

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}


   /* The following function searches the specified List for the        */
   /* specified Connection BD_ADDR.  This function returns NULL if      */
   /* either the List Head is invalid, the BD_ADDR is invalid, or the   */
   /* Connection BD_ADDR was NOT found.                                 */
static DeviceInfo_t *SearchLEDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, GAP_LE_Address_Type_t AddressType, BD_ADDR_t BD_ADDR)
{
   DeviceInfo_t *DeviceInfo;

   /* Verify that the input parameters are semi-valid.                  */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Check to see if this is a resolvable address type.  If so we   */
      /* will search the list based on the IRK.                         */
      if((AddressType == latRandom) && (GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(BD_ADDR)))
      {
         /* Walk the list and attempt to resolve this entry to an       */
         /* existing entry with IRK.                                    */
         DeviceInfo = *ListHead;
         while(DeviceInfo)
         {
            /* Check to see if the IRK is valid.                        */
            if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)
            {
               /* Attempt to resolve this address with the stored IRK.  */
               if(GAP_LE_Resolve_Address(ApplicationStateInfo.BluetoothStackID, &(DeviceInfo->IRK), BD_ADDR))
               {
                  /* Address resolved so just exit from the loop.       */
                  break;
               }
            }

            DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
         }
      }
      else
         DeviceInfo = NULL;

      /* If all else fail we will attempt to search the list by just the*/
      /* BD_ADDR.                                                       */
      if(DeviceInfo == NULL)
         DeviceInfo = BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)(&BD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead));
   }
   else
      DeviceInfo = NULL;

   return(DeviceInfo);
}

   /* The following function searches the specified List for the        */
   /* specified Connection BD_ADDR.  This function returns NULL if      */
   /* either the List Head is invalid, the BD_ADDR is invalid, or the   */
   /* Connection BD_ADDR was NOT found.                                 */
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR)
{
   return(BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)(&BD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead)));
}

   /* The following function searches the specified Key Info List for   */
   /* the specified BD_ADDR and removes it from the List.  This function*/
   /* returns NULL if either the List Head is invalid, the BD_ADDR is   */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and    */
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling the FreeKeyEntryMemory() function.          */
static DeviceInfo_t *DeleteLEDeviceInfoEntry(DeviceInfo_t **ListHead, GAP_LE_Address_Type_t AddressType, BD_ADDR_t BD_ADDR)
{
   DeviceInfo_t *LastEntry;
   DeviceInfo_t *DeviceInfo;

   if((ListHead) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      /* Check to see if this is a resolvable address type.  If so we   */
      /* will search the list based on the IRK.                         */
      if((AddressType == latRandom) && (GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(BD_ADDR)))
      {
         /* Now, let's search the list until we find the correct entry. */
         DeviceInfo = *ListHead;
         LastEntry  = NULL;
         while((DeviceInfo) && ((!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_IRK_VALID)) || (!GAP_LE_Resolve_Address(ApplicationStateInfo.BluetoothStackID, &(DeviceInfo->IRK), BD_ADDR))))
         {
            LastEntry  = DeviceInfo;

            DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
         }

         /* Check to see if we found the specified entry.               */
         if(DeviceInfo)
         {
            /* OK, now let's remove the entry from the list.  We have to*/
            /* check to see if the entry was the first entry in the     */
            /* list.                                                    */
            if(LastEntry)
            {
               /* Entry was NOT the first entry in the list.            */
               LastEntry->NextDeviceInfoPtr = DeviceInfo->NextDeviceInfoPtr;
            }
            else
               *ListHead = DeviceInfo->NextDeviceInfoPtr;

            DeviceInfo->NextDeviceInfoPtr = NULL;
         }
      }
      else
         DeviceInfo = NULL;

      /* If all else fail we will attempt to search the list by just the*/
      /* BD_ADDR.                                                       */
      if(DeviceInfo == NULL)
         DeviceInfo = BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)(&BD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead));
   }
   else
      DeviceInfo = NULL;

   return(DeviceInfo);
}


   /* The following function searches the specified Key Info List for   */
   /* the specified BD_ADDR and removes it from the List.  This function*/
   /* returns NULL if either the List Head is invalid, the BD_ADDR is   */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and    */
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling the FreeKeyEntryMemory() function.          */
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR)
{
   return(BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)(&BD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead)));
}

   /* This function frees the specified Key Info Information member     */
   /* memory.                                                           */
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)(EntryToFree));
}

   /* The following function is responsible for the specified string    */
   /* into data of type BD_ADDR.  The first parameter of this function  */
   /* is the BD_ADDR string to be converted to a BD_ADDR.  The second   */
   /* parameter of this function is a pointer to the BD_ADDR in which   */
   /* the converted BD_ADDR String is to be stored.                     */
static void StrToBD_ADDR(char *BoardStr, BD_ADDR_t *Board_Address)
{
     char          *TempPtr;
     unsigned int   StringLength;
     unsigned int   Index;
     unsigned char  Value;

     if((BoardStr) && ((StringLength = BTPS_StringLength(BoardStr)) >= (sizeof(BD_ADDR_t) * 2)) && (Board_Address))
     {
        TempPtr = BoardStr;
        if((StringLength >= (sizeof(BD_ADDR_t) * 2) + 2) && (TempPtr[0] == '0') && ((TempPtr[1] == 'x') || (TempPtr[1] == 'X')))
           TempPtr += 2;

        for(Index=0;Index<6;Index++)
        {
           Value  = (char)(ToInt(*TempPtr) * 0x10);
           TempPtr++;
           Value += (char)ToInt(*TempPtr);
           TempPtr++;
           ((char *)Board_Address)[5-Index] = (Byte_t)Value;
        }
     }
     else
     {
        if(Board_Address)
           BTPS_MemInitialize(Board_Address, 0, sizeof(BD_ADDR_t));
     }
}

   /* The following function provides a mechanism for sending a pairing */
   /* request to a device that is connected on an LE Link.              */
static int SendPairingRequest(BD_ADDR_t BD_ADDR, Boolean_t ConnectionMaster)
{
   int                           ret_val;
   BoardStr_t                    BoardStr;
   GAP_LE_Pairing_Capabilities_t Capabilities;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      /* Make sure the BD_ADDR is valid.                                */
      if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
      {
         /* Configure the application pairing parameters.               */
         ConfigureCapabilities(&Capabilities);

         /* Set the BD_ADDR of the device that we are attempting to pair*/
         /* with.                                                       */
         CurrentRemoteBD_ADDR = BD_ADDR;

         BD_ADDRToStr(BD_ADDR, BoardStr);
         Display(("Attempting to Pair to %s.\r\n", BoardStr));

         /* Attempt to pair to the remote device.                       */
         if(ConnectionMaster)
         {
            /* Start the pairing process.                               */
            ret_val = GAP_LE_Pair_Remote_Device(BluetoothStackID, BD_ADDR, &Capabilities, GAP_LE_Event_Callback, 0);

            if(ret_val == 0)
               Display(("GAP_LE_Pair_Remote_Device success.\r\n", ret_val));
            else
               DisplayFunctionError("     GAP_LE_Pair_Remote_Device", ret_val);

         }
         else
         {
            /* As a slave we can only request that the Master start     */
            /* the pairing process.                                     */
            ret_val = GAP_LE_Request_Security(BluetoothStackID, BD_ADDR, Capabilities.Bonding_Type, Capabilities.MITM, GAP_LE_Event_Callback, 0);

            if(ret_val == 0)
               Display(("GAP_LE_Request_Security success.\r\n", ret_val));
            else
               DisplayFunctionError("     GAP_LE_Request_Security", ret_val);
         }
      }
      else
      {
         Display(("Invalid Parameters.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      Display(("Stack ID Invalid.\r\n"));

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}


   /* The following function is responsible for registering a LLS       */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int RegisterLLS(void)
{
   int ret_val;

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(!LLSInstanceID)
      {
         /* Register the LLS Service with GATT.                         */
         ret_val = LLS_Initialize_Service(BluetoothStackID, LLS_EventCallback, 0, &LLSInstanceID);
         if((ret_val > 0) && (LLSInstanceID > 0))
         {
            /* Display success message.                                 */
            slogf(LOG_DEST_BOTH, "Successfully registered LLS Service.");

            /* Save the ServiceID of the registered service.            */
            LLSInstanceID  = (unsigned int)ret_val;

            /* Initialize internal LLS variables                        */
            {
               Byte_t  AlertLevel            = DEFAULT_ALERT_LEVEL;
               LLS_Set_Alert_Level(BluetoothStackID, LLSInstanceID, AlertLevel);
            }
            ret_val        = 0;
         }
      }
      else
      {
         Display(("LLS Service already registered.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      Display(("Connection currently active.\r\n"));

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for registering a TPS       */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int RegisterTPS(void)
{
   int ret_val;

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(!TPSInstanceID)
      {
         /* Register the TPS Service with GATT.                         */
         ret_val = TPS_Initialize_Service(BluetoothStackID, &TPSInstanceID);
         if((ret_val > 0) && (TPSInstanceID > 0))
         {
            /* Display success message.                                 */
            slogf(LOG_DEST_BOTH, "Successfully registered TPS Service.");

            /* Save the ServiceID of the registered service.            */
            TPSInstanceID  = (unsigned int)ret_val;
            ret_val        = 0;
         }
      }
      else
      {
         Display(("TPS Service already registered.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      Display(("Connection currently active.\r\n"));

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for registering a IAS       */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int RegisterIAS(void)
{
   int ret_val;

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(!IASInstanceID)
      {
         /* Register the IAS Service with GATT.                         */
         ret_val = IAS_Initialize_Service(BluetoothStackID,IAS_EventCallback,0,&IASInstanceID);
         if((ret_val > 0) && (IASInstanceID > 0))
         {
            /* Display success message.                                 */
            slogf(LOG_DEST_BOTH, "Successfully registered IAS Service.");

            /* Save the ServiceID of the registered service.            */
            IASInstanceID = (unsigned int)ret_val;
            ret_val       = 0;
         }
      }
      else
      {
         Display(("IAS Service already registered.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      Display(("Connection currently active.\r\n"));

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for registering a custom    */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int RegisterPasswordVault(void)
{
   int                           ret_val;
   GATT_Attribute_Handle_Group_t ServiceHandleGroup;

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(!PasswordVaultServiceID)
      {
         /* Initialize the handle group to 0 .                          */
         ServiceHandleGroup.Starting_Handle = 0;
         ServiceHandleGroup.Ending_Handle   = 0;

         /* Register the SPPLE Service.                                 */
         ret_val = GATT_Register_Service(BluetoothStackID, PWV_SERVICE_FLAGS, PWV_SERVICE_ATTRIBUTE_COUNT, (GATT_Service_Attribute_Entry_t *)PWV_Service, &ServiceHandleGroup, GATT_ServerEventCallback, 0);
         if(ret_val > 0)
         {
            /* Display success message.                                 */
            Display(("Sucessfully registered Password Vault Service.\r\n"));

            /* Save the ServiceID of the registered service.            */
            PasswordVaultServiceID = (unsigned int)ret_val;

            /* Return success to the caller.                            */
            ret_val        = 0;
         }
      }
      else
      {
         Display(("Password Vault Service already registered.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      Display(("Connection currently active.\r\n"));

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for enabling LE             */
   /* Advertisements.  This function returns zero on successful         */
   /* execution and a negative value on all errors.                     */
static int AdvertiseLE(ParameterList_t *TempParam)
{
   int                                ret_val = 0;
   int                                Length;
   int                                locked = 1;
   int                                NameInAdData = 0;
   int                                AdvertiseDataLength;
   int                                ScanResponseDataLength;
   int                                serviceUUIDCount;
   int                                serviceUUIDIndex;
   GAP_LE_Advertising_Parameters_t    AdvertisingParameters;
   GAP_LE_Connectability_Parameters_t ConnectabilityParameters;
   //GAP_LE_Address_Type_t              OwnAddressType = latPublic;
   GAP_LE_Address_Type_t              OwnAddressType = latRandom;
   BD_ADDR_t                          BD_ADDR;
   union
   {
      Advertising_Data_t   AdvertisingData;
      Scan_Response_Data_t ScanResponseData;
   } Advertisement_Data_Buffer;
   
   // Is advertising disabled?
   if (!Settings.Advertising_Enabled) {     
      Display(("[AdvertiseLE] Advertising disabled by user settings\r\n"));
      return -1;
   }

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if(TempParam) 
      {
         if  ((TempParam->NumberofParameters >= 1) && ((TempParam->Params[0].intParam == 0) || (TempParam->Params[0].intParam == 1)))
         {
      
             /* Determine whether to enable or disable Advertising.         */
             if(TempParam->Params[0].intParam == 0)
             {
                /* Disable Advertising.                                     */
                ret_val = GAP_LE_Advertising_Disable(BluetoothStackID);
                if(!ret_val)
                {
                   Display(("   GAP_LE_Advertising_Disable success.\r\n"));
                }
                else
                {
                   DisplayFunctionError("GAP_LE_Advertising_Disable", ret_val);

                   ret_val = FUNCTION_ERROR;
                }
             }
             else
             {
                 /* AdvertiseLE [1 = Enable]  [0 = Unlocked, 1 = Locked] [1 = Random Address] [Random BD Address] */
                 /* Check for lock status parameter                       */
                 if ((TempParam->NumberofParameters >= 2) && ((TempParam->Params[1].intParam == 0) || (TempParam->Params[1].intParam == 1))) {
                               locked = TempParam->Params[1].intParam;
                 } else if (TempParam->NumberofParameters >= 2) {
                               ret_val = INVALID_PARAMETERS_ERROR;
                 }
                    
                /* Verifying enable Advertising.parameters                  */
                if(TempParam->NumberofParameters >= 3) 
				{
                    if (TempParam->Params[2].intParam == 1)
                    {
                       OwnAddressType = latRandom;
                       if (BTPS_StringLength(TempParam->Params[3].strParam) >= (sizeof(BD_ADDR_t)*2))
    		           {
    		                /* Convert the parameter to a Bluetooth Device Address.        */
                       	        StrToBD_ADDR(TempParam->Params[3].strParam, &BD_ADDR);
    	  	           }
    		           else
    		   	           ret_val = INVALID_PARAMETERS_ERROR;
                    }
                    else 
                       if ((TempParam->Params[2].intParam != 0) || (TempParam->NumberofParameters < 3))
      		   	           ret_val = INVALID_PARAMETERS_ERROR;
	            }
                
                if (!ret_val)
                {                        
                   /* Enable Advertising.                                   */
                   /* Set the Advertising Data.                             */
                   BTPS_MemInitialize(&(Advertisement_Data_Buffer.AdvertisingData), 0, sizeof(Advertising_Data_t));

                   /* Set the Flags A/D Field (1 byte type and 1 byte Flags.*/
                   Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] = 2;
                   Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_FLAGS;
//#if 1 // db??? Android issue
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_BR_EDR_NOT_SUPPORTED_FLAGS_BIT_MASK;
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] |= HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
//#else
                   /* Configure the flags field based on the Discoverability*/
                   /* Mode.                                                 */
                   //if(LE_Parameters.DiscoverabilityMode == dmGeneralDiscoverableMode)
                   //   Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
                   //else
                   //{
                   //   if(LE_Parameters.DiscoverabilityMode == dmLimitedDiscoverableMode)
                   //      Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
                   //}
//#endif
                   
                   /* Configure the Device Appearance value.                         */
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3]   = 3;
                  Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[4] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_APPEARANCE;
                  ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[5]), GAP_DEVICE_APPEARENCE_VALUE_HID_KEYBOARD);
                              
                   
                   AdvertiseDataLength = 7;
                   
                   // Build complete UUID service list if any are active
                   if (LLSInstanceID || TPSInstanceID || IASInstanceID || BASInstanceID) {
                     
                        // Save current position so we can update it with the correct length after we are done
                        // assembling all of the service UUIDs
                        serviceUUIDIndex = AdvertiseDataLength;
                        serviceUUIDCount = 0;          
                        AdvertiseDataLength += 2;
                        
                        // Link loss service
                        if(LLSInstanceID)
                        {
                                serviceUUIDCount++;
                                LLS_ASSIGN_LLS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength]));
                                AdvertiseDataLength                                           += 2;
                        }
                              
                        // TX power service
                        if(TPSInstanceID)
                        {                
                                serviceUUIDCount++;
                                TPS_ASSIGN_TPS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength]));
                                AdvertiseDataLength                                          += 2;
                        }
                   
                        // Immediate alert service
                        if(IASInstanceID)
                        {                    
                                serviceUUIDCount++;
                                IAS_ASSIGN_IAS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength]));
                                AdvertiseDataLength                                          += 2;
                        }
                   
                        // Battery alert service
                        if(BASInstanceID)
                        {
                                serviceUUIDCount++;
                                BAS_ASSIGN_BAS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength]));
                                AdvertiseDataLength                                          += 2;
                        }   
                   
                        // Calculate and update length of service UUID data
                        if (serviceUUIDCount > 0) {
                                Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[serviceUUIDIndex] = 1 + serviceUUIDCount * 2; 
                                Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[serviceUUIDIndex+1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;
                        }
                   }
                   
                   /* Add Manufacturer Specific Data                        */
                   /* 1 byte type code = 0xFF                               */
                   /* 0x0387 = BluStor Unique Identifier                    */
                   /* Lock Flag = [0x00 = unlocked, 0x01 = locked]          */
                   Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength] = 4;
                   Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength+1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_MANUFACTURER_SPECIFIC;
                   Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength+2] = 0x03;
                   Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength+3] = 0x87;
                   if (locked) {
                      Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength+4] = 0x01;
                   } else {
                      Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength+4] = 0x00;
                   }
                   AdvertiseDataLength                                          += 5;
                   
                   /* If we have room, add the complete local name to the advertising data */
                   Length = BTPS_StringLength(APP_CYBERGATE_LE);
                   if ((AdvertiseDataLength + 2  + Length) <= ADVERTISING_DATA_MAXIMUM_SIZE) {
                        Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength] = 1 + Length;
                        Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength+1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_COMPLETE;
                        BTPS_MemCopy(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[AdvertiseDataLength+2]),APP_CYBERGATE_LE, Length);
                        AdvertiseDataLength = AdvertiseDataLength + 2 + Length;
                        NameInAdData = 1;
                   }
                   
                   /* Write the advertising data to the chip.               */
                   ret_val = GAP_LE_Set_Advertising_Data(BluetoothStackID, AdvertiseDataLength, &(Advertisement_Data_Buffer.AdvertisingData));
                   if(!ret_val)
                   {
                      BTPS_MemInitialize(&(Advertisement_Data_Buffer.ScanResponseData), 0, sizeof(Scan_Response_Data_t));
                      ScanResponseDataLength = 0;

                      Length = BTPS_StringLength(APP_CYBERGATE_LE);
                      
                      /* If we didn't include fullname in advertisement data, try to include it in scan data */
                      if(Length <= (ADVERTISING_DATA_MAXIMUM_SIZE - 2 - 18) && (!NameInAdData))
                      {
                         Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[ScanResponseDataLength] = (Byte_t)(1 + Length);
                         Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[ScanResponseDataLength + 1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_COMPLETE;

                         BTPS_MemCopy(&(Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[ScanResponseDataLength + 2]),APP_CYBERGATE_LE,Length);                   
                         ScanResponseDataLength += (2 + Length);
                      } else {
 
                         // Otherwise, we can skip adding any scan data and move on
                         ret_val = 0;
                      }
                      /* Set the Scan Response Data.                        */
                      Length = 16;
                      Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[ScanResponseDataLength + 0] = (Byte_t)(1 + Length);
                      Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[ScanResponseDataLength + 1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_128_BIT_SERVICE_UUID_COMPLETE;
                      PWV_ASSIGN_PWV_SERVICE_UUID_128(&(Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[ScanResponseDataLength + 2]));
                      ScanResponseDataLength += (2 + Length);
                      //BTPS_MemCopy(&(Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[2]),PWV_SERVICE_BLUETOOTH_UUID_CONSTANT,Length);                                 
                      ret_val = GAP_LE_Set_Scan_Response_Data(BluetoothStackID, ScanResponseDataLength, &(Advertisement_Data_Buffer.ScanResponseData));

                      if(!ret_val)
                      {
                         /* Set up the advertising parameters.              */
                         AdvertisingParameters.Advertising_Channel_Map   = HCI_LE_ADVERTISING_CHANNEL_MAP_DEFAULT;
                         AdvertisingParameters.Scan_Request_Filter       = fpNoFilter;
                         if((TempParam->NumberofParameters >= 5) && (TempParam->Params[4].intParam == 1)) {
                           slogf(LOG_DEST_BOTH, "Connect with whitelist");
                           AdvertisingParameters.Connect_Request_Filter    = fpWhiteList;
                         } else {
                           slogf(LOG_DEST_BOTH, "Connect to all");
                           AdvertisingParameters.Connect_Request_Filter    = fpNoFilter;
                         }
                         AdvertisingParameters.Advertising_Interval_Min  = Settings.Advertising_Interval_Min;         //1000;
                         AdvertisingParameters.Advertising_Interval_Max  = Settings.Advertising_Interval_Max;         //1500;

                         /* Configure the Connectability Parameters.        */
                         /* * NOTE * Since we do not ever put ourselves to  */
                         /*          be direct connectable then we will set */
                         /*          the DirectAddress to all 0s.           */

                         //ConnectabilityParameters.Connectability_Mode   = LE_Parameters.ConnectableMode;
                         ConnectabilityParameters.Connectability_Mode   = lcmConnectable;
                         ConnectabilityParameters.Own_Address_Type      = OwnAddressType;
                         //ConnectabilityParameters.Direct_Address_Type   = latPublic;
                         //ASSIGN_BD_ADDR(ConnectabilityParameters.Direct_Address, 0, 0, 0, 0, 0, 0);
                         ConnectabilityParameters.Direct_Address = BD_ADDR;
                         
                         // Create compliant psuedo-random BD_ADDR based on the original address
                         if(!GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR))
                         {
                            // Make it a compliant random address
                            BD_ADDR.BD_ADDR5 |= 0x42;
                         }

                         /*If its a Random Adrees if So set the Random address first   */                
                        //if((OwnAddressType) && (!ret_val))
                        //{
                        //    ret_val = HCI_LE_Set_Random_Address(BluetoothStackID, BD_ADDR, &StatusResult);
                        //    ret_val += StatusResult;
                        //}

                         /* Now enable advertising.                         */
                        //if(!ret_val)
                        //{
                             //GAP_LE_Generate_Static_Address(BluetoothStackID, &BD_ADDR);
                             //GAP_LE_Generate_Resolvable_Address(BluetoothStackID, &IRK, &BD_ADDR);
                             GAP_LE_Set_Random_Address(BluetoothStackID, BD_ADDR);
                             ConnectabilityParameters.Direct_Address = BD_ADDR;
                             ret_val = GAP_LE_Advertising_Enable(BluetoothStackID, TRUE, &AdvertisingParameters, &ConnectabilityParameters, GAP_LE_Event_Callback, 0);
                             if(!ret_val)
                             {
                                slogf(LOG_DEST_BOTH, "GAP_LE_Advertising_Enable success.");
                             }
                             else
                             {
                                slogf(LOG_DEST_BOTH, "GAP_LE_Advertising_Enable failed: %d", ret_val);

                                ret_val = FUNCTION_ERROR;
                             }
                        //}
                        //else
                        //{
                        //    DisplayFunctionError("HCI_LE_Set_Random_Address", ret_val);
                        //    ret_val = FUNCTION_ERROR;
                        //}

                      }
                      else
                      {
                         DisplayFunctionError("GAP_LE_Set_Advertising_Data(dtScanResponse)", ret_val);

                         ret_val = FUNCTION_ERROR;
                      }

                   }
                   else
                   {
                      DisplayFunctionError("GAP_LE_Set_Advertising_Data(dtAdvertising)", ret_val);

                      ret_val = FUNCTION_ERROR;
                   }
                }
                else
                {
                   DisplayUsage("AdvertiseLE [0 = Disable, 1 = Enable(Default - Public Address)] [0 = Unlocked, 1 = Locked]");
                   DisplayUsage("AdvertiseLE [1 = Enable]  [0 = Unlocked, 1 = Locked] [0 = Public Address]");
                   DisplayUsage("AdvertiseLE [1 = Enable]  [0 = Unlocked, 1 = Locked] [1 = Random Address] [Random BD Address]");
                   ret_val = INVALID_PARAMETERS_ERROR;
                }
             }
         }
         else
         {
             DisplayUsage("AdvertiseLE [0 = Disable, 1 = Enable(Default - Public Address)] [0 = Unlocked, 1 = Locked]");
             DisplayUsage("AdvertiseLE [1 = Enable]  [0 = Unlocked, 1 = Locked] [0 = Public Address]");
             DisplayUsage("AdvertiseLE [1 = Enable]  [0 = Unlocked, 1 = Locked] [1 = Random Address] [Random BD Address]");;
             ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
          DisplayUsage("AdvertiseLE [0 = Disable, 1 = Enable(Default - Public Address)] [0 = Unlocked, 1 = Locked] ");
          DisplayUsage("AdvertiseLE [1 = Enable]  [0 = Unlocked, 1 = Locked] [0 = Public Address]");
          DisplayUsage("AdvertiseLE [1 = Enable]  [0 = Unlocked, 1 = Locked] [1 = Random Address] [Random BD Address]");;;
          ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

static int AdvertiseLEEnable(uint8_t whitelist)
{
   int ret_val = 0;
   ParameterList_t parm;
   
   parm.NumberofParameters = 5;
   parm.Params[0].intParam = 1;
   if (FTPLocked) {
      parm.Params[1].intParam = 1;
   } else {
      parm.Params[1].intParam = 0;
   }
   parm.Params[2].intParam = 0;
   parm.Params[3].strParam = NULL;
   parm.Params[4].intParam = whitelist;
   if (Settings.Advertising_Enabled) {
      ret_val = AdvertiseLE(&parm);
      if(!ret_val) {
         AdvertisingStatus = TRUE;
      }
   }
   return ret_val;
}

static int AdvertiseLEDisable(void)
{
   int ret_val = 0;
   ParameterList_t parm;
   
   parm.NumberofParameters = 1;
   parm.Params[0].intParam = 0;
   // Disable BLE advertising
   if (Settings.Advertising_Enabled) {
      ret_val = AdvertiseLE(&parm);
      if(!ret_val) {
         AdvertisingStatus = FALSE;
      }
   }
   return ret_val;
}

/* The following function changes the BLE Advertising data to indicate  */
/* if the card is locked or unlocked.  In order to do this, BLE         */
/* advertising must first be disabled and then renabled with the        */
/* updated advertising data parameters.                                 */
void AdvertiseLockStatus(int locked) {
  
  if (Settings.Advertising_Enabled) {
  
    ParameterList_t parm;
    
    // First let's disabled BLE Advertising                  
    parm.NumberofParameters = 1;
    parm.Params[0].intParam = 0;
    if (!AdvertiseLE(&parm)) {
      AdvertisingStatus = FALSE;
      
      // Short delay to ensure etLE_Disconnection_Complete finishes before restarting
      // http://e2e.ti.com/support/wireless_connectivity/bluetooth_cc256x/f/660/p/242821/853646
      // Error codes defined in BTErrors.h
      //BTPS_Delay(1000);]
      
      // Now let's update the lock status 
      parm.NumberofParameters = 2;
      parm.Params[0].intParam = 1;
      if (locked) {
        parm.Params[1].intParam = 1;
      } else {
        parm.Params[1].intParam = 0;
      }
      
      // Renable BLE Advertising
      if (!AdvertiseLE(&parm)) {
        AdvertisingStatus = TRUE;
      } else {
        Display(("[AdvertiseLockStatus] failed to re-enable BLE advertising.\r\n"));
      }
      
    } else {
      Display(("[AdvertiseLockStatus] failed to disable BLE advertising.\r\n"));
    }
  
  } else {
      Display(("[AdvertiseLockStatus] Advertising disabled by user settings.\r\n"));
  }
  
}


   /* The following function is responsible for opening the SS1         */
   /* Bluetooth Protocol Stack.  This function accepts a pre-populated  */
   /* HCI Driver Information structure that contains the HCI Driver     */
   /* Transport Information.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization)
{
   int                        Result;
   int                        ret_val = 0;
   Byte_t                     Status;
   Byte_t                     NumberLEPackets;
   Word_t                     LEPacketLength;
   L2CA_Link_Connect_Params_t L2CA_Link_Connect_Params;
   unsigned int  ServiceID;

   /* First check to see if the Stack has already been opened.          */
   if(!BluetoothStackID)
   {
      /* Next, makes sure that the Driver Information passed appears to */
      /* be semi-valid.                                                 */
      if((HCI_DriverInformation) && (BTPS_Initialization))
      {

         SelfTest.SerialBT = SELF_TEST_COM_INIT_BT;
         /* Initialize BTPSKNRl.                                        */
         BTPS_Init(BTPS_Initialization);

         Display(("\r\nOpenStack().\r\n"));
         
         SelfTest.SerialBT = SELF_TEST_STACK_INIT_BT;
         /* Initialize the Stack                                        */
         Result = BSC_Initialize(HCI_DriverInformation, 0);

         /* Next, check the return value of the initialization to see if*/
         /* it was successful.                                          */
         if(Result > 0)
         {
            //SByte_t TxPower = 0;
            BluetoothStackID = Result;
            
            // HIDS sample code expects stack ID to be part of application state structure
            ApplicationStateInfo.BluetoothStackID = Result;
            
            //ret_val = VS_Set_Max_Output_Power(BluetoothStackID, TxPower);
            //printf("VS_Set_Max_Output_Power= %d ret: %d\r\n", TxPower, ret_val);
            BTPS_Delay(200);
            
            /* The Stack was initialized successfully, inform the user  */
            /* and set the return value of the initialization function  */
            /* to the Bluetooth Stack ID.                               */
            BluetoothStackID = Result;
            slogf(LOG_DEST_BOTH,"Bluetooth Stack ID: %d", BluetoothStackID);
            
            /* Attempt to enable the WBS feature.                       */
            Result = BSC_EnableFeature(BluetoothStackID, BSC_FEATURE_BLUETOOTH_LOW_ENERGY);
            if(!Result)
            {
              slogf(LOG_DEST_BOTH, "LOW ENERGY Support initialized.");
            }
            else
            {
              slogf(LOG_DEST_BOTH, "LOW ENERGY Support not initialized %d.", Result);
            }            
            
            /* Initialize the Default Pairing Parameters.               */
            //LE_Parameters.IOCapability   = DEFAULT_LE_IO_CAPABILITY;
            //LE_Parameters.MITMProtection = DEFAULT_LE_MITM_PROTECTION;
            LE_Parameters.OOBDataPresent = FALSE;
            
            // HIDS LE parameters
            LE_Parameters.IOCapability   = licNoInputNoOutput;
            LE_Parameters.MITMProtection = FALSE;
            
            /* Initialize the default Secure Simple Pairing parameters. */
            //IOCapability     = DEFAULT_IO_CAPABILITY;
            //OOBSupport       = FALSE;
            //MITMProtection   = DEFAULT_MITM_PROTECTION;
            
            IOCapability     = licNoInputNoOutput;
            OOBSupport       = FALSE;
            MITMProtection   = FALSE;

#ifdef CONSOLE_SUPPORT
            {
              char BluetoothAddress[16];
              HCI_Version_t HCIVersion;
              BD_ADDR_t BD_ADDR;
              if(!HCI_Version_Supported(BluetoothStackID, &HCIVersion)) {
                slogf(LOG_DEST_BOTH, "Device Chipset: %s", (HCIVersion <= NUM_SUPPORTED_HCI_VERSIONS)?HCIVersionStrings[HCIVersion]:HCIVersionStrings[NUM_SUPPORTED_HCI_VERSIONS]);
              }

              /* Printing the BTPS version                                */
              slogf(LOG_DEST_BOTH, "BTPS Version  : %s", BTPS_VERSION_VERSION_STRING);
              /* Printing the FW version                                  */
              DisplayFWVersion();

              /* Printing the Demo Application version                    */
              slogf(LOG_DEST_BOTH, "App Name      : %s", APP_DEMO_NAME);
              slogf(LOG_DEST_BOTH, "App Version   : %s", DEMO_APPLICATION_VERSION_STRING);            
              
              /* Let's output the Bluetooth Device Address so that the    */
              /* user knows what the Device Address is.                   */
              if(!GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR))
              {
                 BD_ADDRToStr(BD_ADDR, BluetoothAddress);

                 slogf(LOG_DEST_BOTH, "LOCAL BD_ADDR: %s", BluetoothAddress);
              }
            }
#endif // CONSOLE_SUPPORT            

            /* Go ahead and allow Master/Slave Role Switch.             */
            L2CA_Link_Connect_Params.L2CA_Link_Connect_Request_Config  = cqAllowRoleSwitch;
            L2CA_Link_Connect_Params.L2CA_Link_Connect_Response_Config = csMaintainCurrentRole;

            L2CA_Set_Link_Connection_Configuration(BluetoothStackID, &L2CA_Link_Connect_Params);

            if(HCI_Command_Supported(BluetoothStackID, HCI_SUPPORTED_COMMAND_WRITE_DEFAULT_LINK_POLICY_BIT_NUMBER) > 0) {
               HCI_Write_Default_Link_Policy_Settings(BluetoothStackID, (HCI_LINK_POLICY_SETTINGS_ENABLE_MASTER_SLAVE_SWITCH|HCI_LINK_POLICY_SETTINGS_ENABLE_SNIFF_MODE), &Status);
            }

            LocalDeviceIsMaster = FALSE;

            /* Regenerate IRK and DHK from the constant Identity Root   */
            /* Key.                                                     */
            GAP_LE_Diversify_Function(BluetoothStackID, (Encryption_Key_t *)(&IR), 1,0, &IRK);
            GAP_LE_Diversify_Function(BluetoothStackID, (Encryption_Key_t *)(&IR), 3, 0, &DHK);
            
            /* Flag that we have no Key Information in the Key List.    */
            DeviceInfoList = NULL;
            
            /* Load in DeviceInfoList from flash                        */
            LoadDeviceInfoList();
            
            /* Regenerate IRK and DHK from the constant Identity Root Key. */
            GAP_LE_Diversify_Function(ApplicationStateInfo.BluetoothStackID, (Encryption_Key_t *)(&IR), 1,0, &IRK);
            GAP_LE_Diversify_Function(ApplicationStateInfo.BluetoothStackID, (Encryption_Key_t *)(&IR), 3, 0, &DHK);
            
            /* Initialize the GATT Service.                             */
            Result = GATT_Initialize(BluetoothStackID, GATT_INITIALIZATION_FLAGS_SUPPORT_LE, GATT_Connection_Event_Callback, 0);
            if(!Result)
            {
               /* Determine the number of LE packets that the controller*/
               /* will accept at a time.                                */
               if((!HCI_LE_Read_Buffer_Size(BluetoothStackID, &Status, &LEPacketLength, &NumberLEPackets)) && (!Status) && (LEPacketLength))
               {
                  NumberLEPackets = (NumberLEPackets/MAX_LE_CONNECTIONS);
                  NumberLEPackets = (NumberLEPackets == 0)?1:NumberLEPackets;
               }
               else
                  NumberLEPackets = 1;

               /* Set a limit on the number of packets that we will     */
               /* queue internally.                                     */
               GATT_Set_Queuing_Parameters(BluetoothStackID, (unsigned int)NumberLEPackets, (unsigned int)(NumberLEPackets-1), FALSE);
               
               /* Initialize the GAPS Service.                          */
               Result = GAPS_Initialize_Service(BluetoothStackID, &ServiceID);
               if(Result > 0)
               {
                  /* Save the Instance ID of the GAP Service.           */
                  GAPSInstanceID = (unsigned int)Result;
                  
                  /* HIDS - Save the Instance ID of the GAP Service.    */
                  ApplicationStateInfo.GAPSInstanceID = (unsigned int)Result;
               
                  /* Set the GAP Device Name and Device Appearance.     */
                  GAP_Set_Local_Device_Name (BluetoothStackID,APP_DEMO_NAME);
                  
                  GAPS_Set_Device_Name(BluetoothStackID, GAPSInstanceID, APP_CYBERGATE_LE);
                  GAPS_Set_Device_Appearance(BluetoothStackID, GAPSInstanceID, GAP_DEVICE_APPEARENCE_VALUE_GENERIC_COMPUTER);
                  
                  /* Initialize the DIS Service.                        */
                  Result = DIS_Initialize_Service(BluetoothStackID, &ServiceID);
                  if(Result > 0)
                  {
                       /* Save the Instance ID of the GAP Service.        */
                       DISInstanceID = (unsigned int)Result;

                       /* Set the discoverable attributes                 */
                       DIS_Set_Manufacturer_Name(BluetoothStackID, DISInstanceID, BTPS_VERSION_COMPANY_NAME_STRING);
                       DIS_Set_Model_Number(BluetoothStackID, DISInstanceID, BTPS_VERSION_VERSION_STRING);
                       DIS_Set_Serial_Number(BluetoothStackID, DISInstanceID, BTPS_VERSION_VERSION_STRING);
                       DIS_Set_IEEE_Certification_Data(BluetoothStackID, DISInstanceID, sizeof(IEEE_DATA_LIST), IEEE_DATA_LIST);

                        /* Initialize the BAS Service.                                       */
                        Result = BAS_Initialize_Service(BluetoothStackID, BAS_Event_Callback, 0, &ServiceID);
                        if (Result > 0)
                        {
                           BASInstanceID = (unsigned int)Result;
                           
                        }                       
                  }
               }
               else
               {
                  /* The Stack was NOT initialized successfully, inform */
                  /* the user and set the return value of the           */
                  /* initialization function to an error.               */
                  DisplayFunctionError("GAPS_Initialize_Service", Result);

                  /* Cleanup GATT Module.                               */
                  GATT_Cleanup(BluetoothStackID);

                  BluetoothStackID = 0;
                  
                  // HIDS sample code expects stack ID to be part of application state structure
                  ApplicationStateInfo.BluetoothStackID = 0;

                  ret_val          = UNABLE_TO_INITIALIZE_STACK;
               }
            }
            else
            {
               /* The Stack was NOT initialized successfully, inform the*/
               /* user and set the return value of the initialization   */
               /* function to an error.                                 */
               DisplayFunctionError("GATT_Initialize", Result);

               BluetoothStackID = 0;
               
               // HIDS sample code expects stack ID to be part of application state structure
               ApplicationStateInfo.BluetoothStackID = 0;

               ret_val          = UNABLE_TO_INITIALIZE_STACK;
            }
         }
         else
         {
            /* The Stack was NOT initialized successfully, inform the   */
            /* user and set the return value of the initialization      */
            /* function to an error.                                    */
#ifdef CONSOLE_SUPPORT
           DisplayFunctionError("Stack Init", Result);
#endif // CONSOLE_SUPPORT
            BluetoothStackID = 0;

            ret_val          = UNABLE_TO_INITIALIZE_STACK;
         }
      }
      else
      {
         /* One or more of the necessary parameters are invalid.        */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }

   return(ret_val);
}

   /* The following function is responsible for closing the SS1         */
   /* Bluetooth Protocol Stack.  This function requires that the        */
   /* Bluetooth Protocol stack previously have been initialized via the */
   /* OpenStack() function.  This function returns zero on successful   */
   /* execution and a negative value on all errors.                     */
static int CloseStack(void)
{
   int ret_val = 0;

   /* First check to see if the Stack has been opened.                  */
   if(BluetoothStackID)
   {
      /* Simply close the Stack                                         */
      BSC_Shutdown(BluetoothStackID);

      /* Free BTPSKRNL allocated memory.                                */
      BTPS_DeInit();

      slogf(LOG_DEST_BOTH,"Stack Shutdown.");

      /* Flag that the Stack is no longer initialized.                  */
      BluetoothStackID = 0;

      /* Flag success to the caller.                                    */
      ret_val          = 0;
   }
   else
   {
      /* A valid Stack ID does not exist, inform to user.               */
      ret_val = UNABLE_TO_INITIALIZE_STACK;
   }

   return(ret_val);
}

   /* The following function is responsible for placing the Local       */
   /* Bluetooth Device into General Discoverablity Mode.  Once in this  */
   /* mode the Device will respond to Inquiry Scans from other Bluetooth*/
   /* Devices.  This function requires that a valid Bluetooth Stack ID  */
   /* exists before running.  This function returns zero on successful  */
   /* execution and a negative value if an error occurred.              */
static int SetDisc(int disc_mode)
{
   int ret_val = 0;
   GAP_Discoverability_Mode_t GapDiscMode;

   
   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* A semi-valid Bluetooth Stack ID exists, now attempt to set the */
      /* attached Devices Discoverablity Mode to General.               */
     if (disc_mode) {
       GapDiscMode = dmGeneralDiscoverableMode;
     } else {
       GapDiscMode = dmNonDiscoverableMode;
     }
      ret_val = GAP_Set_Discoverability_Mode(BluetoothStackID, GapDiscMode, 0);

      /* Next, check the return value of the GAP Set Discoverability    */
      /* Mode command for successful execution.                         */
      if(!ret_val)
      {
         /* * NOTE * Discoverability is only applicable when we are     */
         /*          advertising so save the default Discoverability    */
         /*          Mode for later.                                    */
         LE_Parameters.DiscoverabilityMode = GapDiscMode;

      }
      else
      {
         /* An error occurred while trying to set the Discoverability   */
         /* Mode of the Device.                                         */
#ifdef CONSOLE_SUPPORT
        DisplayFunctionError("Set Discoverable Mode", ret_val);
#endif
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   if(ret_val == 0)
   {
     BTDiscMode = disc_mode;
     slogf(LOG_DEST_BOTH, "SetDisc(%d)", disc_mode);
   }
   
   return(ret_val);
}

   /* The following function is responsible for placing the Local       */
   /* Bluetooth Device into Connectable Mode.  Once in this mode the    */
   /* Device will respond to Page Scans from other Bluetooth Devices.   */
   /* This function requires that a valid Bluetooth Stack ID exists     */
   /* before running.  This function returns zero on success and a      */
   /* negative value if an error occurred.                              */
static int SetConnect(uint8_t connectable)
{
   int ret_val = 0;
   
   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to set the attached Device to be Connectable.          */
      if(connectable) {
         slogf(LOG_DEST_BOTH, "Enable connectability");
         ret_val = GAP_Set_Connectability_Mode(BluetoothStackID, cmConnectableMode);
      } else {
         slogf(LOG_DEST_BOTH, "Disable connectability");
         ret_val = GAP_Set_Connectability_Mode(BluetoothStackID, cmNonConnectableMode);
      }

      /* Next, check the return value of the                            */
      /* GAP_Set_Connectability_Mode() function for successful          */
      /* execution.                                                     */
      if(!ret_val)
      {
         /* * NOTE * Connectability is only an applicable when          */
         /*          advertising so we will just save the default       */
         /*          connectability for the next time we enable         */
         /*          advertising.                                       */
         LE_Parameters.ConnectableMode = lcmDirectConnectable;
      }
      else
      {
         /* An error occurred while trying to make the Device           */
         /* Connectable.                                                */
#ifdef CONSOLE_SUPPORT
         DisplayFunctionError("Set Connectability Mode", ret_val);
#endif
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for placing the local       */
   /* Bluetooth device into Pairable mode.  Once in this mode the device*/
   /* will response to pairing requests from other Bluetooth devices.   */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int SetPairable(int pairable)
{
   int Result;
   int ret_val = 0;
   GAP_Pairability_Mode_t GapPairMode;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
     if (pairable) {
       GapPairMode = pmPairableMode_EnableSecureSimplePairing;
     } else {
       GapPairMode = pmNonPairableMode;
     }
       
      /* Attempt to set the attached device to be pairable.             */
      Result = GAP_Set_Pairability_Mode(BluetoothStackID, GapPairMode);

      /* Next, check the return value of the GAP Set Pairability mode   */
      /* command for successful execution.                              */
      if(Result)
      {
         /* An error occurred while trying to make the device pairable. */
#ifdef CONSOLE_SUPPORT
         DisplayFunctionError("Set Pairability Mode", Result);
#endif
         ret_val = Result;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   if(ret_val == 0)
   {
     slogf(LOG_DEST_BOTH, "SetPairable(%d)", pairable);
     if(GapPairMode == pmPairableMode_EnableSecureSimplePairing)
     {
         DisplayIOCapabilities();
     }
   }
   return(ret_val);
}

   /* The following function is responsible for placing the local       */
   /* Bluetooth device into Pairable mode.  Once in this mode the device*/
   /* will response to pairing requests from other Bluetooth devices.   */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int SetPairableLE(int pairable)
{
   int Result;
   int ret_val = 0;
   GAP_LE_Pairability_Mode_t GapLEPairmode;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      if (pairable) {
       GapLEPairmode = lpmPairableMode;
      } else {
       GapLEPairmode = lpmNonPairableMode;
      }

      /* Attempt to set the attached device to be pairable.       */
      Result = GAP_LE_Set_Pairability_Mode(BluetoothStackID, GapLEPairmode);
      //Result = GAP_LE_Set_Pairability_Mode(ApplicationStateInfo.BluetoothStackID, GapLEPairmode);

      /* Next, check the return value of the GAP Set Pairability  */
      /* mode command for successful execution.                   */
      if(Result)
      {
         /* An error occurred while trying to make the device     */
         /* pairable.                                             */
         DisplayFunctionError("GAP_LE_Set_Pairability_Mode", Result);

         ret_val = Result;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   if(ret_val == 0)
   {
     slogf(LOG_DEST_BOTH, "SetPairableLE(%d)", pairable);
   }
   return(ret_val);
}

static int RegisterAuthentication(void) 
{
   int Result;
   int ret_val = 0;
   
   /* The device has been set to pairable mode, now register an   */
   /* Authentication Callback to handle the Authentication events */
   /* if required.                                                */
   Result = GAP_Register_Remote_Authentication(BluetoothStackID, GAP_Event_Callback, (unsigned long)0);

   /* Next, check the return value of the GAP Register Remote     */
   /* Authentication command for successful execution.            */
   if(!Result)
   {
      /* The device has been set to pairable mode, now register*/
      /* an Authentication Callback to handle the              */
      /* Authentication events if required.                    */
      Result = GAP_LE_Register_Remote_Authentication(BluetoothStackID, GAP_LE_Event_Callback, (unsigned long)0);

      /* Next, check the return value of the GAP Register      */
      /* Remote Authentication command for successful          */
      /* execution.                                            */
      if(Result)
      {
       /* An error occurred while trying to execute this     */
       /* function.                                          */
#ifdef CONSOLE_SUPPORT
       DisplayFunctionError("GAP_LE_Register_Remote_Authentication", Result);
#endif
       ret_val = Result;
      }
   }
   else
   {
      /* An error occurred while trying to execute this function. */
#ifdef CONSOLE_SUPPORT
      DisplayFunctionError("Auth", Result);
#endif
      ret_val = Result;
   }
      
   if(ret_val == 0)
   {
     slogf(LOG_DEST_BOTH, "Auth register status: %d", ret_val);
   }
   return(ret_val);
}
  

   /* The following function is a utility function that exists to delete*/
   /* the specified Link Key from the Local Bluetooth Device.  If a NULL*/
   /* Bluetooth Device Address is specified, then all Link Keys will be */
   /* deleted.                                                          */
int DeleteLinkKey(BD_ADDR_t BD_ADDR)
{
   int       Result;
   Byte_t    Status_Result;
   Word_t    Num_Keys_Deleted = 0;
   BD_ADDR_t NULL_BD_ADDR;
   FIL fp;
   FRESULT res;
   UINT read; 
   LinkKeyInfo_t BtKey;
   INT index = 0;

   Result = HCI_Delete_Stored_Link_Key(BluetoothStackID, BD_ADDR, TRUE, &Status_Result, &Num_Keys_Deleted);

   /* Any stored link keys for the specified address (or all) have been */
   /* deleted from the chip.  Now, let's make sure that our stored Link */
   /* Key Array is in sync with these changes.                          */

   /* First check to see all Link Keys were deleted.                    */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   if(COMPARE_BD_ADDR(BD_ADDR, NULL_BD_ADDR))
   {
      f_unlink(BT_KEY_FILENAME);
      BT_LinkedDeviceNb = 0;
      BTPaired = FALSE;
   }else
   {
      /* Individual Link Key.  Go ahead and see if know about the entry */
      /* in the list. 
     */
     eMMC_PowerOn();
      res = f_open(&fp, BT_KEY_FILENAME, FA_READ | FA_WRITE | FA_OPEN_EXISTING);
      if (res == FR_OK) 
      {
        do
        {
          res = f_read(&fp, (void *) &BtKey, sizeof(BtKey), &read);
          if(read == sizeof(BtKey))
          {
            if(COMPARE_BD_ADDR(BD_ADDR, BtKey.BD_ADDR))
            {
              f_lseek(&fp, index*sizeof(BtKey));
              f_write(&fp, (void *)&NULL_BD_ADDR, sizeof(BD_ADDR_t), &read);
              break;
            }
          }
          index++;
        }while(read == sizeof(BtKey));
        f_close(&fp);
      }
   }

   return(Result);
}

#ifdef CONSOLE_SUPPORT
   /* The following function is responsible for displaying the current  */
   /* Command Options for either Serial Port Client or Serial Port      */
   /* Server.  The input parameter to this function is completely       */
   /* ignored, and only needs to be passed in because all Commands that */
   /* can be entered at the Prompt pass in the parsed information.  This*/
   /* function displays the current Command Options that are available  */
   /* and always returns zero.                                          */
static int DisplayHelp(ParameterList_t *TempParam)
{
   if(UI_Mode == UI_MODE_IS_CLIENT) {
      Display(("\r\n"));
      Display(("******************************************************************\r\n"));
      Display(("* Command Options: Inquiry, DisplayInquiryList, Pair,            *\r\n"));
      Display(("*                  EndPairing, PINCodeResponse, PassKeyResponse, *\r\n"));
      Display(("*                  UserConfirmationResponse,                     *\r\n"));
      Display(("*                  SetDiscoverabilityMode, SetConnectabilityMode,*\r\n"));
      Display(("*                  SetPairabilityMode,                           *\r\n"));
      Display(("*                  ChangeSimplePairingParameters,                *\r\n"));
      Display(("*                  GetLocalAddress, GetLocalName, SetLocalName,  *\r\n"));
      Display(("*                  GetClassOfDevice, SetClassOfDevice,           *\r\n"));
      Display(("*                  GetRemoteName, SniffMode, ExitSniffMode,      *\r\n"));
      Display(("*                  Open, Close, Read, Write,                     *\r\n"));
      Display(("*                  GetConfigParams, SetConfigParams,             *\r\n"));
      Display(("*                  GetQueueParams, SetQueueParams,               *\r\n"));
      Display(("*                  Loopback, DisplayRawModeData,                 *\r\n"));
      Display(("*                  AutomaticReadMode, SetBaudRate, Send          *\r\n"));
      Display(("*                  Help, Quit                                    *\r\n"));
      Display(("******************************************************************\r\n"));
   } else if(UI_Mode == UI_MODE_IS_SERVER) {
       Display(("\r\n"));
       Display(("******************************************************************\r\n"));
       Display(("* Command Options: Inquiry, DisplayInquiryList, Pair,            *\r\n"));
       Display(("*                  EndPairing, PINCodeResponse, PassKeyResponse, *\r\n"));
       Display(("*                  UserConfirmationResponse,                     *\r\n"));
       Display(("*                  SetDiscoverabilityMode, SetConnectabilityMode,*\r\n"));
       Display(("*                  SetPairabilityMode,                           *\r\n"));
       Display(("*                  ChangeSimplePairingParameters,                *\r\n"));
       Display(("*                  GetLocalAddress, GetLocalName, SetLocalName,  *\r\n"));
       Display(("*                  GetClassOfDevice, SetClassOfDevice,           *\r\n"));
       Display(("*                  GetRemoteName, SniffMode, ExitSniffMode,      *\r\n"));
       Display(("*                  Open, Close, Read, Write,                     *\r\n"));
       Display(("*                  GetConfigParams, SetConfigParams,             *\r\n"));
       Display(("*                  GetQueueParams, SetQueueParams,               *\r\n"));
       Display(("*                  Loopback, DisplayRawModeData,                 *\r\n"));
       Display(("*                  AutomaticReadMode, SetBaudRate, Send          *\r\n"));
       Display(("*                  Help, Quit                                    *\r\n"));
       Display(("******************************************************************\r\n"));
   } else if(UI_Mode == UI_MODE_IS_PRODTOOLS) {
       Display(("\r\n"));
       Display(("******************************************************************\r\n"));
       Display(("* Command Options: Format, Free, FirmwareUpdate,                 *\r\n"));
       Display(("*                  Help, Quit                                    *\r\n"));
       Display(("******************************************************************\r\n"));
    } else {
       Display(("\r\n"));
       Display(("******************************************************************\r\n"));
       Display(("* Command Options: Server, Client, Prod, Help                    *\r\n"));
       Display(("******************************************************************\r\n"));
    }

   return(0);
}

   /* The following function is responsible for performing a General    */
   /* Inquiry for discovering Bluetooth Devices.  This function requires*/
   /* that a valid Bluetooth Stack ID exists before running.  This      */
   /* function returns zero is successful or a negative value if there  */
   /* was an error.                                                     */
static int Inquiry(ParameterList_t *TempParam)
{
   int Result;
   int ret_val = 0;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Use the GAP_Perform_Inquiry() function to perform an Inquiry.  */
      /* The Inquiry will last 10 seconds or until MAX_INQUIRY_RESULTS  */
      /* Bluetooth Devices are found.  When the Inquiry Results become  */
      /* available the GAP_Event_Callback is called.                    */
      Result = GAP_Perform_Inquiry(BluetoothStackID, itGeneralInquiry, 0, 0, 10, MAX_INQUIRY_RESULTS, GAP_Event_Callback, (unsigned long)NULL);

      /* Next, check to see if the GAP_Perform_Inquiry() function was   */
      /* successful.                                                    */
      if(!Result)
      {
         /* The Inquiry appears to have been sent successfully.         */
         /* Processing of the results returned from this command occurs */
         /* within the GAP_Event_Callback() function.                   */

         /* Flag that we have found NO Bluetooth Devices.               */
         NumberofValidResponses = 0;

         ret_val                = 0;
      }
      else
      {
         /* A error occurred while performing the Inquiry.              */
         DisplayFunctionError("Inquiry", Result);

         ret_val = Result;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* display the current Inquiry List (with Indexes).  This is useful  */
   /* in case the user has forgotten what Inquiry Index a particular    */
   /* Bluteooth Device was located in.  This function returns zero on   */
   /* successful execution and a negative value on all errors.          */
static int DisplayInquiryList(ParameterList_t *TempParam)
{
   int          ret_val = 0;
   unsigned int Index;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Simply display all of the items in the Inquiry List.           */
      Display(("Inquiry List: %d Devices%s\r\n\r\n", NumberofValidResponses, NumberofValidResponses?":":"."));

      for(Index=0;Index<NumberofValidResponses;Index++)
      {
         BD_ADDRToStr(InquiryResultList[Index], Function_BoardStr);

         Display((" Inquiry Result: %d, %s.\r\n", (Index+1), Function_BoardStr));
      }

      if(NumberofValidResponses)
         Display(("\r\n"));

      /* All finished, flag success to the caller.                      */
      ret_val = 0;
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the             */
   /* Discoverability Mode of the local device.  This function returns  */
   /* zero on successful execution and a negative value on all errors.  */
static int SetDiscoverabilityMode(ParameterList_t *TempParam)
{
   int                        Result;
   int                        ret_val;
   GAP_Discoverability_Mode_t DiscoverabilityMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 2))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 1)
            DiscoverabilityMode = dmLimitedDiscoverableMode;
         else
         {
            if(TempParam->Params[0].intParam == 2)
               DiscoverabilityMode = dmGeneralDiscoverableMode;
            else
               DiscoverabilityMode = dmNonDiscoverableMode;
         }

         /* Parameters mapped, now set the Discoverability Mode.        */
         Result = GAP_Set_Discoverability_Mode(BluetoothStackID, DiscoverabilityMode, (DiscoverabilityMode == dmLimitedDiscoverableMode)?60:0);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            slogf(LOG_DEST_BOTH, "Discoverability: %s.", (DiscoverabilityMode == dmNonDiscoverableMode)?"Non":((DiscoverabilityMode == dmGeneralDiscoverableMode)?"General":"Limited"));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            DisplayFunctionError("GAP_Set_Discoverability_Mode", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetDiscoverabilityMode [Mode(0 = Non Discoverable, 1 = Limited Discoverable, 2 = General Discoverable)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the             */
   /* Connectability Mode of the local device.  This function returns   */
   /* zero on successful execution and a negative value on all errors.  */
static int SetConnectabilityMode(ParameterList_t *TempParam)
{
   int                       Result;
   int                       ret_val;
   GAP_Connectability_Mode_t ConnectableMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 1))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
            ConnectableMode = cmNonConnectableMode;
         else
            ConnectableMode = cmConnectableMode;

         /* Parameters mapped, now set the Connectabilty Mode.          */
         Result = GAP_Set_Connectability_Mode(BluetoothStackID, ConnectableMode);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            slogf(LOG_DEST_BOTH, "Connectability Mode: %s.", (ConnectableMode == cmNonConnectableMode)?"Non Connectable":"Connectable");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            DisplayFunctionError("GAP_Set_Connectability_Mode", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetConnectabilityMode [(0 = NonConectable, 1 = Connectable)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Pairability */
   /* Mode of the local device.  This function returns zero on          */
   /* successful execution and a negative value on all errors.          */
static int SetPairabilityMode(ParameterList_t *TempParam)
{
   int                     Result;
   int                     ret_val;
   char                   *Mode;
   GAP_Pairability_Mode_t  PairabilityMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 2))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
         {
            PairabilityMode = pmNonPairableMode;
            Mode            = "pmNonPairableMode";
         }
         else
         {
            if(TempParam->Params[0].intParam == 1)
            {
               PairabilityMode = pmPairableMode;
               Mode            = "pmPairableMode";
            }
            else
            {
               PairabilityMode = pmPairableMode_EnableSecureSimplePairing;
               Mode            = "pmPairableMode_EnableSecureSimplePairing";
            }
         }

         /* Parameters mapped, now set the Pairability Mode.            */
         Result = GAP_Set_Pairability_Mode(BluetoothStackID, PairabilityMode);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            Display(("Pairability Mode Changed to %s.\r\n", Mode));

            /* If Secure Simple Pairing has been enabled, inform the    */
            /* user of the current Secure Simple Pairing parameters.    */
            if(PairabilityMode == pmPairableMode_EnableSecureSimplePairing)
               DisplayIOCapabilities();

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            DisplayFunctionError("GAP_Set_Pairability_Mode", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetPairabilityMode [Mode (0 = Non Pairable, 1 = Pairable, 2 = Pairable (Secure Simple Pairing)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for changing the Secure     */
   /* Simple Pairing Parameters that are exchanged during the Pairing   */
   /* procedure when Secure Simple Pairing (Security Level 4) is used.  */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int ChangeSimplePairingParameters(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 3))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
            IOCapability = icDisplayOnly;
         else
         {
            if(TempParam->Params[0].intParam == 1)
               IOCapability = icDisplayYesNo;
            else
            {
               if(TempParam->Params[0].intParam == 2)
                  IOCapability = icKeyboardOnly;
               else
                  IOCapability = icNoInputNoOutput;
            }
         }

         /* Finally map the Man in the Middle (MITM) Protection valid.  */
         MITMProtection = (Boolean_t)(TempParam->Params[1].intParam?TRUE:FALSE);

         /* Inform the user of the New I/O Capabilities.                */
         DisplayIOCapabilities();

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("ChangeSimplePairingParameters [I/O Capability (0 = Display Only, 1 = Display Yes/No, 2 = Keyboard Only, 3 = No Input/Output)] [MITM Requirement (0 = No, 1 = Yes)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for initiating bonding with */
   /* a remote device.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int Pair(ParameterList_t *TempParam)
{
   int                Result;
   int                ret_val;
   GAP_Bonding_Type_t BondingType;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that we are not already connected.             */
      if(!Connected)
      {
         /* There are currently no active connections, make sure that   */
         /* all of the parameters required for this function appear to  */
         /* be at least semi-valid.                                     */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
         {
            /* Check to see if General Bonding was specified.           */
            if(TempParam->NumberofParameters > 1)
               BondingType = TempParam->Params[1].intParam?btGeneral:btDedicated;
            else
               BondingType = btDedicated;

            /* Before we submit the command to the stack, we need to    */
            /* make sure that we clear out any Link Key we have stored  */
            /* for the specified device.                                */
            DeleteLinkKey(InquiryResultList[(TempParam->Params[0].intParam - 1)]);

            /* Attempt to submit the command.                           */
            Result = GAP_Initiate_Bonding(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], BondingType, GAP_Event_Callback, (unsigned long)0);

            /* Check the return value of the submitted command for      */
            /* success.                                                 */
            if(!Result)
            {
               /* Display a message indicating that Bonding was         */
               /* initiated successfully.                               */
               Display(("GAP_Initiate_Bonding(%s): Success.\r\n", (BondingType == btDedicated)?"Dedicated":"General"));

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Display a message indicating that an error occurred   */
               /* while initiating bonding.                             */
               DisplayFunctionError("GAP_Initiate_Bonding", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            DisplayUsage("Pair [Inquiry Index] [0 = Dedicated, 1 = General (optional)]");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* Display an error to the user describing that Pairing can    */
         /* only occur when we are not connected.                       */
         Display(("Only valid when not connected.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for ending a previously     */
   /* initiated bonding session with a remote device.  This function    */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int EndPairing(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to submit the command.                              */
         Result = GAP_End_Bonding(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)]);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a message indicating that the End bonding was    */
            /* successfully submitted.                                  */
            DisplayFunctionSuccess("GAP_End_Bonding");

            /* Flag success to the caller.                              */
            ret_val = 0;

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* ending bonding.                                          */
            DisplayFunctionError("GAP_End_Bonding", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("EndPairing [Inquiry Index]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}
#endif // CONSOLE_SUPPORt

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a PIN Code value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int PINCodeResponse(ParameterList_t *TempParam)
{
   int                              Result;
   int                              ret_val;
   PIN_Code_t                       PINCode;
   GAP_Authentication_Information_t GAP_Authentication_Information;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_BD_ADDR(CurrentRemoteBD_ADDR, NullADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam) && (BTPS_StringLength(TempParam->Params[0].strParam) > 0) && (BTPS_StringLength(TempParam->Params[0].strParam) <= sizeof(PIN_Code_t)))
         {
            /* Parameters appear to be valid, go ahead and convert the  */
            /* input parameter into a PIN Code.                         */

            /* Initialize the PIN code.                                 */
            ASSIGN_PIN_CODE(PINCode, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

            BTPS_MemCopy(&PINCode, TempParam->Params[0].strParam, BTPS_StringLength(TempParam->Params[0].strParam));

            /* Populate the response structure.                         */
            GAP_Authentication_Information.GAP_Authentication_Type      = atPINCode;
            GAP_Authentication_Information.Authentication_Data_Length   = (Byte_t)(BTPS_StringLength(TempParam->Params[0].strParam));
            GAP_Authentication_Information.Authentication_Data.PIN_Code = PINCode;

            /* Submit the Authentication Response.                      */
            Result = GAP_Authentication_Response(BluetoothStackID, CurrentRemoteBD_ADDR, &GAP_Authentication_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               Display(("GAP_Authentication_Response(), Pin Code Response Success.\r\n"));

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               Display(("GAP_Authentication_Response() Failure: %d.\r\n", Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
#ifdef CONSOLE_SUPPORT
           DisplayUsage("PINCodeResponse [PIN Code]");
#endif // CONSOLE_SUPPORT

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         Display(("PIN Code Authentication Response: Authentication not in progress.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}


#ifdef CONSOLE_SUPPORT

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a Pass Key value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int PassKeyResponse(ParameterList_t *TempParam)
{
   int                              Result;
   int                              ret_val;
   GAP_Authentication_Information_t GAP_Authentication_Information;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_BD_ADDR(CurrentRemoteBD_ADDR, NullADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (BTPS_StringLength(TempParam->Params[0].strParam) <= GAP_PASSKEY_MAXIMUM_NUMBER_OF_DIGITS))
         {
            /* Parameters appear to be valid, go ahead and populate the */
            /* response structure.                                      */
            GAP_Authentication_Information.GAP_Authentication_Type     = atPassKey;
            GAP_Authentication_Information.Authentication_Data_Length  = (Byte_t)(sizeof(DWord_t));
            GAP_Authentication_Information.Authentication_Data.Passkey = (DWord_t)(TempParam->Params[0].intParam);

            /* Submit the Authentication Response.                      */
            Result = GAP_Authentication_Response(BluetoothStackID, CurrentRemoteBD_ADDR, &GAP_Authentication_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               DisplayFunctionSuccess("Passkey Response Success");
               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               DisplayFunctionError("GAP_Authentication_Response", Result);
               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            Display(("PassKeyResponse[Numeric Passkey(0 - 999999)].\r\n"));

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         Display(("Pass Key Authentication Response: Authentication not in progress.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a User Confirmation value specified  */
   /* via the input parameter.  This function returns zero on successful*/
   /* execution and a negative value on all errors.                     */
static int UserConfirmationResponse(ParameterList_t *TempParam)
{
   int                              Result;
   int                              ret_val;
   GAP_Authentication_Information_t GAP_Authentication_Information;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_BD_ADDR(CurrentRemoteBD_ADDR, NullADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Parameters appear to be valid, go ahead and populate the */
            /* response structure.                                      */
            GAP_Authentication_Information.GAP_Authentication_Type          = atUserConfirmation;
            GAP_Authentication_Information.Authentication_Data_Length       = (Byte_t)(sizeof(Byte_t));
            GAP_Authentication_Information.Authentication_Data.Confirmation = (Boolean_t)(TempParam->Params[0].intParam?TRUE:FALSE);

            /* Submit the Authentication Response.                      */
            Result = GAP_Authentication_Response(BluetoothStackID, CurrentRemoteBD_ADDR, &GAP_Authentication_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               DisplayFunctionSuccess("User Confirmation Response");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               DisplayFunctionError("GAP_Authentication_Response", Result);

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            DisplayUsage("UserConfirmationResponse [Confirmation(0 = No, 1 = Yes)]");
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         Display(("User Confirmation Authentication Response: Authentication is not currently in progress.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Device Address of the local Bluetooth Device.  This function      */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int GetLocalAddress(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Attempt to submit the command.                                 */
      Result = GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR);

      /* Check the return value of the submitted command for success.   */
      if(!Result)
      {
         BD_ADDRToStr(BD_ADDR, Function_BoardStr);

         Display(("BD_ADDR of Local Device is: %s.\r\n", Function_BoardStr));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Display a message indicating that an error occurred while   */
         /* attempting to query the Local Device Address.               */
         Display(("GAP_Query_Local_BD_ADDR() Failure: %d.\r\n", Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the name of the */
   /* local Bluetooth Device to a specified name.  This function returns*/
   /* zero on successful execution and a negative value on all errors.  */
static int SetLocalName(ParameterList_t *TempParam)
{
   int Result;
   int ret_val = 0;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam))
      {
         /* Attempt to submit the command.                              */
         Result = GAP_Set_Local_Device_Name(BluetoothStackID, TempParam->Params[0].strParam);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a message indicating that the Device Name was    */
            /* successfully submitted.                                  */
            Display(("Local Device Name: %s.\r\n", TempParam->Params[0].strParam));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* attempting to set the local Device Name.                 */
            DisplayFunctionError("GAP_Set_Local_Device_Name", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetLocalName [Local Name]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Device Name of the local Bluetooth Device.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int GetLocalName(ParameterList_t *TempParam)
{
   int   Result;
   int   ret_val;
   char *LocalName;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Allocate a Buffer to hold the Local Name.                      */
      if((LocalName = BTPS_AllocateMemory(257)) != NULL)
      {
         /* Attempt to submit the command.                              */
         Result = GAP_Query_Local_Device_Name(BluetoothStackID, 257, (char *)LocalName);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            Display(("Name of Local Device is: %s.\r\n", LocalName));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* attempting to query the Local Device Name.               */
            Display(("GAP_Query_Local_Device_Name() Failure: %d.\r\n", Result));

            ret_val = FUNCTION_ERROR;
         }

         BTPS_FreeMemory(LocalName);
      }
      else
      {
         Display(("Failed to allocate buffer to hold Local Name.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Class of    */
   /* Device of the local Bluetooth Device to a Class Of Device value.  */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int SetClassOfDevice(ParameterList_t *TempParam)
{
   int               Result;
   int               ret_val;
   Class_of_Device_t Class_of_Device;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to submit the command.                              */
         ASSIGN_CLASS_OF_DEVICE(Class_of_Device, (Byte_t)((TempParam->Params[0].intParam) & 0xFF), (Byte_t)(((TempParam->Params[0].intParam) >> 8) & 0xFF), (Byte_t)(((TempParam->Params[0].intParam) >> 16) & 0xFF));

         Result = GAP_Set_Class_Of_Device(BluetoothStackID, Class_of_Device);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a message indicating that the Class of Device was*/
            /* successfully submitted.                                  */
            DisplayClassOfDevice(Class_of_Device);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* attempting to set the local Class of Device.             */
            DisplayFunctionError("GAP_Set_Class_Of_Device", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetClassOfDevice [Class of Device]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Class of Device of the local Bluetooth Device.  This function     */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int GetClassOfDevice(ParameterList_t *TempParam)
{
   int               Result;
   int               ret_val;
   Class_of_Device_t Class_of_Device;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Attempt to submit the command.                                 */
      Result = GAP_Query_Class_Of_Device(BluetoothStackID, &Class_of_Device);

      /* Check the return value of the submitted command for success.   */
      if(!Result)
      {
         Display(("Local Class of Device is: 0x%02X%02X%02X.\r\n", Class_of_Device.Class_of_Device0, Class_of_Device.Class_of_Device1, Class_of_Device.Class_of_Device2));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Display a message indicating that an error occurred while   */
         /* attempting to query the Local Class of Device.              */
         Display(("GAP_Query_Class_Of_Device() Failure: %d.\r\n", Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Device Name of the specified remote Bluetooth Device.  This       */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int GetRemoteName(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to submit the command.                              */
         Result = GAP_Query_Remote_Device_Name(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], GAP_Event_Callback, (unsigned long)0);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a message indicating that Remote Name request was*/
            /* initiated successfully.                                  */
            DisplayFunctionSuccess("GAP_Query_Remote_Device_Name");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* initiating the Remote Name request.                      */
            DisplayFunctionError("GAP_Query_Remote_Device_Name", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("GetRemoteName [Inquiry Index]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for putting a specified     */
   /* connection into HCI Sniff Mode with passed in parameters.         */
static int SniffMode(ParameterList_t *TempParam)
{
   Word_t Sniff_Max_Interval;
   Word_t Sniff_Min_Interval;
   Word_t Sniff_Attempt;
   Word_t Sniff_Timeout;
   Byte_t Status;
   int       Result;
   int       ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 4))
      {
         Sniff_Max_Interval   = (Word_t)MILLISECONDS_TO_BASEBAND_SLOTS(TempParam->Params[0].intParam);
         Sniff_Min_Interval   = (Word_t)MILLISECONDS_TO_BASEBAND_SLOTS(TempParam->Params[1].intParam);
         Sniff_Attempt        = TempParam->Params[2].intParam;
         Sniff_Timeout        = TempParam->Params[3].intParam;

         /* Make sure the Sniff Mode parameters seem semi valid.        */
         if((Sniff_Attempt) && (Sniff_Max_Interval) && (Sniff_Min_Interval) && (Sniff_Min_Interval < Sniff_Max_Interval))
         {
            /* Make sure the connection handle is valid.                */
            if(Connection_Handle)
            {
               /* Now that we have the connection try and go to Sniff.  */
               Result = HCI_Sniff_Mode(BluetoothStackID, Connection_Handle, Sniff_Max_Interval, Sniff_Min_Interval, Sniff_Attempt, Sniff_Timeout, &Status);
               if(!Result)
               {
                  /* Check the Command Status.                          */
                  if(!Status)
                  {
                     DisplayFunctionSuccess("HCI_Sniff_Mode()");

                     /* Return success to the caller.                   */
                     ret_val = 0;
                  }
                  else
                  {
                     Display(("HCI_Sniff_Mode() Returned Non-Zero Status: 0x%02X\r\n", Status));

                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  DisplayFunctionError("HCI_Sniff_Mode()", Result);

                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               Display(("Invalid Connection Handle.\r\n"));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            DisplayUsage("SniffMode [MaxInterval] [MinInterval] [Attempt] [Timeout]");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SniffMode [MaxInterval] [MinInterval] [Attempt] [Timeout]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for attempting to Exit      */
   /* Sniff Mode for a specified connection.                            */
static int ExitSniffMode(ParameterList_t *TempParam)
{
   Byte_t    Status;
   int    Result;
   int    ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((Connection_Handle) && (TempParam))
      {
         /* Attempt to Exit Sniff Mode for the Specified Device.        */
         Result = HCI_Exit_Sniff_Mode(BluetoothStackID, Connection_Handle, &Status);
         if(!Result)
         {
            if(!Status)
            {
               /* Flag that HCI_Exit_Sniff_Mode was successfull.        */
               DisplayFunctionSuccess("HCI_Exit_Sniff_Mode()");

               ret_val = 0;
            }
            else
            {
               /* We received a failure status in the Command Status    */
               Display(("HCI_Exit_Sniff_Mode() Returned Non-Zero Status: 0x%02X.\r\n", Status));
               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* Failed to get exit sniff mode.                           */
            DisplayFunctionError("HCI_Exit_Sniff_Mode()", Result);

            /* Return function error to the caller.                     */
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("ExitSniffMode");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}
#endif // CONSOLE_SUPORT



/* The following function is responsible for opening a Serial Port   */
   /* Server on the Local Device.  This function opens the Serial Port  */
   /* Server on the specified RFCOMM Channel.  This function returns    */
   /* zero if successful, or a negative return value if an error        */
   /* occurred.                                                         */
static int OpenServer(ParameterList_t *TempParam)
{
   int  ret_val;
   char *ServiceName;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Make sure that there is not already a Serial Port Server open. */
      if(!ServerPortID)
      {
         /* Next, check to see if the parameters specified are valid.   */
         if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam))
         {
            /* Simply attempt to open an Serial Server, on RFCOMM Server*/
            /* Port 1.                                                  */
            slogf(LOG_DEST_BOTH, "Opening RFCOMM server on port: %d", TempParam->Params[0].intParam);
            ret_val = SPP_Open_Server_Port(BluetoothStackID, TempParam->Params[0].intParam, SPP_Event_Callback, (unsigned long)0);

            /* If the Open was successful, then note the Serial Port    */
            /* Server ID.                                               */
            if(ret_val > 0)
            {
               /* Note the Serial Port Server ID of the opened Serial   */
               /* Port Server.                                          */
               ServerPortID = ret_val;

               /* Create a Buffer to hold the Service Name.             */
               if((ServiceName = BTPS_AllocateMemory(64)) != NULL)
               {
                  /* The Server was opened successfully, now register a */
                  /* SDP Record indicating that an Serial Port Server   */
                  /* exists. Do this by first creating a Service Name.  */
                  BTPS_SprintF(ServiceName, "Serial Port Server Port %d", (int)(TempParam->Params[0].intParam));

                  /* Now that a Service Name has been created try to    */
                  /* Register the SDP Record.                           */
                  ret_val = SPP_Register_Generic_SDP_Record(BluetoothStackID, ServerPortID, ServiceName, &SPPServerSDPHandle);

                  /* If there was an error creating the Serial Port     */
                  /* Server's SDP Service Record then go ahead an close */
                  /* down the server an flag an error.                  */
                  if(ret_val < 0)
                  {
                     Display(("Unable to Register Server SDP Record, Error = %d.\r\n", ret_val));

                     SPP_Close_Server_Port(BluetoothStackID, ServerPortID);

                     /* Flag that there is no longer an Serial Port     */
                     /* Server Open.                                    */
                     ServerPortID = 0;

                     /* Flag that we are no longer connected.           */
                     Connected    = FALSE;
                     SPPOpened = FALSE;

                     ret_val      = UNABLE_TO_REGISTER_SERVER;
                  }
                  else
                  {
                     /* Simply flag to the user that everything         */
                     /* initialized correctly.                          */
                     slogf(LOG_DEST_BOTH, "RFCOMM server opened on port: %d.", TempParam->Params[0].intParam);

                     /* Flag success to the caller.                     */
                     ret_val = 0;
                  }

                  /* Free the Service Name buffer.                      */
                  BTPS_FreeMemory(ServiceName);
               }
               else
               {
                  Display(("Failed to allocate buffer to hold Service Name in SDP Record.\r\n"));
               }
            }
            else
            {
               slogf(LOG_DEST_BOTH, "Unable to Open Server on port: %d, Error = %d.", TempParam->Params[0].intParam, ret_val);

               ret_val = UNABLE_TO_REGISTER_SERVER;
            }
         }
         else
         {
#ifdef CONSOLE_SUPPORT
            DisplayUsage("Open [Port Number]");
#endif
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* A Server is already open, this program only supports one    */
         /* Server or Client at a time.                                 */
         Display(("Server already open.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

#ifdef CONSOLE_SUPPORT

   /* The following function is responsible for closing a Serial Port   */
   /* Server that was previously opened via a successful call to the    */
   /* OpenServer() function.  This function returns zero if successful  */
   /* or a negative return error code if there was an error.            */
static int CloseServer(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* If a Serial Port Server is already opened, then simply close   */
      /* it.                                                            */
      if(ServerPortID)
      {
         /* If there is an SDP Service Record associated with the Serial*/
         /* Port Server then we need to remove it from the SDP Database.*/
         if(SPPServerSDPHandle)
         {
            SPP_Un_Register_SDP_Record(BluetoothStackID, SerialPortID, SPPServerSDPHandle);

            /* Flag that there is no longer an SDP Serial Port Server   */
            /* Record.                                                  */
            SPPServerSDPHandle = 0;
         }

         /* Finally close the Serial Port Server.                       */
         ret_val = SPP_Close_Server_Port(BluetoothStackID, ServerPortID);

         if(ret_val < 0)
         {
            DisplayFunctionError("SPP_Close_Server_Port", ret_val);

            ret_val = FUNCTION_ERROR;
         }
         else
            ret_val = 0;

         /* Flag that there is no Serial Port Server currently open.    */
         ServerPortID         = 0;
         SerialPortID = 0;
         SendInfo.BytesToSend = 0;

         /* Flag that we are no longer connected.                       */
         Connected    = FALSE;
         SPPOpened = FALSE;

         slogf(LOG_DEST_BOTH, "RFCOMM Server Closed.");
      }
      else
      {
         Display(("NO Server open.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for initiating a connection */
   /* with a Remote Serial Port Server.  This function returns zero if  */
   /* successful and a negative value if an error occurred.             */
static int OpenRemoteServer(ParameterList_t *TempParam)
{
   int ret_val;
   int Result;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, check that we are in client mode.                        */
      if(UI_Mode == UI_MODE_IS_CLIENT)
      {
         /* Next, let's make sure that there is not an Serial Port      */
         /* Client already open.                                        */
         if(!SerialPortID)
         {
            /* Next, let's make sure that the user has specified a      */
            /* Remote Bluetooth Device to open.                         */
            if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)) && (TempParam->Params[1].intParam))
            {
               /* Now let's attempt to open the Remote Serial Port      */
               /* Server.                                               */
               Result = SPP_Open_Remote_Port(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], TempParam->Params[1].intParam, SPP_Event_Callback, (unsigned long)0);

               if(Result > 0)
               {
                  /* Inform the user that the call to open the Remote   */
                  /* Serial Port Server was successful.                 */
                  DisplayFunctionSuccess("SPP_Open_Remote_Port");

                  /* Note the Serial Port Client ID.                    */
                  SerialPortID = Result;

                  /* Flag success to the caller.                        */
                  ret_val = 0;

                  /* Save the BD_ADDR so we can Query the Connection    */
                  /* handle when receive Connection Confirmation Event. */
                  SelectedBD_ADDR = InquiryResultList[(TempParam->Params[0].intParam - 1)];
               }
               else
               {
                  /* Inform the user that the call to Open the Remote   */
                  /* Serial Port Server failed.                         */
                  DisplayFunctionError("SPP_Open_Remote_Port", Result);

                  /* One or more of the necessary parameters is/are     */
                  /* invalid.                                           */
                  ret_val = INVALID_PARAMETERS_ERROR;
               }
            }
            else
            {
               DisplayUsage("Open [Inquiry Index] [RFCOMM Server Port].\r\n");

               /* One or more of the necessary parameters is/are        */
               /* invalid.                                              */
               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            Display(("Unable to open port.\r\n"));

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for terminating a connection*/
   /* with a Remote Serial Port Server.  This function returns zero if  */
   /* successful and a negative value if an error occurred.             */
static int CloseRemoteServer(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, check that we are in client mode.                        */
      if(UI_Mode == UI_MODE_IS_CLIENT)
      {
         /* Next, let's make sure that a Remote Serial Port Server is   */
         /* indeed Open.                                                */
         if(SerialPortID)
         {
            Display(("Client Port closed.\r\n"));

            /* Simply close the Serial Port Client.                     */
            SPP_Close_Port(BluetoothStackID, SerialPortID);

            /* Flag that there is no longer a Serial Port Client        */
            /* connected.                                               */
            SerialPortID = 0;
            SendInfo.BytesToSend = 0;

            /* Flag that we are no longer connected.                    */
            Connected    = FALSE;

            /* Flag success to the caller.                              */
            ret_val      = 0;
         }
         else
         {
            /* Display an error to the user informing them that no      */
            /* Serial Port Client is open.                              */
            Display(("No Open Client Port.\r\n"));

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading data that was   */
   /* received via an Open SPP port.  The function reads a fixed number */
   /* of bytes at a time from the SPP Port and displays it. If the call */
   /* to the SPP_Data_Read() function is successful but no data is      */
   /* available to read the function displays "No data to read.".  This */
   /* function requires that a valid Bluetooth Stack ID and Serial Port */
   /* ID exist before running.  This function returns zero if successful*/
   /* and a negative value if an error occurred.                        */
static int Read(ParameterList_t *TempParam)
{
   int  ret_val;
   int  Result;
   char Buffer[32];

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if((BluetoothStackID) && (SerialPortID))
   {
      /* Only allow the Read Command if we are not in Loopback or       */
      /* Display Raw Data Mode.                                         */
      if((!LoopbackActive) && (!DisplayRawData))
      {
         /* The required parameters appear to be semi-valid, send the   */
         /* command to Read Data from SPP.                              */
         do
         {
            Result = SPP_Data_Read(BluetoothStackID, SerialPortID, (Word_t)(sizeof(Buffer)-1), (Byte_t*)&Buffer);

            /* Next, check the return value to see if the command was   */
            /* successfully.                                            */
            if(Result >= 0)
            {
               /* Null terminate the read data.                         */
               Buffer[Result] = 0;

               /* Data was read successfully, the result indicates the  */
               /* of bytes that were successfully Read.                 */
               Display(("Read: %d.\r\n", Result));

               if(Result > 0)
                  Display(("Message: %s\r\n", Buffer));

               ret_val = 0;
            }
            else
            {
               /* An error occurred while reading from SPP.             */
               DisplayFunctionError("SPP_Data_Read Failure", Result);

               ret_val = Result;
            }
         } while(Result > 0);
      }
      else
      {
         /* Simply inform the user that this command is not available in*/
         /* this mode.                                                  */
         Display(("This operation cannot be performed while in Loopback Mode or while Displaying Raw Data.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}


   /* The following function is responsible for Writing Data to an Open */
   /* SPP Port.  The string that is written is defined by the constant  */
   /* TEST_DATA (at the top of this file).  This function requires that */
   /* a valid Bluetooth Stack ID and Serial Port ID exist before        */
   /* running.  This function returns zero is successful or a negative  */
   /* return value if there was an error.                               */
#if 0
static int Write(ParameterList_t *TempParam)
{
   int  ret_val;
   int  Result;
   uint32_t total_count = 0;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if((BluetoothStackID) && (SerialPortID))
   {
     while (1) {
       Word_t len;
       FTPActivity++;
     /* Simply write out the default string value.                     */
      len = (Word_t)BTPS_StringLength(TEST_DATA);
      Result = SPP_Data_Write(BluetoothStackID, SerialPortID, len, (Byte_t *)TEST_DATA);
      /* Next, check the return value to see if the command was issued  */
      /* successfully.                                                  */
      if(Result >= 0)
      {
         /* The Data was written successfully, Result indicates the     */
         /* number of bytes successfully written.                       */
        total_count += Result;
         Display(("Count: %d.\r\n", total_count));
         //BTPS_Delay(10);

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else if (Result < 0)
      {
         /* There was an error writing the Data to the SPP Port.        */
         Display(("Failed: %d.\r\n", Result));

         /* Flag that an error occurred while submitting the command.   */
         ret_val = FUNCTION_ERROR;
         break;
      }
     }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

#else
static int Write(ParameterList_t *TempParam)
{
   int  ret_val;
   int  Result;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if((BluetoothStackID) && (SerialPortID))
   {
     /* Simply write out the default string value.                     */
      Result = SPP_Data_Write(BluetoothStackID, SerialPortID, (Word_t)BTPS_StringLength(TEST_DATA), (Byte_t *)TEST_DATA);
      /* Next, check the return value to see if the command was issued  */
      /* successfully.                                                  */
      if(Result >= 0)
      {
         /* The Data was written successfully, Result indicates the     */
         /* number of bytes successfully written.                       */
         Display(("Wrote: %d.\r\n", Result));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* There was an error writing the Data to the SPP Port.        */
         Display(("Failed: %d.\r\n", Result));

         /* Flag that an error occurred while submitting the command.   */
         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}
#endif
   /* The following function is responsible for querying the current    */
   /* configuration parameters that are used by SPP.  This function will*/
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int GetConfigParams(ParameterList_t *TempParam)
{
   int                        ret_val;
   SPP_Configuration_Params_t SPPConfigurationParams;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Simply query the configuration parameters.                     */
      ret_val = SPP_Get_Configuration_Parameters(BluetoothStackID, &SPPConfigurationParams);

      if(ret_val >= 0)
      {
         /* Parameters have been queried successfully, go ahead and     */
         /* notify the user.                                            */
         Display(("SPP_Get_Configuration_Parameters(): Success\r\n", ret_val));
         Display(("   MaximumFrameSize   : %d (0x%X)\r\n", SPPConfigurationParams.MaximumFrameSize, SPPConfigurationParams.MaximumFrameSize));
         Display(("   TransmitBufferSize : %d (0x%X)\r\n", SPPConfigurationParams.TransmitBufferSize, SPPConfigurationParams.TransmitBufferSize));
         Display(("   ReceiveBufferSize  : %d (0x%X)\r\n", SPPConfigurationParams.ReceiveBufferSize, SPPConfigurationParams.ReceiveBufferSize));

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error querying the current parameters.                      */
         Display(("SPP_Get_Configuration_Parameters(): Error %d.\r\n", ret_val));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the current     */
   /* configuration parameters that are used by SPP.  This function will*/
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int SetConfigParams(ParameterList_t *TempParam)
{
   int                        ret_val;
   SPP_Configuration_Params_t SPPConfigurationParams;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((TempParam) && (TempParam->NumberofParameters > 2))
      {
         /* Parameters have been specified, go ahead and write them to  */
         /* the stack.                                                  */
         SPPConfigurationParams.MaximumFrameSize   = (unsigned int)(TempParam->Params[0].intParam);
         SPPConfigurationParams.TransmitBufferSize = (unsigned int)(TempParam->Params[1].intParam);
         SPPConfigurationParams.ReceiveBufferSize  = (unsigned int)(TempParam->Params[2].intParam);

         ret_val = SPP_Set_Configuration_Parameters(BluetoothStackID, &SPPConfigurationParams);

         if(ret_val >= 0)
         {
            Display(("SPP_Set_Configuration_Parameters(): Success\r\n", ret_val));
            Display(("   MaximumFrameSize   : %d (0x%X)\r\n", SPPConfigurationParams.MaximumFrameSize, SPPConfigurationParams.MaximumFrameSize));
            Display(("   TransmitBufferSize : %d (0x%X)\r\n", SPPConfigurationParams.TransmitBufferSize, SPPConfigurationParams.TransmitBufferSize));
            Display(("   ReceiveBufferSize  : %d (0x%X)\r\n", SPPConfigurationParams.ReceiveBufferSize, SPPConfigurationParams.ReceiveBufferSize));

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error setting the current parameters.                    */
            Display(("SPP_Set_Configuration_Parameters(): Error %d.\r\n", ret_val));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         DisplayUsage("SetConfigParams [MaximumFrameSize] [TransmitBufferSize (0: don't change)] [ReceiveBufferSize (0: don't change)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the current    */
   /* queuing parameters that are used by SPP/RFCOMM (into L2CAP).  This*/
   /* function will return zero on successful execution and a negative  */
   /* value on errors.                                                  */
static int GetQueueParams(ParameterList_t *TempParam)
{
   int          ret_val;
   unsigned int MaximumNumberDataPackets;
   unsigned int QueuedDataPacketsThreshold;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Simply query the queuing parameters.                           */
      ret_val = SPP_Get_Queuing_Parameters(BluetoothStackID, &MaximumNumberDataPackets, &QueuedDataPacketsThreshold);

      if(ret_val >= 0)
      {
         /* Parameters have been queried successfully, go ahead and     */
         /* notify the user.                                            */
         Display(("SPP_Get_Queuing_Parameters(): Success.\r\n"));
         Display(("   MaximumNumberDataPackets   : %d (0x%X)\r\n", MaximumNumberDataPackets, MaximumNumberDataPackets));
         Display(("   QueuedDataPacketsThreshold : %d (0x%X)\r\n", QueuedDataPacketsThreshold, QueuedDataPacketsThreshold));

         /* Flag success.                                               */
         ret_val = 0;
      }
      else
      {
         /* Error querying the current parameters.                      */
         Display(("SPP_Get_Queuing_Parameters(): Error %d.\r\n", ret_val));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the current     */
   /* queuing parameters that are used by SPP/RFCOMM (into L2CAP).  This*/
   /* function will return zero on successful execution and a negative  */
   /* value on errors.                                                  */
static int SetQueueParams(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((TempParam) && (TempParam->NumberofParameters > 1))
      {
         /* Parameters have been specified, go ahead and write them to  */
         /* the stack.                                                  */
         ret_val = SPP_Set_Queuing_Parameters(BluetoothStackID, (unsigned int)(TempParam->Params[0].intParam), (unsigned int)(TempParam->Params[1].intParam));

         if(ret_val >= 0)
         {
            Display(("SPP_Set_Queuing_Parameters(): Success.\r\n"));
            Display(("   MaximumNumberDataPackets   : %d (0x%X)\r\n", (unsigned int)(TempParam->Params[0].intParam), (unsigned int)(TempParam->Params[0].intParam)));
            Display(("   QueuedDataPacketsThreshold : %d (0x%X)\r\n", (unsigned int)(TempParam->Params[1].intParam), (unsigned int)(TempParam->Params[1].intParam)));

            /* Flag success.                                            */
            ret_val = 0;
         }
         else
         {
            /* Error setting the current parameters.                    */
            Display(("SPP_Set_Queuing_Parameters(): Error %d.\r\n", ret_val));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         DisplayUsage("SetQueueParams [MaximumNumberDataPackets] [QueuedDataPacketsThreshold]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the application */
   /* state to support loopback mode.  This function will return zero on*/
   /* successful execution and a negative value on errors.              */
static int Loopback(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
       if((TempParam) && (TempParam->NumberofParameters >= 1) && 
          ((TempParam->Params[0].intParam == 0) ||(TempParam->Params[0].intParam == 1)))
        {
         if(TempParam->Params[0].intParam)
            LoopbackActive = TRUE;
         else
            LoopbackActive = FALSE;
        }
      else
        {
        LoopbackActive = (LoopbackActive?FALSE:TRUE);
        Display(("Usage Loopback [Active].\r\n"));

        }

      /* Finally output the current Loopback state.                     */
      Display(("Current Loopback Mode set to: %s.\r\n", LoopbackActive?"ACTIVE":"INACTIVE"));

      /* Flag success.                                                  */
      ret_val = 0;
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the application */
   /* state to support displaying Raw Data.  This function will return  */
   /* zero on successful execution and a negative value on errors.      */
static int DisplayRawModeData(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Check to see if Loopback is active.  If it is then we will not */
      /* process this command (and we will inform the user).            */
      if(!LoopbackActive)
      {
         /* Next check to see if the parameters required for the        */
         /* execution of this function appear to be semi-valid.         */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            if(TempParam->Params->intParam)
               DisplayRawData = TRUE;
            else
               DisplayRawData = FALSE;
         }
         else
            DisplayRawData = (DisplayRawData?FALSE:TRUE);

         /* Output the current Raw Data Display Mode state.             */
         Display(("Current Raw Data Display Mode set to: %s.\r\n", DisplayRawData?"ACTIVE":"INACTIVE"));

         /* Flag that the function was successful.                      */
         ret_val = 0;
      }
      else
      {
         Display(("Unable to process Raw Mode Display Request when operating in Loopback Mode.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the application */
   /* state to support Automatically reading all data that is received  */
   /* through SPP.  This function will return zero on successful        */
   /* execution and a negative value on errors.                         */
static int AutomaticReadMode(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Check to see if Loopback is active.  If it is then we will not */
      /* process this command (and we will inform the user).            */
      if(!LoopbackActive)
      {
         /* Next check to see if the parameters required for the        */
         /* execution of this function appear to be semi-valid.         */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            if(TempParam->Params->intParam)
               AutomaticReadActive = TRUE;
            else
               AutomaticReadActive = FALSE;
         }
         else
            AutomaticReadActive = (AutomaticReadActive?FALSE:TRUE);

         /* Output the current Automatic Read Mode state.               */
         Display(("Current Automatic Read Mode set to: %s.\r\n", AutomaticReadActive?"ACTIVE":"INACTIVE"));

         /* Flag that the function was successful.                      */
         ret_val = 0;
      }
      else
      {
         Display(("Unable to process Automatic Read Mode Request when operating in Loopback Mode.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following thread is responsible for checking changing the     */
   /* current Baud Rate used to talk to the Radio.                      */
   /* * NOTE * This function ONLY configures the Baud Rate for a TI     */
   /*          Bluetooth chipset.                                       */
static int SetBaudRate(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam))
      {
         /* Next, write the command to the device.                      */
         ret_val = VS_Update_UART_Baud_Rate(BluetoothStackID, (DWord_t)TempParam->Params[0].intParam);
         if(!ret_val)
         {
            Display(("VS_Update_UART_Baud_Rate(%u): Success.\r\n", TempParam->Params[0].intParam));
         }
         else
         {
            /* Unable to write vendor specific command to chipset.      */
            Display(("VS_Update_UART_Baud_Rate(%u): Failure %d, %d.\r\n", TempParam->Params[0].intParam, ret_val));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         DisplayUsage("SetBaudRate [BaudRate]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for sending a number of     */
   /* characters to a remote device to which a connection exists.  The  */
   /* function receives a parameter that indicates the number of byte to*/
   /* be transferred.  This function will return zero on successful     */
   /* execution and a negative value on errors.                         */
static int SendData(ParameterList_t *TempParam)
{
   int       Ndx;
   Word_t   DataCount;
   Boolean_t Done;

   /* Make sure that all of the parameters required for this function   */
   /* appear to be at least semi-valid.                                 */
   if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam > 0))
   {
      /* Verify that there is a connection that is established.         */
      if(SerialPortID)
      {
         /* Chcek to see if we are sending to another port.             */
         if(!SendInfo.BytesToSend)
         {
            /* Get the count of the number of bytes to send.            */
            SendInfo.BytesToSend  = (DWord_t)TempParam->Params[0].intParam;
            SendInfo.BytesSent    = 0;

            Done = FALSE;
            while((SendInfo.BytesToSend) && (!Done))
            {
               /* Set the Number of bytes to send in the first packet.  */
               if(SendInfo.BytesToSend > DataStrLen)
                  DataCount = DataStrLen;
               else
                  DataCount = SendInfo.BytesToSend;

               Ndx = SPP_Data_Write(BluetoothStackID, SerialPortID, DataCount, (unsigned char *)DataStr);
               if(Ndx >= 0)
               {
                  /* Adjust the counters.                               */
                  SendInfo.BytesToSend -= Ndx;
                  if(Ndx < DataCount)
                  {
                     SendInfo.BytesSent = Ndx;
                     Done               = TRUE;
                  }
               }
               else
               {
                  Display(("SEND failed with error %d\r\n", Ndx));
                  SendInfo.BytesToSend  = 0;
               }
            }
         }
         else
            Display(("Send Currently in progress.\r\n"));
      }
      else
         Display(("No Connection Established\r\n"));
   }
   else
      DisplayUsage("SEND [Number of Bytes to send]\r\n");

   return(0);
}

   /* The following function is responsible for changing the User       */
   /* Interface Mode to Server.                                         */
static int ServerMode(ParameterList_t *TempParam)
{
   UI_Mode = UI_MODE_IS_SERVER;
   UserInterface_Server();

   return(EXIT_MODE);
}

   /* The following function is responsible for changing the User       */
   /* Interface Mode to Client.                                         */
static int ClientMode(ParameterList_t *TempParam)
{
   UI_Mode = UI_MODE_IS_CLIENT;
   UserInterface_Client();

   return(EXIT_MODE);
}

   /* The following function is responsible for changing the User       */
   /* Interface Mode to Production.                                         */
static int ProdToolMode(ParameterList_t *TempParam)
{
   UI_Mode = UI_MODE_IS_PRODTOOLS;
   UserInterface_ProdTools();

   return(EXIT_MODE);
}

/* Close Bluetooth connection of open */
int CloseBluetooth(void) {
  
  int ret_val = -1;
  
     /* indeed Open.                                                */
     if(SerialPortID)
     {
    
#ifdef CONSOLE_SUPPORT           
       Display(("Closing active SPP connection, ID: 0x%04X.\r\n", SerialPortID));
#endif // CONSOLE_SUPPORT

        /* Simply close the Serial Port Client.                     */
        ret_val = SPP_Close_Port(BluetoothStackID, SerialPortID);
        
        if (!ret_val) {
          
            // Forecfully disconnect the link
            GAP_Disconnect_Link(BluetoothStackID, SelectedBD_ADDR);

            SerialPortID = 0;
            SendInfo.BytesToSend = 0;
            Connected    = FALSE;
            SPPOpened = FALSE;    
            Connection_Handle = 0;
        } 
    }  
    
    return ret_val;
}


   /* The following function is responsible for creating an LE          */
   /* connection to the specified Remote Device.                        */
static int ConnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, GAP_LE_Address_Type_t RemoteAddressType ,GAP_LE_Address_Type_t OwnAddressType, Boolean_t UseWhiteList)
{
   int                            Result;
   int                            LEConnectionIndex;
   unsigned int                   WhiteListChanged;
   GAP_LE_White_List_Entry_t      WhiteListEntry;
   GAP_LE_Connection_Parameters_t ConnectionParameters;

   /* First, determine if the input parameters appear to be semi-valid. */
   if((BluetoothStackID) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
        /* Make sure that there are available connections               */
      if(!(ApplicationStateInfo.Flags & APPLICATION_STATE_INFO_FLAGS_LE_CONNECTED))
      {

         /* If everything has been successful, up until this point,  */
         /* then go ahead and attempt the connection.                */
         /* Initialize the connection parameters.                 */
         ConnectionParameters.Connection_Interval_Min    = 50;
         ConnectionParameters.Connection_Interval_Max    = 200;
         ConnectionParameters.Minimum_Connection_Length  = 0;
         ConnectionParameters.Maximum_Connection_Length  = 10000;
         ConnectionParameters.Slave_Latency              = 0;
         ConnectionParameters.Supervision_Timeout        = 20000;

         /* Everything appears correct, go ahead and attempt to   */
         /* make the connection.                                  */
         Result = GAP_LE_Create_Connection(BluetoothStackID, 100, 100, fpNoFilter, RemoteAddressType, &BD_ADDR, OwnAddressType, &ConnectionParameters, GAP_LE_Event_Callback, 0);

         if(!Result)
         {
            Display(("Connection Request successful.\r\n"));
         }
         else
         {
            /* Unable to create connection.                       */
            Display(("Unable to create connection: %d.\r\n", Result));
         }
      }
      else
      {
         /* Device already connected.                                */
         Display(("Device is already connected.\r\n"));

         Result = -2;
      }
   }
   else
      Result = -1;

   return(Result);
}

int DisconnectLE() {
    
   int Result;
   slogf(LOG_DEST_BOTH, "Send disconnect");
   /* First, determine if the input parameters appear to be semi-valid. */
   if((BluetoothStackID) && (!COMPARE_NULL_BD_ADDR(ApplicationStateInfo.LEConnectionInfo.BD_ADDR)))
   {
      /* Make sure that a device with address BD_ADDR is connected      */
      if(ApplicationStateInfo.Flags & APPLICATION_STATE_INFO_FLAGS_LE_CONNECTED)
      {
         Result = GAP_LE_Disconnect(BluetoothStackID, ApplicationStateInfo.LEConnectionInfo.BD_ADDR);
         
         if(!Result)
         {
            Display(("Disconnect Request successful.\r\n"));
         }
         else
         {
            /* Unable to disconnect device.                             */
            Display(("Unable to disconnect device: %d.\r\n", Result));
         }
      }
      else
      {
         /* Device not connected.                                       */
         Display(("Device is not connected.\r\n"));

         Result = -1;
      }
   }
   else
      Result = -1;

   return(Result);
      
}

#endif // CONSOLE_SUPPORT

   /* The following function attempts to send the specified data length         */
   /* out of the specified data buffer. If the GATT buffers fill up, it will    */
   /* send what it can and return the number of bytes that were sent            */
   /* successfully.                                                             */
   /*                                                                           */
static unsigned int PWVSendData(unsigned int BluetoothStackID, DeviceInfo_t *DeviceInfo, unsigned int DataLength, Byte_t *Data)
{
   int          Result;
   Boolean_t    Done;
   unsigned int DataCount;
   unsigned int MaxLength;
   unsigned int TransmitIndex;
   unsigned int PWVBufferLength;
   unsigned int TotalBytesTransmitted = 0;

   /* Verify that the input parameters are semi-valid.                  */
   if((BluetoothStackID) && (DeviceInfo))
   {
      /* Loop while we have data to send and we can send it.            */
      Done              = FALSE;
      TransmitIndex     = 0;
      PWVBufferLength   = 0;
      while(!Done)
      {
         /* Get the maximum length of what we can send in this       */
         /* transaction.                                             */
         MaxLength = PWV_DATA_BUFFER_LENGTH;

         /* If we do not have any outstanding data get some more     */
         /* data.                                                    */
         if(!PWVBufferLength)
         {
            /* Check to see if we have data to send.              */
            if((DataLength) && (Data))
            {
               /* Copy the data to send into the SPPLEBuffer.     */
               PWVBufferLength = (DataLength > MaxLength)?MaxLength:DataLength;

               DataLength -= PWVBufferLength;
            }
            else
            {
               /* No data queued or data left to send so exit the */
               /* loop.                                           */
               Done = TRUE;
            }

            /* Set the count of data that we can send.               */
            DataCount         = PWVBufferLength;

            /* Reset the Transmit Index to 0.                        */
            TransmitIndex     = 0;
         }
         else
         {
            /* We have data to send so cap it at the maximum that can*/
            /* be transmitted.                                       */
            DataCount = (PWVBufferLength > MaxLength)?MaxLength:PWVBufferLength;
         }

         /* Try to write data if not exiting the loop.               */
         if(!Done)
         {
//xxx Debug statement
//xxx               Display(("\r\nTrying to send %u bytes.\r\n", DataCount));


            /* We are acting as SPPLE Server, so notify the Tx    */
            /* Characteristic.                                    */
            if(DeviceInfo->ServerInfo.Control_Point_Client_Configuration_Descriptor == GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
               Result = GATT_Handle_Value_Notification(BluetoothStackID, PasswordVaultServiceID, ConnectionID, PWV_CONTROL_POINT_CHARACTERISTIC_ATTRIBUTE_OFFSET, (Word_t)DataCount, &Data[TransmitIndex]);
            else
            {
               /* Not configured for notifications so exit the    */
               /* loop.                                           */
               Done = TRUE;
            }

            /* Check to see if any data was written.                 */
            if(!Done)
            {
               /* Check to see if the data was written successfully. */
               if(Result >= 0)
               {
                  /* Adjust the counters.                            */
                  TransmitIndex                                  += (unsigned int)Result;
                  PWVBufferLength                                -= (unsigned int)Result;

//xxx Debug statement
//xxx                     Display(("\r\nSent %u, Remaining Credits %u.\r\n", (unsigned int)Result, DeviceInfo->TransmitCredits));

                  /* Flag that data was sent.                        */
                  TotalBytesTransmitted                          += Result;
               }
               else
               {
                  PWVBufferLength = 0;

                  /* Flag that we should exit the loop.              */
                  Done              = TRUE;       
                  
                  if(Result == BTPS_ERROR_INSUFFICIENT_BUFFER_SPACE)
                  {
                     SPP_event = SPP_EVT_GATT_BUFFER_FULL;
                  }
               }
            }
         }
      }
   }

   return(TotalBytesTransmitted);
}

   /* The following function is for an GATT Server Event Callback.  This*/
   /* function will be called whenever a GATT Request is made to the    */
   /* server who registers this function that cannot be handled         */
   /* internally by GATT.  This function passes to the caller the GATT  */
   /* Server Event Data that occurred and the GATT Server Event Callback*/
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the GATT Server Event   */
   /* Data ONLY in the context of this callback.  If the caller requires*/
   /* the Data for a longer period of time, then the callback function  */
   /* MUST copy the data into another Data Buffer.  This function is    */
   /* guaranteed NOT to be invoked more than once simultaneously for the*/
   /* specified installed callback (i.e.  this function DOES NOT have be*/
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another GATT Event (Server/Client or Connection) will not be      */
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_ServerEventCallback(unsigned int BluetoothStackID, GATT_Server_Event_Data_t *GATT_ServerEventData, unsigned long CallbackParameter)
{
   Byte_t        Temp[2];
   Word_t        Value;
   Word_t        PreviousValue;
   Word_t        AttributeOffset;
   Byte_t        ErrorCode;
   DeviceInfo_t *DeviceInfo;
   FRESULT res = FR_OK;
   UINT written;
   Byte_t error[] = {0x99};
   uint8_t *value_array;
     
   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_ServerEventData))
   {
      switch(GATT_ServerEventData->Event_Data_Type)
      {
         case etGATT_Server_Read_Request:
            /* Verify that the Event Data is valid.                     */
            if(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data)
            {
               /* Verify that the read isn't a read blob (no SPPLE      */
               /* readable characteristics are long).                   */
               if(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeValueOffset == 0)
               {
                  /* Grab the device info for the currently          */
                  /* connected device.                               */
                  if((DeviceInfo = SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ApplicationStateInfo.LEConnectionInfo.AddressType, ApplicationStateInfo.LEConnectionInfo.BD_ADDR)) != NULL)
                  {
                     /* Determine which request this read is coming  */
                     /* for.                                         */
                     switch(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset)
                     {
                        case PWV_CONTROL_POINT_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                           ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(Temp, 0xA526);
                           break;
                        case PWV_CONTROL_POINT_CHARACTERISTIC_CCD_ATTRIBUTE_OFFSET:
                           ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(Temp, DeviceInfo->ServerInfo.Control_Point_Client_Configuration_Descriptor);
                           break;
                     }
                     if(ApplicationStateInfo.LEConnectionInfo.Flags & CONNECTION_INFO_FLAGS_CONNECTION_ENCRYPTED)
                     {
                        GATT_Read_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, WORD_SIZE, Temp);
                     }
                     else
                     {
                        GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION);
                     }
                  }
                  else
                  {
                     Display(("Error - No device info entry for this device.\r\n"));
                  }
               }
               else
                  GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
            }
            else
               Display(("Invalid Read Request Event Data.\r\n"));
            break;
         case etGATT_Server_Write_Request:
            /* Verify that the Event Data is valid.                     */
            if(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data)
            {
               if(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueOffset == 0)
               {
                  /* Cache the Attribute Offset.                        */
                  AttributeOffset = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset;

                  /* Verify that the value is of the correct length.    */
                  if(((GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueLength)))
                  {
                     /* Grab the device info for the currently       */
                     /* connected device.                            */
                     if((DeviceInfo = SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ApplicationStateInfo.LEConnectionInfo.AddressType, ApplicationStateInfo.LEConnectionInfo.BD_ADDR)) != NULL)
                     {
                        if(ApplicationStateInfo.LEConnectionInfo.Flags & CONNECTION_INFO_FLAGS_CONNECTION_ENCRYPTED)
                        {
                           /* Since the value appears valid go ahead and*/
                           /* accept the write request.                 */
                           GATT_Write_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID);
                        }
                        else
                        {
                           GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset, ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION);
                           Display(("Write Request Without Permission.\r\n"));
                           return;
                        }

                        if(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueLength == WORD_SIZE)
                           Value = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue);
                        else
                           Value = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue);

                        value_array = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue;
                        
                        /* Determine which attribute this write      */
                        /* request is for.                           */
                        switch(AttributeOffset)
                        {
                           case PWV_CONTROL_POINT_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                              slogf(LOG_DEST_BOTH, "CMD: %d", value_array[0]);
                              switch(value_array[0]){
                                 case PWV_CMD_ENABLE_EDR:
                                    slogf(LOG_DEST_BOTH, "Enable EDR");
                                    SPP_event = SPP_EVT_ENABLE_EDR;
                                    break;
                                 case PWV_CMD_SEND_FILE:
                                    memcpy(le_transfer_filepath, value_array+2, value_array[1]);
                                    SPP_event = SPP_EVT_LE_SEND_FILE;
                                    break;
                                 case PWV_CMD_OPEN_FILE:
                                    slogf(LOG_DEST_BOTH, "Open file");
                                    eMMC_PowerOn();
                                    res = f_open(&fp_download, value_array+2, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
                                    RTC_GetElapsedTime(&LETransferStartTime);
                                    if(res != FR_OK)
                                    {
                                       error[0] = PWV_ERROR_FILE_OPEN_FAIL;
                                       PWVSendData(BluetoothStackID, DeviceInfo, 1, error);
                                    }
                                    break;
                                 case PWV_CMD_CLOSE_FILE:
                                    RTC_GetElapsedTime(&LETransferEndTime);
                                    slogf(LOG_DEST_BOTH, "Close file");
                                    slogf(LOG_DEST_BOTH, "File transfer time: %d", LETransferEndTime - LETransferStartTime);
                                    f_close(&fp_download);
                                    break;
                                 default:
                                    break;
                              }    
                              le_transfer_DeviceInfo = DeviceInfo;
                              break;
                           case PWV_CONTROL_POINT_CHARACTERISTIC_CCD_ATTRIBUTE_OFFSET:
                              /* Cache the previous CCD Value.       */
                              PreviousValue = DeviceInfo->ServerInfo.Control_Point_Client_Configuration_Descriptor;

                              /* Note the updated Control Point CCCD Value.     */
                              DeviceInfo->ServerInfo.Control_Point_Client_Configuration_Descriptor = Value;

                              /* If we were not previously configured*/
                              /* for notifications send the initial  */
                              /* credits to the device.              */
                              if(PreviousValue != GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
                              {
                                 
                              }
                              break;
                           case PWV_FILE_WRITE_CHARACTERISTIC_ATTRIBUTE_OFFSET:
                              if(fp_download.fs != 0)
                              {
                                 slogf(LOG_DEST_BOTH, "Write data");
                                 res = f_write(&fp_download, (void *) GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueLength, &written);
                                 if(res != FR_OK)
                                 {
                                    error[0] = PWV_ERROR_FILE_WRITE_FAIL;
                                    PWVSendData(BluetoothStackID, DeviceInfo, 1, error);
                                 }
                              }
                              else
                              {
                                 slogf(LOG_DEST_BOTH, "File not open");
                                 error[0] = PWV_ERROR_FILE_NOT_OPEN;
                                 PWVSendData(BluetoothStackID, DeviceInfo, 1, error);
                              }
                              break;
                        }
                     }
                     else
                     {
                        Display(("Error - No device info entry for this device.\r\n"));
                     }
                  }
                  else
                     GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset, ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH);
               }
               else
                  GATT_Error_Response(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset, ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
            }
            else
               Display(("Invalid Write Request Event Data.\r\n"));
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));

      Display(("GATT Callback Data: Event_Data = NULL.\r\n"));

      DisplayPrompt();
   }
}

   /*********************************************************************/
   /*                         Event Callbacks                           */
   /*********************************************************************/

   /* The following function is for the GAP Event Receive Data Callback.*/
   /* This function will be called whenever a Callback has been         */
   /* registered for the specified GAP Action that is associated with   */
   /* the Bluetooth Stack.  This function passes to the caller the GAP  */
   /* Event Data of the specified Event and the GAP Event Callback      */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the GAP Event Data ONLY */
   /* in the context of this callback.  If the caller requires the Data */
   /* for a longer period of time, then the callback function MUST copy */
   /* the data into another Data Buffer.  This function is guaranteed   */
   /* NOT to be invoked more than once simultaneously for the specified */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* other GAP Events will not be processed while this function call is*/
   /* outstanding).                                                     */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving other GAP Events.  A  */
   /*          Deadlock WILL occur because NO GAP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter)
{
   int                               Result;
   int                               Index;
   Boolean_t                         OOB_Data;
   Boolean_t                         MITM;
   GAP_IO_Capability_t               RemoteIOCapability;
   GAP_Inquiry_Event_Data_t         *GAP_Inquiry_Event_Data;
   GAP_Remote_Name_Event_Data_t     *GAP_Remote_Name_Event_Data;
   GAP_Authentication_Information_t  GAP_Authentication_Information;
   LinkKeyInfo_t                     BtKey;

   /* First, check to see if the required parameters appear to be       */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (GAP_Event_Data))
   {
      /* The parameters appear to be semi-valid, now check to see what  */
      /* type the incoming event is.                                    */
      switch(GAP_Event_Data->Event_Data_Type)
      {
         case etInquiry_Result:
            /* The GAP event received was of type Inquiry_Result.       */
            GAP_Inquiry_Event_Data = GAP_Event_Data->Event_Data.GAP_Inquiry_Event_Data;

            /* Next, Check to see if the inquiry event data received    */
            /* appears to be semi-valid.                                */
            if(GAP_Inquiry_Event_Data)
            {
               /* Now, check to see if the gap inquiry event data's     */
               /* inquiry data appears to be semi-valid.                */
               if(GAP_Inquiry_Event_Data->GAP_Inquiry_Data)
               {
#ifdef CONSOLE_SUPPORT                  
                 Display(("\r\n"));

                  /* Display a list of all the devices found from       */
                  /* performing the inquiry.                            */
                  for(Index=0;(Index<GAP_Inquiry_Event_Data->Number_Devices) && (Index<MAX_INQUIRY_RESULTS);Index++)
                  {
                     InquiryResultList[Index] = GAP_Inquiry_Event_Data->GAP_Inquiry_Data[Index].BD_ADDR;
                     BD_ADDRToStr(GAP_Inquiry_Event_Data->GAP_Inquiry_Data[Index].BD_ADDR, Callback_BoardStr);

                     Display(("Result: %d, %s.\r\n", (Index+1), Callback_BoardStr));
                  }
#endif // CONSOLE_SUPPORT
                  NumberofValidResponses = GAP_Inquiry_Event_Data->Number_Devices;
               }
            }
            break;
         case etInquiry_Entry_Result:
#ifdef CONSOLE_SUPPORT                  
            /* Next convert the BD_ADDR to a string.                    */
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Inquiry_Entry_Event_Data->BD_ADDR, Callback_BoardStr);

            /* Display this GAP Inquiry Entry Result.                   */
            Display(("\r\n"));
            Display(("Inquiry Entry: %s.\r\n", Callback_BoardStr));
            break;
#endif // CONSOLE_SUPPORT
         case etAuthentication:
            /* An authentication event occurred, determine which type of*/
            /* authentication event occurred.                           */
            switch(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->GAP_Authentication_Event_Type)
            {
               case atLinkKeyRequest:
#ifdef CONSOLE_SUPPORT                  
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atLinkKeyRequest: %s\r\n", Callback_BoardStr));
#endif // CONSOLE_SUPPORT

                  /* Setup the authentication information response      */
                  /* structure.                                         */
                  GAP_Authentication_Information.GAP_Authentication_Type    = atLinkKey;
                  GAP_Authentication_Information.Authentication_Data_Length = 0;

                  /* See if we have stored a Link Key for the specified */
                  /* device.                                            */
                  if(GetLinkedKey(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &BtKey))
                     {
                        /* Link Key information stored, go ahead and    */
                        /* respond with the stored Link Key.            */
                        GAP_Authentication_Information.Authentication_Data_Length   = sizeof(Link_Key_t);
                        GAP_Authentication_Information.Authentication_Data.Link_Key = BtKey.LinkKey;
                  }

                  /* Submit the authentication response.                */
                  Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  /* Check the result of the submitted command.         */
                  if(!Result) {
#ifdef CONSOLE_SUPPORT                  
                     DisplayFunctionSuccess("GAP_Authentication_Response");
#endif // CONSOLE_SUPPORT
                  }
                  else {
#ifdef CONSOLE_SUPPORT                  
                     DisplayFunctionError("GAP_Authentication_Response", Result);
                     
#endif // CONSOLE_SUPPORT
                  }
                  break;
               case atPINCodeRequest:
#ifdef CONSOLE_SUPPORT                  
                  /* A pin code request event occurred, first display   */
                  /* the BD_ADD of the remote device requesting the pin.*/
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  slogf(LOG_DEST_CONSOLE, "");
                  slogf(LOG_DEST_BOTH, "atPINCodeRequest: %s", Callback_BoardStr);
#endif // CONSOLE_SUPPORT

                  /* Note the current Remote BD_ADDR that is requesting */
                  /* the PIN Code.                                      */
                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;
                  {
                    ParameterList_t pincode;
                    char pinstr[] = {'1','2','3','4',0};
                    pincode.NumberofParameters = 1;
                    pincode.Params[0].strParam = pinstr;
                    PINCodeResponse(&pincode);
                      
                  }
                  break;
               case atAuthenticationStatus:
                  /* An authentication status event occurred, display   */
                  /* all relevant information.                          */
#ifdef CONSOLE_SUPPORT                  
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atAuthenticationStatus: %d for %s\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Authentication_Status, Callback_BoardStr));
#endif // CONSOLE_SUPPORT

                  /* Flag that there is no longer a current             */
                  /* Authentication procedure in progress.              */
                  ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
                  break;
               case atLinkKeyCreation:
                  /* A link key creation event occurred, first display  */
                  /* the remote device that caused this event.          */
#ifdef CONSOLE_SUPPORT                  
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  slogf(LOG_DEST_CONSOLE, "");
                  slogf(LOG_DEST_BOTH, "atLinkKeyCreation: %s", Callback_BoardStr);
#endif // CONSOLE_SUPPORT

                  /* Now store the link Key */
                  BtKey.BD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device; 
                  BtKey.LinkKey = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Link_Key_Info.Link_Key;

                  if(AddLinkedKey(&BtKey)==FR_OK)
                  {
                    Display(("Link Key Stored.(%d)\r\n",BT_LinkedDeviceNb));
                  }else
                  {
                    Display(("Error storing Link!\r\n"));
                  }
                  
                  SPP_event = SPP_EVT_EDR_PAIR_COMPLETE;
                  break;
               case atIOCapabilityRequest:
#ifdef CONSOLE_SUPPORT                  
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atIOCapabilityRequest: %s\r\n", Callback_BoardStr));
#endif // CONSOLE_SUPPORT

                  /* Setup the Authentication Information Response      */
                  /* structure.                                         */
                  GAP_Authentication_Information.GAP_Authentication_Type                                      = atIOCapabilities;
                  GAP_Authentication_Information.Authentication_Data_Length                                   = sizeof(GAP_IO_Capabilities_t);
                  GAP_Authentication_Information.Authentication_Data.IO_Capabilities.IO_Capability            = (GAP_IO_Capability_t)IOCapability;
                  GAP_Authentication_Information.Authentication_Data.IO_Capabilities.MITM_Protection_Required = MITMProtection;
                  GAP_Authentication_Information.Authentication_Data.IO_Capabilities.OOB_Data_Present         = OOBSupport;

                  /* Submit the Authentication Response.                */
                  Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  /* Check the result of the submitted command.         */
                  /* Check the result of the submitted command.         */
#ifdef CONSOLE_SUPPORT                  
                  if(!Result)
                     DisplayFunctionSuccess("Auth");
                  else
                     DisplayFunctionError("Auth", Result);
#endif // CONSOLE_SUPPORT
                  break;
               case atIOCapabilityResponse:
#ifdef CONSOLE_SUPPORT                  
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atIOCapabilityResponse: %s\r\n", Callback_BoardStr));
#endif // CONSOLE_SUPPORT

                  RemoteIOCapability = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.IO_Capability;
                  MITM               = (Boolean_t)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.MITM_Protection_Required;
                  OOB_Data           = (Boolean_t)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.OOB_Data_Present;

                  Display(("Capabilities: %s%s%s\r\n", IOCapabilitiesStrings[RemoteIOCapability], ((MITM)?", MITM":""), ((OOB_Data)?", OOB Data":"")));
                  break;
               case atUserConfirmationRequest:
#ifdef CONSOLE_SUPPORT                  
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atUserConfirmationRequest: %s\r\n", Callback_BoardStr));
#endif // CONSOLE_SUPPORT

                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;

                  if(IOCapability != icDisplayYesNo)
                  {
                     /* Invoke JUST Works Process...                    */
                     GAP_Authentication_Information.GAP_Authentication_Type          = atUserConfirmation;
                     GAP_Authentication_Information.Authentication_Data_Length       = (Byte_t)sizeof(Byte_t);
                     GAP_Authentication_Information.Authentication_Data.Confirmation = TRUE;

                     /* Submit the Authentication Response.             */
                     Display(("\r\nAuto Accepting: %u\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value));

                     Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

#ifdef CONSOLE_SUPPORT                  
                     if(!Result)
                        DisplayFunctionSuccess("GAP_Authentication_Response");
                     else
                        DisplayFunctionError("GAP_Authentication_Response", Result);
#endif // CONSOLE_SUPPORT

                     /* Flag that there is no longer a current          */
                     /* Authentication procedure in progress.           */
                     ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
                  }
                  else
                  {
#ifdef CONSOLE_SUPPORT                  
                     Display(("User Confirmation: %u\r\n", (unsigned long)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value));

                     /* Inform the user that they will need to respond  */
                     /* with a PIN Code Response.                       */
                     Display(("Respond with: UserConfirmationResponse\r\n"));
#endif // CONSOLE_SUPPORT
                  }
                  break;
               case atPasskeyRequest:
#ifdef CONSOLE_SUPPORT                  
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atPasskeyRequest: %s\r\n", Callback_BoardStr));
#endif // CONSOLE_SUPPORT

                  /* Note the current Remote BD_ADDR that is requesting */
                  /* the Passkey.                                       */
                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;

                  /* Inform the user that they will need to respond with*/
                  /* a Passkey Response.                                */
                  Display(("Respond with: PassKeyResponse\r\n"));
                  break;
               case atRemoteOutOfBandDataRequest:
#ifdef CONSOLE_SUPPORT                  
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atRemoteOutOfBandDataRequest: %s\r\n", Callback_BoardStr));
#endif // CONSOLE_SUPPORT

                  /* This application does not support OOB data so      */
                  /* respond with a data length of Zero to force a      */
                  /* negative reply.                                    */
                  GAP_Authentication_Information.GAP_Authentication_Type    = atOutOfBandData;
                  GAP_Authentication_Information.Authentication_Data_Length = 0;

                  Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

#ifdef CONSOLE_SUPPORT                  
                  if(!Result)
                     DisplayFunctionSuccess("GAP_Authentication_Response");
                  else
                     DisplayFunctionError("GAP_Authentication_Response", Result);
#endif // CONSOLE_SUPPORT
                  break;
               case atPasskeyNotification:
#ifdef CONSOLE_SUPPORT                  
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atPasskeyNotification: %s\r\n", Callback_BoardStr));

                  Display(("Passkey Value: %d\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value));
#endif // CONSOLE_SUPPORT
                  break;
               case atKeypressNotification:
#ifdef CONSOLE_SUPPORT                  
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atKeypressNotification: %s\r\n", Callback_BoardStr));

                  Display(("Keypress: %d\r\n", (int)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Keypress_Type));
#endif // CONSOLE_SUPPORT
                  break;
               default:
#ifdef CONSOLE_SUPPORT                  
                  Display(("Un-handled Auth. Event.\r\n"));
#endif // CONSOLE_SUPPORT
                  break;
            }
            break;
         case etRemote_Name_Result:
            /* Bluetooth Stack has responded to a previously issued     */
            /* Remote Name Request that was issued.                     */
            GAP_Remote_Name_Event_Data = GAP_Event_Data->Event_Data.GAP_Remote_Name_Event_Data;
            if(GAP_Remote_Name_Event_Data)
            {
#ifdef CONSOLE_SUPPORT                  
               /* Inform the user of the Result.                        */
               BD_ADDRToStr(GAP_Remote_Name_Event_Data->Remote_Device, Callback_BoardStr);

               Display(("\r\n"));
               Display(("BD_ADDR: %s.\r\n", Callback_BoardStr));

               if(GAP_Remote_Name_Event_Data->Remote_Name)
                  Display(("Name: %s.\r\n", GAP_Remote_Name_Event_Data->Remote_Name));
               else
                  Display(("Name: NULL.\r\n"));
#endif // CONSOLE_SUPPORT
            }
            break;
         case etEncryption_Change_Result:
#ifdef CONSOLE_SUPPORT                  
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Remote_Device, Callback_BoardStr);
            Display(("\r\netEncryption_Change_Result for %s, Status: 0x%02X, Mode: %s.\r\n", Callback_BoardStr,
                                                                                             GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Encryption_Change_Status,
                                                                                             ((GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Encryption_Mode == emDisabled)?"Disabled": "Enabled")));
#endif // CONSOLE_SUPPORT
            break;
         default:
            /* An unknown/unexpected GAP event was received.            */
#ifdef CONSOLE_SUPPORT                  
            Display(("\r\nUnknown Event: %d.\r\n", GAP_Event_Data->Event_Data_Type));
#endif // CONSOLE_SUPPORT
            break;
      }
   }
   else
   {
#ifdef CONSOLE_SUPPORT                  
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));
      Display(("Null Event\r\n"));
#endif // CONSOLE_SUPPORT
   }
#ifdef CONSOLE_SUPPORT                  
   DisplayPrompt();
#endif // CONSOLE_SUPPORT
}

   /* The following function is for an SPP Event Callback.  This        */
   /* function will be called whenever a SPP Event occurs that is       */
   /* associated with the Bluetooth Stack.  This function passes to the */
   /* caller the SPP Event Data that occurred and the SPP Event Callback*/
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the SPP Event Data ONLY */
   /* in the context of this callback.  If the caller requires the Data */
   /* for a longer period of time, then the callback function MUST copy */
   /* the data into another Data Buffer.  This function is guaranteed   */
   /* NOT to be invoked more than once simultaneously for the specified */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another SPP Event will not be processed while this function call  */
   /* is outstanding).                                                  */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving SPP Event Packets.  A     */
   /*          Deadlock WILL occur because NO SPP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI SPP_Event_Callback(unsigned int BluetoothStackID, SPP_Event_Data_t *SPP_Event_Data, unsigned long CallbackParameter)
{
   int       ret_val = 0;
   int       Index;
   int       Index1;
   int       TempLength;
#ifdef CONSOLE_SUPPORT           
   Boolean_t _DisplayPrompt = TRUE;
#endif // CONSOLE_SUPPORT
   Boolean_t Done;
   uint32_t TimeFromLastConnect;

   /* **** SEE SPPAPI.H for a list of all possible event types.  This   */
   /* program only services its required events.                   **** */

   /* First, check to see if the required parameters appear to be       */
   /* semi-valid.                                                       */
   if((SPP_Event_Data) && (BluetoothStackID))
   {
      /* The parameters appear to be semi-valid, now check to see what  */
      /* type the incoming event is.                                    */
      switch(SPP_Event_Data->Event_Data_Type)
      {
         case etPort_Open_Request_Indication:
            /* A remote port is requesting a connection.                */
#ifdef CONSOLE_SUPPORT           
            BD_ADDRToStr(SPP_Event_Data->Event_Data.SPP_Open_Port_Request_Indication_Data->BD_ADDR, Callback_BoardStr);

            Display(("\r\n"));
            Display(("SPP Open Request Indication, ID: 0x%04X, Board: %s.\r\n", SPP_Event_Data->Event_Data.SPP_Open_Port_Request_Indication_Data->SerialPortID, Callback_BoardStr));
#endif // CONSOLE_SUPPORT
            /* Accept the connection always.                            */
            SPP_Open_Port_Request_Response(BluetoothStackID, SPP_Event_Data->Event_Data.SPP_Open_Port_Request_Indication_Data->SerialPortID, TRUE);
            break;
         case etPort_Open_Indication:
            /* A remote port is requesting a connection.                */
#ifdef CONSOLE_SUPPORT           
            BD_ADDRToStr(SPP_Event_Data->Event_Data.SPP_Open_Port_Indication_Data->BD_ADDR, Callback_BoardStr);

            slogf(LOG_DEST_CONSOLE, "");
            slogf(LOG_DEST_BOTH, "SPP Open Indication, ID: 0x%04X, Board: %s.", SPP_Event_Data->Event_Data.SPP_Open_Port_Indication_Data->SerialPortID, Callback_BoardStr);
#endif // CONSOLE_SUPPORT
            if(pWriteABuffer == BT_WriteABuffer)
            {
              FtpServerReset();
            }
            /* Save the Serial Port ID for later use.                   */
            SerialPortID = SPP_Event_Data->Event_Data.SPP_Open_Port_Indication_Data->SerialPortID;

            /* Flag that we are now connected.                          */
            Connected  = TRUE;
            SPPOpened = TRUE;
            RTC_GetElapsedTime(&TimeFromLastConnect);
            if ((TimeFromLastConnect > Settings.FTP_AuthenticationTimeout) && (FindValidTemplate() == FR_OK)) {
              Display(("Authentication elapsed: %d > %d\r\n", TimeFromLastConnect, Settings.FTP_AuthenticationTimeout));
              FTPLocked = TRUE;
              AdvertiseLockStatus(FTPLocked);
            }
            BTActivity++;

            /* Query the connection handle.                             */
            ret_val = GAP_Query_Connection_Handle(BluetoothStackID, SPP_Event_Data->Event_Data.SPP_Open_Port_Indication_Data->BD_ADDR, &Connection_Handle);
            if(ret_val)
            {
               /* Failed to Query the Connection Handle.                */
#ifdef CONSOLE_SUPPORT           
               DisplayFunctionError("GAP_Query_Connection_Handle()",ret_val);
#endif // CONSOLE_SUPPORT
               ret_val           = 0;
               Connection_Handle = 0;
            }
            else
               Display(("HCI Connection Handle: 0x%04X.\r\n", Connection_Handle));

            break;
         case etPort_Open_Confirmation:
            /* A Client Port was opened.  The Status indicates the      */
            /* Status of the Open.                                      */
            Display(("\r\n"));
            Display(("SPP Open Confirmation, ID: 0x%04X, Status 0x%04X.\r\n", SPP_Event_Data->Event_Data.SPP_Open_Port_Confirmation_Data->SerialPortID,
                                                                              SPP_Event_Data->Event_Data.SPP_Open_Port_Confirmation_Data->PortOpenStatus));

            /* Check the Status to make sure that an error did not      */
            /* occur.                                                   */
            if(SPP_Event_Data->Event_Data.SPP_Open_Port_Confirmation_Data->PortOpenStatus)
            {
               /* An error occurred while opening the Serial Port so    */
               /* invalidate the Serial Port ID.                        */
               SerialPortID      = 0;
               Connection_Handle = 0;

               /* Flag that we are no longer connected.                 */
               Connected         = FALSE;
               SPPOpened = FALSE;
            }
            else
            {
               /* Flag that we are now connected.                       */
               Connected  = TRUE;
               SPPOpened = TRUE;
               BTActivity++;

               /* Query the connection Handle.                          */
               ret_val = GAP_Query_Connection_Handle(BluetoothStackID, SelectedBD_ADDR, &Connection_Handle);
               if(ret_val)
               {
                  /* Failed to Query the Connection Handle.             */
#ifdef CONSOLE_SUPPORT           
                  DisplayFunctionError("GAP_Query_Connection_Handle()", ret_val);
#endif // CONSOLE_SUPPORT

                  ret_val           = 0;
                  Connection_Handle = 0;
               }
            }
            break;
            
         case etPort_Close_Port_Indication:
            /* The Remote Port was Disconnected.                        */
#ifdef CONSOLE_SUPPORT           
            slogf(LOG_DEST_CONSOLE, "");
            slogf(LOG_DEST_BOTH, "SPP Close Port, ID: 0x%04X", SPP_Event_Data->Event_Data.SPP_Close_Port_Indication_Data->SerialPortID);
#endif // CONSOLE_SUPPORT
            
            // Forecfully disconnect the link
            GAP_Disconnect_Link(BluetoothStackID, SelectedBD_ADDR);


            SerialPortID = 0;
            Connection_Handle = 0;
            SendInfo.BytesToSend = 0;

            /* Flag that we are no longer connected.                    */
            Connected = FALSE;
            //eMMC_TurnOff = TRUE;
            SPPOpened = FALSE;
            RTC_InitTime();
            SPP_event = SPP_EVT_SPP_DISCONNECT;
            /*
            if (!Settings.FTP_AuthenticationTimeout) {
              // Lock FTP server if timeout is set to 0
              FTPLocked = TRUE;
              AdvertiseLockStatus(FTPLocked);
            }
            */
            break;
         case etPort_Status_Indication:
            /* Display Information about the new Port Status.           */
            Display(("\r\n"));
            Display(("SPP Port Status Indication: 0x%04X, Status: 0x%04X, Break Status: 0x%04X, Length: 0x%04X.\r\n", SPP_Event_Data->Event_Data.SPP_Port_Status_Indication_Data->SerialPortID,
                                                                                                                      SPP_Event_Data->Event_Data.SPP_Port_Status_Indication_Data->PortStatus,
                                                                                                                      SPP_Event_Data->Event_Data.SPP_Port_Status_Indication_Data->BreakStatus,
                                                                                                                      SPP_Event_Data->Event_Data.SPP_Port_Status_Indication_Data->BreakTimeout));

            break;
         case etPort_Data_Indication:
            /* Data was received.  Process it differently based upon the*/
            /* current state of the Loopback Mode.                      */
            if(LoopbackActive)
            {
               /* Initialize Done to false.                             */
               Done = FALSE;

               /* Loop until the write buffer is full or there is not   */
               /* more data to read.                                    */
               while((Done == FALSE) && (BufferFull == FALSE))
               {
                  /* The application state is currently in the loop back*/
                  /* state.  Read as much data as we can read.          */
                  if((TempLength = SPP_Data_Read(BluetoothStackID, SerialPortID, (Word_t)sizeof(Buffer), (Byte_t *)Buffer)) > 0)
                  {
                     /* Adjust the Current Buffer Length by the number  */
                     /* of bytes which were successfully read.          */
                     BufferLength = TempLength;

                     /* Next attempt to write all of the data which is  */
                     /* currently in the buffer.                        */
                     if((TempLength = SPP_Data_Write(BluetoothStackID, SerialPortID, (Word_t)BufferLength, (Byte_t *)Buffer)) < (int)BufferLength)
                     {
                        /* Not all of the data was successfully written */
                        /* or an error occurred, first check to see if  */
                        /* an error occurred.                           */
                        if(TempLength >= 0)
                        {
                           /* An error did not occur therefore the      */
                           /* Transmit Buffer must be full.  Adjust the */
                           /* Buffer and Buffer Length by the amount    */
                           /* which as successfully written.            */
                           if(TempLength)
                           {
                              for(Index=0,Index1=TempLength;Index1<BufferLength;Index++,Index1++)
                                 Buffer[Index] = Buffer[Index1];

                              BufferLength -= TempLength;
                           }

                           /* Set the flag indicating that the SPP Write*/
                           /* Buffer is full.                           */
                           BufferFull = TRUE;
                        }
                        else
                           Done = TRUE;
                     }
                  }
                  else
                     Done = TRUE;
               }
#ifdef CONSOLE_SUPPORT           
               _DisplayPrompt = FALSE;
#endif // CONSOLE_SUPPORT           
            }
            else
            {
               /* If we are operating in Raw Data Display Mode then     */
               /* simply display the data that was give to use.         */
               if((DisplayRawData) || (AutomaticReadActive))
               {
                  /* Initialize Done to false.                          */
                  Done = FALSE;

                  /* Loop through and read all data that is present in  */
                  /* the buffer.                                        */
                  while(!Done)
                  {
                     /* Read as much data as possible.                  */
                     if((TempLength = SPP_Data_Read(BluetoothStackID, SerialPortID, (Word_t)sizeof(Buffer)-1, (Byte_t *)Buffer)) > 0)
                     {
                        /* Now simply display each character that we    */
                        /* have just read.                              */
                        if(DisplayRawData)
                        {
                           Buffer[TempLength] = '\0';

                           Display(((char *)Buffer));
                        }
                     }
                     else
                     {
                        /* Either an error occurred or there is no more */
                        /* data to be read.                             */
                        if(TempLength < 0)
                        {
                           /* Error occurred.                           */
                           Display(("SPP_Data_Read(): Error %d.\r\n", TempLength));
                        }

                        /* Regardless if an error occurred, we are      */
                        /* finished with the current loop.              */
                        Done = TRUE;
                     }
                  }
#ifdef CONSOLE_SUPPORT           
                  _DisplayPrompt = FALSE;
#endif // CONSOLE_SUPPORT           
               }
               else
               {
                 if (FTPModeActive) {
                   /* Read as much data as possible.                  */
                   if((TempLength = SPP_Data_Read(BluetoothStackID, SerialPortID, (Word_t)sizeof(Buffer)-1, (Byte_t *)Buffer)) > 0)
                   {
                     if (pWriteABuffer == BT_WriteABuffer) { 
                      HandleASuccessfulRead((char *)Buffer,TempLength);
                     }
                   }
#ifdef CONSOLE_SUPPORT           
                  _DisplayPrompt = FALSE;
#endif // CONSOLE_SUPPORT           
                 }
                 else
                 {
                    /* Simply inform the user that data has arrived.      */
                    Display(("\r\n"));
                    Display(("SPP Data Indication, ID: 0x%04X, Length: 0x%04X.\r\n", SPP_Event_Data->Event_Data.SPP_Data_Indication_Data->SerialPortID,
                                                                                   SPP_Event_Data->Event_Data.SPP_Data_Indication_Data->DataLength));
                 }
               }
            }
            break;
         case etPort_Send_Port_Information_Indication:
            /* Simply Respond with the information that was sent to us. */
            ret_val = SPP_Respond_Port_Information(BluetoothStackID, SPP_Event_Data->Event_Data.SPP_Send_Port_Information_Indication_Data->SerialPortID, &SPP_Event_Data->Event_Data.SPP_Send_Port_Information_Indication_Data->SPPPortInformation);
            break;
         case etPort_Transmit_Buffer_Empty_Indication:
            /* The transmit buffer is now empty after being full.  Next */
            /* check the current application state.                     */
            if(SendInfo.BytesToSend)
            {
               /* Send the remainder of the last attempt.               */
               TempLength            = (DataStrLen-SendInfo.BytesSent);
               SendInfo.BytesSent    = SPP_Data_Write(BluetoothStackID, SerialPortID, TempLength, (unsigned char *)&(DataStr[SendInfo.BytesSent]));
               if((int)(SendInfo.BytesSent) >= 0)
               {
                  if(SendInfo.BytesSent <= SendInfo.BytesToSend)
                     SendInfo.BytesToSend -= SendInfo.BytesSent;
                  else
                     SendInfo.BytesToSend = 0;

                  while(SendInfo.BytesToSend)
                  {
                     /* Set the Number of bytes to send in the next     */
                     /* packet.                                         */
                     if(SendInfo.BytesToSend > DataStrLen)
                        TempLength = DataStrLen;
                     else
                        TempLength = SendInfo.BytesToSend;

                     SendInfo.BytesSent = SPP_Data_Write(BluetoothStackID, SerialPortID, TempLength, (unsigned char *)DataStr);
                     if((int)(SendInfo.BytesSent) >= 0)
                     {
                        SendInfo.BytesToSend -= SendInfo.BytesSent;
                        if(SendInfo.BytesSent < TempLength)
                           break;
                     }
                     else
                     {
                        Display(("SPP_Data_Write returned %d.\r\n", (int)SendInfo.BytesSent));

                        SendInfo.BytesToSend = 0;
                     }
                  }
               }
               else
               {
                  Display(("SPP_Data_Write returned %d.\r\n", (int)SendInfo.BytesSent));

                  SendInfo.BytesToSend = 0;
               }
            }
            else
            {
               if(LoopbackActive)
            {
               /* Initialize Done to false.                             */
               Done = FALSE;

               /* Loop until the write buffer is full or there is not   */
               /* more data to read.                                    */
               while(Done == FALSE)
               {
                     /* The application state is currently in the loop  */
                     /* back state.  Read as much data as we can read.  */
                  if(((TempLength = SPP_Data_Read(BluetoothStackID, SerialPortID, (Word_t)(sizeof(Buffer)-BufferLength), (Byte_t *)&(Buffer[BufferLength]))) > 0) || (BufferLength > 0))
                  {
                        /* Adjust the Current Buffer Length by the      */
                        /* number of bytes which were successfully read.*/
                     if(TempLength > 0)
                        {
                        BufferLength += TempLength;
                        }

                        /* Next attempt to write all of the data which  */
                        /* is currently in the buffer.                  */
                     if((TempLength = SPP_Data_Write(BluetoothStackID, SerialPortID, (Word_t)BufferLength, (Byte_t *)Buffer)) < (int)BufferLength)
                     {
                           /* Not all of the data was successfully      */
                           /* written or an error occurred, first check */
                           /* to see if an error occurred.              */
                        if(TempLength >= 0)
                        {
                           /* An error did not occur therefore the      */
                              /* Transmit Buffer must be full.  Adjust  */
                              /* the Buffer and Buffer Length by the    */
                              /* amount which was successfully written. */
                           if(TempLength)
                           {
                              for(Index=0,Index1=TempLength;Index1<BufferLength;Index++,Index1++)
                                 Buffer[Index] = Buffer[Index1];

                              BufferLength -= TempLength;
                           }
                           else
                              Done = TRUE;

                              /* Set the flag indicating that the SPP   */
                              /* Write Buffer is full.                  */
                           BufferFull = TRUE;
                        }
                        else
                           Done = TRUE;
                     }
                     else
                     {
                        BufferLength = 0;

                        BufferFull   = FALSE;
                     }
                  }
                  else
                     Done = TRUE;
               }
               }
            }
#ifdef CONSOLE_SUPPORT           
            _DisplayPrompt = FALSE;
#endif // CONSOLE_SUPPORT           
            break;
         default:
            /* An unknown/unexpected SPP event was received.            */
            Display(("\r\n"));
            Display(("Unknown Event.\r\n"));
            break;
      }

      /* Check the return value of any function that might have been    */
      /* executed in the callback.                                      */
      if(ret_val)
      {
         /* An error occurred, so output an error message.              */
         Display(("\r\n"));
         Display(("Error %d.\r\n", ret_val));
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("Null Event\r\n"));
   }

#ifdef CONSOLE_SUPPORT
   if(_DisplayPrompt)
      DisplayPrompt();
#endif // CONSOLE_SUPPORT
   
}

   /* The following function is for the GAP LE Event Receive Data       */
   /* Callback.  This function will be called whenever a Callback has   */
   /* been registered for the specified GAP LE Action that is associated*/
   /* with the Bluetooth Stack.  This function passes to the caller the */
   /* GAP LE Event Data of the specified Event and the GAP LE Event     */
   /* Callback Parameter that was specified when this Callback was      */
   /* installed.  The caller is free to use the contents of the GAP LE  */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because other GAP Events will not be  */
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter)
{
   int                                           Result;
   BoardStr_t                                    BoardStr;
   unsigned int                                  Index;
   DeviceInfo_t                                 *DeviceInfo;
   Long_Term_Key_t                               GeneratedLTK;
   GAP_LE_Security_Information_t                 GAP_LE_Security_Information;
   GAP_LE_Advertising_Report_Data_t             *DeviceEntryPtr;
   GAP_LE_Authentication_Event_Data_t           *Authentication_Event_Data;
   GAP_LE_Authentication_Response_Information_t  GAP_LE_Authentication_Response_Information;

   /* Verify that all parameters to this callback are semi-Valid.       */
   if((BluetoothStackID) && (GAP_LE_Event_Data))
   {
      switch(GAP_LE_Event_Data->Event_Data_Type)
      {
         case etLE_Advertising_Report:
            Display(("etLE_Advertising_Report with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size));
            Display(("  %d Responses.\r\n",GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries));

            for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
            {
               DeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Advertising_Data[Index]);

               /* Display the packet type for the device                */
               switch(DeviceEntryPtr->Advertising_Report_Type)
               {
                  case rtConnectableUndirected:
                     Display(("  Advertising Type: %s.\r\n", "rtConnectableUndirected"));
                     break;
                  case rtConnectableDirected:
                     Display(("  Advertising Type: %s.\r\n", "rtConnectableDirected"));
                     break;
                  case rtScannableUndirected:
                     Display(("  Advertising Type: %s.\r\n", "rtScannableUndirected"));
                     break;
                  case rtNonConnectableUndirected:
                     Display(("  Advertising Type: %s.\r\n", "rtNonConnectableUndirected"));
                     break;
                  case rtScanResponse:
                     Display(("  Advertising Type: %s.\r\n", "rtScanResponse"));
                     break;
               }

               /* Display the Address Type.                             */
               if(DeviceEntryPtr->Address_Type == latPublic)
               {
                  Display(("  Address Type: %s.\r\n","atPublic"));
               }
               else
               {
                  Display(("  Address Type: %s.\r\n","atRandom"));
               }

               /* Display the Device Address.                           */
               Display(("  Address: 0x%02X%02X%02X%02X%02X%02X.\r\n", DeviceEntryPtr->BD_ADDR.BD_ADDR5, DeviceEntryPtr->BD_ADDR.BD_ADDR4, DeviceEntryPtr->BD_ADDR.BD_ADDR3, DeviceEntryPtr->BD_ADDR.BD_ADDR2, DeviceEntryPtr->BD_ADDR.BD_ADDR1, DeviceEntryPtr->BD_ADDR.BD_ADDR0));
               Display(("  RSSI: %d.\r\n", DeviceEntryPtr->RSSI));
               Display(("  Data Length: %d.\r\n", DeviceEntryPtr->Raw_Report_Length));

               DisplayAdvertisingData(&(DeviceEntryPtr->Advertising_Data));
            }
            break;
         case etLE_Connection_Complete:
            Display(("etLE_Connection_Complete with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size));

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, BoardStr);

               Display(("   Status:       0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status));
               Display(("   Role:         %s.\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master)?"Master":"Slave"));
               Display(("   Address Type: %s.\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == latPublic)?"Public":"Random"));
               Display(("   BD_ADDR:      %s.\r\n", BoardStr));

               if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status == HCI_ERROR_CODE_NO_ERROR)
               {
                  ConnectionBD_ADDR   = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address;
                  LocalDeviceIsMaster = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master;
                  
                  /* HIDS: Save the Connection Information.                   */
                  ApplicationStateInfo.Flags                        |= APPLICATION_STATE_INFO_FLAGS_LE_CONNECTED;
                  ApplicationStateInfo.LEConnectionInfo.Flags        = CONNECTION_INFO_FLAGS_CONNECTION_VALID;
                  ApplicationStateInfo.LEConnectionInfo.BD_ADDR      = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address;
                  ApplicationStateInfo.LEConnectionInfo.AddressType  = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type;

                  /* Make sure that no entry already exists.            */
                  if((DeviceInfo = SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address)) == NULL)
                  {
                     Display(("New device"));
                     /* No entry exists so create one.                  */
                     if(!CreateNewDeviceInfoEntry(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type, ConnectionBD_ADDR))
                        Display(("Failed to add device to Device Info List.\r\n"));
                  }
                  else
                  {
                    
                     /* HIDS: We have paired with this device previously.*/
                     /* Therefore we will start a timer and if the       */
                     /* Master does not re-establish encryption when the */
                     /* timer expires we will request that he does so.   */
                     Result = BSC_StartTimer(BluetoothStackID, (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Current_Connection_Parameters.Connection_Interval * (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Current_Connection_Parameters.Slave_Latency + 8)), BSC_TimerCallback, 0);
                     if(Result > 0)
                     {
                        slogf(LOG_DEST_BOTH, "Start security timer");
                        /* Save the Security Timer ID.                  */
                        ApplicationStateInfo.LEConnectionInfo.SecurityTimerID = (unsigned int)Result;
                     }
                     else
                        Display(("Error - BSC_StartTimer() returned %d.\r\n", Result));
                    
                     /* Remote device is re-connected                */
                     /* Reset encryption mode of currently connected */
                     /* device to true because it was previously     */
                     /* paired                                       */
                     GAPEncryptionMode = emEnabled;

                     /* If we are the Master of the connection we will  */
                     /* attempt to Re-Establish Security if a LTK for   */
                     /* this device exists (i.e.  we previously paired).*/
                     if(LocalDeviceIsMaster)
                     {
                        /* Re-Establish Security if there is a LTK that */
                        /* is stored for this device.                   */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                        {
                           /* Re-Establish Security with this LTK.      */
                           Display(("Attempting to Re-Establish Security.\r\n"));

                           /* Attempt to re-establish security to this  */
                           /* device.                                   */
                           GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;
                           GAP_LE_Security_Information.Security_Information.Master_Information.LTK                 = DeviceInfo->LTK;
                           GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                           GAP_LE_Security_Information.Security_Information.Master_Information.Rand                = DeviceInfo->Rand;
                           GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                           Result = GAP_LE_Reestablish_Security(BluetoothStackID, ConnectionBD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                           if(Result)
                           {
                              DisplayFunctionError("GAP_LE_Reestablish_Security", Result);
                           }
                        }
                     }
                  } 
               } 
               else  
               {

                    /* Clear the LE Connection Information.               */
                    BTPS_MemInitialize(&(ApplicationStateInfo.LEConnectionInfo), 0, sizeof(ApplicationStateInfo.LEConnectionInfo));

                    /* Clear the LE Connection Flag.                      */
                    ApplicationStateInfo.Flags &= ~APPLICATION_STATE_INFO_FLAGS_LE_CONNECTED;
               }
            }
            BTActivity++;
            break;
         case etLE_Disconnection_Complete:
            Display(("etLE_Disconnection_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size));

            SPP_event = SPP_EVT_LE_DISCONNECT;
            
            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data)
            {
               Display(("   Status: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Status));
               Display(("   Reason: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Reason));

               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address, BoardStr);
               Display(("   BD_ADDR: %s.\r\n", BoardStr));

               /* Check to see if the device info is present in the     */
               /* list.                                                 */
               if((DeviceInfo = SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address_Type, GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address)) != NULL)  
               {
                 
                  /* Check to see if the link is encrypted.  If it isn't*/
                  /* we will delete the device structure.               */
                  if(!(ApplicationStateInfo.LEConnectionInfo.Flags & CONNECTION_INFO_FLAGS_CONNECTION_ENCRYPTED))
                  {
                     /* Connection is not encrypted so delete the device*/
                     /* structure.                                      */
                     //DeviceInfo = DeleteLEDeviceInfoEntry(&DeviceInfoList, DeviceInfo->ConnectionAddressType, DeviceInfo->ConnectionBD_ADDR);
                     //if(DeviceInfo)
                     //   FreeDeviceInfoEntryMemory(DeviceInfo);
                  }
                  

                  /* Clear the LE Connection Information.                  */
                  BTPS_MemInitialize(&(ApplicationStateInfo.LEConnectionInfo), 0, sizeof(ApplicationStateInfo.LEConnectionInfo));

                  /* Clear the LE Connection Flag.                         */
                  ApplicationStateInfo.Flags &= ~APPLICATION_STATE_INFO_FLAGS_LE_CONNECTED;
                   
                  /* Flag that no service discovery operation is        */
                  /* outstanding for this device.                       */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;

                  /* If this device is not paired, then delete it.  The */
                  /* link will be encrypted iff the device is paired.   */
                   if(GAPEncryptionMode == emDisabled)
                  {
                  //    DeviceInfo = DeleteDeviceInfoEntry(&DeviceInfoList, ConnectionBD_ADDR);
                  //        if(NULL != DeviceInfo)
                  //        {
                  //                FreeDeviceInfoEntryMemory(DeviceInfo);
                  //        }
                  }              
                  
               }
               else
                  Display(("Warning - Disconnect from unknown device.\r\n"));

               /* Clear the saved Connection BD_ADDR.                   */
               ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
               LocalDeviceIsMaster = FALSE;
               GAPEncryptionMode = emDisabled;
            }
            AdvertiseLEEnable(0);
            break;
         
                        
            
         case etLE_Encryption_Change:
            Display(("etLE_Encryption_Change with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size));
             /* Verify that the link is currently encrypted.             */
            if((GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->Encryption_Change_Status == HCI_ERROR_CODE_NO_ERROR) && (GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->Encryption_Mode == emEnabled))
               ApplicationStateInfo.LEConnectionInfo.Flags |= CONNECTION_INFO_FLAGS_CONNECTION_ENCRYPTED;
            else
               ApplicationStateInfo.LEConnectionInfo.Flags &= ~CONNECTION_INFO_FLAGS_CONNECTION_ENCRYPTED;
            break;
         case etLE_Encryption_Refresh_Complete:
            Display(("etLE_Encryption_Refresh_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size));
            /* Verify that the link is currently encrypted.             */
            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Refresh_Complete_Event_Data->Status == HCI_ERROR_CODE_NO_ERROR)
               ApplicationStateInfo.LEConnectionInfo.Flags |= CONNECTION_INFO_FLAGS_CONNECTION_ENCRYPTED;
            else
               ApplicationStateInfo.LEConnectionInfo.Flags &= ~CONNECTION_INFO_FLAGS_CONNECTION_ENCRYPTED;
            break;
         case etLE_Authentication:
            Display(("etLE_Authentication with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size));

            /* Make sure the authentication event data is valid before  */
            /* continuing.                                              */
            if((Authentication_Event_Data = GAP_LE_Event_Data->Event_Data.GAP_LE_Authentication_Event_Data) != NULL)
            {
               BD_ADDRToStr(Authentication_Event_Data->BD_ADDR, BoardStr);

               switch(Authentication_Event_Data->GAP_LE_Authentication_Event_Type)
               {
                  case latLongTermKeyRequest:
                     Display(("    latKeyRequest: \r\n"));
                     Display(("      BD_ADDR: %s.\r\n", BoardStr));

                     /* The other side of a connection is requesting    */
                     /* that we start encryption. Thus we should        */
                     /* regenerate LTK for this connection and send it  */
                     /* to the chip.                                    */
                     Result = GAP_LE_Regenerate_Long_Term_Key(BluetoothStackID, (Encryption_Key_t *)(&DHK), (Encryption_Key_t *)(&ER), Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV, &(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand), &GeneratedLTK);
                     if(!Result)
                     {
                        Display(("      GAP_LE_Regenerate_Long_Term_Key Success.\r\n"));

                        /* Respond with the Re-Generated Long Term Key. */
                        GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                                        = larLongTermKey;
                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                        GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;
                        GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key       = GeneratedLTK;
                     }
                     else
                     {
                        DisplayFunctionError("      GAP_LE_Regenerate_Long_Term_Key", Result);

                        /* Since we failed to generate the requested key*/
                        /* we should respond with a negative response.  */
                        GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = larLongTermKey;
                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;
                     }

                     /* Send the Authentication Response.               */
                     Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                     if(!Result)
                     {
                       
                        /* Master is trying to re-encrypt the Link so   */
                        /* therefore we should cancel the Security Timer*/
                        /* if it is active.                             */
                        if(ApplicationStateInfo.LEConnectionInfo.SecurityTimerID)
                        {
                           BSC_StopTimer(BluetoothStackID, ApplicationStateInfo.LEConnectionInfo.SecurityTimerID);
                           ApplicationStateInfo.LEConnectionInfo.SecurityTimerID = 0;
                        }
                        
                     }
                     else
                        Display(("      GAP_LE_Authentication_Response returned %d.\r\n",Result));
                     break;
                  case latSecurityRequest:
                     /* Display the data for this event.                */
                     /* * NOTE * This is only sent from Slave to Master.*/
                     /*          Thus we must be the Master in this     */
                     /*          connection.                            */
                     Display(("    latSecurityRequest:.\r\n"));
                     Display(("      BD_ADDR: %s.\r\n", BoardStr));
                     Display(("      Bonding Type: %s.\r\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.Bonding_Type == lbtBonding)?"Bonding":"No Bonding")));
                     Display(("      MITM: %s.\r\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.MITM == TRUE)?"YES":"NO")));

                     /* Determine if we have previously paired with the */
                     /* device. If we have paired we will attempt to    */
                     /* re-establish security using a previously        */
                     /* exchanged LTK.                                  */
                     if((DeviceInfo = SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address_Type, GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address)) != NULL)
                     {
                        /* Determine if a Valid Long Term Key is stored */
                        /* for this device.                             */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                        {
                           Display(("Attempting to Re-Establish Security.\r\n"));

                           /* Attempt to re-establish security to this  */
                           /* device.                                   */
                           GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;
                           GAP_LE_Security_Information.Security_Information.Master_Information.LTK                 = DeviceInfo->LTK;
                           GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                           GAP_LE_Security_Information.Security_Information.Master_Information.Rand                = DeviceInfo->Rand;
                           GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                           Result = GAP_LE_Reestablish_Security(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                           if(Result)
                           {
                              DisplayFunctionError("GAP_LE_Reestablish_Security", Result);
                           }
                        }
                        else
                        {
                           CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                           /* We do not have a stored Link Key for this */
                           /* device so go ahead and pair to this       */
                           /* device.                                   */
                           SendPairingRequest(Authentication_Event_Data->BD_ADDR, TRUE);
                        }
                     }
                     else
                     {
                        CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                        /* There is no Key Info Entry for this device   */
                        /* so we will just treat this as a slave        */
                        /* request and initiate pairing.                */
                        SendPairingRequest(Authentication_Event_Data->BD_ADDR, TRUE);
                     }

                     break;
                  case latPairingRequest:
                     Display(("latPairingRequest: Pairing Request: %s.\r\n",BoardStr));
                     DisplayPairingInformation(Authentication_Event_Data->Authentication_Event_Data.Pairing_Request);

                     /* Master is trying to pair with us so therefore we*/
                     /* should cancel the Security Timer if it is       */
                     /* active.                                         */
                     if(ApplicationStateInfo.LEConnectionInfo.SecurityTimerID)
                     {
                        BSC_StopTimer(BluetoothStackID, ApplicationStateInfo.LEConnectionInfo.SecurityTimerID);
                        ApplicationStateInfo.LEConnectionInfo.SecurityTimerID = 0;
                     }

                     /* This is a pairing request. Respond with a       */
                     /* Pairing Response.                               */
                     /* * NOTE * This is only sent from Master to Slave.*/
                     /*          Thus we must be the Slave in this      */
                     /*          connection.                            */

                     /* Send the Pairing Response.                      */
                     SlavePairingRequestResponse(BluetoothStackID, Authentication_Event_Data->BD_ADDR);
                     break;
                  case latConfirmationRequest:
                     Display(("latConfirmationRequest.\r\n"));

                     if(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type == crtNone)
                     {
                        Display(("Invoking Just Works.\r\n"));

                        /* Just Accept Just Works Pairing.              */
                        GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = larConfirmation;

                        /* By setting the Authentication_Data_Length to */
                        /* any NON-ZERO value we are informing the GAP  */
                        /* LE Layer that we are accepting Just Works    */
                        /* Pairing.                                     */
                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length = DWORD_SIZE;

                        Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                        if(Result)
                           Display(("GAP_LE_Authentication_Response returned %d.\r\n",Result));
                     }
                     else
                     {
                        if(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type == crtPasskey)
                        {
                           Display(("Respond with: PassKeyResponse [passkey].\r\n"));

                           /* Flag that we are awaiting a Passkey Input.*/
                           ApplicationStateInfo.LEConnectionInfo.Flags         |= CONNECTION_INFO_FLAGS_CONNECTION_AWAITING_PASSKEY;
                           ApplicationStateInfo.LEConnectionInfo.PasskeyDigits  = 0;
                           ApplicationStateInfo.LEConnectionInfo.Passkey        = 0;
                        }
                        else
                        {
                           if(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type == crtDisplay)
                              Display(("Passkey: %06ld.\r\n", Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey));
                        }
                     }
                     break;
                  case latSecurityEstablishmentComplete:
                     Display(("Security Re-Establishment Complete: %s.\r\n", BoardStr));
                     Display(("                            Status: 0x%02X.\r\n", Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status));

                     /* Check to see if the Security Re-establishment   */
                     /* was successful (or if it failed since the remote*/
                     /* device attempted to re-pair.                    */
                     if((Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status != GAP_LE_SECURITY_ESTABLISHMENT_STATUS_CODE_NO_ERROR) && (Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status != GAP_LE_SECURITY_ESTABLISHMENT_STATUS_CODE_DEVICE_TRIED_TO_REPAIR))
                     {
                        /* Security Re-establishment was not successful */
                        /* so delete the stored device information and  */
                        /* disconnect the link.                         */
                        GAP_LE_Disconnect(BluetoothStackID, Authentication_Event_Data->BD_ADDR);

                        /* Delete the stored device info structure.     */
                        if((DeviceInfo = DeleteLEDeviceInfoEntry(&DeviceInfoList, ApplicationStateInfo.LEConnectionInfo.AddressType, ApplicationStateInfo.LEConnectionInfo.BD_ADDR)) != NULL)
                           FreeDeviceInfoEntryMemory(DeviceInfo);
                        SaveDeviceInfoList();
                     }
                     break;
                  case latPairingStatus:
                     ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);

                     Display(("latPairingStatus: Pairing Status: %s.\r\n", BoardStr));
                     Display(("        Status: 0x%02X.\r\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status));

                     /* Check to see if we have paired successfully with*/
                     /* the device.                                     */
                     if(Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status == GAP_LE_PAIRING_STATUS_NO_ERROR)
                     {
                        Display(("        Key Size: %d.\r\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Negotiated_Encryption_Key_Size));

                        /* Search for the Device entry for our current  */
                        /* LE connection.                               */
                        if((DeviceInfo = SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ApplicationStateInfo.LEConnectionInfo.AddressType, ApplicationStateInfo.LEConnectionInfo.BD_ADDR)) != NULL)
                        {
                           /* Save the encryption key size.             */
                           DeviceInfo->EncryptionKeySize = Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Negotiated_Encryption_Key_Size;
                           SPP_Paired_Device = DeviceInfo;
                        }
                        
                        
                        /* Set if encryption is enabled for current     */
                        /* connection                                   */
                        //GAP_LE_Query_Encryption_Mode(BluetoothStackID, ConnectionBD_ADDR, &GAPEncryptionMode);
                        
                        /* Pairing LE successful so disable pairing     */
                        /* capability and save Device pairing data      */
                        SaveDeviceInfoList();
                        SPP_event = SPP_EVT_LE_PAIR_COMPLETE;
                     }
                     else
                     {             
                       
                        /* Disconnect the Link.                         */
                        GAP_LE_Disconnect(BluetoothStackID, Authentication_Event_Data->BD_ADDR);
                        
                        /* Failed to pair so delete the key entry for   */
                        /* this device and disconnect the link.         */
                        if((DeviceInfo = DeleteLEDeviceInfoEntry(&DeviceInfoList, ApplicationStateInfo.LEConnectionInfo.AddressType, ApplicationStateInfo.LEConnectionInfo.BD_ADDR)) != NULL)
                           FreeDeviceInfoEntryMemory(DeviceInfo);
                        SaveDeviceInfoList();
                     }
                     break;
                  case latEncryptionInformationRequest:
                      Display(("latEncryptionInformationRequest: Encryption Information Request %s.\r\n", BoardStr));

                     /* Generate new LTK,EDIV and Rand and respond with */
                     /* them.                                           */
                     EncryptionInformationRequestResponse(Authentication_Event_Data->BD_ADDR, Authentication_Event_Data->Authentication_Event_Data.Encryption_Request_Information.Encryption_Key_Size, &GAP_LE_Authentication_Response_Information);
                     break;
                  case latEncryptionInformation:
                     /* Display the information from the event.         */
                     Display((" Encryption Information from RemoteDevice: %s.\r\n", BoardStr));
                     Display(("                             Key Size: %d.\r\n", Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size));

                     /* ** NOTE ** If we are the Slave we will NOT      */
                     /*            store the LTK that is sent to us by  */
                     /*            the Master.  However if it was ever  */
                     /*            desired that the Master and Slave    */
                     /*            switch roles in a later connection   */
                     /*            we could store that information at   */
                     /*            this point.                          */
                     if(LocalDeviceIsMaster)
                     {
                        /* Search for the entry for this slave to store */
                        /* the information into.                        */
                        if((DeviceInfo = SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address_Type, GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address)) != NULL)
                        {
                           DeviceInfo->LTK               = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.LTK;
                           DeviceInfo->EDIV              = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.EDIV;
                           DeviceInfo->Rand              = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Rand;
                           DeviceInfo->EncryptionKeySize = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size;
                           DeviceInfo->Flags            |= DEVICE_INFO_FLAGS_LTK_VALID;
                        }
                        else
                        {
                           Display(("No Key Info Entry for this Slave.\r\n"));
                        }
                     }
                     break;
                  case latIdentityInformation:
                     Display(("latIdentityInformation.\r\n"));

                     /* Search for the Device entry for our current LE  */
                     /* connection.                                     */
                     if((DeviceInfo = SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ApplicationStateInfo.LEConnectionInfo.AddressType, ApplicationStateInfo.LEConnectionInfo.BD_ADDR)) != NULL)
                     {
                        /* Store the received IRK and also updated the  */
                        /* BD_ADDR that is stored to the "Base" BD_ADDR.*/
                        DeviceInfo->ConnectionAddressType  = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address_Type;
                        DeviceInfo->ConnectionBD_ADDR      = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.Address;
                        DeviceInfo->IRK          = Authentication_Event_Data->Authentication_Event_Data.Identity_Information.IRK;
                        DeviceInfo->Flags       |= DEVICE_INFO_FLAGS_IRK_VALID;
                     }
                     break;                     
               }
            }
            break;
      }

      /* Display the command prompt.                                    */
      DisplayPrompt();
   }
}

/* The following function is for an GATT Connection Event Callback.  */
   /* This function is called for GATT Connection Events that occur on  */
   /* the specified Bluetooth Stack.  This function passes to the caller*/
   /* the GATT Connection Event Data that occurred and the GATT         */
   /* Connection Event Callback Parameter that was specified when this  */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the GATT Client Event Data ONLY in the context of this callback.  */
   /* If the caller requires the Data for a longer period of time, then */
   /* the callback function MUST copy the data into another Data Buffer.*/
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another GATT Event            */
   /* (Server/Client or Connection) will not be processed while this    */
   /* function call is outstanding).                                    */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter)
{
   BoardStr_t    BoardStr;
   Byte_t        AlertLevel;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Connection_Event_Data))
   {
      /* Determine the Connection Event that occurred.                  */
      switch(GATT_Connection_Event_Data->Event_Data_Type)
      {
         case etGATT_Connection_Device_Connection:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data)
            {
               /* Save the Connection ID for later use.                 */
               ConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID;

               Display(("\r\netGATT_Connection_Device_Connection with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size));
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID));
               Display(("   Connection Type: %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType == gctLE)?"LE":"BR/EDR")));
               Display(("   Remote Device:   %s.\r\n", BoardStr));
               Display(("   Connection MTU:  %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->MTU));

               ApplicationStateInfo.Flags                         |= APPLICATION_STATE_INFO_FLAGS_LE_CONNECTED;
               ApplicationStateInfo.LEConnectionInfo.ConnectionID  = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID;
               SPP_event = SPP_EVT_LE_CONNECT;
               AdvertisingStatus = FALSE;
               if(SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ApplicationStateInfo.LEConnectionInfo.AddressType, ApplicationStateInfo.LEConnectionInfo.BD_ADDR) != NULL)
               {
                  if(LocalDeviceIsMaster)
                  {
                     /* Attempt to update the MTU to the maximum        */
                     /* supported.                                      */
                     GATT_Exchange_MTU_Request(BluetoothStackID, ConnectionID, BTPS_CONFIGURATION_GATT_MAXIMUM_SUPPORTED_MTU_SIZE, GATT_ClientEventCallback_LLS, 0);
                  }
               }
               
              
            }
            else
            {
               Display(("Error - Null Connection Data.\r\n"));
            }
            
            BTActivity++;
            
            // Start BLE connection timer so we can force a disconnect if
            // the client sticks around too long
            //StartBLEConnetionTimer(BluetoothStackID, GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice);
             
            break;
            
         case etGATT_Connection_Device_Disconnection:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data)
            {
               /* Clear the Connection ID.                              */
               ConnectionID = 0;

               Display(("\r\netGATT_Connection_Device_Disconnection with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size));
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID));
               Display(("   Connection Type: %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType == gctLE)?"LE":"BR/EDR")));
               Display(("   Remote Device:   %s.\r\nn", BoardStr));

               Display(("\n  Link has been lost.\r\n"));
               if((LLSInstanceID) && (LLS_Query_Alert_Level(BluetoothStackID, LLSInstanceID, &AlertLevel) == 0))
                  Display(("\n  Alert Level = %u.\r\n", AlertLevel));
               
               // Stop BLE connection timer
               //StopBLEConnetionTimer();
               
            }
            else
               Display(("Error - Null Disconnection Data.\r\n"));
            break;
         case etGATT_Connection_Device_Buffer_Empty:
            SPP_event = SPP_EVT_GATT_BUFFER_EMPTY;
            break;
      }

      /* Print the command line prompt.                                 */
      DisplayPrompt();
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));

      Display(("GATT Connection Callback Data: Event_Data = NULL.\r\n"));

      DisplayPrompt();
   }
}

   /* The following represents the a BAS Event Callback.  This function */
   /* will be called whenever an BAS Event occurs that is associated    */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the BAS Event Data that        */
   /* occurred and the BAS Event Callback Parameter that was specified  */
   /* when this Callback was installed.  The caller is free to use the  */
   /* contents of the BAS Event Data ONLY in the context of this        */
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer This function is guaranteed NOT to be invoked more    */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e.  this function DOES NOT have to be re-entrant).It needs to  */
   /* be noted however, that if the same Callback is installed more than*/
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another BAS Event    */
   /* will not be processed while this function call is outstanding).   */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving BAS Event Packets.  */
   /*            A Deadlock WILL occur because NO BAS Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI BAS_Event_Callback(unsigned int BluetoothStackID, BAS_Event_Data_t *BAS_Event_Data, unsigned long CallbackParameter)
{
   int           Result;
   DeviceInfo_t *DeviceInfo;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (BAS_Event_Data))
   {
      /* Search for the Device entry for our current LE connection.     */
      if((DeviceInfo = SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ApplicationStateInfo.LEConnectionInfo.AddressType, ApplicationStateInfo.LEConnectionInfo.BD_ADDR)) != NULL)
      {
         /* Determine the Battery Service Event that occurred.          */
         switch(BAS_Event_Data->Event_Data_Type)
         {
            case etBAS_Server_Read_Client_Configuration_Request:
               if((BAS_Event_Data->Event_Data.BAS_Read_Client_Configuration_Data) && (BAS_Event_Data->Event_Data.BAS_Read_Client_Configuration_Data->ClientConfigurationType == ctBatteryLevel))
               {
                  Display(("Battery Read Battery Client Configuration Request.\r\n"));

                  Result = BAS_Read_Client_Configuration_Response(BluetoothStackID, BASInstanceID, BAS_Event_Data->Event_Data.BAS_Read_Client_Configuration_Data->TransactionID, DeviceInfo->BASServerInformation.Battery_Level_Client_Configuration);
                  if(Result)
                     Display(("Error - BAS_Read_Client_Configuration_Response() %d.\r\n", Result));
               }
               break;
            case etBAS_Server_Client_Configuration_Update:
               if((BAS_Event_Data->Event_Data.BAS_Client_Configuration_Update_Data) && (BAS_Event_Data->Event_Data.BAS_Client_Configuration_Update_Data->ClientConfigurationType == ctBatteryLevel))
               {
                  Display(("Battery Client Configuration Update: %s.\r\n", (BAS_Event_Data->Event_Data.BAS_Client_Configuration_Update_Data->Notify?"ENABLED":"DISABLED")));

                  /* Update the stored configuration for this device.   */
                  if(BAS_Event_Data->Event_Data.BAS_Client_Configuration_Update_Data->Notify)
                     DeviceInfo->BASServerInformation.Battery_Level_Client_Configuration = GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE;
                  else
                     DeviceInfo->BASServerInformation.Battery_Level_Client_Configuration = 0;
               }
               break;
            case etBAS_Server_Read_Battery_Level_Request:
               if(BAS_Event_Data->Event_Data.BAS_Read_Battery_Level_Data)
               {                  
                 
                  
                  //ADC_Bat_GetVal(&BatteryLevel);
                  BatteryLevel = ADC_Bat_GetPercent();
                  
                  Display(("Battery level read request = %d.\r\n", BatteryLevel));
                  
                  /* Just respond with the current Battery Level.       */
                  Result = BAS_Battery_Level_Read_Request_Response(BluetoothStackID, BAS_Event_Data->Event_Data.BAS_Read_Battery_Level_Data->TransactionID, (Byte_t)BatteryLevel);
                  if(Result)
                     Display(("Error - BAS_Battery_Level_Read_Request_Response() %d.\r\n", Result));
                  
                  BTActivity++;
               }
               
               break;             
         }
      }
      
      DisplayPrompt();
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));

      Display(("Battery Service Callback Data: Event_Data = NULL.\r\n"));
   }
    
}

   /* The following is a IAS Server Event Callback. This function will  */
   /* be called whenever an IAS Server Profile Event occurs that is     */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the IAS Event Data   */
   /* that occurred and the IAS Event Callback Parameter that was       */
   /* specified when this Callback was installed. The caller is free to */
   /* use the contents of the IAS Event Data ONLY in the context of this*/
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer This function is guaranteed NOT to be invoked more    */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e.  this function DOES NOT have be re-entrant).  It needs to be*/
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another IAS Event    */
   /* will not be processed while this function call is outstanding).   */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving IAS Event Packets.  */
   /*            A Deadlock WILL occur because NO IAS Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI IAS_EventCallback(unsigned int BluetoothStackID, IAS_Event_Data_t *IAS_Event_Data, unsigned long CallbackParameter)
{
   BoardStr_t    BoardStr;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (IAS_Event_Data))
   {
      switch(IAS_Event_Data->Event_Data_Type)
      {
         case etIAS_Server_Alert_Level_Control_Point_Command:
            Display(("etIAS_Server_Alert_Level_Control_Point_Command with size %u.\r\n", IAS_Event_Data->Event_Data_Size));

            if(IAS_Event_Data->Event_Data.IAS_Alert_Level_Control_Point_Command_Data)
            {
               BD_ADDRToStr(IAS_Event_Data->Event_Data.IAS_Alert_Level_Control_Point_Command_Data->RemoteDevice, BoardStr);
               Display(("   Instance ID:      %u.\r\n", IAS_Event_Data->Event_Data.IAS_Alert_Level_Control_Point_Command_Data->InstanceID));
               Display(("   Connection ID:    %u.\r\n", IAS_Event_Data->Event_Data.IAS_Alert_Level_Control_Point_Command_Data->ConnectionID));
               Display(("   Connection Type:  %s.\r\n", ((IAS_Event_Data->Event_Data.IAS_Alert_Level_Control_Point_Command_Data->ConnectionType == gctLE)?"LE":"BR/EDR")));
               Display(("   Remote Device:    %s.\r\n", BoardStr));

               if(SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ApplicationStateInfo.LEConnectionInfo.AddressType, ApplicationStateInfo.LEConnectionInfo.BD_ADDR) != NULL)
               {
                  /* Validate event parameters                          */
                  if(IAS_Event_Data->Event_Data.IAS_Alert_Level_Control_Point_Command_Data->InstanceID == IASInstanceID)
                  {
                     AlertLevelControlPointCommand = IAS_Event_Data->Event_Data.IAS_Alert_Level_Control_Point_Command_Data->Command;
                     switch(AlertLevelControlPointCommand)
                     {
                        case cpNoAlert:
                           Display(("   Command Type:      cpNoAlert.\r\n"));
                           break;

                        case cpMildAlert:
                           Display(("   Command Type:      cpMildAlert.\r\n"));
                           break;

                        case cpHighAlert:
                           Display(("   Command Type:      cpHighAlert.\r\n"));
                           break;

                        default:
                           Display(("   Command Type:      Unknown.\r\n"));
                           break;
                     }
                  }
                  else
                  {
                     Display(("\r\nInvalid Event data.\r\n"));
                  }
               }
               else
               {
                  Display(("\r\nUnknown Client.\r\n"));
               }
            }
            break;
         default:
            Display(("Unknown IAS Event\r\n"));
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));

      Display(("IAS Callback Data: Event_Data = NULL.\r\n"));
   }

   DisplayPrompt();
}

   /* The following is a LLS Server Event Callback. This function will  */
   /* be called whenever an LLS Server Profile Event occurs that is     */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the LLS Event Data   */
   /* that occurred and the LLS Event Callback Parameter that was       */
   /* specified when this Callback was installed. The caller is free to */
   /* use the contents of the LLS Event Data ONLY in the context of this*/
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer This function is guaranteed NOT to be invoked more    */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e.  this function DOES NOT have be re-entrant).  It needs to be*/
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another LLS Event    */
   /* will not be processed while this function call is outstanding).   */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving LLS Event Packets.  */
   /*            A Deadlock WILL occur because NO LLS Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI LLS_EventCallback(unsigned int BluetoothStackID, LLS_Event_Data_t *LLS_Event_Data, unsigned long CallbackParameter)
{
   BoardStr_t    BoardStr;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (LLS_Event_Data))
   {
      switch(LLS_Event_Data->Event_Data_Type)
      {
         case etLLS_Alert_Level_Update:
            Display(("etLLS_Alert_Level_Update with size %u.\r\n", LLS_Event_Data->Event_Data_Size));

            if(LLS_Event_Data->Event_Data.LLS_Alert_Level_Update_Data)
            {
               BD_ADDRToStr(LLS_Event_Data->Event_Data.LLS_Alert_Level_Update_Data->RemoteDevice, BoardStr);
               Display(("   Instance ID:      %u.\r\n", LLS_Event_Data->Event_Data.LLS_Alert_Level_Update_Data->InstanceID));
               Display(("   Connection ID:    %u.\r\n", LLS_Event_Data->Event_Data.LLS_Alert_Level_Update_Data->ConnectionID));
               Display(("   Connection Type:  %s.\r\n", ((LLS_Event_Data->Event_Data.LLS_Alert_Level_Update_Data->ConnectionType == gctLE)?"LE":"BR/EDR")));
               Display(("   Remote Device:    %s.\r\n", BoardStr));
               Display(("   Alert Level:      %u.\r\n", LLS_Event_Data->Event_Data.LLS_Alert_Level_Update_Data->AlertLevel));

               if(SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ApplicationStateInfo.LEConnectionInfo.AddressType, ApplicationStateInfo.LEConnectionInfo.BD_ADDR) != NULL)
               {
                  LLS_Set_Alert_Level(BluetoothStackID, LLSInstanceID, LLS_Event_Data->Event_Data.LLS_Alert_Level_Update_Data->AlertLevel);
               }
               else
               {
                  Display(("\r\nUnknown Client.\r\n"));
               }
            }
            break;
         default:
            Display(("Unknown LLS Event\r\n"));
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));
      Display(("LLS Callback Data: Event_Data = NULL.\r\n"));
   }
   DisplayPrompt();
}


   /* The following function is for an GATT Client Event Callback.  This*/
   /* function will be called whenever a GATT Response is received for a*/
   /* request that was made when this function was registered.  This    */
   /* function passes to the caller the GATT Client Event Data that     */
   /* occurred and the GATT Client Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the GATT Client Event Data ONLY in the context*/
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e.  this function DOES NOT have be reentrant).  It    */
   /* Needs to be noted however, that if the same Callback is installed */
   /* more than once, then the callbacks will be called serially.       */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* GATT Event (Server/Client or Connection) will not be processed    */
   /* while this function call is outstanding).                         */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_ClientEventCallback_LLS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   DeviceInfo_t *DeviceInfo;
   BoardStr_t    BoardStr;
   Byte_t       *Value;
   Word_t        ValueLength;
   Word_t        i;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data))
   {
      /* Determine the event that occurred.                             */
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
            {
               Display(("\r\nError Response.\r\n"));
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID));
               Display(("   Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID));
               Display(("   Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               Display(("   BD_ADDR:         %s.\r\n", BoardStr));
               Display(("   Error Type:      %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)?"Response Error":"Response Timeout"));

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
               {
                  Display(("   Request Opcode:  0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode));
                  Display(("   Request Handle:  0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle));
                  Display(("   Error Code:      0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode));
               }
            }
            else
               Display(("Error - Null Error Response Data.\r\n"));
            break;

         case etGATT_Client_Read_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               ValueLength = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength;
               Value = GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue;
               Display(("\r\nRead Response.\r\n"));
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionID));
               Display(("   Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->TransactionID));
               Display(("   Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               Display(("   BD_ADDR:         %s.\r\n", BoardStr));
               Display(("   Data Length:     %u.\r\n", ValueLength));

               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what read response this */
               /* is.                                                   */
               if(ValueLength != 0)
               {
                  if(((DeviceInfo = SearchLEDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ApplicationStateInfo.LEConnectionInfo.AddressType, ApplicationStateInfo.LEConnectionInfo.BD_ADDR)) != NULL) && (CallbackParameter != 0))
                  {
                     if(CallbackParameter == DeviceInfo->LLS_ClientInfo.Alert_Level)
                     {
                        if(ValueLength == NON_ALIGNED_BYTE_SIZE)
                        {
                           Display(("\r\n   LLS Alert Level: %u.\r\n", *Value));
                        }
                        else
                           Display(("\r\nError Invalid length  (%u) for Alert Level response\r\n", ValueLength));
                     }
                  }

                  /* If the data has not been decoded and displayed,    */
                  /* then just display the raw data                     */
                  if((DeviceInfo == NULL) || (CallbackParameter == 0))
                  {
                     Display(("   Data:            { "));
                     for(i = 0; i < (ValueLength - 1); i++)
                        Display(("0x%02x, ", Value + i));

                     Display(("0x%02x }\r\n", Value + i));
                  }
               }
            }
            else
               Display(("\r\nError - Null Read Response Data.\r\n"));
            break;

         case etGATT_Client_Exchange_MTU_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data)
            {
               Display(("\r\nExchange MTU Response.\r\n"));
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionID));
               Display(("   Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->TransactionID));
               Display(("   Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               Display(("   BD_ADDR:         %s.\r\n", BoardStr));
               Display(("   MTU:             %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ServerMTU));
            }
            else
               Display(("\r\nError - Null Write Response Data.\r\n"));
            break;

        case etGATT_Client_Write_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data)
            {
               Display(("\r\nWrite Response.\r\n"));
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionID));
               Display(("   Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->TransactionID));
               Display(("   Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               Display(("   BD_ADDR:         %s.\r\n", BoardStr));
               Display(("   Bytes Written:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->BytesWritten));
            }
            else
               Display(("\r\nError - Null Write Response Data.\r\n"));
            break;
      }

      /* Print the command line prompt.                                 */
      DisplayPrompt();
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));

      Display(("GATT Callback Data: Event_Data = NULL.\r\n"));

      DisplayPrompt();
   }
}

   /* The following function represents the Security Timer that is used */
   /* to ensure that the Master re-establishes security after pairing on*/
   /* subsequent connections.                                           */
static void BTPSAPI BSC_TimerCallback(unsigned int BluetoothStackID, unsigned int TimerID, unsigned long CallbackParameter)
{
   /* Verify that the input parameters are semi-valid.                  */
   if((BluetoothStackID) && (TimerID))
   {
      /* Verify that the LE Connection is still active.                 */
      if((ApplicationStateInfo.Flags & APPLICATION_STATE_INFO_FLAGS_LE_CONNECTED) && (ApplicationStateInfo.LEConnectionInfo.SecurityTimerID == TimerID))
      {
         /* Invalidate the Timer ID that just expired.                  */
         ApplicationStateInfo.LEConnectionInfo.SecurityTimerID = 0;

         /* If the connection is not currently encrypted then we will   */
         /* send a Slave Security Request.                              */
         if(!(ApplicationStateInfo.LEConnectionInfo.Flags & CONNECTION_INFO_FLAGS_CONNECTION_ENCRYPTED))
            SlaveSecurityReEstablishment(BluetoothStackID, ApplicationStateInfo.LEConnectionInfo.BD_ADDR);
      }
      else if(SPP_state_timer_id == TimerID)
      {
         SPP_state_timer_id = 0;
         SPP_event = SPP_EVT_SM_TIMEOUT;
      }
   }
}

   /* ***************************************************************** */
   /*                    End of Event Callbacks.                        */
   /* ***************************************************************** */

   /* The following function is used to initialize the application      */
   /* instance.  This function should open the stack and prepare to     */
   /* execute commands based on user input.  The first parameter passed */
   /* to this function is the HCI Driver Information that will be used  */
   /* when opening the stack and the second parameter is used to pass   */
   /* parameters to BTPS_Init.  This function returns the               */
   /* BluetoothStackID returned from BSC_Initialize on success or a     */
   /* negative error code (of the form APPLICATION_ERROR_XXX).          */
int InitializeApplication(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization)
{
   int ret_val = APPLICATION_ERROR_UNABLE_TO_OPEN_STACK;

   /* Initiailize some defaults.                                        */
   SerialPortID           = 0;
   UI_Mode                = UI_MODE_IS_SERVER;
   LoopbackActive         = FALSE;
   DisplayRawData         = FALSE;
   AutomaticReadActive    = FALSE;
   NumberofValidResponses = 0;

   /* Next, makes sure that the Driver Information passed appears to be */
   /* semi-valid.                                                       */
   if((HCI_DriverInformation) && (BTPS_Initialization))
   {
      BTActivity++;
      /* Try to Open the stack and check if it was successful.          */
      if(!OpenStack(HCI_DriverInformation, BTPS_Initialization))
      {
         BTActivity++;
         /* The stack was opened successfully.  Now set some defaults.  */
         GetLinkedKeyNb();
         
         /* First, attempt to set the Device to be Connectable.         */
#ifdef LOWPOWER_ENABLE
         ret_val = 0;
#else       
         ret_val = SetConnect(TRUE);
#endif
         /* Next, check to see if the Device was successfully made      */
         /* Connectable.                                                */
         if(!ret_val)
         {
            /* Now that the device is Connectable attempt to make it    */
            /* Discoverable.                                            */
            //ret_val = SetDisc(TRUE);

            /* Next, check to see if the Device was successfully made   */
            /* Discoverable.                                            */
            if(!ret_val)
            {
               /* Now that the device is discoverable attempt to make it*/
               /* pairable.                                             */
               //ret_val = SetPairable(TRUE);
               
               if(!ret_val) 
               {
                  ret_val = RegisterAuthentication();
               
                  if(!ret_val)
                  {
                      /* Assign a NULL BD_ADDR for comparison.           */
                      ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

#ifdef CONSOLE_SUPPORT           
                      /* Set up the Selection Interface.                 */
                      UserInterface_Server();
#endif
                     {
                       SPP_Configuration_Params_t SPPConfigurationParams;
                       ParameterList_t parm;
                       
                       SPPConfigurationParams.MaximumFrameSize   = 518;
                       SPPConfigurationParams.TransmitBufferSize = 1034;
                       SPPConfigurationParams.ReceiveBufferSize  = 3108;
                       ret_val = SPP_Set_Configuration_Parameters(BluetoothStackID, &SPPConfigurationParams);
                       
                       parm.NumberofParameters = 1;
                       parm.Params[0].intParam = 1;
                       OpenServer(&parm);

                       RegisterLLS();
                       RegisterIAS();
                       RegisterTPS();
                       RegisterPasswordVault();
                       SPP_event = SPP_EVT_INIT_START;
                       BTActivity++;
                     }
#ifdef CONSOLE_SUPPORT           
                     /* Display the first command prompt.               */
                      DisplayPrompt();
#endif
                      /* Return success to the caller.                   */
                      ret_val = (int)BluetoothStackID;
                  } 
                  else 
                  {
#ifdef CONSOLE_SUPPORT           
                     DisplayFunctionError("Register Auth", ret_val);
#endif                    
                  }
               }
               else {
#ifdef CONSOLE_SUPPORT           
                     DisplayFunctionError("SetPairable", ret_val);
#endif
               }
            }
            else {
#ifdef CONSOLE_SUPPORT           
               DisplayFunctionError("SetDisc", ret_val);
#endif
            }
         }
         else {
#ifdef CONSOLE_SUPPORT           
            DisplayFunctionError("SetDisc", ret_val);
#endif
         }

         /* In some error occurred then close the stack.                */
         if(ret_val < 0)
         {
            /* Close the Bluetooth Stack.                               */
            CloseStack();
         }
      }
      else
      {
         /* There was an error while attempting to open the Stack.      */
         Display(("Unable to open the stack.\r\n"));
      }
   }
   else
      ret_val = APPLICATION_ERROR_INVALID_PARAMETERS;

   // Leave card in pairing mode if it hasn't already been paired
   if(BTPaired)
   {
     // Now that card is paired, set it no more discoverable or pairable
     //SetDisc(FALSE);
     //SetPairable(FALSE);
   }
   
   return(ret_val);
}


static int GetLinkedKeyNb(void)
{
  FIL fp;
  FRESULT res;
  UINT read;
  LinkKeyInfo_t BtKey;
  BD_ADDR_t NULL_BD_ADDR; 
  
  BT_LinkedDeviceNb = 0;
  BTPaired = FALSE;
  ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  eMMC_PowerOn();
  res = f_open(&fp, BT_KEY_FILENAME, FA_READ | FA_OPEN_EXISTING);
  if (res == FR_OK) 
  {
    do
    {
      res = f_read(&fp, (void *) &BtKey, sizeof(BtKey), &read);
      if(read == sizeof(BtKey))
      {
        if(!COMPARE_BD_ADDR(NULL_BD_ADDR, BtKey.BD_ADDR))
        {
          BT_LinkedDeviceNb++;
        }
      }
    }while(read == sizeof(BtKey));
    f_close(&fp);
  }
    
  if(BT_LinkedDeviceNb > 0)
  {
      BTPaired = TRUE;
    }
  
  slogf(LOG_DEST_BOTH, "BT_LinkedDeviceNb = %d",BT_LinkedDeviceNb);
   
  return (res);
}

static int LoadDeviceInfoList(void)
{
  FIL fp;
  FRESULT res;
  UINT read; 
  DeviceInfo_t DeviceInfo;
  DeviceInfo_t *DeviceInfoPtr;
  DeviceInfo_t *PreviousDeviceInfoPtr = NULL;
  
   eMMC_PowerOn();
   res = f_open(&fp, BT_LE_KEY_FILENAME, FA_READ | FA_OPEN_EXISTING);
   if (res == FR_OK)
   {
     do
     {
       res = f_read(&fp, (void *) &DeviceInfo, sizeof(DeviceInfo_t), &read);
       if(read == sizeof(DeviceInfo_t))
       {
         /* Allocate the memory for the entry.                             */
         if((DeviceInfoPtr = BTPS_AllocateMemory(sizeof(DeviceInfo_t))) != NULL)
         {
            /* Initialize the entry.                                       */
            BTPS_MemInitialize(DeviceInfoPtr, 0, sizeof(DeviceInfo_t));
            BTPS_MemCopy(DeviceInfoPtr, &DeviceInfo, sizeof(DeviceInfo_t));
            DeviceInfoPtr->NextDeviceInfoPtr = NULL;

            /* Initialize the list head                                    */
            if(DeviceInfoList == NULL) 
            {
              DeviceInfoList = DeviceInfoPtr;
            }
            else
            {
              PreviousDeviceInfoPtr->NextDeviceInfoPtr = DeviceInfoPtr;
            }
            PreviousDeviceInfoPtr = DeviceInfoPtr;


            slogf(LOG_DEST_BOTH, "Read DeviceInfo");
         }
       }
     } while(read == sizeof (DeviceInfo_t));
     slogf(LOG_DEST_BOTH, "Loaded DeviceInfoList");
     f_close(&fp);
   }
   return res;
}

int GetLinkedKey(BD_ADDR_t BTAdd, LinkKeyInfo_t* pKey)
{
  FIL fp;
  FRESULT res;
  UINT read; 
  LinkKeyInfo_t BtKey;
  bool KeyFound = FALSE;
  
  eMMC_PowerOn();
  res = f_open(&fp, BT_KEY_FILENAME, FA_READ | FA_OPEN_EXISTING);
  if (res == FR_OK) 
  {
    do
    {
      res = f_read(&fp, (void *) &BtKey, sizeof(BtKey), &read);
      if(read == sizeof(BtKey))
      {
        if(COMPARE_BD_ADDR(BTAdd, BtKey.BD_ADDR))
        {
          memcpy(pKey, &BtKey, sizeof(LinkKeyInfo_t));
          KeyFound = TRUE;
          break;
        }
      }
    }while(read == sizeof(BtKey));
    f_close(&fp);
  }
    
  return (KeyFound);
}

int BT_WriteABuffer(const char * lpBuf, DWORD dwToWrite){
   int  Result;
   int  ret_val = 0;
   char * ptr = (char *)lpBuf;
   TickType_t xTO = Settings.SPP_Send_Timeout;
   TickType_t xTimeOut;
   
   xTimeOut = xTaskGetTickCount() + xTO;


   if((BluetoothStackID) && (SerialPortID))
   {
      while(dwToWrite) {
        Result = SPP_Data_Write(BluetoothStackID, SerialPortID, (Word_t)dwToWrite, (Byte_t *)ptr);
        if (Result < 0) {
          ret_val = Result;
          break;
        }
        if (xTaskGetTickCount() > xTimeOut) {
          // Timeout, communication is broken
          ret_val = -1;
          break;
        }
        dwToWrite -= Result;
        ptr += Result;
        if (dwToWrite) {
          //printf("tow: %d/%d\r\n", dwToWrite, Result);
          BTPS_Delay(5);
        }
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
  
}

static int SaveDeviceInfoList(void)
{
  FIL fp;
  FRESULT res = FR_OK;
  UINT written;
  DeviceInfo_t *DeviceInfo = DeviceInfoList;
  
  eMMC_PowerOn();
  res = f_open(&fp, BT_LE_KEY_FILENAME, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
  if(res == FR_OK)
  {
    slogf(LOG_DEST_BOTH, "Open btle.key");
    while((DeviceInfo != NULL) && (res == FR_OK)) {
      res = f_write(&fp, (void *) DeviceInfo, sizeof(DeviceInfo_t), &written);
      DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
    }
    f_close(&fp);
    
    if (written != sizeof(DeviceInfo_t)) 
    {
      res = FR_DISK_ERR;
    }
    else 
    {  
      slogf(LOG_DEST_BOTH, "Save DeviceInfoList success.");
    }
  }
  return (res);
}

//YouShouldFreeThisVectorAfterUsage
LinkKeyInfo_t *ReturnAllLinkedKey(int *len){
  FIL fp;
  FRESULT res;
  LinkKeyInfo_t BtKey;
  LinkKeyInfo_t *keys;
  UINT read;
  BD_ADDR_t NULL_BD_ADDR;
  UINT index=0;
  
  ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  
  eMMC_PowerOn();
  res = f_open(&fp, BT_KEY_FILENAME, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
  if(res == FR_OK){
    do{
      res = f_read(&fp, (void *) &BtKey, sizeof(BtKey), &read);
      if(read == sizeof(BtKey)){
        if(COMPARE_BD_ADDR(NULL_BD_ADDR, BtKey.BD_ADDR)){
          f_lseek(&fp, index*sizeof(BtKey));
          break;
        }
        index++;
      }
    }while(read == sizeof(BtKey));
  }
  f_close(&fp);

  *len = index;
  keys = (LinkKeyInfo_t *)malloc(index * sizeof(LinkKeyInfo_t));
  index = 0;
  res = f_open(&fp, BT_KEY_FILENAME, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
  if(res == FR_OK){
    do{
      res = f_read(&fp, (void *) &BtKey, sizeof(BtKey), &read);
      if(read == sizeof(BtKey)){
        if(COMPARE_BD_ADDR(NULL_BD_ADDR, BtKey.BD_ADDR)){
          f_lseek(&fp, index * sizeof(BtKey));
          break;
        }
        memcpy(keys + index, &BtKey, sizeof(BtKey));
        index++;
      }
    }while(read == sizeof(BtKey));
  }
  f_close(&fp);

  return (keys);
}

int AddLinkedKey(LinkKeyInfo_t * pKeyInfo)
{
  FIL fp;
  FRESULT res;
  UINT written;
  LinkKeyInfo_t BtKey;
  UINT read;
  BD_ADDR_t NULL_BD_ADDR;
  BD_ADDR_t NEW_BD_ADDR;
  UINT index=0;
  
  ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  NEW_BD_ADDR = pKeyInfo->BD_ADDR;
  
  eMMC_PowerOn();
  res = f_open(&fp, BT_KEY_FILENAME, FA_READ | FA_WRITE | FA_OPEN_ALWAYS);
  if(res == FR_OK)
  {
    do
    {
      res = f_read(&fp, (void *) &BtKey, sizeof(BtKey), &read);
      if(read == sizeof(BtKey))
      {
        if(COMPARE_BD_ADDR(NULL_BD_ADDR, BtKey.BD_ADDR))
        {
          f_lseek(&fp, index*sizeof(BtKey));
          break;
        }
        if(COMPARE_BD_ADDR(NEW_BD_ADDR, BtKey.BD_ADDR))
        {
          f_lseek(&fp, index*sizeof(BtKey));
          break;
        }
        index++;
      }
    }while(read == sizeof(BtKey));
    res = f_write(&fp, (void *) pKeyInfo, sizeof(LinkKeyInfo_t), &written);
    f_close(&fp);
    if (written != sizeof(LinkKeyInfo_t)) 
    {
      res = FR_DISK_ERR;
    } else 
    {  
      BTPaired = TRUE;
      BT_LinkedDeviceNb++;
    }
  }
  return (res);
}

   /* The following function handles the sleep indication callbacks from*/
   /* the HCI transport.                                                */
static void Sleep_Indication_Callback(Boolean_t SleepAllowed, unsigned long CallbackParameter)
{
   /* Verify parameters.                                                */
   if(CallbackParameter)
   {
      if(SleepAllowed)
      {
         /* Attempt to suspend the transport.                           */
         HCITR_COMSuspend(1);
      }
   }
}

int8_t SPP_Sleep(void) 
{
   int8_t result = -1;
   
   if(SPP_state_timer_id) {
      BSC_StopTimer(BluetoothStackID, SPP_state_timer_id);
      SPP_state_timer_id = 0;
   }
   result = DisconnectLE();
   f_close(&fp_download);
   SetDisc(FALSE);
   SetPairable(FALSE);
#ifdef LOWPOWER_ENABLE
   SetConnect(FALSE);
#endif
   SetPairableLE(FALSE);
   pairing_mode = 0;
   SPP_state = SPP_STATE_IDLE;
   osDelay(100);
   
   return result;
}

int8_t SPP_Enter_Pairing_Mode(void) 
{
   int8_t result = BSC_StartTimer(BluetoothStackID, 5*60000, BSC_TimerCallback, 0);
   if(result > 0) {
     SPP_state_timer_id = result;
     DisconnectLE();
     SetDisc(TRUE);
     SetConnect(TRUE);
     SetPairable(TRUE);
     SetPairableLE(TRUE);
     pairing_mode = 1;
   }
   return result;
}

void Set_SPP_Event(uint8_t event)
{
  SPP_event = event;
}

uint8_t SPP_Pairing_Mode(void) 
{
  return pairing_mode;
}



   /* The following function is the main user interface thread.  It     */
   /* opens the Bluetooth Stack and then drives the main user interface.*/
void SPPThread(void const *argument)
{

   int                           Result;
   BTPS_Initialization_t         BTPS_Initialization;
   HCI_DriverInformation_t       HCI_DriverInformation;
   HCI_HCILLConfiguration_t      HCILLConfig;
   HCI_Driver_Reconfigure_Data_t DriverReconfigureData;
   uint8_t result = 0;
   DeviceInfo_t                 *DeviceInfo;
   FIL fp;
   FRESULT res;
   UINT read;
   int16_t sent;
   
#ifdef CONSOLE_SUPPORT
   HAL_ConfigureConsole(&UartHandle);
   /* Set up the application callbacks.                                 */
   BTPS_Initialization.MessageOutputCallback = DisplayCallback;
#else   
   /* Set up the application callbacks.                                 */
   BTPS_Initialization.MessageOutputCallback = NULL; 
#endif // CONSOLE_SUPPORT
   /* Configure the UART Parameters.                                    */
   HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, 1, Settings.BTBaudRate, cpHCILL_RTS_CTS);
   HCI_DriverInformation.DriverInformation.COMMDriverInformation.InitializationDelay = 100;


   /* Initialize the application.                                       */
   if((Result = InitializeApplication(&HCI_DriverInformation, &BTPS_Initialization)) > 0)
   {
      /* Register a sleep mode callback if we are using HCILL Mode.     */
      if((HCI_DriverInformation.DriverInformation.COMMDriverInformation.Protocol == cpHCILL) || (HCI_DriverInformation.DriverInformation.COMMDriverInformation.Protocol == cpHCILL_RTS_CTS))
      {
         HCILLConfig.SleepCallbackFunction        = Sleep_Indication_Callback;
         HCILLConfig.SleepCallbackParameter       = 0;
         DriverReconfigureData.ReconfigureCommand = HCI_COMM_DRIVER_RECONFIGURE_DATA_COMMAND_CHANGE_HCILL_PARAMETERS;
         DriverReconfigureData.ReconfigureData    = (void *)&HCILLConfig;

         /* Register the sleep mode callback.  Note that if this        */
         /* function returns greater than 0 then sleep is currently     */
         /* enabled.                                                    */
         Result = HCI_Reconfigure_Driver((unsigned int)Result, FALSE, &DriverReconfigureData);
         if(Result > 0)
         {
            /* Flag that sleep mode is enabled.                         */
            Display(("Sleep is allowed.\r\n"));
         }
      }
      
      SelfTest.SerialBT = SELF_TEST_SUCCESS;
      
      //eMMC_TurnOff = TRUE;

      /* Loop forever and process UART characters.                      */
      while(1)
      {
#ifdef CONSOLE_SUPPORT           
         ProcessCharacters(NULL);
#endif // CONSOLE_SUPPORT           
         
         switch(SPP_state) {
         case SPP_STATE_INIT:
            switch(SPP_event){
            case SPP_EVT_INIT_START:
               AdvertiseLEEnable(0);
               if((!BTPaired) && (SPP_Enter_Pairing_Mode() > 0)) {
                  SPP_state = SPP_STATE_PAIRING_MODE;
               } else {
                  SPP_state = SPP_STATE_IDLE;
               }
               break;
            }
            break;
         case SPP_STATE_IDLE:
            switch(SPP_event){
            case SPP_EVT_BUTTON_WAKEUP:                   
               break;
            case SPP_EVT_ENABLE_EDR:
               SetConnect(TRUE);
               SPP_state = SPP_STATE_EDR_ENABLED;
               break;
            case SPP_EVT_BUTTON_MEDIUM_PRESS:
               if(SPP_Enter_Pairing_Mode() > 0) {
                 SPP_state = SPP_STATE_PAIRING_MODE;
               }
               break;
            case SPP_EVT_LE_SEND_FILE:
               res = f_open(&fp, le_transfer_filepath, FA_READ | FA_OPEN_EXISTING);
               if (res == FR_OK)
               {
                  slogf(LOG_DEST_BOTH,"Sending data. . .");
                  SPP_state = SPP_STATE_LE_FILE_TRANSFER_START;
               }
               break;
            }
            break;
         case SPP_STATE_EDR_ENABLED:
            switch(SPP_event){
            case SPP_EVT_LE_DISCONNECT:
            case SPP_EVT_SPP_DISCONNECT:
#ifdef LOWPOWER_ENABLE
               SetConnect(FALSE);
#endif
               SPP_state = SPP_STATE_IDLE;
               break;
            }
            break;
         case SPP_STATE_PAIRING_MODE:
            switch(SPP_event){
            case SPP_EVT_SM_TIMEOUT:
               SPP_Sleep();
               break;
            case SPP_EVT_LE_PAIR_COMPLETE:
               BSC_StopTimer(BluetoothStackID, SPP_state_timer_id);
               SetConnect(FALSE);
               result = BSC_StartTimer(BluetoothStackID, 5*60000, BSC_TimerCallback, 0);
               if(result > 0) {
                  SPP_state_timer_id = result;                
               }

               osDelay(1000);
               
               SetConnect(TRUE);
               DisconnectLE();
               SPP_state = SPP_STATE_PM_WAIT_EDR;
               break;
            case SPP_EVT_EDR_PAIR_COMPLETE:
               SPP_state = SPP_STATE_PM_WAIT_LE;
               break;
            case SPP_EVT_LE_CONNECT:
               BSC_StopTimer(BluetoothStackID, SPP_state_timer_id);
               result = BSC_StartTimer(BluetoothStackID, 10000, BSC_TimerCallback, 0);
               if(result > 0) {
                  SPP_state_timer_id = result;
               }
               break;
            }
            break;
         case SPP_STATE_PM_WAIT_LE:
            switch(SPP_event){
            case SPP_EVT_LE_PAIR_COMPLETE:
               BSC_StopTimer(BluetoothStackID, SPP_state_timer_id);

               osDelay(1000);

               SPP_Sleep();
               break;
            case SPP_EVT_SM_TIMEOUT:
               SPP_Sleep();
               break;
            case SPP_EVT_LE_CONNECT:
               BSC_StopTimer(BluetoothStackID, SPP_state_timer_id);
               result = BSC_StartTimer(BluetoothStackID, 10000, BSC_TimerCallback, 0);
               if(result > 0) {
                  SPP_state_timer_id = result;
               }
               break;
            }
            break;
         case SPP_STATE_PM_WAIT_EDR:
            switch(SPP_event){
            case SPP_EVT_EDR_PAIR_COMPLETE:
               SPP_Sleep();
               break;
            case SPP_EVT_SM_TIMEOUT:
               SPP_Sleep();
               break;            
            }
            break;
         case SPP_STATE_LE_FILE_TRANSFER_START:
            res = f_read(&fp, (void *) &PWVBuffer, PWV_DATA_BUFFER_LENGTH, &read);
            if((res == FR_OK) && (read <= PWV_DATA_BUFFER_LENGTH))
            {
               RTC_GetElapsedTime(&LETransferStartTime);
               SPP_state = SPP_STATE_LE_FILE_TRANSFER_ACTIVE;
            }
            else
            {
               SPP_state = SPP_STATE_IDLE;
            }
         case SPP_STATE_LE_FILE_TRANSFER_ACTIVE:
            sent = PWVSendData(BluetoothStackID, le_transfer_DeviceInfo, read - le_transfer_index, PWVBuffer + le_transfer_index);
            if((sent + le_transfer_index == read))
            {
               if(read < PWV_DATA_BUFFER_LENGTH)
               {
                  slogf(LOG_DEST_BOTH, "Sent file");
                  le_transfer_index = 0;
                  f_close(&fp);
                  RTC_GetElapsedTime(&LETransferEndTime);
                  slogf(LOG_DEST_BOTH, "File transfer time: %d", LETransferEndTime - LETransferStartTime);
                  SPP_state = SPP_STATE_IDLE;
               }
               else
               {
                  le_transfer_index = 0;
                  SPP_state = SPP_STATE_LE_FILE_TRANSFER_START;
               }
            }
            else
            {
               if(SPP_event == SPP_EVT_GATT_BUFFER_FULL)
               {
                  slogf(LOG_DEST_BOTH, "GATT buffers full");
                  le_transfer_index += sent;
                  SPP_state = SPP_STATE_LE_FILE_TRANSFER_HOLD;
               }
               else
               {
                  slogf(LOG_DEST_BOTH, "Error sending file over LE");
                  le_transfer_index = 0;
                  f_close(&fp);
                  SPP_state = SPP_STATE_IDLE;
               }
            }
            break;
         case SPP_STATE_LE_FILE_TRANSFER_HOLD:
            switch(SPP_event){
            case SPP_EVT_GATT_BUFFER_EMPTY:
               slogf(LOG_DEST_BOTH, "GATT buffers ready");
               SPP_state = SPP_STATE_LE_FILE_TRANSFER_ACTIVE;
               break;
            }
            break;
         }
         SPP_event = SPP_EVT_NONE;
         
         if (Settings.BT_DisconnectOnVUSB) {
           static uint8_t PmicChdetTrk = -1;
           if ((PMICStatus & PMIC_STAT_CHDET) != PmicChdetTrk) {
             // VUSB has changed
             PmicChdetTrk = (PMICStatus & PMIC_STAT_CHDET);
             if (PMICStatus & PMIC_STAT_CHDET) {
               // VUSB present
               // Disable pairability, discoverability and close SPP server
               SPP_Off();
               SPP_event = SPP_EVT_PLUG_IN;
             } else {
               // VUSB absent
               SPP_On();
             }
           }
           
         } else {

        }

         BTPS_Delay(200);
      }
   }
   
   printf("Stack failed\r\n");
   while(1)
   {
     BTPS_Delay(10);
   }
}

#ifdef CONSOLE_SUPPORT // For console support
#define MAX_COMMAND_LENGTH                         (64)  /* Denotes the max   */
                                                         /* buffer size used  */
                                                         /* for user commands */
                                                         /* input via the     */
                                                         /* User Interface.   */

#define  COMMAND_LINE_ENTER_PRESSED               (-128) /* The return value  */
                                                         /* for Enter char in */
                                                         /* GetInput function */


  
/* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int InputIndex;
static char         Input[MAX_COMMAND_LENGTH];

    /* The following function is used to process a command line string.  */
    /* This function takes as it's only parameter the command line string*/
    /* to be parsed and returns TRUE if a command was parsed and executed*/
    /* or FALSE otherwise.                                               */
Boolean_t ProcessCommandLine(char *String)
{
   return(CommandLineInterpreter(String));
}


char ConsoleBuf[32];

int HAL_ConsoleWrite(UART_HandleTypeDef *UartHandle, int Length, char *Buffer)
{
  int ret_val = 0;
  int count;
  
  if ((Length) && (Buffer)) {
    while (Length) {
      // wait end of previous message
      while ((UartHandle->State == HAL_UART_STATE_BUSY_TX) || 
             (UartHandle->State == HAL_UART_STATE_BUSY_TX_RX)) {
        BTPS_Delay(1);
      }
      if (Length > sizeof(ConsoleBuf)) {
        count = sizeof(ConsoleBuf);
      } else {
        count = Length;
      }
      BTPS_MemCopy(ConsoleBuf, Buffer, count);
      
      if (HAL_UART_Transmit_IT(UartHandle, (uint8_t *)ConsoleBuf, count) != HAL_OK) {
        Length = 0;
      }
      ret_val += count;
      Length -= count;
      Buffer += count;
    }
  }

#if 0
  // wait end of current message
  while (UartHandle->TxXferCount) {
    BTPS_Delay(1);
  }
#endif
  return (ret_val);
}

#define HAL_INPUT_BUFFER_SIZE             64

typedef struct _tag_HAL_ConsoleContext
{
  unsigned char         RxBuffer[HAL_INPUT_BUFFER_SIZE];
  unsigned int          RxBufferSize;
  volatile unsigned int RxBytesFree;
  unsigned int          RxInIndex;
  unsigned int          RxOutIndex;

} HAL_ConsoleContext_t;

HAL_ConsoleContext_t HAL_ConsoleContext;

   /* The following function is used to retrieve data from the UART     */
   /* input queue.  the function receives a pointer to a buffer that    */
   /* will receive the UART characters a the length of the buffer.  The */
   /* function will return the number of characters that were returned  */
   /* in Buffer.                                                        */
int HAL_ConsoleRead(int Length, char *Buffer)
{
   int ret_val;

   if((Length) && (Buffer))
   {
      /* Set the size to be copied equal to the smaller of the length   */
      /* and the bytes in the receive buffer.                           */
      ret_val = HAL_ConsoleContext.RxBufferSize - HAL_ConsoleContext.RxBytesFree;
      ret_val = (ret_val < Length) ? ret_val : Length;

      if(ret_val > (HAL_ConsoleContext.RxBufferSize - HAL_ConsoleContext.RxOutIndex))
      {
         /* The data wraps around the end of the buffer, so copy it in  */
         /* two steps.                                                  */
         Length = (HAL_ConsoleContext.RxBufferSize - HAL_ConsoleContext.RxOutIndex);
         BTPS_MemCopy(Buffer, &HAL_ConsoleContext.RxBuffer[HAL_ConsoleContext.RxOutIndex], Length);
         BTPS_MemCopy((Buffer + Length), HAL_ConsoleContext.RxBuffer, (ret_val - Length));

         HAL_ConsoleContext.RxOutIndex = ret_val - Length;
      }
      else
      {
         BTPS_MemCopy(Buffer, &HAL_ConsoleContext.RxBuffer[HAL_ConsoleContext.RxOutIndex], ret_val);

         HAL_ConsoleContext.RxOutIndex += ret_val;

         if(HAL_ConsoleContext.RxOutIndex == HAL_ConsoleContext.RxBufferSize)
            HAL_ConsoleContext.RxOutIndex = 0;
      }

      HAL_ConsoleContext.RxBytesFree += ret_val;
   }
   else
      ret_val = 0;

   return(ret_val);
}

void HAL_ConfigureConsole(UART_HandleTypeDef *UartHandle)
{
   BTPS_MemInitialize(&HAL_ConsoleContext, 0, sizeof(HAL_ConsoleContext_t));

   HAL_ConsoleContext.RxBufferSize = HAL_INPUT_BUFFER_SIZE;
   HAL_ConsoleContext.RxBytesFree  = HAL_INPUT_BUFFER_SIZE;
   
   while (UartHandle->Lock != HAL_UNLOCKED) {
     BTPS_Delay(10);
   }
     
   HAL_UART_Receive_IT(UartHandle, &(HAL_ConsoleContext.RxBuffer[HAL_ConsoleContext.RxInIndex]), 1);

}

void HAL_Console_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (HAL_ConsoleContext.RxBytesFree) {
    HAL_ConsoleContext.RxBytesFree --;
    HAL_ConsoleContext.RxInIndex ++;

    /* See if we need to roll the RxInIndex back to 0.                */
    if(HAL_ConsoleContext.RxInIndex == HAL_ConsoleContext.RxBufferSize) {
      HAL_ConsoleContext.RxInIndex = 0;
    }
    if (HAL_ConsoleContext.RxBytesFree) {
      HAL_UART_Receive_IT(huart, &(HAL_ConsoleContext.RxBuffer[HAL_ConsoleContext.RxInIndex]), 1);
    }
  }
}


/* The following function is registered with the application so that */
   /* it can display strings to the debug UART.                         */
static int DisplayCallback(int Length, char *Message)
{
   while(!HAL_ConsoleWrite(&UartHandle, Length, Message))
   {
	   BTPS_Delay(1);
   }

    return TRUE;
}

   /* The following function is responsible for retrieving the Commands */
   /* from the Serial Input routines and copying this Command into the  */
   /* specified Buffer.  This function blocks until a Command (defined  */
   /* to be a NULL terminated ASCII string).  The Serial Data Callback  */
   /* is responsible for building the Command and dispatching the Signal*/
   /* that this function waits for.                                     */
static int GetInput(void)
{
   char Char;
   int  Done;

   /* Initialize the Flag indicating a complete line has been parsed.   */
   Done = 0;

   /* Attempt to read data from the Console.                            */
   while((!Done) && (HAL_ConsoleRead(1, &Char)))
   {
      switch(Char)
      {
         case '\r':
         case '\n':
            /* The user just pressed 'Enter'. Return and print the prompt */
            if(!InputIndex)
            {
                /* Set the return value to (-128) */
                Done = COMMAND_LINE_ENTER_PRESSED; 
                break;
            }
            /* This character is a new-line or a line-feed character    */
            /* NULL terminate the Input Buffer and erase the following  */
			/* Char without the need of clearing the entire buffer      */
			/* before reading.                                          */
            Input[InputIndex]   = '\0';
			Input[InputIndex+1] = '\0';

            /* Set Done to the number of bytes that are to be returned. */
            /* ** NOTE ** In the current implementation any data after a*/
            /*            new-line or line-feed character will be lost. */
            /*            This is fine for how this function is used is */
            /*            no more data will be entered after a new-line */
            /*            or line-feed.                                 */
            Done       = (InputIndex-1);
            InputIndex = 0;
            break;
         case 0x08:
            /* Backspace has been pressed, so now decrement the number  */
            /* of bytes in the buffer (if there are bytes in the        */
            /* buffer).                                                 */
            if(InputIndex)
            {
               InputIndex--;

               while(!HAL_ConsoleWrite(&UartHandle, 1, "\b"))
                  ;
               while(!HAL_ConsoleWrite(&UartHandle, 1, " "))
                  ;
               while(!HAL_ConsoleWrite(&UartHandle, 1, "\b"))
                  ;
            }
            break;
         case 0x7F:
            /* Backspace has been pressed, so now decrement the number  */
            /* of bytes in the buffer (if there are bytes in the        */
            /* buffer).                                                 */
            if(InputIndex)
            {
               InputIndex--;
               while(!HAL_ConsoleWrite(&UartHandle, 1, "\b"))
                  ;
               while(!HAL_ConsoleWrite(&UartHandle, 1, " "))
                  ;
               while(!HAL_ConsoleWrite(&UartHandle, 1, "\b"))
                  ;
            }
            break;
         default:
            /* Accept any other printable characters.                   */
            if((Char >= ' ') && (Char <= '~'))
            {
               /* Add the Data Byte to the Input Buffer, and make sure  */
               /* that we do not overwrite the Input Buffer.            */
               Input[InputIndex++] = Char;
               while(!HAL_ConsoleWrite(&UartHandle, 1, &Char))
                  ;

               /* Check to see if we have reached the end of the buffer.*/
               if(InputIndex == (MAX_COMMAND_LENGTH-1))
               {
                  Input[InputIndex] = 0;
                  Done              = (InputIndex-1);
                  InputIndex        = 0;
               }
            }
            break;
      }
   }

   return(Done);
}

/* The following function processes terminal input.                  */
static void ProcessCharacters(void *UserParameter)
{
   /* Check to see if we have a command to process.                     */
   int ret_val = GetInput();
   if(ret_val > 0)
   {
      /* Attempt to process a character.                                */
      ProcessCommandLine(Input);
   }
   else if (COMMAND_LINE_ENTER_PRESSED == ret_val)
   {
       DisplayPrompt();
   }
}


static int FTPMode(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
       /* Next check to see if the parameters required for the        */
       /* execution of this function appear to be semi-valid.         */
       if((TempParam) && (TempParam->NumberofParameters > 0))
       {
          if(TempParam->Params->intParam)
             FTPModeActive = TRUE;
          else
             FTPModeActive = FALSE;
       }

       /* Output the current FTP Mode state.               */
       Display(("Current FTP Mode set to: %s.\r\n", FTPModeActive?"ACTIVE":"INACTIVE"));

       /* Flag that the function was successful.                      */
       ret_val = 0;
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}


static int FormatMMC(ParameterList_t *TempParam)
{
  FRESULT res;
  DWORD fre_clust, fre_sect, tot_sect;
  FATFS *fs;

  res = FormateMMC();
  if (res == FR_OK) {
    // Get volume information and free clusters of drive
    res = f_getfree(SD_Path0, &fre_clust, &fs);
    if (res == FR_OK) {
      // Get total sectors and free sectors 
      tot_sect = (fs->n_fatent - 2) * fs->csize;
      fre_sect = fre_clust * fs->csize;
      Display(("%10lu KiB total drive space.\r\n%10lu KiB available.\r\n",
             tot_sect / 2, fre_sect / 2));
      
    } else {
      Display(("Error get free: %d\r\n", res));
    }
  } else {
    Display(("Error formatting: %d\r\n", res));
  }

  return(res);
}

static int GetFreeMMC(ParameterList_t *TempParam)
{
  int ret_val = 1;
  uint8_t retSD;   
  DWORD fre_clust, fre_sect, tot_sect;
  FATFS *fs;

  /* Get volume information and free clusters of drive*/
  retSD = f_getfree(SD_Path0, &fre_clust, &fs);
  if (retSD == FR_OK) {
    /* Get total sectors and free sectors */
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    Display(("%10lu KiB total drive space.\r\n%10lu KiB available.\r\n",
           tot_sect / 2, fre_sect / 2));
    
    ret_val = 0;
  } else {
    Display(("Error get free: %d\r\n", retSD));
  }

  return(ret_val);
}

static int StartFirmwareUpdate(ParameterList_t *TempParam)
{
  int ret_val = 1;

  RamParam.BootCmd = CMD_UPDATE_FIRMWARE;
  FLASH_If_SaveParam();
  HAL_NVIC_SystemReset();

  return(ret_val);
}

static int GetRSSI(ParameterList_t *TempParam)
{
  int ret_val = 1;
  Word_t chresult;
  SByte_t rssi;
  Byte_t status;
  
  if(BluetoothStackID) {
    ret_val = HCI_Read_RSSI(BluetoothStackID, Connection_Handle, &status, &chresult, &rssi);
    if (!ret_val) {
      Display(("RSSI:%d\r\n", rssi));
    } else {
      Display(("error reading RSSI: %d\r\n", ret_val));
    }
  }

  return(ret_val);
}


#endif // CONSOLE_SUPPORT
#endif // FCC_TESTS