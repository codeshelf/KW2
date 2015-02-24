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
				if (gRxMsg.rxPacketPtr->rxStatus == rxSuccessStatus_c) {
					// Process the packet we just received.
					processRxPacket(rxBufferNum);
				}
			}
		}
	}
	/* Will only get here if the queue could not be created. */
	for (;;);
}

// --------------------------------------------------------------------------

void radioTransmitTask(void *pvParameters) {
	//smacErrors_t smacError;
	BufferCntType txBufferNum;
	gwBoolean shouldRetry;
	portTickType retryTickCount;
	//NetAddrType cmdDstAddr;
	//NetworkIDType networkID;
	gwUINT8 ackId;
	//gwUINT8 ccrHolder;

	if (gRadioTransmitQueue && gTxAckQueue) {
		for (;;) {

			// Wait until the management thread signals us that we have a full buffer to transmit.
			if (xQueueReceive(gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS) {
				
				//Suspend receive task
				//vTaskSuspend(gRadioReceiveTask);
				
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
					
					//Remove receive task now that we are done
					//TODO Do we really need to suspend it?
					//vTaskResume(gRadioReceiveTask);			

					if (gTxMsg.txStatus == txFailureStatus_c) {
						shouldRetry = TRUE;
						continue;
					}
					
					if (txedAckId != 0 && txCommandType != eCommandNetMgmt && txCommandType != eCommandAssoc) {
						shouldRetry = TRUE;
						
						//Wait up to 50ms for an ACK
						if (xQueueReceive(gTxAckQueue, &ackId, 50 * portTICK_RATE_MS) == pdPASS) {
							if (txedAckId == ackId) {
								shouldRetry = FALSE;
							}
						}

						//If we fail to receive an ACK after enough retries to exceed 500ms then RESET
						if (shouldRetry && ((xTaskGetTickCount() - retryTickCount) > 500)) {
							//GW_RESET_MCU();
							break;
						}
					}
					
				} while (shouldRetry);
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;);
}
