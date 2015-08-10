/*****************************************************************************
* SMAC Interface header file
* 
* Copyright (c) 2012, Freescale, Inc. All rights reserved.
*
* 
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/


#ifndef __SMAC_INTERFACE_H__               
  #define __SMAC_INTERFACE_H__

#include "EmbeddedTypes.h"  // include special data types


/************************************************************************************
*************************************************************************************
* Interface macro definitions
*************************************************************************************
************************************************************************************/

/*If gPromiscuousMode_d is set to TRUE the SMAC init will set the 
  mPromiscuousModeEnabled flag at the initialization*/
#ifndef gPromiscuousMode_d
  #define gPromiscuousMode_d FALSE
#endif




/*The max lengt of the SMAC service data unit(Max SMAC's Payload)*/
#define gMaxSmacSDULength_c            (123)   

/*The max lengt of the SMAC service data unit in promiscuous mode
(Max SMAC's Payload)*/
#define gMaxPromiscuousSmacSDULength_c (125)

#define gChannelOutOfRange_c gTotalChannels_c

#define gMinOutputPower_c       (0x03)
#define gMaxOutputPower_c       (0x1F)
#define gGainOffset_c		(0x9B)

#define gSwSmacVersionLength_c (7)
#define gHwIcVersionLength_c   (2)

/************************************************************************************
*************************************************************************************
* Interface Type definitions
*************************************************************************************
************************************************************************************/

typedef struct smacPdu_tag {
  uint8_t  reserved[2];
  char  u8Data[1];
}smacPdu_t;

typedef struct txPacket_tag
{
  uint8_t   u8DataLength;
  smacPdu_t smacPdu;
}txPacket_t;

typedef enum txStatus_tag
{
  txSuccessStatus_c = 0, 
  txFailureStatus_c,
  txMaxStatus_c
} txStatus_t;

typedef enum rxStatus_tag
{
  rxInitStatus = 0,
  rxProcessingReceptionStatus_c,
  rxSuccessStatus_c, 
  rxTimeOutStatus_c,
  rxAbortedStatus_c,
  rxMaxStatus_c
} rxStatus_t;

typedef struct rxPacket_tag
{
  uint8_t 	 lqi;
  uint8_t    u8MaxDataLength;
  rxStatus_t rxStatus;
  uint8_t    u8DataLength;
  smacPdu_t  smacPdu;
}rxPacket_t;

typedef enum smacErrors_tag
{
  gErrorNoError_c = 0,
  gErrorBusy_c,
  gErrorOutOfRange_c,
  gErrorNoResourcesAvailable_c,
  gErrorNoValidCondition_c,
  gErrorMaxError_c
} smacErrors_t;

typedef enum channels_tag
{
  gChannel11_c = 0x0B,
  gChannel12_c,
  gChannel13_c,
  gChannel14_c,
  gChannel15_c,
  gChannel16_c,
  gChannel17_c,
  gChannel18_c,
  gChannel19_c,
  gChannel20_c,
  gChannel21_c,
  gChannel22_c,
  gChannel23_c,
  gChannel24_c,
  gChannel25_c,
  gChannel26_c,
  gTotalChannels_c
} channels_t;

typedef enum versionedEntity_tag
{
  gSwSmacVersion_c = 0,
  gHwIcVersion_c,
  gMaxVersionedEntity_c
} versionedEntity_t;


