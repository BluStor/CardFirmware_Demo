/*****< hcitrcfg.h >***********************************************************/
/*      Copyright 2012 - 2013 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*      Copyright 2015 Texas Instruments Incorporated.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  HCITRCFG - HCI Transport Layer Configuration parameters.                  */
/*                                                                            */
/*  Author:  Marcus Funk                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   11/08/12  M. Funk        Initial creation.                               */
/*   11/02/15  D. Keren       Changing nShutdown pin from PC9 to PC8          */
/******************************************************************************/
#ifndef __HCITRCFGH__
#define __HCITRCFGH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#include "stm32f4xx.h"           /* STM32F register definitions.              */
#include "stm32f4xx_hal_gpio.h"      /* STM32F GPIO control functions.            */
#include "stm32f4xx_hal_uart.h"     /* STM32F USART control functions.           */
#include "stm32f4xx_hal_rcc.h"       /* STM32F Clock control functions.           */
#include "stm32f4xx_hal_dma.h"       /* STM32F DMA control functions.             */

   /* The following definitions define the UART/USART to be used by the */
   /* HCI transport and the pins that will be used by the UART.  Please */
   /* consult the processor's documentation to determine what pins are  */
   /* available for the desired UART.                                   */
   /* * NOTE * The TXD, RXD, RTS and CTS pins MUST be map-able to the   */
   /*          selected UART.  The RESET pin may be any available GPIO. */
   /* * NOTE * The DMA settinsg (Number = 1 or 2, Stream and channel)   */
   /*          must map to the RXD and TXD streams for the selected     */
   /*          UART.                                                    */

#ifdef STM32F415xx
#define HCITR_UART               3

#define HCITR_TXD_PORT           D
#define HCITR_TXD_PIN            GPIO_PIN_8

#define HCITR_RXD_PORT           D
#define HCITR_RXD_PIN            GPIO_PIN_9

#define HCITR_RTS_PORT           D
#define HCITR_RTS_PIN            GPIO_PIN_12

#define HCITR_CTS_PORT           D
#define HCITR_CTS_PIN            GPIO_PIN_11

// BT_SHUTD_N = PE12
#define HCITR_RESET_PORT         E
#define HCITR_RESET_PIN          GPIO_PIN_12

// BT_HCI_OE = PE9
#define BT_HCI_OE_PORT           E
#define BT_HCI_OE_PIN            GPIO_PIN_9

// VSUP_BT_EN = PD4  already done in pmic.c
//#define VSUP_BT_EN_PORT          D
//#define VSUP_BT_EN_PIN           GPIO_PIN_4

#else
#define HCITR_UART               6

#define HCITR_TXD_PORT           G
#define HCITR_TXD_PIN            GPIO_PIN_14

#define HCITR_RXD_PORT           C
#define HCITR_RXD_PIN            GPIO_PIN_7

#define HCITR_RTS_PORT           G
#define HCITR_RTS_PIN            GPIO_PIN_8

#define HCITR_CTS_PORT           G
#define HCITR_CTS_PIN            GPIO_PIN_13

#define HCITR_RESET_PORT         D
#define HCITR_RESET_PIN          GPIO_PIN_3
#endif // STM32F415xx

   /* The following definitons define the DMA infomation for receive and*/
   /* transmit on the HCI UART.  This includes the DMA number (either 1 */
   /* or 2) as well as the stream and channel.                          */
   /* * NOTE * The DMA information MUST map to the receive and transmit */
   /*          DMA for the specified UART (see the DMA sections of the  */
   /*          processor's User Manual).                                */
#ifdef STM32F415xx
#define HCITR_DMA_RXD_NUMBER     1
#define HCITR_DMA_RXD_STREAM     1
#define HCITR_DMA_RXD_CHANNEL    4

#define HCITR_DMA_TXD_NUMBER     1
#define HCITR_DMA_TXD_STREAM     3
#define HCITR_DMA_TXD_CHANNEL    4
#else
#define HCITR_DMA_RXD_NUMBER     2
#define HCITR_DMA_RXD_STREAM     1
#define HCITR_DMA_RXD_CHANNEL    5

#define HCITR_DMA_TXD_NUMBER     2
#define HCITR_DMA_TXD_STREAM     7
#define HCITR_DMA_TXD_CHANNEL    5
#endif // STM32F415xx

   /* Define the following to enable suspend functionality within       */
   /* HCITRANS.  This will shut down the UART when HCITR_COMSuspend() is*/
   /* called and resume normal functionality when data is received in   */
   /* transmitted.                                                      */
   /* * NOTE * This functionality requires using a lower power protocol */
   /*          such as HCILL and the UART should only be suspended when */
   /*          indicated it is safe to do so by the protocol driver.    */
