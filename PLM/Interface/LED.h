/******************************************************************************
 * Public header file for the LED driver.
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

#ifndef _LED_H_
#define _LED_H_

#include "EmbeddedTypes.h"
#include "PortConfig.h"
#include "TMR_Interface.h"

#ifndef gLEDSupported_d
#define gLEDSupported_d     (TRUE)
#endif


#if (gLEDSupported_d == TRUE)

/*
 * Name: gLEDsOnTargetBoardDefault_c
 * Description: Default value for LEDs count located on a 
 *              generic target board
 */
#define gLEDsOnTargetBoardDefault_c		4

/*
 * Name: gLEDsOnBoardCnt
 * Description: Specifies the number of physical LEDs on the target board
 */
#ifndef gLEDsOnTargetBoardCnt_c
#define gLEDsOnTargetBoardCnt_c		gLEDsOnTargetBoardDefault_c
#endif

/*
 * Name: gLEDBlipEnabled_d
 * Description: Specifies if blip (blink) is supported by the LED module
 */
#ifndef gLEDBlipEnabled_d
#define gLEDBlipEnabled_d	(TRUE)
#endif


/*
 * Name: mLEDInterval_c
 * Description: Interval time (milliseconds) for toggle LED which is used 
 *              for flashing LED 
 */

#ifndef mLEDInterval_c
#define mLEDInterval_c		100
#endif

/*
 * Macros to turn ON, OFF and TOGGLE LEDs
 */

#define Led1Off()		(gLED_PortDataSetReg_c |= gLED_Led1Pin_c)     
#define Led2Off()		(gLED_PortDataSetReg_c |= gLED_Led2Pin_c)
#define Led3Off()		(gLED_PortDataSetReg_c |= gLED_Led3Pin_c)
#define Led4Off()		(gLED_PortDataSetReg_c |= gLED_Led4Pin_c)

#define Led1On()	        (gLED_PortDataClearReg_c |= gLED_Led1Pin_c)
#define Led2On()		(gLED_PortDataClearReg_c |= gLED_Led2Pin_c)
#define Led3On()		(gLED_PortDataClearReg_c |= gLED_Led3Pin_c)
#define Led4On()		(gLED_PortDataClearReg_c |= gLED_Led4Pin_c)

#define Led1Toggle()		(gLED_PortDataToggleReg_c |= gLED_Led1Pin_c)
#define Led2Toggle()		(gLED_PortDataToggleReg_c |= gLED_Led2Pin_c)
#define Led3Toggle()		(gLED_PortDataToggleReg_c |= gLED_Led3Pin_c)
#define Led4Toggle()		(gLED_PortDataToggleReg_c |= gLED_Led4Pin_c)

/*
 *  Turn OFF LEDs macro
 */
#define TurnOffLeds()       	LED_TurnOffAllLeds()

/* 
 * Turn ON LEDs macro
 */
#define TurnOnLeds()        	LED_TurnOnAllLeds()

/* 
 * Start Serial flashing macro 
 */
#define SerialFlashing()    	LED_StartSerialFlash()

/* Flashing LED macros */
#define Led1Flashing()      	LED_StartFlash(LED1)
#define Led2Flashing()          LED_StartFlash(LED2)
#define Led3Flashing()      	LED_StartFlash(LED3)
#define Led4Flashing()      	LED_StartFlash(LED4)
#define StopLed1Flashing()  	LED_StopFlash(LED1)
#define StopLed2Flashing()  	LED_StopFlash(LED2)
#define StopLed3Flashing()  	LED_StopFlash(LED3)
#define StopLed4Flashing()  	LED_StopFlash(LED4)

/*
 * LEDs mapping
 */
#define LED1          0x01
#define LED2          0x02
#define LED3          0x04
#define LED4          0x08
#define LED_ALL       0x0F
#define LED1_FLASH    0x10
#define LED2_FLASH    0x20
#define LED3_FLASH    0x40
#define LED4_FLASH    0x80

/******************************************************************************
 *******************************************************************************
 * Public type definitions
 *******************************************************************************
 ******************************************************************************/

/*
 * Name: LED_t
 * Description: LED type definition 
 */
typedef uint8_t LED_t;

/*
 * Name: LED_OpMode_t
 * Description: enumerated data type for all possible LED operation modes    
 */
typedef enum LED_OpMode_tag{
	gLedFlashing_c,       /* flash at a fixed rate */
	gLedStopFlashing_c,   /* same as gLedOff_c */
	gLedBlip_c,           /* just like flashing, but blinks only once */
	gLedOn_c,             /* on solid */
	gLedOff_c,            /* off solid */
	gLedToggle_c          /* toggle state */
} LED_OpMode_t;

