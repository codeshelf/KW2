/*
 Codeshelf
 © Copyright 2005, 2014 Codeshelf, Inc.
 All rights reserved

 $Id$
 $Name$
 */

#ifndef AISLE_CONTROLLER_TASK_H
#define AISLE_CONTROLLER_TASK_H

#include "gwTypes.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "radioCommon.h"

#define AISLE_CONTROLLER_QUEUE_SIZE		2

#define LED_OFF_TIME					500
#define LED_ON_TIME						250

#define	MAX_DRIFT						500

extern xQueueHandle gAisleControllerQueue;

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

void setupUart();
void setupSsi();
void aisleControllerTask(void *pvParameters);
void ssiInterrupt(void);
gwUINT32 getNextSolidData(void);
gwUINT32 getNextFlashData(void);
LedDataStruct getNextDataStruct(void);

#endif //AISLECONTROLLER_TASK_H