typedef enum clkoFrequency_tag
{
  gClko32MHz_c = 0,     // 0 = 32 MHz
  gClko16MHz_c,         // 1 = 16 MHz
  gClko8MHz_c,          // 2 = 8 MHz
  gClko4MHz_c,          // 3 = 4 MHz
  gClko1MHz_c,          // 4 = 1 MHz
  gClko250kHz_c,        // 5 = 250 kHz        
  gClko62p5kHz_c,       // 6 = 62.5 kHz
  gClko32p787kHz_c,     // 7 = 32.787 kHz
  gClkoDisable_c,
  gClkoOutOfRange_c
} clkoFrequency_t;

  typedef enum timerTimeBase_tag
  {
                              // 000: Reserved
                              // 001: Reserved       
    gTimeBase500kHz_c = 2,    // 010: 500kHz 
    gTimeBase250kHz_c,        // 011: 250kHz
    gTimeBase125kHz_c,        // 100: 125kHz
    gTimeBase62p5kHz_c,       // 101: 62.5kHz 
    gTimeBase31p25kHz_c,      // 110: 31.25kHz
    gTimeBase15p625kHz_c,     // 111: 15.625kHz
    gMaxTimeBase_c             
  } timerTimeBase_t;



typedef enum smacTestMode_tag  
{
  gTestModeForceIdle_c = 0,
  gTestModeContinuousTxModulated_c,
  gTestModeContinuousTxUnmodulated_c,
  gTestModePRBS9_c,
  gMaxTestMode_c
} smacTestMode_t;


typedef enum scanModes_tag
{
  gScanModeCCA_c = 0,
  gScanModeED_c,
  gMaxScanMode_c
} scanModes_t;



/************************************************************************************
*************************************************************************************
* Memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Interface functions
*************************************************************************************
************************************************************************************/

/***********************************************************************************/
/******************************** SMAC Data primitives *****************************/
/***********************************************************************************/

/************************************************************************************
* MCPSDataRequest
* 
* This data primitive is used to send an over the air packet. This is an asyncronous 
* function,  it means it ask SMAC to transmit one OTA packet,  but when the runction 
* returns it is not sent already.
*
* Interface assumptions:
*   The SMAC and radio driver have been initialized and are ready to be used. 
*
* Return value:  
*   gErrorNoError_c: Everything is ok and the transmission will be performed.
*   gErrorOutOfRange_c: One of the members in the pTxMessage structure is out of 
*                      range (no valid bufer size or data buffer pointer is NULL)
*   gErrorNoResourcesAvailable_c: the radio is performing another action.
*   gErrorNoValidCondition_c: The SMAC has not been initialized 
*
************************************************************************************/
extern smacErrors_t MCPSDataRequest
(
txPacket_t *psTxPacket //IN:Pointer to the packet to be transmitted
);

/************************************************************************************
* MLMERXEnableRequest
* 
* Function used to place the radio into receive mode 
*
* Interface assumptions: 
*   The SMAC and radio driver have been initialized and are ready to be used.
*   The timeout time depends on the radio's timer preescaler, the default value is
*   250Khz it is possible to change this value using MLMESetTmrPrescale.
*
* Return value:  
*   gErrorNoError_c: Everything is ok and the reception will be performed.
*   gErrorOutOfRange_c: One of the members in the pRxMessage structure is out of 
*                      range (no valid bufer size or data buffer pointer is NULL).
*   gErrorNoResourcesAvailable_c: the radio is performing another action.
*   gErrorNoValidCondition_c: The SMAC has not been initialized.
*
************************************************************************************/
extern smacErrors_t MLMERXEnableRequest
(
rxPacket_t *gsRxPacket, //OUT: Pointer to the structure where the reception results 
                        //     will be stored.
uint32_t u32Timeout     //IN:  32-bit timeout value, this is directly the value that 
                        //     is stored on the Radio's timer register.
); 

/************************************************************************************
* MLMERXDisableRequest
* 
* Returns the radio to idle mode from receive mode.
*
* Interface assumptions: 
*
* Return value: 
*   gErrorOutOfRange_c If the Radio is not in Rx state.
*   gErrorNoError_c When the message was aborted or disabled. 
*
************************************************************************************/
extern smacErrors_t MLMERXDisableRequest(void); 

/************************************************************************************
* MLMELinkQuality
* 
* This  function  returns 0-255 value  that is the link quality from the last 
* received packet.
*
* Interface assumptions: 
*
* Return value: 
*   gErrorNoError_c: Everything is ok and the reception will be performed.
*   gErrorNoValidCondition_c: The SMAC has not been initialized.
*
************************************************************************************/
extern smacErrors_t MLMELinkQuality
(
uint8_t * u8LinkQuality /* OUT: Pointer to the variable where the Link Quality 
                                will be store. */
);



