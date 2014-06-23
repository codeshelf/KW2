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

#ifndef __TMR_INTERFACE_H__
#define __TMR_INTERFACE_H__

/******************************************************************************
*******************************************************************************
* Includes
*******************************************************************************
******************************************************************************/

#include "EmbeddedTypes.h"

/******************************************************************************
*******************************************************************************
* Public macros
*******************************************************************************
******************************************************************************/

/******************************************************************************
* NAME: gTMR_Enabled_d
* DESCRIPTION: Enables / Disables the Timer platform component
* VALID RANGE: TRUE/FALSE
******************************************************************************/
#ifndef gTMR_Enabled_d
#define gTMR_Enabled_d    TRUE
#endif

/******************************************************************************
 * NAME: gTMR_FTMx
 * DESCRIPTION: Specifies which FTM hardware module is used
 * VALID RANGE: 0..2
 *****************************************************************************/
#ifndef gTMR_FTMx
#define gTMR_FTMx	0
#endif

/******************************************************************************
 * NAME: gTMR_FTM_CNx
 * DESCRIPTION: Specifies which FTM CHANNEL is used
 * VALID RANGE: 0..7
 *****************************************************************************/
#ifndef gTMR_FTM_CNx
#define gTMR_FTM_CNx	0
#endif

/******************************************************************************
 * NAME: gTMR_EnableLowPowerTimers_d
 * DESCRIPTION: Enable/Disable Low Power Timer
 * VALID RANGE: TRUE/FALSE
 *****************************************************************************/
#ifndef gTMR_EnableLowPowerTimers_d
#define gTMR_EnableLowPowerTimers_d    TRUE            
#endif

/******************************************************************************
 * NAME: gTMR_EnableMinutesSecondsTimers_d
 * DESCRIPTION:  Enable/Disable Minutes and Seconds Timers
 * VALID RANGE: TRUE/FALSE
 *****************************************************************************/
#ifndef gTMR_EnableMinutesSecondsTimers_d
#define gTMR_EnableMinutesSecondsTimers_d	TRUE
#endif

/******************************************************************************
 * NAME: gTmrApplicationTimers_c
 * DESCRIPTION: Number of timers needed by the application
 * VALID RANGE: user defined
 *****************************************************************************/
#ifndef gTmrApplicationTimers_c
#define gTmrApplicationTimers_c 4
#endif

/******************************************************************************
 * NAME: gTmrStackTimers_c
 * DESCRIPTION: Number of timers needed by the protocol stack
 * VALID RANGE: user defined
 *****************************************************************************/
#ifndef gTmrStackTimers_c
#define gTmrStackTimers_c	30
#endif

/******************************************************************************
 * NAME: gTmrTotalTimers_c
 * DESCRIPTION: Total number of timers
 * VALID RANGE: sum of application and stack timers
 *****************************************************************************/
#ifndef gTmrTotalTimers_c
#define gTmrTotalTimers_c	( gTmrApplicationTimers_c + gTmrStackTimers_c )
#endif

/******************************************************************************
 * NAME: TmrMilliseconds()
 * DESCRIPTION: Typecast the macro argument into milliseconds
 * VALID RANGE: -
 *****************************************************************************/
#define TmrMilliseconds( n )	( (tmrTimeInMilliseconds_t) (n) )

/******************************************************************************
 * NAME: TmrSeconds()
 * DESCRIPTION: Converts the macro argument (i.e. seconds) into milliseconds
 * VALID RANGE: - 
 *****************************************************************************/
#define TmrSeconds( n )			( (tmrTimeInMilliseconds_t) (TmrMilliseconds(n) * 1000) )

/******************************************************************************
 * NAME: TmrMinutes()
 * DESCRIPTION: Converts the macro argument (i.e. minutes) into milliseconds
 * VALID RANGE: -
 *****************************************************************************/
#define TmrMinutes( n )			( (tmrTimeInMilliseconds_t) (TmrSeconds(n) * 60) )

