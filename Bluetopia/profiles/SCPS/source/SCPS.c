/*****< scps.c >***************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SCPS  - Bluetooth Stack Scan Parameters Service (GATT Based) for          */
/*          Stonestreet One Bluetooth Protocol Stack.                         */
/*                                                                            */
/*  Author:  Ajay Parashar                                                    */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/09/12  A. Parashar    Initial creation.                               */
/******************************************************************************/

#include "SS1BTPS.h"          /* Bluetooth Stack API Prototypes/Constants.    */
#include "SS1BTGAT.h"         /* Bluetooth Stack GATT API Prototypes/Constants*/
#include "SS1BTSCP.h"         /* Bluetooth SCPS API Prototypes/Constants.     */
#include "BTPSKRNL.h"         /* BTPS Kernel Prototypes/Constants.            */
#include "SCPS.h"             /* Bluetooth SCPS Prototypes/Constants.         */

 /* The following controls the number of supported SCPS instances.      */
#define SCPS_MAXIMUM_SUPPORTED_INSTANCES                 (BTPS_CONFIGURATION_SCPS_MAXIMUM_SUPPORTED_INSTANCES)

   /* SCPS Service Instance Block.  This structure contains All         */
   /* information associated with a specific Bluetooth Stack ID (member */
   /* is present in this structure).                                    */
typedef struct _tagSCPSServerInstance_t
{
   unsigned int          BluetoothStackID;
   unsigned int          ServiceID;
   SCPS_Event_Callback_t EventCallback;
   unsigned long         CallbackParameter;
} SCPSServerInstance_t;

#define SCPS_SERVER_INSTANCE_DATA_SIZE                   (sizeof(SCPSServerInstance_t))

   /*********************************************************************/
   /**               Scan Parameters Service Table                     **/
   /*********************************************************************/

   /* The Scan Parameters Service Declaration UUID.                     */
static BTPSCONST GATT_Primary_Service_16_Entry_t SCPS_Service_UUID =
{
   SCPS_SERVICE_BLUETOOTH_UUID_CONSTANT
};

   /* The Scan Interval Window Characteristic Declaration.              */
static BTPSCONST GATT_Characteristic_Declaration_16_Entry_t SCPS_Scan_Interval_Window_Declaration =
{
   GATT_CHARACTERISTIC_PROPERTIES_WRITE_WITHOUT_RESPONSE,
   SCPS_SCAN_INTERVAL_WINDOW_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT
};

   /* The Scan Interval Window Characteristic Value.                    */
static BTPSCONST GATT_Characteristic_Value_16_Entry_t  SCPS_Scan_Interval_Window_Value =
{
   SCPS_SCAN_INTERVAL_WINDOW_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT,
   0,
   NULL
};


#if BTPS_CONFIGURATION_SCPS_SUPPORT_SCAN_REFRESH

   /* The Scan Refresh  Characteristic Declaration.                     */
static BTPSCONST GATT_Characteristic_Declaration_16_Entry_t SCPS_Scan_Refresh_Declaration =
{
   GATT_CHARACTERISTIC_PROPERTIES_NOTIFY,
   SCPS_SCAN_REFRESH_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT
};

   /* The Scan Refresh Characteristic Value.                            */
static BTPSCONST GATT_Characteristic_Value_16_Entry_t  SCPS_Scan_Refresh_Value =
{
   SCPS_SCAN_REFRESH_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT,
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

#endif

   /* The following defines the Scan Parameters service that is         */
   /* registered with the GATT_Register_Service function call.          */
   /* * NOTE * This array will be registered with GATT in the call to   */
   /*          GATT_Register_Service.                                   */
BTPSCONST GATT_Service_Attribute_Entry_t Scan_Parameters_Service[] =
{
   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetPrimaryService16,            (Byte_t *)&SCPS_Service_UUID},
   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration16, (Byte_t *)&SCPS_Scan_Interval_Window_Declaration},
   {GATT_ATTRIBUTE_FLAGS_WRITABLE,          aetCharacteristicValue16,       (Byte_t *)&SCPS_Scan_Interval_Window_Value},

#if BTPS_CONFIGURATION_SCPS_SUPPORT_SCAN_REFRESH
   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration16, (Byte_t *)&SCPS_Scan_Refresh_Declaration},
   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicValue16,       (Byte_t *)&SCPS_Scan_Refresh_Value},

   {GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, aetCharacteristicDescriptor16,  (Byte_t *)&Client_Characteristic_Configuration},
