/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "processRx.h"
#include "remoteRadioTask.h"
#include "radioCommon.h"
#include "commands.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// --------------------------------------------------------------------------
// Global variables.

AckIDType gLastAckId = 0;
UnixTimeType gUnixTime;

extern RxRadioBufferStruct gRxRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType gRxCurBufferNum;
extern BufferCntType gRxUsedBuffers;

extern TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType 		gTxCurBufferNum;
extern BufferCntType 		gTxUsedBuffers;

extern xQueueHandle gRemoteMgmtQueue;
extern xQueueHandle gTxAckQueue;
extern NetworkIDType gMyNetworkID;
extern NetAddrType gMyAddr;

extern portTickType gLastPacketReceivedTick;
extern ELocalStatusType gLocalDeviceState;
extern RadioStateEnum gRadioState;

// --------------------------------------------------------------------------

void processRxPacket(BufferCntType inRxBufferNum, uint8_t lqi) {

	ECommandGroupIDType cmdID;
	NetAddrType cmdDstAddr;
	NetworkIDType networkID;
	ECmdAssocType assocSubCmd;
	AckDataType ackData;
	AckIDType ackId;
	BufferCntType txBufferNum;
	gwBoolean shouldReleasePacket = TRUE;
	gwUINT8 ccrHolder;

	// We just received a valid packet.
	networkID = getNetworkID(inRxBufferNum);

	// Only process packets sent to the broadcast network ID and our assigned network ID.
	if ((networkID != BROADCAST_NET_NUM) && (networkID != gMyNetworkID)) {
		// Do nothing
	} else {

		cmdID = getCommandID(gRxRadioBuffer[inRxBufferNum].bufferStorage);
		cmdDstAddr = getCommandDstAddr(inRxBufferNum);

		// Only process commands sent to the broadcast address or our assigned address.
		if ((cmdDstAddr != ADDR_BROADCAST) && (cmdDstAddr != gMyAddr)) {
			// Do nothing.
		} else {

			// Prepare to handle packet ACK.
			ackId = getAckId(gRxRadioBuffer[inRxBufferNum].bufferStorage);
			
				

				switch (cmdID) {
					case eCommandNetMgmt:
						GW_WATCHDOG_RESET;
						setStatusLed(0, 0, 1);
						break;
					case eCommandAssoc:
						// This will only return sub-commands if the command GUID matches out GUID
						assocSubCmd = getAssocSubCommand(inRxBufferNum);
						if (assocSubCmd == eCmdAssocInvalid) {
							// Do nothing.
						} else if (assocSubCmd == eCmdAssocRESP) {
							// If we're not already running then signal the mgmt task that we just got a command ASSOC resp.
							if (gLocalDeviceState != eLocalStateRun) {
								if (xQueueSend(gRemoteMgmtQueue, &inRxBufferNum, (portTickType) 0)) {
									// The management task will handle this packet.
									shouldReleasePacket = FALSE;
								}
							}
						} else if (assocSubCmd == eCmdAssocACK) {
								// Record the time of the last ACK packet we received.
								gLastPacketReceivedTick = xTaskGetTickCount();
		
								// If the associate state is 1 then we're not associated with this controller anymore.
								// We need to reset the device, so that we can start a whole new session.
								if (1 == gRxRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_STATE]) {
									//GW_RESET_MCU();
								} else {
									gUnixTime.byteFields.byte1 = gRxRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 3];
									gUnixTime.byteFields.byte2 = gRxRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 2];
									gUnixTime.byteFields.byte3 = gRxRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME + 1];
									gUnixTime.byteFields.byte4 = gRxRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME];
								}
								if ((networkID == gMyNetworkID) && (cmdDstAddr == gMyAddr)) {
									if (xQueueSend(gTxAckQueue, &ackId, (portTickType) 0)) {
									}
								/*
								} else if (xQueueSend(gRemoteMgmtQueue, &inRxBufferNum, (portTickType) 0)) {
									// The management task will handle this packet.
									shouldReleasePacket = FALSE;
								}
								*/
								} else if (gLocalDeviceState != eLocalStateRun) {
									if (xQueueSend(gRemoteMgmtQueue, &inRxBufferNum, (portTickType) 0)) {
										// The management task will handle this packet.
										shouldReleasePacket = FALSE;
									}
								}
						}
						break;
	
					case eCommandControl:
						if (gLocalDeviceState == eLocalStateRun) {
							
							if ((ackId == 0 || ackId != gLastAckId)) {
								// Make sure that there is a valid sub-command in the control command.
								
								switch (getControlSubCommand(inRxBufferNum)) {
			
									case eControlSubCmdScan:
										break;
										
									case eControlSubCmdAck:
										processAckSubCommand(inRxBufferNum);
										break;
			
									case eControlSubCmdMessage:
										processDisplayMsgSubCommand(inRxBufferNum);
										break;
										
									case eControlSubCmdSingleLineMessage:
										processDisplaySingleMsgSubCommand(inRxBufferNum);
										break;
										
									case eControlSubCmdClearDisplay:
										processClearDisplay(inRxBufferNum);
										break;
			
									case eControlSubCmdLight:
										processLedSubCommand(inRxBufferNum);
										break;
			
									case eControlSubCmdSetPosController:
										processSetPosControllerSubCommand(inRxBufferNum);
										break;
			
									case eControlSubCmdClearPosController:
										processClearPosControllerSubCommand(inRxBufferNum);
										break;
			
									default:
										break;
								}
							
							
								gLastAckId = ackId;
							}
							
							// Send an ACK if necessary.
							if (ackId != 0 /*&&  ackId != gLastAckId*/) {
								//memset(ackData, 0, ACK_DATA_BYTES);
								txBufferNum = lockTxBuffer();
								//ackData[0] = lqi;
								createAckPacket(txBufferNum, ackId, lqi);
								
								if (transmitPacket(txBufferNum)) {
									while (gTxRadioBuffer[txBufferNum].bufferStatus != eBufferStateFree) {
										vTaskDelay(0);
									}
								}
								//gLastAckId = ackId;
							}
						}
						break;
	
					default:
						break;
				}
			}
		}

	if (shouldReleasePacket) {
			RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
	}
	vTaskDelay(0);
}