/******************************************************************************
 * NAME: gTmrInvalidTimerID_c
 * DESCRIPTION: Reserved for invalid timer id
 * VALID RANGE: 0xFF
 *****************************************************************************/
#define gTmrInvalidTimerID_c	0xFF

/******************************************************************************
 * NAME: gTmrSingleShotTimer_c, gTmrIntervalTimer_c,
 *       gTmrSetMinuteTimer_c, gTmrSetSecondTimer_c,
 *       gTmrLowPowerTimer_c
 * DESCRIPTION: Timer types coded values
 * VALID RANGE: see definitions below
 *****************************************************************************/
#define gTmrSingleShotTimer_c	0x01
#define gTmrIntervalTimer_c     0x02
#define gTmrSetMinuteTimer_c	0x04
#define gTmrSetSecondTimer_c	0x08
#define gTmrLowPowerTimer_c     0x10

/******************************************************************************
 * NAME: gTmrMinuteTimer_c
 * DESCRIPTION: Minute timer definition
 * VALID RANGE: see definition below
 *****************************************************************************/
#define gTmrMinuteTimer_c       ( gTmrSetMinuteTimer_c )

/******************************************************************************
 * NAME: gTmrSecondTimer_c
 * DESCRIPTION: Second timer definition
 * VALID RANGE: see definition below
 *****************************************************************************/
#define gTmrSecondTimer_c       ( gTmrSetSecondTimer_c )

/******************************************************************************
 * NAME: See below
 * DESCRIPTION: LP minute/second/millisecond timer definitions
 * VALID VALUES: See definitions below
 *****************************************************************************/
#define gTmrLowPowerMinuteTimer_c			( gTmrMinuteTimer_c | gTmrLowPowerTimer_c )
#define gTmrLowPowerSecondTimer_c			( gTmrSecondTimer_c | gTmrLowPowerTimer_c )
#define gTmrLowPowerSingleShotMillisTimer_c ( gTmrSingleShotTimer_c | gTmrLowPowerTimer_c )
#define gTmrLowPowerIntervalMillisTimer_c	( gTmrIntervalTimer_c | gTmrLowPowerTimer_c )


/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
******************************************************************************/
 
/******************************************************************************
 * NAME: tmrTimeInMilliseconds_t
 * DESCRIPTION: Times specified in milliseconds (max 0x3ffff)
 *****************************************************************************/
typedef uint32_t	tmrTimeInMilliseconds_t;

/******************************************************************************
 * NAME: tmrTimeInMinutes_t
 * DESCRIPTION: Times specified in minutes (up to 40 days)
 *****************************************************************************/
typedef uint32_t	tmrTimeInMinutes_t;

/******************************************************************************
 * NAME: tmrTimeInSeconds_t
 * DESCRIPTION: Times specified in seconds (up to 65535)
 *****************************************************************************/
typedef uint32_t	tmrTimeInSeconds_t;

/******************************************************************************
 * NAME: tmrTimerType_t
 * DESCRIPTION: Timer type
 *****************************************************************************/
typedef uint8_t		tmrTimerID_t;

/******************************************************************************
 * NAME: tmrTimerType_t
 * DESCRIPTION: Timer type
 *****************************************************************************/
typedef uint8_t		tmrTimerType_t;

/******************************************************************************
 * NAME: pfTmrCallBack_t
 * DESCRIPTION: Timer callback function
 *****************************************************************************/
typedef void ( *pfTmrCallBack_t ) ( tmrTimerID_t );


/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/
 
/******************************************************************************
* TMR_Init initializes the timer with enable interrupts; set output compare 
* mode. Also it calls to the TMR_Task function.
*
* Interface assumptions:
*   Before call another TMR functions, first, be sure to call this function.
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
******************************************************************************/
extern void TMR_Init 
(
    void
);

/******************************************************************************
* TMR_NotifyClkChanged is called when the clock is changed.
*
* Interface assumptions:
*   Before call another TMR functions, first, be sure to call this function.
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void TMR_NotifyClkChanged
(
    uint32_t clkKhz  /*IN: clkKhz (uint32_t) - new clock */
);