#endif
};

#define SCAN_PARAMETERS_SERVICE_ATTRIBUTE_COUNT          (sizeof(Scan_Parameters_Service)/sizeof(GATT_Service_Attribute_Entry_t))

#define SCPS_SCAN_INTERVAL_WINDOW_VALUE_OFFSET              2

#if BTPS_CONFIGURATION_SCPS_SUPPORT_SCAN_REFRESH
   #define SCPS_SCAN_REFRESH_ATTRIBUTE_VALUE_OFFSET         4
   #define SCPS_SCAN_REFRESH_CCD_ATTRIBUTE_OFFSET           5
#endif

   /*********************************************************************/
   /**                    END OF SERVICE TABLE                         **/
   /*********************************************************************/

   /* The following type defines a union large enough to hold all events*/
   /* dispatched by this module.                                        */
typedef union
{
   SCPS_Write_Scan_Interval_Window_Data_t  SCPS_Write_Scan_Interval_Window_Data;
   SCPS_Client_Configuration_Update_Data_t Client_Configuration_Update_Data;
   SCPS_Read_Client_Configuration_Data_t   Read_Client_Data;
} SCPS_Event_Data_Buffer_t;

#define SCPS_EVENT_DATA_BUFFER_SIZE                      (sizeof(SCPS_Event_Data_Buffer_t))

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static SCPSServerInstance_t InstanceList[SCPS_MAXIMUM_SUPPORTED_INSTANCES];
                                            /* Variable which holds the */
                                            /* service instance data.   */

static Boolean_t InstanceListInitialized;   /* Variable that flags that */
                                            /* is used to denote that   */
                                            /* this module has been     */
                                            /* successfully initialized.*/

 /* The following are the prototypes of local functions.                */
static Boolean_t InitializeModule(void);
static void CleanupModule(void);
static Boolean_t InstanceRegisteredByStackID(unsigned int BluetoothStackID);
static SCPSServerInstance_t *AcquireServiceInstance(unsigned int BluetoothStackID, unsigned int *InstanceID);
static int DecodeClientConfigurationValue(unsigned int BufferLength, Byte_t *Buffer, Word_t *ClientConfiguration);

static SCPS_Event_Data_t *FormatEventHeader(unsigned int BufferLength, Byte_t *Buffer, SCPS_Event_Type_t EventType, unsigned int InstanceID, unsigned int ConnectionID, unsigned int *TransactionID, GATT_Connection_Type_t ConnectionType, BD_ADDR_t *BD_ADDR);

static int SCPSRegisterService(unsigned int BluetoothStackID, SCPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

   /* Bluetooth Event Callbacks.                                        */
static void BTPSAPI GATT_ServerEventCallback(unsigned int BluetoothStackID, GATT_Server_Event_Data_t *GATT_ServerEventData, unsigned long CallbackParameter);

   /* The following function is a utility function that is used to      */
   /* reduce the ifdef blocks that are needed to handle the difference  */
   /* between module initialization for Threaded and NonThreaded stacks.*/
static Boolean_t InitializeModule(void)
{
   /* All we need to do is flag that we are initialized.                */
   if(!InstanceListInitialized)
   {
      InstanceListInitialized = TRUE;

      BTPS_MemInitialize(InstanceList, 0, sizeof(InstanceList));
   }
   return(TRUE);
}

  /* The following function is a utility function that exists to        */
  /* perform stack specific (threaded versus nonthreaded) cleanup.      */
static void CleanupModule(void)
{
  /* Flag that we are no longer initialized.                            */
  InstanceListInitialized = FALSE;
}

   /* The following function is a utility function that exists to decode*/
   /* an Scan Interval Window value from remote client                  */
static int DecodeScanIntervalWindow(unsigned int BufferLength, Byte_t *Buffer, SCPS_Scan_Interval_Window_Data_t *ScanIntervalWindow)
{
   int ret_val = SCPS_ERROR_MALFORMATTED_DATA;

   /* Verify that the input parameters are valid.                       */
   if((BufferLength == SCPS_SCAN_INTERVAL_WINDOW_DATA_SIZE) && (Buffer) && (ScanIntervalWindow))
   {
      ScanIntervalWindow->LE_Scan_Interval   = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((SCPS_Scan_Interval_Window_t *)Buffer)->LE_Scan_Interval));
      ScanIntervalWindow->LE_Scan_Window     = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((SCPS_Scan_Interval_Window_t *)Buffer)->LE_Scan_Window));
      ret_val                                = 0;
   }
   else
   {
      if(BufferLength == SCPS_SCAN_INTERVAL_WINDOW_DATA_SIZE)
         ret_val = SCPS_ERROR_INVALID_PARAMETER;
   }

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that exists to format*/
   /* a SCPS Event into the specified buffer.                           */
   /* * NOTE * TransactionID is optional and may be set to NULL.        */
   /* * NOTE * BD_ADDR is NOT optional and may NOT be set to NULL.      */
