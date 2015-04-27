/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "serial.h"
#include "task.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "Wait.h"

uint8_t gCircularBuffer[BUFFER_SIZE + 1];
uint8_t gCircularBufferPos = 0;
uint8_t gCircularBufferSize = 0;


// --------------------------------------------------------------------------

void sendOneChar(UART_MemMapPtr uartRegPtr, UsbDataType data) {

	// Until there is free space in the FIFO don't send a char.
	while (uartRegPtr->TCFIFO >= uartRegPtr->TWFIFO) {
		//vTaskDelay(1);
		Wait_Waitus(25);
	}
	uartRegPtr->D = data;
//	while (uartRegPtr->TCFIFO > uartRegPtr->TWFIFO) {
//		vTaskDelay(1);
//	}
}

// --------------------------------------------------------------------------

void readOneChar(UART_MemMapPtr uartRegPtr, UsbDataPtrType dataPtr) {

	// Loop until the buffer has something in it.
	if (gCircularBufferSize == 0) {
		// Wait until there are characters in the FIFO
		while (uartRegPtr->RCFIFO < 2) {
			vTaskDelay(0);
			// If the status register shows a frame error then clear it by reading S! and D.
			if (uartRegPtr->S1 & UART_S1_FE_MASK) {
				uint8_t clearData = uartRegPtr->D;
			}
		}
		Wait_Waitus(250);
		
//		uint8_t bytesToRead = uartRegPtr->RCFIFO;
//		for (uint8_t byte = 0; byte < bytesToRead; byte++) {
		while (uartRegPtr->RCFIFO > 1) {
			uint8_t uartChar = uartRegPtr ->D;
			//if ((uartRegPtr ->SFIFO & UART_SFIFO_RXUF_MASK) == 0) {
				if ((gCircularBufferPos + gCircularBufferSize) > BUFFER_SIZE) {
					gCircularBuffer[gCircularBufferPos + gCircularBufferSize - BUFFER_SIZE - 1] = uartChar;
				} else {
					gCircularBuffer[gCircularBufferPos + gCircularBufferSize] = uartChar;
				}
				gCircularBufferSize++;
				if (gCircularBufferSize == BUFFER_SIZE) {
					break;
				}
			//}
//			// Weird, screwed up UART behavior doesn't update RCFIFO fast enough.
//			if (uartRegPtr->RCFIFO == 1) {
//				Wait_Waitus(500);
//			}
		}
	}

	// Get the first character out of the buffer.
	*dataPtr = gCircularBuffer[gCircularBufferPos];

	gCircularBufferPos++;
	gCircularBufferSize--;
	if (gCircularBufferPos > BUFFER_SIZE) {
		gCircularBufferPos = 0;
	}
}

// --------------------------------------------------------------------------

void serialTransmitFrame(UART_MemMapPtr uartRegPtr, BufferStoragePtrType inFramePtr, BufferCntType inFrameSize) {

	gwUINT16 charsSent;

	// Send the frame contents to the controller via the serial port.
	// First send the framing character.

	// Send another framing character. (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	sendOneChar(uartRegPtr, END);

	for (charsSent = 0; charsSent < inFrameSize; charsSent++) {

		switch (*inFramePtr) {
			/* if it's the same code as an END character, we send a
			 * special two character code so as not to make the
			 * receiver think we sent an END
			 */
			case END:
				sendOneChar(uartRegPtr, ESC);
				sendOneChar(uartRegPtr, ESC_END);
				break;

				/* if it's the same code as an ESC character,
				 * we send a special two character code so as not
				 * to make the receiver think we sent an ESC
				 */
			case ESC:
				sendOneChar(uartRegPtr, ESC);
				sendOneChar(uartRegPtr, ESC_ESC);
				break;

				/* otherwise, we just send the character
				 */
			default:
				sendOneChar(uartRegPtr, *inFramePtr);
		}
		inFramePtr++;
	}

	// Send another framing character. (For some stupid reason the USB routine doesn't try very hard, so we have to loop until it succeeds.)
	sendOneChar(uartRegPtr, END);
	sendOneChar(uartRegPtr, END);
	
	// Wait until all of the TX bytes have been sent.
	while (uartRegPtr ->TCFIFO > 0) {
		//vTaskDelay(1);
		Wait_Waitus(100);
	}

	GW_WATCHDOG_RESET;
}

// --------------------------------------------------------------------------

BufferCntType serialReceiveFrame(UART_MemMapPtr uartRegPtr, BufferStoragePtrType inFramePtr, BufferCntType inMaxFrameSize) {
	BufferStorageType nextByte;
	BufferCntType bytesReceived = 0;
	gwBoolean frameStarted = FALSE;

	// Loop reading bytes until we put together a whole frame.
	// Make sure not to copy them into the frame if we run out of room.
	while (bytesReceived < inMaxFrameSize) {

		readOneChar(uartRegPtr, &nextByte);

		switch (nextByte) {

			case END:
				if ((frameStarted) && (bytesReceived)) {
					return bytesReceived;
				} else {
					frameStarted = TRUE;
					continue;
					break;
				}

			case ESC:
				readOneChar(uartRegPtr, &nextByte);

				switch (nextByte) {
					case ESC_END:
						nextByte = END;
						break;
					case ESC_ESC:
						nextByte = ESC;
						break;
					}
				break;

			default:
				break;
		}

		if (frameStarted && (bytesReceived < inMaxFrameSize)) {
			inFramePtr[bytesReceived++] = nextByte;
		}
	}
	
}

