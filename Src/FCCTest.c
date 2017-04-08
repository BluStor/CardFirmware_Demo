/* fcctest.c
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
/*                                                                            */
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
#include "FCCTest.h"             /* Application Header.                       */

#ifdef FCC_TESTS        // Compiled only for FCC TESTS

extern int HandleASuccessfulRead(char *lpBuf, DWORD dwRead );
extern int FCCTestEnable;
extern int FCCTestNumber;

#ifdef CONSOLE_SUPPORT
extern UART_HandleTypeDef UartHandle;
#endif

#define MAX_SUPPORTED_COMMANDS                     (36)  /* Denotes the       */
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

#define MAX_SUPPORTED_LINK_KEYS                    (1)   /* Max supported Link*/
                                                         /* keys.             */

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

#define TEST_DATA              "This is a test string."  /* Denotes the data  */
                                                         /* that is sent via  */
                                                         /* SPP when calling  */
                                                         /* SPP_Data_Write(). */

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

   /* Determine the Name we will use for this compilation.              */
#define APP_DEMO_NAME                              "BLUSTOR"


   /* The following represent the possible values of UI_Mode variable.  */
#define UI_MODE_IS_PRODTOOLS   (3)
#define UI_MODE_IS_CLIENT      (2)
#define UI_MODE_IS_SERVER      (1)
#define UI_MODE_SELECT         (0)
#define UI_MODE_IS_INVALID     (-1)

   /* Following converts a Sniff Parameter in Milliseconds to frames.   */
#define MILLISECONDS_TO_BASEBAND_SLOTS(_x)   ((_x) / (0.625))

   /* The following type definition represents the container type which */
   /* holds the mapping between Bluetooth devices (based on the BD_ADDR)*/
   /* and the Link Key (BD_ADDR <-> Link Key Mapping).                  */
typedef struct _tagLinkKeyInfo_t
{
   BD_ADDR_t  BD_ADDR;
   Link_Key_t LinkKey;
} LinkKeyInfo_t;

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
   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

int                        UI_Mode;                 /* Holds the UI Mode.              */

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


static LinkKeyInfo_t       LinkKeyInfo[MAX_SUPPORTED_LINK_KEYS]; /* Variable holds     */
                                                    /* BD_ADDR <-> Link Keys for       */
                                                    /* pairing.                        */

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

   /* Variables which contain information used by the loopback          */
   /* functionality of this test application.                           */
static unsigned int        BufferLength;

static unsigned char       Buffer[530];

static Boolean_t           BufferFull;

static Boolean_t           FTPModeActive = TRUE;



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

static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization);
static int CloseStack(void);

static int SetDisc(void);
static int SetConnect(void);
static int SetPairable(void);
static int DeleteLinkKey(BD_ADDR_t BD_ADDR);

static int PINCodeResponse(ParameterList_t *TempParam);

static int OpenServer(ParameterList_t *TempParam);