static SCPS_Event_Data_t *FormatEventHeader(unsigned int BufferLength, Byte_t *Buffer, SCPS_Event_Type_t EventType, unsigned int InstanceID, unsigned int ConnectionID, unsigned int *TransactionID, GATT_Connection_Type_t ConnectionType, BD_ADDR_t *BD_ADDR)
{
   SCPS_Event_Data_t *EventData = NULL;

   if((BufferLength >= (SCPS_EVENT_DATA_SIZE + SCPS_EVENT_DATA_BUFFER_SIZE)) && (Buffer) && (BD_ADDR))
   {
      /* Format the header of the event, that is data that is common to */
      /* all events.                                                    */
      BTPS_MemInitialize(Buffer, 0, BufferLength);
      EventData                                                                      = (SCPS_Event_Data_t *)Buffer;
      EventData->Event_Data_Type                                                     = EventType;

      EventData->Event_Data.SCPS_Read_Client_Configuration_Data                      = (SCPS_Read_Client_Configuration_Data_t *)(((Byte_t *)EventData) + SCPS_EVENT_DATA_SIZE);

      EventData->Event_Data.SCPS_Read_Client_Configuration_Data->InstanceID          = InstanceID;
      EventData->Event_Data.SCPS_Read_Client_Configuration_Data->ConnectionID        = ConnectionID;

      if(TransactionID)
      {
         EventData->Event_Data.SCPS_Read_Client_Configuration_Data->TransactionID    = *TransactionID;
         EventData->Event_Data.SCPS_Read_Client_Configuration_Data->ConnectionType   = ConnectionType;
         EventData->Event_Data.SCPS_Read_Client_Configuration_Data->RemoteDevice     = *BD_ADDR;
      }
      else
      {
         EventData->Event_Data.SCPS_Client_Configuration_Update_Data->ConnectionType = ConnectionType;
         EventData->Event_Data.SCPS_Client_Configuration_Update_Data->RemoteDevice   = *BD_ADDR;
      }

   }

   /* Finally return the result to the caller.                          */
   return(EventData);
}

   /* The following function is a utility function that exists to check */
   /* to see if an instance has already been registered for a specified */
   /* Bluetooth Stack ID.                                               */
   /* * NOTE * Since this is an internal function no check is done on   */
   /*          the input parameters.                                    */
