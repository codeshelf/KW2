/*
 Codeshelf
 © Copyright 2005, 2014 Codeshelf, Inc.
 All rights reserved

 $Id$
 $Name$
 */

// --------------------------------------------------------------------------
// Kernel includes

#include "appCheController.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "remoteRadioTask.h"
#include "remoteMgmtTask.h"
#include "cheControllerTask.h"
#include "scannerReadTask.h"
#include "commands.h"
#include "string.h"
#include "stdlib.h"
#include "SMAC_Interface.h"
#include "Wait.h"
#include "Rs485.h"
#include "serial.h"
#include "eeprom.h"
#include "smacGlue.h"
#include "TransceiverDrv.h"
#include "TransceiverReg.h"

// --------------------------------------------------------------------------
// Globals
portTickType gLastPacketReceivedTick;

extern xTaskHandle gRadioReceiveTask;
extern xTaskHandle gRadioTransmitTask;
extern xTaskHandle gSerialReceiveTask;
extern xTaskHandle gRemoteManagementTask;
extern xTaskHandle gCartControllerTask;
extern xTaskHandle gScannerReadTask;

extern xQueueHandle gRadioTransmitQueue;
extern xQueueHandle gRadioReceiveQueue;
extern xQueueHandle gRemoteMgmtQueue;
extern xQueueHandle gTxAckQueue;

extern ELocalStatusType gLocalDeviceState;

// --------------------------------------------------------------------------

gwUINT8 sendRs485Message(char* msgPtr, gwUINT8 msgLen) {

	gwUINT16 charsSent;

	RS485_TX_ON
	serialTransmitFrame(Rs485_DEVICE, msgPtr, msgLen);
	vTaskDelay(25);
	RS485_TX_OFF

	return 0;
}

// --------------------------------------------------------------------------

void startApplication(void) {
	
	smacErrors_t smacError;
	uint8_t data;
	
//	readEepromData(0, &data, 1);
	
	MC1324xDrv_SPIInit();
	smacError = MLMERadioInit();
	MLMESetPromiscuousMode(gPromiscuousMode_d);
	smacError = MLMESetChannelRequest(DEFAULT_CHANNEL);
	smacError = MLMEPAOutputAdjust(DEFAULT_POWER);
	smacError = MLMEXtalAdjust(DEFAULT_CRYSTAL_TRIM); 
	//smacError = MLMEFEGainAdjust(15);
	MC1324xDrv_IndirectAccessSPIWrite(ANT_PAD_CTRL, cANT_PAD_CTRL_ANTX_CTRLMODE + cANT_PAD_CTRL_ANTX_EN);
	MC1324xDrv_IndirectAccessSPIWrite(ANT_AGC_CTRL, 0x40 + 0x02);

	gLocalDeviceState = eLocalStateStarted;

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (signed portCHAR *) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask);
	xTaskCreate(radioReceiveTask, (signed portCHAR *) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask);
	xTaskCreate(remoteMgmtTask, (signed portCHAR *) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gRemoteManagementTask);
	xTaskCreate(cartControllerTask, (signed portCHAR *) "LED", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gCartControllerTask);
	xTaskCreate(scannerReadTask, (signed portCHAR *) "Scan", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gScannerReadTask);

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	vQueueAddToRegistry(gRadioReceiveQueue, (signed char*)"RxQ");

	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	vQueueAddToRegistry(gRadioTransmitQueue, (signed char*)"TxQ");

	gRemoteMgmtQueue = xQueueCreate(REMOTE_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
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
