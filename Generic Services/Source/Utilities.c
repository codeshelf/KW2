/*****************************************************************************
* Source file for Utilities.
* 
* 
* Copyright (c) 2009, Freescale, Inc. All rights reserved.
*
* 
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#include "Utilities_Interface.h"
#include "mcg.h"

/************************************************************************************
* Delay_ms
*
************************************************************************************/
/*
void Delay_ms
(
  uint16_t u16ms
) 
{                                            
  uint16_t u16ctr;                                          
  for(u16ctr = 0; u16ctr < ((gSystemClock_d/40000)*u16ms); u16ctr++){
    asm NOP; 
    asm NOP;
  }    
}
*/
/************************************************************************************
* Delay_ms
*
************************************************************************************/

 void delayMs(uint16_t val)
{
	/* Turn on clock to LPTMR module */
	SIM_SCGC5 |= SIM_SCGC5_LPTIMER_MASK;
        
	/* Disable LPTMR */
	LPTMR0_CSR &= ~(LPTMR_CSR_TEN_MASK);

	/* Set compare value */
	LPTMR0_CMR = val;

	/* Use 1Khz LPO clock and bypass prescaler */
	LPTMR0_PSR = LPTMR_PSR_PCS(1)|LPTMR_PSR_PBYP_MASK;

	/* Start counting */
	LPTMR0_CSR |= LPTMR_CSR_TEN_MASK;

	/* Wait for counter to reach compare value */
	while (!(LPTMR0_CSR & LPTMR_CSR_TCF_MASK)) {}

	/* Clear Timer Compare Flag */
	LPTMR0_CSR &= ~LPTMR_CSR_TEN_MASK;

	/* Turn off clock to LPTMR module */
	SIM_SCGC5 &= ~SIM_SCGC5_LPTIMER_MASK;	
}
/************************************************************************************
* Delay_us
*
************************************************************************************/
/*
void Delay_us
(
  uint16_t u16us
) 
{
  uint16_t u16ctr;                                          
  for(u16ctr = 0; u16ctr < (((gSystemClock_d*10/40000000)*u16us)/10); u16ctr++){
    asm NOP; 
    asm NOP;     
  }      
}
*/

 
/*******************************************************************************  
* HexToAscii
*
* This function converts an hexadecimal value in ascii code.
*******************************************************************************/
uint8_t HexToAscii(uint8_t u8Hex)
{
  u8Hex &= 0x0F;
  return u8Hex + ((u8Hex <= 9) ? '0' : ('A'-10));
}



/**************************************************************************************/



/************************************************************************************
* convert_2_bcd function
*
* This function converts the received data as parameter to bcd format
*
************************************************************************************/
sixDigitBcd_t convert_2_bcd(uint16_t u16Value)
{
  sixDigitBcd_t u16RtrnVal;
  uint16_t tmp;
  u16RtrnVal.u8Units = 0;
  u16RtrnVal.u8Tens = 0;
  u16RtrnVal.u8Houndreds = 0;
  u16RtrnVal.u8Thousands = 0;
  u16RtrnVal.u8TensThousands = 0;
  u16RtrnVal.u8HoundredsThousands = 0;
  if(65535 > u16Value)
  {
    tmp = u16Value;
    if(10000 <= u16Value)
    {
      tmp = u16Value/10000;
      u16RtrnVal.u8TensThousands = (uint8_t)tmp;
      tmp = u16Value%10000;
      u16Value = tmp;
    }

    if(1000 <= u16Value)
    {
      tmp = u16Value/1000;
      u16RtrnVal.u8Thousands = (uint8_t)tmp;
      tmp = u16Value%1000;
      u16Value = tmp;
    }
    if(100 <= u16Value)
    {
      tmp = u16Value/100;
      u16RtrnVal.u8Houndreds = (uint8_t)tmp;
      tmp = u16Value%100;
      u16Value = tmp;
    }
    if(10 <= u16Value)
    {
      tmp = u16Value/10;
      u16RtrnVal.u8Tens = (uint8_t)tmp;
      tmp = u16Value%10;
      u16Value = tmp;
    }
    u16RtrnVal.u8Units = (uint8_t)tmp;
  }
  return u16RtrnVal;
} 

