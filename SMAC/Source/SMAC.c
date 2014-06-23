/*****************************************************************************
* SMAC implementation.
* 
* Copyright (c) 2012, Freescale, Inc. All rights reserved.
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#include "SMAC.h"


/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

const uint8_t au8SmacVersion[7] = 
{
  mMajorSmacV_c, 
  mMiddleSmacV_c, 
  mMinorSmacV_c, 
  mMonthSmacV_c,
  mDaySmacV_c, 
  mYearSmacV_c, 
  mConsSmacV_c
};
                                
static uint32_t        smacTimeout;
static uint16_t        smacChannelsBeingScanned;
static uint8_t         smacEdValue;
static smacStates_t    smacState;
static phyRxParams_t   smacLastDataRxParams;
static channels_t      smacCurrentChannel;
static channels_t      smacScanCurrentChannel;
static channels_t      smacClearestChann;
static uint8_t         u8TestBuffer[65] = 
{ 
  0x42,
  0xff,0xc1,0xfb,0xe8,0x4c,0x90,0x72,0x8b,0xe7,0xb3,0x51,0x89,0x63,0xab,0x23,0x23,  
  0x02,0x84,0x18,0x72,0xaa,0x61,0x2f,0x3b,0x51,0xa8,0xe5,0x37,0x49,0xfb,0xc9,0xca,
  0x0c,0x18,0x53,0x2c,0xfd,0x45,0xe3,0x9a,0xe6,0xf1,0x5d,0xb0,0xb6,0x1b,0xb4,0xbe,
  0x2a,0x50,0xea,0xe9,0x0e,0x9c,0x4b,0x5e,0x57,0x24,0xcc,0xa1,0xb7,0x59,0xb8,0x87
};
static phyPacket_t     *pTestPacket;  

volatile static prssPacketPtr_t smacProccesPacketPtr;

flagsRegister_t smacFlags;

static const uint16_t Local_pll_frac[16] = {0x2400, 0x4C00, 0x7400, 0x9C00, 0xC400, 0xEC00, 0x1400, 0x3C00, 0x6400, 0x8C00, 0xB400, 0xDC00, 0x0400, 0x2C00, 0x5400, 0x7C00};


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
************************************************************************************/
smacErrors_t MCPSDataRequest
(
txPacket_t *psTxPacket /* IN:Pointer to the packet to be transmitted */
)
{
  
  uint8_t u8PhyRes = 0; 
  
#if (TRUE == smacParametersValidation_d)
  uint8_t u8MaxLen=0;
  
  if (mPromiscuousModeEnabled)
  {
    u8MaxLen = gMaxPromiscuousSmacSDULength_c;
  }
  else
  {
    u8MaxLen = gMaxSmacSDULength_c;
  }
  
  if((NULL == psTxPacket) || (u8MaxLen < psTxPacket->u8DataLength))
  {
    return gErrorOutOfRange_c;
  } 
#endif

#if(TRUE == smacInitializationValidation_d)
 if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
#endif
  if(mSmacStateIdle_c != smacState)
  {
    return gErrorBusy_c;
  }
  
  smacProccesPacketPtr.smacTxPacketPointer  = SmacFormatTxPacket(psTxPacket);

  u8PhyRes = PhyPdDataRequest(smacProccesPacketPtr.smacTxPacketPointer, gDataReq_NoAck_NoCca_Unslotted_c, NULL);

  if(gPhySuccess_c == u8PhyRes)
  {
    smacState= mSmacStateTransmitting_c; 
    return gErrorNoError_c;
  }
  else
  {
    return gErrorNoResourcesAvailable_c;
  }
}


