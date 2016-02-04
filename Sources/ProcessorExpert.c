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
//#include "Rs485.h"
#include "Critical.h"
//#include "Scanner.h"
#include "Wait.h"
#include "Watchdog.h"
#include "EventTimer.h"
#include "SharpDisplayCS.h"
#include "BitIoLdd1.h"
//#include "SharpDisplay.h"
#ifdef TUNER
#endif
#include "EepromCS.h"
#include "BitIoLdd2.h"
#include "StatusLedClk.h"
#include "StatusLedSdi.h"
#include "CRC1.h"
/* Including shared modules, which are used for whole project */
#include "PE_Types.h"
#include "PE_Error.h"
#include "PE_Const.h"
#include "IO_Map.h"
#include "PDD_Includes.h"
#include "Init_Config.h"
/* User includes (#include below this line is not maintained by Processor Expert) */
#include "globals.h"
#include "eeprom.h"
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

	uint32_t ptc4_mux = (uint32_t) PORTC_PCR4 >> PORT_PCR_MUX_SHIFT;
	uint32_t ptc5_mux = (uint32_t) PORTC_PCR5 >> PORT_PCR_MUX_SHIFT;
	uint32_t ptc6_mux = (uint32_t) PORTC_PCR6 >> PORT_PCR_MUX_SHIFT;
	uint32_t ptc7_mux = (uint32_t) PORTC_PCR7 >> PORT_PCR_MUX_SHIFT;
	uint32_t pddr = GPIOC_PDDR;
	
	PORTC_PCR4 = PORT_PCR_MUX(1);
	PORTC_PCR5 = PORT_PCR_MUX(2);
	PORTC_PCR6 = PORT_PCR_MUX(2);
	PORTC_PCR7 = PORT_PCR_MUX(2);
	GPIOC_PDDR = GPIO_PDDR_PDD(10);
	
	/* Write your code here */
	// Load GUID
	readGuid(gGuid);
	gGuid[8] = NULL;
	
	// Load hardware version
	readHWVersion(gHwVer);
	
	// Load aes key
	readAESKey(gAesKey);
	
	// Load trim
	readTuning(gTrim);
	
	PORTC_PCR4 = PORT_PCR_MUX(ptc4_mux);
	PORTC_PCR5 = PORT_PCR_MUX(ptc5_mux);
	PORTC_PCR6 = PORT_PCR_MUX(ptc6_mux);
	PORTC_PCR7 = PORT_PCR_MUX(ptc7_mux);
	GPIOC_PDDR = GPIO_PDDR_PDD(pddr);
	
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
