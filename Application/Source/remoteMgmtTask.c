/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */
#include "remoteMgmtTask.h"
#include "radioCommon.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwTypes.h"
#include "commands.h"

xTaskHandle gRemoteManagementTask;
extern xQueueHandle gRemoteMgmtQueue;
ELocalStatusType gLocalDeviceState;
extern RadioStateEnum gRadioState;

extern RxRadioBufferStruct gRxRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType gRxCurBufferNum;
extern BufferCntType gRxUsedBuffers;
extern ERxMessageHolderType gRxMsg;
extern xTaskHandle gRadioReceiveTask;

extern TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType gTxCurBufferNum;

void preSleep();

// --------------------------------------------------------------------------

void remoteMgmtTask(void *pvParameters) {
	gwUINT8 ccrHolder;
	//BufferCntType lockedBufferNum;
	BufferCntType rxBufferNum = 0;
	BufferCntType txBufferNum;
	ChannelNumberType channel;
	gwBoolean associated;
	gwBoolean checked;
	ECommandGroupIDType cmdID;
	ECmdAssocType assocSubCmd;

	if (gRemoteMgmtQueue) {

		setStatusLed(5, 0, 0);

		/*
		 * Attempt to associate with our controller.
		 * 1. Send an associate request.
		 * 2. Wait up to 10ms for a response.
		 * 3. If we get a response then start the main proccessing
		 * 4. If no response then change channels and start at step 1.
		 */
		channel = gChannel11_c;
		associated = FALSE;
		while (!associated) {

			//It's okay if we don't start at the first channel
			channel++;
			if (channel == gTotalChannels_c) {
				channel = gChannel11_c;
			}

			GW_WATCHDOG_RESET;

			// Set the channel to the current channel we're testing.
			setRadioChannel(channel - 11);

			// Send an associate request on the current channel.
			txBufferNum = lockTxBuffer();
			createAssocReqCommand(txBufferNum);
			//writeRadioTx();
			if (transmitPacket(txBufferNum)) {
				while (gRadioState == eTx) {
					//vTaskDelay(0);
				}
			}
			
			vTaskDelay(1);

			//Make sure we are in read mode. Transmit just puts the packet on the queue so we may have finished writing the packet to the air or not. We could be in read mode
			// or tx mode. If we are in TxMode readRadioRx will wait. If the Tx hasn't started yet, we will go to read mode, the Tx will cancel the read and we'll be given a
			//non-success status. If the Tx already finished, we're in a good place. We loop in order to allow us to ignore other other packets while trying to escape the
			//read-cancellation described above.

			// Wait up to 50ms for a response. (It actually maybe more if readRadio is waiting for a write but that shouldn't happen, TODO use tickCount)
//			for(gwUINT8 i = 0; i < 10; i++) {
				//It's okay if we're already in read mode
				//readRadioRx();
				if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 750 * portTICK_RATE_MS) == pdTRUE){
					if (gRxMsg.rxPacketPtr->rxStatus == rxSuccessStatus_c) {
						// Check to see what kind of command we just got.
						cmdID = getCommandID(gRxRadioBuffer[rxBufferNum].bufferStorage);
						if (cmdID == eCommandAssoc) {
							assocSubCmd = getAssocSubCommand(rxBufferNum);
							if (assocSubCmd == eCmdAssocRESP) {
								gLocalDeviceState = eLocalStateAssocRespRcvd;
								processAssocRespCommand(rxBufferNum);
								if (gLocalDeviceState == eLocalStateAssociated) {
									associated = TRUE;
									RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
									break;
								}
							}
						}
					}
					RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
				}
				//vTaskDelay(20);
			}
//		}

		checked = FALSE;
		while (!checked) {

			// Wait up to 50ms for a response. (It actually maybe more if readRadio is waiting for a write but that shouldn't happen, TODO use tickCount)
			for(gwUINT8 i = 0; i < 10; i++) {

				//TODO Transmit is not safe.
				txBufferNum = lockTxBuffer();
				createAssocCheckCommand(txBufferNum);
				//writeRadioTx();
				if (transmitPacket(txBufferNum)) {
					while (gRadioState == eTx) {
						//vTaskDelay(0);
					}
				}

				vTaskDelay(1);
				
				//It's okay if we're already in read mode
				//readRadioRx();
				if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 750 * portTICK_RATE_MS) == pdPASS) {
					if (gRxMsg.rxPacketPtr->rxStatus == rxSuccessStatus_c) {
						// Check to see what kind of command we just got.
						cmdID = getCommandID(gRxRadioBuffer[rxBufferNum].bufferStorage);
						if (cmdID == eCommandAssoc) {
							assocSubCmd = getAssocSubCommand(rxBufferNum);
							if (assocSubCmd == eCmdAssocACK) {
								checked = TRUE;
								setStatusLed(0, 0, 1);
								RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
								break;
							}
						}
					}
					RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
				}
			}
			//vTaskDelay(20);
		}

		gLocalDeviceState = eLocalStateRun;
		readRadioRx();
		
		// Just in case we receive any extra management commands we need to free them.
		
		while (TRUE) {
			if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, portMAX_DELAY) == pdPASS) {
				RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;);
}

void sleep() {
	Cpu_SetOperationMode(DOM_STOP, NULL, NULL);
}
