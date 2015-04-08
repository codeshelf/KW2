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
//#include "GpsCalibrate.h"

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
	uint8_t trim = DEFAULT_CRYSTAL_TRIM;
	MLMEXtalAdjust(trim); 
//	MLMEFEGainAdjust(15);
	
	// Capture and measure the rising edges of the GPS PPS signal.
	FTM0_C2V = 0;
	FTM0_MOD = 0xffff;
	FTM0_CNTIN = 0x0001;
	FTM0_COMBINE |= (FTM_COMBINE_DECAP1_MASK);
	// Loop until there is an event.
	while ((FTM0_STATUS & FTM_STATUS_CH3F_MASK) == 0) {
		GW_WATCHDOG_RESET;
	}
	uint16_t cycles = 0;
	if (FTM0_C2V < FTM0_C3V) {
		cycles = FTM0_C3V - FTM0_C2V;
	} else {
		cycles = (0xffff - FTM0_C2V) + FTM0_C3V;
	}
	
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
	
	Cpu_EnableInt();
}

// --------------------------------------------------------------------------

void TunerIsr(void) {
	gLocalDeviceState = eLocalStateStarted;
}

// --------------------------------------------------------------------------

void vApplicationIdleHook(void) {

}