/******************************************************************************
* TMR_AllocateTimer reserves a timer if there is one available. Timers are 
* reserved until they are freed by TMR_FreeTimer(). 
*
* Interface assumptions:
*   It should be called before enable the timer. The timer should be free
*   before to be allocated.
*
* return value:
*   - Returns the timer number that should be passed to other TMR_xxx() API
*     functions.
*   - Returns gTmrInvalidTimerID_c if there are no timers available.
*
* Effects on global data:
*   NONE
*
******************************************************************************/
extern tmrTimerID_t TMR_AllocateTimer
(
    void
);
                                     
/******************************************************************************
 * NAME: TMR_AreAllTimersOff
 * DESCRIPTION: Check if all timers except the LP timers are OFF.
 * PARAMETERS: -
 * RETURN: TRUE if there are no active non-low power timers, FALSE otherwise
 *****************************************************************************/
extern bool_t TMR_AreAllTimersOff
(
    void
);                                      

/******************************************************************************
* TMR_FreeTimer frees a timer. Safe to call even if the timer is running. 
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void TMR_FreeTimer
(
    tmrTimerID_t timerID    /* IN: ID of the timer to be freed. */
); 

/******************************************************************************
* TMR_InterruptHandler is the Timer Module Interrupt Service Routine. 
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/

extern void TMR_InterruptHandler
(
    void
);

/******************************************************************************
* TMR_IsTimerActive checks if a given timer in the timerID parameter is 
* active. 
*
* Interface assumptions:
*   NONE
*
* return value:
*   TRUE - If the timer is running or in active state.
*   FALSE - If the timer is not running.
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern bool_t TMR_IsTimerActive
(
    tmrTimerID_t timerID  /*IN: ID of the timer to check if is active */
);

/******************************************************************************
* TMR_StartTimer  Start a timer. When the timer goes off, call the callback 
* function in non-interrupt context. If the timer is running when this function 
* is called, it will be stopped and restarted. 
*
* Interface assumptions:
*   The timer should be in ready mode or enabled before call this function.
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void TMR_StartTimer
(
    tmrTimerID_t timerID,                       /* IN: ID of the timer to be started */                 
    tmrTimerType_t timerType,                   /* IN: Type in which the timer will be started */               
    tmrTimeInMilliseconds_t timeInMilliseconds, /* IN: Selected interval in ticks */
    void (*pfTimerCallBack)(tmrTimerID_t)       /* IN: Callback to be called when timer goes off*/  
);

/******************************************************************************
* TMR_StartLowPowerTimer starts a low power timer. When the timer goes off, 
* call the callback function in non-interrupt context. If the timer is running 
* when this function is called, it will be stopped and restarted. The MCU can
* enter in low power if there are only active low power timers.
* 
* Interface assumptions:
*   Start the timer with the following timer types:
*   gTmrLowPowerMinuteTimer_c
*   gTmrLowPowerSecondTimer_c
*   gTmrLowPowerSingleShotMillisTimer_c
*   gTmrLowPowerIntervalMillisTimer_c
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void TMR_StartLowPowerTimer
(
    tmrTimerID_t timerId,               /* IN: ID of the timer to be started */	
    tmrTimerType_t timerType,           /* IN: Type of timer to be started */
    uint32_t timeIn,                    /* IN: Interval time */
    void (*pfTmrCallBack)(tmrTimerID_t) /* IN: Callback to be called when timer goes off */
);

/******************************************************************************
* TMR_StartMinuteTimer  Start a minute timer. When the timer goes off, call 
* the callback function in non-interrupt context. This is a single 
* shot timer. There are no interval minute timers.
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
 #if gTMR_EnableMinutesSecondsTimers_d
extern void TMR_StartMinuteTimer
(
    tmrTimerID_t timerId,                 /* IN: ID of the timer to be started */
    tmrTimeInMinutes_t timeInMinutes,     /* IN: Selected interval in minutes*/
    void (*pfTmrCallBack)(tmrTimerID_t)   /* IN: Callback to be called when timer goes off*/
);
#endif
  
