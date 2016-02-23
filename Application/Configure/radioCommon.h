/*
	FlyWeight
	� Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#ifndef RADIOCOMMON_H
#define RADIOCOMMON_H

// Project includes
#include "smacGlue.h"
#include "commandTypes.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "SMAC_Interface.h"
#include "StatusLedClk.h"
#include "StatusLedSdi.h"


// --------------------------------------------------------------------------
// Definitions.

// Priorities assigned to demo application tasks.
#define LED_BLINK_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define MGMT_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define SERIAL_RECV_PRIORITY	( tskIDLE_PRIORITY + 2 )
#define RADIO_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define KEYBOARD_PRIORITY		( tskIDLE_PRIORITY + 2 )

#define LQI_BYTES				1

#define MAX_PACKET_SIZE			123
#define ACK_DATA_BYTES			8

#define GATEWAY_MGMT_QUEUE_SIZE	10
#define REMOTE_MGMT_QUEUE_SIZE	10
#define TX_ACK_QUEUE_SIZE		1

#define RX_QUEUE_SIZE			10
#define RX_BUFFER_COUNT			10
#define RX_BUFFER_SIZE			MAX_PACKET_SIZE

#define TX_QUEUE_SIZE			10
#define TX_BUFFER_COUNT			10
#define TX_BUFFER_SIZE			MAX_PACKET_SIZE

#define INVALID_RX_BUFFER		99
#define INVALID_TX_BUFFER		99

#define KEYBOARD_QUEUE_SIZE		2

#define MAX_DISPLAY_CHARS				30

#define MAX_DISPLAY_STRING_BYTES		42
#define MAX_SCAN_STRING_BYTES			64

#define POS_CTRL_INIT					0x00
#define POS_CTRL_CLEAR					0x01
#define POS_CTRL_DISPLAY				0x02
#define POS_CTRL_MASSSETUP_START		0x06
#define POS_CTRL_CREAT_BUTTON			0x08
#define POS_CTRL_DSP_ADDR				0x09
#define POS_CTRL_MASSSETUP_STOP			0x0a
#define POS_CTRL_LED					0x0b

#define POS_CTRL_ALL_POSITIONS			0x00

#define TOTAL_ERROR_SETS				20
#define DISTANCE_BETWEEN_ERROR_LEDS		16
	
#define MAX_LED_FLASH_POSITIONS 		TOTAL_ERROR_SETS * 2
#define MAX_LED_SOLID_POSITIONS 		0

#define	SCROLL_TIMER					gTmr1_c
#define SCROLL_PRIMARY_SOURCE			gTmrPrimaryClkDiv128_c
#define SCROLL_SECONDARY_SOURCE			gTmrSecondaryCnt0Input_c
#define SCROLL_CLK_RATE					0xffff

#define TOTAL_NUM_CHANNELS				14
#define CON_ATTEMPTS_BEFORE_BACKOFF		3 * TOTAL_NUM_CHANNELS	// This should be a multiple of TOTAL_NUM_CHANNELS
#define SLOW_CON_ATTEMPTS				3 * TOTAL_NUM_CHANNELS	// This should be a multiple of TOTAL_NUM_CHANNELS
#define SLOW_CON_SLEEP_MS				50
#define RAND_BACK_OFF_LIMIT				100

#define RELEASE_RX_BUFFER(rxBufferNum, ccrHolder)	\
	GW_ENTER_CRITICAL(ccrHolder); \
	if (gRxRadioBuffer[rxBufferNum].bufferStatus != eBufferStateFree) { \
		gRxRadioBuffer[rxBufferNum].bufferStatus = eBufferStateFree; \
	} else if (GW_DEBUG) { \
		debugRefreed(rxBufferNum); \
	} \
	gRxUsedBuffers--; \
	GW_EXIT_CRITICAL(ccrHolder);

#define RELEASE_TX_BUFFER(txBufferNum, ccrHolder)	\
	GW_ENTER_CRITICAL(ccrHolder); \
	if (gTxRadioBuffer[txBufferNum].bufferStatus != eBufferStateFree) { \
		gTxRadioBuffer[txBufferNum].bufferStatus = eBufferStateFree; \
	} else if (GW_DEBUG) { \
		debugRefreed(txBufferNum); \
	} \
	gTxUsedBuffers--; \
	GW_EXIT_CRITICAL(ccrHolder);

// --------------------------------------------------------------------------
// Typedefs

typedef enum {
	eIdle,
	eRx,
	eTx,
} RadioStateEnum;

typedef gwUINT8				BufferCntType;
typedef gwUINT8				BufferOffsetType;
typedef gwUINT8				BufferStorageType;
typedef BufferStorageType	*BufferStoragePtrType;

typedef enum {
	eBufferStateFree,
	eBufferStateInUse,
} EBufferStatusType;

typedef struct {
	// Grrr... SMAC now requires static references.  
	// A pointer to a RxRadioBuffer smacpdu will actually be a pointing what "looks" like an rxPacket_t with a smacPdu_t who's u8Data starts in bufferStorage.
	// Our application only wants what's in the buffer and none of the other SMAC stuff.
	uint8_t					rxPacket[sizeof(rxPacket_t) - 1];
	BufferStorageType		bufferStorage[MAX_PACKET_SIZE];
	EBufferStatusType		bufferStatus;
	BufferCntType			bufferSize;
} RxRadioBufferStruct;

typedef struct {
	// Grrr... SMAC now requires static references.  
	// A pointer to a XxRadioBuffer smacpdu will actually be a pointing what "looks" like an txPacket_t with a smacPdu_t who's u8Data starts in bufferStorage.
	// Our application only wants what's in the buffer and none of the other SMAC stuff.
	uint8_t					txPacket[sizeof(txPacket_t) - 1];
	BufferStorageType		bufferStorage[MAX_PACKET_SIZE];
	EBufferStatusType		bufferStatus;
	BufferCntType			bufferSize;
} TxRadioBufferStruct;

typedef struct {
	rxPacket_t		*rxPacketPtr;
	BufferCntType	bufferNum;
} ERxMessageHolderType;

typedef struct {
	txPacket_t		*txPacketPtr;
	txStatus_t		txStatus;
	BufferCntType	bufferNum;
} ETxMessageHolderType;

typedef gwUINT8				PacketVerType;
typedef gwUINT8				NetworkIDType;
typedef gwUINT8				AckIDType;
typedef gwUINT8				AckDataType[ACK_DATA_BYTES];
typedef gwUINT8				AckLQIType;
typedef gwUINT16			NetAddrType;
typedef gwUINT8				EndpointNumType;
typedef gwUINT8				KVPNumType;

typedef gwUINT8				RemoteUniqueIDType[UNIQUE_ID_BYTES + 1];
//typedef RemoteUniqueIDType	*RemoteUniqueIDPtrType;

typedef struct {
	ERemoteStatusType		remoteState;
	RemoteUniqueIDType		remoteUniqueID;
} RemoteDescStruct;

typedef gwUINT16			SampleRateType;
typedef gwUINT8				SampleSizeType;

typedef union UnixTimeUnionType {
	struct{
		gwUINT8 byte1;
		gwUINT8 byte2;
		gwUINT8 byte3;
		gwUINT8 byte4;
	} byteFields;
	gwTime value;
} UnixTimeType;

// From RadioManagement.c
#define MAX_NUM_MSG	4

typedef gwUINT32			TimestampType;
typedef gwUINT32			DataSampleType;

// Aisle Controller types.
typedef enum {
	eLedCycleOff,
	eLedCycleOn
} LedCycleType;

typedef LedCycleType 		LedCycle;
typedef gwUINT8 			LedChannelType;
typedef gwUINT16 			LedPositionType;
typedef gwUINT8 			LedData;

typedef struct {
		LedChannelType		channel;
		LedPositionType 	position;
		LedData				red;
		LedData				green;
		LedData				blue;
		gwUINT32			sample1;
		gwUINT32			sample2;
		gwUINT32			sample3;
		gwUINT32			sample4;
} LedDataStruct;

typedef gwCHAR				DisplayCharType;
typedef DisplayCharType		DisplayStringType[MAX_DISPLAY_STRING_BYTES];
typedef DisplayStringType*	DisplayStringPtrType;
typedef gwUINT8				DisplayStringLenType;

typedef gwUINT8				ScanCharType;
typedef ScanCharType		ScanStringType[MAX_SCAN_STRING_BYTES];
typedef ScanStringType*		ScanStringPtrType;
typedef gwUINT8				ScanStringLenType;
typedef gwUINT8 ChannelNumberType;


// --------------------------------------------------------------------------
// Function prototypes

void advanceRxBuffer(void);
BufferCntType lockRxBuffer(void);
BufferCntType lockTxBuffer(void);
void setupWatchdog(void);

void setRadioChannel(ChannelNumberType channel);
void readRadioRx();
void writeRadioTx(BufferCntType inTxBufferNum);
gwBoolean preProcessPacket(int inBufferNum);

void debugReset();
void debugRefreed(BufferCntType inBufferNum);
void debugCrmCallback(void);
void setStatusLed(uint8_t red, uint8_t green, uint8_t blue);

#endif /* RADIOCOMMON_H */
