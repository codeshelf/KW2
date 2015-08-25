/*
   FlyWeight
   © Copyright 2005, 2006 Jeffrey B. Williams
   All rights reserved

   $Id$
   $Name$
*/

#ifndef PROCESS_RX_H
#define PROCESS_RX_H

#include "radioCommon.h"
#ifndef GATEWAY
void processRxPacket(BufferCntType inRxBufferNum, uint8_t lqi);
#endif
#endif // PROCESS_RX_H
