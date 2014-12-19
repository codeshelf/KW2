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

extern xQueueHandle gRadioTransmitQueue;
extern xQueueHandle gRadioReceiveQueue;
extern xQueueHandle gRemoteMgmtQueue;

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

	gLocalDeviceState = eLocalStateStarted;

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (const signed portCHAR * const) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask );
	xTaskCreate(radioReceiveTask, (const signed portCHAR * const) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask );
	xTaskCreate(remoteMgmtTask, (const signed portCHAR * const) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gRemoteManagementTask );

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	gRemoteMgmtQueue = xQueueCreate(GATEWAY_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));

	// Set the state to running
	gLocalDeviceState = eLocalStateStarted;

	// Initialize the network check.
	gLastPacketReceivedTick = xTaskGetTickCount();
	
	/* Should not reach here! */
	EnterCritical();
	ExitCritical();
	Cpu_EnableInt();
	for (;;)
		;
}

// --------------------------------------------------------------------------

void TunerIsr(void) {
	gLocalDeviceState = eLocalStateStarted;
}

// --------------------------------------------------------------------------

void vApplicationIdleHook(void) {

}
