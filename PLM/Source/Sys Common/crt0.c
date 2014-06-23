/******************************************************************************
 * Filename: crt0.c
 *
 * Description: Startup routines for ARM CORTEX-M4 processor
 *
 * Copyright (c) 2012, Freescale Semiconductor, Inc. All rights reserved.
 *
 ******************************************************************************
 *
 * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 *****************************************************************************/

#include "EmbeddedTypes.h"
#include "PortConfig.h"
#include "crt0.h"
#include "Interrupt.h"
#include "wdog.h"
#include "mcg.h"
#include "Utilities_Interface.h"
#include "vectors.h"

/******************************************************************************
 *******************************************************************************
 * Private macros
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *******************************************************************************
 * Private type definitions
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 ******************************************************************************
 * Public memory definitions
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *******************************************************************************
 * Private function prototypes
 *******************************************************************************
 ******************************************************************************/

#if gCRT0_DEBUG_TRACE_CLK_d

/******************************************************************************
 * Name: trace_clk_init
 * Description: enables the trace clock
 * Parameter(s): -
 * Return: -
 ******************************************************************************/
static void trace_clk_init
(
        void
);

/******************************************************************************
 * Name: fb_clk_init
 * Description: enables the FlexBus clock
 * Parameter(s): -
 * Return: -
 ******************************************************************************/
#if defined(__GNUC__)
static void fb_clk_init
(
        void
);
#endif	// __GNUC__
#endif /* gCRT0_DEBUG_TRACE_CLK_d */


/******************************************************************************
 ******************************************************************************
 * Public functions
 ******************************************************************************
 *****************************************************************************/
#if (defined(__IAR_SYSTEMS_ICC__))

#if STACK_FILL
extern uint32_t __size_cstack__[];
#endif

uint32_t __low_level_init(void)
{ 
  uint8_t *pStack = (uint8_t*)__BOOT_STACK_ADDRESS;
  pStack -= 20; // do not fill the first bytes of the stack
  
  SCB_VTOR = (uint32_t)__vector_table; /* Set the interrupt vector table position */   
  
  /*
  * Enable all of the port clocks. These have to be enabled to configure
  * pin muxing options, so most code will need all of these on anyway.
  */
  SIM_SCGC5 |= (  SIM_SCGC5_PORTA_MASK
                | SIM_SCGC5_PORTB_MASK
                | SIM_SCGC5_PORTC_MASK
                | SIM_SCGC5_PORTD_MASK
                | SIM_SCGC5_PORTE_MASK );  
  
  /*
  Disable the Watchdog because it may reset the core before entering main().
  There are 2 unlock words which shall be provided in sequence before
  accessing the control register.
  */
  *(volatile unsigned short *)gCRT0_WDOG_UnlockAddr_c = gCRT0_WDOG_UnlockSeq1_c;
  *(volatile unsigned short *)gCRT0_WDOG_UnlockAddr_c = gCRT0_WDOG_UnlockSeq2_c;
  *(volatile unsigned short *)gCRT0_WDOG_STCTRLH_Addr_c = gCRT0_WDOG_Disabled_Ctrl_c;
  
#if STACK_FILL

  while (pStack > (uint8_t*)((uint32_t)__BOOT_STACK_ADDRESS - (uint32_t)__size_cstack__))
  {
    *pStack-- = STACK_INIT_VALUE;
  }
#endif
  
  /* For debugging purposes, enable the trace clock and/or FB_CLK so that
  * we'll be able to monitor clocks and know the PLL is at the frequency
  * that we expect.
  */
  
#if gCRT0_DEBUG_TRACE_CLK_d    
  trace_clk_init();
#endif
  
/* enable clock to LLWU module */
#if (MCU_MK60N512VMD100 == 1)	
  SIM_SCGC4 |= SIM_SCGC4_LLWU_MASK;
#elif (MCU_MK60D10 == 1)
  SIM_SCGC4 |= (uint32_t) (1 << 28);
#endif

#if (MCU_MK60N512VMD100 == 1)	  
  if (LLWU_CS & LLWU_CS_ACKISO_MASK)
  {
    LLWU_CS |= LLWU_CS_ACKISO_MASK;
  }
#elif ((MCU_MK60D10 == 1) || (MCU_MK21DN512 == 1) || (MCU_MK21DX256 == 1))
  if (PMC_REGSC & PMC_REGSC_ACKISO_MASK)
  {
    PMC_REGSC |= PMC_REGSC_ACKISO_MASK;
  }
#endif
  
  return 1;
}
#endif