int SaveKey(LinkKeyInfo_t * pKeyInfo);
int RestoreKey(LinkKeyInfo_t * pKeyInfo);
int DeleteKeys(void);

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
static int TestNFaceMatch(ParameterList_t *TempParam);
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
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI SPP_Event_Callback(unsigned int BluetoothStackID, SPP_Event_Data_t *SPP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI HCI_Event_Callback(unsigned int BluetoothStackID, HCI_Event_Data_t *HCI_Event_Data, unsigned long CallbackParameter);
  
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
      AddCommand("DISPLAYRAWMODEDATA", DisplayRawModeData);
      AddCommand("AUTOMATICREADMODE", AutomaticReadMode);
      AddCommand("SETBAUDRATE", SetBaudRate);
      AddCommand("SEND", SendData);
      AddCommand("HELP", DisplayHelp);    
      AddCommand("QUERYMEMORY", QueryMemory);
      AddCommand("FTPMODE", FTPMode);
      AddCommand("GETRSSI", GetRSSI);
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
      AddCommand("NFACEMATCH", TestNFaceMatch);
      
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
        Display(("FW Version    : %d.%d \r\n", ((FW_Version_Details.LMP_SubversionResult >> 7) & 0x07) + ((FW_Version_Details.LMP_SubversionResult >> 12) & 0x08), FW_Version_Details.LMP_SubversionResult & 0x7F));
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

   /* The following function is responsible for opening the SS1         */
   /* Bluetooth Protocol Stack.  This function accepts a pre-populated  */
   /* HCI Driver Information structure that contains the HCI Driver     */
   /* Transport Information.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization)
{
   int                        Result;
   int                        ret_val = 0;
   L2CA_Link_Connect_Params_t L2CA_Link_Connect_Params;

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

         printf("\r\nOpenStack().\r\n");
         
         SelfTest.SerialBT = SELF_TEST_STACK_INIT_BT;
         /* Initialize the Stack                                        */
         Result = BSC_Initialize(HCI_DriverInformation, 0);

         /* Next, check the return value of the initialization to see if*/
         /* it was successful.                                          */
         if(Result > 0)
         {
#if 1 // db??? test RF power level
            static Byte_t Status;
            static SByte_t TxPower = 0;
            BluetoothStackID = Result;
            //ret_val = HCI_Read_Inquiry_Response_Transmit_Power_Level(BluetoothStackID, &Status, &TxPower);
            
            ret_val = VS_Set_Max_Output_Power(BluetoothStackID, TxPower);
            printf("VS_Set_Max_Output_Power= %d ret: %d\r\n", TxPower, ret_val);
            BTPS_Delay(200);
            //ret_val = HCI_Read_Inquiry_Response_Transmit_Power_Level(BluetoothStackID, &Status, &TxPower);
#endif           
           
            /* The Stack was initialized successfully, inform the user  */
            /* and set the return value of the initialization function  */
            /* to the Bluetooth Stack ID.                               */
            BluetoothStackID = Result;
            printf("Bluetooth Stack ID: %d\r\n", BluetoothStackID);

            /* Initialize the default Secure Simple Pairing parameters. */
            IOCapability     = DEFAULT_IO_CAPABILITY;
            OOBSupport       = FALSE;
            MITMProtection   = DEFAULT_MITM_PROTECTION;

#ifdef CONSOLE_SUPPORT
            {
              char BluetoothAddress[16];
              HCI_Version_t HCIVersion;
              BD_ADDR_t BD_ADDR;
              if(!HCI_Version_Supported(BluetoothStackID, &HCIVersion)) {
                Display(("Device Chipset: %s\r\n", (HCIVersion <= NUM_SUPPORTED_HCI_VERSIONS)?HCIVersionStrings[HCIVersion]:HCIVersionStrings[NUM_SUPPORTED_HCI_VERSIONS]));
              }

              /* Printing the BTPS version                                */
              Display(("BTPS Version  : %s \r\n", BTPS_VERSION_VERSION_STRING));
              /* Printing the FW version                                  */
              DisplayFWVersion();

              /* Printing the Demo Application version                    */
              Display(("App Name      : %s \r\n", APP_DEMO_NAME));
              Display(("App Version   : %s \r\n", DEMO_APPLICATION_VERSION_STRING));            
              
              /* Let's output the Bluetooth Device Address so that the    */
              /* user knows what the Device Address is.                   */
              if(!GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR))
              {
                 BD_ADDRToStr(BD_ADDR, BluetoothAddress);

                 Display(("LOCAL BD_ADDR: %s\r\n", BluetoothAddress));
              }
            }
            /* Set the Name for the initial use.              */
            GAP_Set_Local_Device_Name (BluetoothStackID,APP_DEMO_NAME);

            /* Go ahead and allow Master/Slave Role Switch.             */
            L2CA_Link_Connect_Params.L2CA_Link_Connect_Request_Config  = cqAllowRoleSwitch;
            L2CA_Link_Connect_Params.L2CA_Link_Connect_Response_Config = csMaintainCurrentRole;

            L2CA_Set_Link_Connection_Configuration(BluetoothStackID, &L2CA_Link_Connect_Params);

            if(HCI_Command_Supported(BluetoothStackID, HCI_SUPPORTED_COMMAND_WRITE_DEFAULT_LINK_POLICY_BIT_NUMBER) > 0)
               HCI_Write_Default_Link_Policy_Settings(BluetoothStackID, (HCI_LINK_POLICY_SETTINGS_ENABLE_MASTER_SLAVE_SWITCH|HCI_LINK_POLICY_SETTINGS_ENABLE_SNIFF_MODE), &Status);

#endif // CONSOLE_SUPPORT            

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

      printf("Stack Shutdown.\r\n");

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
static int SetDisc(void)
{
   int ret_val = 0;
   
   GAP_Discoverability_Mode_t disc_mode;
   
   if (BTPaired) {
     disc_mode = dmNonDiscoverableMode;
   } else {
     disc_mode = dmGeneralDiscoverableMode;
   }

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* A semi-valid Bluetooth Stack ID exists, now attempt to set the */
      /* attached Devices Discoverablity Mode to General.               */
      ret_val = GAP_Set_Discoverability_Mode(BluetoothStackID, disc_mode, 0);

      /* Next, check the return value of the GAP Set Discoverability    */
      /* Mode command for successful execution.                         */
      if(ret_val)
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

   return(ret_val);
}

   /* The following function is responsible for placing the Local       */
   /* Bluetooth Device into Connectable Mode.  Once in this mode the    */
   /* Device will respond to Page Scans from other Bluetooth Devices.   */
   /* This function requires that a valid Bluetooth Stack ID exists     */
   /* before running.  This function returns zero on success and a      */
   /* negative value if an error occurred.                              */
static int SetConnect(void)
{
   int ret_val = 0;
   
   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to set the attached Device to be Connectable.          */
      ret_val = GAP_Set_Connectability_Mode(BluetoothStackID, cmConnectableMode);

      /* Next, check the return value of the                            */
      /* GAP_Set_Connectability_Mode() function for successful          */
      /* execution.                                                     */
      if(ret_val)
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
static int SetPairable(void)
{
   int Result;
   int ret_val = 0;
   GAP_Pairability_Mode_t pair_mode;
   
   if (BTPaired) {
     pair_mode = pmNonPairableMode;
   } else {
     pair_mode = pmPairableMode;
   }

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to set the attached device to be pairable.             */
      Result = GAP_Set_Pairability_Mode(BluetoothStackID, pair_mode);

      /* Next, check the return value of the GAP Set Pairability mode   */
      /* command for successful execution.                              */
      if(!Result)
      {
         /* The device has been set to pairable mode, now register an   */
         /* Authentication Callback to handle the Authentication events */
         /* if required.                                                */
         Result = GAP_Register_Remote_Authentication(BluetoothStackID, GAP_Event_Callback, (unsigned long)0);

         /* Next, check the return value of the GAP Register Remote     */
         /* Authentication command for successful execution.            */
         if(Result)
         {
            /* An error occurred while trying to execute this function. */
#ifdef CONSOLE_SUPPORT
            DisplayFunctionError("Auth", Result);
#endif
            ret_val = Result;
         }
      }
      else
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

   return(ret_val);
}

   /* The following function is a utility function that exists to delete*/
   /* the specified Link Key from the Local Bluetooth Device.  If a NULL*/
   /* Bluetooth Device Address is specified, then all Link Keys will be */
   /* deleted.                                                          */
static int DeleteLinkKey(BD_ADDR_t BD_ADDR)
{
   int       Result;
   Byte_t    Status_Result;
   Word_t    Num_Keys_Deleted = 0;
   BD_ADDR_t NULL_BD_ADDR;

   Result = HCI_Delete_Stored_Link_Key(BluetoothStackID, BD_ADDR, TRUE, &Status_Result, &Num_Keys_Deleted);

   /* Any stored link keys for the specified address (or all) have been */
   /* deleted from the chip.  Now, let's make sure that our stored Link */
   /* Key Array is in sync with these changes.                          */

   /* First check to see all Link Keys were deleted.                    */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   if(COMPARE_BD_ADDR(BD_ADDR, NULL_BD_ADDR))
      BTPS_MemInitialize(LinkKeyInfo, 0, sizeof(LinkKeyInfo));
   else
   {
      /* Individual Link Key.  Go ahead and see if know about the entry */
      /* in the list.                                                   */
      for(Result=0;(Result<sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t));Result++)
      {
         if(COMPARE_BD_ADDR(BD_ADDR, LinkKeyInfo[Result].BD_ADDR))
         {
            LinkKeyInfo[Result].BD_ADDR = NULL_BD_ADDR;

            break;
         }
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
       Display(("* Command Options: Format, Free, FirmwareUpdate, NFaceMatch,     *\r\n"));
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
            Display(("Discoverability: %s.\r\n", (DiscoverabilityMode == dmNonDiscoverableMode)?"Non":((DiscoverabilityMode == dmGeneralDiscoverableMode)?"General":"Limited")));

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
            Display(("Connectability Mode: %s.\r\n", (ConnectableMode == cmNonConnectableMode)?"Non Connectable":"Connectable"));

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
           Display(("open: %d\r\n", TempParam->Params[0].intParam));
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

                     ret_val      = UNABLE_TO_REGISTER_SERVER;
                  }
                  else
                  {
                     /* Simply flag to the user that everything         */
                     /* initialized correctly.                          */
                     Display(("Server Opened: %d.\r\n", TempParam->Params[0].intParam));

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
               Display(("Unable to Open Server on: %d, Error = %d.\r\n", TempParam->Params[0].intParam, ret_val));

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

         Display(("Server Closed.\r\n"));
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
#endif // CONSOLE_SUPPORT


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
   BD_ADDR_t                         NULL_BD_ADDR;
   Boolean_t                         OOB_Data;
   Boolean_t                         MITM;
   GAP_IO_Capability_t               RemoteIOCapability;
   GAP_Inquiry_Event_Data_t         *GAP_Inquiry_Event_Data;
   GAP_Remote_Name_Event_Data_t     *GAP_Remote_Name_Event_Data;
   GAP_Authentication_Information_t  GAP_Authentication_Information;

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
                  for(Index=0;Index<(sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t));Index++)
                  {
                     if(COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device))
                     {
                        /* Link Key information stored, go ahead and    */
                        /* respond with the stored Link Key.            */
                        GAP_Authentication_Information.Authentication_Data_Length   = sizeof(Link_Key_t);
                        GAP_Authentication_Information.Authentication_Data.Link_Key = LinkKeyInfo[Index].LinkKey;

                        break;
                     }
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
                  Display(("\r\n"));
                  Display(("atPINCodeRequest: %s\r\n", Callback_BoardStr));
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
                  Display(("\r\n"));
                  Display(("atLinkKeyCreation: %s\r\n", Callback_BoardStr));
#endif // CONSOLE_SUPPORT

                  /* Now store the link Key in either a free location OR*/
                  /* over the old key location.                         */
                  ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

                  for(Index=0,Result=-1;Index<(sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t));Index++)
                  {
                     if(COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device))
                        break;
                     else
                     {
                        if((Result == (-1)) && (COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, NULL_BD_ADDR)))
                           Result = Index;
                     }
                  }

                  /* If we didn't find a match, see if we found an empty*/
                  /* location.                                          */
                  if(Index == (sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t)))
                     Index = Result;

                  /* Check to see if we found a location to store the   */
                  /* Link Key information into.                         */
                  if(Index != (-1))
                  {
                     LinkKeyInfo[Index].BD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;
                     LinkKeyInfo[Index].LinkKey = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Link_Key_Info.Link_Key;

                     Display(("Link Key Stored.\r\n"));
                     SaveKey(LinkKeyInfo);
                     // Now that card is paired, set it no more discoverable or pairable
                     SetDisc();
                     SetPairable();
                  }
                  else
                     Display(("Link Key array full.\r\n"));
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

            Display(("\r\n"));
            Display(("SPP Open Indication, ID: 0x%04X, Board: %s.\r\n", SPP_Event_Data->Event_Data.SPP_Open_Port_Indication_Data->SerialPortID, Callback_BoardStr));
