/*
 FlyWeight
 � Copyright 2005, 2006 Jeffrey B. Williams
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
#include "commands.h"

extern xQueueHandle	gRadioReceiveQueue;

RxRadioBufferStruct	gRxRadioBuffer[RX_BUFFER_COUNT];
BufferCntType 		gRxCurBufferNum = 0;
BufferCntType 		gRxUsedBuffers = 0;

TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];
BufferCntType 		gTxCurBufferNum = 0;
BufferCntType 		gTxUsedBuffers = 0;

ERxMessageHolderType gRxMsg;
ETxMessageHolderType gTxMsg;

gwUINT8 gLastLQI;

RadioStateEnum gRadioState = eIdle;

extern ELocalStatusType gLocalDeviceState;
extern NetAddrType gMyAddr;

// --------------------------------------------------------------------------

void advanceRxBuffer() {

	gwUINT8 ccrHolder;

	// The buffers are a shared, critical resource, so we have to protect them before we update.
	GW_ENTER_CRITICAL(ccrHolder);

	gRxRadioBuffer[gRxCurBufferNum].bufferStatus = eBufferStateInUse;

	// marker
	strcpy(gRxRadioBuffer[gRxCurBufferNum].bufferStorage, "advance");

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
			GW_RESET_MCU(eTxBufferFullTimeout)
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

//Make sure radio is idle then change channel. If the channel was in read mode notify the reader task waiting on the queue.
void setRadioChannel(ChannelNumberType channel) {	
	ChannelNumberType oldChannel;
	smacErrors_t smacError;
	gwUINT8 ccrHolder;

	oldChannel = MLMEGetChannelRequest();
	// We need to remain compatible with the old MC1322x until we finally switch away from it.
	// Ivan will need to coordinate the change at the back end of the Java system.
	channel += 11;

	if (channel != oldChannel) {

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
				GW_RESET_MCU(eSmacError)
			}
			gRadioState = eIdle;
		}

		//State is now idle, change the channel
		smacError = MLMESetChannelRequest(channel);
		if (smacError != gErrorNoError_c) {
			GW_RESET_MCU(eSmacError)
		}

		GW_EXIT_CRITICAL(ccrHolder);

		if(readWasCancelled) {
			//Notify anyone listening on the receive queue that the read was canceled.
			//xQueueSend(gRadioReceiveQueue, &gRxMsg.bufferNum, (portBASE_TYPE) 0);
			//xQueueGenericSend(gRadioReceiveQueue, &gRxMsg.bufferNum, (portTickType) 0, (portBASE_TYPE) queueSEND_TO_BACK);
			RELEASE_RX_BUFFER(gRxMsg.bufferNum, ccrHolder);
		}
	}
}

//Make sure the radio is idle then perform Tx.  If the channel was in read mode notify the reader task waiting on the queue.
void writeRadioTx(BufferCntType inTxBufferNum) {
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
			GW_RESET_MCU(eSmacError)
		}
	}
	
	//TODO Do we really have to do this each time?
	gTxMsg.txPacketPtr = (txPacket_t *) &(gTxRadioBuffer[inTxBufferNum].txPacket);
	gTxMsg.txPacketPtr->u8DataLength = gTxRadioBuffer[inTxBufferNum].bufferSize;
	gTxMsg.bufferNum = inTxBufferNum;

	//Request Tx
	smacError = MCPSDataRequest(gTxMsg.txPacketPtr);
	if (smacError != gErrorNoError_c) {
		GW_RESET_MCU(eSmacError)
	}
	gRadioState = eTx;
	
//	serialTransmitFrame(UART0_BASE_PTR, (BufferStoragePtrType) ("TX"),  2);
	
	GW_EXIT_CRITICAL(ccrHolder);
	
	if(readWasCancelled) {
//		//Notify anyone listening on the receive queue that the read was cancelled.
		RELEASE_RX_BUFFER(gRxMsg.bufferNum, ccrHolder);
//		//xQueueSend(gRadioReceiveQueue, &gRxMsg.bufferNum, (portBASE_TYPE) 0);
//		xQueueGenericSend(gRadioReceiveQueue, &gRxMsg.bufferNum, (portTickType) 0, (portBASE_TYPE) queueSEND_TO_BACK);
	}
}

//Make sure the radio is idle then perform Rx. This method sleeps until an existing Tx is complete.
void readRadioRx() {
	BufferCntType lockedBufferNum;
	smacErrors_t smacError;
	gwUINT8 ccrHolder;
	//gwBoolean packetForMe = FALSE;

	GW_ENTER_CRITICAL(ccrHolder);

	//while (!packetForMe) {
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
		lockedBufferNum = lockRxBuffer();
		gRxMsg.rxPacketPtr = (rxPacket_t *) &(gRxRadioBuffer[lockedBufferNum].rxPacket);
		gRxMsg.rxPacketPtr->u8MaxDataLength = RX_BUFFER_SIZE;
		gRxMsg.rxPacketPtr->rxStatus = rxInitStatus;
		gRxMsg.bufferNum = lockedBufferNum;// 0;
	
		//Perform the actual read and store it in the pointer with no timeout.
		smacError = MLMERXEnableRequest(gRxMsg.rxPacketPtr, 0);
		if (smacError != gErrorNoError_c){
			GW_RESET_MCU(eSmacError)
		}
		
		smacError = MLMELinkQuality(&(gRxMsg.rxPacketPtr->lqi));
		if (smacError != gErrorNoError_c){
			GW_RESET_MCU(eSmacError)
		}
		
		gRadioState = eRx;
		
		//preProcessPacket(lockedBufferNum);
	//}
//	serialTransmitFrame(UART0_BASE_PTR, (BufferStoragePtrType) ("RX"),  2);

	GW_EXIT_CRITICAL(ccrHolder);
}

gwBoolean preProcessPacket(int inBufferNum) {
	gwUINT8 ccrHolder;
	ECommandGroupIDType cmdID = getCommandID(gRxRadioBuffer[inBufferNum].bufferStorage);
	ECmdAssocType assocSubCmd;
	NetAddrType cmdDstAddr;
	
	// Always reset on netchecks
	if (cmdID == eCommandNetMgmt && gLocalDeviceState == eLocalStateRun) {
		GW_WATCHDOG_RESET;
		setStatusLed(0, 0, 1);
		RELEASE_RX_BUFFER(inBufferNum, ccrHolder);
		return FALSE;
	}
	
	// Un-associated - Only process incoming association packets
	if (gLocalDeviceState != eLocalStateRun) {
		if (cmdID == eCommandControl) {
			RELEASE_RX_BUFFER(inBufferNum, ccrHolder);
			return FALSE;
		} else {
			assocSubCmd = getAssocSubCommand(inBufferNum);
			
			if (assocSubCmd != eCmdAssocRESP || assocSubCmd != eCmdAssocACK) {
				RELEASE_RX_BUFFER(inBufferNum, ccrHolder);
				return FALSE;
			}
		}
		return TRUE;
	}
	
	// Associated - Only process command packets for us
	if (gLocalDeviceState == eLocalStateRun) {
		cmdDstAddr = getCommandDstAddr(inBufferNum);
	
		if (cmdID != eCommandControl || cmdDstAddr != gMyAddr) {
			RELEASE_RX_BUFFER(inBufferNum, ccrHolder);
			return FALSE;
		} else {
			return TRUE;
		}
	}
	
	return TRUE;
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

void debugRefreed(BufferCntType inBufferNum) {
	gwUINT8 ccrHolder;
	
	// Any code, so that we have a place to set a breakpoint.
	//RELEASE_TX_BUFFER(inBufferNum, ccrHolder);
}


