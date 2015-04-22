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

#define ENABLE_TUNER PORTC_PCR4 = PORT_PCR_MUX(4); PORTC_PCR5 = PORT_PCR_MUX(7); GPIOC_PDDR &= (uint32_t)~(uint32_t)GPIO_PDDR_PDD(0x10); Wait_Waitms(1000);
#define ENABLE_DISPLAY PORTC_PCR4 = PORT_PCR_MUX(1); PORTC_PCR5 = PORT_PCR_MUX(2); GPIOC_PDDR |= (uint32_t)GPIO_PDDR_PDD(0x10); Wait_Waitms(1000);
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
