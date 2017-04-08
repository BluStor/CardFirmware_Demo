/*****< gls.c >****************************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Riggs Reserved.                                                   */
/*                                                                            */
/*  GLS - Bluetooth Stack Glucose Service (GATT Based) for Stonestreet One    */
/*        Bluetooth Protocol Stack.                                           */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/22/11  T. Thomas      Initial creation.                               */
/*   07/16/12  Z. Khan        Updated as per new spec.                        */
/******************************************************************************/
#include "SS1BTPS.h"        /* Bluetooth Stack API Prototypes/Constants.      */
#include "SS1BTGAT.h"       /* Bluetooth Stack GATT API Prototypes/Constants. */
#include "SS1BTGLS.h"       /* Bluetooth GLS API Prototypes/Constants.        */
#include "BTPSKRNL.h"       /* BTPS Kernel Prototypes/Constants.              */
#include "GLS.h"            /* Bluetooth GLS Prototypes/Constants.            */

   /* The following controls the number of supported GLS instances.     */
#define GLS_MAXIMUM_SUPPORTED_INSTANCES                     (BTPS_CONFIGURATION_GLS_MAXIMUM_SUPPORTED_INSTANCES)

#define VALID_OP_CODE(_x)                                   (((_x) >= GLS_RECORD_ACCESS_OPCODE_REPORT_STORED_RECORDS) && ((_x) <= GLS_RECORD_ACCESS_OPCODE_RESPONSE_CODE))

#define NOT_A_NUMBER                                        0x07FF
#define NOT_AT_THIS_RESOLUTION                              0x0800
#define POSITIVE_INFINITY                                   0x07FE
#define NEGATIVE_INFINITY                                   0x0802

   /* The following correspond to the current Reserved for Future Use   */
   /* bits in OpCode field of Record Access Control Point characteristic*/
#define GLS_RECORD_ACCESS_OPCODE_RFU_0                       0x00
#define GLS_RECORD_ACCESS_OPCODE_RFU_7                       0x07
#define GLS_RECORD_ACCESS_OPCODE_RFU_255                     0xFF

   /* The following defines the GLS Instance Data, that contains data   */
   /* is unique for each GLS Service Instance.                          */
typedef __PACKED_STRUCT_BEGIN__ struct _tagGLS_Instance_Data_t
{
   NonAlignedWord_t                Feature_Data_Length;
   NonAlignedWord_t                Feature_Data;
} __PACKED_STRUCT_END__ GLS_Instance_Data_t;

#define GLS_INSTANCE_DATA_SIZE                   (sizeof(GLS_Instance_Data_t))

   /* The following define the instance tags for each GLS service data  */
   /* that is unique per registered service.                            */
#define GLS_FEATURE_DATA_INSTANCE_TAG            (BTPS_STRUCTURE_OFFSET(GLS_Instance_Data_t, Feature_Data_Length))

   /*********************************************************************/
   /**               Glucose Monitor Service Table                     **/
   /*********************************************************************/

   /* The Health Thermometer Service Declaration UUID.                  */
static BTPSCONST GATT_Primary_Service_16_Entry_t GLS_Service_UUID =
{
   GLS_SERVICE_BLUETOOTH_UUID_CONSTANT
};

   /* The Glucose Measurement Characteristic Declaration.               */
static BTPSCONST GATT_Characteristic_Declaration_16_Entry_t GLS_Measurement_Declaration =
{
   GATT_CHARACTERISTIC_PROPERTIES_NOTIFY,
   GLS_MEASUREMENT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT
};

   /* The Glucose Measurement Characteristic Value.                     */
static BTPSCONST GATT_Characteristic_Value_16_Entry_t  GLS_Measurement_Value =
{
   GLS_MEASUREMENT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT,
   0,
   NULL
};

#if BTPS_CONFIGURATION_GLS_SUPPORT_MEASUREMENT_CONTEXT

   /* The Measurement Context Characteristic Declaration.               */
static BTPSCONST GATT_Characteristic_Declaration_16_Entry_t GLS_Measurement_Context_Declaration =
{
   GATT_CHARACTERISTIC_PROPERTIES_NOTIFY,
   GLS_MEASUREMENT_CONTEXT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT
};

   /* The Measurement Context Characteristic Value.                     */
static BTPSCONST GATT_Characteristic_Value_16_Entry_t  GLS_Measurement_Context_Value =
{
   GLS_MEASUREMENT_CONTEXT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT,
   0,
   NULL
};

#endif

   /* The Glucose Feature Characteristic Declaration.                   */
static BTPSCONST GATT_Characteristic_Declaration_16_Entry_t GLS_Feature_Declaration =
{
   GATT_CHARACTERISTIC_PROPERTIES_READ,
   GLS_FEATURE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT
};

   /* The Glucose Feature Characteristic Value.                         */
static BTPSCONST GATT_Characteristic_Value_16_Entry_t  GLS_Feature_Value =
{
   GLS_FEATURE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT,
   GLS_FEATURE_DATA_INSTANCE_TAG,
   NULL
};

   /* The Record Access Control Point Characteristic Declaration.       */
static BTPSCONST GATT_Characteristic_Declaration_16_Entry_t GLS_Record_Access_Control_Point_Declaration =
{
   (GATT_CHARACTERISTIC_PROPERTIES_WRITE |GATT_CHARACTERISTIC_PROPERTIES_INDICATE),
   GLS_RECORD_ACCESS_CONTROL_POINT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT
};

   /* The Record Access Control Point Characteristic Value.             */
static BTPSCONST GATT_Characteristic_Value_16_Entry_t  GLS_Record_Access_Control_Point_Value =
{
   GLS_RECORD_ACCESS_CONTROL_POINT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT,
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

   /* The following defines the Glucose Service that is registered with */
   /* the GATT_Register_Service function call.                          */
   /* * NOTE * This array will be registered with GATT in the call to   */
   /*          GATT_Register_Service.                                   */
BTPSCONST GATT_Service_Attribute_Entry_t Glucose_Monitor_Service[] =
{
   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetPrimaryService16,            (Byte_t *)&GLS_Service_UUID},

   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration16, (Byte_t *)&GLS_Measurement_Declaration},
   {0,                                      aetCharacteristicValue16,       (Byte_t *)&GLS_Measurement_Value},
   {GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, aetCharacteristicDescriptor16,  (Byte_t *)&Client_Characteristic_Configuration},

   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration16, (Byte_t *)&GLS_Feature_Declaration},
   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicValue16,       (Byte_t *)&GLS_Feature_Value},

   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration16, (Byte_t *)&GLS_Record_Access_Control_Point_Declaration},
   {GATT_ATTRIBUTE_FLAGS_WRITABLE,          aetCharacteristicValue16,       (Byte_t *)&GLS_Record_Access_Control_Point_Value},
   {GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, aetCharacteristicDescriptor16,  (Byte_t *)&Client_Characteristic_Configuration},

#if BTPS_CONFIGURATION_GLS_SUPPORT_MEASUREMENT_CONTEXT

   {GATT_ATTRIBUTE_FLAGS_READABLE,          aetCharacteristicDeclaration16, (Byte_t *)&GLS_Measurement_Context_Declaration},
   {0,                                      aetCharacteristicValue16,       (Byte_t *)&GLS_Measurement_Context_Value},
   {GATT_ATTRIBUTE_FLAGS_READABLE_WRITABLE, aetCharacteristicDescriptor16,  (Byte_t *)&Client_Characteristic_Configuration},

#endif
};

#if BTPS_CONFIGURATION_GLS_SUPPORT_MEASUREMENT_CONTEXT
  #define ATTRIBUTE_OFFSET 3
#else
  #define ATTRIBUTE_OFFSET 0
#endif

#define GLS_MONITOR_SERVICE_ATTRIBUTE_COUNT                      (sizeof(Glucose_Monitor_Service)/sizeof(GATT_Service_Attribute_Entry_t))

#define GLS_MEASUREMENT_ATTRIBUTE_OFFSET                                2
#define GLS_MEASUREMENT_CCD_ATTRIBUTE_OFFSET                            3
#define GLS_FEATURE_ATTRIBUTE_OFFSET                                    5
#define GLS_RECORDS_ACCESS_CONTROL_POINT_ATTRIBUTE_OFFSET               7
#define GLS_RECORDS_ACCESS_CONTROL_POINT_CCD_ATTRIBUTE_OFFSET           8
#define GLS_MEASUREMENT_CONTEXT_ATTRIBUTE_OFFSET                       10
#define GLS_MEASUREMENT_CONTEXT_CCD_ATTRIBUTE_OFFSET                   11

   /*********************************************************************/
   /**                    END OF SERVICE TABLE                         **/
   /*********************************************************************/

   /* The following MACRO is a utility MACRO that exists to determine   */
   /* if the Request in Record_Access_Control_Point_Request_Data_t is   */
   /* valid                                                             */
#define GLS_RACP_COMMAND_TYPE_VALID(_x)         ((((Byte_t)(_x)) >= GLS_RECORD_ACCESS_OPCODE_REPORT_STORED_RECORDS) && (((Byte_t)(_x)) <= GLS_RECORD_ACCESS_OPCODE_REPORT_NUM_STORED_RECORDS))

   /* The following MACRO is a utility MACRO that exists to determine   */
   /* if the Filter in Record_Access_Control_Point_Request_Data_t is    */
   /* valid.                                                            */
#define GLS_RACP_OPERATOR_TYPE_VALID(_x)        ((((Byte_t)(_x)) >= GLS_RECORD_ACCESS_OPERATOR_NULL) && (((Byte_t)(_x)) <= GLS_RECORD_ACCESS_OPERATOR_LAST_RECORD))

   /* The following MACRO is a utility MACRO that exists to determine   */
   /* if the FilterType in Record_Access_Control_Point_Request_Data_t is*/
   /* valid.                                                            */
#define GLS_RACP_FILTER_TYPE_VALID(_x)          ((((Byte_t)(_x)) >= GLS_RECORD_ACCESS_FILTER_TYPE_SEQUENCE_NUMBER) && (((Byte_t)(_x)) <= GLS_RECORD_ACCESS_FILTER_TYPE_USER_FACING_TIME))

   /* The following type defines a union large enough to hold all events*/
   /* dispatched by this module.                                        */
typedef union
{
   GLS_Read_Client_Configuration_Data_t          Read_Client_Configuration_Data;
   GLS_Client_Configuration_Update_Data_t        Client_Configuration_Update_Data;
   GLS_Record_Access_Control_Point_Command_Data_t Record_Access_Control_Procedure_Request;
   GLS_Confirmation_Data_t                       Confirmation_Data;
} GLS_Event_Data_Buffer_t;

#define GLS_EVENT_DATA_BUFFER_SIZE                      (sizeof(GLS_Event_Data_Buffer_t))

   /* The following structure represents the information that will be   */
   /* stored during an outstanding RACP indication.                     */
typedef struct _tagRACPIndicatonInfo_t_t
{
   unsigned int ConnectionID;
   int          TransactionID;
} RACPIndicatonInfo_t;

   /* GLS Service Instance Block.  This structure contains All          */
   /* information associated with a specific Bluetooth Stack ID (member)*/
   /* is present in this structure).                                    */
typedef struct _tagGLSServerInstance_t
{
   unsigned int         BluetoothStackID;
   unsigned int         ServiceID;
   GLS_Event_Callback_t EventCallback;
   Byte_t               EventBuffer[GLS_EVENT_DATA_SIZE + GLS_EVENT_DATA_BUFFER_SIZE];
   unsigned long        CallbackParameter;
   Byte_t               PacketBuffer[ATT_PROTOCOL_MTU_MINIMUM_LE];
   RACPIndicatonInfo_t  RACPIndicationInfo;
} GLSServerInstance_t;

