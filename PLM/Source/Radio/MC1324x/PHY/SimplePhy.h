/************************************************************************************
* Header file for physical layer
*
* (c) Copyright 2010, Freescale, Inc.  All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#ifndef _SIMPLE_PHY_H_
#define _SIMPLE_PHY_H_


/************************************************************************************
*************************************************************************************
* Includes
*************************************************************************************
************************************************************************************/

#include "Phy.h"
#include "TransceiverReg.h"
#include "TransceiverDrv.h"
#include "EmbeddedTypes.h"


  #define PhyPlmeSetClockOutRate(x)        PhyPlmeSetClockOutRateReg(x)
  #define PhyAdjustCcaOffsetCmp(x)         PhyAdjustCcaOffsetCmpReg(x)
  #define PhySetRadioTimerPrescaler(x)     PhySetRadioTimerPrescalerReg(x)
  

extern void PhyXtalAdjust(uint8_t);
extern void PhyWakeRequest(void);
extern uint8_t PhyGetRfIcVersion(void);
extern void PhyPlmeDozeRequest(uint32_t);
extern void PhyHibernateRequest(void);

#endif /* _PHY_H_ */