uint8_t HexaToAscii(uint8_t u8Hex)
{
  u8Hex &= 0x0F;
  return u8Hex + ((u8Hex <= 9) ? '0' : ('A'-10));
}

/************************************************************************************
* stringComp
*
* ...
*
************************************************************************************/
bool_t stringComp(char * au8leftString, char * au8RightString, uint8_t bytesToCompare)
{
  do
  {
  }while((*au8leftString++ == *au8RightString++) && --bytesToCompare);
  return(0 == bytesToCompare);
}


/************************************************************************************
* MemoryCpy
*
* This function copy a buffer source to a buffer destination
*
************************************************************************************/

void MemoryCpy
(
  void *pDst, // Destination buffer
  void *pSrc, // Source buffer
  uint8_t n   // Byte count
)
{
    // Code looks weird, but is quite optimal on a HCS08 CPU
  while(n) {
    *((uint8_t *)pDst) = *((uint8_t *)pSrc);
    pDst=((uint8_t *)pDst)+1;
    pSrc=((uint8_t *)pSrc)+1;
    n--;
  }
}

/************************************************************************************
* MemorySet
*
* This function set an specific value into a buffer destination
*
************************************************************************************/

void MemorySet
(
  void *pDst,    // Buffer to be reset
  uint8_t value, // Byte value
  uint16_t cnt    // Byte count
)
{
  while(cnt) {
    ((uint8_t *)pDst)[--cnt] = value;
  }
}

/**************************************************************************************/
void PrintByteOnHexFormatBlocking(uint8_t u8Byte, bool_t bPrintComma, uint8_t port)
{
  static uint8_t HexString[4] = {0};
    
  HexString[0] = HexToAscii(u8Byte>>4);
  HexString[1] = HexToAscii(u8Byte);
  
  if(bPrintComma){
    HexString[2] = ',';  
  }else{
    HexString[2] = 0;      
  }
  HexString[3] = 0;      
  
  CommUtil_Print((char *)HexString, gAllowToBlock_d);
}


/**************************************************************************************/
void PrintWordOnDecimalFormatBlocking(uint16_t u16Dec, uint8_t u8MinDigs,bool_t bNegative, uint8_t port)
{
  static uint8_t decString[7] = {0};
  uint8_t ind,Sing;
    
  int2BCD(u16Dec, &decString[1]);
  
  Sing=0;
  decString[0]=0;
  if(u8MinDigs > 5) u8MinDigs = 5;
  while((0 == decString[Sing+1]) && (Sing < 5-u8MinDigs)){
    Sing++; 
  }
  for(ind=Sing;ind<6;ind++){
    decString[ind] += '0';  
  }
  if(bNegative){
    decString[Sing] = '-';
  }else{
    Sing++;
  }
  CommUtil_Print((char*)&decString[Sing], gAllowToBlock_d);
}

/**************************************************************************************/
void int2BCD(uint16_t u16Num, uint8_t * pu8BCDOut)
{
  uint8_t cont=16;
  uint8_t aux=0;

  for (aux=0;aux<5;aux++){
    pu8BCDOut[aux]=0;
  }
  do{
    for(aux=0;aux<5;aux++) {
      pu8BCDOut[aux]<<=1;
    }
    if(u16Num&0x8000){
      pu8BCDOut[4]+=1;
    }
    u16Num<<=1;
    for(aux=4;aux>0;aux--){
      if (pu8BCDOut[aux]>=10){
        pu8BCDOut[aux-1]++;
        pu8BCDOut[aux]-=10;
      }
    }
    cont--;
  }while(cont);
}