#define GLS_SERVER_INSTANCE_DATA_SIZE                    (sizeof(GLSServerInstance_t))

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

static GLS_Instance_Data_t InstanceData[GLS_MAXIMUM_SUPPORTED_INSTANCES];
                                            /* Variable which holds all */
                                            /* data that is unique for  */
                                            /* each service instance.   */

static GLSServerInstance_t InstanceList[GLS_MAXIMUM_SUPPORTED_INSTANCES];
                                            /* Variable which holds the */
                                            /* service instance data.   */

static Boolean_t           InstanceListInitialized;
                                             /* Variable that flags that*/
                                             /* is used to denote that  */
                                             /* this module has been    */
                                             /* successfully            */
                                             /* initialized.            */


   /* The following are the prototypes of local functions.              */
static Boolean_t InitializeModule(void);
static void CleanupModule(void);

static int FormatGlucoseMeasurement(GLS_Glucose_Measurement_Data_t *Measurement, unsigned int BufferLength, Byte_t *Buffer);
static int FormatGlucoseMeasurementContext(GLS_Glucose_Measurement_Context_Data_t *Context, unsigned int BufferLength, Byte_t *Buffer);

static int DecodeGlucoseMeasurement(unsigned int BufferLength, Byte_t *Value, GLS_Glucose_Measurement_Data_t *Measurement);
static int DecodeGlucoseMeasurementContext(unsigned int BufferLength, Byte_t *Value, GLS_Glucose_Measurement_Context_Data_t *Context);
static int DecodeRACPCommand(unsigned int BufferLength, Byte_t *Buffer, GLS_Record_Access_Control_Point_Format_Data_t *AccessControl);
static int DecodeRACPResponse(unsigned int ValueLength, Byte_t *Value, GLS_Record_Access_Control_Point_Response_Data_t *RACPData);

static GLS_Event_Data_t *FormatEventHeader(unsigned int BufferLength, Byte_t *Buffer, GLS_Event_Type_t EventType, unsigned int InstanceID, unsigned int ConnectionID, unsigned int TransactionID);

static Boolean_t InstanceRegisteredByStackID(unsigned int BluetoothStackID);
static GLSServerInstance_t *AcquireServiceInstance(unsigned int BluetoothStackID, unsigned int *InstanceID);
static int DecodeClientConfigurationValue(unsigned int BufferLength, Byte_t *Buffer, Word_t *ClientConfiguration);

static int GLSRegisterService(unsigned int BluetoothStackID, GLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

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

   /* The following function is a utility function that exists to       */
   /* perform stack specific (threaded versus nonthreaded) cleanup.     */
static void CleanupModule(void)
{
   /* Flag that we are no longer initialized.                           */
   InstanceListInitialized = FALSE;
}

   /* The following function is used to decode a glucose measurement    */
   /* that has been received from a remote Client.                      */
static int DecodeGlucoseMeasurement(unsigned int BufferLength, Byte_t *Value, GLS_Glucose_Measurement_Data_t *Measurement)
{
   int    ret_val;
   Byte_t TempByte;

   /* Verify that the input parameters appear semi-valid.               */
   if((BufferLength) && (Value) && (Measurement))
   {
      /* Verify that the Glucose Measurement is a valid length.         */
      if(BufferLength >= ((unsigned int)GLS_MEASUREMENT_HEADER_SIZE(0)))
      {
         /* Read the Flags from the Glucose Measurement.                */
         Measurement->OptionFlags = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Measurement_Header_t *)Value)->Flags));

         /* Initialize the return value to success.                     */
         ret_val = 0;

         /* Extract the Sequence Number and Base Time for the stream.   */
         Measurement->SequenceNumber   = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Measurement_Header_t *)Value)->Sequence_Number));
         Measurement->BaseTime.Year    = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Measurement_Header_t *)Value)->Base_Time.Year));
         Measurement->BaseTime.Month   = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Measurement_Header_t *)Value)->Base_Time.Month));
         Measurement->BaseTime.Day     = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Measurement_Header_t *)Value)->Base_Time.Day));
         Measurement->BaseTime.Hours   = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Measurement_Header_t *)Value)->Base_Time.Hours));
         Measurement->BaseTime.Minutes = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Measurement_Header_t *)Value)->Base_Time.Minutes));
         Measurement->BaseTime.Seconds = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Measurement_Header_t *)Value)->Base_Time.Seconds));

         /* Advance the pointer to the next field.                      */
         Value        += GLS_MEASUREMENT_HEADER_SIZE(0);
         BufferLength -= GLS_MEASUREMENT_HEADER_SIZE(0);

         /* Check to see if a Time Offset is present.                   */
         if(Measurement->OptionFlags & GLS_MEASUREMENT_FLAGS_TIME_OFFSET_PRESENT)
         {
            /* Verify that there is enough data to hold the offset      */
            /* value.                                                   */
            if(BufferLength >= GLS_TIME_OFFSET_SIZE)
            {
               /* Extract the Time Offset Value and adjust the buffer   */
               /* information.                                          */
               Measurement->TimeOffset = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Time_Offset_t *)Value)->Time_Offset));

               /* Advance the pointer to the next field.                */
               Value        += GLS_TIME_OFFSET_SIZE;
               BufferLength -= GLS_TIME_OFFSET_SIZE;
            }
            else
               ret_val = GLS_ERROR_MALFORMATTED_DATA;
         }

         /* Check to see if Glucose Concentration information is        */
         /* present.                                                    */
         if((!ret_val) && (Measurement->OptionFlags & GLS_MEASUREMENT_FLAGS_CONCENTRATION_AND_TYPE_SAMPLE_LOCATION_PRESENT))
         {
            /* Verify that there is enough data to hold the Glucose     */
            /* Concentration Data.                                      */
            if(BufferLength >= GLS_CONCENTRATION_SIZE)
            {
               /* Read the Concentration Value from the stream and check*/
               /* to see if it is a special value.                      */
               Measurement->GlucoseConcentration.Value = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Concentration_t *)Value)->Concentration));
               if((Measurement->GlucoseConcentration.Value != NOT_A_NUMBER) && (Measurement->GlucoseConcentration.Value != NOT_AT_THIS_RESOLUTION) && (Measurement->GlucoseConcentration.Value != POSITIVE_INFINITY) && (Measurement->GlucoseConcentration.Value != NEGATIVE_INFINITY))
                  Measurement->GlucoseConcentration.ConcentrationValid = TRUE;
               else
                  Measurement->GlucoseConcentration.ConcentrationValid = FALSE;

               /* Extract the Sample Location and Type.                 */
               TempByte                                         = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Concentration_t *)Value)->Type_Sample_Location));
               Measurement->GlucoseConcentration.SampleLocation = (Byte_t)(TempByte >> 4);
               Measurement->GlucoseConcentration.Type           = (Byte_t)(TempByte & 0x0F);

               /* Flag the presence of the Glucose Concentration and    */
               /* adjust the buffer information.                        */
               Value        += GLS_CONCENTRATION_SIZE;
               BufferLength -= GLS_CONCENTRATION_SIZE;
            }
            else
               ret_val = GLS_ERROR_MALFORMATTED_DATA;
         }

         /* Check to see if Glucose Concentration information is        */
         /* present.                                                    */
         if((!ret_val) && (Measurement->OptionFlags & GLS_MEASUREMENT_FLAGS_SENSOR_STATUS_ANNUNCIATION_PRESENT))
         {
            /* Verify that there is enough data to hold the Sensor      */
            /* Status Data.                                             */
            if(BufferLength >= GLS_SENSOR_STATUS_ANNUNCIATION_SIZE)
            {
               Measurement->SensorStatus = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Sensor_Status_Annunciation_t *)Value)->Sensor_Status));
            }
            else
               ret_val = GLS_ERROR_MALFORMATTED_DATA;
         }
      }
      else
         ret_val = GLS_ERROR_MALFORMATTED_DATA;
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is used to decode a glucose measurement    */
   /* that has been received from a remote Client.                      */
static int DecodeGlucoseMeasurementContext(unsigned int BufferLength, Byte_t *Value, GLS_Glucose_Measurement_Context_Data_t *MeasurementContext)
{
   int    ret_val;
   Byte_t TempByte;

   /* Verify that the input parameters appear semi-valid.               */
   if((BufferLength) && (Value) && (MeasurementContext))
   {
      /* Verify that the Glucose Context is a valid length.             */
      if(BufferLength >= ((unsigned int)GLS_GLUCOSE_MEASUREMENT_CONTEXT_HEADER_SIZE))
      {
         /* Initialize the Glucose Context.                             */
         BTPS_MemInitialize(MeasurementContext, 0, GLS_GLUCOSE_MEASUREMENT_CONTEXT_DATA_SIZE);

         /* Initialize the return value to success.                     */
         ret_val = 0;

         /* Read the Flags from the Glucose Measurement.                */
         MeasurementContext->OptionFlags = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Glucose_Measurement_Context_Header_t *)Value)->Flags));

         /* Load the Header Data.                                       */
         MeasurementContext->SequenceNumber  = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Glucose_Measurement_Context_Header_t *)Value)->Sequence_Number));

         /* Update the pointer to the next field.                       */
         Value        += GLS_GLUCOSE_MEASUREMENT_CONTEXT_HEADER_SIZE;
         BufferLength -= GLS_GLUCOSE_MEASUREMENT_CONTEXT_HEADER_SIZE;

         /* Flag if there is extended data present.                     */
         if(MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_EXTENDED_FLAGS_PRESENT)
         {
            if(BufferLength >= GLS_CONTEXT_EXTENDED_FLAGS_SIZE)
            {
               MeasurementContext->ExtendedFlags = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Context_Extended_Flags_t *)Value)->Extended_Flags));

               /* Update the pointer to the next field.                 */
               Value        += GLS_CONTEXT_EXTENDED_FLAGS_SIZE;
               BufferLength -= GLS_CONTEXT_EXTENDED_FLAGS_SIZE;
            }
            else
               ret_val = GLS_ERROR_MALFORMATTED_DATA;
         }

         /* Check to see if Corbohydrate data has been provided.        */
         if((!ret_val) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_CARBOHYDRATE_PRESENT))
         {
            /* Verify that there is enough data to contain the          */
            /* Carbohydrate Data.                                       */
            if(BufferLength >= GLS_CONTEXT_CARBOHYDRATE_SIZE)
            {
               MeasurementContext->Carbohydrate.ID     = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Context_Carbohydrate_t *)Value)->Carbohydrate_ID));
               MeasurementContext->Carbohydrate.Value = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Context_Carbohydrate_t *)Value)->Carbohydrate));

               /* Update the pointer to the next field.                 */
               Value        += GLS_CONTEXT_CARBOHYDRATE_SIZE;
               BufferLength -= GLS_CONTEXT_CARBOHYDRATE_SIZE;
            }
            else
               ret_val = GLS_ERROR_MALFORMATTED_DATA;
         }

         /* Check to see if there is Meal data present.                 */
         if((!ret_val) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_MEAL_PRESENT))
         {
            /* Verify that there is enough data to contain the Meal     */
            /* Information.                                             */
            if(BufferLength >= GLS_CONTEXT_MEAL_SIZE)
            {
               MeasurementContext->Meal = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Context_Meal_t *)Value)->Meal));

               /* Update the pointer to the next field.                 */
               Value        += GLS_CONTEXT_MEAL_SIZE;
               BufferLength -= GLS_CONTEXT_MEAL_SIZE;
            }
            else
               ret_val = GLS_ERROR_MALFORMATTED_DATA;
         }

         /* Check to see if there is Health-Tester data present.        */
         if((!ret_val) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_TESTER_HEALTH_PRESENT))
         {
            /* Verify that there is enough data to contain the          */
            /* Health-Tester Information.                               */
            if(BufferLength >= ((unsigned int)GLS_HEALTH_TESTER_SIZE))
            {
               TempByte                   = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Health_Tester_t *)Value)->Health_Tester));
               MeasurementContext->Health = (Byte_t)(TempByte >> 4);
               MeasurementContext->Tester = (Byte_t)(TempByte & 0x0F);

               /* Update the pointer to the next field.                 */
               Value        += GLS_HEALTH_TESTER_SIZE;
               BufferLength -= GLS_HEALTH_TESTER_SIZE;
            }
            else
               ret_val = GLS_ERROR_MALFORMATTED_DATA;
         }

         /* Check to see if there is Exercise data present.             */
         if((!ret_val) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_EXERCISE_PRESENT))
         {
            /* Verify that there is enough data to contain the Exercise */
            /* Information.                                             */
            if(BufferLength >= GLS_CONTEXT_EXERCISE_SIZE)
            {
               MeasurementContext->ExerciseData.Duration  = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Context_Exercise_t *)Value)->Exercise_Duration));
               MeasurementContext->ExerciseData.Intensity = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Context_Exercise_t *)Value)->Exercise_Intensity));

               /* Update the pointer to the next field.                 */
               Value        += GLS_CONTEXT_EXERCISE_SIZE;
               BufferLength -= GLS_CONTEXT_EXERCISE_SIZE;
            }
            else
               ret_val = GLS_ERROR_MALFORMATTED_DATA;
         }

         /* Check to see if there is Medication data present.           */
         if((!ret_val) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_MEDICATION_PRESENT))
         {
            /* Verify that there is enough data to contain the          */
            /* Medication Information.                                  */
            if(BufferLength >= GLS_CONTEXT_MEDICATION_SIZE)
            {
               MeasurementContext->Medication.Value = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Context_Medication_t *)Value)->Medication));
               MeasurementContext->Medication.ID    = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Context_Medication_t *)Value)->Medication_ID));

               /* Update the pointer to the next field.                 */
               Value        += GLS_CONTEXT_MEDICATION_SIZE;
               BufferLength -= GLS_CONTEXT_MEDICATION_SIZE;
            }
            else
               ret_val = GLS_ERROR_MALFORMATTED_DATA;
         }

         /* Check to see if there is HbA1c data present.                */
         if((!ret_val) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_HBA1C_PRESENT))
         {
            /* Verify that there is enough data to contain the HbA1c    */
            /* Information.                                             */
            if(BufferLength >= GLS_CONTEXT_HBA1C_SIZE)
            {
               MeasurementContext->HbA1c = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Context_HBA1C_t *)Value)->HbA1c));
            }
            else
               ret_val = GLS_ERROR_MALFORMATTED_DATA;
         }
      }
      else
         ret_val = GLS_ERROR_MALFORMATTED_DATA;
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is a utility function that exists to decode*/
   /* an Access Control Point into a user specified value.  This        */
   /* function returns the zero if successful or a negative error code. */