//#define SUPPORT_TRANSPORT_SUSPEND

   /* Define the following if software managed flow control is being    */
   /* used and the NVIC interrupt for the CTS EXTI line is being also   */
   /* used by another EXTI line.  The specified function can then be    */
   /* called by a global interrupt handler when the CTS EXTI interrupt  */
   /* occurs.                                                           */
   /* * NOTE * If defined when software managed flow control is used,   */
   /*          the NVIC interrupt associated with the CTS EXTI line MUST*/
   /*          be handled externally and call this function.  If not    */
   /*          defined, the interrupt will be handled directly by       */
   /*          HCITRANS.                                                */
//#define HCITR_CTS_IRQ_HANDLER HCITR_CTS_IrqHandler

   /* Define the following to enable debug logging of HCI traffic.  If  */
   /* this macro is defined, all incomming and outgoing traffic will be */
   /* logged via BTPS_OutputMessage().                                  */
// #define HCITR_ENABLE_DEBUG_LOGGING


/************************************************************************/
/* !!!DO NOT MODIFY PAST THIS POINT!!!                                  */
/************************************************************************/

   /* The following section builds the macros that can be used with the */
   /* STM32F standard peripheral libraries based on the above           */
   /* configuration.                                                    */

   /* Standard C style concatenation macros                             */
#define DEF_CONCAT2(_x_, _y_)          __DEF_CONCAT2__(_x_, _y_)
#define __DEF_CONCAT2__(_x_, _y_)      _x_ ## _y_

#define DEF_CONCAT3(_x_, _y_, _z_)     __DEF_CONCAT3__(_x_, _y_, _z_)
#define __DEF_CONCAT3__(_x_, _y_, _z_) _x_ ## _y_ ## _z_


/* Determine the Peripheral bus that is used by the UART.            */
#if ((HCITR_UART == 1) ||(HCITR_UART == 6))
   #define HCITR_UART_APB              2
#else
   #define HCITR_UART_APB              1
#endif

   /* Determine the type of UART.                                       */
#if ((HCITR_UART == 1) || (HCITR_UART == 2) || (HCITR_UART == 3) || (HCITR_UART == 6))

   #define HCITR_UART_TYPE             USART

#else
   #error Unknown HCITR_UART or UART not supported
#endif

   /* The following section builds the macro names that can be used with*/
   /* the STM32F standard peripheral libraries.                         */

   /* UART control mapping.                                             */
#define HCITR_UART_BASE                (DEF_CONCAT2(HCITR_UART_TYPE, HCITR_UART))

#define HCITR_UART_RCC_PERIPH_CLK_CMD  (DEF_CONCAT3(RCC_APB, HCITR_UART_APB, PeriphClockCmd))
#define HCITR_UART_RCC_PERIPH_CLK_BIT  (DEF_CONCAT3(DEF_CONCAT3(RCC_APB, HCITR_UART_APB, Periph_), HCITR_UART_TYPE, HCITR_UART))
#ifdef STM32F415xx
#define HCITR_UART_TX_GPIO_AF          GPIO_AF7_USART3
#define HCITR_UART_RX_GPIO_AF          GPIO_AF7_USART3
#define HCITR_UART_RTS_GPIO_AF         GPIO_AF7_USART3
#define HCITR_UART_CTS_GPIO_AF         GPIO_AF7_USART3
#else
#define HCITR_UART_TX_GPIO_AF          GPIO_AF8_USART6
#define HCITR_UART_RX_GPIO_AF          GPIO_AF8_USART6
#define HCITR_UART_RTS_GPIO_AF         GPIO_AF8_USART6
#define HCITR_UART_CTS_GPIO_AF         GPIO_AF8_USART6
#endif

#ifdef STM32F415xx
#define HCITR_UART_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()
#define HCITR_UART_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()
#define HCITR_UART_CLK_ENABLE()         __HAL_RCC_USART3_CLK_ENABLE()
#define HCITR_DMA_CLK_ENABLE()          __HAL_RCC_DMA1_CLK_ENABLE()
#define HCITR_RESET_CLK_ENABLE()        __HAL_RCC_GPIOE_CLK_ENABLE()
#else
#define HCITR_UART_TX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOG_CLK_ENABLE()
#define HCITR_UART_RX_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#define HCITR_UART_CLK_ENABLE()         __HAL_RCC_USART6_CLK_ENABLE()
#define HCITR_DMA_CLK_ENABLE()          __HAL_RCC_DMA2_CLK_ENABLE()
#define HCITR_RESET_CLK_ENABLE()        __HAL_RCC_GPIOD_CLK_ENABLE()
#endif

#define HCITR_IRQ_INTERRUPT_PRIORITY    5


/* GPIO mapping.                                                     */
#define HCITR_TXD_GPIO_PORT            (DEF_CONCAT2(GPIO, HCITR_TXD_PORT))
#define HCITR_RXD_GPIO_PORT            (DEF_CONCAT2(GPIO, HCITR_RXD_PORT))
#define HCITR_RTS_GPIO_PORT            (DEF_CONCAT2(GPIO, HCITR_RTS_PORT))
#define HCITR_CTS_GPIO_PORT            (DEF_CONCAT2(GPIO, HCITR_CTS_PORT))
#define HCITR_RESET_GPIO_PORT          (DEF_CONCAT2(GPIO, HCITR_RESET_PORT))
#ifdef STM32F415xx
#define HCITR_BT_HCI_OE_PORT           (DEF_CONCAT2(GPIO, BT_HCI_OE_PORT))
//#define HCITR_VSUP_BT_EN_PORT          (DEF_CONCAT2(GPIO, VSUP_BT_EN_PORT))  already done in pmic.c
#endif // STM32F415xx