/***********************************************************************************/
/******************************** SMAC Radio primitives ****************************/
/***********************************************************************************/

/************************************************************************************
* MLMERadioInit
* 
* This function initializes the Radio parameters.
*
* Interface assumptions: 
*
* Return value:
*   gErrorNoError_c: the Radio initialization has been done succesfuly.
*   gErrorNoResourcesAvailable_c: the Radio initialization couldn't be performed.
*
************************************************************************************/
extern smacErrors_t MLMERadioInit(void);

/************************************************************************************
* MLMEDozeRequest
* 
* Doze request allow the user to put the radio either in Normal Doze Mode (without 
* CLKO but with automatic wake up) or Acoma Doze Mode (with CLKout, but without 
* timeout). 
* This primitive is used just on the MC1320x and MC1321x platforms, for 
* the MC1323x radio it is just an stub.
*
* Interface assumptions: 
*
* Return value: 
*   gErrorBusy_c: when SMAC is busy in other radio activity as transmitting  or 
*                receiving data. Or performing a channel scan.
*   gErrorNoError_c: the Radio has been set in Doze mode.
*
************************************************************************************/
extern smacErrors_t MLMEDozeRequest
(
uint32_t u32Timeout /* IN: Is the automatic wake up time for this mode. */
); 

/************************************************************************************
* MLMEEnergyDetect
* 
* This call starts an energy detect(ED) cycle, it returns the energy value (-power)
* via the returned argument. For example, if the Energy Detect returns 80 then the 
* interpreted value is -80 dBm.
*
* Interface assumptions: 
*
* Return value: 
*   uint8_t: An unsigned 8-bit value representing the energy on the current channel.
*
************************************************************************************/
extern smacErrors_t MLMEEnergyDetect
(
uint8_t * u8EnergyValue /* OUT: Pointer to the variable where the Energy value 
                                will be store. */
);

/************************************************************************************
* MLMEFEGainAdjust
* 
* This is a compensator for the energy detection and AGC. In case users need to 
* calibrate the readings due to a specific application like an external low noise 
* amplifier, this is where users can set the offset.
* This primitive is used just on the MC1320x and MC1321x platforms, for the 
* MC1323x radio it is just an stub.
*
* Interface assumptions: 
*
* Return value: 
*   gErrorNoError_c: the power compensation value have been set on the radio.
*
************************************************************************************/
extern smacErrors_t MLMEFEGainAdjust
(
uint8_t u8GainValue //IN: 8 bit value for the gain adjust.
); 

/************************************************************************************
* MLMESetChannelRequest
* 
* This function sets the frequency on which the radio will transmit or receive on.
*
* Interface assumptions: 
*
* Return value: 
*   gErrorNoError_c: The channel set has been performed
*   gErrorOutOfRange_c : channel requested is not valid
*   gErrorBusy_c: when SMAC is busy in other radio activity as transmitting  or
*                receiving data. Or performing a channel scan.
*
************************************************************************************/
extern smacErrors_t MLMESetChannelRequest
(
channels_t newChannel //IN: Enumeration value that represents the requested channel. 
);

/************************************************************************************
* MLMEGetChannelRequest
* 
* This function returns the current channel, if an error is detected it returns 
* gChannelOutOfRange_c.
*
* Interface assumptions: 
*
* Return value: 
*   gChannel11_c to gChannel26_c:     The current RF channel.
*   gChannelOutOfRange_c: If current channel could not be detected.
*
************************************************************************************/
extern channels_t MLMEGetChannelRequest(void);

