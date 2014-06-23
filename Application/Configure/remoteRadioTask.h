/*
   FlyWeight
   © Copyright 2005, 2006 Jeffrey B. Williams
   All rights reserved
   
   $Id$
   $Name$	
*/

#ifndef REMOTERADIOTASK_H
#define REMOTERADIOTASK_H

// Project includes
#include "radioCommon.h"

// --------------------------------------------------------------------------
// Externs

// --------------------------------------------------------------------------
// Defines

#define		kAssocCheckTickCount	6000 // 6 seconds
#define		kNetCheckTickCount		5 * kAssocCheckTickCount  // five ACK packets missed then reset - 30sec.

// --------------------------------------------------------------------------
// Functions prototypes.

void radioReceiveTask( void *pvParameters );
void radioTransmitTask( void *pvParameters );

#endif // REMOTERADIOTASK_H

