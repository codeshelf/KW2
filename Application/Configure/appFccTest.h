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
#include "SMAC_Interface.h"

// --------------------------------------------------------------------------
// Definitions.

#define RS485					TRUE

#define LCD_ON					LcdPowerCtl_SetVal(LcdPowerCtl_DeviceData);
#define LCD_OFF					LcdPowerCtl_ClrVal(LcdPowerCtl_DeviceData);

#define DEFAULT_CHANNEL			gChannel11_c
#define DEFAULT_POWER			31
#define DEFAULT_CRYSTAL_TRIM	117

#define GPIO_PIN2 0x02

// --------------------------------------------------------------------------
// Function prototypes.

gwUINT8 sendrs485Message(char* msgPtr, gwUINT8 msgLen);

#endif //APP_CONTROLLER_H
