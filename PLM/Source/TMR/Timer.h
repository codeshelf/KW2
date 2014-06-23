/******************************************************************************
 * Public header file for the Timer driver.
 * 
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2011 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
*******************************************************************************
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
***************************************************************************//*!
******************************************************************************/

 #ifndef __TIMER_H__
 #define __TIMER_H__
 
/******************************************************************************
*******************************************************************************
* Includes
*******************************************************************************
******************************************************************************/

#include "TMR_Interface.h"
#include "EmbeddedTypes.h"

/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/ 
/* SOFTWARE RELATED SECTION */
 
/******************************************************************************
 * NAME: NumberOfElements()
 * DESCRIPTION: calculates the number of elements in a given array
 *****************************************************************************/
#define NumberOfElements(array)   ((sizeof(array) / (sizeof(array[0]))))

/******************************************************************************
 * NAME: mTmrStatusFree_c
 * DESCRIPTION: self explanatory
 *****************************************************************************/
#define mTmrStatusFree_c    0x00

/******************************************************************************
 * NAME: mTmrStatusActive_c
 * DESCRIPTION: self explanatory
 *****************************************************************************/
#define mTmrStatusActive_c    0x20

/******************************************************************************
 * NAME: mTmrStatusReady_c
 * DESCRIPTION: self explanatory
 *****************************************************************************/
#define mTmrStatusReady_c    0x40

/******************************************************************************
 * NAME: mTmrStatusInactive_c
 * DESCRIPTION: self explanatory
 *****************************************************************************/
#define mTmrStatusInactive_c    0x80

/******************************************************************************
 * NAME: mTimerStatusMask_c
 * DESCRIPTION: timer status mask
 *****************************************************************************/
#define mTimerStatusMask_c      ( mTmrStatusActive_c \
| mTmrStatusReady_c \
| mTmrStatusInactive_c)

/******************************************************************************
 * NAME: TMR_IsTimerAllocated()
 * DESCRIPTION: checks if a specified timer is allocated
 *****************************************************************************/
#define TMR_IsTimerAllocated(timerID)   (maTmrTimerStatusTable[(timerID)])

/******************************************************************************
 * NAME: TMR_MarkTimerFree()
 * DESCRIPTION: marks the specified timer as free
 *****************************************************************************/
#define TMR_MarkTimerFree(timerID)       maTmrTimerStatusTable[(timerID)] = 0

/******************************************************************************
 * NAME: IsLowPowerTimer()
 * DESCRIPTION: Detect if the timer is a low-power timer
 *****************************************************************************/
#define IsLowPowerTimer(type)           ((type) & gTmrLowPowerTimer_c)

/******************************************************************************
 * NAME: mTimerType_c
 * DESCRIPTION: timer types
 *****************************************************************************/
#define mTimerType_c            ( gTmrSingleShotTimer_c \
	| gTmrSetSecondTimer_c \
	| gTmrSetMinuteTimer_c \
	| gTmrIntervalTimer_c \
	| gTmrLowPowerTimer_c )

/******************************************************************************
 * NAME: mTMR_Event_c
 * DESCRIPTION: TMR_Task() event flag. Only one event is needed.
 *****************************************************************************/
#define mTMR_Event_c    ( 1 << 0 )

/* HARDWARE RELATED SECTION */

/******************************************************************************
 * NAME: gTMR_FTMIrqNo
 * DESCRIPTION: FTMx interrupt number
 * VALID VALUES: refer to Kinetis manual
 *****************************************************************************/

#if (gTargetBoard_d == gTargetTWR_K60N512_c)
  #if(gTMR_FTMx == 0)
  #define gTMR_FTMIrqNo                   62
  #elif (gTMR_FTMx == 1)
  #define gTMR_FTMIrqNo                   63
  #else
  #define gTMR_FTMIrqNo                   64
  #endif
#elif (gTargetBoard_d == gTargetTWR_KW21D512_c) 
/* FTMx clock gating register */
  #define gTMR_FTM_SIM_SCGC_REG_c         SIM_SCGC6
  #if(gTMR_FTMx == 0)
  #define gTMR_FTMIrqNo                   42
  #define gTMR_FTM_SIM_SCGC_BIT_c         24 
  #elif (gTMR_FTMx == 1)
  #define gTMR_FTMIrqNo                   43
  #define gTMR_FTM_SIM_SCGC_BIT_c         25
  #else
  #define gTMR_FTMIrqNo                   44
  #define gTMR_FTM_SIM_SCGC_BIT_c         26
  #endif
#endif /*gTargetBoard_d*/

