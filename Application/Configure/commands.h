/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved
 
 $Id$
 $Name$	
 */

#ifndef COMMANDS_H
#define COMMANDS_H

// Project includes
#include "gwTypes.h"
#include "commandTypes.h"
#include "radioCommon.h"
#ifdef RS485
#include "Rs485TxCtl.h"
#endif

// --------------------------------------------------------------------------
// Definitions.

#define getMax(a,b)    (((a) > (b)) ? (a) : (b))
#define getMin(a,b)    (((a) < (b)) ? (a) : (b))

#define DEVICE_CONTROLLER			0
#define DEVICE_GATEWAY				1
#define DEVICE_REMOTE				2

#define LED_SAMPLE_BYTES			5
#define POS_INSTRUCTION_BYTES		6

#ifdef RS485
#define RS485_TX_ON					Rs485TxCtl_SetVal(Rs485TxCtl_DeviceData);
#define RS485_TX_OFF				Rs485TxCtl_ClrVal(Rs485TxCtl_DeviceData);
#endif

// --------------------------------------------------------------------------
// Typedefs.

typedef enum {
	eMotorCommandInvalid = -1,
	eMotorCommandFreewheel = 0,
	eMotorCommandFwd = 1,
	eMotorCommandBwd = 2,
	eMotorCommandBrake = 3
} EMotorCommandType;

typedef enum {
	eHooBeeBehaviorInvalid = -1,
	eHooBeeBehaviorLedFlash = 1,
	eHooBeeBehaviorSolenoidPush = 2,
	eHooBeeBehaviorSolenoidPull = 3
} EHooBeeBehaviorType;

// --------------------------------------------------------------------------
// Function prototypes.

/*
 * Note: commands ALWAYS get created inside one of the fixed RX or TX buffers.
 * The reason is that we can't allocate memory, and we don't want to copy/move memory either.
 * For this reason all of the command processing function accept or produce a reference
 * to the RX or TX buffer that contains the command.
 */

gwUINT8 transmitPacket(BufferCntType inTXBufferNum);
gwUINT8 transmitPacketFromISR(BufferCntType inTXBufferNum);

gwBoolean getAckRequired(BufferStoragePtrType inBufferPtr);
ECommandGroupIDType getCommandID(BufferStoragePtrType inBufferPtr);
NetworkIDType getNetworkID(BufferCntType inRXBufferNum);
AckIDType getAckId(BufferStoragePtrType inBufferPtr);
void setAckId(BufferStoragePtrType inBufferPtr);

ENetMgmtSubCmdIDType getNetMgmtSubCommand(BufferStoragePtrType inBufferPtr);
ECmdAssocType getAssocSubCommand(BufferCntType inRXBufferNum);
EControlSubCmdIDType getControlSubCommand(BufferCntType inRXBufferNum);

EndpointNumType getEndpointNumber(BufferCntType inRXBufferNum);
NetAddrType getCommandSrcAddr(BufferCntType inRXBufferNum);
NetAddrType getCommandDstAddr(BufferCntType inRXBufferNum);

gwUINT8 getLEDVaue(gwUINT8 inLEDNum, BufferCntType inRXBufferNum);
void writeAsPString(BufferStoragePtrType inDestPtr, const BufferStoragePtrType inStringPtr, gwUINT8 inStringLen);
gwUINT8 readAsPString(BufferStoragePtrType inDestStringPtr, const BufferStoragePtrType inSrcPtr, gwUINT8 inMaxBytes);

void createNetCheckRespInboundCommand(BufferCntType inRXBufferNum);
void createAckPacket(BufferCntType inTXBufferNum, AckIDType inAckId, AckDataType inAckData);
void createAssocReqCommand(BufferCntType inTXBufferNum);
void createAssocCheckCommand(BufferCntType inTXBufferNum);
void createButtonControlCommand(BufferCntType inTXBufferNum, gwUINT8 inButtonNumber, gwUINT8 inFunctionType);
void createQueryCommand(BufferCntType inTXBufferNum, NetAddrType inRemoteAddr);
//void createAudioCommand(BufferCntType inTXBufferNum);
void createResponseCommand(BufferCntType inTXBufferNum, BufferOffsetType inResponseSize, NetAddrType inRemoteAddr);
void createScanCommand(BufferCntType inTXBufferNum, ScanStringPtrType inScanStringPtr, ScanStringLenType inScanStringLen);
void createButtonCommand(BufferCntType inTXBufferNum, gwUINT8 inButtonNum, gwUINT8 inValue);
#ifdef GATEWAY
void createOutboundNetSetup(void);
#endif

void processNetCheckInboundCommand(BufferCntType inRXBufferNum);
#ifdef GATEWAY
void processNetCheckOutboundCommand(BufferCntType inTXBufferNum);
void processNetIntfTestCommand(BufferCntType inTXBufferNum);
#endif
void processNetSetupCommand(BufferCntType inRXBufferNum);
void processAssocRespCommand(BufferCntType inRXBufferNum);
void processQueryCommand(BufferCntType inRXBufferNum, NetAddrType inRemoteAddr);
void processResponseCommand(BufferCntType inRXBufferNum, NetAddrType inRemoteAddr);

void processDisplayMsgSubCommand(BufferCntType inRXBufferNum);
void processSetPosControllerSubCommand(BufferCntType inRXBufferNum);
void processClearPosControllerSubCommand(BufferCntType inRXBufferNum);
void processLedSubCommand(BufferCntType inRXBufferNum);

void startScrolling();
void stopScrolling();

void createDataSampleCommand(BufferCntType inTXBufferNum, EndpointNumType inEndpoint);
void addDataSampleToCommand(BufferCntType inTXBufferNum, TimestampType inTimestamp, DataSampleType inDataSample, char inUnitsByte);

// --------------------------------------------------------------------------
// Globals.

#endif /* COMMANDS_H */
