/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */
#include "fccTestTask.h"
#include "appFccTest.h"
#include "commands.h"
#include "radioCommon.h"
#include "scannerReadTask.h"
#include "string.h"
#include "Wait.h"
#include "EventTimer.h"
#include "display.h"
#include "serial.h"
#include "eeprom.h"
#include "globals.h"
#include "Watchdog.h"
#include "TransceiverDrv.h"
#include "TransceiverReg.h"

#ifdef RS485
#include "Rs485.h"
#include "Rs485Power.h"
#endif

extern TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];
extern RadioStateEnum gRadioState;

xTaskHandle			gFccSetupTask;
xTaskHandle			gFccTxTask;

portTickType		gLastUserAction;
ScanStringType		gScanString;
BufferCntType		gFccTxBufferNum;
TickType_t			gDelayMs;

char 				gAntennaMsg[20];
char 				gChannelMsg[20];
char 				gPowerMsg[20];
char 				gDelayMsg[20];
char 				gPacketSizeMsg[20];
char 				gPaMsg[20];
char				gDelayStr[20];

// --------------------------------------------------------------------------

void fccTxTask(void *pvParameters) {
	gFccTxBufferNum = lockTxBuffer();
	createAssocReqCommand(gFccTxBufferNum);
	gTxRadioBuffer[gFccTxBufferNum].bufferSize = MAX_PACKET_SIZE;
	gDelayMs = 0;

	// Keep blasting away the same packet with the set delay.
	for (;;) {
		TickType_t ticks = xTaskGetTickCount();
		writeRadioTx(gFccTxBufferNum);
		while(gRadioState == eTx) {
		}
		while ((xTaskGetTickCount() - ticks) < gDelayMs){
		}
	}
}

// --------------------------------------------------------------------------

void fccSetupTask(void *pvParameters) {

	smacErrors_t smacError;
	
	// For the Fcc test disable the Watchdog.
	Watchdog_Disable(Watchdog_DeviceData);
	
	prepUartBus();

	strcpy(gAntennaMsg, "Ant: A1");
	MC1324xDrv_IndirectAccessSPIWrite(ANT_AGC_CTRL, 0x40 /* + 0x02 + 0x01*/);

	strcpy(gPowerMsg, "Power: min");
	smacError = MLMEPAOutputAdjust(gMinOutputPower_c);				

	setRadioChannel(gChannel11_c);
	strcpy(gChannelMsg, "CH11");
	
	gDelayMs = 0;
	strcpy(gDelayMsg, "Delay: 0");

	gTxRadioBuffer[gFccTxBufferNum].bufferSize = MAX_PACKET_SIZE;
	strcpy(gPacketSizeMsg, "Packet: 125");

	displayState();

	for (;;) {
		// Get a scan and change the radio.
		getScan(gScanString);
		vTaskSuspend(gFccTxTask);
		while(gRadioState == eTx) {
		}
		if (strlen(gScanString) > 0) {
			if (strncmp(gScanString, "C%", 2) == 0) {
				setChannel(gScanString);
			} else 	if (strcmp(gScanString, "P%MIN") == 0) {
				strcpy(gPowerMsg, "Power: min");
				smacError = MLMEPAOutputAdjust(gMinOutputPower_c);				
			} else 	if (strcmp(gScanString, "P%MID") == 0) {
				strcpy(gPowerMsg, "Power: mid");
				smacError = MLMEPAOutputAdjust(15);				
			} else 	if (strcmp(gScanString, "P%MAX") == 0) {
				strcpy(gPowerMsg, "Power: max");
				smacError = MLMEPAOutputAdjust(27);				
			} else 	if (strcmp(gScanString, "A%1") == 0) {
				strcpy(gAntennaMsg, "Ant: A1");
				MC1324xDrv_IndirectAccessSPIWrite(ANT_AGC_CTRL, /*0x40 + */0x02);
			} else 	if (strcmp(gScanString, "A%2") == 0) {
				strcpy(gAntennaMsg, "Ant: A2");
				MC1324xDrv_IndirectAccessSPIWrite(ANT_AGC_CTRL, /*0x40 + */ 0x00);
			} else 	if (strcmp(gScanString, "T%ON") == 0) {
				strcpy(gPaMsg, "PA: On");
				uint8_t padCtrl = MC1324xDrv_IndirectAccessSPIRead(ANT_PAD_CTRL) | 0x01;
				MC1324xDrv_IndirectAccessSPIWrite(ANT_PAD_CTRL, padCtrl);
				uint8_t gpioData = MC1324xDrv_IndirectAccessSPIRead(GPIO_DATA) & ~GPIO_PIN2;
//				MC1324xDrv_IndirectAccessSPIWrite(GPIO_DATA, gpioData);
			} else 	if (strcmp(gScanString, "T%OFF") == 0) {
				strcpy(gPaMsg, "PA: Off");
				uint8_t padCtrl = MC1324xDrv_IndirectAccessSPIRead(ANT_PAD_CTRL) & ~0x01;
				MC1324xDrv_IndirectAccessSPIWrite(ANT_PAD_CTRL, padCtrl);
				uint8_t gpioData = MC1324xDrv_IndirectAccessSPIRead(GPIO_DATA) | GPIO_PIN2;
				MC1324xDrv_IndirectAccessSPIWrite(GPIO_DATA, gpioData);
			} else 	if (strncmp(gScanString, "D%", 2) == 0) {
				strncpy(gDelayStr, &gScanString[2], strlen(gScanString) - 1);
				strcpy(gDelayMsg, "Delay: ");
				strcat(gDelayMsg, gDelayStr);
				gDelayMs = atol(gDelayStr);
			} else if (strcmp(gScanString, "S%NORM") == 0) {
				strcpy(gPacketSizeMsg, "Packet: 25");
				gTxRadioBuffer[gFccTxBufferNum].bufferSize = 25;
			} else if (strcmp(gScanString, "S%MAX") == 0) {
				strcpy(gPacketSizeMsg, "Packet: 125");
				gTxRadioBuffer[gFccTxBufferNum].bufferSize = MAX_PACKET_SIZE;
			}
			
			displayState();
		}
		vTaskResume(gFccTxTask);
	}
}