#endif // CONSOLE_SUPPORT

            /* Save the Serial Port ID for later use.                   */
            SerialPortID = SPP_Event_Data->Event_Data.SPP_Open_Port_Indication_Data->SerialPortID;

            /* Flag that we are now connected.                          */
            Connected  = TRUE;

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
            }
            else
            {
               /* Flag that we are now connected.                       */
               Connected  = TRUE;

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
            Display(("\r\n"));
            Display(("SPP Close Port, ID: 0x%04X\r\n", SPP_Event_Data->Event_Data.SPP_Close_Port_Indication_Data->SerialPortID));
#endif // CONSOLE_SUPPORT

            /* Invalidate the Serial Port ID.                           */
            if(UI_Mode == UI_MODE_IS_CLIENT)
               SerialPortID = 0;

            Connection_Handle = 0;
            SendInfo.BytesToSend = 0;

            /* Flag that we are no longer connected.                    */
            Connected         = FALSE;
            // Lock FTP server
            FTPLocked = TRUE;
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
                      HandleASuccessfulRead((char *)Buffer,TempLength);
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

   /* The following function is responsible for processing HCI Mode     */
   /* change events.                                                    */
static void BTPSAPI HCI_Event_Callback(unsigned int BluetoothStackID, HCI_Event_Data_t *HCI_Event_Data, unsigned long CallbackParameter)
{
#ifdef CONSOLE_SUPPORT           
   char *Mode;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (HCI_Event_Data))
   {
      /* Process the Event Data.                                        */
      switch(HCI_Event_Data->Event_Data_Type)
      {
         case etMode_Change_Event:
            if(HCI_Event_Data->Event_Data.HCI_Mode_Change_Event_Data)
            {
               switch(HCI_Event_Data->Event_Data.HCI_Mode_Change_Event_Data->Current_Mode)
               {
                  case HCI_CURRENT_MODE_HOLD_MODE:
                     Mode = "Hold";
                     break;
                  case HCI_CURRENT_MODE_SNIFF_MODE:
                     Mode = "Sniff";
                     break;
                  case HCI_CURRENT_MODE_PARK_MODE:
                     Mode = "Park";
                     break;
                  case HCI_CURRENT_MODE_ACTIVE_MODE:
                  default:
                     Mode = "Active";
                     break;
               }

               Display(("\r\n"));
               Display(("HCI Mode Change Event, Status: 0x%02X, Connection Handle: %d, Mode: %s, Interval: %d\r\n", HCI_Event_Data->Event_Data.HCI_Mode_Change_Event_Data->Status,
                                                                                                                    HCI_Event_Data->Event_Data.HCI_Mode_Change_Event_Data->Connection_Handle,
                                                                                                                    Mode,
                                                                                                                    HCI_Event_Data->Event_Data.HCI_Mode_Change_Event_Data->Interval));
               DisplayPrompt();
            }
            break;
         default:
            break;
      }
   }
#endif // CONSOLE_SUPPORT           
}

