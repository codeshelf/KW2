/******************************************************************************
 * Source file for Timer driver.
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

/******************************************************************************
*******************************************************************************
* Includes
*******************************************************************************
******************************************************************************/

#include "EmbeddedTypes.h"
#include "PortConfig.h"
#include "TMR_Interface.h"
#include "Timer.h"
#include "Interrupt.h"


/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/

/******************************************************************************
 * NAME: previousTimeInTicks
 * DESCRIPTION: The previous time in ticks when the counter register was read
 * VALUES: 0..65535
 *****************************************************************************/
static tmrTimerTicks16_t previousTimeInTicks;

/******************************************************************************
 * NAME: mMaxToCountDown_c
 * DESCRIPTION:  Count to maximum (0xffff - 2*4ms(in ticks)), to be sure that 
 * the currentTimeInTicks will never roll over previousTimeInTicks in the 
 * TMR_Task(); A task have to be executed at most in 4ms.
 * VALUES: 0..65535
 *****************************************************************************/
static uint16_t mMaxToCountDown_c; 

/******************************************************************************
 * NAME: mTicksFor4ms
 * DESCRIPTION:  Ticks for 4ms. The TMR_Task()event will not be issued faster than 4ms
 * VALUES: uint32_t range
 *****************************************************************************/
static uint32_t mTicksFor4ms;

/******************************************************************************
 * NAME: mClkSourceKhz
 * DESCRIPTION:  The source clock in Khz
 * VALUES: see definition
 *****************************************************************************/
static uint32_t mClkSourceKhz = (uint32_t)(gTmrSourceClkKHz_c);

/******************************************************************************
 * NAME: maTmrTimerTable
 * DESCRIPTION:  Main timer table. All allocated timers are stored here.
 *               A timer's ID is it's index in this table.
 * VALUES: see definition
 *****************************************************************************/
static tmrTimerTableEntry_t maTmrTimerTable[gTmrTotalTimers_c];

/******************************************************************************
 * NAME: maTmrTimerStatusTable
 * DESCRIPTION: timer status stable. Making the single-byte-per-timer status
 *              table a separate array saves a bit of code space.
 *              If an entry is == 0, the timer is not allocated.
 * VALUES: see definition
 *****************************************************************************/
static tmrStatus_t maTmrTimerStatusTable[gTmrTotalTimers_c];

/******************************************************************************
 * NAME: numberOfActiveTimers
 * DESCRIPTION: Number of Active timers (without low power capability)
 *              the MCU can not enter low power if numberOfActiveTimers!=0
 * VALUES: 0..255
 *****************************************************************************/
static uint8_t numberOfActiveTimers = 0;

/******************************************************************************
 * NAME: numberOfLowPowerActiveTimers
 * DESCRIPTION: Number of low power active timer.
 *              The MCU can enter in low power if more low power timers are active
 * VALUES:
 *****************************************************************************/
static uint8_t numberOfLowPowerActiveTimers = 0;

#define IncrementActiveTimerNumber(type)  (((type) & gTmrLowPowerTimer_c) \
                                          ?(++numberOfLowPowerActiveTimers) \
                                          :(++numberOfActiveTimers) )                                   
#define DecrementActiveTimerNumber(type)  (((type) & gTmrLowPowerTimer_c) \
                                          ?(--numberOfLowPowerActiveTimers) \
                                          :(--numberOfActiveTimers) ) 
                                          

/******************************************************************************
 * NAME: timerHardwareIsRunning
 * DESCRIPTION: Flag if the hardware timer is running or not
 * VALUES: TRUE/FALSE
 *****************************************************************************/
static bool_t timerHardwareIsRunning = FALSE;

/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/

/******************************************************************************
 * NAME: TMR_GetTimerStatus
 * DESCRIPTION: RETURNs the timer status
 * PARAMETERS:  IN: timerID - the timer ID
 * RETURN: see definition of tmrStatus_t
 * NOTES: none
 *****************************************************************************/
static tmrStatus_t TMR_GetTimerStatus 
( 
    tmrTimerID_t timerID 
);

/******************************************************************************
 * NAME: TMR_SetTimerStatus
 * DESCRIPTION: Set the timer status
 * PARAMETERS:  IN: timerID - the timer ID
 * 			   IN: status - the status of the timer
 * RETURN: None
 * NOTES: none
 *****************************************************************************/
