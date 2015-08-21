/*
 FlyWeight
 © Copyright 2005, 2006 Jeffrey B. Williams
 All rights reserved

 $Id$
 $Name$
 */

#ifndef SCANREADTASK_H
#define SCANREADTASK_H

#include "FreeRTOS.h"
#include "radioCommon.h"

// --------------------------------------------------------------------------
// Local functions prototypes.

void scannerReadTask(void *pvParameters);
void sendLineToScanner(DisplayStringType inString, DisplayStringLenType inLen);
void clearScannerDisplay();

#endif SCANREADTASK_H