typedef struct
{       /* result of int divide */
  VS_Modulation_Type_t VS_Modulation_Type;
  Byte_t Test_Pattern;
  Byte_t Frequency_Channel;
  Byte_t Power_Level;
  DWord_t Generator_Init_Value;
  DWord_t EDR_Generator_Mask;
} FCC_Param_t;


FCC_Param_t FCC_Param_TM1 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtGFSK,             VS_TEST_PATTERN_F0F0,   0,                      15,             0,                      0,
};

FCC_Param_t FCC_Param_TM2 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtGFSK,             VS_TEST_PATTERN_F0F0,   59,                     15,             0,                      0,
};

FCC_Param_t FCC_Param_TM3 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtGFSK,             VS_TEST_PATTERN_F0F0,   39,                     15,             0,                      0,
};

FCC_Param_t FCC_Param_TM4 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtEDR3,             VS_TEST_PATTERN_F0F0,   0,                      15,             0,                      0,
};

FCC_Param_t FCC_Param_TM5 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtEDR3,             VS_TEST_PATTERN_F0F0,   59,                     15,             0,                      0,
};

FCC_Param_t FCC_Param_TM6 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtEDR3,             VS_TEST_PATTERN_F0F0,   39,                     15,             0,                      0,
};

FCC_Param_t FCC_Param_TM7 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtEDR2,             VS_TEST_PATTERN_F0F0,   0,                      15,             0,                      0,
};