static void TMR_SetTimerStatus
( 
    tmrTimerID_t timerID,
    tmrStatus_t status
);

/******************************************************************************
 * NAME: TMR_GetTimerType
 * DESCRIPTION: RETURNs the timer type
 * PARAMETERS:  IN: timerID - the timer ID
 * RETURN: see definition of tmrTimerType_t
 * NOTES: none
 *****************************************************************************/
static tmrTimerType_t TMR_GetTimerType 
( 
    tmrTimerID_t timerID 
);

/******************************************************************************
 * NAME: TMR_SetTimerType
 * DESCRIPTION: Set the timer type
 * PARAMETERS:  IN: timerID - the timer ID
 * 			    IN: type - timer type
 * RETURN: none
 * NOTES: none
 *****************************************************************************/
static void TMR_SetTimerType
(
    tmrTimerID_t timerID,
    tmrTimerType_t type
);

/******************************************************************************
 * NAME: TmrTicksFromMilliseconds
 * DESCRIPTION: Convert milliseconds to ticks
 * PARAMETERS:  IN: milliseconds
 * RETURN: tmrTimerTicks32_t - ticks number
 * NOTES: none
 *****************************************************************************/
static tmrTimerTicks32_t TmrTicksFromMilliseconds 
( 
    tmrTimeInMilliseconds_t milliseconds 
);

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

/******************************************************************************
* NAME: TMR_GetTimerStatus
* DESCRIPTION: Returns the timer status
* PARAMETERS:  IN: timerID - the timer ID
* RETURN: see definition of tmrStatus_t
* NOTES: none
******************************************************************************/
static tmrStatus_t TMR_GetTimerStatus
(
    tmrTimerID_t timerID
)
{
    return maTmrTimerStatusTable[timerID] & mTimerStatusMask_c;
}

/******************************************************************************
* NAME: TMR_SetTimerStatus
* DESCRIPTION: Set the timer status
* PARAMETERS:  IN: timerID - the timer ID
* 			   IN: status - the status of the timer	
* RETURN: None
* NOTES: none
******************************************************************************/
static void TMR_SetTimerStatus
(
    tmrTimerID_t timerID, 
    tmrStatus_t status
)
{
    maTmrTimerStatusTable[timerID] = (maTmrTimerStatusTable[timerID] & ~mTimerStatusMask_c) | status;
}

/******************************************************************************
* NAME: TMR_GetTimerType
* DESCRIPTION: Returns the timer type
* PARAMETERS:  IN: timerID - the timer ID
* RETURN: see definition of tmrTimerType_t
* NOTES: none
******************************************************************************/
static tmrTimerType_t TMR_GetTimerType
(
    tmrTimerID_t timerID
)
{
    return maTmrTimerStatusTable[timerID] & mTimerType_c;
}

/******************************************************************************
* NAME: TMR_SetTimerType
* DESCRIPTION: Set the timer type
* PARAMETERS:  IN: timerID - the timer ID
* 			   IN: type - timer type	
* RETURN: none
* NOTES: none
******************************************************************************/
static void TMR_SetTimerType
(
    tmrTimerID_t timerID, 
    tmrTimerType_t type
)
{
    maTmrTimerStatusTable[timerID] = (maTmrTimerStatusTable[timerID] & ~mTimerType_c) | type;
} 

/******************************************************************************
 * NAME: TmrTicksFromMilliseconds
 * DESCRIPTION: Convert milliseconds to ticks
 * PARAMETERS:  IN: milliseconds
 * RETURN: tmrTimerTicks32_t - ticks number
 * NOTES: none
 ******************************************************************************/
static tmrTimerTicks32_t TmrTicksFromMilliseconds
(
    tmrTimeInMilliseconds_t milliseconds
)
{
    return (milliseconds * (mClkSourceKhz/(0x01<<gFTMxSC_PrescaleCount_c)));
} 

/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/

/******************************************************************************
 * NAME: TMR_Init
 * DESCRIPTION: initialize the timer module
 * PARAMETERS: -
 * RETURN: -
 ******************************************************************************/
