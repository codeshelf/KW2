/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "tunerTask.h"
#include "SMAC.h"
#include "task.h"
#include "queue.h"
#include "commands.h"
#include "Wait.h"
#include "display.h"

#include "scannerReadTask.h"
#include "task.h"
#include "queue.h"
#include "commands.h"
#include "Scanner.h"
#include "Wait.h"
#include "EventTimer.h"
#include "ScannerPower.h"

#define DEFAULT_CRYSTAL_TRIM	0xcd //0xf4
#define PERFECT_REMAINDER		0x6b06

xTaskHandle gTunerTask = NULL;

void adjustTune(gwUINT8 trim);
gwUINT16 measureTune(void);
gwUINT8 tuneRadio();

char aes_key[32];
char hw_ver[8];
char guid[8];
gwUINT8 ccrHolder;

ELocalSetupType setupModeState;
ScanStringType gScanString;
ScanStringLenType gScanStringPos;

// --------------------------------------------------------------------------

void tunerTask(void *pvParameters) {
	gwUINT8 trim = 0;
	
	Tuner_Init();
	trim = tuneRadio();

	Rs485_Init();
	SharpDisplay_Init();
	scanParams();
	
	
	// Stop cpu interrupts
	for(;;){}
}

void adjustTune(gwUINT8 trim) {
	MLMEXtalAdjust(trim); 
	// Wait for the clocks to stabilize.
	Wait_Waitms(200);
}

gwUINT16 measureTune() {
	
	gwUINT16 remainder = 0;
	
	// Capture and measure the rising edges of the GPS PPS signal.
	Tuner_Init();
	FTM0_SC = 0x80;
	FTM0_C2V = 0;
	FTM0_C3V = 0;
	FTM0_MOD = 0xffff;
	FTM0_CNTIN = 0x0001;
	FTM0_CNT = 0x0000;
	FTM0_SC = 0x88;
	FTM0_COMBINE |= (FTM_COMBINE_DECAP1_MASK);
	// Loop until there is an event.
	while ((FTM0_STATUS & FTM_STATUS_CH3F_MASK) == 0) {
		GW_WATCHDOG_RESET;
	}

	if (FTM0_C2V < FTM0_C3V) {
		remainder = FTM0_C3V - FTM0_C2V;
	} else {
		remainder = (0xffff - FTM0_C2V) + FTM0_C3V;
	}
	
	return remainder;
}

gwUINT8 tuneRadio() {
	
	gwUINT8 trim;
	gwUINT16 remainder;
		
	GW_ENTER_CRITICAL(ccrHolder);
			
	trim = DEFAULT_CRYSTAL_TRIM;
	adjustTune(trim);
	remainder = measureTune();
	if (remainder > PERFECT_REMAINDER) {
		while ((remainder > PERFECT_REMAINDER) && (trim < 255)) {
			adjustTune(++trim);
			remainder = measureTune();
		}
	} else if (remainder < PERFECT_REMAINDER) {
		while ((remainder < PERFECT_REMAINDER) && (trim > 0)) {
			adjustTune(--trim);
			remainder = measureTune();
		}
	}
	
	GW_EXIT_CRITICAL(ccrHolder);
	
	return trim;
}

void scanParams() {
	
	
}

// --------------------------------------------------------------------------

int isSetupCommand() {
	if (strlen(gScanString) > 1) {
		if (gScanString[0] == 'S' && gScanString[1] == '%'){
			return 1;
		}
	}
	
	return 0;
}

// --------------------------------------------------------------------------

void handleSetupScan() {
	
	if (setupModeState == eSetupModeGetGuid) {
		setGuid();
	}
	else if (setupModeState == eSetupModeGetTuning) {
		setTuning();
	}
	
}

// --------------------------------------------------------------------------

void handleSetupCommand() {
	if (strcmp(gScanString, "S%SETUP") == 0) {
		enterSetupMode();
	} 
	else if (strcmp(gScanString, "S%SETGUID") == 0) {
		handleSetGuidCmd();
	}
	else if (strcmp(gScanString, "S%SETTUNE") == 0) {
		handleSetTuningCmd();
	}
	else if (strcmp(gScanString, "S%EXIT") == 0) {
		exitSetupMode();
	}
}

// --------------------------------------------------------------------------

void handleSetTuningCmd(){
	setupModeState = eSetupModeGetGuid;

	GW_ENTER_CRITICAL(ccrHolder);
	displayMessage(2, "Scan tuning info", FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
}

// --------------------------------------------------------------------------
void setTuning() {
	// Place holder
	
	// Go back to top level setup
	enterSetupMode();
}


// --------------------------------------------------------------------------

void handleSetGuidCmd(){
	setupModeState = eSetupModeGetGuid;

	GW_ENTER_CRITICAL(ccrHolder);
	displayMessage(2, "Scan GUID", FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
}

// --------------------------------------------------------------------------
void setGuid() {
	GW_ENTER_CRITICAL(ccrHolder);
	displayMessage(2, "Got GUID", FONT_NORMAL);
	displayMessage(3, gScanString, FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
	vTaskDelay(1500);
	
	// Save guid to eeprom
	writeGuid((uint8_t *) &gScanString);

	// Go back to top level setup
	enterSetupMode();
}

// --------------------------------------------------------------------------
void exitSetupMode() {
	// FIXME - huffa - Ok to reboot here?
	GW_ENTER_CRITICAL(ccrHolder);
	displayMessage(2, "Rebooting", FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
	vTaskDelay(300);
	GW_ENTER_CRITICAL(ccrHolder);
	displayMessage(2, "Rebooting .", FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
	vTaskDelay(300);
	GW_ENTER_CRITICAL(ccrHolder);
	displayMessage(2, "Rebooting . .", FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
	vTaskDelay(300);
	GW_ENTER_CRITICAL(ccrHolder);
	displayMessage(2, "Rebooting . . .", FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
	vTaskDelay(300);
	
	GW_RESET_MCU()
}

// --------------------------------------------------------------------------
void enterSetupMode() {
	
	// Enter setup mode
	setupModeState = eSetupModeStarted;
	
	GW_ENTER_CRITICAL(ccrHolder);
	clearDisplay();
	displayMessage(1, "CHE Setup Mode", FONT_NORMAL);
	displayMessage(2, "Scan setup command", FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
}