/******************************************************************************
 * Name: __init_hardware
 * Description: performs the hardware initialization of system 
 * Parameters: -
 * Return: -
 ******************************************************************************/
#if (defined(__GNUC__))

#if SUPPORT_SEMIHOST_ARGC_ARGV
	#define __MAX_CMDLINE_ARGS 10
	static char *argv[__MAX_CMDLINE_ARGS] = { 0 };
#else
	static char *argv[] = { 0 };
#endif

#if SUPPORT_SEMIHOST_ARGC_ARGV
	extern int __argc_argv(char **, int);		// SUPPORT_SEMIHOST_ARGC_ARGV
#endif  

extern int main(int, char **);

#ifdef __VFPV4__
	extern void __fp_init(void);				// __VFPV4__ 
#endif 

extern void __init_registers();

extern void __init_hardware();

extern void __init_user();

#if defined(__APCS_ROPI)
	extern void __init_pic();
#endif

#if defined(__APCS_RWPI)
	extern void __init_pid();
#endif

#if defined(__APCS_ROPI) || defined(__APCS_RWPI)
	extern void __load_static_base();
#endif

#if defined(__SEMIHOSTING)
	extern void __init_semihost(void) _EWL_WEAK;
	extern void sys_exit(int status);
#endif

	extern void __copy_rom_sections_to_ram(void);
	extern char __S_romp[];

static void zero_fill_bss(void)
{
	extern char __START_BSS[];
	extern char __END_BSS[];

	MemorySet(__START_BSS, 0, (__END_BSS - __START_BSS));	
}

extern void _ExitProcess(int status) __attribute__ ((naked));		// _ExitProcess
extern void _ExitProcess(int status)
{
#if defined(__SEMIHOSTING)
	sys_exit(status);
#endif

	while (1);
}

#if STACK_FILL
extern uint32_t __stack_size[];
#endif

void __init_hardware()
{

	  uint8_t *pStack = (uint8_t*)__SP_INIT;
	  pStack -= 20; // do not fill the first bytes of the stack
	  
	/* Set the interrupt vector table position */
	SCB_VTOR = (uint32_t)__vector_table; 

	/*
        Disable the Watchdog because it may reset the core before entering main().
        There are 2 unlock words which shall be provided in sequence before
        accessing the control register.
	 */
	*(volatile unsigned short *)gCRT0_WDOG_UnlockAddr_c = gCRT0_WDOG_UnlockSeq1_c;
	*(volatile unsigned short *)gCRT0_WDOG_UnlockAddr_c = gCRT0_WDOG_UnlockSeq2_c;
	*(volatile unsigned short *)gCRT0_WDOG_STCTRLH_Addr_c = gCRT0_WDOG_Disabled_Ctrl_c;

	/*
	 * Enable all of the port clocks. These have to be enabled to configure
	 * pin muxing options, so most code will need all of these on anyway.
	 */
	SIM_SCGC5 |= (SIM_SCGC5_PORTA_MASK
			| SIM_SCGC5_PORTB_MASK
			| SIM_SCGC5_PORTC_MASK
			| SIM_SCGC5_PORTD_MASK
			| SIM_SCGC5_PORTE_MASK );

#if STACK_FILL
  while (pStack > (uint8_t*)((uint32_t)__SP_INIT - (uint32_t)__stack_size))
  {
    *pStack-- = STACK_INIT_VALUE;
  }
#endif	
	
#if gCRT0_DEBUG_TRACE_CLK_d    
	trace_clk_init();
	fb_clk_init();
#endif // gCRT0_DEBUG_TRACE_CLK_d
	
	/* enable clock to LLWU module */
	#if (MCU_MK60N512VMD100 == 1)	
	  SIM_SCGC4 |= SIM_SCGC4_LLWU_MASK;
	#elif (MCU_MK60D10 == 1)
	  SIM_SCGC4 |= (uint32_t) (1 << 28);
	#endif

	#if (MCU_MK60N512VMD100 == 1)	  
	  if (LLWU_CS & LLWU_CS_ACKISO_MASK)
	  {
	    LLWU_CS |= LLWU_CS_ACKISO_MASK;
	  }
	#elif ((MCU_MK60D10 == 1) || (MCU_MK21DN512 == 1) || (MCU_MK21DX256 == 1))
	  if (PMC_REGSC & PMC_REGSC_ACKISO_MASK)
	  {
	    PMC_REGSC |= PMC_REGSC_ACKISO_MASK;
	  }
	#endif	
}