static Boolean_t InstanceRegisteredByStackID(unsigned int BluetoothStackID)
{
   Boolean_t    ret_val = FALSE;
   unsigned int Index;

   for(Index=0;Index<SCPS_MAXIMUM_SUPPORTED_INSTANCES;Index++)
   {
      if((InstanceList[Index].BluetoothStackID == BluetoothStackID) && (InstanceList[Index].ServiceID))
      {
         ret_val = TRUE;
         break;
      }
   }

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* acquire a specified service instance.                             */
   /* * NOTE * Since this is an internal function no check is done on   */
   /*          the input parameters.                                    */
   /* * NOTE * If InstanceID is set to 0, this function will return the */
   /*          next free instance.                                      */
static SCPSServerInstance_t *AcquireServiceInstance(unsigned int BluetoothStackID, unsigned int *InstanceID)
{
   unsigned int          LocalInstanceID;
   unsigned int          Index;
   SCPSServerInstance_t *ret_val = NULL;

   /* Lock the Bluetooth Stack to gain exclusive access to this         */
   /* Bluetooth Protocol Stack.                                         */
   if(!BSC_LockBluetoothStack(BluetoothStackID))
   {
      /* Acquire the BSC List Lock while we are searching the instance  */
      /* list.                                                          */
      if(BSC_AcquireListLock())
      {
         /* Store a copy of the passed in InstanceID locally.           */
         LocalInstanceID = *InstanceID;

         /* Verify that the Instance ID is valid.                       */
         if((LocalInstanceID) && (LocalInstanceID <= SCPS_MAXIMUM_SUPPORTED_INSTANCES))
         {
            /* Decrement the LocalInstanceID (to access the InstanceList*/
            /* which is 0 based).                                       */
            --LocalInstanceID;

            /* Verify that this Instance is registered and valid.       */
            if((InstanceList[LocalInstanceID].BluetoothStackID == BluetoothStackID) && (InstanceList[LocalInstanceID].ServiceID))
            {
               /* Return a pointer to this instance.                    */
               ret_val = &InstanceList[LocalInstanceID];
            }
         }
         else
         {
            /* Verify that we have been requested to find the next free */
            /* instance.                                                */
            if(!LocalInstanceID)
            {
               /* Try to find a free instance.                          */
               for(Index=0;Index<SCPS_MAXIMUM_SUPPORTED_INSTANCES;Index++)
               {
                  /* Check to see if this instance is being used.       */
                  if(!(InstanceList[Index].ServiceID))
                  {
                     /* Return the InstanceID AND a pointer to the      */
                     /* instance.                                       */
                     *InstanceID = Index+1;
                     ret_val     = &InstanceList[Index];
                     break;
                  }
               }
            }
         }

         /* Release the previously acquired list lock.                  */
         BSC_ReleaseListLock();
      }

      /* If we failed to acquire the instance then we should un-lock the*/
      /* previously acquired Bluetooth Stack.                           */
      if(!ret_val)
         BSC_UnLockBluetoothStack(BluetoothStackID);
   }

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that exists to decode*/
   /* an Client Configuration value into a user specified boolean value.*/
   /* This function returns the zero if successful or a negative error  */
   /* code.                                                             */
static int DecodeClientConfigurationValue(unsigned int BufferLength, Byte_t *Buffer, Word_t *ClientConfiguration)
{
   int ret_val = SCPS_ERROR_MALFORMATTED_DATA;

   /* Verify that the input parameters are valid.                       */
   if(((BufferLength == NON_ALIGNED_BYTE_SIZE) || (BufferLength == GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_LENGTH)) && (Buffer) && (ClientConfiguration))
   {
      /* Read the requested Client Configuration.                       */
      if(BufferLength == NON_ALIGNED_BYTE_SIZE)
         *ClientConfiguration = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(Buffer);
      else
         *ClientConfiguration = READ_UNALIGNED_WORD_LITTLE_ENDIAN(Buffer);

      ret_val              = 0;
   }
   else
   {
      if(BufferLength == GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_LENGTH)
         ret_val = SCPS_ERROR_INVALID_PARAMETER;
   }

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function which is used to     */
   /* register an SCPS Service.  This function returns the positive,    */
   /* non-zero, Instance ID on success or a negative error code.        */
static int SCPSRegisterService(unsigned int BluetoothStackID, SCPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   int                   ret_val;
   unsigned int          InstanceID;
   SCPSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (EventCallback) && (ServiceID))
   {
      /* Verify that no instance is registered to this Bluetooth Stack. */
      if(!InstanceRegisteredByStackID(BluetoothStackID))
      {
         /* Acquire a free SCPS Instance.                               */
         InstanceID = 0;
         if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
         {
            /* Call GATT to register the SCPS service.                  */
            ret_val = GATT_Register_Service(BluetoothStackID, SCPS_SERVICE_FLAGS, SCAN_PARAMETERS_SERVICE_ATTRIBUTE_COUNT, (GATT_Service_Attribute_Entry_t *)Scan_Parameters_Service, ServiceHandleRange, GATT_ServerEventCallback, InstanceID);
            if(ret_val > 0)
            {
               /* Save the Instance information.                        */
               ServiceInstance->BluetoothStackID  = BluetoothStackID;
               ServiceInstance->ServiceID         = (unsigned int)ret_val;
               ServiceInstance->EventCallback     = EventCallback;
               ServiceInstance->CallbackParameter = CallbackParameter;
               *ServiceID                         = (unsigned int)ret_val;

               /* Return the SCPS Instance ID.                          */
               ret_val                            = (int)InstanceID;
            }
            /* UnLock the previously locked Bluetooth Stack.            */
            BSC_UnLockBluetoothStack(BluetoothStackID);
         }
         else
            ret_val = SCPS_ERROR_INSUFFICIENT_RESOURCES;
      }
      else
         ret_val = SCPS_ERROR_SERVICE_ALREADY_REGISTERED;
   }
   else
      ret_val = SCPS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is the GATT Server Event Callback that     */
   /* handles all requests made to the SCPS Service for all registered  */
   /* instances.                                                        */
static void BTPSAPI GATT_ServerEventCallback(unsigned int BluetoothStackID, GATT_Server_Event_Data_t *GATT_ServerEventData, unsigned long CallbackParameter)
{
   Word_t                AttributeOffset;
   Word_t                ValueLength;
   Byte_t               *Value;
   Byte_t                Event_Buffer[SCPS_EVENT_DATA_SIZE + SCPS_EVENT_DATA_BUFFER_SIZE];
   unsigned int          TransactionID;
   unsigned int          InstanceID;
   SCPS_Event_Data_t    *EventData;
   SCPSServerInstance_t *ServiceInstance;

    /* Verify that all parameters to this callback are Semi-Valid.      */
   if((BluetoothStackID) && (GATT_ServerEventData) && (CallbackParameter))
   {
      /* The Instance ID is always registered as the callback parameter.*/
      InstanceID = (unsigned int)CallbackParameter;
      /* Acquire the Service Instance for the specified service.        */
      if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
      {
         switch(GATT_ServerEventData->Event_Data_Type)
         {

#if BTPS_CONFIGURATION_SCPS_SUPPORT_SCAN_REFRESH

            case etGATT_Server_Read_Request:
               /* Verify that the Event Data is valid.                  */
               if(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data)
               {
                  AttributeOffset = GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset;
                  TransactionID   = GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID;

                  /* Verify that they are not trying to write with an   */
                  /* offset or using preprared writes.                  */
                  if(!(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeValueOffset))
                  {
                     /* Verify that the read is for a CCCD (which is    */
                     /* only readable attribute in table).              */
                     if(Scan_Parameters_Service[AttributeOffset].Attribute_Entry_Type == aetCharacteristicDescriptor16)
                     {
                        /* Format the event header.                     */
                        EventData = FormatEventHeader(sizeof(Event_Buffer), Event_Buffer, etSCPS_Server_Read_Client_Configuration_Request, InstanceID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ConnectionID, &TransactionID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ConnectionType, &(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->RemoteDevice));
                        if(EventData)
                        {
                           /* Determine the Correct Client Configuration*/
                           /* Type.                                     */
                           if(AttributeOffset == SCPS_SCAN_REFRESH_CCD_ATTRIBUTE_OFFSET)
                           {
                              /* Format the reast of the event.         */
                              EventData->Event_Data_Size                                                         = SCPS_READ_CLIENT_CONFIGURATION_DATA_SIZE;
                              EventData->Event_Data.SCPS_Read_Client_Configuration_Data->ClientConfigurationType = ctScanRefresh;

                              /* Dispatch the event.                    */
                              __BTPSTRY
                              {
                                 (*ServiceInstance->EventCallback)(ServiceInstance->BluetoothStackID, EventData, ServiceInstance->CallbackParameter);
                              }
                              __BTPSEXCEPT(1)
                              {
                                 /* Do Nothing.                         */
                              }
                           }
                           else
                              GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                        }
                        else
                           GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                     }
                  }
                  else
                     GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
               }
               break;

#endif
              case etGATT_Server_Write_Request:
               /* Verify that the Event Data is valid.                  */
               if(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data)
               {
                  AttributeOffset = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeOffset;
                  TransactionID   = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->TransactionID;
                  ValueLength     = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueLength;
                  Value           = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValue;
                  if((!(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->AttributeValueOffset)) && (!(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->DelayWrite)))
                  {

#if BTPS_CONFIGURATION_SCPS_SUPPORT_SCAN_REFRESH

                     if(Scan_Parameters_Service[AttributeOffset].Attribute_Entry_Type == aetCharacteristicDescriptor16)
                     {
                        /* Format the event header.                     */
                        EventData = FormatEventHeader(sizeof(Event_Buffer), Event_Buffer, etSCPS_Server_Update_Client_Configuration_Request, InstanceID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->ConnectionID, NULL, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->ConnectionType, &(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->RemoteDevice));
                        if(EventData)
                        {
                           /* Verify that this is a write to the Scan   */
                           /* Refresh CCCD.                             */
                           if(AttributeOffset == SCPS_SCAN_REFRESH_CCD_ATTRIBUTE_OFFSET)
                           {
                              /* Format the rest of the event.          */
                              EventData->Event_Data_Size                                                           = SCPS_CLIENT_CONFIGURATION_UPDATE_DATA_SIZE;
                              EventData->Event_Data.SCPS_Client_Configuration_Update_Data->ClientConfigurationType = ctScanRefresh;

                              /* Attempt to decode the client           */
                              /* configuration value.                   */
                              if(!DecodeClientConfigurationValue(ValueLength, Value, &(EventData->Event_Data.SCPS_Client_Configuration_Update_Data->ClientConfiguration)))
                              {
                                 /* Go ahead and accept the write       */
                                 /* request since we have decoded the   */
                                 /* Client Configuration Value          */
                                 /* successfully.                       */
                                 GATT_Write_Response(BluetoothStackID, TransactionID);

                                 /* Dispatch the event.                 */
                                 __BTPSTRY
                                 {
                                    (*ServiceInstance->EventCallback)(ServiceInstance->BluetoothStackID, EventData, ServiceInstance->CallbackParameter);
                                 }
                                 __BTPSEXCEPT(1)
                                 {
                                    /* Do Nothing.                      */
                                 }
                              }
                              else
                                 GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                           }
                           else
                              GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                        }
                        else
                           GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                     }
                     else

#endif
                     {
                        /* Format and Dispatch the event.               */
                        EventData = FormatEventHeader(sizeof(Event_Buffer), Event_Buffer, etSCPS_Server_Write_Scan_Interval_Window_Request, InstanceID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->ConnectionID, NULL, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->ConnectionType, &(GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->RemoteDevice));
                        if(EventData)
                        {
                           /* Format the reast of the event.            */
                           EventData->Event_Data_Size = SCPS_WRITE_SCAN_INTERVAL_WINDOW_DATA_SIZE;

                           /* Parse out the command.                    */
                           if(!DecodeScanIntervalWindow(ValueLength, Value, &(EventData->Event_Data.SCPS_Write_Scan_Interval_Window_Data->ScanIntervalWindowData)))
                           {
                              /* Go ahead and accept the write request  */
                              /* since the SCP Service specifies that   */
                              /* the Write Without Response procedure   */
                              /* must be used.  Therefore we can just   */
                              /* call into GATT with the write response */
                              /* to free the resources for the Write    */
                              /* without Response procedure.            */
                              GATT_Write_Response(BluetoothStackID, TransactionID);

                              /* Dispatch the event.                    */
                              __BTPSTRY
                              {
                                 (*ServiceInstance->EventCallback)(ServiceInstance->BluetoothStackID, EventData, ServiceInstance->CallbackParameter);
                              }
                              __BTPSEXCEPT(1)
                              {
                                 /* Do Nothing.                         */
                              }
                           }
                           else
                              GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                        }
                        else
                           GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                     }
                  }
                  else
                     GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED);
               }
               break;
            default:
               /* Do nothing, as this is just here to get rid of        */
               /* warnings that some compilers flag when not all cases  */
               /* are handled in a switch off of a enumerated value.    */
               break;
         }

         /* UnLock the previously locked Bluetooth Stack.               */
         BSC_UnLockBluetoothStack(ServiceInstance->BluetoothStackID);
      }
   }
}

   /* The following function is responsible for making sure that the    */
   /* Bluetooth Stack SCPS Module is Initialized correctly.  This       */
   /* function *MUST* be called before ANY other Bluetooth Stack SCPS   */
   /* function can be called.  This function returns non-zero if the    */
   /* Module was initialized correctly, or a zero value if there was an */
   /* error.                                                            */
   /* * NOTE * Internally, this module will make sure that this function*/
   /*          has been called at least once so that the module will    */
   /*          function.  Calling this function from an external        */
   /*          location is not necessary.                               */
int InitializeSCPSModule(void)
{
   return((int)InitializeModule());
}

   /* The following function is responsible for instructing the         */
   /* Bluetooth Stack SCPS Module to clean up any resources that it has */
   /* allocated. Once this function has completed, NO other Bluetooth   */
   /* Stack SCPS Functions can be called until a successful call to the */
   /* InitializeSCPSModule() function is made.  The parameter to this   */
   /* function specifies the context in which this function is being    */
   /* called.  If the specified parameter is TRUE, then the module will */
   /* make sure that NO functions that would require waiting/blocking on*/
   /* Mutexes/Events are called.  This parameter would be set to TRUE if*/
   /* this function was called in a context where threads would not be  */
   /* allowed to run.  If this function is called in the context where  */
   /* threads are allowed to run then this parameter should be set to   */
   /* FALSE.                                                            */
void CleanupSCPSModule(Boolean_t ForceCleanup)
{
   /* Check to make sure that this module has been initialized.         */
   if(InstanceListInitialized)
   {
      /* Wait for access to the SCPS Context List.                      */
      if((ForceCleanup) || ((!ForceCleanup) && (BSC_AcquireListLock())))
      {
         /* Cleanup the Instance List.                                  */
         BTPS_MemInitialize(InstanceList, 0, sizeof(InstanceList));

         if(!ForceCleanup)
            BSC_ReleaseListLock();
      }
      /* Cleanup the module.                                            */
      CleanupModule();
   }
}

   /* The following function is responsible for opening a SCPS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered SCPs       */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 SCPS Server may be open at a time, per Bluetooth  */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
int BTPSAPI SCPS_Initialize_Service(unsigned int BluetoothStackID, SCPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID)
{
   GATT_Attribute_Handle_Group_t ServiceHandleRange;

    /* Initialize the Service Handle Group to 0.                        */
   ServiceHandleRange.Starting_Handle = 0;
   ServiceHandleRange.Ending_Handle   = 0;

   return(SCPSRegisterService(BluetoothStackID, EventCallback, CallbackParameter, ServiceID, &ServiceHandleRange));
}

   /* The following function is responsible for opening a SCPS Server.  */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The fourth parameter is a     */
   /* pointer to store the GATT Service ID of the registered SCPS       */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 SCPS Server may be open at a time, per Bluetooth  */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
int BTPSAPI SCPS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, SCPS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   return(SCPSRegisterService(BluetoothStackID, EventCallback, CallbackParameter, ServiceID, ServiceHandleRange));
}

   /* The following function is responsible for closing a previously    */
   /* opened SCPS Server.  The first parameter is the Bluetooth Stack ID*/
   /* on which to close the server.  The second parameter is the        */
   /* InstanceID that was returned from a successful call to            */
   /* SCPS_Initialize_Service().  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
int BTPSAPI SCPS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID)
{
   int                   ret_val;
   SCPSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (InstanceID))
   {
      /* Acquire the specified SCPS Instance.                           */
      if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
      {
         /* Verify that the service is actually registered.             */
         if(ServiceInstance->ServiceID)
         {
            /* Call GATT to un-register the service.                    */
            GATT_Un_Register_Service(BluetoothStackID, ServiceInstance->ServiceID);

            /* mark the instance entry as being free.                   */
            BTPS_MemInitialize(ServiceInstance, 0, SCPS_SERVER_INSTANCE_DATA_SIZE);

            /* return success to the caller.                            */
            ret_val = 0;
         }
         else
            ret_val = SCPS_ERROR_INVALID_PARAMETER;

         /* UnLock the previously locked Bluetooth Stack.               */
         BSC_UnLockBluetoothStack(BluetoothStackID);
      }
      else
         ret_val = SCPS_ERROR_INVALID_INSTANCE_ID;
   }
   else
      ret_val = SCPS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the SCPS Service that is         */
   /* registered with a call to SCPS_Initialize_Service().  This        */
   /* function returns the non-zero number of attributes that are       */
   /* contained in a SCPS Server or zero on failure.                    */
