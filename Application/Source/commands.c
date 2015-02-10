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

#ifdef SHARP_DISPLAY
#include "display.h"
#endif

#ifdef RS485
#include "Rs485.h"
#endif

RemoteDescStruct			gRemoteStateTable[MAX_REMOTES];
NetAddrType					gMyAddr = INVALID_REMOTE;
NetworkIDType				gMyNetworkID = BROADCAST_NET_NUM;
portTickType				gSleepWaitMillis;
BufferStorageType			gLastTxAckId = 1;

extern RxRadioBufferStruct	gRxRadioBuffer[RX_BUFFER_COUNT];
extern TxRadioBufferStruct	gTxRadioBuffer[TX_BUFFER_COUNT];
extern xQueueHandle 		gRadioTransmitQueue;
extern xQueueHandle 		gRadioReceiveQueue;
extern ControllerStateType 	gControllerState;
extern ELocalStatusType		gLocalDeviceState;


// --------------------------------------------------------------------------
// Local function prototypes

void createPacket(BufferCntType inTXBufferNum, ECommandGroupIDType inCmdID, NetworkIDType inNetworkID, NetAddrType inSrcAddr,
		NetAddrType inDestAddr);

// --------------------------------------------------------------------------

gwUINT8 transmitPacket(BufferCntType inTXBufferNum) {
	gwUINT8 result = 0;
	BufferCntType txBufferNum = inTXBufferNum;

	// Transmit the packet.
	result = xQueueGenericSend(gRadioTransmitQueue, &txBufferNum, (portTickType) 0, (portBASE_TYPE) queueSEND_TO_BACK);

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

NetworkIDType getNetworkID(BufferCntType inRXBufferNum) {
	return ((gRxRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_NET_NUM] & PACKETMASK_NET_NUM) >> SHIFTBITS_PKT_NET_NUM);
}

// --------------------------------------------------------------------------

gwBoolean getCommandRequiresACK(BufferCntType inRXBufferNum) {
	return (getAckId != 0);
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

	// The source address is in the first half-byte of the packet.
	NetAddrType result = gRxRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_SRC_ADDR];
	return result;
}

// --------------------------------------------------------------------------

NetAddrType getCommandDstAddr(BufferCntType inRXBufferNum) {

	// The source address is in the first half-byte of the packet.
	NetAddrType result = gRxRadioBuffer[inRXBufferNum].bufferStorage[PCKPOS_DST_ADDR];
	return result;
}

// --------------------------------------------------------------------------

ENetMgmtSubCmdIDType getNetMgmtSubCommand(BufferStoragePtrType inBufferPtr) {
	ENetMgmtSubCmdIDType result = (inBufferPtr[CMDPOS_NETM_SUBCMD]);
	return result;
}

// --------------------------------------------------------------------------

ECmdAssocType getAssocSubCommand(BufferCntType inRXBufferNum) {
	ECmdAssocType result = eCmdAssocInvalid;
	// Make sure the command is actually for us.
	if (memcmp(STR(GUID), &(gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_GUID]), UNIQUE_ID_BYTES) == 0) {
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
	memset((void *) gTxRadioBuffer[inTXBufferNum].bufferStorage, 0, CMDPOS_CMD_ID);

	// Write the packet header.
	// (See the comments at the top of commandTypes.h.)
	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_VERSION] |= (PACKET_VERSION << SHIFTBITS_PKT_VER);
	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_NET_NUM] |= (inNetworkID << SHIFTBITS_PKT_NET_NUM);
	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_SRC_ADDR] = inSrcAddr;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_DST_ADDR] = inDestAddr;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_ACK_ID] = 0;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_CMD_ID] = (inCmdID << SHIFTBITS_CMD_ID);

	gTxRadioBuffer[inTXBufferNum].bufferSize += 4;
}

// --------------------------------------------------------------------------

void createAssocReqCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	// The remote doesn't have an assigned address yet, so we send the broadcast addr as the source.
	createPacket(inTXBufferNum, eCommandAssoc, BROADCAST_NET_NUM, ADDR_CONTROLLER, ADDR_BROADCAST);

	// Set the AssocReq sub-command
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD] = eCmdAssocREQ;

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_GUID]), inUniqueID, UNIQUE_ID_BYTES);

	// Set the device version
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_DEV_VER] = 0x01;
	// Set the system status register
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_SYSSTAT] = GW_GET_SYSTEM_STATUS;

	gTxRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ASSOCREQ_SYSSTAT;
}

// --------------------------------------------------------------------------

