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
#include "String.h"
#include "cstdlib"
#include "cstring"

#define	CLEAR		TRUE
#define DONT_CLEAR	FALSE

extern ELocalStatusType gLocalDeviceState;
xTaskHandle gTunerTask = NULL;

TuneStringType gScanTuneString[MAX_DISPLAY_CHARS];

gwUINT16 gTargetRemainder = 0x701d;
gwUINT8 gStartTrim = 0xba;

// --------------------------------------------------------------------------

void tunerTask(void *pvParameters) {
		
	TuneStringType tempString[MAX_DISPLAY_CHARS];

	gLocalDeviceState = eLocalStateTuning;
	
	Watchdog_Disable(NULL);
	EventTimer_Init(EventTimer_DeviceData);
	
	zeroParams();
	clearDisplay();
	gTargetRemainder = (gwUINT16) (strtol(getScanStr("Remainder", "R%", CLEAR), NULL, 0) >> 8);
	gStartTrim = (gwUINT8) ((strtol(getScanStr("Start trim", "T%", CLEAR), NULL, 0)));

	strncpy(gGuid, getScanStr("GUID", "G%", CLEAR), EEPROM_GUID_LEN);
	gGuid[EEPROM_GUID_LEN] = NULL;
	strncpy(gAesKey, getScanStr("AES Key", "A%", CLEAR), EEPROM_AES_KEY_LEN);
	strncpy(gHwVer, getScanStr("HW Version", "H%", CLEAR), EEPROM_HW_VER_LEN);
	displayMessage(1, "Ready To Tune", FONT_NORMAL);
	
	ENABLE_PTA1_TUNE_OUT
	ENABLE_PTD4_TUNE
	tuneRadio();
	DISABLE_PTD4_TUNE

	clearDisplay();
	sprintf(tempString, "Trim: %d", gTrim);
	displayMessage(3, tempString, FONT_NORMAL);
	sprintf(tempString, "GUID: %s", gGuid);
	displayMessage(4, tempString, FONT_NORMAL);
	sprintf(tempString, "AES Key: %s", gAesKey);
	displayMessage(5, tempString, FONT_NORMAL);
	sprintf(tempString, "HW Ver: %s", gHwVer);
	displayMessage(6, tempString, FONT_NORMAL);

	getScanStr("Save Params", "S%", DONT_CLEAR);

	saveParams();
	attemptConnection();
	
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
	// We do this using the dual-edge capture mode of FTM0 channels 4 & 5.
	// This is the DECAPEN mode and it works for inputs on channels n and n+1 where n = 0, 2 or 4.
	// The 1st rising edge into channel 4 will insert the value of the FTM0 counter into CH4V.
	// The 2nd rising edge into channel 4 will insert the value of the FTM0 counter into CH5V.
	// The delta between these two rising edges should be a remainder of 0x6c00 (remainder of 48,000,000 / 65535).
	// There's a small amount of consistent inaccuracy in the PPS that doesn't drift too much from measure-to-measure.
	// There's a small amount of delay for the FTM to process the PPS events.
	// For the above two reasons the amount of remainder that corresponds to an accurately tuned clock is a little more than 0x6c00.
	// The amount of "expected remainder" will depend a lot on the GPS that outputs the PPS.
	// There's probably a somewhat unchanging "expected remainder" for each GPS.  
	// Tho' we should validate this before each manufacturing run!
	
	// Calculate the expected FTM0 "remainder" for the KW2 modules from this formula:
	// Take the remainder of: (PPS width * 32,000,000 / 65536) and multiply it by 65536.
	// You must first settle in the GPS in order for this to work.
	// The GPS must be put into 115200 baud (send $PMTK251,115200*1F) and 10Hz update mode (send $PMTK220,100*2F)
	// The PAH6H GPS won't be accurate until it settles into a PPM period of roughly 0.999995440 that jitters less than 100ns.
	// It can take a good 10-15 minutes to get settled.
	
	Tuner_Init();
	FTM0_SC = 0x80;
	FTM0_C4V = 0;
	FTM0_C5V = 0;
	FTM0_C6V = gTargetRemainder;
	FTM0_MOD = 0xffff;
	FTM0_CNTIN = 0x0000;
	FTM0_CNT = 0x0000;
	// DECAP2 sets up dual-edge capture for counters 4 and 5.  See the KW2 RM for more details.
	FTM0_COMBINE |= (FTM_COMBINE_DECAP2_MASK);
	FTM0_SC = 0x88;
	
	// Loop until there is a 1st event.
	while ((FTM0_STATUS & FTM_STATUS_CH4F_MASK) == 0) {
		GW_WATCHDOG_RESET;
	}

	// Loop until there is a 2nd event.
	while ((FTM0_STATUS & FTM_STATUS_CH5F_MASK) == 0) {
		GW_WATCHDOG_RESET;
	}

	// Wait 1005ms and there'll be a 2nd event.
	//Wait_Waitms(1005);

	if (FTM0_C4V < FTM0_C5V) {
		remainder = FTM0_C5V - FTM0_C4V;
	} else {
		remainder = (0xffff - FTM0_C4V) + FTM0_C5V;
	}
	
	return remainder;
}

