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
char mac[] = "000652091481";

extern ELocalStatusType gLocalDeviceState;
extern RadioStateEnum gRadioState;

// --------------------------------------------------------------------------

void clearRJ485RXFIFO() {
	// Flush the RX FIFO.
	Scanner_DEVICE ->CFIFO |= UART_CFIFO_RXFLUSH_MASK;
	Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
}

// --------------------------------------------------------------------------

void readBTResponse() {
	clearRJ485RXFIFO();

	gScanString[0] = NULL;
	gScanStringPos = 0;
	EventTimer_ResetCounter(NULL );
	// If there's no characters in 50ms then stop waiting for more.
	while ((EventTimer_GetCounterValue(NULL ) < 150) && (gScanStringPos < MAX_SCAN_STRING_BYTES)) {
		Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
		Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXOF_MASK;
		if ((Scanner_DEVICE ->S1 & UART_S1_RDRF_MASK) != 0) {
			//if ((Scanner_DEVICE ->SFIFO & UART_SFIFO_RXEMPT_MASK) == 0) {
			gScanString[gScanStringPos++] = Scanner_DEVICE ->D;
			;
			gScanString[gScanStringPos] = NULL;
			EventTimer_ResetCounter(NULL );
			Wait_Waitus(500);
		}
	}
}

// --------------------------------------------------------------------------

