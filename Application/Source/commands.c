/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#include "commands.h"
#include "radioCommon.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "string.h"
#include "gwSystemMacros.h"
#include "Wait.h"
#include "globals.h"
#include "scannerReadTask.h"

#ifdef SHARP_DISPLAY
#include "display.h"
#endif

#ifdef RS485
#include "Rs485.h"
#endif

RemoteDescStruct gRemoteStateTable[MAX_REMOTES];
NetAddrType gMyAddr = INVALID_REMOTE;
NetworkIDType gMyNetworkID = BROADCAST_NET_NUM;
portTickType gSleepWaitMillis;
BufferStorageType gLastTxAckId = 1;
EScannerType gScannerType = 0;

extern xQueueHandle gRadioTransmitQueue;
extern xQueueHandle gRadioReceiveQueue;
extern xQueueHandle gTxAckQueue;
extern ControllerStateType gControllerState;
extern ELocalStatusType gLocalDeviceState;

extern RxRadioBufferStruct gRxRadioBuffer[RX_BUFFER_COUNT];
extern BufferCntType gRxCurBufferNum;
extern BufferCntType gRxUsedBuffers;

extern TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];
extern BufferCntType gTxCurBufferNum;
extern BufferCntType gTxUsedBuffers;

extern RadioStateEnum gRadioState;

// --------------------------------------------------------------------------
// Local function prototypes

void createPacket(BufferCntType inTXBufferNum, ECommandGroupIDType inCmdID, NetworkIDType inNetworkID, NetAddrType inSrcAddr,
        NetAddrType inDestAddr);

// --------------------------------------------------------------------------

gwUINT8 transmitPacket(BufferCntType inTXBufferNum) {
	gwUINT8 result = 0;
	BufferCntType txBufferNum = inTXBufferNum;

	// Transmit the packet.
	result = xQueueGenericSend(gRadioTransmitQueue, &txBufferNum, (portTickType) 0, (portBASE_TYPE) queueSEND_TO_BACK );

	return result;

}
// --------------------------------------------------------------------------

gwUINT8 transmitPacketFromISR(BufferCntType inTXBufferNum) {
	gwUINT8 result = 0;

	// Transmit the packet.
	result = xQueueSendFromISR(gRadioTransmitQueue, &inTXBufferNum, (portTickType) 0);

	return result;

}
// --------------------------------------------------------------------------

AckIDType getAckId(BufferStoragePtrType inBufferPtr) {
	// We know we need to ACK if the command ID is not zero.
	return (inBufferPtr[PCKPOS_ACK_ID]);
}
// --------------------------------------------------------------------------

void setAckId(BufferStoragePtrType inBufferPtr) {
	portTickType ticks = xTaskGetTickCount();
	inBufferPtr[PCKPOS_ACK_ID] = gLastTxAckId++;
}
// --------------------------------------------------------------------------

ECommandGroupIDType getCommandID(BufferStoragePtrType inBufferPtr) {

	// The command number is in the third half-byte of the packet.
	ECommandGroupIDType result = ((inBufferPtr[CMDPOS_CMD_ID]) & CMDMASK_CMDID) >> 4;
	return result;
}

// --------------------------------------------------------------------------

//ECommandGroupIDType getCommandID(BufferStoragePtrType inBufferPtr, PacketVerType inPacketVersion) {
//	ECommandGroupIDType result;
//
//	if (inPacketVersion == PROTOCOL_VER_0) {
//		result = ((inBufferPtr[CMDPOS_CMD_ID - 4]) & CMDMASK_CMDID) >> 4;
//	} else {
//		result = ((inBufferPtr[CMDPOS_CMD_ID]) & CMDMASK_CMDID) >> 4;
//	}
//
//	return result;
//}

// --------------------------------------------------------------------------

NetworkIDType getNetworkID(BufferCntType inRXBufferNum) {
	return ((gRxRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_NET_NUM] & PACKETMASK_NET_NUM) >> SHIFTBITS_PKT_NET_NUM);
}

// --------------------------------------------------------------------------

gwUINT8 getPacketVersion(BufferStoragePtrType inBufferPtr) {
	return ((inBufferPtr[PCKPOS_VERSION] & PACKETMASK_VERSION) >> SHIFTBITS_PKT_VER);
}

// --------------------------------------------------------------------------

gwBoolean getCommandRequiresACK(BufferCntType inRXBufferNum) {
	return (getAckId != 0);
}

// --------------------------------------------------------------------------

gwUINT16 getCRC(BufferStoragePtrType inBufferPtr) {
	gwUINT16 crc = 0x0;

	crc |= (uint16) (inBufferPtr[PCKPOS_CRC]<<8);
	crc |= (uint16) (inBufferPtr[PCKPOS_CRC+1] & 0xFF);

	return crc;
}

