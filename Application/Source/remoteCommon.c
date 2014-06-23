/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "remoteCommon.h"
#include "remoteRadioTask.h"
#include "commands.h"
#include "string.h"

// --------------------------------------------------------------------------
// Global variables.

UnixTimeType gUnixTime;

extern RxRadioBufferStruct	gRxRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType		gRxCurBufferNum;
extern BufferCntType		gRxUsedBuffers;

extern xQueueHandle			gRemoteMgmtQueue;
extern NetworkIDType		gMyNetworkID;
extern NetAddrType			gMyAddr;

extern portTickType			gLastPacketReceivedTick;
extern ELocalStatusType		gLocalDeviceState;

// --------------------------------------------------------------------------

void processRxPacket(BufferCntType inRxBufferNum) {

	ECommandGroupIDType cmdID;
	NetAddrType cmdDstAddr;
	NetworkIDType networkID;
	ECmdAssocType assocSubCmd;
	AckDataType ackData;
	AckIDType ackId;
	gwUINT8 ccrHolder;
	EControlCmdAckStateType ackState;
	gwBoolean shouldReleasePacket;
	BufferCntType txBufferNum;

	shouldReleasePacket = TRUE;

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
			ackState = eAckStateNotNeeded;
			memset(ackData, 0, ACK_DATA_BYTES);

			switch (cmdID) {

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
					if (xQueueSend(gRemoteMgmtQueue, &inRxBufferNum, (portTickType) 0)) {
						// The management task will handle this packet.
						shouldReleasePacket = FALSE;
					}
				}
				break;

			case eCommandControl:

				// Make sure that there is a valid sub-command in the control command.
				switch (getControlSubCommand(inRxBufferNum)) {
//						case eControlSubCmdEndpointAdj:
//							break;

				case eControlSubCmdScan:
					break;

				case eControlSubCmdMessage:
					ackState = processDisplayMsgSubCommand(inRxBufferNum);
					break;

				case eControlSubCmdLight:
					ackState = processLedSubCommand(inRxBufferNum);
					break;

				case eControlSubCmdSetPosController:
					ackState = processSetPosControllerSubCommand(inRxBufferNum);
					break;

				case eControlSubCmdClearPosController:
					ackState = processClearPosControllerSubCommand(inRxBufferNum);
					break;

				default:
					// Bogus command.
					// Immediately free this command buffer since we'll never do anything with it.
					//RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
					break;
				}
				break;

			default:
				break;
			}

			// Send an ACK if necessary.
			if ((ackState == eAckStateOk) && (ackId != 0)) {
				txBufferNum = lockTxBuffer();
				createAckPacket(txBufferNum, ackId, ackData);
				if (transmitPacket(txBufferNum)) {
				}
			}

		}
	}

	// If we need to release the packet then do it.
	if (shouldReleasePacket) {
		RELEASE_RX_BUFFER(inRxBufferNum, ccrHolder);
	}
}