/************************************************************************************
* MLMERXEnableRequest
* 
* Function used to configure the radio into receive mode 
*
************************************************************************************/
smacErrors_t MLMERXEnableRequest
(
rxPacket_t *gsRxPacket, /* OUT: Pointer to the structure where the reception results 
                                will be store. */
uint32_t u32Timeout     /* IN:  32-bit timeout value, this is directly the value that 
                                is stored on the Radio's timer register.*/
)
{

  uint8_t u8PhyRes = 0; 
  phyPacket_t * pAuxPacket;

#if(TRUE == smacParametersValidation_d)
  uint8_t u8MaxLen=0;
#endif
  
  if (mPromiscuousModeEnabled)
  {
#if(TRUE == smacParametersValidation_d)
    u8MaxLen = gMaxPromiscuousSmacSDULength_c;
#endif
    pAuxPacket = (phyPacket_t *)&gsRxPacket->smacPdu.reserved[1]; 
  }
  else
  {
#if(TRUE == smacParametersValidation_d)
    u8MaxLen = gMaxSmacSDULength_c;
#endif
    pAuxPacket = (phyPacket_t *)&gsRxPacket->u8DataLength; 
  }
  
#if(TRUE == smacParametersValidation_d)
  if((NULL == gsRxPacket) || (u8MaxLen < gsRxPacket->u8MaxDataLength))
  {
    return gErrorOutOfRange_c;
  }
#endif
  

#if(TRUE == smacInitializationValidation_d)
 if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
#endif

  if(mSmacStateIdle_c != smacState)
  {
    return gErrorBusy_c;
  }
  
  u8PhyRes = PhyPlmeRxRequest(pAuxPacket, &smacLastDataRxParams);
  
  if(gPhySuccess_c == u8PhyRes)
  {
    if(u32Timeout)
    {
      smacTimeout = u32Timeout; 
      SmacSetRxTimeout((zbClock24_t)smacTimeout);
      mSmacTimeoutAsked = TRUE;
    } 
    else
    {
      mSmacTimeoutAsked = FALSE;
    }

    gsRxPacket->rxStatus = rxProcessingReceptionStatus_c;
    smacProccesPacketPtr.smacRxPacketPointer  = gsRxPacket;

    smacState= mSmacStateReceiving_c; 
    return gErrorNoError_c;
  }
  else
  {
    return gErrorNoResourcesAvailable_c;
  }
}

/************************************************************************************
* MLMERXDisableRequest
* 
* Returns the radio to idle mode from receive mode.
*
************************************************************************************/
smacErrors_t MLMERXDisableRequest(void)
{
#if(TRUE == smacInitializationValidation_d)
 if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
#endif
 
  if((mSmacStateReceiving_c != smacState) && (mSmacStateIdle_c != smacState))
  {
    return gErrorNoValidCondition_c;
  }
  
  PhyAbort();
  
  mSmacTimeoutAsked = FALSE;
  
  smacState= mSmacStateIdle_c;
  return gErrorNoError_c;

}
 

/************************************************************************************
* MLMELinkQuality
* 
* This  function  returns  an  integer  value  from 0 to 255
*
************************************************************************************/
smacErrors_t MLMELinkQuality
(
uint8_t * u8LinkQuality /* OUT: Pointer to the variable where the Link Quality 
                                will be store. */
)
{
#if(TRUE == smacInitializationValidation_d)
 if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
#endif
 
  *(u8LinkQuality) = smacLastDataRxParams.linkQuality;
  
  return gErrorNoError_c;
}



/***********************************************************************************/
/******************************** SMAC Radio primitives ****************************/
/***********************************************************************************/

/************************************************************************************
* MLMERadioInit
* 
* This function initializes the Radio parameters.
*
************************************************************************************/
smacErrors_t MLMERadioInit(void)
{    
  /* Configure the transceiver IRQ_B port */  
  MC1324xDrv_IRQ_PortConfig();
  
  /* Configure the transceiver RST_B port */ 
  MC1324xDrv_RST_B_PortConfig();
  
  /* Transceiver Hard/RST_B RESET */
  MC1324xDrv_RST_B_Assert();
  delayMs(mRst_B_AssertTime_c);
  MC1324xDrv_RST_B_Deassert();
  
  /* Wait for transceiver wakeup from POR iterrupt */
  while(!MC1324xDrv_IsIrqPending()){;};
  
   /* Enable transceiver SPI interrupt request */
  NVIC_EnableIRQ(MC1324x_Irq_Number);
  
  NVIC_SetPriority(MC1324x_Irq_Number, MC1324x_Irq_Priority);
  
  /* Enable the transceiver IRQ_B interrupt request */
  MC1324xDrv_IRQ_Enable();
  
  MC1324xDrv_Set_CLK_OUT_Freq(gCLK_OUT_FREQ_4_MHz);
  
  /* wait until the external reference clock is stable */    
    delayMs(mClkSwitchDelayTime_c);
  
  #if ( (gTargetTWR_K60D100M_d == 1) || (gTargetTWR_K60N512_d == 1) )
  /* If target != mohave. disable transceiver CLK_OUT. */
  MC1324xDrv_Set_CLK_OUT_Freq(gCLK_OUT_FREQ_DISABLE); //TODO
  #endif
  
  /* Init MCU Clock*/
  MCU_InitClock();
    
  PhyInit();
  PhyPpSetPromiscuous(TRUE);
  
  smacCurrentChannel = gChannel11_c;
  smacState= mSmacStateIdle_c;
  smacLastDataRxParams.linkQuality = 0;
  smacLastDataRxParams.timeStamp = 0;
  smacEdValue = 0;
  smacClearestChann = gChannelOutOfRange_c; 
  smacFlags.flags.flag0 = gPromiscuousMode_d;
  smacFlags.flags.flag1 = 0;
  smacFlags.flags.flag2 = 0;
  smacFlags.flags.flag3 = 0;
  smacFlags.flags.flag4 = 0;
  smacFlags.flags.flag5 = 0;
  smacFlags.flags.flag6 = 0;
  smacFlags.flags.flag7 = 0;
  /* it means:
              mPromiscuousModeEnabled = FALSE;
              mSmacInitialized        = FALSE;
              mSmacTimeoutAsked       = FALSE;
              mSmacPerformingCCAScan  = FALSE;
              mSmacPerformingEDScan   = FALSE;
              mSmacPerformingED       = FALSE;
  */
        
#if(TRUE == smacInitializationValidation_d)
    mSmacInitialized = TRUE;  
#endif
    return gErrorNoError_c; 
  
  
}

