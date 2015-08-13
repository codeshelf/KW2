/*
 Codeshelf
 © Copyright 2005, 2014 Codeshelf, Inc.
 All rights reserved

 $Id$
 $Name$
 */

// --------------------------------------------------------------------------
// Kernel includes
#include "appTuner.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "remoteRadioTask.h"
#include "remoteMgmtTask.h"
#include "tunerTask.h"
#include "SMAC_Interface.h"
#include "Wait.h"
#include "smacGlue.h"
#include "TransceiverDrv.h"
#include "Cpu.h"

// --------------------------------------------------------------------------
// Globals
portTickType gLastPacketReceivedTick;

extern xTaskHandle gRadioReceiveTask;
extern xTaskHandle gRadioTransmitTask;
extern xTaskHandle gRemoteManagementTask;
extern xTaskHandle gTunerTask;

extern xQueueHandle gRadioTransmitQueue;
extern xQueueHandle gRadioReceiveQueue;
extern xQueueHandle gRemoteMgmtQueue;
extern xQueueHandle gTxAckQueue;

extern ELocalStatusType gLocalDeviceState;

// --------------------------------------------------------------------------

void startApplication(void) {

	MC1324xDrv_SPIInit();
	MLMERadioInit();
	MLMESetPromiscuousMode(gPromiscuousMode_d);
	MLMESetChannelRequest(DEFAULT_CHANNEL);
	MLMEPAOutputAdjust(DEFAULT_POWER);
	MLMEXtalAdjust(DEFAULT_CRYSTAL_TRIM); 
//	MLMEFEGainAdjust(15);
	
	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (const signed portCHAR * const) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask );
	xTaskCreate(radioReceiveTask, (const signed portCHAR * const) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask );
	xTaskCreate(remoteMgmtTask, (const signed portCHAR * const) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gRemoteManagementTask );
	xTaskCreate(tunerTask, (const signed portCHAR * const) "Tune", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gTunerTask );

	
	//gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	//gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	//gRemoteMgmtQueue = xQueueCreate(GATEWAY_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	
	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	vQueueAddToRegistry(gRadioReceiveQueue, (signed char*)"RxQ");

	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	vQueueAddToRegistry(gRadioTransmitQueue, (signed char*)"TxQ");
	
	gRemoteMgmtQueue = xQueueCreate(GATEWAY_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	vQueueAddToRegistry(gRemoteMgmtQueue, (signed char*)"MgmtQ");

	gTxAckQueue = xQueueCreate(TX_ACK_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	vQueueAddToRegistry(gTxAckQueue, (signed char*)"TxAck");

	// Set the state to running
	gLocalDeviceState = eLocalStateTuning;

	// Initialize the network check.
	gLastPacketReceivedTick = xTaskGetTickCount();
	
	Cpu_EnableInt();
}

// --------------------------------------------------------------------------

void TunerIsr(void) {
	gLocalDeviceState = eLocalStateStarted;
}

// --------------------------------------------------------------------------

void vApplicationIdleHook(void) {

}