static int DecodeRACPCommand(unsigned int BufferLength, Byte_t *Buffer, GLS_Record_Access_Control_Point_Format_Data_t *FormatData)
{
   int ret_val = 0;

   /* Verify that the input parameters are valid.                       */
   if((BufferLength >= GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(0)) && (Buffer) && (FormatData))
   {
      /* Assign the command type and operator type for this RACP packet.*/
      /* Validity of the data will be checked later.                    */
      FormatData->CommandType  = (GLS_RACP_Command_Type_t)READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Op_Code));
      FormatData->OperatorType = (GLS_RACP_Operator_Type_t)READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Operator));

      /* Check if a filter is expected.                                 */
      if((FormatData->CommandType == racReportStoredRecordsRequest) || (FormatData->CommandType == racDeleteStoredRecordsRequest) || (FormatData->CommandType == racNumberOfStoredRecordsRequest))
      {
         /* Check if a filter type with one parameter is expected and   */
         /* check minimum length.                                       */
         if(((FormatData->OperatorType == raoLessThanOrEqualTo) || (FormatData->OperatorType == raoGreaterThanOrEqualTo)) && (BufferLength >= ((unsigned int)GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(sizeof(NonAlignedWord_t) + sizeof(NonAlignedByte_t)))))
         {
            FormatData->FilterType = (GLS_RACP_Filter_Type_t)READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[0]));

            if(FormatData->FilterType == rafSequenceNumber)
            {
               /* Assign the filter parameter.                          */
               FormatData->FilterParameters.SequenceNumber = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1]));
            }
            else
            {
               if(FormatData->FilterType == rafUserFacingTime)
               {
                  /* Validate buffer length.                            */
                  if(BufferLength >= ((unsigned int)GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE + sizeof(NonAlignedByte_t))))
                  {
                     /* Assign the filter parameter.                    */
                     FormatData->FilterParameters.UserFacingTime.Year    = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Year));
                     FormatData->FilterParameters.UserFacingTime.Month   = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Month));
                     FormatData->FilterParameters.UserFacingTime.Day     = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Day));
                     FormatData->FilterParameters.UserFacingTime.Hours   = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Hours));
                     FormatData->FilterParameters.UserFacingTime.Minutes = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minutes));
                     FormatData->FilterParameters.UserFacingTime.Seconds = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Seconds));
                  }
                  else
                  {
                     ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
                  }
               }
               else
               {
                  /* Reserved FilterType Received                       */
               }
            }
         }
         else
         {
            /* Check if a filter type with two parameters is expected   */
            /* and check minimum length.                                */
            if((FormatData->OperatorType == raoWithinRangeOf) && (BufferLength >= ((unsigned int)GLS_RECORD_ACCESS_CONTROL_POINT_SIZE((sizeof(NonAlignedWord_t) * 2) + sizeof(NonAlignedByte_t)))))
            {
               FormatData->FilterType = (GLS_RACP_Filter_Type_t)READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[0]));

               /* A filter type with two parameters is expected, format */
               /* the packet based on the filter type.                  */
               if(FormatData->FilterType == rafSequenceNumber)
               {
                  /* Assign the filter parameters.                      */
                  FormatData->FilterParameters.SequenceNumberRange.Minimum = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1]));
                  FormatData->FilterParameters.SequenceNumberRange.Maximum = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[3]));
               }
               else
               {
                  if(FormatData->FilterType == rafUserFacingTime)
                  {
                     /* Validate buffer length.                         */
                     if(BufferLength >= ((unsigned int)GLS_RECORD_ACCESS_CONTROL_POINT_SIZE((GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE * 2) + sizeof(NonAlignedByte_t))))
                     {
                        /* Assign the filter parameters.                */
                        FormatData->FilterParameters.UserFacingTimeRange.Minimum.Year    = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Year));
                        FormatData->FilterParameters.UserFacingTimeRange.Minimum.Month   = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Month));
                        FormatData->FilterParameters.UserFacingTimeRange.Minimum.Day     = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Day));
                        FormatData->FilterParameters.UserFacingTimeRange.Minimum.Hours   = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Hours));
                        FormatData->FilterParameters.UserFacingTimeRange.Minimum.Minutes = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Minutes));
                        FormatData->FilterParameters.UserFacingTimeRange.Minimum.Seconds = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Seconds));

                        FormatData->FilterParameters.UserFacingTimeRange.Maximum.Year    = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Year));
                        FormatData->FilterParameters.UserFacingTimeRange.Maximum.Month   = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Month));
                        FormatData->FilterParameters.UserFacingTimeRange.Maximum.Day     = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Day));
                        FormatData->FilterParameters.UserFacingTimeRange.Maximum.Hours   = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Hours));
                        FormatData->FilterParameters.UserFacingTimeRange.Maximum.Minutes = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Minutes));
                        FormatData->FilterParameters.UserFacingTimeRange.Maximum.Seconds = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Seconds));
                     }
                     else
                     {
                        ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
                     }
                  }
                  else
                  {
                     /* Filter type not allowed for this command /      */
                     /* operator.                                       */
                     ret_val = GLS_ERROR_INVALID_PARAMETER;
                  }
               }
            }
         }
      }
      else
      {
         /* Validate the command and operator types.                    */
         if((FormatData->CommandType != racAbortOperationRequest) || (FormatData->OperatorType != raoNull))
         {
            if(((FormatData->CommandType != GLS_RECORD_ACCESS_OPCODE_RFU_0) && (FormatData->CommandType < GLS_RECORD_ACCESS_OPCODE_RFU_7 || FormatData->CommandType > GLS_RECORD_ACCESS_OPCODE_RFU_255)) || (FormatData->OperatorType != raoAllRecords))
            {
               /* Command / Operator type not allowed.                  */
               ret_val = GLS_ERROR_INVALID_PARAMETER;
            }
         }
      }
   }
   else
   {
      ret_val = GLS_ERROR_INVALID_PARAMETER;
   }

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that exists to decode*/
   /* Value into Record_Access_Control_Point_Response_Data_t.This       */
   /* function returns the zero if successful or a negative error code. */
static int DecodeRACPResponse(unsigned int ValueLength, Byte_t *Value, GLS_Record_Access_Control_Point_Response_Data_t *RACPData)
{
   int    ret_val;
   Byte_t Opcode;

   /* Verify that the input parameters appear semi-valid.               */
   if((ValueLength) && (Value) && (RACPData))
   {
      /* Read the response type from the packet.                        */
      Opcode = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&((((GLS_Record_Access_Control_Point_t *)Value)->Op_Code)));
      if(Opcode == GLS_RECORD_ACCESS_OPCODE_RESPONSE_CODE)
      {
         /* Verify that the Record Access Control Point data has a valid*/
         /* length                                                      */
         if(ValueLength >= (GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(sizeof(NonAlignedByte_t) * 2)))
         {
            /* Load the Response data.                                  */
            RACPData->ResponseType                                     = rarResponseCode;
            RACPData->ResponseData.ResponseCodeValue.RequestOpCode     = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Record_Access_Control_Point_t *)Value)->Variable_Data[0]));
            RACPData->ResponseData.ResponseCodeValue.ResponseCodeValue = READ_UNALIGNED_BYTE_LITTLE_ENDIAN(&(((GLS_Record_Access_Control_Point_t *)Value)->Variable_Data[1]));
            ret_val                                                    = 0;
         }
         else
            ret_val = GLS_ERROR_MALFORMATTED_DATA;
      }
      else
      {
         if(Opcode == GLS_RECORD_ACCESS_OPCODE_NUM_STORED_RECORDS_RESPONSE)
         {
            /* Verify that the Record Access Control Point data has a   */
            /* valid length                                             */
            if(ValueLength >= (GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(sizeof(NonAlignedWord_t))))
            {
               RACPData->ResponseType                             = rarNumberOfStoredRecords;
               RACPData->ResponseData.NumberOfStoredRecordsResult = READ_UNALIGNED_WORD_LITTLE_ENDIAN((((GLS_Record_Access_Control_Point_t *)Value)->Variable_Data));
               ret_val                                            = 0;
            }
            else
            {
               ret_val = GLS_ERROR_MALFORMATTED_DATA;
            }
         }
         else
            ret_val = GLS_ERROR_MALFORMATTED_DATA;
      }
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that exists to format*/
   /* a Glucose Measurement Command into a user specified buffer.       */
