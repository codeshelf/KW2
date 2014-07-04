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

uint8_t gCircularBuffer[BUFFER_SIZE + 1];
uint8_t gCircularBufferPos;
uint8_t gCircularBufferEnd = BUFFER_SIZE + 1;
gwBoolean gCircularBufferIsEmpty = TRUE;

// --------------------------------------------------------------------------

void sendOneChar(UART_MemMapPtr uartRegPtr, UsbDataType data) {

	// Until there is free space in the FIFO don't send a char.
	while (uartRegPtr->TCFIFO >= uartRegPtr->TWFIFO) {
		vTaskDelay(1);
	}
	uartRegPtr->D = data;
	while (uartRegPtr->TCFIFO > 0) {
		vTaskDelay(1);
	}
}

// --------------------------------------------------------------------------

void readOneChar(UART_MemMapPtr uartRegPtr, UsbDataPtrType dataPtr) {

	gwUINT8 bytesToRead;
	gwUINT8 byteNum;

	// Loop until the buffer has something in it.
	if (gCircularBufferIsEmpty == TRUE) {
		// Wait until there are characters in the FIFO
		while (uartRegPtr->RCFIFO == 0) {
			vTaskDelay(1);
		}
		bytesToRead = uartRegPtr->RCFIFO;
		gCircularBufferIsEmpty = FALSE;
		for (byteNum = 0; byteNum < bytesToRead; ++byteNum) {
			gCircularBufferEnd++;
			if (gCircularBufferEnd > BUFFER_SIZE) {
				gCircularBufferEnd = 0;
			}
			gCircularBuffer[gCircularBufferEnd] = uartRegPtr->D;
		}
	}

	// Get the first character out of the buffer.
	*dataPtr = gCircularBuffer[gCircularBufferPos];

	if (gCircularBufferPos == gCircularBufferEnd) {
		gCircularBufferIsEmpty = TRUE;
	}
	gCircularBufferPos++;
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
		vTaskDelay(1);
	}

	GW_WATCHDOG_RESET;
}

// --------------------------------------------------------------------------

BufferCntType serialReceiveFrame(UART_MemMapPtr uartRegPtr, BufferStoragePtrType inFramePtr, BufferCntType inMaxFrameSize) {
	BufferStorageType nextByte;
	BufferCntType bytesReceived = 0;

	// Loop reading bytes until we put together a whole frame.
	// Make sure not to copy them into the frame if we run out of room.
	while (bytesReceived < inMaxFrameSize) {

		readOneChar(uartRegPtr, &nextByte);

		switch (nextByte) {

			case END:
				if (bytesReceived) {
					GW_WATCHDOG_RESET;
					return bytesReceived;
				} else {
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

		if (bytesReceived < inMaxFrameSize)
			inFramePtr[bytesReceived++] = nextByte;
	}
	
}

