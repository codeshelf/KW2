/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#ifndef SERIAL_H
#define SERIAL_H

#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"

// --------------------------------------------------------------------------
// Defines.

#define GATEWAY_MGMT_QUEUE_SIZE		10
#define BUFFER_SIZE					32

#define END							0300    /* indicates end of frame */
#define ESC							0333    /* indicates byte stuffing */
#define ESC_END						0334    /* ESC ESC_END means END data byte */
#define ESC_ESC						0335    /* ESC ESC_ESC means ESC data byte */

// --------------------------------------------------------------------------
// Functions prototypes.

#include "IO_Map.h"

typedef gwUINT8	UsbDataType;
typedef gwUINT8	*UsbDataPtrType;

void UART_ReadOneChar(UART_MemMapPtr uartRegPtr, UsbDataPtrType dataPtr);
void sendOneChar(UART_MemMapPtr uartRegPtr, UsbDataType data);
void serialReceiveTask(void *pvParameters);
void serialTransmitFrame(UART_MemMapPtr uartRegPtr, BufferStoragePtrType inFramePtr, BufferCntType inFrameSize);
BufferCntType serialReceiveFrame(UART_MemMapPtr uartRegPtr, BufferStoragePtrType inFramePtr, BufferCntType inMaxFrameSize);

#endif // SERIAL_H