void TMR_Init 
(
    void
)
{
    /* Configure a FTM channel: enable interrupts; set output compare mode. */

    /* Enable timer IRQ */
    NVIC_EnableIRQ(gTMR_FTMIrqNo);
    /* Set timer IRQ priority */
    NVIC_SetPriority(gTMR_FTMIrqNo, gTMR_FTMInterruptPriority);

    /* Setup the system clock gating */
    gTMR_FTM_SIM_SCGC_REG_c |= (1<<gTMR_FTM_SIM_SCGC_BIT_c);

    /* Dummy read of the FTMx_SC register to clear the interrupt flag */                       
    (void)(gFTMxSC_c == 0U);
    /* Stop the counter */
    gFTMxSC_c = (uint32_t)0x00UL;
    /* Dummy read of the FTMx_CnSC register to clear the interrupt flag */
    (void)(gFTMxCnSC_c == 0U);             

    /* Disable write protection */
    /* FTMx_MODE: WPDIS=1 */
    gFTMxMODE_c |= (uint32_t)0x04UL;       
    /* FTMx_MODE: FTMEN=0 */
    gFTMxMODE_c &= (uint32_t)~0x01UL;     

    /* Clear modulo register */
    gFTMxMOD_c = (uint32_t)0x00UL;	

    gFTMxCnSC_c = (uint32_t)0x50UL; 

    //gTimerTaskID = TS_CreateTask(gTsTimerTaskPriority_c, TMR_Task);
    //TMR_Task();
    /* Count to maximum (0xffff - 2*4ms(in ticks)), to be sure that the currentTimeInTicks 
    will never roll over previousTimeInTicks in the TMR_Task() */
    mMaxToCountDown_c = 0xFFFF - TmrTicksFromMilliseconds(8); 
    /* The TMR_Task()event will not be issued faster than 4ms*/
    mTicksFor4ms = TmrTicksFromMilliseconds(4);
}

/******************************************************************************
 * NAME: TMR_NotifyClkChanged
 * DESCRIPTION: This function is called when the clock is changed
 * PARAMETERS: IN: clkKhz (uint32_t) - new clock
 * RETURN: -
 *****************************************************************************/
void TMR_NotifyClkChanged
(
    uint32_t clkKhz
)
{
    mClkSourceKhz = clkKhz;
    /* Clock was changed, so calculate again  mMaxToCountDown_c.
    Count to maximum (0xffff - 2*4ms(in ticks)), to be sure that the currentTimeInTicks 
    will never roll over previousTimeInTicks in the TMR_Task() */
    mMaxToCountDown_c = 0xFFFF - TmrTicksFromMilliseconds(8); 
    /* The TMR_Task()event will not be issued faster than 4ms*/
    mTicksFor4ms = TmrTicksFromMilliseconds(4);  
}

/******************************************************************************
 * NAME: TMR_AllocateTimer
 * DESCRIPTION: allocate a timer
 * PARAMETERS: -
 * RETURN: timer ID
 *****************************************************************************/

tmrTimerID_t TMR_AllocateTimer
(
    void
)
{
    uint32_t i;
  
    for (i = 0; i < NumberOfElements(maTmrTimerTable); ++i) 
    {
        if (!TMR_IsTimerAllocated(i)) 
        {
            TMR_SetTimerStatus(i, mTmrStatusInactive_c);
            return i;
        }
   }
  
   return gTmrInvalidTimerID_c;
}                                      

/******************************************************************************
 * NAME: TMR_AreAllTimersOff
 * DESCRIPTION: Check if all timers except the LP timers are OFF.
 * PARAMETERS: -
 * RETURN: TRUE if there are no active non-low power timers, FALSE otherwise
 *****************************************************************************/
bool_t TMR_AreAllTimersOff
(
    void
)
{
    return !numberOfActiveTimers;
}                                      

/******************************************************************************
 * NAME: TMR_FreeTimer
 * DESCRIPTION: Free a timer
 * PARAMETERS:  IN: timerID - the ID of the timer
 * RETURN: -
 * NOTES: Safe to call even if the timer is running.
 *        Harmless if the timer is already free.
 *****************************************************************************/
void TMR_FreeTimer
(
    tmrTimerID_t timerID
)
{
    TMR_StopTimer(timerID);
    TMR_MarkTimerFree(timerID);
}                                       

/******************************************************************************
 * NAME: TMR_InterruptHandler
 * DESCRIPTION: Timer Module Interrupt Service Routine
 * PARAMETERS: -
 * RETURN: -
 * NOTES: This function have to be added to Interrupt Vector Table
 *****************************************************************************/
