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
#include "queue.h"
#include "Wait.h"

extern xQueueHandle	gRadioReceiveQueue;

RxRadioBufferStruct	gRxRadioBuffer[RX_BUFFER_COUNT];
TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];

ERxMessageHolderType gRxMsg;
ETxMessageHolderType gTxMsg;

RadioStateEnum gRadioState = eIdle;

//Make sure radio is idle then change channel. If the channel was in read mode notify the reader task waiting on the queue.
void setRadioChannel(ChannelNumberType channel) {	
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	GW_ENTER_CRITICAL(ccrHolder);
	
	//Wait for Tx to finish
	while(gRadioState == eTx) {
		GW_EXIT_CRITICAL(ccrHolder);
		vTaskDelay(1);
		//Entry critical before we check the state again
		GW_ENTER_CRITICAL(ccrHolder);
	}
	
	gwBoolean readWasCancelled = FALSE;
	if (gRadioState == eRx) {
		readWasCancelled = TRUE;
		smacError = MLMERXDisableRequest();
		if (smacError != gErrorNoError_c){
			GW_RESET_MCU()
		}
		gRadioState = eIdle;
	}

	//State is now idle, change the channel
	smacError = MLMESetChannelRequest(channel);
	if (smacError != gErrorNoError_c) {
		GW_RESET_MCU()
	}

	GW_EXIT_CRITICAL(ccrHolder);
	
	if(readWasCancelled) {
		//Notify anyone listening on the receive queue that the read was canceled.
		//xQueueSend(gRadioReceiveQueue, &gRxMsg.bufferNum, (portBASE_TYPE) 0);
		xQueueGenericSend(gRadioReceiveQueue, &gRxMsg.bufferNum, (portTickType) 0, (portBASE_TYPE) queueSEND_TO_BACK);
	}
}

//Make sure the radio is idle then perform Tx.  If the channel was in read mode notify the reader task waiting on the queue.
void writeRadioTx() {
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	GW_ENTER_CRITICAL(ccrHolder);

	//If we're already doing a Tx do nothing since we only have one buffer. This should never happen since we only have one task that writes.
	//Unfortunately, since we only have one buffer sends can get messed up
	if(gRadioState == eTx) {
		GW_EXIT_CRITICAL(ccrHolder);
		return;
	}

	//If we are in Rx Mode then we will disable the rxRadio. Contrast this to the read method below.
	gwBoolean readWasCancelled = FALSE;
	if (gRadioState == eRx) {   
		readWasCancelled = TRUE;
		smacError = MLMERXDisableRequest();
		if (smacError != gErrorNoError_c){
			GW_RESET_MCU()
		}
	}
	
	//TODO Do we really have to do this each time?
	gTxMsg.txPacketPtr = (txPacket_t *) &(gTxRadioBuffer[0].txPacket);
	gTxMsg.txPacketPtr->u8DataLength = gTxRadioBuffer[0].bufferSize;
	gTxMsg.bufferNum = 0;

	//Request Tx
	smacError = MCPSDataRequest(gTxMsg.txPacketPtr);
	if (smacError != gErrorNoError_c) {
		GW_RESET_MCU()
	}
	gRadioState = eTx;
	GW_EXIT_CRITICAL(ccrHolder);
	
	if(readWasCancelled) {
		//Notify anyone listening on the receive queue that the read was cancelled.
		//xQueueSend(gRadioReceiveQueue, &gRxMsg.bufferNum, (portBASE_TYPE) 0);
		xQueueGenericSend(gRadioReceiveQueue, &gRxMsg.bufferNum, (portTickType) 0, (portBASE_TYPE) queueSEND_TO_BACK);

	}

}

//Make sure the radio is idle then perform Rx. This method sleeps until an existing Tx is complete.
void readRadioRx() {	
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	GW_ENTER_CRITICAL(ccrHolder);

	//Wait for Tx to finish
	while(gRadioState == eTx) {
		GW_EXIT_CRITICAL(ccrHolder);
		vTaskDelay(1);
		//Entry critical before we check the state again
		GW_ENTER_CRITICAL(ccrHolder);
	}

	//If we're already doing an Rx do nothing since we only have one buffer. This should never happen since we only have 1 reader
	if(gRadioState == eRx) {
		GW_EXIT_CRITICAL(ccrHolder);
		return;
	}
	
	//TODO Do we really have to do this each time? Maybe only set rXStatus
	gRxMsg.rxPacketPtr = (rxPacket_t *) &(gRxRadioBuffer[0].rxPacket);
	gRxMsg.rxPacketPtr->u8MaxDataLength = RX_BUFFER_SIZE;
	gRxMsg.rxPacketPtr->rxStatus = rxInitStatus;
	gRxMsg.bufferNum = 0;

	//Perform the actual read and store it in the pointer with no timeout.
	smacError = MLMERXEnableRequest(gRxMsg.rxPacketPtr, 0);
	if (smacError != gErrorNoError_c){
		GW_RESET_MCU()
	}
	gRadioState = eRx;
	GW_EXIT_CRITICAL(ccrHolder);
}

void setStatusLed(uint8_t red, uint8_t green, uint8_t blue) {
	uint8_t ccrHolder;

	GW_ENTER_CRITICAL(ccrHolder);

	// Red
	for (uint8_t bitpos = 128; bitpos > 0; bitpos >>= 1) {
		if (red & bitpos) {
			StatusLedSdi_SetVal(StatusLedSdi_DeviceData);
		} else {
			StatusLedSdi_ClrVal(StatusLedSdi_DeviceData);
		}
		Wait_Waitus(1);
		StatusLedClk_SetVal(StatusLedClk_DeviceData);
		Wait_Waitus(1);
		StatusLedClk_ClrVal(StatusLedClk_DeviceData);
		Wait_Waitus(1);
	}

	// Green
	for (uint8_t bitpos = 128; bitpos > 0; bitpos >>= 1) {
		if (green & bitpos) {
			StatusLedSdi_SetVal(StatusLedSdi_DeviceData);
		} else {
			StatusLedSdi_ClrVal(StatusLedSdi_DeviceData);
		}
		Wait_Waitus(1);
		StatusLedClk_SetVal(StatusLedClk_DeviceData);
		Wait_Waitus(1);
		StatusLedClk_ClrVal(StatusLedClk_DeviceData);
		Wait_Waitus(1);
	}

	// Blue
	for (uint8_t bitpos = 128; bitpos > 0; bitpos >>= 1) {
		if (blue & bitpos) {
			StatusLedSdi_SetVal(StatusLedSdi_DeviceData);
		} else {
			StatusLedSdi_ClrVal(StatusLedSdi_DeviceData);
		}
		Wait_Waitus(1);
		StatusLedClk_SetVal(StatusLedClk_DeviceData);
		Wait_Waitus(1);
		StatusLedClk_ClrVal(StatusLedClk_DeviceData);
		Wait_Waitus(1);
	}

	GW_EXIT_CRITICAL(ccrHolder);
}

void debugReset() {
	Cpu_SystemReset();
}


void debugWatchdogallback(void) {
	Cpu_SystemReset();
}
