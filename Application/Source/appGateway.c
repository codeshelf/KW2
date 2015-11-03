/*
 Codeshelf
 © Copyright 2005, 2014 Codeshelf, Inc.
 All rights reserved

 $Id$
 $Name$
 */

// --------------------------------------------------------------------------
// Kernel includes

#include "appGateway.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "remoteRadioTask.h"
#include "gatewayTask.h"
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

extern xQueueHandle gRadioTransmitQueue;
extern xQueueHandle gRadioReceiveQueue;
extern xQueueHandle gTxAckQueue;

ELocalStatusType gLocalDeviceState;

// --------------------------------------------------------------------------

void startApplication(void) {
	
	smacErrors_t smacError;
	uint8_t data;
	
	MC1324xDrv_SPIInit();
	smacError = MLMERadioInit();
	MLMESetPromiscuousMode(gPromiscuousMode_d);
	smacError = MLMESetChannelRequest(DEFAULT_CHANNEL);
	smacError = MLMEPAOutputAdjust(DEFAULT_POWER);
	smacError = MLMEXtalAdjust(gTrim[0]); 
	//smacError = MLMEFEGainAdjust(15);
	MC1324xDrv_IndirectAccessSPIWrite(ANT_PAD_CTRL, cANT_PAD_CTRL_ANTX_CTRLMODE + cANT_PAD_CTRL_ANTX_EN);
	MC1324xDrv_IndirectAccessSPIWrite(ANT_AGC_CTRL, 0x40 + 0x02 /*+ 0x01*/);

	//MC1324xDrv_Set_CLK_OUT_Freq(gCLK_OUT_FREQ_32_MHz);
	//Cpu_SetClockConfiguration(1);
	
	gLocalDeviceState = eLocalStateStarted;

	/* Start the task that will handle the radio */
	xTaskCreate(radioTransmitTask, (signed portCHAR *) "RadioTX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioTransmitTask);
	xTaskCreate(radioReceiveTask, (signed portCHAR *) "RadioRX", configMINIMAL_STACK_SIZE, NULL, RADIO_PRIORITY, &gRadioReceiveTask);
	xTaskCreate(serialReceiveTask, (signed portCHAR *) "Serial", configMINIMAL_STACK_SIZE, NULL, SERIAL_RECV_PRIORITY, &gSerialReceiveTask);

	gRadioReceiveQueue = xQueueCreate(RX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	vQueueAddToRegistry(gRadioReceiveQueue, (signed char*)"RxQ");

	gRadioTransmitQueue = xQueueCreate(TX_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(BufferCntType));
	vQueueAddToRegistry(gRadioTransmitQueue, (signed char*)"TxQ");

	gTxAckQueue = xQueueCreate(TX_ACK_QUEUE_SIZE, (unsigned portBASE_TYPE) sizeof(gwUINT8));
	vQueueAddToRegistry(gTxAckQueue, (signed char*)"TxAck");

	// Set the state to running
	gLocalDeviceState = eLocalStateRun;

	// Initialize the network check.
	gLastPacketReceivedTick = xTaskGetTickCount();
}

// --------------------------------------------------------------------------

void vApplicationIdleHook(void) {

}