FCC_Param_t FCC_Param_TM8 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtEDR2,             VS_TEST_PATTERN_F0F0,   59,                     15,             0,                      0,
};

FCC_Param_t FCC_Param_TM9 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtEDR2,             VS_TEST_PATTERN_F0F0,   39,                     15,             0,                      0,
};

FCC_Param_t FCC_Param_TM14 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtContinuousWave,   VS_TEST_PATTERN_ALL_ONE, 59,                     15,             0,                      0,
};

FCC_Param_t FCC_Param_TM21 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtBLE,              VS_TEST_PATTERN_F0F0,   0,                      15,             0,                      0,
};

FCC_Param_t FCC_Param_TM22 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtBLE,              VS_TEST_PATTERN_F0F0,   59,                     15,             0,                      0,
};

FCC_Param_t FCC_Param_TM23 = {
// VS_Modulation_Type   Test_Pattern            Frequency_Channel       Power_Level     Generator_Init_Value    EDR_Generator_Mask
    mtBLE,              VS_TEST_PATTERN_F0F0,   39,                     15,             0,                      0,
};


typedef struct
{
  Byte_t Frequency_Mode;        // 0x00 - Hopping, 0x03 - Single frequency
  Byte_t Tx_Single_Frequency;   // 00 - 78 (Freq = 2402 + 2k, for k = 0, 1, 2 … 39 Freq = 2403 + 2(k–40), for k = 40, 41…78)
  Byte_t Rx_Single_Frequency;   // 00 - 78 (See above) 0xff - Disable RX (packet TX only)
  Byte_t ACL_Packet_Type;       // 0x00 - DM1, 0x01 - DH1, 0x02 - DM3, 0x03 - DH3, 0x04 - DM5, 0x05 - DH5, 0x06 - 2-DH1, 0x07 - 2-DH3, 0x08 - 2-DH5, 0x09 - 3-DH1, 0x0a - 3-DH3, 0x0b - 3-DH5 
  Byte_t ACL_Packet_Pattern;    // 0x00 - All 0, 0x01 - All 1, 0x02 - 5555, 0x03 - F0F0, 0x04 - Ordered, 0x05 PRBS9 random
  Word_t ACL_Packet_Len;        // 0-17 - DM1, 0-27 - DH1, 0-121 - DM3, 0-183 - DH3, 0-224 - DM5, 0-339 - DH5, 0-54 - 2-DH1, 0-367 - 2-DH3, 0-679 - 2-DH5, 0-83 - 3-DH1, 0-552 - 3-DH3, 0-1021 - 3-DH5
  Byte_t Power_Level;           // 0-15, 15= Max Output Power, 0 = Min Output Power
  Byte_t Dis_Whitening;         // 0 - Enable whitening, 1 - Disable whitening
  Word_t PRBS9_Val;             // 0x0000-0x01ff - Used only in PRBS9 patterns to initialize PRBS9 data
} TX_RX_Param_t;

