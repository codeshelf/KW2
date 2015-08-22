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
#include "globals.h"

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

	strcpy(msg, "HW v");
	strcat(msg, gHwVer);
	strcat(msg, " FW v");
	strcat(msg, STR(FIRMWARE_VERSION));
	displayMessage(2, msg, FONT_NORMAL);
	
	strcpy(msg, "GUID ");
	strcat(msg, gGuid);
	displayMessage(3, msg, FONT_NORMAL);
	
	displayCodeshelfLogo(48, 135);
	
	GW_EXIT_CRITICAL(ccrHolder);

	for (;;) {

		// Clear the RS485 string.
		rs485String[0] = 0;
		rs485StringPos = 0;
		
		serialReceiveFrame(Rs485_DEVICE, &rs485String, MAX_SCAN_STRING_BYTES);

		// Indicate a user operation
		gLastUserAction = xTaskGetTickCount();

		// Now send the scan string.
		BufferCntType txBufferNum = lockTxBuffer();
		createButtonCommand(txBufferNum, rs485String[1], rs485String[2]);
		transmitPacket(txBufferNum);
	}
}