void __thumb_startup(void) __attribute__ ((naked));
void __thumb_startup(void)
{
		__init_registers();				// Setup registers
		
		__init_hardware();				// setup hardware

#if defined(__APCS_ROPI) || defined(__APCS_RWPI)
		__load_static_base();			//	static base register initialization
#endif

#if defined(__APCS_RWPI)
		__init_pid();					//	-pid
										//	setup static base for SB relative position independent data
										//	perform runtime relocation
#endif

#if defined(__APCS_ROPI)
		__init_pic();					//	-pic
										//	perform runtime relocation for position independent code
#endif

		zero_fill_bss();				//	zero-fill the .bss section

		if (__S_romp != 0L)
			__copy_rom_sections_to_ram();

#ifdef __VFPV4__
		__fp_init();					//      initialize the floating-point library
#endif

		__init_user();					// initializations before main, user specific

#if defined(__SEMIHOSTING)
		__init_semihost();				// semihost initializations
#endif

#if SUPPORT_SEMIHOST_ARGC_ARGV
		exit(main(__argc_argv(argv, __MAX_CMDLINE_ARGS), argv));
#else
//		exit(main(0, argv));			//	call main(argc, &argv)
		main(0, argv);					//	call main(argc, &argv)		
#endif

		while (1);						//	should never get here
}


#endif // __GNUC__



/******************************************************************************
 *******************************************************************************
 * Private functions
 *******************************************************************************
 ******************************************************************************/

#if gCRT0_DEBUG_TRACE_CLK_d

/******************************************************************************
 * Name: trace_clk_init
 * Description: enables the trace clock
 * Parameter(s): -
 * Return: -
 ******************************************************************************/
static void trace_clk_init(void)
{
#if (defined(MCU_MK60N512VMD100))
    /* Set the trace clock to the core clock frequency */
    SIM_SOPT2 |= SIM_SOPT2_TRACECLKSEL_MASK;

    /* Enable the TRACE_CLKOUT pin function on PTA6 (alt7 function) */
    PORTA_PCR6 = ( PORT_PCR_MUX(0x7));
#endif
}

/******************************************************************************
 * Name: fb_clk_init
 * Description: enables the FlexBus clock
 * Parameter(s): -
 * Return: -
 ******************************************************************************/
#if defined(__GNUC__)
static void fb_clk_init(void)
{
#if (defined(MCU_MK60N512VMD100))
    /* Enable the clock to the FlexBus module */
    SIM_SCGC7 |= SIM_SCGC7_FLEXBUS_MASK;

    /* Enable the FB_CLKOUT function on PTC3 (alt5 function) */
    PORTC_PCR3 = ( PORT_PCR_MUX(0x5));
#endif
}
#endif	// __GNUC__
/********************************************************************/

#endif /* gCRT0_DEBUG_TRACE_CLK_d */

