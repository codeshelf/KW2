/******************************************************************************
 * Source file for LED driver.
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

#include "LED.h"

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
 *******************************************************************************
 * Public memory definitions
 *******************************************************************************
 ******************************************************************************/

#if (gLEDSupported_d == TRUE)

/******************************************************************************
 *******************************************************************************
 * Private memory definitions
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *******************************************************************************
 * Private function prototypes
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 * LED_FlashTimeout
 * 
 * Timer callback function that is called each time the timer expires
 ******************************************************************************/
static void LED_FlashTimeout
(
  tmrTimerID_t timerId     /* IN: the timer ID*/
);

/******************************************************************************
 * LED_ExitSerialFlash
 *  
 * Stops serial flashing mode, if serial flash mode is active.Turns all LEDs off.
 ******************************************************************************/
static void LED_ExitSerialFlash
(
  void
);

/******************************************************************************
 * LED_DecrementBlip
 *
 * Decrements the blink count
 ******************************************************************************/
#if gLEDBlipEnabled_d
static void LED_DecrementBlip
(
  void
);
#endif

/******************************************************************************
 *******************************************************************************
 * Private type definitions
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *******************************************************************************
 * Private Memory Declarations
 *******************************************************************************
 ******************************************************************************/

/*
 * Name: mfLedInSerialMode
 * Description: indicates if the LED module is in serial mode or not
 */
static bool_t mfLedInSerialMode;      /* doing serial lights */

/*
 * Name: mLedFlashingLEDs
 * Description: indicates how many LEDs are in flashing mode 
 */
static uint8_t mLedFlashingLEDs = 0;  /* flashing LEDs */

/*
 * Name: mLEDTimerID
 * Description: LED timer ID
 */
static tmrTimerID_t mLEDTimerID = gTmrInvalidTimerID_c;

/*
 * Name: mLedBlips
 * Description: array that stores the LEDs blip information
 */		
#if gLEDBlipEnabled_d
static uint8_t mLedBlips[gLEDsOnTargetBoardCnt_c];
#endif


/******************************************************************************
 ******************************************************************************
 * Public functions
 ******************************************************************************
 *****************************************************************************/

/******************************************************************************
 * LED_Init
 * 
 * Initialize the LED module
 ******************************************************************************/
void LED_Init
(
  void
)
{
	/* enable clock */
	gLED_ClkGatingReg_c |= gLED_ClkGatingBit_c; 

	/* pin MUX configuration */
	gLED_Led1PinCtrlReg_c &= ~PORT_PCR_MUX_MASK;
	gLED_Led1PinCtrlReg_c |= PORT_PCR_MUX(gLED_PinCtrlAltFunc_c);
	gLED_Led2PinCtrlReg_c &= ~PORT_PCR_MUX_MASK;
	gLED_Led2PinCtrlReg_c |= PORT_PCR_MUX(gLED_PinCtrlAltFunc_c);
	gLED_Led3PinCtrlReg_c &= ~PORT_PCR_MUX_MASK;
	gLED_Led3PinCtrlReg_c |= PORT_PCR_MUX(gLED_PinCtrlAltFunc_c);
	gLED_Led4PinCtrlReg_c &= ~PORT_PCR_MUX_MASK;
	gLED_Led4PinCtrlReg_c |= PORT_PCR_MUX(gLED_PinCtrlAltFunc_c);

	/* pins direction -> output */
	gLED_PortDataDirReg_c |= (gLED_Led1Pin_c | gLED_Led2Pin_c | 
			gLED_Led3Pin_c | gLED_Led4Pin_c);

	/* turn off all LEDs */
	gLED_PortDataOutReg_c |= (gLED_Led1Pin_c | gLED_Led2Pin_c | 
			gLED_Led3Pin_c | gLED_Led4Pin_c);
        /* allocate a timer for use in flashing LEDs */
        mLEDTimerID = TMR_AllocateTimer();
}                   /* LED_Init() */

/******************************************************************************
 * LED_UnInit
 * 
 * Turns off all the LEDs and disables clock gating for LED port
 ******************************************************************************/
void LED_UnInit
(
  void
)
{
	/* turn off all LEDs */
	gLED_PortDataOutReg_c &= ~(gLED_Led1Pin_c | gLED_Led2Pin_c | 
			gLED_Led3Pin_c | gLED_Led4Pin_c);

	/* disable clock */
	gLED_ClkGatingReg_c &= ~gLED_ClkGatingBit_c;
}                 /* LED_UnInit() */

/******************************************************************************
 * LED_TurnOnLed
 * 
 * Turns ON the specified LED(s) 
 ******************************************************************************/
