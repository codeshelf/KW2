/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#ifndef TUNERTASK_H
#define TUNERTASK_H

#include "FreeRTOS.h"

typedef enum {
	eSetupModeStarted,
	eSetupModeGetGuid,
	eSetupModeGetTuning
} ELocalSetupType;

// --------------------------------------------------------------------------
// Local functions prototypes.

void tunerTask(void *pvParameters);

#endif TUNERTASK_H