/*
 * Name: LedState_t
 * Description: possible LED states for LED_SetLed()
 * Note: all LEDs can operate independently    
 */
typedef uint8_t LedState_t;


/******************************************************************************
 *******************************************************************************
 * Public prototypes
 *******************************************************************************
 ******************************************************************************/

/******************************************************************************
 * LED_Init initializes the LED module
 *
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_Init
(
  void
);

/******************************************************************************
 * LED_UnInit un-initilizes the LED module. turns off all the LEDs and disables 
 * clock gating for LED port.
 *
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_UnInit
(
  void
);

/******************************************************************************
 * LED_TurnOnLed turns ON the specified LED(s)
 * 
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_TurnOnLed
(
  LED_t LEDNr     /* IN: LED number(s) to be turned ON*/
);

/******************************************************************************
 * LED_TurnOffLed turns OFF the specified LED(s)
 *  
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_TurnOffLed
(
  LED_t LEDNr     /*IN: LED number(s) to be turned OFF*/
);

/******************************************************************************
 * LED_ToggleLed toggles the specified LED(s)
 *  
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_ToggleLed
(
  LED_t LEDNr     /*IN: LED number(s) to be toggled*/
);

/******************************************************************************
 * LED_TurnOffAllLeds turns OFF all LEDs
 *  
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_TurnOffAllLeds
(
  void
);

/******************************************************************************
 * LED_TurnOnAllLeds turns ON all LEDs
 *  
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_TurnOnAllLeds
(
  void
);

/******************************************************************************
 * LED_StopFlashingAllLeds stops flashing and turns OFF all LEDs
 *  
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_StopFlashingAllLeds
(
  void
);

/******************************************************************************
 * LED_StartFlash starts flashing one or more LEDs
 *  
 * Interface assumptions:
 *   The parameter used should be LEDn_FLASH definitions.
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_StartFlash
(
  LED_t LEDNr       /*IN: LED Number (may be an OR of the list)*/
);

#if gLEDBlipEnabled_d

/******************************************************************************
 * LED_StartBlip set up for blinking one or more LEDs once
 *  
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_StartBlip
(
  LED_t LEDNr      /*IN: LED Number (may be an OR of the list)*/
);
#endif
/******************************************************************************
 * LED_StopFlash stops a LED from flashing.
 *  
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_StopFlash
(
  LED_t LEDNr     /*IN: LED Number (may be an OR of the list)*/
);

/******************************************************************************
 * LED_StartSerialFlash starts serial flashing LEDs
 *  
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_StartSerialFlash
(
  void
);

/******************************************************************************
 * LED_SetHex sets a specified hex value on the LEDs
 *  
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_SetHex
(
uint8_t hexValue    /* IN: the value to be set on LEDs*/
);

/******************************************************************************
 * LED_SetLed sets the specified LED(s) in one of the  
 * following states: On, Off, Toggle, Flashing or StopFlash
 *  
 * Interface assumptions:
 *   NONE
 *
 * return value:
 *   NONE
 *   
 * Effects on global data:
 *   NONE
 ******************************************************************************/
extern void LED_SetLed
(
  LED_t LEDNr,        /* IN: LED(s) to be set*/
  LedState_t state    /* IN: one of the possible states listed above*/
);

#else /* LEDs not supported */

/* define empty prototypes */
#define LED_Init()
#define LED_TurnOffLed(LEDNr)
#define LED_TurnOnLed(LEDNr)
#define LED_ToggleLed(LEDNr)
#define LED_StartFlash(LEDNr)
#define LED_StopFlash(LEDNr)
#define LED_StartSerialFlash()
#define LED_TurnOffAllLeds()
#define LED_TurnOnAllLeds()
#define LED_StopFlashingAllLeds()
#define LED_SetLed(LEDNr,state)
#define LED_SetHex(hexValue)

#define Led1On()
#define Led1Off()
#define Led1Toggle()

#define Led2On()
#define Led2Off()
#define Led2Toggle()

#define Led3On()
#define Led3Off()
#define Led3Toggle()

#define Led4On()
#define Led4Off()
#define Led4Toggle()

#define Led1Flashing()
#define Led2Flashing()
#define Led3Flashing()
#define Led4Flashing()
#define StopLed1Flashing()
#define StopLed2Flashing()
#define StopLed3Flashing()
#define StopLed4Flashing()
#define TurnOffLeds()
#define TurnOnLeds()
#define SerialFlashing()

#endif /* gLEDSupported_d */

#endif /* _LED_H_ */
