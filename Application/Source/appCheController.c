/*
 Codeshelf
 � Copyright 2005, 2014 Codeshelf, Inc.
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
#include "eeprom.h"
#include "smacGlue.h"
#include "TransceiverDrv.h"
#include "TransceiverReg.h"
#include "globals.h"

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

void startApplication(void) {
	
	smacErrors_t smacError;
	uint8_t data;
	uint32_t *tmp;
	
//	Cpu_EnableInt();
//	while (1) {
//		
//	}
//	
//	tmp = 0x08000001;
//	tmp = *tmp;

	MC1324xDrv_SPIInit();
	smacError = MLMERadioInit();
	MLMESetPromiscuousMode(gPromiscuousMode_d);
	smacError = MLMESetChannelRequest(DEFAULT_CHANNEL);
	smacError = MLMEPAOutputAdjust(DEFAULT_POWER);
	smacError = MLMEXtalAdjust(gTrim[0]);
	//smacError = MLMEFEGainAdjust(15);
	MC1324xDrv_IndirectAccessSPIWrite(ANT_PAD_CTRL, cANT_PAD_CTRL_ANTX_CTRLMODE + cANT_PAD_CTRL_ANTX_EN);
	MC1324xDrv_IndirectAccessSPIWrite(ANT_AGC_CTRL, 0x40 + 0x02 + 0x01);

	// Set the modem's GPIO2 to output and value 1.
	uint8_t gpioDir = MC1324xDrv_IndirectAccessSPIRead(GPIO_DIR);
	MC1324xDrv_IndirectAccessSPIWrite(GPIO_DIR, gpioDir | cGPIO_DIR_2);
	uint8_t gpioData = MC1324xDrv_IndirectAccessSPIRead(GPIO_DATA);
	MC1324xDrv_IndirectAccessSPIWrite(GPIO_DATA, gpioDir | cGPIO_DATA_2);

	// Random connection start delay based on guid
	uint32_t seed = 0;
	seed = (uint32_t) (gGuid[6] << 16) | (gGuid[7] & 0xff);
	srand(seed);
	vTaskDelay(rand() % RAND_BACK_OFF_LIMIT);

	gLocalDeviceState = eLocalStateStarted;

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (signed portCHAR *) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY , &gRadioTransmitTask);
	xTaskCreate(radioReceiveTask, (signed portCHAR *) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY , &gRadioReceiveTask);
	xTaskCreate(remoteMgmtTask, (signed portCHAR *) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gRemoteManagementTask);
	xTaskCreate(cheControllerTask, (signed portCHAR *) "LED", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gCartControllerTask);
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
