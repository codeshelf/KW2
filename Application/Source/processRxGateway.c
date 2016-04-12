/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "processRx.h"
#include "serial.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// --------------------------------------------------------------------------
// Global variables.

UnixTimeType gUnixTime;

extern RxRadioBufferStruct	gRxRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType		gRxCurBufferNum;
extern BufferCntType		gRxUsedBuffers;

extern xQueueHandle			gRemoteMgmtQueue;
extern xQueueHandle			gTxAckQueue;
extern NetworkIDType		gMyNetworkID;
extern NetAddrType			gMyAddr;

extern portTickType			gLastPacketReceivedTick;
extern ELocalStatusType		gLocalDeviceState;

// --------------------------------------------------------------------------

void processRxPacket(BufferCntType inRxBufferNum, uint8_t lqi) {
	gwUINT8	ccrHolder;
	bool crc = TRUE;
	PacketVerType packetVersion;

	packetVersion = getPacketVersion(gRxRadioBuffer[inRxBufferNum].bufferStorage);

	if (packetVersion == PROTOCOL_VER_1) {
		crc = checkCRC(gRxRadioBuffer[inRxBufferNum].bufferStorage, gRxRadioBuffer[inRxBufferNum].bufferSize);
	} else {
		crc = TRUE;
	}

	// Transmit if the CRC checks out
	if (crc) {
		// Include LQI as last byte in the frame
		gRxRadioBuffer[inRxBufferNum].bufferStorage[gRxRadioBuffer[inRxBufferNum].bufferSize] = lqi;
		serialTransmitFrame(UART0_BASE_PTR, (BufferStoragePtrType) (&gRxRadioBuffer[inRxBufferNum].bufferStorage),  gRxRadioBuffer[inRxBufferNum].bufferSize + LQI_BYTES);
	}

	RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
}