static int FormatGlucoseMeasurement(GLS_Glucose_Measurement_Data_t *Measurement, unsigned int BufferLength, Byte_t *Buffer)
{
   int ret_val;

   /* Verify that the input parameters appear valid.                    */
   if((Measurement) && (Buffer) && (BufferLength >= GLS_MEASUREMENT_HEADER_SIZE(0)))
   {
      /* Assign all of the static data to the Measurement Structure.    */
      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Measurement_Header_t *)Buffer)->Flags),            Measurement->OptionFlags);
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Measurement_Header_t *)Buffer)->Sequence_Number),  Measurement->SequenceNumber);
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Measurement_Header_t *)Buffer)->Base_Time.Year),   Measurement->BaseTime.Year);
      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Measurement_Header_t *)Buffer)->Base_Time.Month),  Measurement->BaseTime.Month);
      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Measurement_Header_t *)Buffer)->Base_Time.Day),    Measurement->BaseTime.Day);
      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Measurement_Header_t *)Buffer)->Base_Time.Hours),   Measurement->BaseTime.Hours);
      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Measurement_Header_t *)Buffer)->Base_Time.Minutes), Measurement->BaseTime.Minutes);
      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Measurement_Header_t *)Buffer)->Base_Time.Seconds), Measurement->BaseTime.Seconds);

      /* Initialize the number of Bytes that have been used and adjust  */
      /* the buffer information.                                        */
      ret_val       = GLS_MEASUREMENT_HEADER_SIZE(0);
      Buffer       += GLS_MEASUREMENT_HEADER_SIZE(0);
      BufferLength -= GLS_MEASUREMENT_HEADER_SIZE(0);

      /* Set a pointer to the first optional field and start adding any */
      /* optional data that is specified.                               */

      /* Check to see if Time Offset Present.                           */
      if(Measurement->OptionFlags & GLS_MEASUREMENT_FLAGS_TIME_OFFSET_PRESENT)
      {
         /* Verify that there is room in the buffer to hold the Time    */
         /* Offset.                                                     */
         if(BufferLength >= GLS_TIME_OFFSET_SIZE)
         {
            ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(Buffer, Measurement->TimeOffset);
            ret_val      += GLS_TIME_OFFSET_SIZE;
            Buffer       += GLS_TIME_OFFSET_SIZE;
            BufferLength -= GLS_TIME_OFFSET_SIZE;
         }
         else
            ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
      }

      /* Verify that no error has occurred and that there is Glucose    */
      /* Concentration, Type and Sample Location Present.               */
      if((ret_val > 0) && (Measurement->OptionFlags & GLS_MEASUREMENT_FLAGS_CONCENTRATION_AND_TYPE_SAMPLE_LOCATION_PRESENT))
      {
         /* Verify that there is room in the buffer to hold the         */
         /* Concentration Data.                                         */
         if(BufferLength >= GLS_CONCENTRATION_SIZE)
         {
            ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Concentration_t *)Buffer)->Concentration),        Measurement->GlucoseConcentration.Value);
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Concentration_t *)Buffer)->Type_Sample_Location), ((Byte_t)((Measurement->GlucoseConcentration.SampleLocation & 0x0F) << 4) | (Byte_t)(Measurement->GlucoseConcentration.Type & 0x0F)));

            /* Adjust the return value and the Information data.        */
            ret_val      += GLS_CONCENTRATION_SIZE;
            Buffer       += GLS_CONCENTRATION_SIZE;
            BufferLength -= GLS_CONCENTRATION_SIZE;
         }
         else
            ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
      }

      /* Verify that no error has occurred and that there is Sensor     */
      /* Status Annunciation Present.                                   */
      if((ret_val > 0) && (Measurement->OptionFlags & GLS_MEASUREMENT_FLAGS_SENSOR_STATUS_ANNUNCIATION_PRESENT))
      {
         /* Verify that there is room in the buffer to hold the Status  */
         /* Information.                                                */
         if(BufferLength >= GLS_SENSOR_STATUS_ANNUNCIATION_SIZE)
         {
            ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Sensor_Status_Annunciation_t *)Buffer)->Sensor_Status), Measurement->SensorStatus);

            ret_val += GLS_SENSOR_STATUS_ANNUNCIATION_SIZE;
         }
         else
            ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
      }
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that exists to format*/
   /* Glucose Measurement Context data into a user specified buffer.    */
static int FormatGlucoseMeasurementContext(GLS_Glucose_Measurement_Context_Data_t *MeasurementContext, unsigned int BufferLength, Byte_t *Buffer)
{
   int ret_val;

   /* Verify that the input parameters appear valid.                    */
   if((MeasurementContext) && (Buffer) && (BufferLength >= GLS_GLUCOSE_MEASUREMENT_CONTEXT_HEADER_SIZE))
   {
      /* Assign all of the static data to the Measurement Structure.    */
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Glucose_Measurement_Context_Header_t *)Buffer)->Flags),           MeasurementContext->OptionFlags);
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Glucose_Measurement_Context_Header_t *)Buffer)->Sequence_Number), MeasurementContext->SequenceNumber);

      /* Set a pointer to the first optional field and start adding any */
      /* optional data that is specified.                               */
      ret_val       = GLS_GLUCOSE_MEASUREMENT_CONTEXT_HEADER_SIZE;
      Buffer       += GLS_GLUCOSE_MEASUREMENT_CONTEXT_HEADER_SIZE;
      BufferLength -= GLS_GLUCOSE_MEASUREMENT_CONTEXT_HEADER_SIZE;

      /* Check to see if Extended Flags are present.                    */
      if(MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_EXTENDED_FLAGS_PRESENT)
      {
         /* Verify that there is enough buffer space to contain the     */
         /* Extended Flags.                                             */
         if(BufferLength >= GLS_CONTEXT_EXTENDED_FLAGS_SIZE)
         {
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Context_Extended_Flags_t *)Buffer)->Extended_Flags), MeasurementContext->ExtendedFlags);

            /* Adjust the return value and the Information data.        */
            ret_val      += GLS_CONTEXT_EXTENDED_FLAGS_SIZE;
            Buffer       += GLS_CONTEXT_EXTENDED_FLAGS_SIZE;
            BufferLength -= GLS_CONTEXT_EXTENDED_FLAGS_SIZE;
         }
         else
            ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
      }

      /* Check to see if Carbohydrates are present.                     */
      if((ret_val > 0) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_CARBOHYDRATE_PRESENT))
      {
         /* Verify that there is enough buffer space to contain the     */
         /* Carbhydrate Data.                                           */
         if(BufferLength >= GLS_CONTEXT_CARBOHYDRATE_SIZE)
         {
            ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Context_Carbohydrate_t *)Buffer)->Carbohydrate),    MeasurementContext->Carbohydrate.Value);
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Context_Carbohydrate_t *)Buffer)->Carbohydrate_ID), MeasurementContext->Carbohydrate.ID);

            /* Adjust the return value and the Information data.        */
            ret_val      += GLS_CONTEXT_CARBOHYDRATE_SIZE;
            Buffer       += GLS_CONTEXT_CARBOHYDRATE_SIZE;
            BufferLength -= GLS_CONTEXT_CARBOHYDRATE_SIZE;
         }
         else
            ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
      }

      /* Verify that no error has occurred and that there is Meal data  */
      /* present.                                                       */
      if((ret_val > 0) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_MEAL_PRESENT))
      {
         /* Verify that there is room in the buffer to hold the Meal    */
         /* Information Data.                                           */
         if(BufferLength >= GLS_CONTEXT_MEAL_SIZE)
         {
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Context_Meal_t *)Buffer)->Meal), MeasurementContext->Meal);

            /* Adjust the return value and the Information data.        */
            ret_val      += GLS_CONTEXT_MEAL_SIZE;
            Buffer       += GLS_CONTEXT_MEAL_SIZE;
            BufferLength -= GLS_CONTEXT_MEAL_SIZE;
         }
         else
            ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
      }

      /* Verify that no error has occurred and that there is            */
      /* Tester/Health data present.                                    */
      if((ret_val > 0) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_TESTER_HEALTH_PRESENT))
      {
         /* Verify that there is room in the buffer to hold the Meal    */
         /* Information Data.                                           */
         if(BufferLength >= GLS_HEALTH_TESTER_SIZE)
         {
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Health_Tester_t *)Buffer)->Health_Tester), ((Byte_t)(((MeasurementContext->Health & 0x0F) << 4) | (MeasurementContext->Tester & 0x0F))));

            /* Adjust the return value and the Information data.        */
            ret_val      += GLS_HEALTH_TESTER_SIZE;
            Buffer       += GLS_HEALTH_TESTER_SIZE;
            BufferLength -= GLS_HEALTH_TESTER_SIZE;
         }
         else
            ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
      }

      /* Verify that no error has occurred and that there is Exercise   */
      /* data present.                                                  */
      if((ret_val > 0) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_EXERCISE_PRESENT))
      {
         /* Verify that there is room in the buffer to hold the Meal    */
         /* Information Data.                                           */
         if(BufferLength >= GLS_CONTEXT_EXERCISE_SIZE)
         {
            ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Context_Exercise_t *)Buffer)->Exercise_Duration),  MeasurementContext->ExerciseData.Duration);
            ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Context_Exercise_t *)Buffer)->Exercise_Intensity), MeasurementContext->ExerciseData.Intensity);

            /* Adjust the return value and the Information data.        */
            ret_val      += GLS_CONTEXT_EXERCISE_SIZE;
            Buffer       += GLS_CONTEXT_EXERCISE_SIZE;
            BufferLength -= GLS_CONTEXT_EXERCISE_SIZE;
         }
         else
            ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
      }
      /* Verify that no error has occurred and that there is Medication */
      /* data present.                                                  */
      if((ret_val > 0) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_MEDICATION_PRESENT))
      {
         /* Verify that there is room in the buffer to hold the Meal    */
         /* Information Data.                                           */
         if(BufferLength >= GLS_CONTEXT_MEDICATION_SIZE)
         {
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Context_Medication_t *)Buffer)->Medication_ID), MeasurementContext->Medication.ID);
            ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Context_Medication_t *)Buffer)->Medication), MeasurementContext->Medication.Value);

            /* Adjust the return value and the Information data.        */
            ret_val      += GLS_CONTEXT_MEDICATION_SIZE;
            Buffer       += GLS_CONTEXT_MEDICATION_SIZE;
            BufferLength -= GLS_CONTEXT_MEDICATION_SIZE;
         }
         else
            ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
      }

      /* Verify that no error has occurred and that there is HBA1C data */
      /* present.                                                       */
      if((ret_val > 0) && (MeasurementContext->OptionFlags & GLS_MEASUREMENT_CONTEXT_FLAGS_HBA1C_PRESENT))
      {
         /* Verify that there is room in the buffer to hold the Meal    */
         /* Information Data.                                           */
         if(BufferLength >= GLS_CONTEXT_HBA1C_SIZE)
         {
            ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Context_HBA1C_t *)Buffer)->HbA1c), MeasurementContext->HbA1c);

            ret_val += GLS_CONTEXT_HBA1C_SIZE;
         }
         else
            ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
      }

   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function that exists to format*/
   /* a GLS Event into the specified buffer.                            */
   /* * NOTE * TransactionID is optional and may be set to 0.           */
   /* * NOTE * BD_ADDR is NOT optional and may NOT be set to NULL.      */