// --------------------------------------------------------------------------

void setCRC(BufferStoragePtrType inBufferPtr, gwUINT16 crc) {
	inBufferPtr[PCKPOS_CRC] = (crc >> 8) & 0xFF;
	inBufferPtr[PCKPOS_CRC+1] = (crc) & 0xFF;
}

// --------------------------------------------------------------------------

gwUINT16 calculateCRC16(BufferStoragePtrType inBufferPtr, BufferCntType inPcktSize) {
	gwUINT32 crc = 0;
	gwUINT8 size = inPcktSize - (gwUINT8)(PCKPOS_HDR_ENDPOINT);

	gwUINT8 ccrHolder;

	GW_ENTER_CRITICAL(ccrHolder);
	CRC1_ResetCRC(CRC1_DeviceData);
	CRC1_GetBlockCRC (CRC1_DeviceData, &(inBufferPtr[PCKPOS_HDR_ENDPOINT]), size, &crc);
	GW_EXIT_CRITICAL(ccrHolder);

	return (gwUINT16) crc & 0xFFFF;
}

// --------------------------------------------------------------------------

gwBoolean checkCRC(BufferStoragePtrType inBufferPtr, BufferCntType inPcktSize) {
	gwUINT16 pcktCrc = 0x0;
	gwUINT16 calculatedCrc = 0x0;
	gwUINT8		pcktVer = 0;

	pcktCrc = getCRC(inBufferPtr);
	calculatedCrc = calculateCRC16(inBufferPtr, inPcktSize);

	if (calculatedCrc == pcktCrc) {
		return TRUE;
	} else {
		return FALSE;
	}
}

// --------------------------------------------------------------------------

EndpointNumType getEndpointNumber(BufferCntType inRXBufferNum) {

	// The command number is in the third half-byte of the packet.
	EndpointNumType result = ((gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ENDPOINT] & CMDMASK_ENDPOINT)
	        >> SHIFTBITS_CMD_ENDPOINT);
	return result;
}

// --------------------------------------------------------------------------

NetAddrType getCommandSrcAddr(BufferCntType inRXBufferNum) {

	NetAddrType result = 0;
	result |= (uint16) (gRxRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_SRC_ADDR]<<8);
	result |= (uint16) (gRxRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_SRC_ADDR+1] & 0xFF);
	return result;
}

// --------------------------------------------------------------------------

NetAddrType getCommandDstAddr(BufferCntType inRXBufferNum) {

	NetAddrType result = 0;
	result |= (uint16) (gRxRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_DST_ADDR]<<8);
	result |= (uint16) (gRxRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_DST_ADDR+1] & 0xFF);
	return result;
}

// --------------------------------------------------------------------------

ENetMgmtSubCmdIDType getNetMgmtSubCommand(BufferStoragePtrType inBufferPtr) {
	ENetMgmtSubCmdIDType result = (inBufferPtr[CMDPOS_NETM_SUBCMD]);
	return result;
}

// --------------------------------------------------------------------------

//ENetMgmtSubCmdIDType getNetMgmtSubCommand(BufferStoragePtrType inBufferPtr, PacketVerType inPacketVersion) {
//	ENetMgmtSubCmdIDType result;
//
//	if (inPacketVersion == PROTOCOL_VER_0) {
//		result = (inBufferPtr[CMDPOS_NETM_SUBCMD - 4]);
//	} else {
//		result = (inBufferPtr[CMDPOS_NETM_SUBCMD]);
//	}
//	return result;
//}

// --------------------------------------------------------------------------

ECmdAssocType getAssocSubCommand(BufferCntType inRXBufferNum) {
	ECmdAssocType result = eCmdAssocInvalid;
	// Make sure the command is actually for us.
	if (memcmp(gGuid, &(gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_GUID]), UNIQUE_ID_BYTES) == 0) {
		result = (gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD]);
	}
	return result;
}

// --------------------------------------------------------------------------

EControlSubCmdIDType getControlSubCommand(BufferCntType inRXBufferNum) {
	EControlSubCmdIDType result = (gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD]);
	return result;
}

// --------------------------------------------------------------------------

void writeAsPString(BufferStoragePtrType inDestPtr, const BufferStoragePtrType inStringPtr, gwUINT8 inStringLen) {
	inDestPtr[0] = (gwUINT8) inStringLen;
	memcpy(inDestPtr + 1, inStringPtr, (gwUINT8) inStringLen);
}

// --------------------------------------------------------------------------

