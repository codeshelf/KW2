/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#ifndef REMOTEMGMTTASK_H
#define REMOTEMGMTTASK_H

#include "gwTypes.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"

// --------------------------------------------------------------------------
// Local functions prototypes.

void remoteMgmtTask(void *pvParameters);
void sleep();
//void sleep(gwUINT8 inSleepMillis);

#endif /* REMOTEMGMTTASK_H */
