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
	gwUINT8 ccrHolder;
#ifdef TUNER
	while(gLocalDeviceState == eLocalStateTuning){
		vTaskDelay(1);
	}
#endif

	//TODO Remove buffer code. It seems we only need to do this once

	/*
	gRxMsg.rxPacketPtr = (rxPacket_t *) &(gRxRadioBuffer[rxBufferNum].rxPacket);
	gRxMsg.rxPacketPtr->u8MaxDataLength = RX_BUFFER_SIZE;
	gRxMsg.rxPacketPtr->rxStatus = rxInitStatus;
	gRxMsg.bufferNum = rxBufferNum;
	*/
	// The radio receive task will return a pointer to a radio data packet.
	if (gRadioReceiveQueue) {
		for (;;) {

			//A callback will add the packet to the queue below
			readRadioRx();

			//Wait for packet in the queue
			if (xQueueReceive(gRadioReceiveQueue, &rxBufferNum, portMAX_DELAY) == pdPASS ) {
				while (gRxMsg.rxPacketPtr->rxStatus == rxProcessingReceptionStatus_c) {
					vTaskDelay(0);
				}

				//PacketVerType packetVersion;
				//packetVersion = getPacketVersion(gRxRadioBuffer[rxBufferNum].bufferStorage);

				// Did we get a a packet and is the crc ok?
				if (gRxMsg.rxPacketPtr->rxStatus == rxSuccessStatus_c &&
					checkCRC(gRxRadioBuffer[rxBufferNum].bufferStorage, gRxMsg.rxPacketPtr->u8DataLength)) {

					// Process the packet we just received.
					gRxRadioBuffer[rxBufferNum].bufferSize = gRxMsg.rxPacketPtr->u8DataLength;
					processRxPacket(rxBufferNum, gRxMsg.rxPacketPtr->lqi);
				} else {
					RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
				}
			} else {
				RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
			}
			vTaskDelay(0);
		}
	}
	/* Will only get here if the queue could not be created. */
	for (;;);
}

// --------------------------------------------------------------------------

void radioGatewayTransmitTask(void *pvParameters) {
	BufferCntType txBufferNum;
	gwBoolean shouldRetry;
	portTickType retryTickCount;
	gwUINT8 ccrHolder;
	PacketVerType packetVersion;
	gwUINT16 crc = 0;

	if (gRadioTransmitQueue && gTxAckQueue) {
		for (;;) {

			// Wait until the management thread signals us that we have a full buffer to transmit.
			if (xQueueReceive(gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS) {

				packetVersion = getPacketVersion(gTxRadioBuffer[txBufferNum].bufferStorage);

				if (packetVersion == PROTOCOL_VER_1) {
					// Calculate the crc for the messages protocol v1 messages
					crc = calculateCRC16(gTxRadioBuffer[txBufferNum].bufferStorage, gTxRadioBuffer[txBufferNum].bufferSize);
					setCRC(gTxRadioBuffer[txBufferNum].bufferStorage, crc);
				}

				// Store current tick count
				retryTickCount = xTaskGetTickCount();

				do {
					shouldRetry = FALSE;

					//Write tx to the air. Callback will notify us when done.
					writeRadioTx(txBufferNum);

					while (gRadioState == eTx) {
					}

					if (gTxMsg.txStatus == txFailureStatus_c) {
						shouldRetry = TRUE;
						continue;
					}

					readRadioRx();

					// If we fail to send for more than 500ms break out of loop
					if (shouldRetry && ((xTaskGetTickCount() - retryTickCount) > 5000)) {
						shouldRetry = FALSE;
					}

				} while (shouldRetry);

				RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;);
}
