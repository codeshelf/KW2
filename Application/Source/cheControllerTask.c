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
#include "display.h"
#include "serial.h"
#include "Rs485Power.h"
#include "Rs485.h"
#include "eeprom.h"

xTaskHandle			gCartControllerTask;
xQueueHandle		gCartControllerQueue;

portTickType		gLastUserAction;

// --------------------------------------------------------------------------

gwUINT8 sendRs485Message(char* msgPtr, gwUINT8 msgLen) {

	RS485_TX_ON;
	serialTransmitFrame(Rs485_DEVICE, msgPtr, msgLen);
	vTaskDelay(25);
	RS485_TX_OFF;

	return 0;
}

void clearAllPositions() {
	gwUINT8 message[] = { POS_CTRL_CLEAR, POS_CTRL_ALL_POSITIONS };
	sendRs485Message(message, 2);
}

// --------------------------------------------------------------------------

void cheControllerTask(void *pvParameters) {

	static ScanStringType rs485String;
	static ScanStringLenType rs485StringPos;
	char msg[20];
	gwUINT8 uartStatus1;
	gwUINT8 ccrHolder;
	
	clearAllPositions();

	// Turn on the Rs485 bus, but disable the UART RE until it's stable.  (To avoid frame errors.)
	Rs485_DEVICE->C2 &= ~UART_C2_RE_MASK;
	Rs485Power_SetVal(Rs485Power_DeviceData);
	vTaskDelay(5);
	Rs485_DEVICE->C2 |= UART_C2_RE_MASK;
	RS485_TX_ON;
	sendOneChar(Rs485_DEVICE, END);
	sendOneChar(Rs485_DEVICE, END);
	RS485_TX_OFF;

	GW_ENTER_CRITICAL(ccrHolder);
	clearDisplay();
	displayMessage(1, "CONNECTING...", FONT_NORMAL);
	
	//uint8_t data[] = {5};
	//writeEepromData(0, data, 1);
	
	
	//uint8_t data2[1];
	//readEepromData(0, data2, 1);
	
	//char_t data3[1];
	//data3[0] = (char_t) data2[0];
	//displayMessage(2, data3, FONT_NORMAL);


	strcpy(msg, "HW v");
	strcat(msg, STR(HARDWARE_VERSION));
	strcat(msg, " FW v");
	strcat(msg, STR(FIRMWARE_VERSION));
	//displayMessage(2, msg, FONT_NORMAL);
	
	strcpy(msg, "GUID ");
	strcat(msg, STR(GUID));
	displayMessage(3, msg, FONT_NORMAL);
	
	displayCodeshelfLogo(48, 135);
	
	GW_EXIT_CRITICAL(ccrHolder);

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
			// If the status register shows a frame error then clear it by reading S! and D.
			if (Rs485_DEVICE->S1 & UART_S1_FE_MASK) {
				uint8_t clearData = Rs485_DEVICE->D;
			}
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
		BufferCntType txBufferNum = 0;//lockTxBuffer();
		createButtonCommand(txBufferNum, rs485String[1], rs485String[2]);
		transmitPacket(txBufferNum);

	}
}
