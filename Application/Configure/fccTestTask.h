/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#ifndef FCC_TEST_TASK_H
#define FCC_TEST_TASK_H

#include "gwTypes.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"
#include "TMR_Interface.h"

#define FCC_TEST_TASK_QUEUE_SIZE		2

#define LED_OFF_TIME					500
#define LED_ON_TIME						250

#define SSI_24BIT_WORD					0x0b
#define SSI_20BIT_WORD					0x09
#define SSI_8BIT_WORD					0x03
#define SSI_FRAME_LEN2					0x01
#define SSI_FRAME_LEN7					0x06

typedef union {
	gwUINT32 word;
	struct {
		gwUINT8 byte3;
		gwUINT8 byte2;
		gwUINT8 byte1;
		gwUINT8 byte0;
	} bytes;
} ULedSampleType;

// --------------------------------------------------------------------------
// Local functions prototypes.

void clearAllPositions();
void fccSetupTask(void *pvParameters);
void fccTxTask(void *pvParameters);
void displayState();
void setChannel(ScanStringType scanString);

#endif //FCC_TEST_TASK_H
