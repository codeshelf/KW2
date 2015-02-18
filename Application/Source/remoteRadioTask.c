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
xQueueHandle gRemoteMgmtQueue = NULL;
xQueueHandle gTxAckQueue = NULL;

ERxMessageHolderType gRxMsg;
ETxMessageHolderType gTxMsg;
portTickType gLastAssocCheckTickCount;
gwBoolean gIsReceiving;
gwBoolean gIsTransmitting;

extern RxRadioBufferStruct gRxRadioBuffer[RX_BUFFER_COUNT];
extern TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];

extern NetworkIDType gMyNetworkID;
extern NetAddrType gMyAddr;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	//BufferCntType lockedBufferNum;
	BufferCntType rxBufferNum;
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	// The radio receive task will return a pointer to a radio data packet.
	if (gRadioReceiveQueue) {
		for (;;) {

			GW_WATCHDOG_RESET;

			gRxMsg.rxPacketPtr = (rxPacket_t *) &(gRxRadioBuffer[0].rxPacket);
			gRxMsg.rxPacketPtr->u8MaxDataLength = RX_BUFFER_SIZE;
			gRxMsg.rxPacketPtr->rxStatus = rxInitStatus;
			gRxMsg.bufferNum = 0;//lockedBufferNum;

			gIsReceiving = TRUE;
			
			//Perform the actual read and store it in the pointer with no timeout
			smacError = MLMERXEnableRequest(gRxMsg.rxPacketPtr, 0);

			if (xQueueReceive(gRadioReceiveQueue, &rxBufferNum, portMAX_DELAY) == pdPASS ) {
				if ((rxBufferNum != 255) && (gRxMsg.rxPacketPtr->rxStatus == rxSuccessStatus_c)) {
					// Process the packet we just received.
					processRxPacket(rxBufferNum);
					// N.B.: processRXPacket above releases the RX buffer if necessary.
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
	portTickType retryTickCount;
	NetAddrType cmdDstAddr;
	NetworkIDType networkID;
	gwUINT8 ackId;
	gwUINT8 ccrHolder;

	if (gRadioTransmitQueue && gTxAckQueue) {
		for (;;) {

			// Wait until the management thread signals us that we have a full buffer to transmit.
			if (xQueueReceive(gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS) {
				vTaskSuspend(gRadioReceiveTask);
				retryTickCount = xTaskGetTickCount();
				
				gwUINT8 txedAckId = getAckId(gTxRadioBuffer[txBufferNum].bufferStorage);
				ECommandGroupIDType txCommandType = getCommandID(gTxRadioBuffer[txBufferNum].bufferStorage);
				
				do {
					shouldRetry = FALSE;

					// Setup for TX.
					gTxMsg.txPacketPtr = (txPacket_t *) &(gTxRadioBuffer[txBufferNum].txPacket);
					gTxMsg.txPacketPtr->u8DataLength = gTxRadioBuffer[txBufferNum].bufferSize;
					gTxMsg.bufferNum = txBufferNum;

					suspendRadioRx();
					gIsTransmitting = TRUE;
					smacError = MCPSDataRequest(gTxMsg.txPacketPtr);

					if (smacError != gErrorNoError_c) {
						//TODO Why reset the MCU here?
						GW_RESET_MCU()
					}
					
					while (gIsTransmitting) {
						vTaskDelay(1);
					}
					
					resumeRadioRx();
					vTaskResume(gRadioReceiveTask);			

					if (gTxMsg.txStatus == txFailureStatus_c) {
						shouldRetry = TRUE;
						continue;
					}
					
					if (txedAckId != 0 && txCommandType != eCommandNetMgmt && txCommandType != eCommandAssoc) {
						shouldRetry = TRUE;
						
						//Wait up to 20ms for an ACK
						if (xQueueReceive(gTxAckQueue, &ackId, 20 * portTICK_RATE_MS) == pdPASS) {
							if (txedAckId == ackId) {
								shouldRetry = FALSE;
							}
						}

						//If we fail to receive an ACK after enough retries to exceed 500ms then RESET
						if (((shouldRetry) && ((xTaskGetTickCount() - retryTickCount) > 500)) || (smacError != gErrorNoError_c)) {
							GW_RESET_MCU();
						}
					}
					
				} while (shouldRetry);
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}
