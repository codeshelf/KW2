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
#include "globals.h"
#include "stdlib.h"

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

extern xTaskHandle gRemoteManagementTask;

void preSleep();

__attribute__ ((section(".m_data_20000000"))) byte gLastChannel;

// --------------------------------------------------------------------------

void remoteMgmtTask(void *pvParameters) {
	gwUINT8 conNumAttempts = 0;
	gwUINT32 conRandBackOff = 0;
	gwUINT8 ccrHolder;
	//BufferCntType lockedBufferNum;
	BufferCntType rxBufferNum = 0;
	BufferCntType txBufferNum;
	ChannelNumberType channel;
	gwBoolean associated;
	gwBoolean checked;
	ECommandGroupIDType cmdID;
	ECmdAssocType assocSubCmd;
	vTaskDelay(0);
	
	gwBoolean haveLastChannel = FALSE;
	gwBoolean triedLastChannel = FALSE;
	int nextChannelToTry = gChannel11_c;
	
#ifdef TUNER
	while(gLocalDeviceState == eLocalStateTuning){
		vTaskDelay(1);
	}
#endif

	if (gRemoteMgmtQueue) {
		
		// Try to read last used channel from memory. Otherwise use default starting
		if ((gLastChannel >= gChannel11_c) && (gLastChannel < gTotalChannels_c)) {
			channel = gLastChannel;
			haveLastChannel = TRUE;
		} else {
			channel = gChannel11_c;
		}

		setStatusLed(5, 0, 0);

		// Compute random backoff value
		srand((uint32_t) (gGuid[6] << 16) | (gGuid[7] & 0xff));
		conRandBackOff = rand() % RAND_BACK_OFF_LIMIT;
		
		// Random backoff
		vTaskDelay(conRandBackOff);

		/*
		 * Attempt to associate with our controller.
		 * 1. Send an associate request.
		 * 2. Wait up to 10ms for a response.
		 * 3. If we get a response then start the main proccessing
		 * 4. If no response then change channels and start at step 1.
		 */

		associated = FALSE;
		while (!associated) {
	
			if (conNumAttempts >= (SLOW_CON_ATTEMPTS + CON_ATTEMPTS_BEFORE_BACKOFF)) {
				conNumAttempts = 0;
			} else {
				conNumAttempts++;
			}

			// Back-off
			if (conNumAttempts > CON_ATTEMPTS_BEFORE_BACKOFF) {
 				vTaskDelay(SLOW_CON_SLEEP_MS + conRandBackOff);
			}

			GW_WATCHDOG_RESET;

			// Set the channel to the current channel we're testing.
			setRadioChannel(channel - 11);

			for (gwUINT8 i = 0; i < 1; i++) {
				// Send an associate request on the current channel.
				txBufferNum = lockTxBuffer();
				createAssocReqCommand(txBufferNum);
				//writeRadioTx();
				if (transmitPacket(txBufferNum)) {
					while (gRadioState == eTx) {
						vTaskDelay(0);
					}
				}

				//Make sure we are in read mode. Transmit just puts the packet on the queue so we may have finished writing the packet to the air or not. We could be in read mode
				// or tx mode. If we are in TxMode readRadioRx will wait. If the Tx hasn't started yet, we will go to read mode, the Tx will cancel the read and we'll be given a
				//non-success status. If the Tx already finished, we're in a good place. We loop in order to allow us to ignore other other packets while trying to escape the
				//read-cancellation described above.

				// Wait up to 50ms for a response. (It actually maybe more if readRadio is waiting for a write but that shouldn't happen, TODO use tickCount)
				//for(gwUINT8 i = 0; i < 3; i++) {
				//It's okay if we're already in read mode
				//readRadioRx();

				if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 50 * portTICK_RATE_MS) == pdTRUE ) {
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
									gLastChannel = channel;
									RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
									break;
								}
							}
						}
					}
					RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
				}
			}
			//}

			if (!associated) {
				if (triedLastChannel || !haveLastChannel){
					if (nextChannelToTry == gTotalChannels_c) {
						channel = gChannel11_c;
						nextChannelToTry = channel++;
					} else {
						channel = nextChannelToTry;
						nextChannelToTry++;
					}
				} else {
					channel = gLastChannel;
					triedLastChannel = TRUE;
				}
			}
		}

		checked = FALSE;
		while (!checked) {
			setStatusLed(0, 1, 0);

			// Wait up to 50ms for a response. (It actually maybe more if readRadio is waiting for a write but that shouldn't happen, TODO use tickCount)
			for (gwUINT8 i = 0; i < 3; i++) {

				//TODO Transmit is not safe.
				txBufferNum = lockTxBuffer();
				createAssocCheckCommand(txBufferNum);
				//writeRadioTx();
				if (transmitPacket(txBufferNum)) {
					while (gRadioState == eTx) {
						vTaskDelay(0);
					}
				}

				//It's okay if we're already in read mode
				//readRadioRx();

				if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 200 * portTICK_RATE_MS) == pdPASS ) {
					if (gRxMsg.rxPacketPtr->rxStatus == rxSuccessStatus_c) {
						// Check to see what kind of command we just got.
						cmdID = getCommandID(gRxRadioBuffer[rxBufferNum].bufferStorage);
						if (cmdID == eCommandAssoc) {
							assocSubCmd = getAssocSubCommand(rxBufferNum);
							if (assocSubCmd == eCmdAssocACK) {
								checked = TRUE;
								setStatusLed(0, 0, 1);
								RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
								GW_WATCHDOG_RESET
								;
								break;
							}
						}
					}
					RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
				}
			}
		}

		gLocalDeviceState = eLocalStateRun;
		readRadioRx();
		gLastChannel = channel;
		channel = 0;
		
		// Clear last reset cause and related data
		gRestartCause = 0;
		memset(gRestartData, 0, ASSOC_RST_DATA_LEN);
		gProgramCounter = 0;

		// Just in case we receive any extra management commands we need to free them.
		/*
		 while (TRUE) {	
		 if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 10 * portTICK_RATE_MS) == pdPASS) {
		 RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
		 }
		 }*/
	}

	// This thread is no longer needed
	vTaskSuspend(gRemoteManagementTask);
	/* Will only get here if the queue could not be created. */
	for (;;);
}

void sleep() {
	Cpu_SetOperationMode(DOM_STOP, NULL, NULL);
}