/**************************************************************************************/
uint16_t HexString2Dec16(uint8_t * au8String)
{
  uint8_t u8LocIndex=0;
  uint8_t u8LocIndex2=0;
  uint16_t u16DecValue = 0;
  
  while(au8String[u8LocIndex]){
    u8LocIndex++;
  }
  
  while(u8LocIndex--){
    if((au8String[u8LocIndex] >= '0') && (au8String[u8LocIndex] <= '9'))
     u16DecValue |= ((uint16_t)(au8String[u8LocIndex] - '0'))<<(u8LocIndex2*4);
    else if((au8String[u8LocIndex] >= 'A') && (au8String[u8LocIndex] <= 'F')){
     u16DecValue |= ((uint16_t)(au8String[u8LocIndex] - 'A' + 0x0A))<<(u8LocIndex2*4);    
    }else{
     u16DecValue |= ((uint16_t)(au8String[u8LocIndex] - 'a' + 0x0A))<<(u8LocIndex2*4);        
    }
    u8LocIndex2++;
  }
  
  return u16DecValue;
  
}
/**************************************************************************************/

void MCU_InitClock(void)
{
    gMCG_coreClkMHz = MCG_PLLInit();
   /*
   * Use the value obtained from the pll_init function to define variables
   * for the core clock in kHz and also the peripheral clock. These
   * variables can be used by other functions that need awareness of the
   * system frequency.
   */
  gMCG_coreClkKHz = gMCG_coreClkMHz * 1000;
  gMCG_periphClkKHz = gMCG_coreClkKHz / (((SIM_CLKDIV1 & SIM_CLKDIV1_OUTDIV2_MASK) >> 24)+ 1);

}

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
  )
{

   uint8_t*  	from8_ptr = (uint8_t*) from_ptr;
   uint8_t*  	to8_ptr = (uint8_t*) to_ptr;
   uint16_t* 	from16_ptr = (uint16_t*) from_ptr;
   uint16_t* 	to16_ptr = (uint16_t*) to_ptr;
   register 	uint32_t* from32_ptr = (uint32_t*) from_ptr;
   register 	uint32_t* to32_ptr = (uint32_t*) to_ptr;
   
   register 	uint32_t loops;

   if (number_of_bytes > 3) 
   {
      /* Try to align source on word */
      if ((uint32_t)from_ptr & 1) 
      {
         from8_ptr = (uint8_t* ) from_ptr;
         to8_ptr = (uint8_t* ) to_ptr;

         *to8_ptr++ = *from8_ptr++;

         from_ptr = from8_ptr;
         to_ptr = to8_ptr;
         --number_of_bytes;
      }

      /* Try to align source on longword */
      if ((uint32_t)from_ptr & 2) 
      {
         from16_ptr = (uint16_t*) from_ptr;
         to16_ptr = (uint16_t*) to_ptr;

         *to16_ptr++ = *from16_ptr++;

         from_ptr = from16_ptr;
         to_ptr = to16_ptr;
         number_of_bytes -= 2;
      }

      from32_ptr = (uint32_t*) from_ptr;
      to32_ptr = (uint32_t*) to_ptr;

      for (loops = number_of_bytes >> 2; loops != 0; loops--) 
      {
         *to32_ptr++ = *from32_ptr++;
      }

      from_ptr = from32_ptr;
      to_ptr = to32_ptr;
   } 

   /* Copy all remaining bytes */
   if (number_of_bytes & 2) 
   {
      from16_ptr = (uint16_t*) from_ptr;
      to16_ptr = (uint16_t*) to_ptr;

      *to16_ptr++ = *from16_ptr++;

      from_ptr = from16_ptr;
      to_ptr = to16_ptr;
   } 

   if (number_of_bytes & 1) 
   {
      * (uint8_t* ) to_ptr = * (uint8_t* ) from_ptr;
   }
}