void TMR_InterruptHandler
(
    void
) 
{
    /* Clearing the overflow flag requires reading it and then writing it. */

    if(gFTMxSC_c & gFTMxSC_TOF_c) 
    {
        gFTMxSC_c  &= ~ gFTMxSC_TOF_c;
    }

    if ( gFTMxCnSC_c & gFTMxCnSC_CHF_c ) 
    {
        gFTMxCnSC_c &= ~gFTMxCnSC_CHF_c;
        TMR_Task();
    }
}

/******************************************************************************
 * NAME: TMR_IsTimerActive
 * DESCRIPTION: Check if a specified timer is active
 * PARAMETERS: IN: timerID - the ID of the timer
 * RETURN: TRUE if the timer (specified by the timerID) is active,
 *         FALSE otherwise
 *****************************************************************************/
bool_t TMR_IsTimerActive
(
    tmrTimerID_t timerID
)
{
    return TMR_GetTimerStatus(timerID) == mTmrStatusActive_c;
} 

/******************************************************************************
 * NAME: TMR_StartTimer (BeeStack or application)
 * DESCRIPTION: Start a specified timer
 * PARAMETERS: IN: timerId - the ID of the timer
 *             IN: timerType - the type of the timer
 *             IN: timeInMilliseconds - time expressed in millisecond units
 *             IN: pfTmrCallBack - callback function
 * RETURN: -
 * NOTES: When the timer expires, the callback function is called in
 *        non-interrupt context. If the timer is already running when
 *        this function is called, it will be stopped and restarted.
 *****************************************************************************/
void TMR_StartTimer
(
    tmrTimerID_t timerID,                       
    tmrTimerType_t timerType,                   
    tmrTimeInMilliseconds_t timeInMilliseconds, 
    void (*pfTimerCallBack)(tmrTimerID_t)       
)
{
    tmrTimerTicks32_t intervalInTicks;
    
    /* check if timer is not allocated or if it has an invalid ID (fix@ENGR223389) */
    if (!TMR_IsTimerAllocated(timerID) || (gTmrInvalidTimerID_c == timerID)) 
      return;

    /* Stopping an already stopped timer is harmless. */
    TMR_StopTimer(timerID);

    intervalInTicks = TmrTicksFromMilliseconds(timeInMilliseconds);
    if (!intervalInTicks) 
    {
        intervalInTicks = 1;
    }

    TMR_SetTimerType(timerID, timerType);
    maTmrTimerTable[timerID].intervalInTicks = intervalInTicks;
    maTmrTimerTable[timerID].remainingTicks = intervalInTicks;
    maTmrTimerTable[timerID].pfCallBack = pfTimerCallBack;

    /* Enable timer, the timer task will do the rest of the work. */
    TMR_EnableTimer(timerID);
}

/******************************************************************************
 * NAME: TMR_StartLowPowerTimer
 * DESCRIPTION: Start a low power timer. When the timer goes off, call the 
 *              callback function in non-interrupt context. 
 *              If the timer is running when this function is called, it will 
 *              be stopped and restarted. 
 *              Start the timer with the following timer types:
 *                          - gTmrLowPowerMinuteTimer_c
 *                          - gTmrLowPowerSecondTimer_c
 *                          - gTmrLowPowerSingleShotMillisTimer_c
 *                          - gTmrLowPowerIntervalMillisTimer_c
 *              The MCU can enter in low power if there are only active low 
 *              power timers.
 * PARAMETERS: IN: timerId - the ID of the timer
 *             IN: timerType - the type of the timer
 *             IN: timeIn - time in ticks
 *             IN: pfTmrCallBack - callback function
 * RETURN: type/DESCRIPTION
 *****************************************************************************/
void TMR_StartLowPowerTimer
(
    tmrTimerID_t timerId,
    tmrTimerType_t timerType,
    uint32_t timeIn,
    void (*pfTmrCallBack)(tmrTimerID_t)
) 
{
#if(gTMR_EnableLowPowerTimers_d)
    TMR_StartTimer(timerId, timerType | gTmrLowPowerTimer_c, timeIn, pfTmrCallBack);
#else
    timerId = timerId;
    timerType = timerType;
    timeIn = timeIn;
    pfTmrCallBack = pfTmrCallBack;
#endif
}

