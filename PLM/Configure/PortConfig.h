/************************************************************************************
 * Mapping of the IO ports and pins.
 *
 * (c) Copyright 2006, Freescale Semiconductor, Inc. All rights reserved. 
 *
 *
 * No part of this document must be reproduced in any form - including copied,
 * transcribed, printed or by any electronic means - without specific written
 * permission from Freescale Semiconductor.
 ************************************************************************************/

#ifndef _PORT_CONFIG_H_
#define _PORT_CONFIG_H_

#define gTargetTWR_K60N512_c    0
#define gTargetTWR_KW21D512_c   1

#define gMcuK60N512_c           0 
#define gMcuKW21D512_c          1

#define gComm_CDC_c             0  // USB_CDC COM
#define gComm_UART_c            1  // UART COM

#define gXcvrMC1324x_c          0

/* Comm interface definition */

#define gCommInterface_c         gComm_UART_c

/* Target Board Definition */

#define gTargetBoard_d          gTargetTWR_KW21D512_c
#define gUserTransceiverType_d   gXcvrMC1324x_c
       

#if (gTargetBoard_d == gTargetTWR_K60N512_c)
  #define gTargetMcu_d                  gMcuK60N512_c
  #include "TargetTWR-K60N512.h"
  
#elif (gTargetBoard_d == gTargetTWR_KW21D512_c)
  #define gTargetMcu_d                  gMcuKW21D512_c
  #include "TargetTWR-KW21D512.h"
  #define MCU_MK21DN512	1
#endif /*gTargetBoard_d*/

#if(gCommInterface_c == gComm_CDC_c)
  #define gCDCInterface_d	      1
  #define gComm_Type_d            gCommUsb_c
  #define gUsbIncluded_d          TRUE
#else
  #define gUARTInterface_d        1
  #define gComm_Type_d            gCommUart_c
#endif

#endif /* _PORT_CONFIG_H_ */
