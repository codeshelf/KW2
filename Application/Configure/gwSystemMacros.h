/*
 * gwSystemMacros.h
 *
 *  Created on: Mar 10, 2009
 *      Author: jeffw
 */

#ifndef GWSYSTEMMACROS_H_
#define GWSYSTEMMACROS_H_

#include "portmacro.h"
#include "Watchdog.h"
#include "Critical.h"
#include "SMAC_Interface.h"
#include "IO_Map.h"
#include "globals.h"

// Define this to debug RX/X packet handing, COP/Watchdog and MCU_RESET
#define GW_DEBUG							TRUE

	#if (GW_DEBUG) 
		#define GW_RESET_MCU(cause) \
			do { \
				gRestartCause = cause; \
				debugReset(); \
			} while(0);
	#else
		#define GW_RESET_MCU(cause)	\
			do { \
				gRestartCause = cause; \
				 CRM_SoftReset(); \
			} while(0);
	#endif
		

#define GW_ENTER_CRITICAL(saveState)		Critical_EnterCritical(saveState);
#define GW_EXIT_CRITICAL(restoreState)		Critical_ExitCritical(restoreState);

#define GW_GET_SYSTEM_STATUS				RCM_SRS0;

#define	GW_WATCHDOG_RESET					Watchdog_Clear(Watchdog_DeviceData);

#define GW_ENERGY_DETECT(channel)			0

#define XSTR(x)								#x
#define STR(x)								XSTR(x)

#endif /* GWSYSTEMMACROS_H_ */
