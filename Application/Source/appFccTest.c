/*
 Codeshelf
 © Copyright 2005, 2014 Codeshelf, Inc.
 All rights reserved

 $Id$
 $Name$
 */

// --------------------------------------------------------------------------
// Kernel includes

#include "appFccTest.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "remoteRadioTask.h"
#include "remoteMgmtTask.h"
#include "fccTestTask.h"
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
extern xTaskHandle gFccSetupTask;
extern xTaskHandle gFccTxTask;
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
	MC1324xDrv_IndirectAccessSPIWrite(ANT_PAD_CTRL, /*cANT_PAD_CTRL_ANTX_CTRLMODE +*/ cANT_PAD_CTRL_ANTX_EN);
	MC1324xDrv_IndirectAccessSPIWrite(ANT_AGC_CTRL, 0x40 + 0x02 /*+ 0x01*/);

	uint8_t gpioDir = MC1324xDrv_IndirectAccessSPIRead(GPIO_DIR) | GPIO_PIN2;
	MC1324xDrv_IndirectAccessSPIWrite(GPIO_DIR, gpioDir);
	uint8_t gpioData = MC1324xDrv_IndirectAccessSPIRead(GPIO_DATA) & ~GPIO_PIN2;
	MC1324xDrv_IndirectAccessSPIWrite(GPIO_DATA, gpioData);
	uint8_t gpioPulEn = MC1324xDrv_IndirectAccessSPIRead(GPIO_PUL_EN) | GPIO_PIN2;
	MC1324xDrv_IndirectAccessSPIWrite(GPIO_PUL_EN, gpioPulEn);
	uint8_t gpioPulSel = MC1324xDrv_IndirectAccessSPIRead(GPIO_PUL_SEL) & ~GPIO_PIN2;
	MC1324xDrv_IndirectAccessSPIWrite(GPIO_PUL_SEL, gpioPulSel);

	gLocalDeviceState = eLocalStateStarted;

	/* Start the task that will handle the radio */
	//xTaskCreate(radioTransmitTask, (signed portCHAR *) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY , &gRadioTransmitTask);
	//xTaskCreate(radioReceiveTask, (signed portCHAR *) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY , &gRadioReceiveTask);
	//xTaskCreate(remoteMgmtTask, (signed portCHAR *) "Mgmt", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gRemoteManagementTask);
	xTaskCreate(fccSetupTask, (signed portCHAR *) "FCCSetup", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gFccSetupTask);
	xTaskCreate(fccTxTask, (signed portCHAR *) "FccTx", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gFccTxTask);
	xTaskCreate(scannerReadTask, (signed portCHAR *) "Scan", configMINIMAL_STACK_SIZE, NULL, MGMT_PRIORITY, &gScannerReadTask);
	

//	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
//	vQueueAddToRegistry(gRadioReceiveQueue, (signed char*)"RxQ");

//	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
//	vQueueAddToRegistry(gRadioTransmitQueue, (signed char*)"TxQ");

//	gRemoteMgmtQueue = xQueueCreate(REMOTE_MGMT_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
//	vQueueAddToRegistry(gRemoteMgmtQueue, (signed char*)"MgmtQ");

//	gTxAckQueue = xQueueCreate(TX_ACK_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
//	vQueueAddToRegistry(gTxAckQueue, (signed char*)"TxAck");

	// Set the state to running
	gLocalDeviceState = eLocalStateStarted;

	// Initialize the network check.
	gLastPacketReceivedTick = xTaskGetTickCount();
}

// --------------------------------------------------------------------------

void vApplicationIdleHook(void) {

}
