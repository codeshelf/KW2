/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#ifndef COMMANDSTYPES_H
#define COMMANDSTYPES_H

// Project includes
#include "gwTypes.h"

// --------------------------------------------------------------------------
// Definitions.

#define UNIQUE_ID_BYTES			8

#define PRIVATE_GUID			"00000000"

/*
 * The format of a packet on the network is as follows:
 *
 * 2b - Version
 * 1b - Ack requested bit
 * 1b - Reserved
 * 4b - Network number
 * 1B - Packet source address
 * 1B - Packet dest address
 * 1B - Ack Id
 * 4b - Command ID
 * 4b - Command endpoint
 * nB - Command bytes
 *
 * Where B = byte, b = bit
 *
 */

// Command format positioning constants.
// Packet
#define PCKPOS_VERSION			0
#define PCKPOS_PCK_TYPE_BIT		PCKPOS_VERSION
#define PCKPOS_RESERVED			PCKPOS_PCK_TYPE_BIT
#define PCKPOS_NET_NUM			PCKPOS_RESERVED
#define PCKPOS_SRC_ADDR			PCKPOS_NET_NUM + 1
#define PCKPOS_DST_ADDR 		PCKPOS_SRC_ADDR + 1
#define PCKPOS_ACK_ID			PCKPOS_DST_ADDR + 1

//#define PCKPOS_ACK_DATA			PCKPOS_ACK_ID + 1

#define CMDPOS_CMD_ID			PCKPOS_ACK_ID + 1
#define CMDPOS_ENDPOINT			CMDPOS_CMD_ID


#define CMDPOS_STARTOFCMD		CMDPOS_ENDPOINT + 1
#define PCKPOS_ACK_DATA			CMDPOS_ENDPOINT + 1

// Network Mgmt
#define CMDPOS_NETM_SUBCMD			CMDPOS_STARTOFCMD
#define CMDPOS_NETM_CHKCMD_TYPE		CMDPOS_NETM_SUBCMD + 1
#define CMDPOS_NETM_CHKCMD_NET_NUM	CMDPOS_NETM_CHKCMD_TYPE + 1
#define CMDPOS_NETM_CHKCMD_GUID		CMDPOS_NETM_CHKCMD_NET_NUM + 1
#define CMDPOS_NETM_CHKCMD_CHANNEL	CMDPOS_NETM_CHKCMD_GUID + 8
#define CMDPOS_NETM_CHKCMD_ENERGY	CMDPOS_NETM_CHKCMD_CHANNEL + 1
#define CMDPOS_NETM_CHKCMD_LINKQ	CMDPOS_NETM_CHKCMD_ENERGY + 1
#define CMDPOS_NETM_TSTCMD_NUM		CMDPOS_NETM_SUBCMD + 1
#define CMDPOS_NETM_ACKCMD_ID		CMDPOS_NETM_SUBCMD + 1
#define CMDPOS_NETM_SETCMD_CHANNEL	CMDPOS_NETM_SUBCMD + 1

// Assoc Command
#define CMDPOS_ASSOC_SUBCMD		CMDPOS_STARTOFCMD
#define CMDPOS_ASSOC_GUID		CMDPOS_ASSOC_SUBCMD + 1
#define CMDPOS_ASSOCREQ_HW_VER	CMDPOS_ASSOC_GUID + 8
#define CMDPOS_ASSOCREQ_FW_VER	CMDPOS_ASSOCREQ_HW_VER + 4
#define CMDPOS_ASSOCREQ_RP_VER	CMDPOS_ASSOCREQ_FW_VER + 4
#define CMDPOS_ASSOCREQ_SYSSTAT CMDPOS_ASSOCREQ_RP_VER + 1

#define CMDPOS_ASSOCRESP_ADDR	CMDPOS_ASSOC_GUID + 8
#define CMDPOS_ASSOCRESP_NET	CMDPOS_ASSOCRESP_ADDR + 1
#define CMDPOS_ASSOCRESP_SLEEP	CMDPOS_ASSOCRESP_NET + 1
#define CMDPOS_ASSOCACK_STATE	CMDPOS_ASSOC_GUID + 8
#define CMDPOS_ASSOCACK_TIME	CMDPOS_ASSOCACK_STATE + 1
#define CMDPOS_ASSOCCHK_BATT	CMDPOS_ASSOC_GUID + 8
#define CMDPOS_ASSOCCHK_LRC		CMDPOS_ASSOCCHK_BATT + 1
#define CMDPOS_ASSOCCHK_RD		CMDPOS_ASSOCCHK_LRC + 1

// Info Command
#define CMDPOS_INFO_SUBCMD		CMDPOS_STARTOFCMD
#define CMDPOS_INFO_QUERY		CMDPOS_INFO_SUBCMD + 1
#define CMDPOS_INFO_RESPONSE	CMDPOS_INFO_QUERY + 0