/************************************************************************************
* MLMEDozeRequest
* 
* Doze request allow the user to put the radio either in Normal Doze Mode (without 
* CLKO but with automatic wake up) or Acoma Doze Mode (with CLKout, but without 
* timeout). 
*
************************************************************************************/
smacErrors_t MLMEDozeRequest
(
  uint32_t u32Timeout /* IN: Is the automatic wake up time for this mode.*/
)
{	
  if(mSmacStateIdle_c != smacState)
  {
      return gErrorBusy_c;   /* The radio is busy in other activity as transmitting  
                                  or receiving data. Or performing a channel scan*/
  }
              
  PhyPlmeDozeRequest(u32Timeout);
  
  smacState = mSmacStateDoze_c;
  return gErrorNoError_c; /*The radio has been set in Doze Mode*/;
  
}

/************************************************************************************
* MLMEEnergyDetect
* 
* This call starts an energy detect(ED) cycle, it returns the energy value (-power)
* via the returned argument. For example, if the Energy Detect returns 80 then the 
* interpreted value is -80 dBm.
*
************************************************************************************/
smacErrors_t MLMEEnergyDetect
(
uint8_t * u8EnergyValue /* OUT: Pointer to the variable where the Energy value 
                                will be store. */
)
{
	
#if(TRUE == smacInitializationValidation_d)
  if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
#endif

  if(mSmacStateIdle_c != smacState)
  {
    return gErrorBusy_c;
  }
  mSmacPerformingED = TRUE;
  smacEdValue = 0;
  
  if(PhyPlmeEdRequest())
  {
    return gErrorNoValidCondition_c;
  }
  
  while(TRUE == mSmacPerformingED)
  {
    /*Add a method to avoid blocking the system*/
  }
  *(u8EnergyValue) = smacEdValue;
  
  return gErrorNoError_c;
}

/************************************************************************************
* MLMEFEGainAdjust
* 
* This is a compensator for the energy detection and AGC. In case users need to 
* calibrate the readings due to a specific application like an external low noise 
* amplifier, this is where users can set the offset.
*
************************************************************************************/
smacErrors_t MLMEFEGainAdjust
(
  uint8_t u8GainValue /* IN: 8 bit value for the gain adjust.*/
)
{
  if(mSmacStateIdle_c != smacState)
  {
    return gErrorBusy_c;
  }
  PhyAdjustCcaOffsetCmp(u8GainValue);
  
  return gErrorNoError_c;
}

/************************************************************************************
* MLMESetChannelRequest
* 
* This function sets the frequency on which the radio will transmit or receive on.
*
************************************************************************************/
smacErrors_t MLMESetChannelRequest
(
channels_t newChannel /* IN: Enumeration value that represents the requested channel. */
)
{
 uint8_t u8PhyRes;

#if(TRUE == smacParametersValidation_d)
  if((gChannel11_c > newChannel) || (gChannel26_c < newChannel))
  {
    return gErrorOutOfRange_c;
  }
#endif

#if(TRUE == smacInitializationValidation_d)
  if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
#endif

  if((mSmacStateIdle_c != smacState) && (mSmacStatePerformingTest_c != smacState))
  {
    return gErrorBusy_c;
  }
  
  u8PhyRes = PhyPlmeSetCurrentChannelRequest((uint8_t)newChannel);
  MC1324xDrv_IRQ_Clear();
  NVIC_ClearPendingIRQ(MC1324x_Irq_Number);
  
  if(!u8PhyRes)
  {
    smacCurrentChannel = newChannel;
    return gErrorNoError_c;
  }
  else 
  {
    return gErrorNoResourcesAvailable_c;
  }
}