gwUINT8 readAsPString(BufferStoragePtrType inDestStringPtr, const BufferStoragePtrType inSrcPtr, gwUINT8 inMaxBytes) {
	gwUINT8 stringLen = getMin((gwUINT8) inSrcPtr[0], inMaxBytes);
	memcpy(inDestStringPtr, inSrcPtr + 1, (gwUINT8) stringLen);
	inDestStringPtr[stringLen] = (gwUINT8) NULL;
	return stringLen;
}

// --------------------------------------------------------------------------

void createPacket(BufferCntType inTXBufferNum, ECommandGroupIDType inCmdID, NetworkIDType inNetworkID, NetAddrType inSrcAddr,
        NetAddrType inDestAddr) {

	// First clear the packet header.
	memset((void *) gTxRadioBuffer[inTXBufferNum].bufferStorage, 0, CMDPOS_CMD_ID + 1);

	// Write the packet header.
	// (See the comments at the top of commandTypes.h.)
	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_NET_NUM] |= (inNetworkID << SHIFTBITS_PKT_NET_NUM);

	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_SRC_ADDR] = (inSrcAddr >> 8) & 0xFF;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_SRC_ADDR + 1] = (inSrcAddr) & 0xFF;

	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_DST_ADDR] = (inDestAddr >> 8) & 0xFF;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_DST_ADDR + 1] = (inDestAddr) & 0xFF;

	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_ACK_ID] = 0;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CMD_ID] = (inCmdID << SHIFTBITS_CMD_ID);

	gTxRadioBuffer[inTXBufferNum].bufferSize += 5;
}

// --------------------------------------------------------------------------

void createAssocReqCommand(BufferCntType inTXBufferNum) {

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createPacket(inTXBufferNum, eCommandAssoc, BROADCAST_NET_NUM, ADDR_CONTROLLER, ADDR_BROADCAST);

	// Set the AssocReq sub-command
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD] = eCmdAssocREQ;

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_GUID]), gGuid, UNIQUE_ID_BYTES);

	// Set the hardware version (4 bytes)
	memcpy((void *) &(gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_HW_VER]), STR(HARDWARE_VERSION), 4);

	// Set the firmware version (4 bytes)
	memcpy((void *) &(gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_FW_VER]), STR(FIRMWARE_VERSION), 4);

	// Set the radio protocol version (1 byte)
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_RP_VER] = RADIO_PROTOCOL_VERSION;

	// Set the system status register
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_SYSSTAT] = GW_GET_SYSTEM_STATUS;

	gTxRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ASSOCREQ_SYSSTAT + 1;
}


// --------------------------------------------------------------------------

void createAssocCheckCommand(BufferCntType inTXBufferNum) {

	gwUINT8 batteryLevel;

	// Create the command for checking if we're associated
	createPacket(inTXBufferNum, eCommandAssoc, gMyNetworkID, gMyAddr, ADDR_BROADCAST);

	// Set the AssocReq sub-command
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD] = eCmdAssocCHECK;

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_GUID]), gGuid, UNIQUE_ID_BYTES);

	// Nominalize to a 0-100 scale.
	if (batteryLevel < 0) {
		batteryLevel = 0;
	} else {
		// Double to get to a 100 scale.
		batteryLevel = batteryLevel << 1;
	}

	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_BATT] = batteryLevel;
	
	if ( RCM_SRS0 & RCM_SRS0_PIN_MASK ) { 
		// Reset button
		gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_LRC] = eUserRestart;
		gProgramCounter = 0;
	} else if (RCM_SRS0 & RCM_SRS0_POR_MASK) {
		// Power on
		gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_LRC] = ePowerOn;
		gProgramCounter = 0;
	} else {
		gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_LRC] = gRestartCause;
	}

	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_RCM0] = GW_GET_SYSTEM_STATUS0;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_RCM1] = GW_GET_SYSTEM_STATUS1;

	
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_PC + 0] = (gProgramCounter >> 24) & 0xFF;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_PC + 1] = (gProgramCounter >> 16) & 0xFF;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_PC + 2] = (gProgramCounter >> 8) & 0xFF;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_PC + 3] = (gProgramCounter >> 0) & 0xFF;
	
	gTxRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ASSOCCHK_PC + 4;

}

// --------------------------------------------------------------------------

void createAckPacket(BufferCntType inTXBufferNum, AckIDType inAckId, AckLQIType inAckLQI) {

	createPacket(inTXBufferNum, eCommandControl, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD] = eControlSubCmdAck;

	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ACK_NUM] = inAckId;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ACK_LQI] = inAckLQI;

	gTxRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ACK_LQI + 1;
}

// --------------------------------------------------------------------------

