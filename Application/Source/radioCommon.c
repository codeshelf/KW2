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

ERxMessageHolderType gRxMsg;
ETxMessageHolderType gTxMsg;

RadioStateEnum gRadioState = eIdle;

//Make sure radio is idle then change channel.
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

	gwBoolean wasInRxMode = FALSE;
	if (gRadioState == eRx) {
		wasInRxMode = TRUE;
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
	
	if(wasInRxMode) {
		//Go back to RxMode -- If we're already in Rx mode this method will just return.
		readRadioRx();
	}
}

//Make sure the radio is idle then perform Tx. This method blocks until the Tx is complete. This method will return the radio to the read state if it was
//reading before.
void writeRadioTx() {
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	//TODO Do we really have to do this each time?
	gTxMsg.txPacketPtr = (txPacket_t *) &(gTxRadioBuffer[0].txPacket);
	gTxMsg.txPacketPtr->u8DataLength = gTxRadioBuffer[0].bufferSize;
	gTxMsg.bufferNum = 0;

	GW_ENTER_CRITICAL(ccrHolder);

	//If we're already doing a Tx do nothing since we only have one buffer. This should never happen since we only have one task that writes
	if(gRadioState == eTx) {
		return;
	}

	//If we are in Rx Mode then we will disable the rxRadio. Contrast this to the read method below.
	gwBoolean wasInRxMode = FALSE;
	if (gRadioState == eRx) {
		wasInRxMode = TRUE;
		smacError = MLMERXDisableRequest();
		if (smacError != gErrorNoError_c){
			GW_RESET_MCU()
		}
		//Should be idle now
	}

	//Request Tx
	smacError = MCPSDataRequest(gTxMsg.txPacketPtr);
	if (smacError != gErrorNoError_c) {
		GW_RESET_MCU()
	}
	gRadioState = eTx;
	GW_EXIT_CRITICAL(ccrHolder);
	
	//Wait for write to finish
	while (gRadioState == eTx) {
		vTaskDelay(1);
	}
	
	if(wasInRxMode) {
		//Go back to RxMode -- If we're already in Rx mode this method will just return.
		readRadioRx();
	}

}

//Make sure the radio is idle then perform Rx. This method sleeps until an existing Tx is complete.
void readRadioRx() {	
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	//TODO Do we really have to do this each time? Maybe only set rXStatus
	gRxMsg.rxPacketPtr = (rxPacket_t *) &(gRxRadioBuffer[0].rxPacket);
	gRxMsg.rxPacketPtr->u8MaxDataLength = RX_BUFFER_SIZE;
	gRxMsg.rxPacketPtr->rxStatus = rxInitStatus;
	gRxMsg.bufferNum = 0;

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
		return;
	}

	//Perform the actual read and store it in the pointer with no timeout.
	smacError = MLMERXEnableRequest(gRxMsg.rxPacketPtr, 0);
	if (smacError != gErrorNoError_c){
		GW_RESET_MCU()
	}
	gRadioState = eRx;
	GW_EXIT_CRITICAL(ccrHolder);
}


void debugReset() {
	Cpu_SystemReset();
}


void debugWatchdogallback(void) {
	Cpu_SystemReset();
}