unsigned int BTPSAPI SCPS_Query_Number_Attributes(void)
{
   /* Simply return the number of attributes that are contained in a    */
   /* SCPS service.                                                     */
   return(SCAN_PARAMETERS_SERVICE_ATTRIBUTE_COUNT);
}

   /* The following function is responsible for responding to a SCPS    */
   /* Read Client Configuration Request.  The first parameter is the    */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* SCPS_Initialize_Server().  The third is the Transaction ID of the */
   /* request.  The final parameter contains the Client Configuration to*/
   /* send to the remote device.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
int BTPSAPI SCPS_Read_Client_Configuration_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t Client_Configuration)
{
#if BTPS_CONFIGURATION_SCPS_SUPPORT_SCAN_REFRESH

   int                   ret_val;
   Word_t                ValueLength;
   NonAlignedWord_t      ClientConfiguration;
   SCPSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (InstanceID) && (TransactionID))
   {
      /* Acquire the specified SCPS Instance.                           */
      if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
      {
         /* Format the Read Response.                                   */
         ValueLength = GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_LENGTH;
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&ClientConfiguration, Client_Configuration);

         /* Send the response.                                          */
         ret_val = GATT_Read_Response(ServiceInstance->BluetoothStackID, TransactionID, (unsigned int)ValueLength, (Byte_t *)&ClientConfiguration);

         /* UnLock the previously locked Bluetooth Stack.               */
         BSC_UnLockBluetoothStack(ServiceInstance->BluetoothStackID);
      }
      else
         ret_val = SCPS_ERROR_INVALID_INSTANCE_ID;
   }
   else
      ret_val = SCPS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);

