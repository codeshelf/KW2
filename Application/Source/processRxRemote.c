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
#include "CRC1.h"

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
	PacketVerType packetVersion;
	uint32_t crc = 0;
	LDD_TError err;

	packetVersion = getPacketVersion(gRxRadioBuffer[inRxBufferNum].bufferStorage);

	// We just received a valid packet.
	networkID = getNetworkID(inRxBufferNum);

	if (packetVersion != PACKET_VERSION) {
		// Do nothing if the packet version is incorrect

	} else if ((networkID != BROADCAST_NET_NUM) && (networkID != gMyNetworkID)) {
		// Only process packets sent to the broadcast network ID and our assigned network ID.
		// Do nothing
	} else {
		// Reset watchdog if we see messages to our network
		GW_WATCHDOG_RESET;
		setStatusLed(0, 0, 1);

		cmdID = getCommandID(gRxRadioBuffer[inRxBufferNum].bufferStorage);
		cmdDstAddr = getCommandDstAddr(inRxBufferNum);

		// Only process commands sent to the broadcast address or our assigned address.
		if ((cmdDstAddr != ADDR_BROADCAST) && (cmdDstAddr != gMyAddr)) {
			// Do nothing.
		} else {

			// Prepare to handle packet ACK.
			ackId = getAckId(gRxRadioBuffer[inRxBufferNum].bufferStorage);

			// Send an ACK if necessary.
			if (ackId != 0) {
				txBufferNum = lockTxBuffer();
				createAckPacket(txBufferNum, ackId, lqi);

				if (transmitPacket(txBufferNum)) {
				}
			}

			switch (cmdID) {
				case eCommandNetMgmt:
					setStatusLed(0, 0, 1);
					GW_WATCHDOG_RESET;
					break;
				case eCommandAssoc:
					// Should not get command associate messages at this stage
					break;

				case eCommandControl:
					if (gLocalDeviceState == eLocalStateRun) {

						if ((ackId == 0 || ackId != gLastAckId)) {
							// Make sure that there is a valid sub-command in the control command.

							if (ackId != 0) {
								gLastAckId = ackId;
							}

							switch (getControlSubCommand(inRxBufferNum)) {

								case eControlSubCmdScan:
									break;

								case eControlSubCmdAck:
									shouldReleasePacket = FALSE;
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

								case eControlSubCmdCreateScan:
									processCreateScanSubCommand(inRxBufferNum);
									break;

								case eControlSubCmdCreateButton:
									processCreateButtonSubCommand(inRxBufferNum);
									break;

								case eControlSubCmdPosconAddrDisp:
									processDspAddrPosControllerSubCommand(inRxBufferNum);
									break;

								case eControlSubCmdPosconSetupStart:
									processPosControllerMassSetupStart();
									break;

								case eControlSubCmdPosconLedBroadcast:
									processPosconLedBroadcastSubCommand(inRxBufferNum);
									break;

								case eControlSubCmdPosconFWVerDisplay:
									processPosconFWVerDisplaySubCommand(inRxBufferNum);
									break;

								case eControlSubCmdPosconSetBroadcast:
									processPosconSetBroadcastSubCommand(inRxBufferNum);
									break;

								case eControlSubCmdPosconBroadcast:
									processPosconBroadcastSubCommand(inRxBufferNum);
									break;

								default:
									break;
							}
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

}