/************************************************************************************
* MLMEGetChannelRequest
* 
* This function returns the current channel, if an error is detected it returns 
* gChannelOutOfRange_c.
*
************************************************************************************/
channels_t MLMEGetChannelRequest(void)
{
#if(TRUE == smacInitializationValidation_d)
  if(FALSE == mSmacInitialized)
  {
    return gChannelOutOfRange_c;
  }
#endif
  if((gChannel11_c > smacCurrentChannel) || (gChannel26_c < smacCurrentChannel))
  {
    return gChannelOutOfRange_c;
  }
  else
  {
    return smacCurrentChannel;
  }
}

/************************************************************************************
* MLMEHibernateRequest
* 
* This call places the radio into Hibernate mode. 
*
************************************************************************************/
smacErrors_t MLMEHibernateRequest(void)
{
 #if(TRUE == smacInitializationValidation_d)
  if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
 #endif

  if(mSmacStateIdle_c != smacState)
  {
    return gErrorBusy_c;
  }

  PhyHibernateRequest();
  
  smacState = mSmacStateHibernate_c;
  
  return gErrorNoError_c;
}


/************************************************************************************
* MLMEPAOutputAdjust
* 
* This function adjusts the output power of the transmitter. 
*
************************************************************************************/
smacErrors_t MLMEPAOutputAdjust
(
uint8_t u8PaValue /*IN: Value for the output power desired. Values 3-31 are required.*/
)
{    
  uint8_t u8PhyRes;

#if(TRUE == smacParametersValidation_d)
  if((gMaxOutputPower_c < u8PaValue) || (gMinOutputPower_c > u8PaValue))
  {
    return gErrorOutOfRange_c;
  }
#endif

#if(TRUE == smacInitializationValidation_d)
  if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
#endif

  if((mSmacStateIdle_c != smacState) && (mSmacStatePerformingTest_c != smacState))
  {
    return gErrorBusy_c;
  }
  
  u8PhyRes = PhyPlmeSetPwrLevelRequest(u8PaValue);
  
  if(!u8PhyRes)
  {
    return gErrorNoError_c;
  }
  else 
  {
    return gErrorNoResourcesAvailable_c;
  }
}

/************************************************************************************
* MLMEPHYXtalAdjust
* 
* This function is used to adjust the radio reference clock by a trim value.
*
************************************************************************************/
smacErrors_t MLMEXtalAdjust
(
uint8_t u8ReqValue /* IN: value representing the trim value to the oscillator.*/
)
{
  if((mSmacStateIdle_c != smacState) && (mSmacStatePerformingTest_c != smacState))
  {
    return gErrorBusy_c;
  }
  
  PhyXtalAdjust(u8ReqValue);
  
  return gErrorNoError_c;
}

/************************************************************************************
* MLMESoftReset
* 
* This function performs a software reset on the radio,  PHY and SMAC state machines.
*
************************************************************************************/
smacErrors_t MLMESoftReset(void)
{
  PhyAbort();
  return MLMERadioInit();
}

/************************************************************************************
* MLMESetClockRate
* 
* This function is called to set the desired clock out from radio.
*
************************************************************************************/
smacErrors_t MLMESetClockRate
(
clkoFrequency_t clkoFreq  /* IN: Enumeration value that represents radio clock out 
                                frequency (CLKO). */ 
)
{
  return (smacErrors_t)PhyPlmeSetClockOutRate(clkoFreq);
}



/************************************************************************************
* MLMESetTmrPrescale
* 
* This function changes the rate aacollinst which the radio timers operate.
*
************************************************************************************/
smacErrors_t MLMESetTmrPrescale
(
timerTimeBase_t TimeBase /*IN: enumeration value that represents that represents  
                               timer prescale or time base.*/
)
{
#if(TRUE == smacParametersValidation_d)
  if((gTimeBase500kHz_c > TimeBase) || (gTimeBase15p625kHz_c < TimeBase))
  {
    return gErrorOutOfRange_c;
  }
#endif

#if(TRUE == smacInitializationValidation_d)
  if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
#endif

  (void)PhySetRadioTimerPrescaler((uint8_t)TimeBase);
  return gErrorNoError_c;
}

