/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */
#include "remoteRadioTask.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "commands.h"
#include "radioCommon.h"
#include "Wait.h"

// --------------------------------------------------------------------------
// Global variables.

xTaskHandle gRadioReceiveTask = NULL;
xTaskHandle gRadioTransmitTask = NULL;
// The queue used to send data from the radio to the radio receive task.
xQueueHandle gRadioTransmitQueue = NULL;
xQueueHandle gRadioReceiveQueue = NULL;

ERxMessageHolderType gRxMsg;
ETxMessageHolderType gTxMsg;
portTickType gLastAssocCheckTickCount;
gwBoolean gIsTransmitting;

extern RxRadioBufferStruct gRxRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType gRxCurBufferNum;
extern BufferCntType gRxUsedBuffers;

extern TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType gTxCurBufferNum;
extern BufferCntType gTxUsedBuffers;

extern NetworkIDType gMyNetworkID;
extern NetAddrType gMyAddr;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType rxBufferNum;
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	// The radio receive task will return a pointer to a radio data packet.
	if (gRadioReceiveQueue) {
		for (;;) {

			GW_WATCHDOG_RESET

			// Setup for the next RX cycle.
			advanceRxBuffer();
			gRxCurBufferNum = lockRxBuffer();

			gRxMsg.rxPacketPtr = (rxPacket_t *) &(gRxRadioBuffer[gRxCurBufferNum].rxPacket);
			gRxMsg.rxPacketPtr->u8MaxDataLength = RX_BUFFER_SIZE;
			gRxMsg.rxPacketPtr->rxStatus = rxInitStatus;
			gRxMsg.bufferNum = gRxCurBufferNum;

			smacError = MLMERXEnableRequest(gRxMsg.rxPacketPtr, 0);

			if (xQueueReceive(gRadioReceiveQueue, &rxBufferNum, portMAX_DELAY) == pdPASS ) {
				if ((rxBufferNum != 255) && (gRxMsg.rxPacketPtr->rxStatus == rxSuccessStatus_c)) {
					// Process the packet we just received.
					gRxRadioBuffer[gRxMsg.bufferNum].bufferSize = gRxMsg.rxPacketPtr->u8DataLength;
					processRxPacket(gRxMsg.bufferNum);
					// processRXPacket releases the RX buffer if necessary.
				} else {
					// Probably failed or aborted, release it.
					RELEASE_RX_BUFFER(gRxMsg.bufferNum, ccrHolder);
				}
			}
		}
	}
	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}

// --------------------------------------------------------------------------

void radioTransmitTask(void *pvParameters) {
	smacErrors_t smacError;
	BufferCntType txBufferNum;
	gwBoolean shouldRetry;
	NetAddrType cmdDstAddr;
	NetworkIDType networkID;
	gwUINT8 ccrHolder;

	if (gRadioTransmitQueue) {
		for (;;) {

			// Wait until the management thread signals us that we have a full buffer to transmit.
			if (xQueueReceive(gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS ) {

				// Setup for TX.
				gTxMsg.txPacketPtr = (txPacket_t *) &(gTxRadioBuffer[txBufferNum].txPacket);
				gTxMsg.txPacketPtr->u8DataLength = gTxRadioBuffer[txBufferNum].bufferSize;
				gTxMsg.bufferNum = txBufferNum;
				
				suspendRadioRx();

				shouldRetry = FALSE;
				do {
					gIsTransmitting = TRUE;
					smacError = MCPSDataRequest(gTxMsg.txPacketPtr);
					
					// If the radio can't TX then we're in big trouble.  Just reset.
					if (smacError != gErrorNoError_c) {
						GW_RESET_MCU()
					}
					
					while (gIsTransmitting) {
						vTaskDelay(2);
					}

					if (gTxMsg.txStatus == txFailureStatus_c) {
						shouldRetry = TRUE;
//					} else if ((getAckId(gTxRadioBuffer[txBufferNum].bufferStorage) != 0)
//					        && (getCommandID(gTxRadioBuffer[txBufferNum].bufferStorage) != eCommandNetMgmt)) {
//						// If the TX packet had an ACK id then retry TX until we get the ACK or reset.
//
//						// We'll simply use the RX packet from the suspended task.
//						smacError = MLMERXEnableRequest(gRxMsg.rxPacketPtr, 0);
//
//						if (smacError == gErrorNoError_c) {
//							networkID = getNetworkID(gRxMsg.bufferNum);
//							cmdDstAddr = getCommandDstAddr(gRxMsg.bufferNum);
//							if ((getAckId(gTxRadioBuffer[txBufferNum].bufferStorage)
//							        == getAckId(gRxRadioBuffer[gRxCurBufferNum].bufferStorage)) && (networkID == gMyNetworkID)
//							        && (cmdDstAddr == gMyAddr)) {
//								shouldRetry = TRUE;
//							}
//						}
					}
				} while (shouldRetry);

				RELEASE_TX_BUFFER(gTxMsg.bufferNum, ccrHolder);
			} else {

			}
		}
	}
	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