void displayState() {
	clearDisplay();
	displayString(15, 15, gChannelMsg, FONT_ARIAL16);
	displayString(15, 45, gPowerMsg, FONT_ARIAL16);
	displayString(15, 75, gAntennaMsg, FONT_ARIAL16);
	displayString(15, 105, gDelayMsg, FONT_ARIAL16);
	displayString(15, 135, gPacketSizeMsg, FONT_ARIAL16);
	displayString(15, 165, gPaMsg, FONT_ARIAL16);
}

void setChannel(ScanStringType scanString) {
	if (strcmp(scanString, "C%11") == 0) {
		setRadioChannel(gChannel11_c);
		strcpy(gChannelMsg, "CH11");
	} else if (strcmp(scanString, "C%12") == 0) {
		setRadioChannel(gChannel12_c);
		strcpy(gChannelMsg, "CH12");
	} else if (strcmp(scanString, "C%13") == 0) {
		setRadioChannel(gChannel13_c);
		strcpy(gChannelMsg, "CH13");
	} else if (strcmp(scanString, "C%14") == 0) {
		setRadioChannel(gChannel14_c);
		strcpy(gChannelMsg, "CH14");
	} else if (strcmp(scanString, "C%15") == 0) {
		setRadioChannel(gChannel15_c);
		strcpy(gChannelMsg, "CH15");
	} else if (strcmp(scanString, "C%16") == 0) {
		setRadioChannel(gChannel16_c);
		strcpy(gChannelMsg, "CH16");
	} else if (strcmp(scanString, "C%17") == 0) {
		setRadioChannel(gChannel17_c);
		strcpy(gChannelMsg, "CH17");
	} else if (strcmp(scanString, "C%18") == 0) {
		setRadioChannel(gChannel18_c);
		strcpy(gChannelMsg, "CH18");
	} else if (strcmp(scanString, "C%19") == 0) {
		setRadioChannel(gChannel19_c);
		strcpy(gChannelMsg, "CH19");
	} else if (strcmp(scanString, "C%20") == 0) {
		setRadioChannel(gChannel20_c);
		strcpy(gChannelMsg, "CH20");
	} else if (strcmp(scanString, "C%21") == 0) {
		setRadioChannel(gChannel21_c);
		strcpy(gChannelMsg, "CH21");
	} else if (strcmp(scanString, "C%22") == 0) {
		setRadioChannel(gChannel22_c);
		strcpy(gChannelMsg, "CH22");
	} else if (strcmp(scanString, "C%23") == 0) {
		setRadioChannel(gChannel23_c);
		strcpy(gChannelMsg, "CH23");
	} else if (strcmp(scanString, "C%24") == 0) {
		setRadioChannel(gChannel24_c);
		strcpy(gChannelMsg, "CH24");
	} else if (strcmp(scanString, "C%25") == 0) {
		setRadioChannel(gChannel25_c);
		strcpy(gChannelMsg, "CH25");
	} else if (strcmp(scanString, "C%26") == 0) {
		// Channel 26 cannot go above mid power.
		uint8_t power = MC1324xDrv_IndirectAccessSPIRead(PA_PWR);
		if (power > 15) {
			MC1324xDrv_DirectAccessSPIWrite(PA_PWR, 10);
		}
		strcpy(gChannelMsg, "CH26");
		setRadioChannel(gChannel26_c);
	}
}
