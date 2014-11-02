/*
 Codeshelf
 � Copyright 2005, 2014 Codeshelf, Inc.
 All rights reserved

 $Id$
 $Name$
 */

// --------------------------------------------------------------------------
// Kernel includes
#include "appAisleController.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "remoteRadioTask.h"
#include "remoteMgmtTask.h"
#include "aisleControllerTask.h"
#include "scannerReadTask.h"
#include "commands.h"
#include "SMAC_Interface.h"
#include "Wait.h"
#include "smacGlue.h"
#include "TransceiverDrv.h"

// --------------------------------------------------------------------------
// Globals
portTickType gLastPacketReceivedTick;

extern xTaskHandle gRadioReceiveTask;
extern xTaskHandle gRadioTransmitTask;
extern xTaskHandle gSerialReceiveTask;
extern xTaskHandle gRemoteManagementTask;
extern xTaskHandle gAisleControllerTask;

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
	xTaskCreate(aisleControllerTask, (const signed portCHAR * const) "LED", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gAisleControllerTask );

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	gRemoteMgmtQueue = xQueueCreate(GATEWAY_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));

	// Set the state to running
	gLocalDeviceState = eLocalStateStarted;

	// Initialize the network check.
	gLastPacketReceivedTick = xTaskGetTickCount();
	
//	/* All the tasks have been created - start the scheduler. */
//	vTaskStartScheduler();
//
//	/* Should not reach here! */
//	for (;;)
//		;
}

// --------------------------------------------------------------------------

void vApplicationIdleHook(void) {

//	GW_WATCHDOG_RESET;
//
//	// If we haven't received a packet in by timeout seconds then reset.
//	portTickType ticks = xTaskGetTickCount();
//	if (ticks > (gLastPacketReceivedTick + kNetCheckTickCount)) {
//		GW_RESET_MCU();
//	}
//	//vTaskDelay(100);
}