static GLS_Event_Data_t *FormatEventHeader(unsigned int BufferLength, Byte_t *Buffer, GLS_Event_Type_t EventType, unsigned int InstanceID, unsigned int ConnectionID, unsigned int TransactionID)
{
   GLS_Event_Data_t *EventData = NULL;

   if((BufferLength >= (GLS_EVENT_DATA_SIZE + GLS_EVENT_DATA_BUFFER_SIZE)) && (Buffer))
   {
      /* Format the header of the event, that is data that is common to */
      /* all events.                                                    */
      EventData                                                                 = (GLS_Event_Data_t *)Buffer;
      EventData->Event_Data_Type                                                = EventType;

      EventData->Event_Data.GLS_Client_Configuration_Update_Data                = (GLS_Client_Configuration_Update_Data_t *)(((Byte_t *)EventData) + GLS_EVENT_DATA_SIZE);
      EventData->Event_Data.GLS_Client_Configuration_Update_Data->InstanceID    = InstanceID;
      EventData->Event_Data.GLS_Client_Configuration_Update_Data->ConnectionID  = ConnectionID;

      if(TransactionID)
      {
         EventData->Event_Data.GLS_Read_Client_Configuration_Data->TransactionID = TransactionID;
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

   for(Index=0; Index < GLS_MAXIMUM_SUPPORTED_INSTANCES; Index++)
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
static GLSServerInstance_t *AcquireServiceInstance(unsigned int BluetoothStackID, unsigned int *InstanceID)
{
   unsigned int         LocalInstanceID;
   unsigned int         Index;
   GLSServerInstance_t *ret_val = NULL;

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
         if((LocalInstanceID) && (LocalInstanceID <= GLS_MAXIMUM_SUPPORTED_INSTANCES))
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
               for(Index=0;Index<GLS_MAXIMUM_SUPPORTED_INSTANCES;Index++)
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
   int ret_val = GLS_ERROR_MALFORMATTED_DATA;

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
         ret_val = GLS_ERROR_INVALID_PARAMETER;
   }

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is a utility function which is used to     */
   /* register an GLS Service.  This function returns the positive,     */
   /* non-zero, Instance ID on success or a negative error code.        */
static int GLSRegisterService(unsigned int BluetoothStackID, GLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   int                  ret_val;
   unsigned int         InstanceID;
   GLSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (EventCallback) && (ServiceID))
   {
      /* Verify that no instance is registered to this Bluetooth Stack. */
      if(!InstanceRegisteredByStackID(BluetoothStackID))
      {
         /* Acquire a free GLS Instance.                                */
         InstanceID = 0;
         if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
         {
            /* Call GATT to register the GLS service.                   */
            ret_val = GATT_Register_Service(BluetoothStackID, GLS_SERVICE_FLAGS, GLS_MONITOR_SERVICE_ATTRIBUTE_COUNT, (GATT_Service_Attribute_Entry_t *)Glucose_Monitor_Service, ServiceHandleRange, GATT_ServerEventCallback, InstanceID);
            if(ret_val > 0)
            {
               /* Save the Instance information.                        */
               ServiceInstance->BluetoothStackID  = BluetoothStackID;
               ServiceInstance->ServiceID         = (unsigned int)ret_val;
               ServiceInstance->EventCallback     = EventCallback;
               ServiceInstance->CallbackParameter = CallbackParameter;
               *ServiceID                         = (unsigned int)ret_val;

               /* Intilize the Instance Data for this instance.         */
               BTPS_MemInitialize(&InstanceData[InstanceID-1], 0, GLS_INSTANCE_DATA_SIZE);

               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(InstanceData[InstanceID-1].Feature_Data_Length), sizeof(Word_t));

               /* Return the GLS Instance ID.                           */
               ret_val                            = (int)InstanceID;
            }

            /* UnLock the previously locked Bluetooth Stack.            */
            BSC_UnLockBluetoothStack(BluetoothStackID);
         }
         else
            ret_val = GLS_ERROR_INSUFFICIENT_RESOURCES;
      }
      else
         ret_val = GLS_ERROR_SERVICE_ALREADY_REGISTERED;
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is the GATT Server Event Callback that     */
   /* handles all requests made to the GLS Service for all registered   */
   /* instances.                                                        */
static void BTPSAPI GATT_ServerEventCallback(unsigned int BluetoothStackID, GATT_Server_Event_Data_t *GATT_ServerEventData, unsigned long CallbackParameter)
{
   Byte_t                 ErrorCode;
   Word_t                 AttributeOffset;
   Word_t                 InstanceTag;
   Word_t                 ValueLength;
   Byte_t                *Value;
   unsigned int           TransactionID;
   unsigned int           InstanceID;
   GLS_Event_Data_t      *EventData;
   GLSServerInstance_t   *ServiceInstance;
   GAP_Encryption_Mode_t  GAP_Encryption_Mode;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_ServerEventData) && (CallbackParameter))
   {
      /* The Instance ID is always registered as the callback parameter.*/
      InstanceID = (unsigned int)CallbackParameter;

      /* Acquire the Service Instance for the specified service.        */
      if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
      {
         switch(GATT_ServerEventData->Event_Data_Type)
         {
            case etGATT_Server_Read_Request:
               /* Verify that the Event Data is valid.                  */
               if(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data)
               {
                  AttributeOffset = GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeOffset;
                  TransactionID   = GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->TransactionID;

                  /* Verify that they are not trying to Read a Long     */
                  /* value, which is indicated by a non-zeroi offset.   */
                  if(!(GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->AttributeValueOffset))
                  {
                     if((Glucose_Monitor_Service[AttributeOffset].Attribute_Entry_Type == aetCharacteristicDescriptor16))
                     {
                        EventData = FormatEventHeader(sizeof(ServiceInstance->EventBuffer), ServiceInstance->EventBuffer, etGLS_Read_Client_Configuration_Request, InstanceID, GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ConnectionID, TransactionID);
                        if(EventData)
                        {
                           /* Set the Error Code for Success.           */
                           ErrorCode = 0;

                           /* Format the rest of the event.             */
                           EventData->Event_Data_Size = GLS_READ_CLIENT_CONFIGURATION_DATA_SIZE;

                           /* Determine the Correct Client Configuration*/
                           /* Type.                                     */
                           if(AttributeOffset == GLS_MEASUREMENT_CCD_ATTRIBUTE_OFFSET)
                           {
                              EventData->Event_Data.GLS_Read_Client_Configuration_Data->ClientConfigurationType = ctGlucoseMeasurement;
                           }
                           else
                           {
                              if(AttributeOffset == GLS_RECORDS_ACCESS_CONTROL_POINT_CCD_ATTRIBUTE_OFFSET)
                                 EventData->Event_Data.GLS_Read_Client_Configuration_Data->ClientConfigurationType = ctRecordAccessControlPoint;
                              else
                              {
                                 if(AttributeOffset == GLS_MEASUREMENT_CONTEXT_CCD_ATTRIBUTE_OFFSET)
                                 {
                                    EventData->Event_Data.GLS_Read_Client_Configuration_Data->ClientConfigurationType = ctGlucoseMeasurementContext;
                                 }
                                 else
                                 {
                                    ErrorCode = ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED;
                                 }
                              }
                           }

                           /* Verify that the processing of the request */
                           /* was successful.                           */
                           if(!ErrorCode)
                           {
                              /* Add the information about the remote   */
                              /* device.                                */
                              EventData->Event_Data.GLS_Read_Client_Configuration_Data->ConnectionType = GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->ConnectionType;
                              EventData->Event_Data.GLS_Read_Client_Configuration_Data->RemoteDevice   = GATT_ServerEventData->Event_Data.GATT_Read_Request_Data->RemoteDevice;

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
                              GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ErrorCode);
                        }
                        else
                           GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                     }
                     else
                     {
                        if(AttributeOffset == GLS_FEATURE_ATTRIBUTE_OFFSET)
                        {
                           /* Get the instance tag for the              */
                           /* characteristic.                           */
                           InstanceTag = (Word_t)(((GATT_Characteristic_Value_16_Entry_t *)Glucose_Monitor_Service[AttributeOffset].Attribute_Value)->Characteristic_Value_Length);
                           ValueLength = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(((Byte_t *)(&InstanceData[InstanceID-1]))[InstanceTag]));
                           Value       = (Byte_t *)(&(((Byte_t *)(&InstanceData[InstanceID-1]))[InstanceTag + WORD_SIZE]));

                           /* Respond with the data.                    */
                           GATT_Read_Response(BluetoothStackID, TransactionID, (unsigned int)ValueLength, Value);
                        }
                        else
                           GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED);
                     }
                  }
                  else
                     GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG);
               }
               break;
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
                     if(Glucose_Monitor_Service[AttributeOffset].Attribute_Entry_Type == aetCharacteristicDescriptor16)
                     {
                        /* Begin formatting the Client Configuration    */
                        /* Update event.                                */
                        EventData = FormatEventHeader(sizeof(ServiceInstance->EventBuffer), ServiceInstance->EventBuffer, etGLS_Client_Configuration_Update, InstanceID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->ConnectionID, 0);
                        if(EventData)
                        {
                           /* Format the rest of the event.             */
                           EventData->Event_Data_Size = GLS_CLIENT_CONFIGURATION_UPDATE_DATA_SIZE;

                           /* Attempt to decode the client configuration*/
                           /* value.                                    */
                           if(!DecodeClientConfigurationValue(ValueLength, Value, &(EventData->Event_Data.GLS_Client_Configuration_Update_Data->ClientConfiguration)))
                           {
                              /* Set the Error Code for Success.        */
                              ErrorCode = 0;

                              /* Determine the Correct Client           */
                              /* Configuration Type.                    */
                              if(AttributeOffset == GLS_MEASUREMENT_CCD_ATTRIBUTE_OFFSET)
                              {
                                 EventData->Event_Data.GLS_Client_Configuration_Update_Data->ClientConfigurationType  = ctGlucoseMeasurement;
                                 EventData->Event_Data.GLS_Client_Configuration_Update_Data->ClientConfiguration     &= GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE;
                              }
                              else
                              {
                                 if(AttributeOffset == GLS_RECORDS_ACCESS_CONTROL_POINT_CCD_ATTRIBUTE_OFFSET)
                                 {
                                    EventData->Event_Data.GLS_Client_Configuration_Update_Data->ClientConfigurationType  = ctRecordAccessControlPoint;
                                    EventData->Event_Data.GLS_Client_Configuration_Update_Data->ClientConfiguration     &= GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_INDICATE_ENABLE;
                                 }
                                 else
                                 {
                                    if(AttributeOffset == GLS_MEASUREMENT_CONTEXT_CCD_ATTRIBUTE_OFFSET)
                                    {
                                       EventData->Event_Data.GLS_Client_Configuration_Update_Data->ClientConfigurationType  = ctGlucoseMeasurementContext;
                                       EventData->Event_Data.GLS_Client_Configuration_Update_Data->ClientConfiguration     &= GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE;
                                    }
                                    else
                                       ErrorCode = ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED;
                                 }
                              }

                              if(!ErrorCode)
                              {
                                 /* Go ahead and accept the write       */
                                 /* request since we have decoded the   */
                                 /* Client Configuration Value          */
                                 /* successfully.                       */
                                 GATT_Write_Response(BluetoothStackID, TransactionID);

                                 /* Add the information about the remote*/
                                 /* device.                             */
                                 EventData->Event_Data.GLS_Client_Configuration_Update_Data->ConnectionType = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->ConnectionType;
                                 EventData->Event_Data.GLS_Client_Configuration_Update_Data->RemoteDevice   = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->RemoteDevice;

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
                                 GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ErrorCode);
                           }
                           else
                              GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                        }
                        else
                           GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                     }
                     else
                     {
                        /* Check to see if this is a write to the Access*/
                        /* Control Point.                               */
                        if(AttributeOffset == GLS_RECORDS_ACCESS_CONTROL_POINT_ATTRIBUTE_OFFSET)
                        {
                           /* Verify that we have an Authenticated Link.*/
                           if((!GAP_LE_Query_Encryption_Mode(BluetoothStackID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->RemoteDevice, &GAP_Encryption_Mode)) && (GAP_Encryption_Mode == emEnabled))
                           {
                              /* Format and Dispatch the event.         */
                              EventData = FormatEventHeader(sizeof(ServiceInstance->EventBuffer), ServiceInstance->EventBuffer, etGLS_Record_Access_Control_Point_Command, InstanceID, GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->ConnectionID, TransactionID);
                              if(EventData)
                              {
                                 /* Format the rest of the event.       */
                                 EventData->Event_Data_Size = GLS_RECORD_ACCESS_CONTROL_POINT_COMMAND_DATA_SIZE;

                                 /* Parse out the command.              */
                                 if(!DecodeRACPCommand(ValueLength, Value, &(EventData->Event_Data.GLS_Record_Access_Control_Point_Command_Data->FormatData)))
                                 {
                                    /* Add the information about the    */
                                    /* remote device.                   */
                                    EventData->Event_Data.GLS_Record_Access_Control_Point_Command_Data->ConnectionType = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->ConnectionType;
                                    EventData->Event_Data.GLS_Record_Access_Control_Point_Command_Data->RemoteDevice   = GATT_ServerEventData->Event_Data.GATT_Write_Request_Data->RemoteDevice;

                                    /* Dispatch the event.              */
                                    __BTPSTRY
                                    {
                                       (*ServiceInstance->EventCallback)(ServiceInstance->BluetoothStackID, EventData, ServiceInstance->CallbackParameter);
                                    }
                                    __BTPSEXCEPT(1)
                                    {
                                       /* Do Nothing.                   */
                                    }
                                 }
                                 else
                                    GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH);
                              }
                              else
                                 GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR);
                           }
                           else
                              GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION);
                        }
                        else
                           GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED);
                     }
                  }
                  else
                     GATT_Error_Response(BluetoothStackID, TransactionID, AttributeOffset, ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED);
               }
               break;
            case etGATT_Server_Confirmation_Response:
               /* Verify that the Event Data is valid.                  */
               if(GATT_ServerEventData->Event_Data.GATT_Confirmation_Data)
               {
                  /* Determine the type of the indication that has been */
                  /* confirmed.                                         */
                  if((ServiceInstance->RACPIndicationInfo.ConnectionID == GATT_ServerEventData->Event_Data.GATT_Confirmation_Data->ConnectionID) && ((unsigned int)ServiceInstance->RACPIndicationInfo.TransactionID == GATT_ServerEventData->Event_Data.GATT_Confirmation_Data->TransactionID))
                  {
                     ServiceInstance->RACPIndicationInfo.ConnectionID = 0;

                     EventData                                                   = (GLS_Event_Data_t *)ServiceInstance->EventBuffer;
                     EventData->Event_Data_Type                                  = etGLS_Confirmation_Data;

                     EventData->Event_Data.GLS_Confirmation_Data                 = (GLS_Confirmation_Data_t *)(((Byte_t *)EventData) + GLS_EVENT_DATA_SIZE);
                     EventData->Event_Data.GLS_Confirmation_Data->InstanceID     = ServiceInstance->ServiceID;
                     EventData->Event_Data.GLS_Confirmation_Data->ConnectionID   = GATT_ServerEventData->Event_Data.GATT_Confirmation_Data->ConnectionID;
                     EventData->Event_Data.GLS_Confirmation_Data->ConnectionType = GATT_ServerEventData->Event_Data.GATT_Confirmation_Data->ConnectionType;
                     EventData->Event_Data.GLS_Confirmation_Data->RemoteDevice   = GATT_ServerEventData->Event_Data.GATT_Confirmation_Data->RemoteDevice;
                     EventData->Event_Data.GLS_Confirmation_Data->Status         = GATT_ServerEventData->Event_Data.GATT_Confirmation_Data->Status;

                     /* Dispatch the event.                             */
                     __BTPSTRY
                     {
                        (*ServiceInstance->EventCallback)(ServiceInstance->BluetoothStackID, EventData, ServiceInstance->CallbackParameter);
                     }
                     __BTPSEXCEPT(1)
                     {
                        /* Do Nothing.                                  */
                     }
                  }
               }
               break;
            case etGATT_Server_Device_Disconnection:
               /* Verify that the Event Data is valid.                  */
               if(GATT_ServerEventData->Event_Data.GATT_Device_Disconnection_Data)
               {
                  /* Check for an outstanding RACP indication.          */
                  if(GATT_ServerEventData->Event_Data.GATT_Device_Disconnection_Data->ConnectionID == ServiceInstance->RACPIndicationInfo.ConnectionID)
                  {
                     ServiceInstance->RACPIndicationInfo.ConnectionID = 0;
                  }
               }
               break;
            default:
               /* Do Nothing.                                           */
               break;
         }

         /* UnLock the previously locked Bluetooth Stack.               */
         BSC_UnLockBluetoothStack(ServiceInstance->BluetoothStackID);
      }
   }
}

   /* The following function is responsible for making sure that the    */
   /* Bluetooth Stack GLS Module is Initialized correctly.  This        */
   /* function *MUST* be called before ANY other Bluetooth Stack GLS    */
   /* function can be called.  This function returns non-zero if the    */
   /* Module was initialized correctly, or a zero value if there was an */
   /* error.                                                            */
   /* * NOTE * Internally, this module will make sure that this function*/
   /*          has been called at least once so that the module will    */
   /*          function.  Calling this function from an external        */
   /*          location is not necessary.                               */
