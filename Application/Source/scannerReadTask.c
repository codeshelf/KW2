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

xTaskHandle gScannerReadTask = NULL;

ScanStringType gScanString;
ScanStringLenType gScanStringPos;

// --------------------------------------------------------------------------

void scannerReadTask(void *pvParameters) {

	gwUINT8 ccrHolder;

	for (;;) {

		// Clear the scan string.
		gScanString[0] = NULL;
		gScanStringPos = 0;

		// Flush the RX FIFO.
		Scanner_DEVICE ->CFIFO |= UART_CFIFO_RXFLUSH_MASK;
		Scanner_DEVICE ->SFIFO |= UART_SFIFO_RXUF_MASK;
		// Wait until there are characters in the FIFO
		while ((Scanner_DEVICE ->S1 & UART_S1_RDRF_MASK) == 0) {
			vTaskDelay(1);
		}
		Wait_Waitus(100);

		// Now we have characters - read until there are no more.
		// Do the read in a critical-area-busy-wait loop to make sure we've gotten all characters that will arrive.
		GW_ENTER_CRITICAL(ccrHolder);
		EventTimer_ResetCounter(NULL);
		// If there's no characters in 50ms then stop waiting for more.
		while ((EventTimer_GetCounterValue(NULL) < 200) && (gScanStringPos < MAX_SCAN_STRING_BYTES)) {
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

		// Disable all of the position controllers on the cart.
		// TODO

		// Now send the scan string.
		BufferCntType txBufferNum = 0;//lockTxBuffer();
		createScanCommand(txBufferNum, &gScanString, gScanStringPos);
		transmitPacket(txBufferNum);

	}
}
