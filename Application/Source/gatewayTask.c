/*
FlyWeight
 Copyright 2005, 2006 Jeffrey B. Williams
All rights reserved

$Id$
$Name$	
*/

#include "gatewayTask.h"
#include "remoteGatewayRadioTask.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwSystemMacros.h"
#include "gwTypes.h"
#include "commands.h"
#include "serial.h"
#include "string.h"
#include "Gateway.h"
#include "radioCommon.h"
#include "gatewayCommonTypes.h"

xQueueHandle			gGatewayMgmtQueue;
ControllerStateType		gControllerState;

xTaskHandle gSerialReceiveTask = NULL;
extern xQueueHandle	gRadioTransmitQueue;

extern RxRadioBufferStruct gRxRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType gRxCurBufferNum;
extern BufferCntType gRxUsedBuffers;

extern xTaskHandle gRadioTransmitTask;
extern TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType 		gTxCurBufferNum;
extern BufferCntType 		gTxUsedBuffers;

// --------------------------------------------------------------------------

void serialReceiveTask(void *pvParameters ) {

	ECommandGroupIDType		cmdID;
	ENetMgmtSubCmdIDType	subCmdID;
	BufferCntType			txBufferNum = 0;
	PacketVerType 			packetVersion;
	gwUINT16				crc = 0;
	gwUINT8 				ccrHolder;
	
	// Setup the USB interface.
	Gateway_Init();
	
	/* PORTD_PCR6: ISF=0,MUX=3 */
	PORTD_PCR6 = (uint32_t)((PORTD_PCR6 & (uint32_t)~(uint32_t)(
				PORT_PCR_ISF_MASK |
				PORT_PCR_MUX(0x04)
			   )) | (uint32_t)(
				PORT_PCR_MUX(0x03)
			   ));
	/* PORTD_PCR7: ISF=0,MUX=3 */
	PORTD_PCR7 = (uint32_t)((PORTD_PCR7 & (uint32_t)~(uint32_t)(
				PORT_PCR_ISF_MASK |
				PORT_PCR_MUX(0x04)
			   )) | (uint32_t)(
				PORT_PCR_MUX(0x03)
			   ));
	/* PORTD_PCR5: ISF=0,MUX=3 */
	PORTD_PCR5 = (uint32_t)((PORTD_PCR5 & (uint32_t)~(uint32_t)(
				PORT_PCR_ISF_MASK |
				PORT_PCR_MUX(0x04)
			   )) | (uint32_t)(
				PORT_PCR_MUX(0x03)
			   ));
	/* PORTD_PCR4: ISF=0,MUX=3 */
	PORTD_PCR4 = (uint32_t)((PORTD_PCR4 & (uint32_t)~(uint32_t)(
				PORT_PCR_ISF_MASK |
				PORT_PCR_MUX(0x04)
			   )) | (uint32_t)(
				PORT_PCR_MUX(0x03)
			   ));

	// Send a net-setup command to the controller.
	// It will respond with the channel that we should be using.
	createOutboundNetSetupV1();
	createOutboundNetSetupV0();
	
	for ( ;; ) {
		
		txBufferNum = lockTxBuffer();
		gTxRadioBuffer[txBufferNum].bufferSize = serialReceiveFrame(UART0_BASE_PTR, gTxRadioBuffer[txBufferNum].bufferStorage, TX_BUFFER_SIZE);

		if (gTxRadioBuffer[txBufferNum].bufferSize > 0) {
			gTxRadioBuffer[txBufferNum].bufferStatus = eBufferStateInUse;

			packetVersion = getPacketVersion(gTxRadioBuffer[txBufferNum].bufferStorage);

			// We manipulate packets based on version
			if (packetVersion == PROTOCOL_VER_1) {
				cmdID = ((gTxRadioBuffer[txBufferNum].bufferStorage[V1_CMDPOS_CMD_ID]) & CMDMASK_CMDID) >> 4;

				if (cmdID == eCommandNetMgmt) {
					subCmdID = gTxRadioBuffer[txBufferNum].bufferStorage[V1_CMDPOS_NETM_SUBCMD];
					switch (subCmdID) {
						case eNetMgmtSubCmdNetCheck:
							processNetCheckOutboundCommandV1(txBufferNum);
							break;

						case eNetMgmtSubCmdNetSetup:
							processNetSetupCommandV1(txBufferNum);
							// We don't ever want to broadcast net-setup, so continue.
							RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
							continue;
							break;

						case eNetMgmtSubCmdNetIntfTest:
							processNetIntfTestCommandV1(txBufferNum);
							// We don't ever want to broadcast intf-test, so continue.
							RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
							continue;
							break;

						case eNetMgmtSubCmdInvalid:
							RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
							continue;
							break;
					}
				}

				// Calculate the crc for the messages protocol v1 messages
				crc = calculateCRC16(gTxRadioBuffer[txBufferNum].bufferStorage, gTxRadioBuffer[txBufferNum].bufferSize);
				setCRC(gTxRadioBuffer[txBufferNum].bufferStorage, crc);

			} else {
				cmdID = ((gTxRadioBuffer[txBufferNum].bufferStorage[V0_CMDPOS_CMD_ID]) & CMDMASK_CMDID) >> 4;

				if (cmdID == eCommandNetMgmt) {
					subCmdID = gTxRadioBuffer[txBufferNum].bufferStorage[V0_CMDPOS_NETM_SUBCMD];
					switch (subCmdID) {
						case eNetMgmtSubCmdNetCheck:
							processNetCheckOutboundCommandV0(txBufferNum);
							break;

						case eNetMgmtSubCmdNetSetup:
							processNetSetupCommandV0(txBufferNum);
							// We don't ever want to broadcast net-setup, so continue.
							RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
							continue;
							break;

						case eNetMgmtSubCmdNetIntfTest:
							processNetIntfTestCommandV0(txBufferNum);
							// We don't ever want to broadcast intf-test, so continue.
							RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
							continue;
							break;

						case eNetMgmtSubCmdInvalid:
							RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
							continue;
							break;
					}
				}
			}

			// Now send the buffer to the transmit queue.
			if (xQueueSend(gRadioTransmitQueue, &txBufferNum, (portTickType) 0) != pdTRUE) {
				// We couldn't queue the buffer, so release it.
				RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
			} else {
				vTaskResume(gRadioTransmitTask);
			}
		} else {
			RELEASE_TX_BUFFER(txBufferNum, ccrHolder);
		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}