/******************************************************************************
 * NAME: TMR_StartMinuteTimer
 * DESCRIPTION: Starts a minutes timer
 * PARAMETERS:  IN: timerId - the ID of the timer
 *              IN: timeInMinutes - time expressed in minutes
 *              IN: pfTmrCallBack - callback function
 * RETURN: None
 * NOTES: Customized form of TMR_StartTimer(). This is a single shot timer.
 *        There are no interval minute timers.
 *****************************************************************************/
 #if gTMR_EnableMinutesSecondsTimers_d
void TMR_StartMinuteTimer
(
    tmrTimerID_t timerId, 
    tmrTimeInMinutes_t timeInMinutes, 
    void (*pfTmrCallBack)(tmrTimerID_t)
)
{
    TMR_StartTimer(timerId, gTmrMinuteTimer_c, TmrMinutes(timeInMinutes), pfTmrCallBack);
}
#endif
  
/******************************************************************************
 * NAME: TMR_StartSecondTimer
 * DESCRIPTION: Starts a seconds timer
 * PARAMETERS:  IN: timerId - the ID of the timer
 *              IN: timeInSeconds - time expressed in seconds
 *              IN: pfTmrCallBack - callback function
 * RETURN: None
 * NOTES: Customized form of TMR_StartTimer(). This is a single shot timer.
 *        There are no interval seconds timers.
 *****************************************************************************/
 #if gTMR_EnableMinutesSecondsTimers_d
void TMR_StartSecondTimer
(
    tmrTimerID_t timerId, 
    tmrTimeInSeconds_t timeInSeconds, 
    void (*pfTmrCallBack)(tmrTimerID_t)
) 
{
    TMR_StartTimer(timerId, gTmrSecondTimer_c, TmrSeconds(timeInSeconds), pfTmrCallBack);
}
#endif
 
/******************************************************************************
 * NAME: TMR_StartIntervalTimer
 * DESCRIPTION: Starts an interval count timer
 * PARAMETERS:  IN: timerId - the ID of the timer
 *              IN: timeInMilliseconds - time expressed in milliseconds
 *              IN: pfTmrCallBack - callback function
 * RETURN: None
 * NOTES: Customized form of TMR_StartTimer()
 *****************************************************************************/
void TMR_StartIntervalTimer
(
    tmrTimerID_t timerID,
    tmrTimeInMilliseconds_t timeInMilliseconds,
    void (*pfTimerCallBack)(tmrTimerID_t)
)
{
    TMR_StartTimer(timerID, gTmrIntervalTimer_c, timeInMilliseconds, pfTimerCallBack);
}

/******************************************************************************
 * NAME: TMR_StartSingleShotTimer
 * DESCRIPTION: Starts an single-shot timer
 * PARAMETERS:  IN: timerId - the ID of the timer
 *              IN: timeInMilliseconds - time expressed in milliseconds
 *              IN: pfTmrCallBack - callback function
 * RETURN: None
 * NOTES: Customized form of TMR_StartTimer()
 *****************************************************************************/
void TMR_StartSingleShotTimer
(
    tmrTimerID_t timerID,
    tmrTimeInMilliseconds_t timeInMilliseconds,
    void (*pfTimerCallBack)(tmrTimerID_t)
)
{
    TMR_StartTimer(timerID, gTmrSingleShotTimer_c, timeInMilliseconds, pfTimerCallBack);
}

/******************************************************************************
 * NAME: TMR_StopTimer
 * DESCRIPTION: Stop a timer
 * PARAMETERS:  IN: timerID - the ID of the timer
 * RETURN: None
 * NOTES: Associated timer callback function is not called, even if the timer
 *        expires. Does not frees the timer. Safe to call anytime, regardless
 *        of the state of the timer.
 *****************************************************************************/
void TMR_StopTimer
(
    tmrTimerID_t timerID
)
{
    tmrStatus_t status;
    unsigned int saveInt;
    
    saveInt = IntDisableAll();
    status = TMR_GetTimerStatus(timerID);
    
    if ( (status == mTmrStatusActive_c) || (status == mTmrStatusReady_c) ) 
    {
        TMR_SetTimerStatus(timerID, mTmrStatusInactive_c);
        DecrementActiveTimerNumber(TMR_GetTimerType(timerID));
        /* if no sw active timers are enabled, */
        /* call the TMR_Task() to countdown the ticks and stop the hw timer*/
        if (!numberOfActiveTimers && !numberOfLowPowerActiveTimers) 
            TMR_Task();
    }	

    IntRestoreAll(saveInt);
}  

