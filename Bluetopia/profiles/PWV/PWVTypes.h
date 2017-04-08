/*****< PWVtyp.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*      Copyright 2015 Texas Instruments Incorporated.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  PWVTYP - Embedded Bluetooth SPP Emulation using GATT (LE) Types File.   */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   04/16/12  Tim Cook       Initial creation.                               */
/******************************************************************************/
#ifndef __PWVTYP_H__
#define __PWVTYP_H__

   /* The following MACRO is a utility MACRO that assigns the PWV     */
   /* Service 16 bit UUID to the specified UUID_128_t variable.  This   */
   /* MACRO accepts one parameter which is a pointer to a UUID_128_t    */
   /* variable that is to receive the PWV UUID Constant value.        */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define PWV_ASSIGN_PWV_SERVICE_UUID_128(_x)                 ASSIGN_BLUETOOTH_UUID_128(*((UUID_128_t *)(_x)), 0x42, 0x3A, 0xD8, 0x7A, 0xB1, 0x00, 0x4F, 0x14, 0x9E, 0xAA, 0x5E, 0xB5, 0x83, 0x9F, 0x2A, 0x54)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined PWV Service UUID in UUID16 form.  This   */
   /* MACRO only returns whether the UUID_128_t variable is equal to the*/
   /* PWV Service UUID (MACRO returns boolean result) NOT less        */
   /* than/greater than.  The first parameter is the UUID_128_t variable*/
   /* to compare to the PWV Service UUID.                             */
#define PWV_COMPARE_PWV_SERVICE_UUID_TO_UUID_128(_x)        COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x42, 0x3A, 0xD8, 0x7A, 0xB1, 0x00, 0x4F, 0x14, 0x9E, 0xAA, 0x5E, 0xB5, 0x83, 0x9F, 0x2A, 0x54)

   /* The following defines the PWV Service UUID that is used when    */
   /* building the PWV Service Table. Stored in reverse format        */
#define PWV_SERVICE_BLUETOOTH_UUID_CONSTANT            { 0x54, 0x2A, 0x9F, 0x83, 0xB5, 0x5E, 0xAA, 0x9E, 0x14, 0x4F, 0x00, 0xB1, 0x7A, 0xD8, 0x3A, 0x42 }

   /* The following MACRO is a utility MACRO that assigns the PWV TX  */
   /* Characteristic 16 bit UUID to the specified UUID_128_t variable.  */
   /* This MACRO accepts one parameter which is the UUID_128_t variable */
   /* that is to receive the PWV TX UUID Constant value.              */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define PWV_ASSIGN_TX_UUID_128(_x)                            ASSIGN_BLUETOOTH_UUID_128((_x), 0x07, 0x34, 0x59, 0x4A, 0xA8, 0xE7, 0x4b, 0x1a, 0xA6, 0xB1, 0xCD, 0x52, 0x43, 0x05, 0x9A, 0x57)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined PWV TX UUID in UUID16 form.  This MACRO  */
   /* only returns whether the UUID_128_t variable is equal to the TX   */
   /* UUID (MACRO returns boolean result) NOT less than/greater than.   */
   /* The first parameter is the UUID_128_t variable to compare to the  */
   /* PWV TX UUID.                                                    */
#define PWV_COMPARE_PWV_TX_UUID_TO_UUID_128(_x)             COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x07, 0x34, 0x59, 0x4A, 0xA8, 0xE7, 0x4b, 0x1a, 0xA6, 0xB1, 0xCD, 0x52, 0x43, 0x05, 0x9A, 0x57)

   /* The following defines the PWV TX Characteristic UUID that is    */
   /* used when building the PWV Service Table.                       */
