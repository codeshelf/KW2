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
	eSetupModeGetAes,
	eSetupModeGetHWver,
	eSetupModeWaitingForSave,
	eSetupModeComplete,
	
} ELocalSetupType;

// --------------------------------------------------------------------------
// Local functions prototypes.

void tunerTask(void *pvParameters);

void tuneRadio();


void scanParams();
int isSetupCommand();
void handleSetupScan();
void handleSetupCommand();
void enterSetupMode();
void clearParams();
void saveParams();
void getGuid();
void getAes();
void getHwVersion();

#endif TUNERTASK_H
