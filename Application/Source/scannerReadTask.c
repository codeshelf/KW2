/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */
#include "scannerReadTask.h"
#include "task.h"
#include "queue.h"
#include "commands.h"
#include "Scanner.h"
#include "Wait.h"
#include "EventTimer.h"
#include "ScannerPower.h"
#include "Serial.h"
#include "String.h"

xTaskHandle gScannerReadTask = NULL;

ScanStringType gScanString;
ScanStringLenType gScanStringPos;

extern ELocalStatusType gLocalDeviceState;
extern RadioStateEnum gRadioState;

// --------------------------------------------------------------------------

void scannerReadTask(void *pvParameters) {

	gwUINT8 ccrHolder;
	char currChar = NULL;
	ScannerPower_SetVal(ScannerPower_DeviceData);

	// Pause until associated.
	while (gLocalDeviceState != eLocalStateRun) {
		vTaskDelay(5);
	}

	// Flush the RX FIFO.
	Scanner_DEVICE ->CFIFO |= UART_CFIFO_RXFLUSH_MASK;
	Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;

	for (;;) {

		// Clear the scan string.
		gScanString[0] = NULL;
		gScanStringPos = 0;
		currChar = NULL;

		// Flush the RX FIFO.
		Scanner_DEVICE ->CFIFO |= UART_CFIFO_RXFLUSH_MASK;
		Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
		// Wait until there are characters in the FIFO
		while ((Scanner_DEVICE ->S1 & UART_S1_RDRF_MASK) == 0) {
			//while ((Scanner_DEVICE ->RCFIFO) == 0) {
			vTaskDelay(1);
		}
		GW_ENTER_CRITICAL(ccrHolder);
		Wait_Waitus(50);

		// Now we have characters - read until there are no more.
		// Do the read in a critical-area-busy-wait loop to make sure we've gotten all characters that will arrive.
		
		EventTimer_ResetCounter(EventTimer_DeviceData);
		// If there's no characters in 50ms then stop waiting for more.
		while ((EventTimer_GetCounterValue(EventTimer_DeviceData) < 150) && (gScanStringPos < MAX_SCAN_STRING_BYTES)) {
			Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
			Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXOF_MASK;
			//if ((Scanner_DEVICE ->SFIFO & UART_SFIFO_RXEMPT_MASK) == 0) {
			if ((Scanner_DEVICE ->S1 & UART_S1_RDRF_MASK) != 0) {

				currChar = Scanner_DEVICE ->D;
				if (currChar != '\r' && currChar != '\n') {
					gScanString[gScanStringPos++] = currChar; //Scanner_DEVICE ->D;;
					gScanString[gScanStringPos] = NULL;
				}
				EventTimer_ResetCounter(EventTimer_DeviceData);
				Wait_Waitus(500);
			}
		}
		
		
		// Disable all of the position controllers on the cart.
		// TODO
		
		//Wait_Waitms(5);
		GW_EXIT_CRITICAL(ccrHolder);

		// Now send the scan string.
		if (strlen(gScanString) > 0) {
			BufferCntType txBufferNum = lockTxBuffer();
			createScanCommand(txBufferNum, &gScanString, gScanStringPos);
			//transmitPacket(txBufferNum);
			if (transmitPacket(txBufferNum)) {
				while (gRadioState == eTx) {
					vTaskDelay(1);
				}
			}
		}
	}
}

//// --------------------------------------------------------------------------
//
//void sendLineToScanner(DisplayStringType inString, DisplayStringLenType inLen) {
//	int i = 0;
//	UsbDataType nextChar = NULL;
//	
//	sendOneChar(Scanner_DEVICE, '|');
//	for (i=0; i < inLen; i++ ) {
//		nextChar = inString[i];
//		sendOneChar(Scanner_DEVICE, nextChar);
//	}
//	
//	sendOneChar(Scanner_DEVICE, '\r');
//	sendOneChar(Scanner_DEVICE, '\n');
//}
//
//// --------------------------------------------------------------------------
//
//void clearScannerDisplay() {
//	sendOneChar(Scanner_DEVICE, '|');
//	sendOneChar(Scanner_DEVICE, 'c');
//	sendOneChar(Scanner_DEVICE, 'l');
//	sendOneChar(Scanner_DEVICE, 'e');
//	sendOneChar(Scanner_DEVICE, 'a');
//	sendOneChar(Scanner_DEVICE, 'r');
//	sendOneChar(Scanner_DEVICE, '\r');
//	sendOneChar(Scanner_DEVICE, '\n');
//}