int InitializeGLSModule(void)
{
   return((int)InitializeModule());
}

   /* The following function is responsible for instructing the         */
   /* Bluetooth Stack GLSC Module to clean up any resources that it     */
   /* has allocated.  Once this function has completed, NO other        */
   /* Bluetooth Stack GLS Functions can be called until a successful    */
   /* call to the InitializeGLSModule() function is made.  The          */
   /* parameter to this function specifies the context in which this    */
   /* function is being called.  If the specified parameter is TRUE,    */
   /* then the module will make sure that NO functions that would       */
   /* require waiting/blocking on Mutexes/Events are called.  This      */
   /* parameter would be set to TRUE if this function was called in a   */
   /* context where threads would not be allowed to run.  If this       */
   /* function is called in the context where threads are allowed to run*/
   /* then this parameter should be set to FALSE.                       */
void CleanupGLSModule(Boolean_t ForceCleanup)
{
   /* Check to make sure that this module has been initialized.         */
   if(InstanceListInitialized)
   {
      /* Wait for access to the PASP Context List.                      */
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

   /* GLS Server API.                                                   */

   /* The following function is responsible for opening a GLS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered GLS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 GLS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
int BTPSAPI GLS_Initialize_Service(unsigned int BluetoothStackID, GLS_Event_Callback_t EventCallback, unsigned long CallbackParameter,  unsigned int *ServiceID)
{
   GATT_Attribute_Handle_Group_t ServiceHandleRange;

    /* Initialize the Service Handle Group to 0.                        */
   ServiceHandleRange.Starting_Handle = 0;
   ServiceHandleRange.Ending_Handle   = 0;

   return(GLSRegisterService(BluetoothStackID, EventCallback, CallbackParameter, ServiceID, &ServiceHandleRange));
}

   /* The following function is responsible for opening a GLS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The fourth parameter is a     */
   /* pointer to store the GATT Service ID of the registered GLS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 GLS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
int BTPSAPI GLS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, GLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange)
{
   return(GLSRegisterService(BluetoothStackID, EventCallback, CallbackParameter, ServiceID, ServiceHandleRange));
}

   /* The following function is responsible for closing a previously    */
   /* opened GLS Server.  The first parameter is the Bluetooth Stack    */
   /* ID on which to close the server.  The second parameter is the     */
   /* InstanceID that was returned from a successful call to            */
   /* GLS_Initialize_Service().  This function returns a zero if        */
   /* successful or a negative return error code if an error occurs.    */
int BTPSAPI GLS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID)
{
   int                  ret_val;
   GLSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (InstanceID))
   {
      /* Acquire the specified GLS Instance.                            */
      if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
      {
         /* Verify that the service is actually registered.             */
         if(ServiceInstance->ServiceID)
         {
            /* Call GATT to un-register the service.                    */
            GATT_Un_Register_Service(BluetoothStackID, ServiceInstance->ServiceID);

            /* mark the instance entry as being free.                   */
            BTPS_MemInitialize(ServiceInstance, 0, GLS_SERVER_INSTANCE_DATA_SIZE);

            /* return success to the caller.                            */
            ret_val = 0;
         }
         else
            ret_val = GLS_ERROR_INVALID_PARAMETER;

         /* Release the Lock on the Stack.                              */
         BSC_UnLockBluetoothStack(BluetoothStackID);
      }
      else
         ret_val = GLS_ERROR_INVALID_INSTANCE_ID;
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the GLS Service that is          */
   /* registered with a call to GLS_Initialize_Service().  This function*/
   /* returns the non-zero number of attributes that are contained in a */
   /* GLS Server or zero on failure.                                    */
unsigned int BTPSAPI GLS_Query_Number_Attributes(void)
{
   /* Simply return the number of attributes that are contained in a GLS*/
   /* service.                                                          */
   return(GLS_MONITOR_SERVICE_ATTRIBUTE_COUNT);
}

   /* The following function is responsible for setting the supported   */
   /* Glucose features on the specified GLS Instance.  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.      */
   /* The second parameter is the InstanceID returned from a successful */
   /* call to GLS_Initialize_Server().  The final parameter is the      */
   /* supported features to set for the specified GLS Instance.  This   */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * The SupportedFeatures parameter should be in range       */
   /* between GLS_FEATURE_LOW_BATTERY_DETECTION_DURING_MEASUREMENT to   */
   /* GLS_FEATURE_MULTIPLE_BOND_SUPPORT                                 */
int BTPSAPI GLS_Set_Glucose_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t SupportedFeatures)
{
   int                  ret_val;
   GLSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (InstanceID))
   {
      /* Acquire the specified GLS Instance.                            */
      if((ServiceInstance = AcquireServiceInstance(BluetoothStackID , &InstanceID)) != NULL)
      {
         /* Assign the Glucose Features to the specified instance.      */
          ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(InstanceData[InstanceID-1].Feature_Data), SupportedFeatures);

          /* Return success to the caller.                              */
         ret_val = 0;

         /* Release the Lock on the Stack.                              */
         BSC_UnLockBluetoothStack(ServiceInstance->BluetoothStackID);
      }
      else
         ret_val = GLS_ERROR_INVALID_INSTANCE_ID;
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for querying the current    */
   /* Glucose Features on the specified GLS Instance.  The first        */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to GLS_Initialize_Server().  The final parameter is a pointer to  */
   /* return the current Glucose Features for the specified GLS         */
   /* Instance.  This function returns a zero if successful or a        */
   /* negative return error code if an error occurs.                    */