TX_RX_Param_t TX_RX_Param_TM11 = {
// Frequency_Mode       Tx_Single_Frequency     Rx_Single_Frequency     ACL_Packet_Type ACL_Packet_Pattern      ACL_Packet_Len  Power_Level     Dis_Whitening          PRBS9_Val
    0,                  0,                      0xff,                   0x01,           0x03,                   27,             15,             1,                      0x1ff,
};

TX_RX_Param_t TX_RX_Param_TM12 = {
// Frequency_Mode       Tx_Single_Frequency     Rx_Single_Frequency     ACL_Packet_Type ACL_Packet_Pattern      ACL_Packet_Len  Power_Level     Dis_Whitening          PRBS9_Val
    0,                  0,                      0xff,                   0x03,           0x03,                   183,            15,             1,                      0x1ff,
};

TX_RX_Param_t TX_RX_Param_TM13 = {
// Frequency_Mode       Tx_Single_Frequency     Rx_Single_Frequency     ACL_Packet_Type ACL_Packet_Pattern      ACL_Packet_Len  Power_Level     Dis_Whitening          PRBS9_Val
0,                  0,                      0xff,                   0x05,           0x03,                   339,            15,             1,                      0x1ff,
};

int StartRFTest(void)
{
  int ret_val = APPLICATION_ERROR_INVALID_PARAMETERS;
  FCC_Param_t * pFCC_Param;
  TX_RX_Param_t * pTX_RX_Param;
  
  switch (FCCTestNumber) {
    case 1: 
      pFCC_Param = &FCC_Param_TM1;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 2:
      pFCC_Param = &FCC_Param_TM2;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 3:
      pFCC_Param = &FCC_Param_TM3;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 4:
      pFCC_Param = &FCC_Param_TM4;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 5:
      pFCC_Param = &FCC_Param_TM5;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 6:
      pFCC_Param = &FCC_Param_TM6;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 7:
      pFCC_Param = &FCC_Param_TM7;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 8:
      pFCC_Param = &FCC_Param_TM8;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 9:
      pFCC_Param = &FCC_Param_TM9;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 11:
      pTX_RX_Param = &TX_RX_Param_TM11;
      ret_val = VS_DRPb_Tester_Packet_Tx_Rx(BluetoothStackID,
                                       pTX_RX_Param->Frequency_Mode,
                                       pTX_RX_Param->Tx_Single_Frequency, 
                                       pTX_RX_Param->Rx_Single_Frequency,
                                       pTX_RX_Param->ACL_Packet_Type,
                                       pTX_RX_Param->ACL_Packet_Pattern,
                                       pTX_RX_Param->ACL_Packet_Len,
                                       pTX_RX_Param->Power_Level,
                                       pTX_RX_Param->Dis_Whitening,
                                       pTX_RX_Param->PRBS9_Val);
      break;
    case 12:
      pTX_RX_Param = &TX_RX_Param_TM12;
      ret_val = VS_DRPb_Tester_Packet_Tx_Rx(BluetoothStackID,
                                       pTX_RX_Param->Frequency_Mode,
                                       pTX_RX_Param->Tx_Single_Frequency, 
                                       pTX_RX_Param->Rx_Single_Frequency,
                                       pTX_RX_Param->ACL_Packet_Type,
                                       pTX_RX_Param->ACL_Packet_Pattern,
                                       pTX_RX_Param->ACL_Packet_Len,
                                       pTX_RX_Param->Power_Level,
                                       pTX_RX_Param->Dis_Whitening,
                                       pTX_RX_Param->PRBS9_Val);
      break;
    case 13:
      pTX_RX_Param = &TX_RX_Param_TM13;
      ret_val = VS_DRPb_Tester_Packet_Tx_Rx(BluetoothStackID,
                                       pTX_RX_Param->Frequency_Mode,
                                       pTX_RX_Param->Tx_Single_Frequency, 
                                       pTX_RX_Param->Rx_Single_Frequency,
                                       pTX_RX_Param->ACL_Packet_Type,
                                       pTX_RX_Param->ACL_Packet_Pattern,
                                       pTX_RX_Param->ACL_Packet_Len,
                                       pTX_RX_Param->Power_Level,
                                       pTX_RX_Param->Dis_Whitening,
                                       pTX_RX_Param->PRBS9_Val);
      break;
    case 14:
      pFCC_Param = &FCC_Param_TM14;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 15:
      ret_val = VS_DRPb_Tester_Con_Rx(BluetoothStackID, 0);
      break;
    case 16:
      ret_val = VS_DRPb_Tester_Con_Rx(BluetoothStackID, 59);
      break;
    case 17:
      ret_val = VS_DRPb_Tester_Con_Rx(BluetoothStackID, 39);
      break;
    case 21:
      pFCC_Param = &FCC_Param_TM21;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 22:
      pFCC_Param = &FCC_Param_TM22;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    case 23:
      pFCC_Param = &FCC_Param_TM23;
      ret_val = VS_Enable_FCC_Test_Mode(BluetoothStackID,
                                         pFCC_Param->VS_Modulation_Type,
                                         pFCC_Param->Test_Pattern, 
                                         pFCC_Param->Frequency_Channel,
                                         pFCC_Param->Power_Level,
                                         pFCC_Param->Generator_Init_Value,
                                         pFCC_Param->EDR_Generator_Mask);
      break;
    default:
      Display(("StartRFTest - Bad test number:%d\r\n", FCCTestNumber));
  }
  
  return (ret_val);
}

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
   UI_Mode                = UI_MODE_SELECT;
   LoopbackActive         = FALSE;
   DisplayRawData         = FALSE;
   AutomaticReadActive    = FALSE;
   NumberofValidResponses = 0;

   /* Next, makes sure that the Driver Information passed appears to be */
   /* semi-valid.                                                       */
   if((HCI_DriverInformation) && (BTPS_Initialization))
   {
      /* Try to Open the stack and check if it was successful.          */
      if(!OpenStack(HCI_DriverInformation, BTPS_Initialization))
      {
         ret_val = StartRFTest();
         printf("StartRFTest: %d\r\n", ret_val);

         
#ifdef CONSOLE_SUPPORT
         /* Set up the Selection Interface.                 */
         UserInterface_Selection();

        /* Display the first command prompt.               */
         DisplayPrompt();
#endif // CONSOLE_SUPPORT
         /* Return success to the caller.                   */
         ret_val = (int)BluetoothStackID;
      }
      else
      {
         /* There was an error while attempting to open the Stack.      */
         printf("Unable to open the stack.\r\n");
      }
   }
   else
      ret_val = APPLICATION_ERROR_INVALID_PARAMETERS;

   return(ret_val);
}


