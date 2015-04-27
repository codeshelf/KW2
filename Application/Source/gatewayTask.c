/*
FlyWeight
 Copyright 2005, 2006 Jeffrey B. Williams
All rights reserved

$Id$
$Name$	
*/

#include "gatewayTask.h"
#include "remoteRadioTask.h"
#include "commands.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "gwSystemMacros.h"
#include "gwTypes.h"
#include "commands.h"
#include "serial.h"
#include "string.h"
#include "Gateway.h"

xQueueHandle			gGatewayMgmtQueue;
ControllerStateType		gControllerState;

xTaskHandle gSerialReceiveTask = NULL;
extern xQueueHandle	gRadioTransmitQueue;

extern RxRadioBufferStruct gRxRadioBuffer[RX_BUFFER_COUNT];
extern TxRadioBufferStruct gTxRadioBuffer[TX_BUFFER_COUNT];

// --------------------------------------------------------------------------

void serialReceiveTask(void *pvParameters ) {

	ECommandGroupIDType		cmdID;
	ENetMgmtSubCmdIDType	subCmdID;
	BufferCntType			txBufferNum = 0;
	
	// Setup the USB interface.
	Gateway_Init();
	
	/* PORTD_PCR6: ISF=0,MUX=3 */
	PORTD_PCR6 = (uint32_t)((PORTD_PCR6 & (uint32_t)~(uint32_t)(
				PORT_PCR_ISF_MASK |
				PORT_PCR_MUX(0x04)
			   )) | (uint32_t)(
				PORT_PCR_MUX(0x03)
			   ));
	/* PORTD_PCR7: ISF=0,MUX=3 */
	PORTD_PCR7 = (uint32_t)((PORTD_PCR7 & (uint32_t)~(uint32_t)(
				PORT_PCR_ISF_MASK |
				PORT_PCR_MUX(0x04)
			   )) | (uint32_t)(
				PORT_PCR_MUX(0x03)
			   ));
	/* PORTD_PCR5: ISF=0,MUX=3 */
	PORTD_PCR5 = (uint32_t)((PORTD_PCR5 & (uint32_t)~(uint32_t)(
				PORT_PCR_ISF_MASK |
				PORT_PCR_MUX(0x04)
			   )) | (uint32_t)(
				PORT_PCR_MUX(0x03)
			   ));
	/* PORTD_PCR4: ISF=0,MUX=3 */
	PORTD_PCR4 = (uint32_t)((PORTD_PCR4 & (uint32_t)~(uint32_t)(
				PORT_PCR_ISF_MASK |
				PORT_PCR_MUX(0x04)
			   )) | (uint32_t)(
				PORT_PCR_MUX(0x03)
			   ));

	// Send a net-setup command to the controller.
	// It will respond with the channel that we should be using.
	createOutboundNetSetup();
	
	for ( ;; ) {
		while (gTxRadioBuffer[txBufferNum].bufferStatus == eBufferStateInUse) {
			vTaskDelay(0);
		}
//		serialTransmitFrame(UART0_BASE_PTR, (BufferStoragePtrType) ("IN"),  2);
		
		gTxRadioBuffer[txBufferNum].bufferSize = serialReceiveFrame(UART0_BASE_PTR, gTxRadioBuffer[txBufferNum].bufferStorage, TX_BUFFER_SIZE);

//		serialTransmitFrame(UART0_BASE_PTR, (BufferStoragePtrType) ("OU"),  2);

		GW_WATCHDOG_RESET;
		
		if (gTxRadioBuffer[txBufferNum].bufferSize > 0) {
			gTxRadioBuffer[txBufferNum].bufferStatus = eBufferStateInUse;

		  	cmdID = getCommandID(gTxRadioBuffer[txBufferNum].bufferStorage);
			if (cmdID == eCommandNetMgmt) {
				subCmdID = getNetMgmtSubCommand(gTxRadioBuffer[txBufferNum].bufferStorage);
				switch (subCmdID) {
					case eNetMgmtSubCmdNetCheck:
						processNetCheckOutboundCommand(txBufferNum);
						break;
						
					case eNetMgmtSubCmdNetSetup:
						processNetSetupCommand(txBufferNum);
						// We don't ever want to broadcast net-setup, so continue.
						gTxRadioBuffer[txBufferNum].bufferStatus = eBufferStateFree;
						continue;
						break;
						
					case eNetMgmtSubCmdNetIntfTest:
						processNetIntfTestCommand(txBufferNum);
						// We don't ever want to broadcast intf-test, so continue.
						gTxRadioBuffer[txBufferNum].bufferStatus = eBufferStateFree;
						continue;
						break;
						
					case eNetMgmtSubCmdInvalid:
						gTxRadioBuffer[txBufferNum].bufferStatus = eBufferStateFree;
						continue;
				}
			}

//			serialTransmitFrame(UART0_BASE_PTR, (BufferStoragePtrType) ("SN"),  2);

			// Now send the buffer to the transmit queue.
			if (xQueueSend(gRadioTransmitQueue, &txBufferNum, (portTickType) 0) != pdTRUE) {
				// We couldn't queue the buffer, so release it.
				gTxRadioBuffer[txBufferNum].bufferStatus = eBufferStateFree;
			}

		}
	}

	/* Will only get here if the queue could not be created. */
	for ( ;; );
}
