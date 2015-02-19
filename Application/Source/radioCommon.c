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
TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];

extern xTaskHandle 			gRadioReceiveTask;
extern ERxMessageHolderType gRxMsg;
extern gwBoolean			gIsReceiving;
extern gwBoolean			gIsTransmitting;

// --------------------------------------------------------------------------

void suspendRadioRx() {	
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	GW_ENTER_CRITICAL(ccrHolder);
	if (gIsReceiving) {
		smacError = MLMERXDisableRequest();
		if (smacError != gErrorNoError_c){
			GW_RESET_MCU()
		}
		gIsReceiving = FALSE;
	}
	GW_EXIT_CRITICAL(ccrHolder);
}

// --------------------------------------------------------------------------

void resumeRadioRx() {
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	// ALlow any TX to complete.  
	// Run outside the critical section so that tasks context switch until complete.
	while (gIsTransmitting) {
		vTaskDelay(1);
	}

	GW_ENTER_CRITICAL(ccrHolder);
	if (gIsReceiving) {
		smacError = MLMERXDisableRequest();
		if (smacError != gErrorNoError_c){
			GW_RESET_MCU()
		}
	}
	// Reset the buffer to resume RX.
	gRxMsg.rxPacketPtr->u8MaxDataLength = RX_BUFFER_SIZE;
	gRxMsg.rxPacketPtr->rxStatus = rxInitStatus;
	smacError = MLMERXEnableRequest(gRxMsg.rxPacketPtr, 0);
	gIsReceiving = TRUE;
	if (smacError != gErrorNoError_c){
		GW_RESET_MCU()
	}
	GW_EXIT_CRITICAL(ccrHolder);
}

// --------------------------------------------------------------------------

void debugReset() {
	Cpu_SystemReset();
}

// --------------------------------------------------------------------------

void debugWatchdogallback(void) {
	Cpu_SystemReset();
}