// --------------------------------------------------------------------------

void tuneRadio() {
	
	gwUINT8 trim;
	gwUINT16 remainder;
	gwUINT16 displayRemainder;
	char displayTuningStr[7 + EEPROM_TUNING_LEN + 1 + 10];
	gwUINT8 ccrHolder;
		
	displayMessage(1, "Tuning...", FONT_NORMAL);
	
	// Set the modem EXTAL to 32MHz and put the MCU into BPLE mode so that it clocks exactly 32MHz.
	Cpu_SetClockConfiguration(2);
	MC1324xDrv_Set_CLK_OUT_Freq(gCLK_OUT_FREQ_32_MHz);
	
	GW_ENTER_CRITICAL(ccrHolder);
			
	trim = gStartTrim;
	adjustTune(trim);
	remainder = measureTune();
	if (remainder > gTargetRemainder) {
		while ((remainder > gTargetRemainder) && (trim < 255)) {
			adjustTune(++trim);
			remainder = measureTune();
			displayRemainder = remainder;
			sprintf(displayTuningStr, "Trim: %X", trim & 0xff);
			displayMessage(3, displayTuningStr, FONT_NORMAL);
			sprintf(displayTuningStr, "Rem: %X (Goal: %X)", displayRemainder, gTargetRemainder);
			displayMessage(4, displayTuningStr, FONT_NORMAL);
			GW_WATCHDOG_RESET;
		}
	} else if (remainder < gTargetRemainder) {
		while ((remainder < gTargetRemainder) && (trim > 0)) {
			adjustTune(--trim);
			remainder = measureTune();
			displayRemainder = remainder;
			sprintf(displayTuningStr, "Trim: %X", trim & 0xff);
			displayMessage(3, displayTuningStr, FONT_NORMAL);
			sprintf(displayTuningStr, "Rem: %X (Goal: %X)", displayRemainder, gTargetRemainder);
			displayMessage(4, displayTuningStr, FONT_NORMAL);
			GW_WATCHDOG_RESET;
		}
	}
	
	GW_EXIT_CRITICAL(ccrHolder);
	
	// You can now verify the tuning as needed for a manufacturing run.
	// You need to put a breakpoint just above here (before we reset the clock to allow the "save" scan to work right.)
	
	// Prior to the tuning we put the modem in 32MHz clock mode and have the output show up on PTA18.
	// We also set the MCU clock to BLPE mode so that this 32MHz clock comes straight into the FTM0 unchanged.
	// PTA1 is now setup to output an edge-align PWM signal derived from FTM0 which is derived from the trimmed XTAL32 crystal.
	// The PWM period is 65,536 FTM0 clock cycles and the duty cycle is FTM0 clock cycles "remainder" scanned during the setup.
	// This way there will exist a PWM high-low transition that corresponds to a multiple of exactly 32,000,000 FTM ticks.
	// The KW2 can hold the 802.15.4 spec as long as the input crystal is 32MHz +/- 1280 (40ppm)
	// How to verify-measure?
	// On the logic analyzer capture 2 seconds of PWM (PTA1) and the PPS (PTD4).
	// Create a marker A1 on the first PPS rising edge and A2 on the second rising edge.
	// Then create a "measurement" and enable "rising edge count".  
	// Put the first measure point on the first PWM rising edge before the first PPS.
	// Put the second measure point 489 "rising" PWM cycles away.
	// Then create a new marker B1 that's on the first PWM rising edge from the measurement.
	// Put B2 on the first lowering edge after the PWM measurement area.
	// The time values of A1/A2 and B1/B2 should be equal to within 50ns.

	// Set the modem EXTAL back to 50MHz.
	Cpu_SetClockConfiguration(1);
	MC1324xDrv_Set_CLK_OUT_Freq(gCLK_OUT_FREQ_32_MHz);

	gTrim[0] = trim;
	//FTM0_C6V = 0x6C00;
	GW_WATCHDOG_RESET;

}

