/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#ifndef APP_CART_CONTROLLER_H
#define APP_CART_CONTROLLER_H

#include "gwTypes.h"
#include "LcdPowerCtl.h"
#include "SMAC_Interface.h"

// --------------------------------------------------------------------------
// Definitions.

#define	CHE_CONTROLLER			TRUE

#define LCD_ON					LcdPowerCtl_SetVal(LcdPowerCtl_DeviceData);
#define LCD_OFF					LcdPowerCtl_ClrVal(LcdPowerCtl_DeviceData);

#define DEFAULT_CHANNEL			gChannel15_c
#define DEFAULT_POWER			23
#define DEFAULT_CRYSTAL_TRIM	115

// --------------------------------------------------------------------------
// Function prototypes.

gwUINT8 sendrs485Message(char* msgPtr, gwUINT8 msgLen);
void startApplication(void);

#endif //APP_CONTROLLER_H