int BTPSAPI GLS_Query_Glucose_Feature(unsigned int BluetoothStackID, unsigned int InstanceID, Word_t *SupportedFeatures)
{
   int ret_val;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (InstanceID) && (SupportedFeatures))
   {
      /* Acquire the specified GLS Instance.                            */
      if(AcquireServiceInstance(BluetoothStackID, &InstanceID))
      {
         /* Query the curent Glucose Features.                          */
         *SupportedFeatures = READ_UNALIGNED_WORD_LITTLE_ENDIAN(&(InstanceData[InstanceID-1].Feature_Data));

         /* Return success to the caller.                               */
         ret_val = 0;

         /* UnLock the previously locked Bluetooth Stack.               */
         BSC_UnLockBluetoothStack(BluetoothStackID);
      }
      else
         ret_val = GLS_ERROR_INVALID_INSTANCE_ID;
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for responding to a GLS Read*/
   /* Client Configuration Request.  The first parameter is the         */
   /* Bluetooth Stack ID of the Bluetooth Device.  The second parameter */
   /* is the InstanceID returned from a successful call to              */
   /* GLS_Initialize_Server().  The third is the Transaction ID of the  */
   /* request.  The final parameter contains the Client Configuration to*/
   /* send to the remote device.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
int BTPSAPI GLS_Read_Client_Configuration_Response(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int TransactionID, Word_t ClientConfiguration)
{
   int                  ret_val;
   NonAlignedWord_t     NonAlignedClientConfiguration;
   GLSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (InstanceID) && (TransactionID))
   {
      /* Acquire the specified GLS Instance.                            */
      if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
      {
         /* Format the Read Response.                                   */
         ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&NonAlignedClientConfiguration, ClientConfiguration);

         /* Send the response.                                          */
         ret_val = GATT_Read_Response(ServiceInstance->BluetoothStackID, TransactionID, GATT_CLIENT_CHARACTERISTIC_CONFIGURATION_LENGTH, (Byte_t *)&NonAlignedClientConfiguration);

         /* UnLock the previously locked Bluetooth Stack.               */
         BSC_UnLockBluetoothStack(BluetoothStackID);
      }
      else
         ret_val = GLS_ERROR_INVALID_INSTANCE_ID;
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible to responding to a Record   */
   /* Access Control Point Command received from a remote device.  The  */
   /* first parameter is the Bluetooth Stack ID of the Bluetooth Device.*/
   /* The second is the TransactionID that was received in the Record   */
   /* Access Control Point event.  The final parameter is an error code */
   /* that is used to determine if the Request is being accepted by the */
   /* server or if an error response should be issued instead.  This    */
   /* function returns a zero if successful or a negative return error  */
   /* code if an error occurs.                                          */
   /* * NOTE * If the ErrorCode parameter is set to 0x00 the Procedure  */
   /*          Request will be accepted.                                */
   /* * NOTE * If the ErrorCode is non-zero than an error response will */
   /*          be sent to the remote device.                            */
   /* * NOTE * This function is primarily provided to allow a way to    */
   /*          reject Record Access Control Point commands when the     */
   /*          Server has not been configured properly for RACP         */
   /*          operation, the Client does not have proper authentication*/
   /*          to write to the RACP characteristic or a RACP procedure  */
   /*          with the Client is already in progress.  All other       */
   /*          reasons should return ZERO for the ErrorCode and then    */
   /*          send RACP Result indication to indicate any other errors.*/
   /*          For Example: If the Operand in the Request is not        */
   /*          supported by the Server this API should be called with   */
   /*          ErrorCode set to ZERO and then the                       */
   /*          GLS_Indicate_Record_Access_Control_Point_Result() should */
   /*          be called with the ResponseCode set to                   */
   /*          GLS_RECORD_ACCESS_RESPONSE_CODE_OPERATOR_NOT_SUPPORTED.  */
int BTPSAPI GLS_Record_Access_Control_Point_Response(unsigned int BluetoothStackID, unsigned int TransactionID, Byte_t ErrorCode)
{
   int ret_val;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (TransactionID))
   {
      if(!ErrorCode)
         ret_val = GATT_Write_Response(BluetoothStackID, TransactionID);
      else
         ret_val = GATT_Error_Response(BluetoothStackID, TransactionID, GLS_RECORDS_ACCESS_CONTROL_POINT_ATTRIBUTE_OFFSET, ErrorCode);
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending an Glucose      */
   /* Measurement notification to a specified remote device.  The first */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to GLS_Initialize_Server().  The third parameter is the           */
   /* ConnectionID of the remote device to send the notification to.    */
   /* The final parameter is the Glucose Measurement Data strcuture that*/
   /* contains all of the required and optional data for the            */
   /* notification.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
int BTPSAPI GLS_Notify_Glucose_Measurement(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, GLS_Glucose_Measurement_Data_t *GlucoseMeasurement)
{
   int                  ret_val;
   GLSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (InstanceID) && (ConnectionID) && (GlucoseMeasurement))
   {
      /* Acquire the specified GLS Instance.                            */
      if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
      {
         /* Format the notification.                                    */
         ret_val = FormatGlucoseMeasurement(GlucoseMeasurement, (unsigned int)(sizeof(ServiceInstance->PacketBuffer)), ServiceInstance->PacketBuffer);
         if(ret_val > 0)
         {
            /* Attempt to send the notification.                        */
            ret_val = GATT_Handle_Value_Notification(ServiceInstance->BluetoothStackID, ServiceInstance->ServiceID, ConnectionID, GLS_MEASUREMENT_ATTRIBUTE_OFFSET, (Word_t)ret_val, ServiceInstance->PacketBuffer);
            if(ret_val > 0)
               ret_val = 0;
         }

         /* UnLock the previously locked Bluetooth Stack.               */
         BSC_UnLockBluetoothStack(BluetoothStackID);
      }
      else
         ret_val = GLS_ERROR_INVALID_INSTANCE_ID;
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending an Glucose      */
   /* Measurement Context notification to a specified remote device.    */
   /* The first parameter is the Bluetooth Stack ID of the Bluetooth    */
   /* Device. The second parameter is the InstanceID returned from a    */
   /* successful call to GLS_Initialize_Server().  The third parameter  */
   /* is the ConnectionID of the remote device to send the notification */
   /* to. The final parameter is the Glucose Context Data strcuture     */
   /* that contains all of the required and optional data for the       */
   /* notification.  This function returns a zero if successful or a    */
   /* negative return error code if an error occurs.                    */
int BTPSAPI GLS_Notify_Glucose_Measurement_Context(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, GLS_Glucose_Measurement_Context_Data_t *GlucoseContext)
{
#if BTPS_CONFIGURATION_GLS_SUPPORT_MEASUREMENT_CONTEXT

   int                  ret_val;
   GLSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (InstanceID) && (ConnectionID) && (GlucoseContext))
   {
      /* Acquire the specified GLS Instance.                            */
      if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
      {
         ret_val = FormatGlucoseMeasurementContext(GlucoseContext, (unsigned int)(sizeof(ServiceInstance->PacketBuffer)), ServiceInstance->PacketBuffer);
         if(ret_val > 0)
         {
            /* Attempt to send the notification.                        */
            ret_val = GATT_Handle_Value_Notification(ServiceInstance->BluetoothStackID, ServiceInstance->ServiceID, ConnectionID, GLS_MEASUREMENT_CONTEXT_ATTRIBUTE_OFFSET, (Word_t)ret_val, ServiceInstance->PacketBuffer);
            if(ret_val > 0)
               ret_val = 0;
         }

         /* UnLock the previously locked Bluetooth Stack.               */
         BSC_UnLockBluetoothStack(BluetoothStackID);
      }
      else
         ret_val = GLS_ERROR_INVALID_INSTANCE_ID;
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);

#else

   return(BTPS_ERROR_FEATURE_NOT_AVAILABLE);

#endif
}

   /* The following function is responsible for Number of Stored        */
   /* Records indication to a specified remote device.  The first       */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to GLS_Initialize_Server().  The third parameter is the           */
   /* ConnectionID of the remote device to send the notification to.    */
   /* The last parameter is number of stored records to be indicated.   */
   /* This function returns a zero if successful or a negative return   */
   /* error code if an error occurs.                                    */
   /* * NOTE * Only 1 Number of Stored Records indication may be        */
   /*          outstanding per GLS Instance.                            */
int BTPSAPI GLS_Indicate_Number_Of_Stored_Records(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, Word_t NumberOfStoredRecords)
{
   int                  ret_val;
   GLSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (InstanceID) && (ConnectionID))
   {
         /* Acquire the specified GLS Instance.                         */
         if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
         {
            /* Verify that no Indication is outstanding.                */
            if(!(ServiceInstance->RACPIndicationInfo.ConnectionID))
            {
               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)ServiceInstance->PacketBuffer)->Op_Code),          GLS_RECORD_ACCESS_OPCODE_NUM_STORED_RECORDS_RESPONSE);
               ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)ServiceInstance->PacketBuffer)->Operator),         GLS_RECORD_ACCESS_OPERATOR_NULL);
               ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Record_Access_Control_Point_t *)ServiceInstance->PacketBuffer)->Variable_Data[0]), NumberOfStoredRecords);

               /* Indicate the data.                                    */
               ret_val = GATT_Handle_Value_Indication(ServiceInstance->BluetoothStackID, ServiceInstance->ServiceID, ConnectionID, GLS_RECORDS_ACCESS_CONTROL_POINT_ATTRIBUTE_OFFSET, GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(sizeof(NonAlignedWord_t)), ServiceInstance->PacketBuffer);
               if(ret_val > 0)
               {
                  /* Save the Indication information.                   */
                  ServiceInstance->RACPIndicationInfo.ConnectionID  = ConnectionID;
                  ServiceInstance->RACPIndicationInfo.TransactionID = ret_val;

                  /* Set the return value to success.                   */
                  ret_val = 0;
               }
            }
            else
               ret_val = GLS_ERROR_INDICATION_OUTSTANDING;

            /* UnLock the previously locked Bluetooth Stack.            */
            BSC_UnLockBluetoothStack(BluetoothStackID);
         }
         else
            ret_val = GLS_ERROR_INVALID_INSTANCE_ID;
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for sending a Record Access */
   /* Control Point indication to a specified remote device.  The first */
   /* parameter is the Bluetooth Stack ID of the Bluetooth Device.  The */
   /* second parameter is the InstanceID returned from a successful call*/
   /* to GLS_Initialize_Server().  The third parameter the ConnectionID */
   /* of the remote device to send the indication to.  The fourth       */
   /* parameter is the Request data to indicate.  The last parameter is */
   /* response code.  This function returns a zero if successful or a   */
   /* negative return error code if an error occurs.                    */
   /* * NOTE * Only 1 RACP Request indication may be outstanding per GLS*/
   /*          Instance.                                                */
int BTPSAPI GLS_Indicate_Record_Access_Control_Point_Result(unsigned int BluetoothStackID, unsigned int InstanceID, unsigned int ConnectionID, GLS_RACP_Command_Type_t CommandType, Byte_t ResponseCode)
{
   int                  ret_val;
   GLSServerInstance_t *ServiceInstance;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((BluetoothStackID) && (InstanceID) && (ConnectionID) && (CommandType <= GLS_RECORD_ACCESS_OPCODE_RFU_255))
   {
      /* Acquire the specified GLS Instance.                            */
      if((ServiceInstance = AcquireServiceInstance(BluetoothStackID, &InstanceID)) != NULL)
      {
         /* Verify that no Indication is outstanding.                   */
        if(!(ServiceInstance->RACPIndicationInfo.ConnectionID))
         {
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)ServiceInstance->PacketBuffer)->Op_Code),          GLS_RECORD_ACCESS_OPCODE_RESPONSE_CODE);
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)ServiceInstance->PacketBuffer)->Operator),         GLS_RECORD_ACCESS_OPERATOR_NULL);
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)ServiceInstance->PacketBuffer)->Variable_Data[0]), CommandType);
            ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)ServiceInstance->PacketBuffer)->Variable_Data[1]), ResponseCode);

            /* Attempt to send the notification.                        */
            ret_val = GATT_Handle_Value_Indication(ServiceInstance->BluetoothStackID, ServiceInstance->ServiceID, ConnectionID, GLS_RECORDS_ACCESS_CONTROL_POINT_ATTRIBUTE_OFFSET, GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(sizeof(NonAlignedByte_t) * 2), ServiceInstance->PacketBuffer);
            if(ret_val > 0)
            {
               /* Save the Indication information.                      */
               ServiceInstance->RACPIndicationInfo.ConnectionID  = ConnectionID;
               ServiceInstance->RACPIndicationInfo.TransactionID = ret_val;

               /* Set the return value to success.                      */
               ret_val = 0;
            }
         }
         else
            ret_val = GLS_ERROR_INDICATION_OUTSTANDING;

         /* UnLock the previously locked Bluetooth Stack.               */
         BSC_UnLockBluetoothStack(BluetoothStackID);
      }
      else
         ret_val = GLS_ERROR_INVALID_INSTANCE_ID;
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* GLS Client API.                                                   */

   /* The following function is responsible for parsing a value received*/
   /* from a remote GLS Server interpreting it as a Glucose             */
   /* Measurement characteristic.  The first parameter is the length of */
   /* the value returned by the remote GLS Server.  The second          */
   /* parameter is a pointer to the data returned by the remote GLS     */
   /* Server.  The final parameter is a pointer to store the parsed     */
   /* Glucose Measurement value.  This function returns a zero if       */
   /* successful or a negative return error code if an error occurs.    */
