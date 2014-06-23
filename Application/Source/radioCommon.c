/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved
 
 $Id$
 $Name$	
 */

#include "radioCommon.h"
#include "gwSystemMacros.h"
#include "FreeRTOS.h"
#include "task.h"

RxRadioBufferStruct	gRxRadioBuffer[RX_BUFFER_COUNT];
BufferCntType 		gRxCurBufferNum = 0;
BufferCntType 		gRxUsedBuffers = 0;

TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];
BufferCntType 		gTxCurBufferNum = 0;
BufferCntType 		gTxUsedBuffers = 0;

extern xTaskHandle 	gRadioReceiveTask;
extern ERxMessageHolderType gRxMsg;


// --------------------------------------------------------------------------

void advanceRxBuffer() {

	gwUINT8 ccrHolder;

	// The buffers are a shared, critical resource, so we have to protect them before we update.
	GW_ENTER_CRITICAL(ccrHolder);

	gRxRadioBuffer[gRxCurBufferNum].bufferStatus = eBufferStateInUse;

	// Advance to the next buffer.
	gRxCurBufferNum++;
	if (gRxCurBufferNum >= (RX_BUFFER_COUNT))
		gRxCurBufferNum = 0;

	// Account for the number of used buffers.
	if (gRxUsedBuffers < RX_BUFFER_COUNT)
		gRxUsedBuffers++;

	GW_EXIT_CRITICAL(ccrHolder);
}

// --------------------------------------------------------------------------

BufferCntType lockRxBuffer() {

	BufferCntType result;
	gwUINT8 ccrHolder;

	// Wait until there is a free buffer.
	while (gRxRadioBuffer[gRxCurBufferNum].bufferStatus == eBufferStateInUse) {
		vTaskDelay(1);
	}

	// The buffers are a shared, critical resource, so we have to protect them before we update.
	GW_ENTER_CRITICAL(ccrHolder);

	result = gRxCurBufferNum;

	gRxRadioBuffer[result].bufferStatus = eBufferStateInUse;

	// Advance to the next buffer.
	gRxCurBufferNum++;
	if (gRxCurBufferNum >= (RX_BUFFER_COUNT))
		gRxCurBufferNum = 0;

	// Account for the number of used buffers.
	gRxUsedBuffers++;

	GW_EXIT_CRITICAL(ccrHolder);

	return result;
}

// --------------------------------------------------------------------------

BufferCntType lockTxBuffer() {

	BufferCntType result;
	gwUINT8 ccrHolder;

	// Wait until there is a free buffer.
	gwUINT8 retries = 0;
	while (gTxRadioBuffer[gTxCurBufferNum].bufferStatus == eBufferStateInUse) {
		vTaskDelay(1);
		if (retries++ > 100) {
			GW_RESET_MCU()
			;
		}
	}

	// The buffers are a shared, critical resource, so we have to protect them before we update.
	GW_ENTER_CRITICAL(ccrHolder);

	result = gTxCurBufferNum;
	gTxRadioBuffer[result].bufferStatus = eBufferStateInUse;

	// Advance to the next buffer.
	gTxCurBufferNum++;
	if (gTxCurBufferNum >= (TX_BUFFER_COUNT))
		gTxCurBufferNum = 0;

	// Account for the number of used buffers.
	gTxUsedBuffers++;

	GW_EXIT_CRITICAL(ccrHolder);

	return result;
}

// --------------------------------------------------------------------------

void suspendRadioRx() {	
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

//	GW_ENTER_CRITICAL(ccrHolder);
	smacError = MLMERXDisableRequest();
//	vTaskSuspend(gRadioReceiveTask);
//	GW_EXIT_CRITICAL(ccrHolder);
}

// --------------------------------------------------------------------------

void resumeRadioRx() {
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	//	GW_ENTER_CRITICAL(ccrHolder);
	// The radio's state won't go to idle, becuase this routine gets called during 
	smacError = MLMERXDisableRequest();
	smacError = MLMERXEnableRequest(gRxMsg.rxPacketPtr, 0);
//	GW_EXIT_CRITICAL(ccrHolder);
//
//	vTaskResume(gRadioReceiveTask);
}
// --------------------------------------------------------------------------

void debugReset() {
	Cpu_SystemReset();
}

// --------------------------------------------------------------------------

void debugWatchdogallback(void) {
	Cpu_SystemReset();
}
// --------------------------------------------------------------------------

void debugRefreed(BufferCntType inBufferNum) {
	// Any code, so that we have a place to set a breakpoint.
	gTxRadioBuffer[inBufferNum].bufferStatus = eBufferStateFree;
}