/************************************************************************************
* MLMEWakeRequest
* 
* This primitive brings the radio out of low power mode.
*
************************************************************************************/
smacErrors_t MLMEWakeRequest(void)
{

   #if(TRUE == smacInitializationValidation_d)
  if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
 #endif

  if (smacState == mSmacStateDoze_c || smacState == mSmacStateHibernate_c)
  {
      PhyWakeRequest();
      return gErrorNoError_c;
  }

  return gErrorNoValidCondition_c;
  
}

/************************************************************************************
* XCVRContReset
* 
* This function asserts the reset line of the transceiver, shutting it down to its 
* lowest power mode.
*
************************************************************************************/
void XCVRContReset(void)
{
  MC1324xDrv_RST_B_Assert();
}

/************************************************************************************
* XCVRRestart
* 
* This function deasserts the reset line, thus it power the transceiver up.
*
************************************************************************************/
void XCVRRestart(void)
{
  MC1324xDrv_RST_B_Deassert();
}


/***********************************************************************************/
/***************************** SMAC management primitives **************************/
/***********************************************************************************/

/************************************************************************************
* MLMEGetRficVersion
* 
* This function is used to read the version number of different hardware and software 
* modules inside the SMAC platform.
*
************************************************************************************/
smacErrors_t MLMEGetRficVersion
(
versionedEntity_t Entity, /* IN: The module for which the version is required. */
uint8_t *Buffer           /* OUT: A pointer to the buffer where the version will be 
                                  written. */
)
{
  uint8_t u8i;

#if(TRUE == smacParametersValidation_d)
  if(gMaxVersionedEntity_c <= Entity){
    return gErrorOutOfRange_c;
  }
#endif

#if(TRUE == smacInitializationValidation_d)
  if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
#endif

  if(mSmacStateIdle_c != smacState)
  {
    return gErrorBusy_c;
  }

  if(gSwSmacVersion_c == Entity)
  {
    for(u8i=0; u8i < gSwSmacVersionLength_c; u8i++)
    {
      *Buffer++ = au8SmacVersion[u8i];
    }
  }
  else
  {
   *Buffer = PhyGetRfIcVersion();    
  }

  return gErrorNoError_c;
}

/************************************************************************************
* MLMESetPromiscuousMode
* 
* This promiscous mode is different than PHY layer promiscous. 
* SMAC appends 0xFF7E as the first two bytes of the packet to identify its own 
* packets from other packets in the same PHY layer. When promiscuous mode is off 
* SMAC filters the received packets and just allows those beginning with 0xFF7E; 
* when promiscuous mode is on SMAC let pass all the messages beginning or not with
* 0xFF7E. MLMESetPromiscuousMode allows setting or clearing the promiscuous mode. 
*
************************************************************************************/
void MLMESetPromiscuousMode
(
bool_t isPromiscousMode /* IN: This is a boolean value that indicates if the promiscuous 
                           mode is on (TRUE) or off (FALSE). */
)
{
   mPromiscuousModeEnabled = isPromiscousMode; 
}

/************************************************************************************
* MLMEGetPromiscuousMode
* 
* This function returns the current state of promiscuous mode.
*
************************************************************************************/
bool_t MLMEGetPromiscuousMode(void)
{
  return mPromiscuousModeEnabled;
}

/************************************************************************************
* MLMEScanRequest
* 
* This function scans the different channels using one out of two techniques and 
* returns the amount of energy in all of the channels.
*
************************************************************************************/
smacErrors_t MLMEScanRequest
(
uint16_t u16ChannelsToScan, /*IN: Bitmap of the Channels to be scanned.*/
scanModes_t ScanMode,       /*IN: Technique to be used CCA or ED.*/
uint8_t *pu8ChannelScan     /*OUT: The buffer where the values of the scan will be 
                                  returned. */
)
{

  smacErrors_t scanRes;

#if(TRUE == smacParametersValidation_d)
  if((0 == u16ChannelsToScan) || (gMaxScanMode_c <= ScanMode) || (NULL == pu8ChannelScan))
  {
    return gErrorOutOfRange_c;
  }
#endif

#if(TRUE == smacInitializationValidation_d)
  if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
#endif

  if(mSmacStateIdle_c != smacState)
  {
    return gErrorBusy_c;
  }
  
  smacChannelsBeingScanned = u16ChannelsToScan;
  smacScanCurrentChannel   = gChannel11_c;
  smacClearestChann        = gChannel11_c;
  smacProccesPacketPtr.smacScanResultsPointer = pu8ChannelScan;
  smacEdValue              = 0xFF;
  
  if(gScanModeED_c == ScanMode)
  {
    mSmacPerformingEDScan = TRUE;
  }
  else
  {
    mSmacPerformingCCAScan = TRUE;
  }

  smacState = mSmacStateScanningChannels_c; 
  scanRes = SmacScanTheNextChannel();
  
  if(scanRes)
  {
    smacState = mSmacStateIdle_c; 
    mSmacPerformingEDScan  = FALSE;
    mSmacPerformingCCAScan = FALSE;
  }
  
  return scanRes;
}

