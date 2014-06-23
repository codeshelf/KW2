/******************************************************************************
 * Filename: vectors.c
 *
 * Description: Interrupt vector table for ARM CORTEX-M4 processor
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

#include "PortConfig.h"
#include "vectors.h"
#include "mcg.h"

/******************************************************************************
 ******************************************************************************
 * User Interfaces
 ******************************************************************************
 ******************************************************************************/

#include "TMR_Interface.h"
#include "Utilities_Interface.h"

#if defined(__IAR_SYSTEMS_ICC__)
#include "intrinsics.h"
#endif

#if defined(__IAR_SYSTEMS_ICC__)
extern void __startup(void);
#else
extern void __thumb_startup(void);
#endif

extern void PHY_InterruptHandler( void );
extern void Switch_Press_ISR(void);
#if gUsbIncluded_d
extern void USB_ISR(void);
#endif
/******************************************************************************
 ******************************************************************************
 * Public functions
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 * Name: VECT_NMIInterrupt
 * Description: NMI interrupt handler 
 * Parameter(s): -
 * Return: -
 ******************************************************************************/ 
void VECT_NMIInterrupt
(
		void
) 
{	 
	/* EMPTY IMPLEMENTATION */
}

/******************************************************************************
 * Name: VECT_DefaultISR
 * Description: default CPU interrupt handler 
 * Parameter(s): -
 * Return: -
 ******************************************************************************/
void VECT_DefaultISR
(
		void
) 
{
#ifdef NDEBUG
  asm("loop: BL loop");
#else
  asm ("BKPT 255");
#endif
}

/******************************************************************************
 * Name: VECT_HWfaultISR
 * Description: hardware fault interrupt handler 
 * Parameter(s): -
 * Return: -
 ******************************************************************************/
void VECT_HWfaultISR
(
		void
) 
{   
#ifdef NDEBUG
  asm("loop: BL loop");
#else
  asm ("BKPT 255");
#endif
}