// Control Command
#define CMDPOS_CONTROL			CMDPOS_STARTOFCMD
#define CMDPOS_CONTROL_SUBCMD	CMDPOS_CONTROL + 0
#define CMDPOS_CONTROL_DATA		CMDPOS_CONTROL_SUBCMD + 1

// Message Command
#define CMDPOS_MESSAGE				CMDPOS_CONTROL_DATA

// Ack Command
#define CMDPOS_ACK_NUM				CMDPOS_CONTROL_DATA
#define CMDPOS_ACK_LQI				CMDPOS_ACK_NUM + 1

// Single line message command
#define CMDPOS_MESSAGE_FONT			CMDPOS_CONTROL_DATA
#define CMDPOS_MESSAGE_POSX			CMDPOS_MESSAGE_FONT + 1
#define CMDPOS_MESSAGE_POSY			CMDPOS_MESSAGE_POSX + 2
#define CMDPOS_MESSAGE_SINGLE		CMDPOS_MESSAGE_POSY + 2

// Button Command
#define CMDPOS_BUTTON_NUM			CMDPOS_CONTROL_DATA
#define CMDPOS_BUTTON_VAL			CMDPOS_BUTTON_NUM + 1

// LED Command
#define CMDPOS_LED_CHANNEL			CMDPOS_CONTROL_DATA
#define CMDPOS_LED_EFFECT			CMDPOS_LED_CHANNEL + 1
#define CMDPOS_LED_SAMPLE_COUNT		CMDPOS_LED_EFFECT + 1
#define CMDPOS_LED_SAMPLES			CMDPOS_LED_SAMPLE_COUNT + 1

// Position controller set command
#define CMDPOS_INSTRUCTION_COUNT	CMDPOS_CONTROL_DATA
#define CMDPOS_INSTRUCTIONS			CMDPOS_INSTRUCTION_COUNT + 1
#define CMDPOS_POS	 				0
#define CMDPOS_REQ_QTY 				1
#define CMDPOS_MIN_QTY 				2
#define CMDPOS_MAX_QTY 				3
#define CMDPOS_FREQ 				4
#define CMDPOS_DUTY_CYCLE 			5

// Position controller clear command
#define CMDPOS_CLEAR_POS			CMDPOS_CONTROL_DATA

// Command masks
#define PACKETMASK_VERSION		0xc0    /* 0b11000000 */
#define PACKETMASK_ACK_REQ		0x20	/* 0b00100000 */
#define PACKETMASK_RSV_HDR		0x10	/* 0b00010000 */
#define PACKETMASK_NET_NUM		0x0f    /* 0b00001111 */
#define CMDMASK_CMDID			0xf0    /* 0b11110000 */
#define CMDMASK_ENDPOINT		0x0f    /* 0b00001111 */

#define SHIFTBITS_PKT_VER		6
#define SHIFTBITS_PCK_TYPE		5
#define SHIFTBITS_RSV_HDR		4
#define SHIFTBITS_PKT_NET_NUM	0
#define SHIFTBITS_CMD_ID		4
#define SHIFTBITS_CMD_ENDPOINT	0

#define PACKET_VERSION			0x00

#define BROADCAST_NET_NUM		0x0f
#define ADDR_CONTROLLER			0x00
#define ADDR_BROADCAST			0xff

#define MAX_REMOTES				0xfe
#define INVALID_REMOTE			0xff

//#define SD_MODE_OK				0x00
//#define SD_MODE_FAILED			0x01
//
//#define SD_UPDATE_OK			0x00
//#define SD_UPDATE_BAD_ADDR		0x01
//#define SD_UPDATE_BAD_SPIMODE	0x02
//#define SD_UPDATE_BAD_WRITE		0x03

// --------------------------------------------------------------------------
// Typedefs

/*
 * The controller state machine;
 *
 * ControllerStateInit
 *
 * This is the state of the controller when the controller first starts.
 *
 * 		-> ControllerStateRun - After initialization.
 */
typedef enum {
	eControllerStateUnknown, eControllerStateInit, eControllerStateRun
} ControllerStateType;

/*
 * The remote device state machine.  (From the POV of the controller.)
 *
 * RemoteStateInit
 *
 * This is the state of the remote when the remote first starts.
 *
 * 		-> RemoteStateWakeSent - After initialization the remote broadcasts the wake command.
 *
 * RemoteStateWakeSent
 *
 * Yhis is the state after the remote broadcasts the wake command
 *
 * 		-> RemoteStateRun - After receiving an assign command.
 */
typedef enum {
	eRemoteStateUnknown, eRemoteStateAssocReqRcvd, eRemoteStateQuerySent, eRemoteStateRespRcvd, eRemoteStateRun
} ERemoteStatusType;

typedef enum {
	eLocalStateUnknown,
	eLocalStateStarted,
	eLocalStateAssocReqSent,
	eLocalStateAssocRespRcvd,
	eLocalStateAssociated,
	eLocalStateQueryRcvd,
	eLocalStateRespSent,
	eLocalStateRun,
	eLocalStateTuning
} ELocalStatusType;