/******************************************************************************
 * NAME: gTMR_FTMInterruptPriority
 * DESCRIPTION: FTMx interrupt priority
 * VALID VALUES: 0..15
 *****************************************************************************/
#ifndef gTMR_FTMInterruptPriority
#define gTMR_FTMInterruptPriority       3
#endif

/******************************************************************************
 * NAME: gTmrFTM_max
 * DESCRIPTION: Maximum number of FTM hardware modules 
 * 			   available on target platform	  
 * VALID VALUES: refer to Kinetis manual
 *****************************************************************************/

#ifndef gTmrFTM_max
#define gTmrFTM_max                     2 
#endif

/******************************************************************************
 * NAME: gTmrFTM_CN_max
 * DESCRIPTION: Maximum number of FTM channels
 * 			   available on target platform	  
 * VALID VALUES: refer to Kinetis manual
 *****************************************************************************/

#ifndef gTmrFTM_CN_max
#define gTmrFTM_CN_max                  7
#endif

/* Some checks ... */
#if ( gTMR_FTMx > gTmrFTM_max )
#error "FTM timer out of range [0-2]"
#endif

#if ( gTMR_FTM_CNx > gTmrFTM_CN_max )
#error "FTM channel out of range [0-7]"
#endif

/******************************************************************************
 * NAME: see below
 * DESCRIPTION: FTM registers definitions  
 * VALID VALUES: refer to Kinetis manual
 *****************************************************************************/

#if(gTMR_FTMx == 0)
    #define gFTMxSC_c       		FTM0_SC                 /* FTM0 Status and Control register. */
    #define gFTMxCNT        		FTM0_CNT                /* FTM0 Counter register */
    #define gFTMxMOD_c     			FTM0_MOD                /* FTM0 Modulo register */
    #define gFTMxMODE_c			    FTM0_MODE               /* FTM0 Features Mode Selection register */
    #define gFTMxCNTIN_c			FTM0_CNTIN              /* FTM0 Counter Initial Value register */ 

/******************************************************************************
 * NAME: see below
 * DESCRIPTION: FTM channel registers definition
 * VALID VALUES: refer to Kinetis manual
 *****************************************************************************/

    #if (gTMR_FTM_CNx == 0)
        #define gFTMxCnSC_c     	FTM0_C0SC               /* Channel 0 Status and Control register */
        #define gFTMxCnV_c      	FTM0_C0V                /* Channel 0 Value register */
    #elif (gTMR_FTM_CNx == 1)
        #define gFTMxCnSC_c     	FTM0_C1SC               /* Channel 1 Status and Control register */
        #define gFTMxCnV_c      	FTM0_C1V                /* Channel 1 Value register */
    #elif (gTMR_FTM_CNx == 2)
        #define gFTMxCnSC_c     	FTM0_C2SC               /* Channel 2 Status and Control register */
        #define gFTMxCnV_c      	FTM0_C2V                /* Channel 2 Value register */
    #elif (gTMR_FTM_CNx == 3)
        #define gFTMxCnSC_c     	FTM0_C3SC               /* Channel 3 Status and Control register */
        #define gFTMxCnV_c      	FTM0_C3V                /* Channel 3 Value register */
    #elif (gTMR_FTM_CNx == 4)
        #define gFTMxCnSC_c     	FTM0_C4SC               /* Channel 4 Status and Control register */
        #define gFTMxCnV_c      	FTM0_C4V                /* Channel 4 Value register */
    #elif (gTMR_FTM_CNx == 5)
        #define gFTMxCnSC_c     	FTM0_C5SC               /* Channel 5 Status and Control register */
        #define gFTMxCnV_c      	FTM0_C5V                /* Channel 5 Value register */
    #elif (gTMR_FTM_CNx == 6)
        #define gFTMxCnSC_c     	FTM0_C6SC               /* Channel 6 Status and Control register */
        #define gFTMxCnV_c      	FTM0_C6V                /* Channel 6 Value register */
    #else
        #define gFTMxCnSC_c     	FTM0_C7SC               /* Channel 7 Status and Control register */
        #define gFTMxCnV_c      	FTM0_C7V                /* Channel 7 Value register */
    #endif
    