/************************************************************************************
* MLMEHibernateRequest
* 
* This call places the radio into Hibernate mode. 
* On the MC1320x and MC1321x platforms it sets the radio to hebernate 
* operation mode, for the MC1323x radio it disables the clock source for the 
* tranceiver module.
*
* Interface assumptions: 
*
* Return value: 
*   gErrorBusy_c: when SMAC is busy in other radio activity as transmitting  or 
*                receiving data. Or performing a channel scan.
*   gErrorNoError_c: the Radio has been set in Hibernate mode.
*
************************************************************************************/
extern smacErrors_t MLMEHibernateRequest(void);

/************************************************************************************
* MLMEPAOutputAdjust
* 
* This function adjusts the output power of the transmitter. 
*
* Interface assumptions: 
*
* Return value:
*   gErrorOutOfRange_c: If u8Power exceeds the maximum power value (0x13).
*   gErrorNoError_c: If the action is performed.
*
************************************************************************************/
extern smacErrors_t MLMEPAOutputAdjust
(
uint8_t u8PaValue //IN: Value for the output power desired. Values 0-15 are required.
);

/************************************************************************************
* MLMEPHYXtalAdjust
* 
* This function is used to adjust the radio reference clock by a trim value.
*
* Interface assumptions: 
*
* Return value:
*   gErrorOutOfRange_c: If TrimValue exceeds the maximum trim value.
*   gErrorNoError_c: If the action is performed.
*
************************************************************************************/
extern smacErrors_t MLMEXtalAdjust
(
uint8_t u8ReqValue //IN: value representing the trim value to the oscillator.
);

/************************************************************************************
* MLMESoftReset
* 
* This function performs a software reset on the radio,  PHY and SMAC state machines.
*
* Interface assumptions: 
*
* Return value: 
*   gErrorNoError_c: If the action is performed.
*
************************************************************************************/
extern smacErrors_t MLMESoftReset(void);

/************************************************************************************
* MLMESetClockRate
* 
* This function is called to set the desired clock out from radio.
* This primitive is used just on the MC1320x and MC1321x platforms, for the 
* MC1323x radio it is just an stub.
*
* Interface assumptions: 
*
* Return value: 
*   gErrorOutOfRange_c: If Freq exceeds the maximum  clock out frequency value.
*   gErrorNoError_c If the action is performed.
*
************************************************************************************/
extern smacErrors_t MLMESetClockRate
(
clkoFrequency_t clkoFreq  //IN: Enumeration value that represents radio clock out 
                      //    frequency (CLKO). 
); 


/************************************************************************************
* MLMESetTmrPrescale
* 
* This function changes the rate at which the radio timers operate.
*
* Interface assumptions: 
*
* Return value: 
*   gErrorOutOfRange_c: If TimeBase exceeds the maximum  valid  value.
*   gErrorNoError_c: If the action is performed.
*
************************************************************************************/
extern smacErrors_t MLMESetTmrPrescale
(
timerTimeBase_t TimeBase //IN: enumeration value that represents that represents  
                         //    timer prescale or time base.
); 

/************************************************************************************
* MLMEWakeRequest
* 
* This primitive brings the radio out of low power mode.
*
* Interface assumptions: 
*
* Return value:
*   gErrorNoValidCondition_c: If the radio was not in low power mode.
*   gErrorNoError_c: If the action is performed.
*
************************************************************************************/
extern smacErrors_t MLMEWakeRequest(void); 

/************************************************************************************
* XCVRContReset
* 
* This function asserts the reset line of the transceiver, shutting it down to its 
* lowest power mode.
* This primitive is used just on the MC1320x and MC1321x platforms, for the
* MC1323x radio it is just an stub.
*
* Interface assumptions: 
*
* Return value: 
*   void
*
************************************************************************************/
extern void XCVRContReset(void);

/************************************************************************************
* XCVRRestart
* 
* This function deasserts the reset line, thus it power the transceiver up.
* This primitive is used just on the MC1320x and MC1321x platforms, for the
* MC1323x radio it is just an stub.
*
* Interface assumptions: 
*
* Return value: 
*   void
*
************************************************************************************/
extern void XCVRRestart(void);




