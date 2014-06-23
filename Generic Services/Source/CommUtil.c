/************************************************************************************
* Includes the Comm Utils.
*
* (c) Copyright 2012, Freescale, Inc. All rights reserved.
*
* No part of this document may be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*
************************************************************************************/
#include "EmbeddedTypes.h"
#include "CommUtil.h"
#include "Utilities_Interface.h"
#include "MsgSystem.h"
/******************************************************************************/
/******************************************************************************/

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
extern uint32_t __region_ROM_start__[];
extern uint32_t __region_ROM_end__[];
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

#if (gComm_Type_d != gNoComm_c)
/************************************************************************************/
#ifndef gUartDefaultBaud_c
#define gUartDefaultBaud_c UartX_DefaultBaud
#endif

/************************************************************************************/

void CommUtil_TxCallback(unsigned char const *pBuf)
{
  if(pBuf != NULL)
  {
    MM_Free((void*)pBuf);
  }
  
}

/************************************************************************************/

void CommUtil_Tx(uint8_t * pData, uint8_t length)
{
  /* wait until module can insert message into Comm buffers */
  if ( ((uint32_t)pData > (uint32_t)__region_ROM_start__) &&
            ((uint32_t)pData < (uint32_t)__region_ROM_end__))
  {
    uint8_t* pRam = MM_Alloc(length);
    if(pRam == NULL)
    {
      return;
    }
    MemoryCpy( pRam, pData, length );
    while(!Comm_Transmit(pRam,(index_t)length,CommUtil_TxCallback));
  }

  else
  {
    while(!Comm_Transmit(pData,(index_t)length,NULL));
  }
  
  /* wait until tx complete */
  while (Comm_IsTxActive());
}

/************************************************************************************/

void CommUtil_Print(char* pString, commUtilBlock_t allowToBlock)
{
  uint16_t i = 0;
  char *  pTmpString = pString ;

  while (*pTmpString != '\0')
  {
    pTmpString++;
    i++;
  }
  if(allowToBlock)
  {
    /* wait until module can insert message into Comm buffers and until tx completes */
	CommUtil_Tx((uint8_t *)pString,(uint8_t)i);
  }
  else
  {
    if ( ((uint32_t)pString > (uint32_t)__region_ROM_start__) &&
            ((uint32_t)pString < (uint32_t)__region_ROM_end__))
    {
      uint8_t* pRam = MM_Alloc((uint8_t)i);
      if(pRam == NULL)
      {
        return;
      }
      MemoryCpy( pRam, pString, (uint8_t)i );
      (void)Comm_Transmit((unsigned char const*)pRam,(index_t)i,CommUtil_TxCallback);
    }
    else
    {
      (void)Comm_Transmit((unsigned char const*)pString,(index_t)i,NULL);      
    }    

  }
}

/************************************************************************************/

void CommUtil_PrintHex(uint8_t *hex, uint8_t len, uint8_t flags)
{
  uint8_t i=0;
  uint8_t hexString[6]; /* 2 bytes  - hexadecimal display
                           1 byte   - separator ( comma)
						   1 byte   - separator ( space)
                           2 bytes  - new line (\n\r)  */

  if(! (flags & gPrtHexBigEndian_c))
   {
    hex = hex + (len-1);
   }

  while(len)
  {
    /* wait until previous hex print transmission is over */
    while (Comm_IsTxActive());

    /* start preparing the print of a new byte */
    i=0;
    hexString[i++] = HexToAscii((*hex)>>4);
    hexString[i++] = HexToAscii( *hex);

    if(flags & gPrtHexCommas_c)
     {
      hexString[i++] = ',';
     }
    if(flags & gPrtHexSpaces_c)
     {
      hexString[i++] = ' ';
     }
    hex = hex + (flags & gPrtHexBigEndian_c ? 1 : -1);
    len--;

    if((len == 0) && (flags & gPrtHexNewLine_c))
     {
      hexString[i++] = '\n';
      hexString[i++] = '\r';
     }
     /* transmit formatted byte */
    (void)Comm_Transmit((unsigned char const*)hexString,(index_t)i,NULL);
  }
  /* wait to complete the last transmission */
  while (Comm_IsTxActive());
  (void)hexString;
}

/************************************************************************************/
void CommUtil_PrintDec(uint32_t nr)
{
  uint8_t i = gDecStringLen_d-1;
  uint8_t decString[gDecStringLen_d];

  if (nr == 0)
  {
    decString[i] = '0';
  }
  else
  {
    while (nr)
    {
      decString[i] = '0' + (uint8_t)(nr % 10);
      nr = nr / 10;
      i--;
    }
    i++;
  }

  /* wait to complete the last transmission */
  while (Comm_IsTxActive());

   /* transmit formatted byte */
  (void)Comm_Transmit((uint8_t*)&decString[i], gDecStringLen_d-i, NULL);

  /* wait to complete the last transmission */
  while (Comm_IsTxActive());
  (void)decString;
}

/************************************************************************************/

#else /* (gComm_Type_d != gNoComm_c) */

/************************************************************************************/
/* Create  dummy functions  */
/************************************************************************************/
void CommUtil_Init(pCommRxCallBack_t pCommRxCallBack)
{
  (void)pCommRxCallBack;
}

void CommUtil_Print(uint8_t * pString, commUtilBlock_t allowToBlock)
{
  /* Avoid compiler warning */
  (void)pString;
  (void)allowToBlock;
}

/************************************************************************************/

void CommUtil_PrintHex(uint8_t *hex, uint8_t len, uint8_t flags)
{
  /* Avoid compiler warning */
  (void)hex;
  (void)len;
  (void)flags;
}

/************************************************************************************/

void CommUtil_Tx(uint8_t * pData, uint8_t length){
  /* Avoid compiler warning */
  (void)pData;
  (void)length;
}

/************************************************************************************/
#endif  /* (gComm_Type_d != gNoComm_c) */
