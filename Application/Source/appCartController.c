/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

// --------------------------------------------------------------------------
// Kernel includes

#include "appCartController.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "remoteRadioTask.h"
#include "remoteMgmtTask.h"
#include "cartControllerTask.h"
#include "scannerReadTask.h"
#include "commands.h"
#include "string.h"
#include "stdlib.h"
#include "SMAC_Interface.h"
#include "Wait.h"
#include "Lcd.h"
#include "Rs485.h"
#include "serial.h"
#include "smacGlue.h"
#include "TransceiverDrv.h"

// --------------------------------------------------------------------------
// Globals
portTickType gLastPacketReceivedTick;

extern xTaskHandle gRadioReceiveTask;
extern xTaskHandle gRadioTransmitTask;
extern xTaskHandle gSerialReceiveTask;
extern xTaskHandle gRemoteManagementTask;
extern xTaskHandle gCartControllerTask;
extern xTaskHandle gScannerReadTask;

extern xQueueHandle gRadioTransmitQueue;
extern xQueueHandle gRadioReceiveQueue;
extern xQueueHandle gRemoteMgmtQueue;

extern ELocalStatusType gLocalDeviceState;

extern DisplayStringType gDisplayDataLine[2];
extern DisplayStringLenType gDisplayDataLineLen[2];
extern DisplayStringLenType gDisplayDataLinePos[2];

// --------------------------------------------------------------------------

void lcdScrollIsr() {
	gwUINT8 error;
	gwUINT8 remainingSpace;

	for (int line = 0; line < 4; ++line) {

		if (gDisplayDataLineLen[line] > DISPLAY_WIDTH) {
			if (line == 0) {
				error = sendDisplayMessage(LINE1_FIRST_POS, strlen(LINE1_FIRST_POS));
			} else if (line == 1) {
				error = sendDisplayMessage(LINE2_FIRST_POS, strlen(LINE2_FIRST_POS));
			} else if (line == 2) {
				error = sendDisplayMessage(LINE3_FIRST_POS, strlen(LINE3_FIRST_POS));
			} else if (line == 3) {
				error = sendDisplayMessage(LINE4_FIRST_POS, strlen(LINE4_FIRST_POS));
			}
			error = sendDisplayMessage(&(gDisplayDataLine[line][gDisplayDataLinePos[line]]),
			        getMin(DISPLAY_WIDTH, (gDisplayDataLineLen[line] - gDisplayDataLinePos[line])));

			// If there's space at the end of the display then restart from the beginning of the message.
			if ((gDisplayDataLineLen[line] - gDisplayDataLinePos[line]) < DISPLAY_WIDTH) {
				remainingSpace = DISPLAY_WIDTH - (gDisplayDataLineLen[line] - gDisplayDataLinePos[line]);
				error = sendDisplayMessage("   ", getMin(3, remainingSpace));
				if (remainingSpace > 3) {
					error = sendDisplayMessage(&(gDisplayDataLine[line][0]), remainingSpace - 3);
				}
			}

			gDisplayDataLinePos[line] += 3;
			if (gDisplayDataLinePos[line] >= gDisplayDataLineLen[line]) {
				gDisplayDataLinePos[line] = 0;
			}
		}
	}
}

// --------------------------------------------------------------------------

gwUINT8 sendLcdMessage(char* msgPtr, gwUINT8 msgLen) {

	gwUINT16 charsSent;

	for (charsSent = 0; charsSent < msgLen; charsSent++) {
		sendOneChar(Lcd_DEVICE, msgPtr);
		msgPtr++;
	}

	// Wait until all of the TX bytes have been sent.
	while (Lcd_DEVICE ->TCFIFO > 0) {
		vTaskDelay(1);
	}

	return 0;
}

// --------------------------------------------------------------------------

gwUINT8 sendRs485Message(char* msgPtr, gwUINT8 msgLen) {

	gwUINT16 charsSent;

	RS485_TX_ON
	serialTransmitFrame(Rs485_DEVICE, msgPtr, msgLen);
	vTaskDelay(25);
	RS485_TX_OFF

	return 0;
}

// --------------------------------------------------------------------------

void startApplication(void) {

	MC1324xDrv_SPIInit();
	MLMERadioInit();
	MLMESetPromiscuousMode(gPromiscuousMode_d);
	MLMESetChannelRequest(DEFAULT_CHANNEL);
	MLMEPAOutputAdjust(DEFAULT_POWER);
	MLMEXtalAdjust(DEFAULT_CRYSTAL_TRIM); 

	LCD_OFF
	Wait_Waitms(50);
	LCD_ON

	Wait_Waitms(750);

	//sendLcdMessage(CLEAR_DISPLAY, strlen(CLEAR_DISPLAY));
	//sendLcdMessage("DISCONNECTED", 12);

	gLocalDeviceState = eLocalStateStarted;
	MLMEFEGainAdjust(15);

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (signed portCHAR *) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask);
	xTaskCreate(radioReceiveTask, (signed portCHAR *) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask);
	xTaskCreate(remoteMgmtTask, (signed portCHAR *) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gRemoteManagementTask);
	xTaskCreate(cartControllerTask, (signed portCHAR *) "LED", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gCartControllerTask);
	xTaskCreate(scannerReadTask, (signed portCHAR *) "Scan", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gScannerReadTask);

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	vQueueAddToRegistry(gRadioReceiveQueue, (signed char*)"RxQ");

	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	vQueueAddToRegistry(gRadioTransmitQueue, (signed char*)"TxQ");

	gRemoteMgmtQueue = xQueueCreate(REMOTE_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	vQueueAddToRegistry(gRemoteMgmtQueue, (signed char*)"MgmtQ");

	// Set the state to running
	gLocalDeviceState = eLocalStateStarted;

	// Initialize the network check.
	gLastPacketReceivedTick = xTaskGetTickCount();
}

// --------------------------------------------------------------------------

void vApplicationIdleHook(void) {

//	GW_WATCHDOG_RESET;
//
//	// If we haven't received a packet in by timeout seconds then reset.
//	portTickType ticks = xTaskGetTickCount();
//	if (ticks > (gLastPacketReceivedTick + kNetCheckTickCount)) {
//		GW_RESET_MCU()
//		;
//	}
}
