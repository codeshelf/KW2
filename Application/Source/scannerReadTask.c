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

xTaskHandle gScannerReadTask = NULL;

ScanStringType gScanString;
ScanStringLenType gScanStringPos;

extern ELocalStatusType gLocalDeviceState;

// --------------------------------------------------------------------------

void clearRJ485RXFIFO(){
	// Flush the RX FIFO.
	Scanner_DEVICE ->CFIFO |= UART_CFIFO_RXFLUSH_MASK;
	Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
}

// --------------------------------------------------------------------------

void readBTResponse() {
	gwUINT8 ccrHolder;

	GW_ENTER_CRITICAL(ccrHolder);
	gScanString[0] = NULL;
	gScanStringPos = 0;
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
}

// --------------------------------------------------------------------------

bool hasBTDongle() {
	gwUINT8 ccrHolder;
	vTaskDelay(500);
	
	GW_ENTER_CRITICAL(ccrHolder);
	sendOneChar(Scanner_DEVICE ,'$');
	sendOneChar(Scanner_DEVICE ,'$');
	sendOneChar(Scanner_DEVICE ,'$');
	GW_EXIT_CRITICAL(ccrHolder);
	
	readBTResponse();
	
	if(strncmp(gScanString, "CMD", 3) == 0){
		return TRUE;
	} else {
		return FALSE;
	}
}

// --------------------------------------------------------------------------

void connectBTScanner() {
	
	gwUINT8 ccrHolder;
	int i = 0;
	char mac[] = "000652091481";
	
	// Set remote MAC address
	GW_ENTER_CRITICAL(ccrHolder);
	sendOneChar(Scanner_DEVICE ,'S');
	sendOneChar(Scanner_DEVICE ,'R');
	sendOneChar(Scanner_DEVICE ,',');
	
	for(i=0; i<12; i++) {
		sendOneChar(Scanner_DEVICE , mac[i]);
	}
	sendOneChar(Scanner_DEVICE ,'\r');
	sendOneChar(Scanner_DEVICE ,'\n');
	
	readBTResponse();
	
	// Connect to remote MAC address
	sendOneChar(Scanner_DEVICE ,'C');
	sendOneChar(Scanner_DEVICE ,'\r');
	sendOneChar(Scanner_DEVICE ,'\n');
	
	sendOneChar(Scanner_DEVICE ,'S');
	sendOneChar(Scanner_DEVICE ,'M');
	sendOneChar(Scanner_DEVICE ,',');
	sendOneChar(Scanner_DEVICE ,'2');
	sendOneChar(Scanner_DEVICE ,'\r');
	sendOneChar(Scanner_DEVICE ,'\n');
	
	readBTResponse();
	sendOneChar(Scanner_DEVICE ,'-');
	sendOneChar(Scanner_DEVICE ,'-');
	sendOneChar(Scanner_DEVICE ,'-');
	sendOneChar(Scanner_DEVICE ,'\r');
	sendOneChar(Scanner_DEVICE ,'\n');
	
	GW_EXIT_CRITICAL(ccrHolder);
}

// --------------------------------------------------------------------------

int attemptBTConnection() {
	gwUINT8 ccrHolder;
	int BTDongleConnected = 0;
	
	// Clear buffer
	clearRJ485RXFIFO();

	// If a BlueTooth dongle is connected set mac address and connect
	if (hasBTDongle()) {
		connectBTScanner();
	}
}

// --------------------------------------------------------------------------

void scannerReadTask(void *pvParameters) {

	gwUINT8 ccrHolder;
	char	currChar = NULL;
	ScannerPower_SetVal(ScannerPower_DeviceData);
	
	//
	attemptBTConnection();
	
	// Pause until associated.
	while (gLocalDeviceState != eLocalStateRun) {
		vTaskDelay(100);
	}
	
	

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
				
				currChar = Scanner_DEVICE ->D;
				if (currChar != '\r' && currChar != '\n') {
					gScanString[gScanStringPos++] = currChar; //Scanner_DEVICE ->D;;
					gScanString[gScanStringPos] = NULL;
				}
				EventTimer_ResetCounter(NULL);
				Wait_Waitus(500);
			}
		}
		GW_EXIT_CRITICAL(ccrHolder);

		// Disable all of the position controllers on the cart.
		// TODO

		// Now send the scan string.
		if (strlen(gScanString) > 0) {
			BufferCntType txBufferNum = 0;//lockTxBuffer();
			createScanCommand(txBufferNum, &gScanString, gScanStringPos);
			transmitPacket(txBufferNum);
		}

	}
}