#ifdef GATEWAY
void createOutboundNetSetup() {
	BufferCntType txBufferNum;
	gwUINT8 ccrHolder;

	txBufferNum = lockTxBuffer();

	//vTaskSuspend(gRadioReceiveTask);

	// This command gets setup in the TX buffers, because it only gets sent back to the controller via
	// the serial interface.  This command never comes from the air.  It's created by the gateway (dongle)
	// directly.

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	//createPacket(inTXBufferNum, eCommandNetMgmt, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
	gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
	gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_NET_NUM] |= (BROADCAST_NET_NUM << SHIFTBITS_PKT_NET_NUM);
	gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_SRC_ADDR] = ADDR_CONTROLLER;
	gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_DST_ADDR] = ADDR_CONTROLLER;
	gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CMD_ID] = (eCommandNetMgmt << SHIFTBITS_CMD_ID);

	// Set the sub-command.
	gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_SUBCMD] = eNetMgmtSubCmdNetSetup;
	gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_SETCMD_CHANNEL] = 0;

	gTxRadioBuffer[txBufferNum].bufferSize = CMDPOS_NETM_SETCMD_CHANNEL + 1;

	serialTransmitFrame(UART0_BASE_PTR, (gwUINT8*) (&gTxRadioBuffer[txBufferNum].bufferStorage), gTxRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

//	vTaskResume(gRadioReceiveTask);

}
#endif

// --------------------------------------------------------------------------

void createScanCommand(BufferCntType inTXBufferNum, ScanStringPtrType inScanStringPtr, ScanStringLenType inScanStringLen) {

	createPacket(inTXBufferNum, eCommandControl, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);
	setAckId(gTxRadioBuffer[inTXBufferNum].bufferStorage);
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD] = eControlSubCmdScan;

	writeAsPString(gTxRadioBuffer[inTXBufferNum].bufferStorage + CMDPOS_CONTROL_DATA, (BufferStoragePtrType) inScanStringPtr,
	        inScanStringLen);

	gTxRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_STARTOFCMD + inScanStringLen + 2;
}

// --------------------------------------------------------------------------

void createButtonCommand(BufferCntType inTXBufferNum, gwUINT8 inButtonNum, gwUINT8 inValue) {

	createPacket(inTXBufferNum, eCommandControl, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CONTROL_SUBCMD] = eControlSubCmdButton;

	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_BUTTON_NUM] = inButtonNum;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_BUTTON_VAL] = inValue;

	gTxRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_BUTTON_VAL + 1;
}

// --------------------------------------------------------------------------

void processNetSetupCommand(BufferCntType inTXBufferNum) {

	ChannelNumberType channel;
	gwUINT8 ccrHolder;

	// Network Setup ALWAYS comes in via the serial interface to the gateway (dongle)
	// This means we process it FROM the TX buffers and SEND from the TX buffers.
	// These commands NEVER go onto the air.

	// Get the requested channel number.
	channel = gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_SETCMD_CHANNEL];
	setRadioChannel(channel);

	gLocalDeviceState = eLocalStateRun;
}
// --------------------------------------------------------------------------

#ifdef GATEWAY
void processNetIntfTestCommand(BufferCntType inTXBufferNum) {

	BufferCntType txBufferNum;
	gwUINT8 ccrHolder;

	/*
	 * At this point we transmit one inbound interface test back over the serial link from the gateway (dongle) itself.
	 *
	 * The only rational way to do this is to use a transmit buffer.  The reason is that the radio may
	 * already be waiting to fill an inbound packet that we already sent to the MAC.  There is no
	 * way to let the MAC know that we're about to switch the current RX buffer.  For this reason
	 * the only safe buffer available to us comes from the TX buffer.
	 */

	txBufferNum = lockTxBuffer();

	//vTaskSuspend(gRadioReceiveTask);

	// This command gets setup in the TX buffers, because it only gets sent back to the controller via
	// the serial interface.  This command never comes from the air.  It's created by the gateway (dongle)
	// directly.

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	//createPacket(inTXBufferNum, eCommandNetMgmt, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
	gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
	gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_NET_NUM] |= (BROADCAST_NET_NUM << SHIFTBITS_PKT_NET_NUM);
	gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_SRC_ADDR] = ADDR_CONTROLLER;
	gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_DST_ADDR] = ADDR_CONTROLLER;
	gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CMD_ID] = (eCommandNetMgmt << SHIFTBITS_CMD_ID);

	// Set the sub-command.
	gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_SUBCMD] = eNetMgmtSubCmdNetIntfTest;
	gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_TSTCMD_NUM] =
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_TSTCMD_NUM];

	gTxRadioBuffer[txBufferNum].bufferSize = CMDPOS_NETM_TSTCMD_NUM + 1;

	serialTransmitFrame(UART0_BASE_PTR, (gwUINT8*) (&gTxRadioBuffer[txBufferNum].bufferStorage), gTxRadioBuffer[txBufferNum].bufferSize);

