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

extern ERxMessageHolderType gRxMsg;
extern ETxMessageHolderType gTxMsg;

portTickType gLastAssocCheckTickCount;

extern RadioStateEnum gRadioState;

extern RxRadioBufferStruct gRxRadioBuffer[RX_BUFFER_COUNT];
extern TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];

extern NetworkIDType gMyNetworkID;
extern NetAddrType gMyAddr;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType rxBufferNum;

	//TODO Remove buffer code. It seems we only need to do this once
	/*gRxMsg.rxPacketPtr = (rxPacket_t *) &(gRxRadioBuffer[0].rxPacket);
	gRxMsg.rxPacketPtr->u8MaxDataLength = RX_BUFFER_SIZE;
	gRxMsg.rxPacketPtr->rxStatus = rxInitStatus;
	gRxMsg.bufferNum = 0;
	*/
	// The radio receive task will return a pointer to a radio data packet.
	if (gRadioReceiveQueue) {
		for (;;) {

			//GW_WATCHDOG_RESET;

			//A callback will add the packet to the queue below
			readRadioRx();

			//Wait for packet in the queue
			if (xQueueReceive(gRadioReceiveQueue, &rxBufferNum, portMAX_DELAY) == pdPASS ) {
				while (gRxMsg.rxPacketPtr->rxStatus == rxProcessingReceptionStatus_c) {
					vTaskDelay(0);
				}
				if (gRxMsg.rxPacketPtr->rxStatus == rxSuccessStatus_c) {
					// Process the packet we just received.
					gRxRadioBuffer[rxBufferNum].bufferSize = gRxMsg.rxPacketPtr->u8DataLength;
					processRxPacket(rxBufferNum);
				} else {
					readRadioRx();
				}
			} else {
				readRadioRx();
			}

		}
	}
	/* Will only get here if the queue could not be created. */
	for (;;);
}

// --------------------------------------------------------------------------

void radioTransmitTask(void *pvParameters) {
	BufferCntType txBufferNum;
	gwBoolean shouldRetry;
	portTickType retryTickCount;
	gwUINT8 ackId;

	if (gRadioTransmitQueue && gTxAckQueue) {
		for (;;) {

			// Wait until the management thread signals us that we have a full buffer to transmit.
			if (xQueueReceive(gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS) {
				
				//Store current tick count
				retryTickCount = xTaskGetTickCount();
				
				gwUINT8 txedAckId = getAckId(gTxRadioBuffer[txBufferNum].bufferStorage);
				ECommandGroupIDType txCommandType = getCommandID(gTxRadioBuffer[txBufferNum].bufferStorage);
				
				do {
					shouldRetry = FALSE;

					//Write tx to the air. Callback will notify us when done.
					writeRadioTx();
					
					while (gRadioState == eTx) {
						vTaskDelay(1);
					}
		
					if (gTxMsg.txStatus == txFailureStatus_c) {
						shouldRetry = TRUE;
						continue;
					}
					
					readRadioRx();

#ifndef GATEWAY
					if (txedAckId != 0 && txCommandType != eCommandNetMgmt && txCommandType != eCommandAssoc) {
						shouldRetry = TRUE;
						
						//Wait up to 50ms for an ACK
						if (xQueueReceive(gTxAckQueue, &ackId, 50 * portTICK_RATE_MS) == pdPASS) {
							if (txedAckId == ackId) {
								shouldRetry = FALSE;
							}
						}

						//If we fail to receive an ACK after enough retries to exceed 500ms then break out of the loop.
						if (shouldRetry && ((xTaskGetTickCount() - retryTickCount) > 500)) {
							shouldRetry = FALSE;
						}
					}
#endif					
				} while (shouldRetry);
				gTxRadioBuffer[txBufferNum].bufferStatus = eBufferStateFree;
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;);
}