typedef enum {
	eResponseTimeout = 1,
	eTxBufferFullTimeout = 2,
	eRxBufferFullTimeout = 3,
	eSmacError = 4,
	eOtherError = 5
} ELocalRestartSoftwareReasonType;

typedef enum {
	eUserRestart = 1,
	eSoftwareRestart = 2,
	eWatchDogRestart = 3
} ELocalRestartCauseType;

/*
 * Network  commands
 *
 * CommandNetSetup
 *
 * This command is sent to the gateway (dongle) by the controller and never gets transmitted to the
 * radio network.  It is used to negotiate the creation of a new network on behalf of the controller
 * when it restarts.
 *
 * CommandAssocReq
 *
 * When a remote first starts it broadcasts an associate command which contains a unique ID for the remote.
 * It does this on every channel until a controller responds with an associate response.
 *
 * CommandAssocResp
 *
 * The controller responds to the associate request command by assigning a local destination address for the remote.
 * The remote should then act on all messages sent to that address or the broadcast address.
 *
 * CommandNetCheck
 *
 * This command is used *after* the remote is associated with a remote.
 * The remote should send the net check request command from time-to-time.  The controller will respond.
 * If the contoller doesn't respond then the remote should assume the controller has quit or reset.
 *
 * QueryCommand
 *
 * The controller sends a query to the remote asking for details of one or more facilities.
 *
 * ResponseCommand
 *
 * The remote responds to the query command with the requested information about the facility.
 *
 * ControlAudioCommand
 *
 * The controller or the remote can send a control audio command that contains audio to be played on
 * an endpoint.
 *
 * ControlMotorCommand
 *
 * The controller or the remote can send a control motor command that contains instructions to run a
 * motor at the specified endpoint
 *
 * DataSampleCommand
 *
 * The remote collects and sends a command that contains one or more data samples.
 *
 */
typedef enum {
	eCommandInvalid = -1,
	eCommandNetMgmt = 0,
	eCommandAssoc = 1,
	eCommandInfo = 2,
	eCommandControl = 3,
	eCommandAudio = 4,
	eCommandDataSample = 5
} ECommandGroupIDType;

typedef enum {
	eNetMgmtSubCmdInvalid = -1, eNetMgmtSubCmdNetSetup = 0, eNetMgmtSubCmdNetCheck = 1, eNetMgmtSubCmdNetIntfTest = 2
} ENetMgmtSubCmdIDType;

typedef enum {
	eCmdAssocInvalid = -1, eCmdAssocREQ = 0, eCmdAssocRESP = 1, eCmdAssocCHECK = 2, eCmdAssocACK = 3
} ECmdAssocType;

typedef enum {
	eCmdInfoInvalid = -1, eCmdInfoQuery = 0, eCmdInfoResponse = 1
} EInfoSubCmdIDType;

typedef enum {
	eControlSubCmdInvalid = -1,
	eControlSubCmdScan = 0,
	eControlSubCmdMessage = 1,
	eControlSubCmdLight = 2,
	eControlSubCmdSetPosController = 3,
	eControlSubCmdClearPosController = 4,
	eControlSubCmdButton = 5,
	eControlSubCmdSingleLineMessage = 6,
	eControlSubCmdClearDisplay = 7,
	eControlSubCmdPosconSetup = 8,
	eControlSubCmdPosconBroadcast = 9,
	eControlSubCmdAck = 10
//	eControlSubCmdEndpointAdj = 1,
//	eControlSubCmdMotor = 2,
//	eControlSubCmdButton = 3,
//	eControlSubCmdHooBee = 4,
//	eControlSubCmdSDCardUpdate = 5,
//	eControlSubCmdSDCardUpdateCommit = 6,
//	eControlSubCmdSDCardModeControl = 7,
//	eControlSubCmdSDCardBlockCheck = 8,
//	eControlSubCmdSDCardCheck = 9
} EControlSubCmdIDType;

//typedef enum {
//	eRemoteDataSubCmdInvalid = -1,
//	eRemoteDataSubCmdSample = 1,
//	eRemoteDataSubCmdRateCtrl = 2,
//	eRemoteDataSubCmdCalibrate = 3
//} ERemoteDataSubCmdIDType;

//typedef enum {
//	eSDCardActionInvalid = -1,
//	eSDCardActionSdProtocol = 1,
//	eSDCardActionSpiProtocol = 2,
//} ESDCardControlActionType;

//typedef enum {
//	eSDCardDeviceInvalid = -1,
//	eSDCardDeviceType1 = 1,
//	eSDCardDeviceType2 = 2,
//} ESDCardControlDeviceType;

typedef enum {
	eLedEffectInvalid = -1, eLedEffectSolid = 0, eLedEffectFlash = 1, eLedEffectError = 2, eLedEffectMotel = 3
} ESDCardControlDeviceType;

// --------------------------------------------------------------------------
// Function prototypes.

// --------------------------------------------------------------------------
// Globals.

#endif /* COMMANDSTYPES_H */