//	vTaskResume(gRadioReceiveTask);

}

// --------------------------------------------------------------------------

void processNetCheckOutboundCommand(BufferCntType inTXBufferNum) {
	BufferCntType txBufferNum;
	ChannelNumberType channel;

	//vTaskSuspend(gRadioReceiveTask);

	// Switch to the channel requested in the outbound net-check.
	channel = gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_CHANNEL];
	setRadioChannel(channel);

	// We need to put the gateway (dongle) GUID into the outbound packet before it gets transmitted.
	memcpy(&(gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_GUID]), gGuid, UNIQUE_ID_BYTES);

	/*
	 * At this point we transmit one inbound net-check back over the serial link from the gateway (dongle) itself.
	 * The reason is that we need to guarantee that at least one net-check goes back to the controller
	 * for each channel.  If there are no other controllers operating or listening at least the controller
	 * will have a net-check from the dongle itself.  This is necessary, so that the controller can
	 * assess the energy detect (ED) for each channel.  Even though there may be no other controllers
	 * on the channel, but there may be other systems using the channel with different protocols or
	 * modulation schemes.
	 *
	 * The only rational way to do this is to use a transmit buffer.  The reason is that the radio may
	 * already be waiting to fill an inbound packet that we already sent to the MAC.  There is no
	 * way to let the MAC know that we're about to switch the current RX buffer.  For this reason
	 * the only safe buffer available to us comes from the TX buffer.
	 */

	if (gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_TYPE] == eCmdAssocREQ) {
		gwUINT8 ccrHolder;

		txBufferNum = lockTxBuffer();

		// This command gets setup in the TX buffers, because it only gets sent back to the controller via
		// the serial interface.  This command never comes from the air.  It's created by the gateway (dongle)
		// directly.

		// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
		//createPacket(inTXBufferNum, eCommandNetMgmt, BROADCAST_NETID, ADDR_CONTROLLER, ADDR_BROADCAST);
		gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
		gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_NET_NUM] |= (BROADCAST_NET_NUM << SHIFTBITS_PKT_NET_NUM);
		gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_SRC_ADDR] = ADDR_CONTROLLER;
		gTxRadioBuffer[txBufferNum].bufferStorage[PCKPOS_DST_ADDR] = ADDR_CONTROLLER;
		gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_CMD_ID] = (eCommandNetMgmt << SHIFTBITS_CMD_ID);

		// Set the sub-command.
		gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_SUBCMD] = eNetMgmtSubCmdNetCheck;
		gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_TYPE] = eCmdAssocRESP;

		gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_NET_NUM] = BROADCAST_NET_NUM;
		memcpy((void *) &(gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_GUID]), PRIVATE_GUID, UNIQUE_ID_BYTES);
		gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_CHANNEL] = channel;
		gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_ENERGY] = GW_ENERGY_DETECT(channel);
		gTxRadioBuffer[txBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_LINKQ] = 0;

		gTxRadioBuffer[txBufferNum].bufferSize = CMDPOS_NETM_CHKCMD_LINKQ + 1;

		serialTransmitFrame(UART0_BASE_PTR, (gwUINT8*) (&gTxRadioBuffer[txBufferNum].bufferStorage), gTxRadioBuffer[txBufferNum].bufferSize);

//		vTaskResume(gRadioReceiveTask);

	}
//	gTxRadioBuffer[inTXBufferNum].bufferStatus = eBufferStateFree;
}
#endif

// --------------------------------------------------------------------------

void processNetCheckInboundCommand(BufferCntType inRXBufferNum) {
	// The gateway (dongle) needs to add the link quality to the channel energy field.
	// This way the gateway can assess channel energy.
}
// --------------------------------------------------------------------------

void processAssocRespCommand(BufferCntType inRXBufferNum) {

	NetAddrType result = INVALID_REMOTE;
	gwUINT8 ccrHolder;

	gMyAddr = (uint16) (gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_ADDR] << 8);
	gMyAddr |= (uint16) (gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_ADDR + 1] & 0xFF);

	gMyNetworkID = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_NET];
	gSleepWaitMillis = (gwUINT16) gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_SLEEP + 0] << 8;
	gSleepWaitMillis += (gwUINT16) gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_SLEEP + 1];

	gScannerType = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_SCANNER];
	gSleepWaitMillis *= 1000;
	gLocalDeviceState = eLocalStateAssociated;
}