#define PWV_TX_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x57, 0x9A, 0x05, 0x43, 0x52, 0xCD, 0xB1, 0xA6, 0x1a, 0x4b, 0xE7, 0xA8, 0x4A, 0x59, 0x34, 0x07 }

   /* The following MACRO is a utility MACRO that assigns the PWV     */
   /* TX_CREDITS Characteristic 16 bit UUID to the specified UUID_128_t */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the PWV TX_CREDITS UUID  */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define PWV_ASSIGN_TX_CREDITS_UUID_128(_x)                    ASSIGN_BLUETOOTH_UUID_128((_x), 0xBA, 0x04, 0xC4, 0xB2, 0x89, 0x2B, 0x43, 0xbe, 0xB6, 0x9C, 0x5D, 0x13, 0xF2, 0x19, 0x53, 0x92)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined PWV TX_CREDITS UUID in UUID16 form.  This*/
   /* MACRO only returns whether the UUID_128_t variable is equal to the*/
   /* TX_CREDITS UUID (MACRO returns boolean result) NOT less           */
   /* than/greater than.  The first parameter is the UUID_128_t variable*/
   /* to compare to the PWV TX_CREDITS UUID.                          */
#define PWV_COMPARE_PWV_TX_CREDITS_UUID_TO_UUID_128(_x)     COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0xBA, 0x04, 0xC4, 0xB2, 0x89, 0x2B, 0x43, 0xbe, 0xB6, 0x9C, 0x5D, 0x13, 0xF2, 0x19, 0x53, 0x92)

   /* The following defines the PWV TX_CREDITS Characteristic UUID    */
   /* that is used when building the PWV Service Table.               */
#define PWV_TX_CREDITS_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x92, 0x53, 0x19, 0xF2, 0x13, 0x5D, 0x9C, 0xB6, 0xbe, 0x43, 0x2B, 0x89, 0xB2, 0xC4, 0x04, 0xBA }

   /* The following MACRO is a utility MACRO that assigns the PWV RX  */
   /* Characteristic 16 bit UUID to the specified UUID_128_t variable.  */
   /* This MACRO accepts one parameter which is the UUID_128_t variable */
   /* that is to receive the PWV RX UUID Constant value.              */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define PWV_ASSIGN_RX_UUID_128(_x)                            ASSIGN_BLUETOOTH_UUID_128((_x), 0x8B, 0x00, 0xAC, 0xE7, 0xEB, 0x0B, 0x49, 0xb0, 0xBB, 0xE9, 0x9A, 0xEE, 0x0A, 0x26, 0xE1, 0xA3)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined PWV RX UUID in UUID16 form.  This MACRO  */
   /* only returns whether the UUID_128_t variable is equal to the RX   */
   /* UUID (MACRO returns boolean result) NOT less than/greater than.   */
   /* The first parameter is the UUID_128_t variable to compare to the  */
   /* PWV RX UUID.                                                    */
#define PWV_COMPARE_PWV_RX_UUID_TO_UUID_128(_x)             COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0x8B, 0x00, 0xAC, 0xE7, 0xEB, 0x0B, 0x49, 0xb0, 0xBB, 0xE9, 0x9A, 0xEE, 0x0A, 0x26, 0xE1, 0xA3)

   /* The following defines the PWV RX Characteristic UUID that is    */
   /* used when building the PWV Service Table.                       */
#define PWV_RX_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT  { 0xA3, 0xE1, 0x26, 0x0A, 0xEE, 0x9A, 0xE9, 0xBB, 0xb0, 0x49, 0x0B, 0xEB, 0xE7, 0xAC, 0x00, 0x8B }

   /* The following MACRO is a utility MACRO that assigns the PWV     */
   /* RX_CREDITS Characteristic 16 bit UUID to the specified UUID_128_t */
   /* variable.  This MACRO accepts one parameter which is the          */
   /* UUID_128_t variable that is to receive the PWV RX_CREDITS UUID  */
   /* Constant value.                                                   */
   /* * NOTE * The UUID will be assigned into the UUID_128_t variable in*/
   /*          Little-Endian format.                                    */
#define PWV_ASSIGN_RX_CREDITS_UUID_128(_x)                    ASSIGN_BLUETOOTH_UUID_128((_x), 0xE0, 0x6D, 0x5E, 0xFB, 0x4F, 0x4A, 0x45, 0xc0, 0x9E, 0xB1, 0x37, 0x1A, 0xE5, 0xA1, 0x4A, 0xD4)

   /* The following MACRO is a utility MACRO that exist to compare a    */
   /* UUID 16 to the defined PWV RX_CREDITS UUID in UUID16 form.  This*/
   /* MACRO only returns whether the UUID_128_t variable is equal to the*/
   /* RX_CREDITS UUID (MACRO returns boolean result) NOT less           */
   /* than/greater than.  The first parameter is the UUID_128_t variable*/
   /* to compare to the PWV RX_CREDITS UUID.                          */
