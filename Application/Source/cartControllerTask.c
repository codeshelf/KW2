/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */
#include "cartControllerTask.h"
#include "appCartController.h"
#include "commands.h"
#include "radioCommon.h"
#include "remoteMgmtTask.h"
#include "string.h"
#include "Rs485.h"

xTaskHandle			gCartControllerTask;
xQueueHandle		gCartControllerQueue;

portTickType		gLastUserAction;

void clearAllPositions() {
	gwUINT8 message[] = { POS_CTRL_CLEAR, POS_CTRL_ALL_POSITIONS };
	sendRs485Message(message, 2);
}

// --------------------------------------------------------------------------

void cartControllerTask(void *pvParameters) {

	static ScanStringType rs485String;
	static ScanStringLenType rs485StringPos;
	gwUINT8 ccrHolder;

	clearAllPositions();
	
	for (;;) {

		// Clear the RS485 string.
		rs485String[0] = 0;
		rs485StringPos = 0;

		// Wait until there are characters in the FIFO
		while (Rs485_DEVICE ->RCFIFO == 0) {
			vTaskDelay(1);
		}

		// Now we have characters - read until there are no more.
		// Do the read in a critical-area-busy-wait loop to make sure we've gotten all characters that will arrive.
		GW_ENTER_CRITICAL(ccrHolder);
		while ((Rs485_DEVICE ->RCFIFO != 0) && (rs485StringPos < MAX_SCAN_STRING_BYTES)) {
			rs485String[rs485StringPos++] = Rs485_DEVICE->D;
			rs485String[rs485StringPos] = 0;

			// If there's no characters - then wait 2ms to see if more will arrive.
			if ((Rs485_DEVICE->RCFIFO == 0)) {
				// Can't be vTaskDelay b/c we've entered an atomic-critical area with no running OS.
				Wait_Waitms(10);
			}
		}
		GW_EXIT_CRITICAL(ccrHolder);

		// Indicate a user operation
		gLastUserAction = xTaskGetTickCount();

		// Now send the scan string.
		BufferCntType txBufferNum = lockTxBuffer();
		createButtonCommand(txBufferNum, rs485String[1], rs485String[2]);
		transmitPacket(txBufferNum);

	}
}