void createAssocCheckCommand(BufferCntType inTXBufferNum, RemoteUniqueIDPtrType inUniqueID) {

	gwUINT8 batteryLevel;

	// Create the command for checking if we're associated
	createPacket(inTXBufferNum, eCommandAssoc, gMyNetworkID, gMyAddr, ADDR_BROADCAST);

	// Set the AssocReq sub-command
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_SUBCMD] = eCmdAssocCHECK;

	// The next 8 bytes are the unique ID of the device.
	memcpy((void *) &(gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOC_GUID]), inUniqueID, UNIQUE_ID_BYTES);

	// Set the device version
	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCREQ_DEV_VER] = 0x01;

	// Nominalize to a 0-100 scale.
	if (batteryLevel < 0) {
		batteryLevel = 0;
	} else {
		// Double to get to a 100 scale.
		batteryLevel = batteryLevel << 1;
	}

	gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_ASSOCCHK_BATT] = batteryLevel;
	gTxRadioBuffer[inTXBufferNum].bufferSize = CMDPOS_ASSOCCHK_BATT + 1;
}
// --------------------------------------------------------------------------

void createAckPacket(BufferCntType inTXBufferNum, AckIDType inAckId, AckDataType inAckData) {

	createPacket(inTXBufferNum, eCommandNetMgmt, gMyNetworkID, gMyAddr, ADDR_CONTROLLER);
	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_PCK_TYPE_BIT] |= 1 << SHIFTBITS_PCK_TYPE;
	gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_ACK_ID] = inAckId;
	memcpy((void *) &(gTxRadioBuffer[inTXBufferNum].bufferStorage[PCKPOS_ACK_DATA]), inAckData, ACK_DATA_BYTES);
	gTxRadioBuffer[inTXBufferNum].bufferSize = PCKPOS_ACK_DATA + ACK_DATA_BYTES;
}

// --------------------------------------------------------------------------

#ifdef IS_GATEWAY
void createOutboundNetSetup() {
	BufferCntType txBufferNum;
	gwUINT8 ccrHolder;

	txBufferNum = lockTXBuffer();

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

	serialTransmitFrame(UART_1, (gwUINT8*) (&gTxRadioBuffer[txBufferNum].bufferStorage), gTxRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

	vTaskResume(gRadioReceiveTask);

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

	// Write this value to MV_RAM if it's different than what we already have.
	// NB: Writing to flash causes it to wear out over time.  FSL says 10K write cycles, but there
	// are other issues that may reduce this number.  So only write if we have to.
	//	if (NV_RAM_ptr->ChannelSelect != channel) {
	//		// (cast away the "const" of the NVRAM channel number.)
	//		EnterCritical();
	//		Update_NV_RAM(&(NV_RAM_ptr->ChannelSelect), &channel, 1);
	//		//WriteFlashByte(channel, &(NV_RAM_ptr->ChannelSelect));
	//		ExitCritical();
	//	}

	suspendRadioRx();
	MLMESetChannelRequest(channel);
	resumeRadioRx();

	gLocalDeviceState = eLocalStateRun;
}
// --------------------------------------------------------------------------

#ifdef IS_GATEWAY
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

	txBufferNum = lockTXBuffer();

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

	serialTransmitFrame(UART_1, (gwUINT8*) (&gTxRadioBuffer[txBufferNum].bufferStorage), gTxRadioBuffer[txBufferNum].bufferSize);
	RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

	vTaskResume(gRadioReceiveTask);

}

// --------------------------------------------------------------------------

void processNetCheckOutboundCommand(BufferCntType inTXBufferNum) {
	BufferCntType txBufferNum;
	ChannelNumberType channel;
	ChannelNumberType newChannel;

	//vTaskSuspend(gRadioReceiveTask);

	// Switch to the channel requested in the outbound net-check.
	channel = MLMEGetChannelRequest();
	newChannel = gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_CHANNEL];

	if (channel != newChannel) {
		MLMESetChannelRequest(newChannel);
		channel = newChannel;
	}

	// We need to put the gateway (dongle) GUID into the outbound packet before it gets transmitted.
	memcpy(&(gTxRadioBuffer[inTXBufferNum].bufferStorage[CMDPOS_NETM_CHKCMD_GUID]), STR(GUID), UNIQUE_ID_BYTES);

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

		txBufferNum = lockTXBuffer();

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

		serialTransmitFrame(UART_1, (gwUINT8*) (&gTxRadioBuffer[txBufferNum].bufferStorage), gTxRadioBuffer[txBufferNum].bufferSize);
		RELEASE_TX_BUFFER(txBufferNum, ccrHolder);

		vTaskResume(gRadioReceiveTask);

	}
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

	// Let's first make sure that this assign command is for us.
	if (memcmp(STR(GUID), &(gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOC_GUID]), UNIQUE_ID_BYTES) == 0) {
		// The destination address is the third half-byte of the command.
		gMyAddr = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_ADDR];
		gMyNetworkID = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_NET];
		gSleepWaitMillis = (gwUINT16) gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_SLEEP + 0] << 8;
		gSleepWaitMillis += (gwUINT16) gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCRESP_SLEEP + 1];
		gSleepWaitMillis *= 1000;
		gLocalDeviceState = eLocalStateAssociated;

		BufferCntType txBufferNum = 0;//lockTxBuffer();
		createAssocCheckCommand(txBufferNum, (RemoteUniqueIDPtrType) STR(GUID));
		if (transmitPacket(txBufferNum)) {
		}

	}
}
// --------------------------------------------------------------------------