/************************************************************************************
* MLMETestMode
* 
* By employing this function, users can execute a test of the radio. Test mode 
* implements the following:
*   -PRBS9 Mode, 
*   -Force_idle, 
*   -Continuos TX without modulation, 
*   -Continuos TX with modulation.
*
************************************************************************************/
smacErrors_t MLMETestMode
(
smacTestMode_t  mode  /*IN: The test mode to start.*/
)
{
  
#if(TRUE == smacParametersValidation_d)
  if(gMaxTestMode_c <= mode)
  {
    return gErrorOutOfRange_c;
  }
#endif

#if(TRUE == smacInitializationValidation_d)
  if(FALSE == mSmacInitialized)
  {
    return gErrorNoValidCondition_c;
  }
#endif
  
  if((mSmacStateIdle_c != smacState) && (mSmacStatePerformingTest_c != smacState))
  {
    return gErrorBusy_c;
  }

  pTestPacket = (phyPacket_t*)&u8TestBuffer;
  
  smacState = mSmacStatePerformingTest_c;
  
  /* Disable Continuous Tx Mode */
  MC1324xDrv_IndirectAccessSPIWrite(TESTMODE_CTRL, 0x00);
  MC1324xDrv_IndirectAccessSPIWrite(TX_MODE_CTRL, 0x00);   
  
  /* Abort Radio Sequences */
  PhyAbort(); 
  
   
  if(gTestModeForceIdle_c == mode)
  {
    mSmacPerformingPRBS9 = FALSE;
    smacState = mSmacStateIdle_c;
  }
   
  else if(gTestModeContinuousTxModulated_c == mode)
  {
    MC1324xDrv_IndirectAccessSPIWrite(TESTMODE_CTRL, 0x02);     //write_indirect(0xA3, 0x02);  TESTMODE_CTRL, CONTINUOUS_EN=1
    MC1324xDrv_IndirectAccessSPIWrite(TX_MODE_CTRL, 0x07);      //write_indirect(0x4E, 0x01);  TX_MODE_CTRL, DTS_MODE=ONES
    MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL1, 0x02);           //write_direct(0x03,0x02);     PHY_CTRL1, SEQUENCE T   
  } 
  
  else if(gTestModeContinuousTxUnmodulated_c == mode)
  {
    //MC1324xDrv_DirectAccessSPIMultiByteRead(PLL_FRAC0_LSB,au8TempPllFrac,2);
    //au8TempPllFrac[1] -= 0x04;
    //MC1324xDrv_DirectAccessSPIMultiByteWrite(PLL_FRAC0_LSB, au8TempPllFrac, 2);
    MC1324xDrv_DirectAccessSPIMultiByteWrite(PLL_FRAC0_LSB, (uint8_t *) &Local_pll_frac[smacCurrentChannel - 11], 2);
    
    MC1324xDrv_IndirectAccessSPIWrite(TESTMODE_CTRL, 0x02);     //write_indirect(0xA3, 0x02);  TESTMODE_CTRL, CONTINUOUS_EN=1
    MC1324xDrv_IndirectAccessSPIWrite(TX_MODE_CTRL, 0x01); 
    MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL1, 0x02);           //write_direct(0x03,0x02);     PHY_CTRL1, SEQUENCE T
  } 

  else if(gTestModePRBS9_c == mode)
  {
    mSmacPerformingPRBS9 = TRUE;
    (void)PhyPdDataRequest(pTestPacket, gDataReq_NoAck_NoCca_Unslotted_c, NULL);
  }
  return gErrorNoError_c;
}




/***********************************************************************************/
/******************************** SMAC MCU primitives ******************************/
/***********************************************************************************/


/************************************************************************************
* SMACEnableInterrupts
* 
* This function enables the interrupts that are used by SMAC.
*
************************************************************************************/

void SMACEnableInterrupts(void)
{
  NVIC_EnableIRQ(Radio_Irq_Number);
}

