/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */
#include "cheControllerTask.h"
#include "appCheController.h"
#include "commands.h"
#include "radioCommon.h"
#include "remoteMgmtTask.h"
#include "string.h"
#include "Rs485.h"
#include "Wait.h"
#include "EventTimer.h"

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
	
	sendLcdMessage(CLEAR_DISPLAY, strlen(CLEAR_DISPLAY));
	sendLcdMessage("DISCONNECTED", 12);

	for (;;) {

		// Clear the RS485 string.
		rs485String[0] = 0;
		rs485StringPos = 0;

		// Flush the RX FIFO.
		Rs485_DEVICE ->CFIFO |= UART_CFIFO_RXFLUSH_MASK;
		Rs485_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
		// Wait until there are characters in the FIFO
		while ((Rs485_DEVICE ->RCFIFO) == 0) {
			vTaskDelay(1);
		}
		Wait_Waitus(750);

		// Now we have characters - read until there are no more.
		// Do the read in a critical-area-busy-wait loop to make sure we've gotten all characters that will arrive.
		GW_ENTER_CRITICAL(ccrHolder);
		EventTimer_ResetCounter(NULL);
		// If there's no characters in 50ms then stop waiting for more.
		while ((EventTimer_GetCounterValue(NULL) < 50) && (rs485StringPos < MAX_SCAN_STRING_BYTES)) {
			Rs485_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
			if ((Rs485_DEVICE ->SFIFO & UART_SFIFO_RXEMPT_MASK) == 0) {
				rs485String[rs485StringPos++] = Rs485_DEVICE ->D;
				rs485String[rs485StringPos] = NULL;
				EventTimer_ResetCounter(NULL);
				Wait_Waitus(750);
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
