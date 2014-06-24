/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
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

xTaskHandle					gRemoteManagementTask;
xQueueHandle				gRemoteMgmtQueue;
ELocalStatusType			gLocalDeviceState;

extern RxRadioBufferStruct	gRxRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType		gRxCurBufferNum;
extern BufferCntType		gRxUsedBuffers;

extern TxRadioBufferStruct	gTxRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType		gTxCurBufferNum;

void preSleep();

// --------------------------------------------------------------------------

void remoteMgmtTask(void *pvParameters) {
	gwUINT8 ccrHolder;
	BufferCntType rxBufferNum;
	BufferCntType txBufferNum;
	ChannelNumberType channel;
	gwBoolean associated;
	gwBoolean checked;
	gwUINT8 trim = 128;
	gwUINT8 assocAttempts;
	ECommandGroupIDType cmdID;
	ECmdAssocType assocSubCmd;

	if (gRemoteMgmtQueue) {

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

			GW_WATCHDOG_RESET;

			// Set the channel to the current channel we're testing.
			suspendRadioRx();
			MLMESetChannelRequest(channel);
			resumeRadioRx();

			// Send an associate request on the current channel.
			txBufferNum = lockTxBuffer();
			createAssocReqCommand(txBufferNum, (RemoteUniqueIDPtrType) STR(GUID));
			if (transmitPacket(txBufferNum)) {
			};

			// Wait up to 1000ms for a response.
			if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 1000 * portTICK_RATE_MS) == pdPASS) {
				if (rxBufferNum != 255) {
					// Check to see what kind of command we just got.
					cmdID = getCommandID(gRxRadioBuffer[rxBufferNum].bufferStorage);
					if (cmdID == eCommandAssoc) {
						assocSubCmd = getAssocSubCommand(rxBufferNum);
						if (assocSubCmd == eCmdAssocRESP) {
							gLocalDeviceState = eLocalStateAssocRespRcvd;
							processAssocRespCommand(rxBufferNum);
							if (gLocalDeviceState == eLocalStateAssociated) {
								associated = TRUE;
							}
						}
					}
					RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
				}
			}

			// If we're still not associated then change channels.
			if (!associated) {
				//	MLMEMC13192XtalAdjust(trim--);
				channel++;
				if (channel == gTotalChannels_c) {
					channel = gChannel11_c;
					assocAttempts++;
					if (assocAttempts % 2) {
						//sleep(5);
					}
				}
				// Delay 100ms before changing channels.
				//vTaskDelay(250 * portTICK_RATE_MS);
			}
		}
		gLocalDeviceState = eLocalStateRun;

		checked = FALSE;
		while (!checked) {

			GW_WATCHDOG_RESET;

			if (!checked) {
				BufferCntType txBufferNum = lockTxBuffer();
				createAssocCheckCommand(txBufferNum, (RemoteUniqueIDPtrType) STR(GUID));
				if (transmitPacket(txBufferNum)) {
				}
				// Wait up to 1000ms for a response.
				if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 1000 * portTICK_RATE_MS) == pdPASS ) {
					if (rxBufferNum != 255) {
						// Check to see what kind of command we just got.
						cmdID = getCommandID(gRxRadioBuffer[rxBufferNum].bufferStorage);
						if (cmdID == eCommandAssoc) {
							assocSubCmd = getAssocSubCommand(rxBufferNum);
							if (assocSubCmd == eCmdAssocACK) {
								checked = TRUE;
							}
						}
						RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
					}
				}
				vTaskDelay(50);
			}

		}
		vTaskSuspend(gRemoteManagementTask);
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}

void sleep() {
	Cpu_SetOperationMode(DOM_STOP, NULL, NULL);
}