// --------------------------------------------------------------------------

void zeroParams() {
	memset(gGuid, (uint8_t)0, EEPROM_GUID_LEN);
	memset(gHwVer, (uint8_t)0, EEPROM_HW_VER_LEN);
	memset(gAesKey, (uint8_t)0, EEPROM_AES_KEY_LEN);
}

// --------------------------------------------------------------------------

void saveParams() {
	char displayGuidStr[6 + EEPROM_GUID_LEN + 1];
	char displayTuningStr[7 + EEPROM_TUNING_LEN + 1 + 10];
	gwUINT8 ccrHolder;

	// Write parameters to eeprom
	writeGuid(gGuid);
	writeAESKey(gAesKey);
	writeHWVersion(gHwVer);
	writeTuning(&gTrim);
}

// --------------------------------------------------------------------------

void attemptConnection() {
	gwUINT8 ccrHolder;

	GW_ENTER_CRITICAL(ccrHolder);
	clearDisplay();
	displayMessage(1, "Radio Test Mode", FONT_NORMAL);
	displayMessage(2, "Attempting Connection...", FONT_NORMAL);
	GW_EXIT_CRITICAL(ccrHolder);
	
	gLocalDeviceState = eLocalStateStarted;
	
	while (gLocalDeviceState != eLocalStateRun) {
		GW_WATCHDOG_RESET;
		vTaskDelay(1);
	}	
}

// --------------------------------------------------------------------------

TuneStringPtrType getScanStr(TuneStringPtrType promptStrPtr, TuneStringPtrType prefixStrPtr, gwBoolean shouldClear) {
	
	gwBoolean validScan = FALSE;
	ScannerPower_SetVal(ScannerPower_DeviceData );
	TuneStringType displayStr[MAX_DISPLAY_CHARS];
	gwUINT8 scanCharCount;
	gwUINT8 ccrHolder;

	GW_WATCHDOG_RESET;
	
	if (shouldClear) {
		clearDisplay();
	}
	while (!validScan) {

		strcpy(displayStr, "Scan: ");
		strcat(displayStr, promptStrPtr);
		displayMessage(1, displayStr, FONT_NORMAL);

		// Clear the scan string.
		gScanTuneString[0] = NULL;

		// Flush the RX FIFO.
		Scanner_DEVICE ->CFIFO |= UART_CFIFO_RXFLUSH_MASK;
		Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
		// Wait until there are characters in the FIFO
		while ((Scanner_DEVICE ->S1 & UART_S1_RDRF_MASK) == 0) {
			GW_WATCHDOG_RESET;
			vTaskDelay(1);
		}
		Wait_Waitus(50);

		// Now we have characters - read until there are no more.
		// Do the read in a critical-area-busy-wait loop to make sure we've gotten all characters that will arrive.
		GW_ENTER_CRITICAL(ccrHolder);
		EventTimer_ResetCounter(EventTimer_DeviceData);
		// If there's no characters in 50ms then stop waiting for more.
		scanCharCount = 0;
		while ((EventTimer_GetCounterValue(EventTimer_DeviceData) < 150) && (scanCharCount < MAX_SCAN_STRING_BYTES)) {
			Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
			Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXOF_MASK;
			if ((Scanner_DEVICE ->S1 & UART_S1_RDRF_MASK) != 0) {
				gScanTuneString[scanCharCount++] = Scanner_DEVICE ->D;
				gScanTuneString[scanCharCount] = NULL;
				EventTimer_ResetCounter(EventTimer_DeviceData);
				Wait_Waitus(500);
			}
			GW_WATCHDOG_RESET;
		}
		
		gwUINT8 len = strlen(gScanTuneString);
		gwUINT8 prefixLen = strlen(prefixStrPtr);
		if ((len > 0) && (strncmp(prefixStrPtr, gScanTuneString, prefixLen) == 0)) {
			if (len > prefixLen) {
				memcpy(gScanTuneString, &gScanTuneString[prefixLen], len - prefixLen);
				validScan = TRUE;
			}
		}
		
		if (!validScan) {
			displayMessage(3, "Invalid scan.  Scan again.", FONT_NORMAL);
		}
		
		GW_EXIT_CRITICAL(ccrHolder);
	}
	
	return ((TuneStringPtrType) &gScanTuneString);
}