bool isBTRemoteConnected() {
	sendOneChar(Scanner_DEVICE, 'G');
	sendOneChar(Scanner_DEVICE, 'K');
	sendOneChar(Scanner_DEVICE, '\r');
	sendOneChar(Scanner_DEVICE, '\n');

	readBTResponse();

	if (strncmp(gScanString, "1", 1) == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

// --------------------------------------------------------------------------

bool isBTRemoteMacAddrCorrect() {
	sendOneChar(Scanner_DEVICE, 'G');
	sendOneChar(Scanner_DEVICE, 'R');
	sendOneChar(Scanner_DEVICE, '\r');
	sendOneChar(Scanner_DEVICE, '\n');

	readBTResponse();

	if (strncmp(gScanString, mac, 12) == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void setBTRemoteMacAddr() {
	int i = 0;

	sendOneChar(Scanner_DEVICE, 'S');
	sendOneChar(Scanner_DEVICE, 'R');
	sendOneChar(Scanner_DEVICE, ',');
	for (i = 0; i < 12; i++) {
		sendOneChar(Scanner_DEVICE, mac[i]);
	}
	sendOneChar(Scanner_DEVICE, '\r');
	sendOneChar(Scanner_DEVICE, '\n');

	Wait_Waitms(100);
}

// --------------------------------------------------------------------------

void resetBTDongle() {
	sendOneChar(Scanner_DEVICE, 'R');
	sendOneChar(Scanner_DEVICE, ',');
	sendOneChar(Scanner_DEVICE, '1');
	sendOneChar(Scanner_DEVICE, '\r');
	sendOneChar(Scanner_DEVICE, '\n');
}

// --------------------------------------------------------------------------

void disconnectBTDevice() {
	sendOneChar(Scanner_DEVICE, 'K');
	sendOneChar(Scanner_DEVICE, ',');
	sendOneChar(Scanner_DEVICE, '\r');
	sendOneChar(Scanner_DEVICE, '\n');
}

// --------------------------------------------------------------------------

void exitBTCommandMode() {
	sendOneChar(Scanner_DEVICE, '-');
	sendOneChar(Scanner_DEVICE, '-');
	sendOneChar(Scanner_DEVICE, '-');
	sendOneChar(Scanner_DEVICE, '\r');
	sendOneChar(Scanner_DEVICE, '\n');
}

// --------------------------------------------------------------------------

void enterBTCommandMode() {
	sendOneChar(Scanner_DEVICE, '$');
	//Wait_Waitms(50);
	sendOneChar(Scanner_DEVICE, '$');
	//Wait_Waitms(50);
	sendOneChar(Scanner_DEVICE, '$');
	//Wait_Waitms(50);
}

// --------------------------------------------------------------------------

bool hasBTDongle() {
	gwUINT8 ccrHolder;

	// Wait for dongle to boot
	vTaskDelay(1000);

	GW_ENTER_CRITICAL(ccrHolder);
	enterBTCommandMode();
	readBTResponse();
	resetBTDongle();
	GW_EXIT_CRITICAL(ccrHolder);

	// Wait for dongle to boot
	vTaskDelay(1000);

	if (strncmp(gScanString, "CMD", 3) == 0) {
		return TRUE;
	} else {
		return FALSE;
	}
}

// --------------------------------------------------------------------------

void connectBTScanner() {
	gwUINT8 ccrHolder;
	int i = 0;

	GW_ENTER_CRITICAL(ccrHolder);

	enterBTCommandMode();
	Wait_Waitms(100);

	// Set the remote mac address
	sendOneChar(Scanner_DEVICE, 'S');
	sendOneChar(Scanner_DEVICE, 'R');
	sendOneChar(Scanner_DEVICE, ',');
	for (i = 0; i < 12; i++) {
		sendOneChar(Scanner_DEVICE, mac[i]);
	}
	sendOneChar(Scanner_DEVICE, '\r');
	sendOneChar(Scanner_DEVICE, '\n');

	Wait_Waitms(100);

	// Connect!
	sendOneChar(Scanner_DEVICE, 'C');
	sendOneChar(Scanner_DEVICE, '\r');
	sendOneChar(Scanner_DEVICE, '\n');

	exitBTCommandMode();
	GW_EXIT_CRITICAL(ccrHolder);

}

/*
 * Note this doesn't work in auto-connect mode
 * because when the dongle connects it drops us out of
 * connect mode. We have no idea when that will happen.
 * 
 // --------------------------------------------------------------------------

 void connectBTScanner() {
 gwUINT8 ccrHolder;
 int i = 0;
 
 
 GW_ENTER_CRITICAL(ccrHolder);

 enterBTCommandMode();
 Wait_Waitms(100);
 
 if (isBTRemoteConnected()){
 if(!isBTRemoteMacAddrCorrect()) {
 
 // Disconnect from current bluetooth device
 disconnectBTDevice();
 
 // Killing BT connection knocks us out out command mode
 enterBTCommandMode();
 Wait_Waitms(100);	
 
 // Set the remote mac address
 setBTRemoteMacAddr();
 
 // Connect!
 sendOneChar(Scanner_DEVICE ,'C');
 sendOneChar(Scanner_DEVICE ,'\r');
 sendOneChar(Scanner_DEVICE ,'\n');
 }
 }

 exitBTCommandMode();
 GW_EXIT_CRITICAL(ccrHolder);

 }
 */
// --------------------------------------------------------------------------
void attemptBTConnection() {
	gwUINT8 ccrHolder;

	// Clear buffer
	clearRJ485RXFIFO();

	// If a BlueTooth dongle is connected set mac address and connect
	if (hasBTDongle()) {
		connectBTScanner();
	}

	// Flush the RX FIFO.
	Scanner_DEVICE ->CFIFO |= UART_CFIFO_RXFLUSH_MASK;
	Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
}

// --------------------------------------------------------------------------

void scannerReadTask(void *pvParameters) {

	gwUINT8 ccrHolder;
	char currChar = NULL;
	ScannerPower_SetVal(ScannerPower_DeviceData );

	//attemptBTConnection();

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
		
		EventTimer_ResetCounter(NULL );
		// If there's no characters in 50ms then stop waiting for more.
		while ((EventTimer_GetCounterValue(NULL ) < 250) && (gScanStringPos < MAX_SCAN_STRING_BYTES)) {
			Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
			Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXOF_MASK;
			//if ((Scanner_DEVICE ->SFIFO & UART_SFIFO_RXEMPT_MASK) == 0) {
			if ((Scanner_DEVICE ->S1 & UART_S1_RDRF_MASK) != 0) {

				currChar = Scanner_DEVICE ->D;
				if (currChar != '\r' && currChar != '\n') {
					gScanString[gScanStringPos++] = currChar; //Scanner_DEVICE ->D;;
					gScanString[gScanStringPos] = NULL;
				}
				EventTimer_ResetCounter(NULL );
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