#else

     return(BTPS_ERROR_FEATURE_NOT_AVAILABLE);

#endif
}

   /* The following function is responsible for sending an Scan         */
   /* Refresh notification to a specified remote device.  The first     */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to SCPS_Initialize_Server().  The third parameter is the          */
   /* ConnectionID of the remote device to send the notification to.    */
   /* This function returns a zero if successful or a negative          */
   /* return error code if an error occurs.                             */
int BTPSAPI SCPS_Notify_Scan_Refresh(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, Byte_t ScanRefreshValue)
{
#if BTPS_CONFIGURATION_SCPS_SUPPORT_SCAN_REFRESH

   int                   ret_val;
   SCPSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (InstanceID) && (ConnectionID))
   {
      /* Acquire the specified SCPS Instance.                           */
      if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
      {
         /* Attempt to send the notification.                           */
         ret_val = GATT_Handle_Value_Notification(ServiceInstance->BluetoothStackID, ServiceInstance->ServiceID, ConnectionID, SCPS_SCAN_REFRESH_ATTRIBUTE_VALUE_OFFSET, NON_ALIGNED_BYTE_SIZE, (Byte_t *)&ScanRefreshValue);
         if(ret_val > 0)
            ret_val = 0;

         /* UnLock the previously locked Bluetooth Stack.               */
         BSC_UnLockBluetoothStack(BluetoothStackID);
      }
      else
         ret_val = SCPS_ERROR_INVALID_INSTANCE_ID;
   }
   else
      ret_val = SCPS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);

