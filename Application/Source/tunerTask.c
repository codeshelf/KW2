/*
 FlyWeight
  Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "tunerTask.h"
#include "SMAC.h"
#include "task.h"
#include "queue.h"
#include "commands.h"
#include "commandTypes.h"
#include "Wait.h"
#include "display.h"
#include "scannerReadTask.h"
#include "Scanner.h"
#include "EventTimer.h"
#include "ScannerPower.h"
#include "eeprom.h"

#define DEFAULT_CRYSTAL_TRIM	0xff
#define PERFECT_REMAINDER		0x6b06
#define PROMPT_DELAY_TIME		2000

extern ELocalStatusType gLocalDeviceState;

void adjustTune(gwUINT8 trim);
gwUINT16 measureTune();

xTaskHandle gTunerTask = NULL;

unsigned char aes_key[EEPROM_AES_KEY_LEN];
unsigned char hw_ver[EEPROM_HW_VER_LEN];
unsigned char guid[EEPROM_GUID_LEN];
unsigned char guidStr[EEPROM_GUID_LEN + 1];
gwUINT8 tune_param;
gwUINT8 ccrHolder;

ELocalSetupType setupModeState;
ScanStringType gScanString;
ScanStringLenType gScanStringPos;

// --------------------------------------------------------------------------

void tunerTask(void *pvParameters) {
	gLocalDeviceState = eLocalStateTuning;
	
	ENABLE_DISPLAY
	GW_ENTER_CRITICAL(ccrHolder);
	clearDisplay();
	displayMessage(1, "Tuning...", FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
	
	ENABLE_TUNER
	tuneRadio();
	
	ENABLE_DISPLAY
	scanParams();
	
	for(;;){}
}

// --------------------------------------------------------------------------

void adjustTune(gwUINT8 trim) {
	MLMEXtalAdjust(trim); 
	// Wait for the clocks to stabilize.
	Wait_Waitms(200);
}

// --------------------------------------------------------------------------

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

// --------------------------------------------------------------------------

void tuneRadio() {
	
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
	
	tune_param = trim;
}

// --------------------------------------------------------------------------

void scanParams() {
	
	zeroParams();
	enterSetupMode();
	
	gwUINT8 ccrHolder;
	ScannerPower_SetVal(ScannerPower_DeviceData);
	
	for (;;) {

		// Clear the scan string.
		gScanString[0] = NULL;
		gScanStringPos = 0;

		// Flush the RX FIFO.
		Scanner_DEVICE ->CFIFO |= UART_CFIFO_RXFLUSH_MASK;
		Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
		// Wait until there are characters in the FIFO
		while ((Scanner_DEVICE ->S1 & UART_S1_RDRF_MASK) == 0) {
			vTaskDelay(1);
		}
		Wait_Waitus(50);

		// Now we have characters - read until there are no more.
		// Do the read in a critical-area-busy-wait loop to make sure we've gotten all characters that will arrive.
		GW_ENTER_CRITICAL(ccrHolder);
		EventTimer_ResetCounter(NULL);
		// If there's no characters in 50ms then stop waiting for more.
		while ((EventTimer_GetCounterValue(NULL) < 150) && (gScanStringPos < MAX_SCAN_STRING_BYTES)) {
			Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
			Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXOF_MASK;
			if ((Scanner_DEVICE ->S1 & UART_S1_RDRF_MASK) != 0) {
				gScanString[gScanStringPos++] = Scanner_DEVICE ->D;;
				gScanString[gScanStringPos] = NULL;
				EventTimer_ResetCounter(NULL);
				Wait_Waitus(500);
			}
		}
		GW_EXIT_CRITICAL(ccrHolder);

		// Now send the scan string.
		if (strlen(gScanString) > 0) {
			if (isSetupCommand()) {
				handleSetupCommand();
			} else {
				handleSetupScan();
			}
		}
	}
}

// --------------------------------------------------------------------------

void zeroParams() {
	memset(guid, (uint8_t)0, EEPROM_GUID_LEN);
	memset(hw_ver, (uint8_t)0, EEPROM_HW_VER_LEN);
	memset(aes_key, (uint8_t)0, EEPROM_AES_KEY_LEN);
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
	
	if (setupModeState == eSetupModeGetGuidAes) {
		getGuidAes();
	}
	else if (setupModeState == eSetupModeGetHWver) {
		getHwVersion();
	}
}

// --------------------------------------------------------------------------

void handleSetupCommand() {
	if (strcmp(gScanString, "S%CLEAR") == 0) {
		clearParams();
	}
	else if (strcmp(gScanString, "S%SAVE") == 0) {
		saveParams();
	} 
	else if (strcmp(gScanString, "S%ENTER") == 0) {
		attemptConnection();
	}
}

// --------------------------------------------------------------------------

void enterSetupMode() {
	
	// Next expected scan is GUID
	setupModeState = eSetupModeGetGuidAes;
	
	GW_ENTER_CRITICAL(ccrHolder);
	clearDisplay();
	displayMessage(1, "KW2 Setup Mode", FONT_NORMAL);
	displayMessage(2, "Scan GUID/AES", FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
}

// --------------------------------------------------------------------------

void clearParams() {

	zeroParams();
	
	GW_ENTER_CRITICAL(ccrHolder);
	clearDisplay();
	displayMessage(1, "KW2 Setup Mode", FONT_NORMAL);
	displayMessage(2, "Cleared params", FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
	vTaskDelay(1500);
	
	enterSetupMode();
}

// --------------------------------------------------------------------------

void saveParams() {
	
	char * guidStrHdr = "GUID: ";
	char displayGuidStr[strlen(guidStrHdr) + EEPROM_GUID_LEN + 1];
	
	char *tuningStrHdr = "Tuning: ";
	char displayTuningStr[strlen(tuningStrHdr) + EEPROM_TUNING_LEN + 1];
	
	if (setupModeState == eSetupModeWaitingForSave) {
		
		// Write parameters to eeprom
		writeGuid(guid);
		writeAESKey(aes_key);
		writeHWVersion(hw_ver);
		writeTuning(&tune_param);
		
		sprintf(displayTuningStr,"%s %X", tuningStrHdr, tune_param);
		displayTuningStr[EEPROM_TUNING_LEN] = NULL;
		
		strncpy(displayGuidStr, guidStrHdr, strlen(guidStrHdr));
		strncat(displayGuidStr, guid, EEPROM_GUID_LEN);
		displayGuidStr[EEPROM_GUID_LEN] = NULL;
		
		GW_ENTER_CRITICAL(ccrHolder);
		clearDisplay();
		displayMessage(1, "KW2 Setup Mode ", FONT_NORMAL);
		displayMessage(2, displayGuidStr, FONT_NORMAL);
		displayMessage(3, displayTuningStr, FONT_NORMAL);
		displayMessage(4, "Scan ENTER to continue...", FONT_NORMAL);
		GW_EXIT_CRITICAL(ccrHolder);
		
		setupModeState = eSetupModeParamsSaved;
	}
}

// --------------------------------------------------------------------------

void getGuidAes() {
	
	if (gScanString[0] == 'G' && gScanString[1] == '%') {
		
		strncpy(guidStr, gScanString+2, EEPROM_GUID_LEN);
		guidStr[EEPROM_GUID_LEN] = NULL;
		
		GW_ENTER_CRITICAL(ccrHolder);
		displayMessage(2, "Scanned GUID/AES", FONT_NORMAL);
		displayMessage(3, guidStr, FONT_NORMAL);
		GW_EXIT_CRITICAL(ccrHolder);
		vTaskDelay(PROMPT_DELAY_TIME);
	
		strncpy(guid, gScanString + 2, EEPROM_GUID_LEN);
		
		strncpy(aes_key, gScanString + 2 + EEPROM_GUID_LEN, EEPROM_AES_KEY_LEN);
		
		// Next we want the HW version
		setupModeState = eSetupModeGetHWver;
		GW_ENTER_CRITICAL(ccrHolder);
		clearDisplay();
		displayMessage(1, "KW2 Setup Mode", FONT_NORMAL);
		displayMessage(2, "Scan Hardware Version", FONT_NORMAL);
		GW_EXIT_CRITICAL(ccrHolder);
	} else {
		GW_ENTER_CRITICAL(ccrHolder);
		displayMessage(3, "Invaild Scan", FONT_NORMAL);
		GW_EXIT_CRITICAL(ccrHolder);
		
		vTaskDelay(PROMPT_DELAY_TIME);
		
		GW_ENTER_CRITICAL(ccrHolder);
		displayMessage(3, "", FONT_NORMAL);
		GW_EXIT_CRITICAL(ccrHolder);
	}
}

// --------------------------------------------------------------------------

void getHwVersion() {
	
	if (gScanString[0] == 'H' && gScanString[1] == '%') {
		char guidDispStr[6 + EEPROM_GUID_LEN + 1];
		strcpy(guidDispStr, "GUID: ");
		strcat(guidDispStr, guidStr);
		
		GW_ENTER_CRITICAL(ccrHolder);
		clearDisplay();
		displayMessage(1, "KW2 Setup Mode", FONT_NORMAL);
		displayMessage(2, "Scanned HW Version", FONT_NORMAL);
		displayMessage(3, gScanString + 2, FONT_NORMAL);
		GW_EXIT_CRITICAL(ccrHolder);
		vTaskDelay(PROMPT_DELAY_TIME);
		
		strncpy(hw_ver, gScanString + 2, EEPROM_HW_VER_LEN);
		
		// Next we want the AES key
		setupModeState = eSetupModeWaitingForSave;
		GW_ENTER_CRITICAL(ccrHolder);
		clearDisplay();
		displayMessage(1, "KW2 Setup Mode", FONT_NORMAL);
		displayMessage(2, "Setup complete", FONT_NORMAL);
		displayMessage(3, guidDispStr, FONT_NORMAL);
		displayMessage(6, "Scan SAVE or CLEAR", FONT_NORMAL);
		GW_EXIT_CRITICAL(ccrHolder);
		
	} else {
		GW_ENTER_CRITICAL(ccrHolder);
		displayMessage(3, "Invaild Scan", FONT_NORMAL);
		GW_EXIT_CRITICAL(ccrHolder);
		
		vTaskDelay(PROMPT_DELAY_TIME);
		
		GW_ENTER_CRITICAL(ccrHolder);
		displayMessage(3, "", FONT_NORMAL);
		GW_EXIT_CRITICAL(ccrHolder);
	}
}

// --------------------------------------------------------------------------

void attemptConnection() {
	if (setupModeState == eSetupModeParamsSaved) {
		
	}
}
