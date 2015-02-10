/*
	FlyWeight
	© Copyright 2005, 2006 Jeffrey B. Williams
	All rights reserved

	$Id$
	$Name$
*/

#include "smacGlue.h"
#include "radioCommon.h"
#include "gwSystemMacros.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "gwTypes.h"
#include "SMAC.h"

gwSINT8						gNoReceive = -1;

extern xQueueHandle			gRadioReceiveQueue;
extern BufferCntType		gRxCurBufferNum;
extern gwBoolean			gIsReceiving;
extern gwBoolean			gIsTransmitting;
extern xQueueHandle gRemoteMgmtQueue;
extern ELocalStatusType gLocalDeviceState;

extern ERxMessageHolderType gRxMsg;
extern ETxMessageHolderType gTxMsg;

// --------------------------------------------------------------------------
// The SMAC calls MCPSDataIndication() during an ISR when it receives data from the radio.
// We intercept that here, and patch the message over to the radio task's queue.
// In FreeRTOS we can't RTI or task switch during and ISR, so we need to keep this
// short and sweet.  (And let a swapped-in task deal with this during a context switch.)

void MCPSDataIndication(rxPacket_t *inRxPacket) {
	gIsReceiving = FALSE;

	if (inRxPacket->rxStatus == rxSuccessStatus_c) {

		GW_WATCHDOG_RESET;
		
		// If we haven't initialized the radio receive queue then cause a debug trap.
		if (gRadioReceiveQueue == NULL)
			GW_RESET_MCU();

		// Send the message to the radio task's queue.
		if (gLocalDeviceState == eLocalStateRun) {
			xQueueSendFromISR(gRadioReceiveQueue, &gRxMsg.bufferNum, (portBASE_TYPE) 0);
		} else {
			xQueueSendFromISR(gRemoteMgmtQueue, &gRxMsg.bufferNum, (portBASE_TYPE) 0);
		}

		//advanceRxBuffer();

	} else {
		// Send the message to the radio task's queue that we didn't get any packets before timing out.
		if (xQueueSendFromISR(gRadioReceiveQueue, &gNoReceive, (portBASE_TYPE) 0)) {
		}
	}
}

void MCPSDataConfirm(txStatus_t txStatus) {
	gIsTransmitting = FALSE;
	gTxMsg.txStatus = txStatus;
	
	// We're done transmitting the packet, so resume the RX mode.
	//resumeRadioRx();
}


void MLMEScanConfirm(channels_t channels) {
}

void MLMEResetIndication(void) {
}

void MLMEWakeConfirm(void) {
}