/***********************************************************************************/
/***************************** SMAC management primitives **************************/
/***********************************************************************************/

/************************************************************************************
* MLMEGetRficVersion
* 
* This function is used to read the version number of different hardware and software 
* modules inside the SMAC platform.
*
* Interface assumptions: 
*
* Return value: 
*   gErrorOutOfRange_c: If the requested Entity is not part of the stored ones.
*   gErrorNoError_c: If the action is performed.
*
************************************************************************************/
extern smacErrors_t MLMEGetRficVersion
(
versionedEntity_t Entity, //IN: The module for which the version is required.  
uint8_t *Buffer          //OUT: A pointer to the buffer where the version will be 
                          //     written.
);

/************************************************************************************
* MLMESetPromiscuousMode
* 
* SMAC appends 0xFF7E as the first two bytes of the packet to identify its own 
* packets from other packets in the same PHY layer. When promiscuous mode is off 
* SMAC filters the received packets and just allows those beginning with 0xFF7E; 
* when promiscuous mode is on SMAC let pass all the messages beginning or not with
* 0xFF7E. MLMESetPromiscuousMode allows setting or clearing the promiscuous mode. 
*
* Interface assumptions: 
*
* Return value: 
*   void
*
************************************************************************************/
extern void MLMESetPromiscuousMode
(
bool_t isPromiscousMode //IN: This is a boolean value that indicates if the promiscuous 
                        //    mode is on (TRUE) or off (FALSE).
);

/************************************************************************************
* MLMEGetPromiscuousMode
* 
* This function returns the current state of promiscuous mode.
*
* Interface assumptions: 
*
* Return value:
*   TRUE :If promiscuous mode is active.
*   FALSE :If promiscuous mode is inactive.
*
************************************************************************************/
extern bool_t MLMEGetPromiscuousMode(void);

/************************************************************************************
* MLMEScanRequest
* 
* This function scans the different channels using one out of two techniques and 
* returns the amount of energy in all of the channels.
*
* Interface assumptions: 
*
* Return value:
*   gErrorBusy_c: when SMAC is busy in other radio activity as transmitting  or 
*                receiving data. Or performing a channel scan.
*   gErrorNoError_c: the Radio has been set in Doze mode.
*
************************************************************************************/
extern smacErrors_t MLMEScanRequest
(
uint16_t u16ChannelsToScan, //IN: Bitmap of the Channels to be scanned.
scanModes_t ScanMode, //IN: Technique to be used CCA or ED.
uint8_t *pu8ChannelScan //OUT: The buffer where the values of the scan will be 
                        //     returned. 
);

/************************************************************************************
* MLMETestMode
* 
* By employing this function, users can execute a test of the radio. Test mode 
* implements the following:
*   -PRBS9 Mode, 
*   -Force_idle, 
*   -Continuos RX, 
*   -Continuos TX without modulation, 
*   -Continuos TX with modulation.
*
* Interface assumptions: 
*
* Return value: 
*   gErrorBusy_c: when SMAC is busy in other radio activity as transmitting  or 
*                receiving data. Or performing a channel scan.
*   gErrorNoError_c: the Radio has been set in the selected test mode.
*
************************************************************************************/
extern smacErrors_t MLMETestMode
(
smacTestMode_t  mode  //IN: The test mode to start.
);

/***********************************************************************************/
/******************************** SMAC MCU primitives ******************************/
/***********************************************************************************/

/************************************************************************************
* SMACEnableInterrupts
* 
* This function enables the interrupts that are used by SMAC.
*
* Interface assumptions: 
*
* Return value: 
*   void
*
************************************************************************************/
extern void SMACEnableInterrupts(void);

/************************************************************************************
* SMACDisableInterrupts
* 
* This function disables the interrupts that are used by SMAC.
*
* Interface assumptions: 
*
* Return value: 
*
************************************************************************************/
extern void SMACDisableInterrupts(void);


#endif //__SMAC_INTERFACE_H__

  
