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

__attribute__ ((section(".m_data_20000000"))) byte g_lastChannel;
//byte lastChannel = 0;

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
	
	// TODO - huffa If in tuning mode wait for tuning to be complete
#ifdef TUNER
	while(gLocalDeviceState == eLocalStateTuning){
		vTaskDelay(1);
	}
#endif

	if (gRemoteMgmtQueue) {
		
		// Try to read last used channel from memory
		// If it doesn't exist use default stating channel
		if ((g_lastChannel >= gChannel11_c) && (g_lastChannel < gTotalChannels_c)) {
			channel = g_lastChannel;
			haveLastChannel = TRUE;
		} else {
			channel = gChannel11_c;
		}

		setStatusLed(5, 0, 0);

		// Compute random backoff value
		uint32_t seed = 0;
		seed = (g_guid[6] << 16) | (g_guid[7] & 0xff);
		srand(seed);
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
		//channel = gChannel11_c;
		associated = FALSE;
		while (!associated) {

			//Do stupid back-off
			if (conNumAttempts >= (SLOW_CON_ATTEMPTS + CON_ATTEMPTS_BEFORE_BACKOFF)) {
				conNumAttempts = 0;
			} else {
				conNumAttempts++;
			}

			if (conNumAttempts > CON_ATTEMPTS_BEFORE_BACKOFF) {
				vTaskDelay(SLOW_CON_SLEEP_MS + conRandBackOff);
				conNumAttempts = 0;
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
				//vTaskDelay(1);

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
									g_lastChannel = channel;
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
					channel = g_lastChannel;
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

				//vTaskDelay(1);

				//It's okay if we're already in read mode
				//readRadioRx();
				//vTaskDelay(5);
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
			//vTaskDelay(5);
		}

		gLocalDeviceState = eLocalStateRun;
		readRadioRx();
		g_lastChannel = channel;
		channel = 0;

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
	for (;;)
		;
}

void sleep() {
	Cpu_SetOperationMode(DOM_STOP, NULL, NULL);
}