void LED_TurnOnLed
(
  LED_t LEDNr     /* IN: LED number(s) to be turned ON */
)
{
	if (LEDNr & LED1)
		Led1On(); /* turn ON LED 1 */
	if (LEDNr & LED2)
		Led2On(); /* turn ON LED 2 */
	if (LEDNr & LED3)
		Led3On(); /* turn ON LED 3 */
	if (LEDNr & LED4)
		Led4On(); /* turn ON LED 4 */
}                 /* LED_TurnOnLed() */

/******************************************************************************
 * LED_TurnOffLed
 * 
 * Turns OFF the specified LED(s)
 ******************************************************************************/
void LED_TurnOffLed
(
  LED_t LEDNr     /* IN: LED number(s) to be turned OFF */
)
{
	if (LEDNr & LED1)
		Led1Off(); /* turn OFF LED 1 */
	if (LEDNr & LED2)
		Led2Off(); /* turn OFF LED 2 */
	if (LEDNr & LED3)
		Led3Off(); /* turn OFF LED 3 */
	if (LEDNr & LED4)
		Led4Off(); /* turn OFF LED 4 */
}             /* LED_TurnOffLed() */

/******************************************************************************
 * LED_ToggleLed 
 * 
 * Toggles the specified LED(s)
 ******************************************************************************/
void LED_ToggleLed
(
  LED_t LEDNr     /* IN: LED number(s) to be toggled */
)
{
	if (LEDNr & LED1)
		Led1Toggle(); /* toggle LED 1 */
	if (LEDNr & LED2)
		Led2Toggle(); /* toggle LED 2 */
	if (LEDNr & LED3)
		Led3Toggle(); /* toggle LED 3 */
	if (LEDNr & LED4)
		Led4Toggle(); /* toggle LED 4 */
}             /* LED_ToggleLed() */

/******************************************************************************
 * LED_TurnOffAllLeds
 * 
 * Turns OFF all LEDs
 ******************************************************************************/
void LED_TurnOffAllLeds
(
  void
)
{
  LED_TurnOffLed(LED_ALL);
}           /* LED_TurnOffAllLeds() */

/******************************************************************************
 * Name: LED_TurnOnAllLeds
 * 
 * Turns ON all LEDs
 ******************************************************************************/
void LED_TurnOnAllLeds
(
  void
)
{
  LED_TurnOnLed(LED_ALL);
}           /* LED_TurnOnAllLeds() */

/******************************************************************************
 * Name: LED_StopFlashingAllLeds
 * 
 * Stops flashing and turns OFF all LEDs
 ******************************************************************************/
void LED_StopFlashingAllLeds
(
  void
)
{
  LED_SetLed(LED_ALL, gLedOff_c);
}           /* LED_StopFlashingAllLeds() */         

/******************************************************************************
 * LED_StartFlash
 *
 * Starts flashing one or more LEDs
 *******************************************************************************/
void LED_StartFlash
(
  LED_t LEDNr     /* IN: LED Number (may be an OR of the list)*/
)
{
    /* indicate which LEDs are flashing */
    mLedFlashingLEDs |= LEDNr;
    
    /* start the timer */
    TMR_StartIntervalTimer(mLEDTimerID, mLEDInterval_c, LED_FlashTimeout);
}            /* LED_StartFlash() */ 

#if gLEDBlipEnabled_d
/******************************************************************************
 * LED_StartBlip
 * 
 * Set up for blinking one or more LEDs once 
 ******************************************************************************/
void LED_StartBlip
(
  LED_t LEDNr     /* IN: LED Number (may be an OR of the list)*/
)
{
    uint8_t iLedIndex;

    /* set up for blinking one or more LEDs once */
    for(iLedIndex = 0; iLedIndex < gLEDsOnTargetBoardCnt_c; ++iLedIndex) {
            if(LEDNr & (1 << iLedIndex))
                    mLedBlips[iLedIndex] = 2;   /* blink on, then back off */
    }

    /* start flashing */
    LED_StartFlash(LEDNr);
}               /* LED_StartBlip() */               
#endif

/******************************************************************************
 * LED_StopFlash
 * 
 * Stop an LED from flashing. 
 ******************************************************************************/
void LED_StopFlash
(
  LED_t LEDNr     /* IN: LED Number (may be an OR of the list)*/
)
{
    /* leave stopped LEDs in the off state */
    LED_TurnOffLed(LEDNr);

    /* stop flashing on one or more LEDs */
    mLedFlashingLEDs &= (~LEDNr);

    /* if ALL LEDs have stopped flashing, then stop timer */
    if(!mLedFlashingLEDs)
    TMR_StopTimer(mLEDTimerID);
}             /* LED_StopFlash() */

/******************************************************************************
 * LED_StartSerialFlash
 * 
 * Starts serial flashing LEDs
 ******************************************************************************/