/******************************************************************************
 * NAME: TMR_Task
 * DESCRIPTION: Timer task. 
 *              Called by the kernel when the timer ISR posts a timer event.
 * PARAMETERS:  IN: events - timer events mask
 * RETURN: None
 * NOTES: none
 *****************************************************************************/
void TMR_Task
(
    void
)
{
    tmrTimerTicks16_t nextInterruptTime;
    pfTmrCallBack_t pfCallBack;
    tmrTimerTicks16_t currentTimeInTicks;
    tmrTimerStatus_t status;
    tmrTimerTicks16_t ticksSinceLastHere, ticksdiff; 
    uint8_t timerID;
    unsigned int saveInt;
    tmrTimerType_t timerType;

    saveInt = IntDisableAll();

    FTMReadCNTRegister(currentTimeInTicks);

    IntRestoreAll(saveInt);

    /* calculate difference between current and previous.  */
    ticksSinceLastHere = (currentTimeInTicks - previousTimeInTicks);
    /* remember for next time */
    previousTimeInTicks = currentTimeInTicks;
  
    for (timerID = 0; timerID < NumberOfElements(maTmrTimerTable); ++timerID) 
    {
        saveInt = IntDisableAll();
        status = TMR_GetTimerStatus(timerID);
        /* If TMR_StartTimer() has been called for this timer, start it's count */
        /* down as of now. */
        if (status == mTmrStatusReady_c) 
        {
            TMR_SetTimerStatus(timerID, mTmrStatusActive_c);
            IntRestoreAll(saveInt);
            continue;
        }
        IntRestoreAll(saveInt);

        /* Ignore any timer that is not active. */
        if (status != mTmrStatusActive_c) 
        {
            continue;
        }

        /* This timer is active. Decrement it's countdown.. */
        if (maTmrTimerTable[timerID].remainingTicks > ticksSinceLastHere) 
        {
            maTmrTimerTable[timerID].remainingTicks -= ticksSinceLastHere;
            continue;
        }

        timerType = TMR_GetTimerType(timerID);
        /* If this is an interval timer, restart it. Otherwise, mark it as inactive. */
        if ( (timerType & gTmrSingleShotTimer_c) ||
                (timerType & gTmrSetMinuteTimer_c) ||
                (timerType & gTmrSetSecondTimer_c)  ) 
        {
            TMR_StopTimer(timerID);
        } 
        else 
        {
            maTmrTimerTable[timerID].remainingTicks = maTmrTimerTable[timerID].intervalInTicks;
        }
        /* This timer has expired. */
        pfCallBack = maTmrTimerTable[timerID].pfCallBack;
        /*Call callback if it is not NULL
        This is done after the timer got updated,
        in case the timer gets stopped or restarted in the callback*/
        if (pfCallBack) 
        {
            pfCallBack(timerID);
        }
    }  /* for (timerID = 0; timerID < ... */
  
    /* Find the shortest active timer. */
    nextInterruptTime = mMaxToCountDown_c;
    
    for (timerID = 0; timerID < NumberOfElements(maTmrTimerTable); ++timerID) 
    {
        if (TMR_GetTimerStatus(timerID) == mTmrStatusActive_c) 
        {
            if (nextInterruptTime > maTmrTimerTable[timerID].remainingTicks) 
            {
                nextInterruptTime = maTmrTimerTable[timerID].remainingTicks;
            }
        }
    }
    
	saveInt = IntDisableAll();
    
	/* Check to be sure that the timer was not programmed in the past for different source clocks.
	 * The interrupts are now disabled.
	 */      	 
	FTMReadCNTRegister(ticksdiff);  
    
    /* Number of ticks to be here */
    ticksdiff = (uint16_t)(ticksdiff - currentTimeInTicks); 
    
    /* Next ticks to count already expired?? */
    if(ticksdiff >= nextInterruptTime)
    {  
        /* Is assumed that a task has to be executed in 4ms...
    so if the ticks already expired enter in TMR_Task() after 4ms*/
        nextInterruptTime = ticksdiff + mTicksFor4ms;
    } 
    else 
    {
        /* Time reference is 4ms, so be sure that won't be loaded 
           in Cmp Reg. less that 4ms in ticks */
        if((nextInterruptTime - ticksdiff) < mTicksFor4ms) 
        {
            nextInterruptTime = ticksdiff + mTicksFor4ms;
        }
    }
    /* Update the compare register */
    nextInterruptTime += currentTimeInTicks;
    gFTMxCnV_c = nextInterruptTime;
    IntRestoreAll(saveInt);

	if (!numberOfActiveTimers && !numberOfLowPowerActiveTimers) 
    {
		FTMStopTimerHardware();
		timerHardwareIsRunning = FALSE;
	} 
    else if (!timerHardwareIsRunning) 
    {
		FTMStartTimerHardware();
		timerHardwareIsRunning = TRUE;
	}
}