#elif (gTMR_FTMx == 1)
    #define gFTMxSC_c       		FTM1_SC                 /* FTM1 Status and Control register. */
    #define gFTMxCNT        		FTM1_CNT                /* FTM1 Counter register */
    #define gFTMxMOD_c     			FTM1_MOD                /* FTM1 Modulo register */
    #define gFTMxMODE_c			    FTM1_MODE               /* FTM1 Features Mode Selection register */
    #define gFTMxCNTIN_c			FTM1_CNTIN              /* FTM1 Counter Initial Value register */

    /******************************************************************************
     * NAME: see below
     * DESCRIPTION: FTM channel registers definition
     * VALID VALUES: refer to Kinetis manual
     ******************************************************************************/
     
    #if (gTMR_FTM_CNx == 0)
        #define gFTMxCnSC_c     	FTM1_C0SC               /* Channel 0 Status and Control register */
        #define gFTMxCnV_c      	FTM1_C0V                /* Channel 0 Value register */
    #else
        #define gFTMxCnSC_c     	FTM1_C1SC               /* Channel 1 Status and Control register */
        #define gFTMxCnV_c      	FTM1_C1V                /* Channel 1 Value register */
    #endif

#else 
    #define gFTMxSC_c       		FTM2_SC                 /* FTM2 Status and Control register. */
    #define gFTMxCNT        		FTM2_CNT                /* FTM2 Counter register */
    #define gFTMxMOD_c     			FTM2_MOD                /* FTM2 Modulo register */
    #define gFTMxMODE_c			    FTM2_MODE               /* FTM2 Features Mode Selection register */
    #define gFTMxCNTIN_c			FTM2_CNTIN              /* FTM2 Counter Initial Value register */
    
    /******************************************************************************
     * NAME: see below
     * DESCRIPTION: FTM channel registers definition
     * VALID VALUES: refer to Kinetis manual
     ******************************************************************************/
    #if (gTMR_FTM_CNx == 0)
        #define gFTMxCnSC_c     	FTM2_C0SC         		/* Channel 0 Status and Control register */
        #define gFTMxCnV_c      	FTM2_C0V          		/* Channel 0 Value register */
    #else
        #define gFTMxCnSC_c     	FTM2_C1SC         		/* Channel 1 Status and Control register */
        #define gFTMxCnV_c      	FTM2_C1V          		/* Channel 1 Value register */
    #endif
    
#endif

/******************************************************************************
 * NAME: gTmrSourceClkKHz_c
 * DESCRIPTION: Source timer clock in Khz
 * VALID VALUES: refer to Kinetis manual
 *****************************************************************************/
#define gTmrSourceClkKHz_c      48000

/*
 * NAME: gFTMxSC_PrescaleX_c
 * DESCRIPTION: Prescaler values
 * VALID VALUES: refer to Kinetis manual
 */
#define gFTMxSC_Prescale1_c     0x00    					/* For prescale Value of 1   */
#define gFTMxSC_Prescale2_c     0x01    					/* For prescale Value of 2   */
#define gFTMxSC_Prescale4_c     0x02    					/* For prescale Value of 4   */
#define gFTMxSC_Prescale8_c     0x03    					/* For prescale Value of 8   */
#define gFTMxSC_Prescale16_c    0x04    					/* For prescale Value of 16  */
#define gFTMxSC_Prescale32_c    0x05    					/* For prescale Value of 32  */
#define gFTMxSC_Prescale64_c    0x06    					/* For prescale Value of 64  */
#define gFTMxSC_Prescale128_c   0x07    					/* For prescale Value of 128 */

/*
The resolution of the timer is between 
4ms(max. time a task to be executed) - 0xffff * (1tick in ms).  
 */

/******************************************************************************
 * NAME: gFTMxSC_PrescaleCount_c
 * DESCRIPTION: Prescaler value used by TMR module
 *              Changing the prescaler the resolution will increase or decrease
 * VALID VALUES: refer to Kinetis manual
 *****************************************************************************/ 
#define gFTMxSC_PrescaleCount_c     gFTMxSC_Prescale128_c

/******************************************************************************
 * NAME: see below
 * DESCRIPTION: Bits in the FTMx timer status and control registers (FTMxSC)
 * VALID VALUES: refer to Kinetis manual
 *****************************************************************************/
#define gFTMxSC_TOF_c           0x80    					/* Timer overflow flag. (RO) */
#define gFTMxSC_TOIE_c          0x40    					/* Timer overflow interrupt enable. (RW) */
#define gFTMxSC_CPWMS_c         0x20    					/* Center-aliged PWM select. (RW) */
#define gFTMxSC_CLKSB_c         0x10    					/* Clock source, high bit. (RW) */
#define gFTMxSC_CLKSA_c         0x08    					/* Clock source, low bit. (RW) */
#define gFTMxSC_PS2_c           0x04    					/* Prescale divisor select, high bit. (RW) */
#define gFTMxSC_PS1_c           0x02    					/* Prescale divisor select, middle bit. (RW) */
#define gFTMxSC_PS0_c           0x01    					/* Prescale divisor select, low bit. (RW) */

