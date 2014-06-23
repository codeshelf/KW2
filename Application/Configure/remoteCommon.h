/*
   FlyWeight
   © Copyright 2005, 2006 Jeffrey B. Williams
   All rights reserved

   $Id$
   $Name$
*/

#ifndef REMOTECOMMON_H
#define REMOTECOMMON_H

// Project includes
#include "smacGlue.h"
#include "commandTypes.h"
#include "gwTypes.h"
#include "gwSystemMacros.h"
#include "radioCommon.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

void processRxPacket(BufferCntType inRxBufferNum);

#endif // REMOTECOMMON_H
