/************************************************************************************
* Header file for Utilities.
*
*
* (c) Copyright 2009, Freescale, Inc.  All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale.
*
*************************************************************************************/
#ifndef _UTILITIES_INTERFACE_H_
#define _UTILITIES_INTERFACE_H_
   
/* Include files section */   
#include "PortConfig.h"
#include "EmbeddedTypes.h"  // include special data types
#if (gCommInterface_c == gComm_UART_c)
#include "UART_Interface.h" // include UART functionality
#elif (gCommInterface_c == gComm_CDC_c)
#include "CDC_Interface.h" // include CDC functionality
#endif
#include "CommUtil.h" // include UART functionality
#include "mcg.h" // include UART functionality

/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
*******************************************************************************/


typedef struct sixDigitBcd_tag{
  uint8_t u8Units:4;
  uint8_t u8Tens:4;
  uint8_t u8Houndreds:4;
  uint8_t u8Thousands:4;
  uint8_t u8TensThousands:4;
  uint8_t u8HoundredsThousands:4;
}sixDigitBcd_t;


/******************************************************************************
*******************************************************************************
* Public macros
*******************************************************************************
*******************************************************************************/

/*Blocking Delay, Max Milis = 2^16/(gSystemClock_d/40000)*/
#define StringCopy(pSourcePtr, pDestPtr, stringlength)\
  do{\
    uint8_t u8f;\
    for(u8f = 0; u8f < stringlength;u8f++) *(pDestPtr+u8f) = *(pSourcePtr+u8f);\
  }while(0)

#define mMilisecondsAdjust_c 307

#define LOW_POWER_WHILE() _asm wait

/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
*******************************************************************************/

void delayMs(uint16_t val);

/************************************************************************************
*
* convert_2_bcd
*
* Description: This function converts the received data as parameter to bcd format  
*
* Interface assumptions: none
* 
* Return value:   sixDigitBcd_t -> bcd format value
*
************************************************************************************/

extern sixDigitBcd_t convert_2_bcd
(
  uint16_t u16Value  /* IN: Value to convert */
);

/************************************************************************************
*
* HexToAscii
*
* Description:  This function converts an hexadecimal value in ascii code.
*
* Interface assumptions: none
* 
* Return value: uint8_t -> Ascii value 
*
************************************************************************************/


extern uint8_t HexToAscii
(
  uint8_t u8Hex  /* IN: Value to convert */
);

/************************************************************************************
*
* stringComp
*
* Description:  
*
* Interface assumptions: 
* 
* Return value: bool_t 
*
************************************************************************************/


extern bool_t stringComp
(
  char * au8leftString, 
  char * au8RightString, 
  uint8_t bytesToCompare
);

/************************************************************************************
*
* MemoryCpy
*
* Description:  
*
* Interface assumptions: 
* 
* Return value:  
*
************************************************************************************/


extern void MemoryCpy
(
  void *pDst, /* Pointer to Destination buffer */ 
  void *pSrc, /* Source buffer */
  uint8_t n   /* Byte count */
);

/************************************************************************************
*
* MemorySet
*
* Description:  
*
* Interface assumptions: 
* 
* Return value:  
*
************************************************************************************/


extern void MemorySet
(
  void *pDst,    // Buffer to be reset
  uint8_t value, // Byte value
  uint16_t cnt    // Byte count
);

/************************************************************************************
*
* PrintWordOnDecimalFormatBlocking
*
* Description:  
*
* Interface assumptions: 
* 
* Return value:  
*
************************************************************************************/

void PrintWordOnDecimalFormatBlocking
(
  uint16_t u16Dec, 
  uint8_t u8MinDigs,
  bool_t bNegative, 
  uint8_t port
);

/************************************************************************************
*
* PrintByteOnHexFormatBlocking
*
* Description:  
*
* Interface assumptions: 
* 
* Return value:  
*
************************************************************************************/


void PrintByteOnHexFormatBlocking
(
  uint8_t u8Byte, 
  bool_t bPrintComma, 
  uint8_t port
);

/************************************************************************************
*
* int2BCD
*
* Description:  
*
* Interface assumptions: 
* 
* Return value:  
*
************************************************************************************/

void int2BCD
(
  uint16_t u16Num, 
  uint8_t * pu8BCDOut
);

/************************************************************************************
*
* HexString2Dec16
*
* Description:  
*
* Interface assumptions: 
* 
* Return value:  
*
************************************************************************************/

uint16_t HexString2Dec16
(
  uint8_t * au8String
);

/************************************************************************************
*
* MCU_InitClock
*
* Description:  
*
* Interface assumptions: 
* 
* Return value:  
*
************************************************************************************/
void MCU_InitClock(void);

/************************************************************************************
* This function copies the specified number of bytes from the
* source address to the destination address.  No attempt is made
* to handle overlapping copies to prevent loss of data.
* The copying is optimized to avoid alignment problems, and attempts
* to copy 32bit numbers optimally.
*   
* Interface assumptions:
*   None
*   
* Return value:
*   None
* 
************************************************************************************/
void FLib_MemCpyAligned32bit
  (
     /* [IN] address to copy data from */
     void*  from_ptr,

     /* [IN] address to copy data to */
     void*  to_ptr,

     /* [IN] number of bytes to copy */
     register uint32_t number_of_bytes
  );

#endif /* _UTILITIES_H_ */