// --------------------------------------------------------------------------

void processAssocAckCommand(BufferCntType inRXBufferNum) {
	// No can do in FreeRTOS :-(
	//xTaskSetTickCount(gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME)]);
}

// --------------------------------------------------------------------------

void processAckSubCommand(BufferCntType inRxBufferNum) {
	AckIDType ackId;

	ackId = gRxRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_ACK_NUM];

	if (xQueueSend(gTxAckQueue, &ackId, (portTickType) 0)) {
	}
}

// --------------------------------------------------------------------------

DisplayStringType gDisplayDataLine[4];
DisplayStringLenType gDisplayDataLineLen[4];
DisplayStringLenType gDisplayDataLinePos[4];

void processDisplayMsgSubCommand(BufferCntType inRXBufferNum) {
#ifdef CHE_CONTROLLER
	gwUINT8 ccrHolder, i;

	GW_ENTER_CRITICAL(ccrHolder);
	clearDisplay();
	
	if (gScannerType == eCodeCorp3600) {
		clearScannerDisplay();
	}
	
	// First display line.
	BufferStoragePtrType bufferPtr = gRxRadioBuffer[inRXBufferNum].bufferStorage + CMDPOS_MESSAGE;
	gDisplayDataLineLen[0] = readAsPString(gDisplayDataLine[0], bufferPtr, MAX_DISPLAY_STRING_BYTES);

	displayMessage(1, gDisplayDataLine[0], FONT_NORMAL);

	// Second display line.
	bufferPtr += gDisplayDataLineLen[0] + 1;
	gDisplayDataLineLen[1] = readAsPString(gDisplayDataLine[1], bufferPtr, MAX_DISPLAY_STRING_BYTES);

	displayMessage(2, gDisplayDataLine[1], FONT_NORMAL);

	// Third display line.
	bufferPtr += gDisplayDataLineLen[1] + 1;
	gDisplayDataLineLen[2] = readAsPString(gDisplayDataLine[2], bufferPtr, MAX_DISPLAY_STRING_BYTES);

	displayMessage(3, gDisplayDataLine[2], FONT_NORMAL);

	// Fourth display line.
	bufferPtr += gDisplayDataLineLen[2] + 1;
	gDisplayDataLineLen[3] = readAsPString(gDisplayDataLine[3], bufferPtr, MAX_DISPLAY_STRING_BYTES);

	displayMessage(4, gDisplayDataLine[3], FONT_NORMAL);

	// Send lines to CodeCorp3600 scanner if attached
	if (gScannerType == eCodeCorp3600) {

		for (i=0; i<4; i++) {
			if (gDisplayDataLineLen[i] > 0) {
				sendLineToScanner(gDisplayDataLine[i],gDisplayDataLineLen[i]);
			}
		}
	}

	GW_EXIT_CRITICAL(ccrHolder);
#endif
}

// --------------------------------------------------------------------------

void processDisplaySingleMsgSubCommand(BufferCntType inRXBufferNum) {
#ifdef CHE_CONTROLLER
	gwUINT8 ccrHolder;
	gwUINT8 fontType;
	gwUINT16 posX = 0;
	gwUINT16 posY = 0;

	GW_ENTER_CRITICAL(ccrHolder);

	// First display line.
	BufferStoragePtrType bufferPtr = gRxRadioBuffer[inRXBufferNum].bufferStorage + CMDPOS_MESSAGE_SINGLE;
	gDisplayDataLineLen[0] = readAsPString(gDisplayDataLine[0], bufferPtr, MAX_DISPLAY_STRING_BYTES);

	fontType = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_MESSAGE_FONT];

	posX |= (uint16) (gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_MESSAGE_POSX]<<8);
	posX |= (uint16) (gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_MESSAGE_POSX+1] & 0xFF);

	posY |= (uint16) (gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_MESSAGE_POSY]<<8);
	posY |= (uint16) (gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_MESSAGE_POSY+1] & 0xFF);

	displayStringByFont(posX, posY, gDisplayDataLine[0], 1, fontType);

	// Send to scanner display if connected to CodeCorp3600
	if (gScannerType == eCodeCorp3600) {
		if (gDisplayDataLineLen[0] > 0) {
			sendLineToScanner(gDisplayDataLine[0],gDisplayDataLineLen[0]);
		}
	}

	GW_EXIT_CRITICAL(ccrHolder);
#endif
}

// --------------------------------------------------------------------------

