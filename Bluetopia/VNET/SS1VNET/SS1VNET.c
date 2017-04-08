/*****< ss1vnet.c >************************************************************/
/*      Copyright 2003 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SS1VNET - Stonestreet One Virtual Network Driver Layer Library for Linux. */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/24/03  D. Lange       Initial creation.                               */
/******************************************************************************/
#include "SS1VNET.h"            /* Main VNET Driver DLL Prototypes/Constants. */

   /* The following function is responsible for changing the current    */
   /* Ethernet Address that is being used by the specified Virtual      */
   /* Network Driver.  The first parameter to this function specifies   */
   /* the Virtual Network Driver Index of the Actual Virtual Network    */
   /* Driver in the system.  The second parameter specifies the actual  */
   /* Ethernet Address to use for the specified Driver (based on the    */
   /* first parameter).  This function returns the following status     */
   /* information:                                                      */
   /*    - Zero (No change - already configured with specified address).*/
   /*    - Positive (Changed - previously configured with a different   */
   /*      address).                                                    */
   /*    - Negative return error code (error).                          */
int BTPSAPI VNET_Configure_Ethernet_Address(unsigned int VNETIndex, VNET_Ethernet_Address_t EthernetAddress)
{
   int                     ret_val;
   VNET_Ethernet_Address_t NULL_EthernetAddress;

   /* Assign an invalid Ethernet address to make sure that the passed in*/
   /* address is semi-valid.                                            */
   VNET_ASSIGN_ETHERNET_ADDRESS(NULL_EthernetAddress, 0, 0, 0, 0, 0, 0);

   /* Make sure that the passed in parameter seems semi-valid.          */
   if(!VNET_COMPARE_ADDRESSES(NULL_EthernetAddress, EthernetAddress))
   {

//xxx Port to platform
         ret_val = 0;

   }
   else
      ret_val = VNET_DRIVER_ERROR_INVALID_PARAMETER;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for Opening a virtual       */
   /* Network Driver with the specified Index.  The first parameter to  */
   /* this function specifies the Virtual Network Driver Index of the   */
   /* Actual Virtual Network Driver in the system.  The second and third*/
   /* parameters specify the VNET Driver Callback function that is to be*/
   /* called when a VNET Event Occurs.  All parameters to this function */
   /* *MUST* be specified.  If this function is successful, the caller  */
   /* will receive a non-zero, non-negative return value which serves as*/
   /* the VNETDriverID parameter for all other functions in the VNET    */
   /* Driver.  Since all VNET functions require a valid VNET Driver ID, */
   /* this function must be called successfully before any other        */
   /* function can be called.  If this function fails then the return   */
   /* value is a negative error code (see error codes above).           */
   /* * NOTE * If this function call is successful, no Ethernet Packets */
   /*          can be sent or received, until the Ethernet Connected    */
   /*          State is set to connected.                               */
   /*          The VNET_Set_Ethernet_Connected_State() function is      */
   /*          used for this purpose.  In other words, the default      */
   /*          Ethernet Connected State is disconnected (i.e. no        */
   /*          Ethernet Packets can be sent/received).                  */
int BTPSAPI VNET_Open_Driver(unsigned int VNETIndex, VNET_Driver_Callback_t VNETCallback, unsigned long CallbackParameter)
{
   int ret_val;

   /* Make sure that the passed in parameter seems semi-valid.          */
   if(VNETCallback)
   {

//xxx Port to platform (Return VNET Driver ID)
      ret_val = 1;

   }
   else
      ret_val = VNET_DRIVER_ERROR_INVALID_CALLBACK_INFORMATION;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for Closing the Virtual     */
   /* Network Driver that was opened via a successful call to the       */
   /* VNET_Open_Driver() function.  The Input parameter to this function*/
   /* MUST have been acquired by a successful call to                   */
   /* VNET_Open_Driver().  Once this function completes, the Virtual    */
   /* Network Driver that was closed cannot be accessed again           */
   /* (sending/receiving data) by this module until the Driver is       */
   /* Re-Opened by calling the VNET_Open_Driver() function.             */
void BTPSAPI VNET_Close_Driver(unsigned int VNETDriverID)
{
   /* Make sure that the passed in parameter seems semi-valid.          */
   if(VNETDriverID)
   {

//xxx Port to platform

   }
}

   /* The following function is responsible for querying the current    */
   /* Ethernet Address that is being used by the specified Virtual      */
   /* Network Driver.  The VNETDriverID parameter that is passed to this*/
   /* function MUST have been established via a successful call to the  */
   /* VNET_Open_Driver() function.  The final parameter to this function*/
   /* is a pointer to a buffer that will hold the current Ethernet      */
   /* Address of the Virtual Network Driver (if this function is        */
   /* successful).  This function returns zero if successful or a       */
   /* negative return error code if there was an error.  If this        */
   /* function is successful then the buffer pointed to by the          */
   /* EthernetAddress parameter will contain the currently opened       */
   /* Virtual Network Driver.  If this function is unsuccessful then the*/
   /* contents of the EthernetAddress parameter buffer will be          */
   /* undefined.                                                        */
int BTPSAPI VNET_Query_Ethernet_Address(unsigned int VNETDriverID, VNET_Ethernet_Address_t *EthernetAddress)
{
   int ret_val;

   /* Make sure that the passed in parameter seems semi-valid.          */
   if(VNETDriverID)
   {
      if(EthernetAddress)
      {

//xxx Port to platform
         ret_val = 0;

      }
      else
         ret_val = VNET_DRIVER_ERROR_INVALID_PARAMETER;
   }
   else
      ret_val = VNET_DRIVER_ERROR_INITIALIZING_DRIVER;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for specifying the current  */
   /* Ethernet Connected State that is to be used by the specified      */
   /* Virtual Network Driver.  The VNETDriverID parameter that is passed*/
   /* to this function MUST have been established via a successful call */
   /* to the VNET_Open_Driver() function.  The final parameter to this  */
   /* function is a BOOLEAN parameter that specifies whether or not an  */
   /* Ethernet Cable is connected (TRUE) or disconnected (FALSE).  This */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
   /* * NOTE * This function has to be called with the second parameter */
   /*          set to TRUE (connected) before ANY Ethernet Packets can  */
   /*          be sent or received.                                     */
   /* * NOTE * When the Ethernet Connected State is disconnected, no    */
   /*          Ethenet Packets can be sent or received.                 */
int BTPSAPI VNET_Set_Ethernet_Connected_State(unsigned int VNETDriverID, Boolean_t Connected)
{
   int ret_val;

   /* Make sure that the passed in parameter seems semi-valid.          */
   if(VNETDriverID)
   {

//xxx Port to platform
      ret_val = 0;

   }
   else
      ret_val = VNET_DRIVER_ERROR_INITIALIZING_DRIVER;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

   /* The following function is responsible for sending an Ethernet     */
   /* Packet to the specified Virtual Network Driver.  The VNETDriverID */
   /* parameter that is passed to this function MUST have been          */
   /* established via a successful call to the VNET_Open_Driver()       */
   /* function.  The remaining parameters to this function are the      */
   /* Ethernet Packet Header to use for the packet, followed by the     */
   /* Length of the Data to send and a pointer to the Data Buffer to    */
   /* Send (Payload only).  This function returns zero if successful,   */
   /* or a negative return error code if unsuccessful.                  */
   /* * NOTE * If the Ethernet Connected State is disconnected, then    */
   /*          no Ethernet packets will be passed through the Virtual   */
   /*          Network Driver.  The Ethernet connected state is set via */
   /*          a call to the VNET_Set_Ethernet_Connected_State()        */
   /*          function.                                                */
   /* * NOTE * The default state of the Ethernet Connected State is     */
   /*          disconnected.                                            */
int BTPSAPI VNET_Write_Ethernet_Packet(unsigned int VNETDriverID, VNET_Ethernet_Header_t *EthernetHeader, unsigned int PayloadLength, unsigned char *PayloadBuffer)
{
   int ret_val;

   /* Make sure that the passed in parameter seems semi-valid.          */
   if(VNETDriverID)
   {
      if((EthernetHeader) && (PayloadLength) && (PayloadBuffer))
      {

//xxx Port to platform
         ret_val = 0;

      }
      else
         ret_val = VNET_DRIVER_ERROR_INVALID_PARAMETER;
   }
   else
      ret_val = VNET_DRIVER_ERROR_INITIALIZING_DRIVER;

   /* Return the result to the caller.                                  */
   return(ret_val);
}

