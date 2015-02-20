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
xTaskHandle gRemoteManagementTask;
xQueueHandle gRemoteMgmtQueue;
ELocalStatusType gLocalDeviceState;

extern RxRadioBufferStruct gRxRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType gRxCurBufferNum;
extern BufferCntType gRxUsedBuffers;
extern ERxMessageHolderType gRxMsg;

extern TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType gTxCurBufferNum;

void preSleep();

// --------------------------------------------------------------------------

void remoteMgmtTask(void *pvParameters) {
	gwUINT8 ccrHolder;
	//BufferCntType lockedBufferNum;
	BufferCntType rxBufferNum;
	BufferCntType txBufferNum;
	ChannelNumberType channel;
	gwBoolean associated;
	gwBoolean checked;
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
			
			//It's okay if we don't start at the first channel
			channel++;
			if (channel == gTotalChannels_c) {
				channel = gChannel11_c;
			}
			
			GW_WATCHDOG_RESET;

			// Set the channel to the current channel we're testing.
			setRadioChannel(channel);

			// Send an associate request on the current channel.
			txBufferNum = 0;//lockTxBuffer();
			createAssocReqCommand(txBufferNum);
			if (transmitPacket(txBufferNum)) {			
			};

			// Wait up to 50ms for a response.
			if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 50 * portTICK_RATE_MS) == pdTRUE){
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
			}
		}

		checked = FALSE;
		while (!checked) {

			GW_WATCHDOG_RESET;

			createAssocCheckCommand(txBufferNum);
			if (transmitPacket(txBufferNum)) {
			}

			// Wait up to 50ms for a response.
			if (xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, 50 * portTICK_RATE_MS) == pdPASS) {
				// Check to see what kind of command we just got.
				cmdID = getCommandID(gRxRadioBuffer[rxBufferNum].bufferStorage);
				if (cmdID == eCommandAssoc) {
					assocSubCmd = getAssocSubCommand(rxBufferNum);
					if (assocSubCmd == eCmdAssocACK) {
						checked = TRUE;
					}
				}
			}
			vTaskDelay(50);

		}

		gLocalDeviceState = eLocalStateRun;

		// Just in case we receive any extra management commands we need to free them.
		while (TRUE) {
			if ((xQueueReceive(gRemoteMgmtQueue, &rxBufferNum, portMAX_DELAY) == pdPASS ) && (rxBufferNum != 255)) {
				//RELEASE_RX_BUFFER(rxBufferNum, ccrHolder);
			}
		}
	}

	/* Will only get here if the queue could not be created. */
	for (;;)
		;
}

void sleep() {
	Cpu_SetOperationMode(DOM_STOP, NULL, NULL);
}