void processClearDisplay(BufferCntType inRXBufferNum) {
#ifdef CHE_CONTROLLER
	gwUINT8 ccrHolder;

	GW_ENTER_CRITICAL(ccrHolder);

	clearDisplay();
	if (gScannerType == eCodeCorp3600) {
		clearScannerDisplay();
	}

	GW_EXIT_CRITICAL(ccrHolder);
#endif
}

// --------------------------------------------------------------------------

LedPositionType gTotalLedPositions;

LedDataStruct gLedFlashData[MAX_LED_FLASH_POSITIONS];
LedPositionType gCurLedFlashDataElement;
LedData	gCurLedFlashBitPos;
LedPositionType gTotalLedFlashDataElements;
LedPositionType gNextFlashLedPosition;

LedDataStruct gLedSolidData[MAX_LED_SOLID_POSITIONS];
LedPositionType gCurLedSolidDataElement;
LedData	gCurLedSolidBitPos;
LedPositionType gTotalLedSolidDataElements;
LedPositionType gNextSolidLedPosition;

void processLedSubCommand(BufferCntType inRXBufferNum) {
	gwUINT8 ccrHolder;
	gwUINT8 effect;
	gwUINT8 sampleCount;
	gwUINT8 sampleNum;
	LedDataStruct ledData;

	effect = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_EFFECT];
	sampleCount = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLE_COUNT];

	for (sampleNum = 0; sampleNum < sampleCount; ++sampleNum) {

		ledData.channel = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_CHANNEL];
		//memcpy(&ledData.position, (void *) &(gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES + (sampleNum * LED_SAMPLE_BYTES + 0)]), sizeof(ledData.position));
		ledData.position = (gwUINT16) gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES
		        + (sampleNum * LED_SAMPLE_BYTES + 0)] << 8;
		ledData.position += (gwUINT16) gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES
		        + (sampleNum * LED_SAMPLE_BYTES + 1)];
		ledData.red = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES + (sampleNum * LED_SAMPLE_BYTES + 2)];
		ledData.green = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES + (sampleNum * LED_SAMPLE_BYTES + 3)];
		ledData.blue = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_SAMPLES + (sampleNum * LED_SAMPLE_BYTES + 4)];
		ledData.sample1 = 0;
		ledData.sample2 = 0;
		ledData.sample3 = 0;
		ledData.sample4 = 0;

		switch (effect) {
			case eLedEffectSolid:
				if (ledData.position == ((gwUINT16) -1)) {
					gTotalLedSolidDataElements = 0;
					gCurLedSolidDataElement = 0;
					gNextSolidLedPosition = 0;
				} else {
					gLedSolidData[gTotalLedSolidDataElements] = ledData;
					gTotalLedSolidDataElements += 1;
				}
				break;

			case eLedEffectFlash:
				if (ledData.position == ((gwUINT16) -1)) {
					gTotalLedFlashDataElements = 0;
					gCurLedFlashDataElement = 0;
					gNextFlashLedPosition = 0;
				} else {
					gLedFlashData[gTotalLedFlashDataElements] = ledData;
					gTotalLedFlashDataElements += 1;
				}
				break;

			case eLedEffectError:
				break;

			case eLedEffectMotel:
				break;

			default:
				break;
		}
	}
}

// --------------------------------------------------------------------------

void processSetPosControllerSubCommand(BufferCntType inRXBufferNum) {
#ifdef RS485

	gwUINT8 instructionNum;
	gwUINT8 instructionCount;

	RS485_TX_ON

	instructionCount = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_INSTRUCTION_COUNT];
	for (instructionNum = 0; instructionNum < instructionCount; ++instructionNum) {
		gwUINT8 pos = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_INSTRUCTIONS
		+ (instructionNum * POS_INSTRUCTION_BYTES + CMDPOS_POS)];
		gwUINT8 reqQty = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_INSTRUCTIONS
		+ (instructionNum * POS_INSTRUCTION_BYTES + CMDPOS_REQ_QTY)];
		gwUINT8 minQty = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_INSTRUCTIONS
		+ (instructionNum * POS_INSTRUCTION_BYTES + CMDPOS_MIN_QTY)];
		gwUINT8 maxQty = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_INSTRUCTIONS
		+ (instructionNum * POS_INSTRUCTION_BYTES + CMDPOS_MAX_QTY)];
		gwUINT8 freq = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_INSTRUCTIONS
		+ (instructionNum * POS_INSTRUCTION_BYTES + CMDPOS_FREQ)];
		gwUINT8 dutyCycle = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_INSTRUCTIONS
		+ (instructionNum * POS_INSTRUCTION_BYTES + CMDPOS_DUTY_CYCLE)];

		gwUINT8 message[] = {POS_CTRL_DISPLAY, pos, reqQty, minQty, maxQty, freq, dutyCycle};
		serialTransmitFrame(Rs485_DEVICE, message, 7);

		//vTaskDelay(5);
	}
	//vTaskDelay(10);

	RS485_TX_OFF