/******************************************************************************
 * NAME: see below
 * DESCRIPTION: Bits in the FTMx channel N status and control registers (FTMxCnSC)
 * VALID VALUES: refer to Kinetis manual
 *****************************************************************************/
#define gFTMxCnSC_CHF_c	        0x80    					/* Channel Overflow flag. */
#define gFTMxCnSC_CHIE_c        0x40    					/* Channel Interrupt enable. */        
#define gFTMxCnSC_MSB_c         0x20    					/* Channel Mode select bit B. */
#define gFTMxCnSC_MSA_c         0x10    					/* Channel Mode select bit A. */
#define gFTMxCnSC_ELSB_c        0x08    					/* Channel Edge/level select high bit. */
#define gFTMxCnSC_ELSA_c        0x04    					/* Channel Edge/level select low bit. */
#define gFTMxCnSC_DMA_c         0x01    					/* DMA transfer enable bit */

/******************************************************************************
 * NAME: gFTMxSC_ClockSource_c
 * DESCRIPTION: Clock source for TMR module
 * VALID VALUES: refer to Kinetis manual
 *****************************************************************************/
#define gFTMxSC_ClockSource_c	gFTMxSC_CLKSA_c 			/* Select the system clock. */

/******************************************************************************
 * NAME: gFTMxSC_Stop_c
 * DESCRIPTION: FTMx_SC register value for disabling (stopping) the timer
 * VALID VALUES: refer to Kinetis manual
 *****************************************************************************/
#define gFTMxSC_Stop_c          0x00

/******************************************************************************
 * NAME: FTMReadCnVRegister()
 * DESCRIPTION: Macro used to read the compare registers 
 *****************************************************************************/
#define FTMReadCnVRegister(variable)	(variable) = gFTMxCnV_c;

/******************************************************************************
 * NAME: FTMReadCNTRegister()
 * DESCRIPTION: Macro used to read the free running counter
 *****************************************************************************/
#define FTMReadCNTRegister(variable)   	(variable) = gFTMxCNT;

/******************************************************************************
 * NAME: FTMReadCNTRegister()
 * DESCRIPTION: Enable the hardware timer
 *****************************************************************************/
#define FTMStartTimerHardware()		(gFTMxSC_c |= (gFTMxSC_ClockSource_c | gFTMxSC_PrescaleCount_c))

/******************************************************************************
 * NAME: FTMReadCNTRegister()
 * DESCRIPTION: Disable the hardware timer
 *****************************************************************************/
#define FTMStopTimerHardware()		(gFTMxSC_c = gFTMxSC_Stop_c)

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/

/******************************************************************************
 * Type name: tmrTimerStatus_t
 * Type description: The status and type are bitfields, to save RAM.
 *                   This costs some code space, though.
 * Members: N/A
 *****************************************************************************/
typedef uint8_t tmrTimerStatus_t;

/******************************************************************************
 * NAME: tmrStatus_t
 * DESCRIPTION: timer status - see the status macros.
 *              If none of these flags are on, the timer is not allocated.
 *              For allocated timers, exactly one of these flags will be set.
 *              mTmrStatusActive_c - Timer has been started and has not yet expired.
 *              mTmrStatusReady_c - TMR_StartTimer() has been called for this timer, but
 *                                  the timer task has not yet actually started it. The
 *                                  timer is considered to be active.
 *              mTmrStatusInactive_c Timer is allocated, but is not active.
 *****************************************************************************/
typedef uint8_t tmrStatus_t;

/******************************************************************************
 * NAME: tmrTimerTicks_t
 * DESCRIPTION: 16-bit timer ticks type definition
 * VALID VALUES: see definition
 *****************************************************************************/
typedef uint16_t tmrTimerTicks16_t;

/******************************************************************************
 * NAME: tmrTimerTicks_t
 * DESCRIPTION: 32-bit timer ticks type definition
 * VALID VALUES: see definition
 *****************************************************************************/
typedef uint32_t tmrTimerTicks32_t;

/******************************************************************************
 * Type name: tmrTimerTableEntry_tag
 * Type description: One entry in the main timer table.
 * Members: intervalInTicks - The timer's original duration, in ticks.
 *                            Used to reset intervnal timers.
 *
 *          countDown - When a timer is started, this is set to the duration.
 *                      The timer task decrements this value. When it reaches
 *                      zero, the timer has expired.
 *          pfCallBack - Pointer to the callback function
 *****************************************************************************/
typedef struct tmrTimerTableEntry_tag {
  tmrTimerTicks32_t intervalInTicks;
  tmrTimerTicks32_t remainingTicks;
  pfTmrCallBack_t pfCallBack;
} tmrTimerTableEntry_t;

#endif /* #ifndef __TIMER_H__ */

 /****************************************************************************/