/************************************************************************************
* SMACDisableInterrupts
* 
* This function disables the interrupts that are used by SMAC.
*
************************************************************************************/
void SMACDisableInterrupts(void)
{
  NVIC_DisableIRQ(Radio_Irq_Number);
}


/************************************************************************************
*************************************************************************************
* private functions
*************************************************************************************
************************************************************************************/

/***********************************************************************************/
smacErrors_t SmacScanTheNextChannel(void)
{
  uint8_t  u8PhyRes;
  uint16_t u16ChannelMask;

  /*Look for the next channel to scan*/
  u16ChannelMask = 0x0001 << (uint8_t)(smacScanCurrentChannel - gChannel11_c);
  while(!(u16ChannelMask & smacChannelsBeingScanned) && (u16ChannelMask))
  {
    smacScanCurrentChannel++;
    u16ChannelMask<<=1;
  }

  if(gTotalChannels_c == smacScanCurrentChannel)
  {
    return gErrorOutOfRange_c;
  }
  else
  {
    u8PhyRes = PhyPlmeSetCurrentChannelRequest(smacScanCurrentChannel);
    if(!u8PhyRes)
    {
      if(mSmacPerformingEDScan)
      {
        return (smacErrors_t)PhyPlmeEdRequest();
      }
      else
      {
        return (smacErrors_t)PhyPlmeCcaRequest(gCcaCCA_MODE1_c);
      }
    }
    else 
    {
      return (smacErrors_t)u8PhyRes;
    }
  }
}
 
/*********************************************************/
void SmacSetRxTimeout(zbClock24_t timeoutSymbols)
{
  zbClock24_t time;
  PhyTimeReadClock(&time);
  time += timeoutSymbols + 4;
  PhyTimeSetEventTimeout(&time);
}

/*********************************************************/
phyPacket_t * SmacFormatTxPacket(txPacket_t *psTxPacket)
{
  phyPacket_t * packetToPhy;
  
  packetToPhy = (phyPacket_t *)psTxPacket;

  if(FALSE == mPromiscuousModeEnabled)
  {
    packetToPhy = (phyPacket_t *)psTxPacket;
    packetToPhy->frameLength += 2; //Add 2 for Code bytes
    packetToPhy->data[0] = 0x7E;
    packetToPhy->data[1] = 0xFF;
  }
  else
  {
    packetToPhy = (phyPacket_t *)&psTxPacket->smacPdu.reserved[1];
    packetToPhy->frameLength = psTxPacket->u8DataLength;
  }
  
  packetToPhy->frameLength += 2; //Add 2 for FCS
  
  return packetToPhy;
}
 
  
   
/***********************************************************************************/
/****************************** PHY Callback Functions *****************************/
/***********************************************************************************/
/***********************************************************************************/
void PhyPdDataConfirm(void)
{
  if(mSmacStateTransmitting_c == smacState)
  {
    if(!mPromiscuousModeEnabled){
      smacProccesPacketPtr.smacTxPacketPointer->frameLength-=2;
    }
    // JBW/CS - 20JUN14: Changed the order of these two calls so that MCPSDataConfirm can switch back on the receiver.
    smacState= mSmacStateIdle_c;
    MCPSDataConfirm(txSuccessStatus_c);
  }
  else if(mSmacStatePerformingTest_c == smacState)
  {
    if(TRUE == mSmacPerformingPRBS9)
    {
      delayMs(9);
      (void)PhyPdDataRequest(pTestPacket, gDataReq_NoAck_NoCca_Unslotted_c, NULL);
    }
  }
}


/***********************************************************************************/
void PhyPdDataIndication(void)
{
  uint8_t  u8PhyRes;

  if(!mPromiscuousModeEnabled){
    smacProccesPacketPtr.smacRxPacketPointer->u8DataLength-=2;
	}
  else
  {
    smacProccesPacketPtr.smacRxPacketPointer->u8DataLength = ((smacProccesPacketPtr.smacRxPacketPointer->smacPdu.reserved[1]));
  }
  
  if( ((!mPromiscuousModeEnabled) && (FALSE == areCodeBytesValid(smacProccesPacketPtr.smacRxPacketPointer)) )||
    (smacProccesPacketPtr.smacRxPacketPointer->u8DataLength > smacProccesPacketPtr.smacRxPacketPointer->u8MaxDataLength))
  {
    u8PhyRes = PhyPlmeRxRequest((phyPacket_t *)&(smacProccesPacketPtr.smacRxPacketPointer->u8DataLength), &smacLastDataRxParams);
    if(!u8PhyRes){
      if(mSmacTimeoutAsked)
      {
        SmacSetRxTimeout((zbClock24_t)smacTimeout);
      }
    }else{
      smacProccesPacketPtr.smacRxPacketPointer->rxStatus = rxAbortedStatus_c;
      PhyTimeDisableEventTimeout();
      MCPSDataIndication(smacProccesPacketPtr.smacRxPacketPointer);
      smacState = mSmacStateIdle_c;
    }
  }
  else
  {
      smacProccesPacketPtr.smacRxPacketPointer->rxStatus = rxSuccessStatus_c;
      PhyTimeDisableEventTimeout();
      MCPSDataIndication(smacProccesPacketPtr.smacRxPacketPointer);
      smacState= mSmacStateIdle_c;
  }
}