#endif
}

void processClearPosControllerSubCommand(BufferCntType inRXBufferNum) {
#ifdef RS485
	gwUINT8 pos = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CLEAR_POS];

	RS485_TX_ON
	gwUINT8 message[] = {POS_CTRL_CLEAR, pos};
	serialTransmitFrame(Rs485_DEVICE, message, 2);

	//vTaskDelay(5);

	RS485_TX_OFF

#endif
}

void processDspAddrPosControllerSubCommand(BufferCntType inRXBufferNum) {
#ifdef RS485
	gwUINT8 pos = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_DSP_ADDR_POS];

	RS485_TX_ON
	gwUINT8 message[] = {POS_CTRL_DSP_ADDR, pos};
	serialTransmitFrame(Rs485_DEVICE, message, 2);

	//vTaskDelay(5);

	RS485_TX_OFF

#endif
}

void processPosControllerMassSetupStop(BufferCntType inRXBufferNum) {
#ifdef RS485
	gwUINT8 pos = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_DSP_ADDR_POS];

	RS485_TX_ON
	gwUINT8 message[] = {POS_CTRL_MASSSETUP_STOP, pos};
	serialTransmitFrame(Rs485_DEVICE, message, 2);

	//vTaskDelay(5);

	RS485_TX_OFF

#endif
}

void processPosControllerMassSetupStart() {
#ifdef RS485

	RS485_TX_ON
	gwUINT8 message[] = {POS_CTRL_MASSSETUP_START, 0x00};
	serialTransmitFrame(Rs485_DEVICE, message, 2);

	//vTaskDelay(5);

	RS485_TX_OFF

#endif
}

void processCreateScanSubCommand(BufferCntType inRxBufferNum) {
#ifdef CHE_CONTROLLER
	ScanStringType commandStr;
	ScanStringLenType commandStrLen = 0;
	
	BufferStoragePtrType bufferPtr = gRxRadioBuffer[inRxBufferNum].bufferStorage + CMDPOS_SCAN_COMMAND;
	commandStrLen = readAsPString(&commandStr, bufferPtr, MAX_DISPLAY_STRING_BYTES);
	
	if (commandStrLen > 0) {
		BufferCntType txBufferNum = lockTxBuffer();
		createScanCommand(txBufferNum, &commandStr, commandStrLen);
		if (transmitPacket(txBufferNum)) {
			while (gRadioState == eTx) {
				vTaskDelay(1);
			}
		}
	}
#endif
}

void processCreateButtonSubCommand(BufferCntType inRxBufferNum) {
#ifdef RS485
	
	gwUINT8 pos = gRxRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_CREATE_BUTTON_POS];
	gwUINT8 num = gRxRadioBuffer[inRxBufferNum].bufferStorage[CMDPOS_CREATE_BUTTON_NUM];
	
	RS485_TX_ON
	gwUINT8 message[] = {POS_CTRL_CREAT_BUTTON, pos, num};
	serialTransmitFrame(Rs485_DEVICE, message, 3);

	vTaskDelay(5);

	RS485_TX_OFF

	
#endif
}

// -------------------------------------------------------------------------

void processPosconLedBroadcastSubCommand(BufferCntType inRXBufferNum) {
#ifdef RS485
	gwUINT8 currByteCount = 0;
	gwUINT8 i = 0;
	gwUINT8 mask = 1;
	gwUINT8 pos = 0;

	gwUINT8 redValue = 0;
	gwUINT8 greenValue = 0;
	gwUINT8 blueValue = 0;

	gwUINT8 lightStyle = 0;
	gwUINT8 byteCount = 0;

	RS485_TX_ON

	redValue = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_RED_VALUE];
	greenValue = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_GREEN_VALUE];
	blueValue = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_BLUE_VALUE];
	lightStyle = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LIGHT_STYLE];
	byteCount = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_COUNT];

	for (currByteCount = 0; currByteCount < byteCount; ++currByteCount) {
		gwUINT8 currByte = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_LED_MAP_START + currByteCount];

		pos = (currByteCount * 8) + 1;
		mask = 1;

		for (i = 0; i < 8; i++, pos++) {
			if ((currByte & mask) != 0) {
				gwUINT8 message[] = {POS_CTRL_LED, pos, redValue, greenValue, blueValue, lightStyle};
				serialTransmitFrame(Rs485_DEVICE, message, 6);
			}

			mask <<= 1;
		}
	}

	RS485_TX_OFF

#endif
}