void LED_StartSerialFlash
(
  void
)
{
    /* indicate going into flashing mode (must be done first) */
    LED_StartFlash(0);

    /* set up for serial lights */
    LED_TurnOffAllLeds();
    Led1On();
    mLedFlashingLEDs = LED1 | LED2; /* toggle these to on first flash */

    /* indicate in serial flash mode */
    mfLedInSerialMode = TRUE;
}             /* LED_StartSerialFlash() */

/******************************************************************************
 * Name: LED_SetHex
 * 
 * Sets a specified hex value on the LEDs 
 ******************************************************************************/
void LED_SetHex
(
  uint8_t hexValue    /* IN: the value to be set on LEDs*/
)
{
    LED_SetLed(LED_ALL, gLedOff_c);
    LED_SetLed(hexValue, gLedOn_c);
}             /* LED_SetHex() */

/******************************************************************************
 * LED_SetLed
 *   
 * This function can set the specified LED(s) in one of the  following states: 
 * On, Off, Toggle, Flashing or StopFlash 
 ******************************************************************************/
void LED_SetLed
(
  LED_t LEDNr,        /* IN: LED(s) to be set*/
  LedState_t state    /* IN: one of the possible states listed above*/
)
{
    /* turning off flashing same as off state */
    if(state == gLedStopFlashing_c)
            state = gLedOff_c;

    /* turn off serial lights if in serial mode */
    LED_ExitSerialFlash();

    /* flash LED */
    if(state == gLedFlashing_c)
    LED_StartFlash(LEDNr);

#if gLEDBlipEnabled_d
    /* blink LEDs */
    else if(state == gLedBlip_c)
            LED_StartBlip(LEDNr);
#endif

    /* On, Off or Toggle Led*/
    else {

        /* on, off or toggle will stop flashing on the LED. Also exits serial mode */
        if(mfLedInSerialMode || (mLedFlashingLEDs & LEDNr))
                LED_StopFlash(LEDNr);

        /* Select the operation to be done on the port */
        if(state == gLedOn_c)
                LED_TurnOnLed(LEDNr);
        if(state == gLedOff_c)
                LED_TurnOffLed(LEDNr);
        if(state == gLedToggle_c)
                LED_ToggleLed(LEDNr);
    }
}               /* LED_SetLed() */

/******************************************************************************
 *******************************************************************************
 * Private functions
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 * LED_ExitSerialFlash
 * 
 * Stop serial flashing mode, if serial flash mode is active. Turns all LEDs off.
 ******************************************************************************/
static void LED_ExitSerialFlash
(
  void
)
{
    if(mfLedInSerialMode) 
    {
        mLedFlashingLEDs = 0;   /* no LEDs are currently flashing */
        LED_TurnOffAllLeds();
        TMR_StopTimer(mLEDTimerID);
        mfLedInSerialMode = FALSE;
    }
}

#if gLEDBlipEnabled_d
/******************************************************************************
 * LED_DecrementBlip
 *
 * Decrements the blink count
 ******************************************************************************/
static void LED_DecrementBlip(void)
{
    uint8_t iLedIndex;	

    for(iLedIndex = 0; iLedIndex < gLEDsOnTargetBoardCnt_c; ++iLedIndex) 
    {
            if(mLedBlips[iLedIndex]) 
            {
                    --mLedBlips[iLedIndex];
                    if(!mLedBlips[iLedIndex])
                    {
                            mLedFlashingLEDs &= ~(1 << iLedIndex);
                    }
            }
    }

    /* done, stop the timer */
    if(!mLedFlashingLEDs)
    {
            TMR_StopTimer(mLEDTimerID);
    }
}
#endif

/******************************************************************************
 * LED_FlashTimeout
 * 
 * Timer callback function that is called each time the timer expires
 ******************************************************************************/
static void LED_FlashTimeout
(
  tmrTimerID_t timerId /* IN: TimerID. */
)
{
    (void)timerId;  /* prevent compiler warning */

    if(mLedFlashingLEDs & LED1)
            Led1Toggle();
    if(mLedFlashingLEDs & LED2)
            Led2Toggle();
    if(mLedFlashingLEDs & LED3)
            Led3Toggle();
    if(mLedFlashingLEDs & LED4)
            Led4Toggle();

#if gLEDBlipEnabled_d
    /* decrement blips */
    LED_DecrementBlip();
#endif

    /* if serial lights, move on to next light */
    if(mfLedInSerialMode)
    {
            mLedFlashingLEDs = mLedFlashingLEDs << 1;
            if(mLedFlashingLEDs & (LED4 << 1))  /* wrap around */
            {
                    mLedFlashingLEDs &= LED_ALL;
                    mLedFlashingLEDs |= LED1;
            }
    }
}

#endif /* gLEDSupported_d */