int BTPSAPI GLS_Decode_Glucose_Measurement(unsigned int ValueLength, Byte_t *Value, GLS_Glucose_Measurement_Data_t *GlucoseMeasurement)
{
   int ret_val;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((ValueLength) && (Value) && (GlucoseMeasurement))
      ret_val = DecodeGlucoseMeasurement(ValueLength, Value, GlucoseMeasurement);
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for parsing a value received*/
   /* from a remote GLS Server interpreting it as a Glucose Context     */
   /* characteristic.  The first parameter is the length of the value   */
   /* returned by the remote GLS Server.  The second parameter is a     */
   /* pointer to the data returned by the remote GLS Server.  The       */
   /* final parameter is a pointer to store the parsed Glucose Context  */
   /* value.  This function returns a zero if successful or a negative  */
   /* return error code if an error occurs.                             */
int BTPSAPI GLS_Decode_Glucose_Measurement_Context(unsigned int ValueLength, Byte_t *Value, GLS_Glucose_Measurement_Context_Data_t *GlucoseContext)
{
#if BTPS_CONFIGURATION_GLS_SUPPORT_MEASUREMENT_CONTEXT

   int ret_val;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((ValueLength) && (Value) && (GlucoseContext))
      ret_val = DecodeGlucoseMeasurementContext(ValueLength, Value, GlucoseContext);
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);

#else

   return(BTPS_ERROR_FEATURE_NOT_AVAILABLE);

#endif
}

   /* The following function is responsible for parsing a value received*/
   /* from a remote GLS Server interpreting it as a response code of    */
   /* record access control point.The first parameter is the length of  */
   /* the value returned by the remote GLS Server.The second parameter  */
   /* is a pointer to the data returned by the remote GLS Server.The    */
   /* final parameter is a pointer to store the parsed Record Access    */
   /* Control Point Response data value.This function returns a zero if */
   /* successful or a negative return error code if an error occurs.    */
int BTPSAPI GLS_Decode_Record_Access_Control_Point_Response(unsigned int ValueLength, Byte_t *Value, GLS_Record_Access_Control_Point_Response_Data_t *RACPData)
{
   int ret_val;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((ValueLength) && (Value) && (RACPData))
   {
      ret_val = DecodeRACPResponse(ValueLength, Value, RACPData);
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for formatting a Record     */
   /* Access Control Point Command into a user specified buffer.  The   */
   /* first parameter is the input command to format.  The second       */
   /* parameter is size of the input Record Access Control Point Request*/
   /* Data.  The final parameter is the output that will contain data in*/
   /* Buffer after formatting.  This function returns a zero if         */
   /* successful or a negative return error code if an error occurs.    */
   /* * NOTE * The third parameter BufferLength is the size of input    */
   /*          request and the same will hold the size of output Buffer */
   /*          after formatting.                                        */
int BTPSAPI GLS_Format_Record_Access_Control_Point_Command(GLS_Record_Access_Control_Point_Format_Data_t *FormatData, unsigned int *BufferLength, Byte_t *Buffer)
{
   int ret_val = 0;

   /* Make sure the parameters passed to us are semi-valid.             */
   if((FormatData) && (*BufferLength >= GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(0)) && (Buffer))
   {
      /* Assign the command type and operator type for this RACP packet.*/
      /* Validity of the data will be checked later.                    */
      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Op_Code),  (FormatData->CommandType));
      ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Operator), (FormatData->OperatorType));

      /* Check if a filter is expected.                                 */
      if((FormatData->CommandType == racReportStoredRecordsRequest) || (FormatData->CommandType == racDeleteStoredRecordsRequest) || (FormatData->CommandType == racNumberOfStoredRecordsRequest))
      {
         /* Check if a filter type with one parameter is expected.      */
         if((FormatData->OperatorType == raoLessThanOrEqualTo) || (FormatData->OperatorType == raoGreaterThanOrEqualTo))
         {
            /* A filter type with one parameter is expected, format the */
            /* packet based on the filter type.                         */
            if(FormatData->FilterType == rafSequenceNumber)
            {
               /* Validate buffer length.                               */
               if(*BufferLength >= ((unsigned int)GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(sizeof(NonAlignedWord_t) + sizeof(NonAlignedByte_t))))
               {
                  /* Assign the filter type and parameter.              */
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[0]), (FormatData->FilterType));
                  ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1]), (FormatData->FilterParameters.SequenceNumber));

                  /* Set the number of bytes used in the buffer.        */
                  *BufferLength = GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(sizeof(NonAlignedWord_t) + sizeof(NonAlignedByte_t));
               }
               else
               {
                  ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
               }
            }
            else
            {
               if(FormatData->FilterType == rafUserFacingTime)
               {
                  /* Validate buffer length.                            */
                  if(*BufferLength >= ((unsigned int)GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE + sizeof(NonAlignedByte_t))))
                  {
                     /* Assign the filter type and parameter.           */
                     ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[0]), (FormatData->FilterType));

                     ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Year),    (FormatData->FilterParameters.UserFacingTime.Year));
                     ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Month),   (FormatData->FilterParameters.UserFacingTime.Month));
                     ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Day),     (FormatData->FilterParameters.UserFacingTime.Day));
                     ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Hours) ,  (FormatData->FilterParameters.UserFacingTime.Hours));
                     ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minutes), (FormatData->FilterParameters.UserFacingTime.Minutes));
                     ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GATT_Date_Time_Characteristic_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Seconds), (FormatData->FilterParameters.UserFacingTime.Seconds));

                     /* Set the number of bytes used in the buffer.     */
                     *BufferLength = GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE + sizeof(NonAlignedByte_t));
                  }
                  else
                  {
                     ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
                  }
               }
               else
               {
                  /* It is a Reserved FilterType                        */
                  ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[0]), (FormatData->FilterType));
                  ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1]), (0));

                  /* Set the number of bytes used in the buffer.        */
                  *BufferLength = GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(sizeof(NonAlignedWord_t) + sizeof(NonAlignedByte_t));
               }
            }
         }
         else
         {
            /* Check if a filter type with two parameters is expected.  */
            if(FormatData->OperatorType == raoWithinRangeOf)
            {
               /* A filter type with two parameters is expected, format */
               /* the packet based on the filter type.                  */
               if(FormatData->FilterType == rafSequenceNumber)
               {
                  /* Validate buffer length.                            */
                  if(*BufferLength >= ((unsigned int)GLS_RECORD_ACCESS_CONTROL_POINT_SIZE((sizeof(NonAlignedWord_t) * 2) + sizeof(NonAlignedByte_t))))
                  {
                     /* Assign the filter type and parameters.          */
                     ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[0]), (FormatData->FilterType));
                     ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1]), (FormatData->FilterParameters.SequenceNumberRange.Minimum));
                     ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[3]), (FormatData->FilterParameters.SequenceNumberRange.Maximum));

                     /* Set the number of bytes used in the buffer.     */
                     *BufferLength = GLS_RECORD_ACCESS_CONTROL_POINT_SIZE((sizeof(NonAlignedWord_t) * 2) + sizeof(NonAlignedByte_t));
                  }
                  else
                  {
                     ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
                  }
               }
               else
               {
                  if(FormatData->FilterType == rafUserFacingTime)
                  {
                     /* Validate buffer length.                         */
                     if(*BufferLength >= ((unsigned int)GLS_RECORD_ACCESS_CONTROL_POINT_SIZE((GATT_DATE_TIME_CHARACTERISTIC_DATA_SIZE * 2) + sizeof(NonAlignedByte_t))))
                     {
                        /* Assign the filter type and parameters.       */
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[0]), (FormatData->FilterType));

                        ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Year),    (FormatData->FilterParameters.UserFacingTimeRange.Minimum.Year));
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Month),   (FormatData->FilterParameters.UserFacingTimeRange.Minimum.Month));
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Day),     (FormatData->FilterParameters.UserFacingTimeRange.Minimum.Day));
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Hours) ,  (FormatData->FilterParameters.UserFacingTimeRange.Minimum.Hours));
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Minutes), (FormatData->FilterParameters.UserFacingTimeRange.Minimum.Minutes));
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Minimum_Value.Seconds), (FormatData->FilterParameters.UserFacingTimeRange.Minimum.Seconds));

                        ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Year),    (FormatData->FilterParameters.UserFacingTimeRange.Maximum.Year));
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Month),   (FormatData->FilterParameters.UserFacingTimeRange.Maximum.Month));
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Day),     (FormatData->FilterParameters.UserFacingTimeRange.Maximum.Day));
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Hours) ,  (FormatData->FilterParameters.UserFacingTimeRange.Maximum.Hours));
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Minutes), (FormatData->FilterParameters.UserFacingTimeRange.Maximum.Minutes));
                        ASSIGN_HOST_BYTE_TO_LITTLE_ENDIAN_UNALIGNED_BYTE(&(((GLS_Date_Time_Range_t *)&((GLS_Record_Access_Control_Point_t *)Buffer)->Variable_Data[1])->Maximum_Value.Seconds), (FormatData->FilterParameters.UserFacingTimeRange.Maximum.Seconds));

                        /* Set the number of bytes used in the buffer.  */
                        *BufferLength = GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(GLS_DATE_TIME_RANGE_SIZE + sizeof(NonAlignedByte_t));
                     }
                     else
                     {
                        ret_val = GLS_ERROR_INSUFFICIENT_BUFFER_SPACE;
                     }
                  }
                  else
                  {
                     /* Filter type not allowed for this command /      */
                     /* operator.                                       */
                     ret_val = GLS_ERROR_INVALID_PARAMETER;
                  }
               }
            }
            else
            {
               /* Either it is a Reserved Operator or other valid       */
               /* Operator.                                             */
               /* Set the number of bytes used in the buffer.           */
               *BufferLength = GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(0);
            }
         }
      }
      else
      {
         /* Validate the command and operator types.                    */
         if((FormatData->CommandType == racAbortOperationRequest) && (FormatData->OperatorType == raoNull))
         {
            /* Set the number of bytes used in the buffer.              */
            *BufferLength = GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(0);
         }
         else
         {
            if(((FormatData->CommandType == GLS_RECORD_ACCESS_OPCODE_RFU_0) || (FormatData->CommandType >= GLS_RECORD_ACCESS_OPCODE_RFU_7 && FormatData->CommandType <= GLS_RECORD_ACCESS_OPCODE_RFU_255)) && (FormatData->OperatorType == raoAllRecords))
            {
               /* Set the number of bytes used in the buffer.           */
               *BufferLength = GLS_RECORD_ACCESS_CONTROL_POINT_SIZE(0);
            }
            else
            {
               /* Command / Operator type not allowed.                  */
               ret_val = GLS_ERROR_INVALID_PARAMETER;
            }
         }
      }
   }
   else
      ret_val = GLS_ERROR_INVALID_PARAMETER;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