/***********************************************************************************/
void PhyPlmeCcaConfirm(bool_t channelInUse)
{
  smacErrors_t scanRes;
  smacProccesPacketPtr.smacScanResultsPointer[smacScanCurrentChannel - gChannel11_c] = channelInUse;
  smacScanCurrentChannel++;
  scanRes = SmacScanTheNextChannel();
  
  if(scanRes)
  {
    smacClearestChann = gChannelOutOfRange_c; 
    (void)PhyPlmeSetCurrentChannelRequest(smacCurrentChannel);
    MLMEScanConfirm(smacClearestChann);
    mSmacPerformingCCAScan = FALSE;
    smacState= mSmacStateIdle_c;
  }
}

/***********************************************************************************/
void PhyPlmeEdConfirm(uint8_t energyLevel)
{
  smacErrors_t scanRes;
  
  if(mSmacPerformingED)
  {
    smacEdValue = energyLevel; 
    mSmacPerformingED = FALSE;
  }
  else if(mSmacPerformingEDScan)
  {
    if(energyLevel < smacEdValue)
    {
      smacEdValue = energyLevel;
      smacClearestChann = smacScanCurrentChannel;
    }
    smacProccesPacketPtr.smacScanResultsPointer[smacScanCurrentChannel - gChannel11_c] = energyLevel;//energyLevel/2; 
    smacScanCurrentChannel++;
    scanRes = SmacScanTheNextChannel();
    
    if(scanRes)
    {
      MLMEScanConfirm(smacClearestChann);
      (void)PhyPlmeSetCurrentChannelRequest(smacCurrentChannel);
      mSmacPerformingEDScan  = FALSE;
      smacState= mSmacStateIdle_c;
    }
  }
}

/***********************************************************************************/
void PhyPlmeSyncLossIndication(void)
{
  uint8_t  u8PhyRes;
  if(mSmacStateReceiving_c == smacState)
  {
    u8PhyRes = PhyPlmeRxRequest((phyPacket_t *)&(smacProccesPacketPtr.smacRxPacketPointer->u8DataLength), &smacLastDataRxParams);
    if(u8PhyRes)
    {
      smacProccesPacketPtr.smacRxPacketPointer->rxStatus = rxAbortedStatus_c;
      PhyTimeDisableEventTimeout();
      MCPSDataIndication(smacProccesPacketPtr.smacRxPacketPointer);
      smacState= mSmacStateIdle_c;
    }
  }
  else if(mSmacStateTransmitting_c == smacState)
  {
    (void)PhyPdDataRequest(smacProccesPacketPtr.smacTxPacketPointer, gDataReq_NoAck_NoCca_Unslotted_c, NULL);
  }
  else if(mSmacStateScanningChannels_c == smacState)
  {
    (void)SmacScanTheNextChannel();
  }
  else if(TRUE == mSmacPerformingED)
  {
    if(PhyPlmeEdRequest())
    {
      smacEdValue = 0xFF;
      mSmacPerformingED = FALSE;
    }
  }
}

/***********************************************************************************/
void PhyPlmeResetIndication(void)
{
  MLMEResetIndication();
}

/***********************************************************************************/
void    PhyPlmeWakeConfirm(void)
{
  MLMEWakeConfirm();
}

/***********************************************************************************/
void PhyTimeRxTimeoutIndication(void)
{
  smacProccesPacketPtr.smacRxPacketPointer->rxStatus = rxTimeOutStatus_c;
  MCPSDataIndication(smacProccesPacketPtr.smacRxPacketPointer);
  smacState= mSmacStateIdle_c;
}

/***********************************************************************************/
void PhyTimeStartEventIndication(void)
{
  
}