#if defined(__IAR_SYSTEMS_ICC__)
#pragma location = ".intvec"
const pfVectorEntry_t  __vector_table[] = 	
{
	(pfVectorEntry_t)__BOOT_STACK_ADDRESS,	/*	0x0000_0000	0		ARM core Initial Stack Pointer	*/
	(pfVectorEntry_t)__iar_program_start,				/*	0x0000_0004	1		ARM core Initial Program Counter	*/
#elif defined(__GNUC__)
	const pfVectorEntry_t __vector_table[] __attribute__ ((section(".vectortable"))) =
	{
	  (pfVectorEntry_t)__SP_INIT,                     /* 0x00  0x00000000   -   ivINT_Initial_Stack_Pointer   */
	  (pfVectorEntry_t)__thumb_startup,         /* 0x01  0x00000004   -   ivINT_Initial_Program_Counter  */
#endif
	
/*********************************** MK21DN512 / MK21DX256 ***********************************************/
#if defined (MCU_MK21DN512) || defined (MCU_MK21DX256)
  (pfVectorEntry_t)&VECT_NMIInterrupt,  /*  0x0000_0008  2    ARM core Non-maskable Interrupt (NMI)  */
  (pfVectorEntry_t)&VECT_HWfaultISR,  /*  0x0000_000C  3    ARM core Hard Fault  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0010  4    ARM core MemManage Fault  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0014  5    ARM core Bus Fault  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0018  6    ARM core Usage Fault  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_001C  7      */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0020  8      */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0024  9      */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0028  10      */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_002C  11    ARM core Supervisor call (SVCall)  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0030  12    ARM core Debug Monitor  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0034  13      */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0038  14    ARM core Pendable request for system service  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_003C  15    ARM core System tick timer (SysTick)  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0040  16  0  DMA DMA channel 0 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0044  17  1  DMA DMA channel 1 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0048  18  2  DMA DMA channel 2 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_004C  19  3  DMA DMA channel 3 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0050  20  4  DMA DMA channel 4 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0054  21  5  DMA DMA channel 5 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0058  22  6  DMA DMA channel 6 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_005C  23  7  DMA DMA channel 7 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0060  24  8  DMA DMA channel 8 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0064  25  9  DMA DMA channel 9 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0068  26  10  DMA DMA channel 10 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_006C  27  11  DMA DMA channel 11 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0070  28  12  DMA DMA channel 12 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0074  29  13  DMA DMA channel 13 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0078  30  14  DMA DMA channel 14 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_007C  31  15  DMA DMA channel 15 transfer complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0080  32  16  DMA DMA error interrupt channels 0-15  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0084  33  17  MCM   */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0088  34  18  Flash memory Command complete  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_008C  35  19  Flash memory Read collision  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0090  36  20  Mode Controller Low-voltage detect  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0094  37  21  LLWU Low Leakage Wakeup  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0098  38  22  WDOG   */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_009C  39  23  RNG Randon Number Generator  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00A0  40  24  I2C0   */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00A4  41  25  I2C1   */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00A8  42  26  SPI0 Single interrupt vector for all sources  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00AC  43  27  SPI1 Single interrupt vector for all sources  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00B0  44  28  I2S0 Transmit  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00B4  45  29  I2S0 Receive  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00B8  46  30  UART0 Single interrupt vector for UART LON  */
#if (gUart1_Enabled_d == TRUE) && (gUart1_c == 0)
  (pfVectorEntry_t)Uart1Isr,        /*  0x0000_00BC  47  31  UART0 Single interrupt vector for UART status  */
  (pfVectorEntry_t)Uart1ErrorIsr,   /*  0x0000_00C0  48  32  UART0 Single interrupt vector for UART error  */
#elif (gUart2_Enabled_d == TRUE) && (gUart2_c == 0)
  (pfVectorEntry_t)Uart2Isr,        /*  0x0000_00BC  47  31  UART0 Single interrupt vector for UART status  */
  (pfVectorEntry_t)Uart2ErrorIsr,   /*  0x0000_00C0  48  32  UART0 Single interrupt vector for UART error  */
#else
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00BC  47  31  UART0 Single interrupt vector for UART status  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00C0  48  32  UART0 Single interrupt vector for UART error  */
#endif
#if (gUart1_Enabled_d == TRUE) && (gUart1_c == 1)
  (pfVectorEntry_t)Uart1Isr,        /*  0x0000_00C4  49  33  UART1 Single interrupt vector for UART status  */
  (pfVectorEntry_t)Uart1ErrorIsr,   /*  0x0000_00C8  50  34  UART1 Single interrupt vector for UART error  */
#elif (gUart2_Enabled_d == TRUE) && (gUart2_c == 1)
  (pfVectorEntry_t)Uart2Isr,        /*  0x0000_00C4  49  33  UART1 Single interrupt vector for UART status  */
  (pfVectorEntry_t)Uart2ErrorIsr,   /*  0x0000_00C8  50  34  UART1 Single interrupt vector for UART error  */
#else
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00C4  49  33  UART1 Single interrupt vector for UART status  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00C8  50  34  UART1 Single interrupt vector for UART error  */
#endif
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00CC  51  35  UART2 Single interrupt vector for UART status  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00D0  52  36  UART2 Single interrupt vector for UART error  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00D4  53  37  UART3 Single interrupt vector for UART status  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00D8  54  38  UART3 Single interrupt vector for UART error  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00DC  55  39  ADC0   */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00E0  56  40  CMP0   */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00E4  57  41  CMP1   */
#if (gTMR_Enabled_d == TRUE)
  (pfVectorEntry_t)TMR_InterruptHandler, /*  0x0000_00E8  58  42  FTM0 Single interrupt vector for all sources  */
#else
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00E8  58  42  FTM0 Single interrupt vector for all sources  */
#endif
#if (gTMR2_Enabled == TRUE)
  (pfVectorEntry_t)TMR2_InterruptHandler,  /*  0x0000_00EC  59  43  FTM1 Single interrupt vector for all sources  */
#else
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00EC  59  43  FTM1 Single interrupt vector for all sources  */
#endif
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00F0  60  44  FTM2 Single interrupt vector for all sources  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00F4  61  45  CMT   */
#if (gLpmIncluded_d == 1)
  (pfVectorEntry_t)PWRLib_RTC_Isr, /*  0x0000_00F8  62  46  RTC Alarm interrupt  */
#else
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00F8  62  46  RTC Alarm interrupt  */
#endif
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_00FC  63  47  RTC Seconds interrupt  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0100  64  48  PIT Channel 0  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0104  65  49  PIT Channel 1  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0108  66  50  PIT Channel 2  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_010C  67  51  PIT Channel 3  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0110  68  52  PDB   */
#if gUsbIncluded_d
  (pfVectorEntry_t)USB_ISR,          /*  0x0000_0114  69  53*/
#else
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0114  69  53  USB OTG   */
#endif
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0118  70  54  USB Charger  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_011C  71  55     */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0120  72  56  DAC0   */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0124  73  57  MCG   */
#if (gLpmIncluded_d == 1)
  (pfVectorEntry_t)PWRLib_LPTMR_Isr,  /*  0x0000_0128  74  58  Low Power Timer   */
#else
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0128  74  58  Low Power Timer   */
#endif
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_012C  75  59  Port control module Pin detect (Port A)  */
  (pfVectorEntry_t)PHY_InterruptHandler, /*  0x0000_0130  76  60  Port control module Pin detect (Port B)  */
  (pfVectorEntry_t)Switch_Press_ISR, /*  0x0000_0134  77  61  Port control module Pin detect (Port C)  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0138  78  62  Port control module Pin detect (Port D)  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_013C  79  63  Port control module Pin detect (Port E)  */
  (pfVectorEntry_t)VECT_DefaultISR,  /*  0x0000_0140  80  64  Software Software interrupt4  */
	};

#endif /* MCU_MK21DN512 */
        