/******************************************************************************
* TMR_StartSecondTimer  Start a second timer. When the timer goes off, call 
* the callback function in non-interrupt context. This is a single 
* shot timer. There are no interval second timers.
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
 #if gTMR_EnableMinutesSecondsTimers_d
extern void TMR_StartSecondTimer
(
    tmrTimerID_t timerId,                 /* IN: ID of the timer to be started */
    tmrTimeInSeconds_t timeInSeconds,     /* IN: Selected interval in Seconds*/
    void (*pfTmrCallBack)(tmrTimerID_t)   /* IN: Callback to be called when timer goes off*/
);
#endif
 
/******************************************************************************
* TMR_StartIntervalTimer starts a timer. When the timer goes off, call the 
* callback function in non-interrupt context. If the timer is running when 
* this function is called, it will be stopped and restarted. Different from
* TMR_StartTimer(), it needs one less parameter to pass. 
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void TMR_StartIntervalTimer
(
    tmrTimerID_t timerID,
    tmrTimeInMilliseconds_t timeInMilliseconds,
    void (*pfTimerCallBack)(tmrTimerID_t)
);

/******************************************************************************
* TMR_StartSingleShotTimer starts a timer. When the timer goes off, call the 
* callback function in non-interrupt context. If the timer is running when 
* this function is called, it will be stopped and restarted. Different from
* TMR_StartTimer(), it needs one less parameter to pass. 
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void TMR_StartSingleShotTimer
(
    tmrTimerID_t timerID,                       /* IN: ID of the timer to be started */
    tmrTimeInMilliseconds_t timeInMilliseconds, /* IN: Selected interval in miliseconds, limited to 0x3ffff milliseconds */ 
    void (*pfTimerCallBack)(tmrTimerID_t)       /* IN: Callback to be called when timer goes off*/
);

/******************************************************************************
* TMR_StopTimer Stops a timer. Does not trigger the timer, i.e. does not call 
* the timer's callback function. If the timer has expired, but the callback 
* function has not yet been called, this function will prevent the timer's 
* callback from being called. Does not free the timer. Safe to call anytime, 
* regardless of the state of the timer. 
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/  
extern void TMR_StopTimer
(
    tmrTimerID_t timerID      /* IN: ID of the timer to be stopped */
); 

/******************************************************************************
* TMR_Task  makes the entire job for Timer module. Every time an interrupt 
* occurs or a timer is started (setting it in ready state), this task is 
* called to program the next shortest timer (the time to come back in 
* the task). TMR_Task is the main function for the timer. 
* Process timer events in non-interrupt context.
*
* Interface assumptions:
*   NONE
*
* return value:
*   Time in millisecond that wasn't counted before entering in sleep
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void TMR_Task
(
    void
);

/******************************************************************************
* TMR_EnableTimer enables the timer specified by the tmrID if the timer is
* in inactive status. In other words, it set the timer to a Ready status.  
*
* Interface assumptions:
*   The status of the given tmrID should be in inactive mode.
*
* return value:
*   TRUE if the timer is active
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void TMR_EnableTimer
(
    tmrTimerID_t tmrID  /* IN: ID of the timer to be enabled */
);

/******************************************************************************
 * TMR_NotCountedMillisTimeBeforeSleep is called by Low Power module. Also, 
 * this function stops the hardware timer.
 * 
 * Interface assumptions:
 *  It should be called inside a low power module.
 * 
 * return value: 
 *  uint16 - time in millisecond that wasn't counted before
 *		  entering in sleep
 * Effects on global data:
 *   NONE
 *****************************************************************************/
extern uint16_t TMR_NotCountedTicksBeforeSleep
(
    void
);

/******************************************************************************
* TMR_SyncLpmTimers is called by the Low Power Module each time the MCU wakes 
* up.  
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/                             
extern void TMR_SyncLpmTimers
(
uint32_t sleepDurationTmrTicks  /*IN: sleep duration in milliseconds*/
);

#endif /* #ifndef __TMR_INTERFACE_H__ */

/*****************************************************************************/
