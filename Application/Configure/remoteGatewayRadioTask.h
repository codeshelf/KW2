/*
 * radioGatewayTask.h
 *
 *  Created on: Feb 18, 2016
 *      Author: andrewhuff
 */

#ifndef APPLICATION_CONFIGURE_REMOTEGATEWAYRADIOTASK_H_
#define APPLICATION_CONFIGURE_REMOTEGATEWAYRADIOTASK_H_

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

void radioGatewayReceiveTask( void *pvParameters );
void radioGatewayTransmitTask( void *pvParameters );

#endif /* APPLICATION_CONFIGURE_REMOTEGATEWAYRADIOTASK_H_ */
