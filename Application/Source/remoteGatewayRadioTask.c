/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */
#include "remoteGatewayRadioTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "commands.h"
#include "radioCommon.h"
#include "Wait.h"
#include "globals.h"

// --------------------------------------------------------------------------
// Global variables.
__attribute__ ((section(".m_data_20000000"))) byte gRestartCause;
__attribute__ ((section(".m_data_20000000"))) byte gRestartData[2];
__attribute__ ((section(".m_data_20000000"))) uint32_t gProgramCounter;

xTaskHandle gRadioReceiveTask = NULL;
xTaskHandle gRadioTransmitTask = NULL;
// The queue used to send data from the radio to the radio receive task.
xQueueHandle gRadioTransmitQueue = NULL;
xQueueHandle gRadioReceiveQueue = NULL;
xQueueHandle gRemoteMgmtQueue = NULL;
xQueueHandle gTxAckQueue = NULL;

extern ERxMessageHolderType gRxMsg;
extern ETxMessageHolderType gTxMsg;

portTickType gLastAssocCheckTickCount;

extern RadioStateEnum gRadioState;
extern ELocalStatusType gLocalDeviceState;

extern RxRadioBufferStruct	gRxRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType 		gRxCurBufferNum;
extern BufferCntType 		gRxUsedBuffers;

extern TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType 		gTxCurBufferNum;
extern BufferCntType 		gTxUsedBuffers;

extern NetworkIDType gMyNetworkID;
extern NetAddrType gMyAddr;

// --------------------------------------------------------------------------

void radioGatewayReceiveTask(void *pvParameters) {
	BufferCntType rxBufferNum = 0;
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	// The radio receive task will return a pointer to a radio data packet.
	if (gRadioReceiveQueue) {
		for (;;) {

			readRadioRx();

			if (xQueueReceive(gRadioReceiveQueue, &rxBufferNum, 5) == pdPASS ) {
				// Did we get a a packet and is the crc ok?
				if (gRxMsg.rxPacketPtr->rxStatus == rxSuccessStatus_c
					&& checkGatewayCRC(gRxRadioBuffer[rxBufferNum].bufferStorage, gRxMsg.rxPacketPtr->u8DataLength)) {
					// Process the packet we just received.
					gRxRadioBuffer[rxBufferNum].bufferSize = gRxMsg.rxPacketPtr->u8DataLength;
					processRxPacket(rxBufferNum, gRxMsg.rxPacketPtr->lqi);
				} else {
					RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
				}
			} else {
				RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
				vTaskDelay(0);
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;);
}

// --------------------------------------------------------------------------

void radioGatewayTransmitTask(void *pvParameters) {
	BufferCntType txBufferNum;
	gwUINT8 ccrHolder;
	PacketVerType packetVersion;
	smacErrors_t smacError;
	gwUINT16 crc = 0;

	if (gRadioTransmitQueue && gTxAckQueue) {
		for (;;) {

			// Receive packet to transmit
			// Wait time of 0 returns immediately: fasle if queue is empty
			// We do not want to busy wait this thread when we could be receiving
			if (xQueueReceive(gRadioTransmitQueue, &txBufferNum, 0) == pdPASS) {

				vTaskSuspend(gRadioReceiveTask);

				writeRadioTx(txBufferNum);
				while (gRadioState == eTx) {
					vTaskDelay(1);
				}

				RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
				vTaskResume(gRadioReceiveTask);

			} else {
				vTaskSuspend(gRadioTransmitTask);
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;);
}

// --------------------------------------------------------------------------

gwBoolean checkGatewayCRC(BufferStoragePtrType inBufferPtr, BufferCntType inPcktSize) {
	gwUINT16 pcktCrc = 0x0;
	gwUINT16 calculatedCrc = 0x0;
	PacketVerType packetVersion;

	packetVersion = getPacketVersion(inBufferPtr);

	// Protocol version 0 has no CRC
	if (packetVersion == PROTOCOL_VER_0) {
		return TRUE;
	}

	pcktCrc = getCRC(inBufferPtr);
	calculatedCrc = calculateCRC16(inBufferPtr, inPcktSize);

	if (calculatedCrc == pcktCrc) {
		return TRUE;
	} else {
		return FALSE;
	}
}