#else

   return(BTPS_ERROR_FEATURE_NOT_AVAILABLE);

#endif
}

   /* The following function is responsible for formatting a Scan       */
   /* Interval Window Command into a user specified buffer. The         */
   /* first parameter is the SCPS_Scan_Interval_Window_Data_t to format.*/
   /* The final two parameters contain the length of the buffer,        */
   /* and the buffer, to  format the command into.                      */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * The BufferLength and Buffer parameter must point to a    */
   /*          buffer of at least                                       */
   /*          SCPS_SCAN_INTERVAL_WINDOW_SIZE in size.                  */
int BTPSAPI SCPS_Format_Scan_Interval_Window(SCPS_Scan_Interval_Window_Data_t *Scan_Interval_Window, unsigned int BufferLength, Byte_t *Buffer)
{
   int ret_val;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BufferLength >= SCPS_SCAN_INTERVAL_WINDOW_DATA_SIZE) && (Buffer))
   {
      /* Assign the command into the user specified buffer.             */

      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((SCPS_Scan_Interval_Window_t *)Buffer)->LE_Scan_Interval), (Scan_Interval_Window->LE_Scan_Interval));
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((SCPS_Scan_Interval_Window_t *)Buffer)->LE_Scan_Window), (Scan_Interval_Window->LE_Scan_Window));

      /* Return success to the caller.                                  */
      ret_val = 0;
   }
   else
      ret_val = SCPS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}
