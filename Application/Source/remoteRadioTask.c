/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
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

gwBoolean	waitingForAck = FALSE;

// --------------------------------------------------------------------------

void radioReceiveTask(void *pvParameters) {
	BufferCntType rxBufferNum = 0;
	gwUINT8 ccrHolder;
	ECommandGroupIDType cmdID;
	
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
				if (gRxMsg.rxPacketPtr->rxStatus == rxSuccessStatus_c) {
					// Process the packet we just received.
					gRxRadioBuffer[rxBufferNum].bufferSize = gRxMsg.rxPacketPtr->u8DataLength;
					
					if (!waitingForAck) {
						processRxPacket(rxBufferNum, gRxMsg.rxPacketPtr->lqi);
					} else {
						cmdID = getCommandID(gRxRadioBuffer[rxBufferNum].bufferStorage);
						
						// Process netchecks always
						if (cmdID == eCommandNetMgmt) {
							processRxPacket(rxBufferNum, gRxMsg.rxPacketPtr->lqi);
						} else if (cmdID == eCommandControl) {
							// Only process ack messages
							EControlSubCmdIDType subcmd = getControlSubCommand(rxBufferNum);
							
							if (subcmd == eControlSubCmdAck) {
								processRxPacket(rxBufferNum, gRxMsg.rxPacketPtr->lqi);
							} else {
								RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
							}
						} else {
							RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
						}
					}
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

void radioTransmitTask(void *pvParameters) {
	BufferCntType txBufferNum;
	BufferCntType rxBufferNum;
	gwBoolean shouldRetry;
	portTickType retryTickCount;
	gwUINT8 ackId;
	gwUINT8 ccrHolder;
	gwBoolean isAck = FALSE;
	
#ifdef TUNER
	while(gLocalDeviceState == eLocalStateTuning){
		vTaskDelay(1);
	}
#endif

	if (gRadioTransmitQueue && gTxAckQueue) {
		for (;;) {
			waitingForAck = FALSE;
			// Wait until the management thread signals us that we have a full buffer to transmit.
			if (xQueueReceive(gRadioTransmitQueue, &txBufferNum, portMAX_DELAY) == pdPASS) {
				
				//Store current tick count
				retryTickCount = xTaskGetTickCount();
				
				gwUINT8 txedAckId = getAckId(gTxRadioBuffer[txBufferNum].bufferStorage);
				ECommandGroupIDType txCommandType = getCommandID(gTxRadioBuffer[txBufferNum].bufferStorage); 
				
				do {
					//waitingForAck = FALSE;
					shouldRetry = FALSE;
					isAck = FALSE;

					//Write tx to the air. Callback will notify us when done.
					writeRadioTx(txBufferNum);
					
					while (gRadioState == eTx) {
					}
		
					if (gTxMsg.txStatus == txFailureStatus_c) {
						shouldRetry = TRUE;
						continue;
					}

					readRadioRx();
					
					if ((gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD]) == eControlSubCmdAck) {
						isAck = TRUE;
						
					}
#ifndef GATEWAY
					if (txedAckId != 0 && txCommandType != eCommandNetMgmt && txCommandType != eCommandAssoc && !isAck) {
						shouldRetry = TRUE;
						waitingForAck = TRUE;
						//Wait up to 50ms for an ACK
						if (xQueueReceive(gTxAckQueue, &rxBufferNum, 50 * portTICK_RATE_MS) == pdPASS) {
							ackId = gRxRadioBuffer[rxBufferNum].bufferStorage[CMDPOS_ACK_NUM];
							
							if (txedAckId == ackId) {
								shouldRetry = FALSE;
								waitingForAck = FALSE;
							}
							
							RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
						}

						//If we fail to receive an ACK after enough retries to exceed 500ms then break out of the loop.
						if (shouldRetry && ((xTaskGetTickCount() - retryTickCount) > 10000)) {
							shouldRetry = FALSE;
							waitingForAck = FALSE;
						}
					}
#endif					
				} while (shouldRetry);

				RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;);
}
