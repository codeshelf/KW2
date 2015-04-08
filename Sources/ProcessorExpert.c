/* ###################################################################
 **     Filename    : ProcessorExpert.c
 **     Project     : ProcessorExpert
 **     Processor   : MK21DX256VMC5
 **     Version     : Driver 01.01
 **     Compiler    : GNU C Compiler
 **     Date/Time   : 2014-03-24, 15:29, # CodeGen: 0
 **     Abstract    :
 **         Main module.
 **         This module contains user's application code.
 **     Settings    :
 **     Contents    :
 **         No public methods
 **
 ** ###################################################################*/
/*!
 ** @file ProcessorExpert.c
 ** @version 01.01
 ** @brief
 **         Main module.
 **         This module contains user's application code.
 */
/*!
 **  @addtogroup ProcessorExpert_module ProcessorExpert module documentation
 **  @{
 */
/* MODULE ProcessorExpert */
/* Including needed modules to compile this module/procedure */
#include "Cpu.h"
#include "Events.h"
#include "FRTOS1.h"
#include "UTIL1.h"
#include "UTIL2.h"
#include "INT_PORTB.h"
#include "Wait.h"
#include "Critical.h"
#include "Watchdog.h"
<<<<<<< Upstream, based on origin/master
#include "EventTimer.h"
#include "LedDrive.h"
=======
#include "Tuner.h"
>>>>>>> 2ccb9fc * Get the tuner project FTM0 basis to work. (More to do.) * Merge PE changes from Saba into my project changes.
#include "EepromCS.h"
#include "BitIoLdd2.h"
#include "StatusLedClk.h"
#include "StatusLedSdi.h"
#include "ScannerPower.h"
#include "Rs485Power.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
/* User includes (#include below this line is not maintained by Processor Expert) */
void startApplication(void);

PE_ISR(Rs485Isr) {	
}

PE_ISR(Rs485ErrorIsr) {
}

PE_ISR(ScannerIsr) {	
}

PE_ISR(ScannerErrorIsr) {	
}

PE_ISR(LcdIsr) {	
}

PE_ISR(LcdErrorIsr) {	
}

/*lint -restore Enable MISRA rule (6.3) checking. */
int main(void) {
	/* Write your local variable definition here */

	/*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
	PE_low_level_init();
	/*** End of Processor Expert internal initialization.                    ***/

	/* Write your code here */
	startApplication();
	
	/*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END ProcessorExpert */
/*!
 ** @}
 */
/*
 ** ###################################################################
 **
 **     This file was created by Processor Expert 10.3 [05.08]
 **     for the Freescale Kinetis series of microcontrollers.
 **
 ** ###################################################################
 */