void processAssocAckCommand(BufferCntType inRXBufferNum) {
	// No can do in FreeRTOS :-(
	//xTaskSetTickCount(gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_ASSOCACK_TIME)]);
}


// --------------------------------------------------------------------------

DisplayStringType gDisplayDataLine[4];
DisplayStringLenType gDisplayDataLineLen[4];
DisplayStringLenType gDisplayDataLinePos[4];

EControlCmdAckStateType processDisplayMsgSubCommand(BufferCntType inRXBufferNum) {
	EControlCmdAckStateType result = eAckStateOk;

#ifdef CHE_CONTROLLER
	clearDisplay();

	// First display line.
	BufferStoragePtrType bufferPtr = gRxRadioBuffer[inRXBufferNum].bufferStorage + CMDPOS_MESSAGE;
	gDisplayDataLineLen[0] = readAsPString(gDisplayDataLine[0], bufferPtr, MAX_DISPLAY_STRING_BYTES);

	displayMessage(1, gDisplayDataLine[0], getMin(MAX_DISPLAY_CHARS, gDisplayDataLineLen[0]));

	// Second display line.
	bufferPtr += gDisplayDataLineLen[0] + 1;
	gDisplayDataLineLen[1] = readAsPString(gDisplayDataLine[1], bufferPtr, MAX_DISPLAY_STRING_BYTES);

	displayMessage(2, gDisplayDataLine[1], getMin(MAX_DISPLAY_CHARS, gDisplayDataLineLen[1]));

	// Third display line.
	bufferPtr += gDisplayDataLineLen[1] + 1;
	gDisplayDataLineLen[2] = readAsPString(gDisplayDataLine[2], bufferPtr, MAX_DISPLAY_STRING_BYTES);

	displayMessage(3, gDisplayDataLine[2], getMin(MAX_DISPLAY_CHARS, gDisplayDataLineLen[2]));

	// Fourth display line.
	bufferPtr += gDisplayDataLineLen[2] + 1;
	gDisplayDataLineLen[3] = readAsPString(gDisplayDataLine[3], bufferPtr, MAX_DISPLAY_STRING_BYTES);

	displayMessage(4, gDisplayDataLine[3], getMin(MAX_DISPLAY_CHARS, gDisplayDataLineLen[3]));

#endif

	return result;
}

// --------------------------------------------------------------------------

LedPositionType gTotalLedPositions;

LedDataStruct gLedFlashData[MAX_LED_FLASH_POSITIONS];
LedPositionType gCurLedFlashDataElement;
LedPositionType gTotalLedFlashDataElements;
LedPositionType gNextFlashLedPosition;

LedDataStruct gLedSolidData[MAX_LED_SOLID_POSITIONS];
LedPositionType gCurLedSolidDataElement;
LedPositionType gTotalLedSolidDataElements;
LedPositionType gNextSolidLedPosition;

EControlCmdAckStateType processLedSubCommand(BufferCntType inRXBufferNum) {

	EControlCmdAckStateType result = eAckStateOk;
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
	return result;
}

// --------------------------------------------------------------------------

EControlCmdAckStateType processSetPosControllerSubCommand(BufferCntType inRXBufferNum) {

	EControlCmdAckStateType result = eAckStateOk;
	
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

		vTaskDelay(5);
	}
	vTaskDelay(10);

	RS485_TX_OFF

#endif
	return result;
}

EControlCmdAckStateType processClearPosControllerSubCommand(BufferCntType inRXBufferNum) {
	EControlCmdAckStateType result = eAckStateOk;

#ifdef RS485
	gwUINT8 pos = gRxRadioBuffer[inRXBufferNum].bufferStorage[CMDPOS_CLEAR_POS];

	RS485_TX_ON
	gwUINT8 message[] = {POS_CTRL_CLEAR, pos};
	serialTransmitFrame(Rs485_DEVICE, message, 2);

	vTaskDelay(5);

	RS485_TX_OFF

#endif

	return result;
}