int RestoreKey(LinkKeyInfo_t * pKeyInfo)
{
  FIL fp;
  FRESULT res;
  UINT read;

  res = f_open(&fp, BT_KEY_FILENAME, FA_READ | FA_OPEN_EXISTING);
  if (res == FR_OK) {
    res = f_read(&fp, (void *) pKeyInfo, sizeof(LinkKeyInfo_t), &read);
    f_close(&fp);
    if (read != sizeof(LinkKeyInfo_t)) {
      res = FR_DISK_ERR;
    } else {
      BTPaired = TRUE;
    }
  }
   
  return (res);
}

int DeleteKeys(void)
{
  FRESULT res;
  BD_ADDR_t BD_ADDR;

  res = f_unlink(BT_KEY_FILENAME);
  /* Delete all Stored Link Keys.                             */
  ASSIGN_BD_ADDR(BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  DeleteLinkKey(BD_ADDR);
  
  return (res);
}

int BT_WriteABuffer(const char * lpBuf, DWORD dwToWrite){
   int  Result;
   int  ret_val = 0;
   char * ptr = (char *)lpBuf;

   if((BluetoothStackID) && (SerialPortID))
   {
      while(dwToWrite) {
        Result = SPP_Data_Write(BluetoothStackID, SerialPortID, (Word_t)dwToWrite, (Byte_t *)ptr);
        if (Result < 0) {
          ret_val = Result;
          break;
        }
        dwToWrite -= Result;
        ptr += Result;
        if (dwToWrite) {
          //printf("tow: %d/%d\r\n", dwToWrite, Result);
          BTPS_Delay(10);
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

int SaveKey(LinkKeyInfo_t * pKeyInfo)
{
  FIL fp;
  FRESULT res;
  UINT written;
  
  res = f_open(&fp, BT_KEY_FILENAME, FA_WRITE | FA_CREATE_ALWAYS);
  if(res == FR_OK){
    res = f_write(&fp, (void *) pKeyInfo, sizeof(LinkKeyInfo_t), &written);
    f_close(&fp);
    if (written != sizeof(LinkKeyInfo_t)) {
      res = FR_DISK_ERR;
    } else {
      BTPaired = TRUE;
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

   /* The following function is the main user interface thread.  It     */
   /* opens the Bluetooth Stack and then drives the main user interface.*/
void FCCTestsThread(void const *argument)
{

   int                           Result;
   BTPS_Initialization_t         BTPS_Initialization;
   HCI_DriverInformation_t       HCI_DriverInformation;

#ifdef CONSOLE_SUPPORT
   HAL_ConfigureConsole(&UartHandle);
   /* Set up the application callbacks.                                 */
   BTPS_Initialization.MessageOutputCallback = DisplayCallback;
#else   
   /* Set up the application callbacks.                                 */
   BTPS_Initialization.MessageOutputCallback = NULL; 
#endif // CONSOLE_SUPPORT
   
   /* Configure the UART Parameters.                                    */
   HCI_DRIVER_SET_COMM_INFORMATION(&HCI_DriverInformation, 1, VENDOR_BAUD_RATE, cpHCILL_RTS_CTS);
   HCI_DriverInformation.DriverInformation.COMMDriverInformation.InitializationDelay = 100;


  /* Loop forever and process UART characters.                      */
  while(1)
  {
#ifdef CONSOLE_SUPPORT           
     ProcessCharacters(NULL);
#endif // CONSOLE_SUPPORT           
     {
       static int FCCEnableTrk = 0;
     
      if (FCCTestEnable != FCCEnableTrk) {
         // FCCTestEnable has changed
         FCCEnableTrk = FCCTestEnable;
         if (FCCTestEnable) {
           printf("FCCTestNumber: %d\r\n", FCCTestNumber);
           Result = InitializeApplication(&HCI_DriverInformation, &BTPS_Initialization);
           printf("InitializeApplication: %d\r\n", Result);
         } else {
           Result = CloseStack();
           printf("CloseStack: %d\r\n", Result);
         }
       }
     }

     BTPS_Delay(200);
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
#include "templates.h"
static int TestNFaceMatch(ParameterList_t *TempParam)
{
  int ret_val = 1;
  enum NStatusCodes ret;
  
  
  ret = NFaceMatch(tmpl_1, tmpl_1_len, tmpl_2, tmpl_2_len, 50);
  printf("Call NFaceMatch(tmpl_1, tmpl_1_len, tmpl_2, tmpl_2_len, 50)...");
  switch(ret)
  {
  case NST_ERROR:
    printf("NST_ERROR\r\n");
    break;
  case NST_MISMATCH:
    printf("NST_MISMATCH\r\n");
    break;
  case NST_MATCH:
    printf("NST_MATCH\r\n");
    break;
  }
    
  ret = NFaceMatch(tmpl_bad, tmpl_bad_len, tmpl_2, tmpl_2_len, 50);
  printf("Call NFaceMatch(tmpl_bad, tmpl_bad_len, tmpl_2, tmpl_2_len, 50)...");
  switch(ret)
  {
  case NST_ERROR:
    printf("NST_ERROR\r\n");
    break;
  case NST_MISMATCH:
    printf("NST_MISMATCH\r\n");
    break;
  case NST_MATCH:
    printf("NST_MATCH\r\n");
    break;
  }
  
  ret_val=0;
    
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