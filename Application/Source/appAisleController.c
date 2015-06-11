/*
 Codeshelf
 © Copyright 2005, 2014 Codeshelf, Inc.
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
#include "TransceiverReg.h"
#include "globals.h"
#include "radioCommon.h"

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
extern xQueueHandle gTxAckQueue;

extern ELocalStatusType gLocalDeviceState;

// --------------------------------------------------------------------------

void startApplication(void) {

	MC1324xDrv_SPIInit();
	MLMERadioInit();
	MLMESetPromiscuousMode(gPromiscuousMode_d);
	MLMESetChannelRequest(DEFAULT_CHANNEL);
	MLMEPAOutputAdjust(DEFAULT_POWER);
	MLMEXtalAdjust(trim[0]);
	MC1324xDrv_IndirectAccessSPIWrite(ANT_PAD_CTRL, cANT_PAD_CTRL_ANTX_CTRLMODE + cANT_PAD_CTRL_ANTX_EN);
	MC1324xDrv_IndirectAccessSPIWrite(ANT_AGC_CTRL, 0x40 + 0x02);
//	MLMEFEGainAdjust(15);
	
	// Random connection start delay based on guid
	uint32_t seed = 0;
	seed = (uint32_t) (guid[6] << 16) | (guid[7] & 0xff);
	srand(seed);
	vTaskDelay(rand() % RAND_BACK_OFF_LIMIT);

	gLocalDeviceState = eLocalStateStarted;

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (const signed portCHAR * const) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask );
	xTaskCreate(radioReceiveTask, (const signed portCHAR * const) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask );
	xTaskCreate(remoteMgmtTask, (const signed portCHAR * const) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gRemoteManagementTask );
	xTaskCreate(aisleControllerTask, (const signed portCHAR * const) "LED", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gAisleControllerTask );

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	vQueueAddToRegistry(gRadioReceiveQueue, (signed char*)"RxQ");

	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	vQueueAddToRegistry(gRadioTransmitQueue, (signed char*)"TxQ");
	
	gRemoteMgmtQueue = xQueueCreate(GATEWAY_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	vQueueAddToRegistry(gRemoteMgmtQueue, (signed char*)"MgmtQ");

	gTxAckQueue = xQueueCreate(TX_ACK_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	vQueueAddToRegistry(gTxAckQueue, (signed char*)"TxAck");

	// Set the state to running
	gLocalDeviceState = eLocalStateStarted;

	// Initialize the network check.
	gLastPacketReceivedTick = xTaskGetTickCount();
	
}

// --------------------------------------------------------------------------

void vApplicationIdleHook(void) {

}