/******************************************************************************
 * NAME: TMR_EnableTimer
 * DESCRIPTION: Enable the specified timer
 * PARAMETERS:  IN: tmrID - the timer ID
 * RETURN: None
 * NOTES: none
 *****************************************************************************/
void TMR_EnableTimer
(
    tmrTimerID_t tmrID
)
{    		
    unsigned int saveInt;
    saveInt = IntDisableAll();

    if (TMR_GetTimerStatus(tmrID) == mTmrStatusInactive_c)
    {      
        IncrementActiveTimerNumber(TMR_GetTimerType(tmrID));    
        TMR_SetTimerStatus(tmrID, mTmrStatusReady_c);
        TMR_Task();
    }  	

    IntRestoreAll(saveInt);
}

/******************************************************************************
 * NAME: TMR_NotCountedMillisTimeBeforeSleep
 * DESCRIPTION: This function is called by Low Power module;
 * Also this function stops the hardware timer.
 * PARAMETERS:  none
 * RETURN: uint32 - time in millisecond that wasn't counted before
 *		  entering in sleep
 * NOTES: none
 *****************************************************************************/
uint16_t TMR_NotCountedTicksBeforeSleep
(
    void
)
{
#if (gTMR_EnableLowPowerTimers_d)  
    uint16_t currentTimeInTicks;
  
    if (!numberOfLowPowerActiveTimers)
        return 0;

    FTMReadCNTRegister(currentTimeInTicks);
    FTMStopTimerHardware();
    timerHardwareIsRunning = FALSE; 

    /* The hw timer is stopped but keep timerHardwareIsRunning = TRUE...*/ 
    /* The Lpm timers are considered as being in running mode, so that  */
    /* not to start the hw timer if a TMR event occurs (this shouldn't happen) */ 
     
    return  (uint16_t)(currentTimeInTicks - previousTimeInTicks);
#else  
    return 0;
#endif
} 

/******************************************************************************
 * NAME: TMR_SyncLpmTimers
 * DESCRIPTION: This function is called by the Low Power module
 * each time the MCU wakes up.
 * PARAMETERS:  sleep duration in milliseconds
 * RETURN: none
 * NOTES: none
 *****************************************************************************/                             
void TMR_SyncLpmTimers
(
    uint32_t sleepDurationTmrTicks
)
{
#if (gTMR_EnableLowPowerTimers_d) 
    index_t  timerID;
    tmrTimerType_t timerType;

    /* Check if there are low power active timer */
    if (!numberOfLowPowerActiveTimers)
        return;          

    /* For each timer, detect the timer type and count down the spent duration in sleep */  
    for (timerID = 0; timerID < NumberOfElements(maTmrTimerTable); ++timerID) 
    {

        /* Detect the timer type and count down the spent duration in sleep */
        timerType = TMR_GetTimerType(timerID);

        /* Sync. only the low power timers that are active */
        if ( (TMR_GetTimerStatus(timerID) == mTmrStatusActive_c)
                && (IsLowPowerTimer(timerType)) ) 
        {
            /* Timer expired when MCU was in sleep mode??? */
            if( maTmrTimerTable[timerID].remainingTicks > sleepDurationTmrTicks) 
            {
                maTmrTimerTable[timerID].remainingTicks -= sleepDurationTmrTicks;

            } 
            else 
            {
                maTmrTimerTable[timerID].remainingTicks = 0;           
            }

        }

    }/* end for (timerID = 0;.... */ 

    FTMStartTimerHardware();
    FTMReadCNTRegister(previousTimeInTicks);

    TMR_Task();
#else
    sleepDurationTmrTicks = sleepDurationTmrTicks;
#endif /* #if (gTMR_EnableLowPowerTimers_d) */ 
}

/*****************************************************************************/