#define PWV_COMPARE_PWV_RX_CREDITS_UUID_TO_UUID_128(_x)     COMPARE_BLUETOOTH_UUID_128_TO_CONSTANT((_x), 0xE0, 0x6D, 0x5E, 0xFB, 0x4F, 0x4A, 0x45, 0xc0, 0x9E, 0xB1, 0x37, 0x1A, 0xE5, 0xA1, 0x4A, 0xD4)

   /* The following defines the PWV Control Point Characteristic UUID    */
   /* that is used when building the PWV Service Table.                  */
#define PWV_CONTROL_POINT_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x54, 0x2A, 0x9F, 0x83, 0xB5, 0x5E, 0xAA, 0x9E, 0x14, 0x4F, 0x01, 0x00, 0x7A, 0xD8, 0x3A, 0x42 }

   /* The following defines the PWV File Write Characteristic UUID       */
   /* that is used when building the PWV Service Table.                  */
#define PWV_FILE_WRITE_CHARACTERISTIC_BLUETOOTH_UUID_CONSTANT { 0x54, 0x2A, 0x9F, 0x83, 0xB5, 0x5E, 0xAA, 0x9E, 0x14, 0x4F, 0x02, 0x00, 0x7A, 0xD8, 0x3A, 0x42 }

   /* The following defines the structure that holds all of the PWV   */
   /* Characteristic Handles that need to be cached by a PWV Client.  */
typedef struct _tagPWV_Client_Info_t
{
   Word_t Tx_Characteristic;
   Word_t Tx_Client_Configuration_Descriptor;
   Word_t Rx_Characteristic;
   Word_t Tx_Credit_Characteristic;
   Word_t Rx_Credit_Characteristic;
   Word_t Rx_Credit_Client_Configuration_Descriptor;
} PWV_Client_Info_t;

#define PWV_CLIENT_INFO_DATA_SIZE                      (sizeof(PWV_Client_Info_t))

#define PWV_CLIENT_INFORMATION_VALID(_x)               (((_x).Tx_Characteristic) && ((_x).Tx_Client_Configuration_Descriptor) && ((_x).Rx_Characteristic) && ((_x).Tx_Credit_Characteristic) && ((_x).Rx_Credit_Characteristic) && ((_x).Rx_Credit_Client_Configuration_Descriptor))

   /* The following defines the structure that holds the information    */
   /* that needs to be cached by a PWV Server for EACH paired PWV   */
   /* Client.                                                           */
typedef struct _tagPWV_Server_Info_t
{
   Word_t Control_Point_Client_Configuration_Descriptor;
   Word_t Rx_Credit_Client_Configuration_Descriptor;
} PWV_Server_Info_t;

#define PWV_SERVER_INFO_DATA_SIZE                      (sizeof(PWV_Server_Info_t))

   /* The following defines the length of the PWV CTS characteristic  */
   /* value.                                                            */
#define PWV_TX_CREDIT_VALUE_LENGTH                     (WORD_SIZE)

   /* The following defines the length of the PWV RTS characteristic  */
   /* value.                                                            */
#define PWV_RX_CREDIT_VALUE_LENGTH                     (WORD_SIZE)

   /* The following defines the length of the Client Characteristic     */
   /* Configuration Descriptor.                                         */
#define PWV_CLIENT_CHARACTERISTIC_CONFIGURATION_VALUE_LENGTH (WORD_SIZE)

   /* The following defines the PWV GATT Service Flags MASK that      */
   /* should be passed into GATT_Register_Service when the HRS Service  */
   /* is registered.                                                    */
#define PWV_SERVICE_FLAGS                              (GATT_SERVICE_FLAGS_LE_SERVICE)

#endif

