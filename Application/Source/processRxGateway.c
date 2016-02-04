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

void processRxPacket(BufferCntType inRxBufferNum) {

	gwUINT8	ccrHolder;
//	ECommandGroupIDType		cmdID;
//	ENetMgmtSubCmdIDType	subCmdID;
//
//  	cmdID = getCommandID(gRxRadioBuffer[inRxBufferNum].bufferStorage);
//	if (cmdID == eCommandAssoc) {
//		subCmdID = getNetMgmtSubCommand(gRxRadioBuffer[inRxBufferNum].bufferStorage);
//		switch (subCmdID) {
//			case eCmdAssocACK:
//				serialTransmitFrame(UART0_BASE_PTR, (BufferStoragePtrType) ("AK"),  2);
//				break;
//
//			case eCmdAssocInvalid:
//				break;
//		}
//	}
//	serialTransmitFrame(UART0_BASE_PTR, (BufferStoragePtrType) ("PC"),  2);

	// Include LQI as last byte in the frame
	gRxRadioBuffer[inRxBufferNum].bufferStorage[gRxRadioBuffer[inRxBufferNum].bufferSize] = lqi;

	serialTransmitFrame(UART0_BASE_PTR, (BufferStoragePtrType) (&gRxRadioBuffer[inRxBufferNum].bufferStorage),  gRxRadioBuffer[inRxBufferNum].bufferSize + LQI_BYTES);
	RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
}