#define HCITR_TXD_GPIO_AHB_BIT         (DEF_CONCAT2(RCC_AHB1Periph_GPIO, HCITR_TXD_PORT))
#define HCITR_RXD_GPIO_AHB_BIT         (DEF_CONCAT2(RCC_AHB1Periph_GPIO, HCITR_RXD_PORT))
#define HCITR_RTS_GPIO_AHB_BIT         (DEF_CONCAT2(RCC_AHB1Periph_GPIO, HCITR_RTS_PORT))
#define HCITR_CTS_GPIO_AHB_BIT         (DEF_CONCAT2(RCC_AHB1Periph_GPIO, HCITR_CTS_PORT))
#define HCITR_RESET_GPIO_AHB_BIT       (DEF_CONCAT2(RCC_AHB1Periph_GPIO, HCITR_RESET_PORT))
#ifdef STM32F415xx
#define HCITR_BT_HCI_OE_GPIO_AHB_BIT   (DEF_CONCAT2(RCC_AHB1Periph_GPIO, HCITR_BT_HCI_OE_PORT))
//#define HCITR_VSUP_BT_EN_GPIO_ABH_BIT  (DEF_CONCAT2(RCC_ABH1Periph_GPIO, HCITR_VSUP_BT_EN_PORT))  already done in pmic.c
#endif // STM32F415xx

#define HCITR_RXD_DMA_FLAG_TCIF        DMA_FLAG_TCIF1_5
#define HCITR_RXD_DMA_FLAG_HTIF        DMA_FLAG_HTIF1_5
#define HCITR_RXD_DMA_STREAM           (DEF_CONCAT2(DEF_CONCAT3(DMA, HCITR_DMA_RXD_NUMBER, _Stream), HCITR_DMA_RXD_STREAM))
#define HCITR_TXD_DMA_STREAM           (DEF_CONCAT2(DEF_CONCAT3(DMA, HCITR_DMA_TXD_NUMBER, _Stream), HCITR_DMA_TXD_STREAM))
#define HCITR_RXD_DMA_CHANNEL          (DEF_CONCAT2(DMA_CHANNEL_, HCITR_DMA_RXD_CHANNEL))
#define HCITR_TXD_DMA_CHANNEL          (DEF_CONCAT2(DMA_CHANNEL_, HCITR_DMA_TXD_CHANNEL))

#define HCITR_TXD_IRQ                  (DEF_CONCAT3(DEF_CONCAT3(DMA, HCITR_DMA_TXD_NUMBER, _Stream), HCITR_DMA_TXD_STREAM, _IRQn))
#define HCITR_RXD_IRQ                  (DEF_CONCAT3(DEF_CONCAT3(DMA, HCITR_DMA_RXD_NUMBER, _Stream), HCITR_DMA_RXD_STREAM, _IRQn))

   /* Location of the Data register for the UART in use.                */
#define HCITR_UART_DR_REGISTER_ADDRESS (((unsigned int)(DEF_CONCAT3(HCITR_UART_TYPE, HCITR_UART, _BASE))) + 4)

   /* Interrupt mapping.                                                */
//#ifdef SUPPORT_TRANSPORT_SUSPEND

//   #if (HCITR_CTS_PIN   < 5)
//      #define HCITR_CTS_EXTI_NUMBER    HCITR_CTS_PIN
//   #elif (HCITR_CTS_PIN < 10)
//      #define HCITR_CTS_EXTI_NUMBER    9_5
//   #elif (HCITR_CTS_PIN < 16)
//      #define HCITR_CTS_EXTI_NUMBER    15_10
//   #endif

   /* NOTE: "EXTI" is defined in the STM32F std periph headers so can   */
   /* not be used directly.                                             */
   #define HCITR_CTS_IRQ               EXTI15_10_IRQn           //DEF_CONCAT3(EXT, DEF_CONCAT2(I, HCITR_CTS_EXTI_NUMBER), _IRQn))         
   #define HCITR_CTS_EXTI_PORT         (DEF_CONCAT2(EXTI_PortSourceGPIO, HCITR_CTS_PORT))
   #define HCITR_CTS_IRQ_LINE          (DEF_CONCAT2(EXTI_Line, HCITR_CTS_PIN))

   #ifndef HCITR_CTS_IRQ_HANDLER
//      #define HCITR_CTS_IRQ_HANDLER    (DEF_CONCAT3(EXT, DEF_CONCAT2(I, HCITR_CTS_EXTI_NUMBER), _IRQHandler))
      #define HCITR_CTS_IRQ_HANDLER    EXTI15_10_IRQHandler
   #endif

//#endif

#endif

