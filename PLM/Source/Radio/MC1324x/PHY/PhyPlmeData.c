/*****************************************************************************
 *                               INCLUDED HEADERS                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the headers that this module needs to include.    *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
#include "EmbeddedTypes.h"
#include "PortConfig.h"
#include "TransceiverDrv.h"
#include "TransceiverReg.h"
#include "Phy.h"
#include "Interrupt.h"
#include "Utilities_Interface.h" 

/*****************************************************************************
 *                               PRIVATE VARIABLES                           *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have local       *
 * (file) scope.                                                             *
 * Each of this declarations shall be preceded by the 'static' keyword.      *
 * These variables / constants cannot be accessed outside this module.       *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

                                     //2405   2410    2415    2420    2425    2430    2435    2440    2445    2450    2455    2460    2465    2470    2475    2480
static const uint8_t  pll_int[16] =  {0x0B,   0x0B,   0x0B,   0x0B,   0x0B,   0x0B,   0x0C,   0x0C,   0x0C,   0x0C,   0x0C,   0x0C,   0x0D,   0x0D,   0x0D,   0x0D};
static const uint16_t pll_frac[16] = {0x2800, 0x5000, 0x7800, 0xA000, 0xC800, 0xF000, 0x1800, 0x4000, 0x6800, 0x9000, 0xB800, 0xE000, 0x0800, 0x3000, 0x5800, 0x8000};

/*****************************************************************************
 *                               PUBLIC VARIABLES                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have global      *
 * (project) scope.                                                          *
 * These variables / constants can be accessed outside this module.          *
 * These variables / constants shall be preceded by the 'extern' keyword in  *
 * the interface header.                                                     *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

static uint8_t gPhyCurrentChannelPAN0 = 0x0B;
static uint8_t gPhyCurrentChannelPAN1 = 0x0B;

/*****************************************************************************
 *                           PRIVATE FUNCTIONS PROTOTYPES                    *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions prototypes that have local (file)   *
 * scope.                                                                    *
 * These functions cannot be accessed outside this module.                   *
 * These declarations shall be preceded by the 'static' keyword.             *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

/*****************************************************************************
 *                                PRIVATE FUNCTIONS                          *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions that have local (file) scope.       *
 * These functions cannot be accessed outside this module.                   *
 * These definitions shall be preceded by the 'static' keyword.              *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

/*****************************************************************************
 *                             PUBLIC FUNCTIONS                              *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions that have global (project) scope.   *
 * These functions can be accessed outside this module.                      *
 * These functions shall have their declarations (prototypes) within the     *
 * interface header file and shall be preceded by the 'extern' keyword.      *
 *---------------------------------------------------------------------------*
 *****************************************************************************/


/*---------------------------------------------------------------------------
 * Name: PhyPdDataRequest
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPdDataRequest
(
  phyPacket_t *pTxPacket,
  uint8_t phyTxMode,
  phyRxParams_t *pRxParams
)
{
  volatile uint8_t phyCtrl1Reg, phyCtrl2Reg, phyCtrl4Reg, irqSts1Reg, irqSts2Reg, irqSts3Reg;
  //volatile uint8_t pllInt, pllFracLsb, pllFracMsb;
  
    
  
#ifdef PHY_PARAMETERS_VALIDATION
  // null pointer
  if(NULL == pTxPacket)
  {
    return gPhyInvalidParam_c;
  }

  
  // cannot have packets shorter than FCS field (2 byte)
  if(pTxPacket->frameLength < 2)
  {
    return gPhyInvalidParam_c;
  }

  // if CCA required ...
  if(gCcaEnMask_c & phyTxMode)
  { // ... cannot perform other types than MODE1 and MODE2
    if( (gCcaCCA_MODE1_c != (gCcaTypeMask_c & phyTxMode)) && (gCcaCCA_MODE2_c != (gCcaTypeMask_c & phyTxMode)) )
    {
      return gPhyInvalidParam_c;
    }
    // ... cannot perform Continuous CCA on T or TR sequences
    if(gContinuousEnMask_c & phyTxMode)
    {
      return gPhyInvalidParam_c;
    }
  }

#endif // PHY_PARAMETERS_VALIDATION

  /**/
  if( gIdle_c != PhyGetSeqState() )
  {
      return gPhyBusy_c;
  }
  
  
  MC1324xDrv_PB_SPIBurstWrite( (uint8_t *) pTxPacket, (uint8_t) (pTxPacket->frameLength - 1)); 
  phyCtrl1Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL1); 
  phyCtrl2Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL2);  
  phyCtrl4Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL4);
  
  (void)phyCtrl1Reg;
  (void)phyCtrl2Reg;
  
   PhyPassRxParams(NULL);
   PhyPassRxDataPtr(NULL);
    
   phyCtrl1Reg &= (uint8_t) ~(cPHY_CTRL1_XCVSEQ);
   phyCtrl1Reg |=  gTX_c;

  
  phyCtrl2Reg &= (uint8_t) ~(cPHY_CTRL2_SEQMSK); // unmask SEQ interrupt
  
  irqSts1Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS1);
  irqSts2Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS2);
  irqSts3Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS3);
  
  irqSts3Reg |= (uint8_t) (cIRQSTS3_TMR3MSK);   // mask TMR3 interrupt
  
  // ensure that no spurious interrupts are raised
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS1, irqSts1Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS2, irqSts2Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS3, irqSts3Reg);
  
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL2, phyCtrl2Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL4, phyCtrl4Reg);
  
  // start the TX or TRX sequence
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL1, phyCtrl1Reg);  
  
  return gPhySuccess_c;
}

/*---------------------------------------------------------------------------
 * Name: PhyPlmeRxRequest
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeRxRequest
(
  phyPacket_t *pRxData,
  phyRxParams_t *pRxParams
)
{
  volatile uint8_t phyCtrl1Reg, phyCtrl2Reg, irqSts1Reg, irqSts2Reg, irqSts3Reg; //, seqState, phyCtrl4Reg;
  //volatile uint8_t pllInt, pllFracLsb, pllFracMsb, phyStsIar;

  if( gIdle_c != PhyGetSeqState() )
  {
    return gPhyBusy_c;
  }

  PhyPassRxParams(pRxParams);
  PhyPassRxDataPtr(pRxData);
  
  phyCtrl1Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL1);
  phyCtrl2Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL2);
 // phyCtrl4Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL4);
  
  /* program the RX sequence */
  phyCtrl1Reg &= (uint8_t) ~(cPHY_CTRL1_XCVSEQ);
  phyCtrl1Reg |=  gRX_c;
  
  phyCtrl2Reg &= (uint8_t) ~(cPHY_CTRL2_SEQMSK); // unmask SEQ interrupt
 //phyCtrl2Reg &= (uint8_t) ~(cPHY_CTRL2_RXMSK); // unmask RX interrupt
 // phyCtrl4Reg |= (uint8_t) (cPHY_CTRL4_PROMISCUOUS);
  

  irqSts1Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS1);
  irqSts2Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS2);
  irqSts3Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS3);
  
  
  irqSts3Reg |= (uint8_t) (cIRQSTS3_TMR3MSK);   // mask TMR3 interrupt
  
/*
  pllInt =     MC1324xDrv_DirectAccessSPIRead(PLL_INT0);
  pllFracLsb = MC1324xDrv_DirectAccessSPIRead(PLL_FRAC0_LSB);
  pllFracMsb = MC1324xDrv_DirectAccessSPIRead(PLL_FRAC0_MSB);
  */
  // ensure that no spurious interrupts are raised
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS1, irqSts1Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS2, irqSts2Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS3, irqSts3Reg);
  
 // MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL4, phyCtrl4Reg);
  
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL2, phyCtrl2Reg);
  // start the RX sequence
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL1, phyCtrl1Reg);

    return gPhySuccess_c;
}

/*---------------------------------------------------------------------------
 * Name: PhyPlmeCcaEdRequest
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeCcaEdRequest
(
  uint8_t ccaParam
)
{
  uint8_t phyCtrl1Reg, phyCtrl2Reg, phyCtrl4Reg, irqSts1Reg, irqSts2Reg, irqSts3Reg;
  
  //PHY_DEBUG_LOG(PDBG_PLME_CCA_ED_REQUEST); TODO

#ifdef PHY_PARAMETERS_VALIDATION
  // illegal CCA type
  if(gInvalidCcaType_c == (gCcaTypeMask_c & ccaParam))
  {
    return gPhyInvalidParam_c;
  }

  // cannot perform Continuous CCA using ED type
  if( (gContinuousEnMask_c & ccaParam) && (gCcaED_c == (gCcaTypeMask_c & ccaParam)) )
  {
    return gPhyInvalidParam_c;
  }

  // cannot perform ED request using other type than gCcaED_c
  if( (gCcaEnMask_c & ccaParam) && (gCcaED_c != (gCcaTypeMask_c & ccaParam)) )
  {
    return gPhyInvalidParam_c;
  }

  // cannot perform ED request using Continuous mode
  if( (gContinuousEnMask_c & ccaParam) && ((gCcaEnMask_c & ccaParam)==0) )
  {
    return gPhyInvalidParam_c;
  }

#endif // PHY_PARAMETERS_VALIDATION

  if( gIdle_c != PhyGetSeqState() )
  {
    return gPhyBusy_c;
  }

  phyCtrl1Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL1);
  phyCtrl2Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL2);
  phyCtrl4Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL4);
  
  // write in PHY CTRL4 the desired type of CCA
  phyCtrl4Reg &= (uint8_t) ~(cPHY_CTRL4_CCATYPE << cPHY_CTRL4_CCATYPE_Shift_c);
  phyCtrl4Reg |= (uint8_t) ((gCcaTypeMask_c & ccaParam) << (cPHY_CTRL4_CCATYPE_Shift_c - gCcaTypePos_c));
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL4, phyCtrl4Reg);
  
  // slotted operation
  if(gSlottedEnMask_c & ccaParam)
  {
    phyCtrl1Reg |= (uint8_t) (cPHY_CTRL1_SLOTTED);
  }
  else
  {
    phyCtrl1Reg &= (uint8_t) ~(cPHY_CTRL1_SLOTTED);
  }
  
  // continuous CCA
  if(gContinuousEnMask_c & ccaParam)
  {
    // start the continuous CCA sequence
    // immediately or by TC2', depending on a previous PhyTimeSetEventTrigger() call)
     phyCtrl1Reg &= (uint8_t) ~(cPHY_CTRL1_XCVSEQ);
     phyCtrl1Reg |= gCCCA_c;
    // at the end of the scheduled sequence, an interrupt will occur:
    // CCA , SEQ or TMR3
  }
  // normal CCA (not continuous)
  else
  {
    // start the CCA or ED sequence (this depends on CcaType used)
    // immediately or by TC2', depending on a previous PhyTimeSetEventTrigger() call)
     phyCtrl1Reg &= (uint8_t) ~(cPHY_CTRL1_XCVSEQ);
     phyCtrl1Reg |= gCCA_c;
    // at the end of the scheduled sequence, an interrupt will occur:
    // CCA , SEQ or TMR3
  }
  
  phyCtrl2Reg &= (uint8_t) ~(cPHY_CTRL2_SEQMSK); // unmask SEQ interrupt
  
  irqSts1Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS1);
  irqSts2Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS2);
  irqSts3Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS3);
  
  irqSts3Reg |= (uint8_t) (cIRQSTS3_TMR3IRQ);   // mask TMR3 interrupt
  
  // ensure that no spurious interrupts are raised
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS1, irqSts1Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS2, irqSts2Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS3, irqSts3Reg);
  
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL2, phyCtrl2Reg);
  // start the CCA/ED or CCCA sequence
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL1, phyCtrl1Reg);
  
  return gPhySuccess_c;
}

/*---------------------------------------------------------------------------
 * Name: PhyAdjustCcaOffsetCmp
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyAdjustCcaOffsetCmpReg
(
  uint8_t ccaOffset
)
{
  #ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == ccaOffset)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION 
  MC1324xDrv_DirectAccessSPIWrite(CCA1_ED_OFFSET_COMP, ccaOffset);
  return 0;
}

/*---------------------------------------------------------------------------
 * Name: PhyPlmeSetClockOutRateReg
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeSetClockOutRateReg
(
  uint8_t clockOutRate
)
{
  MC1324xDrv_DirectAccessSPIWrite(CLK_OUT_CTRL, clockOutRate);
  return 0;
}

/*---------------------------------------------------------------------------
 * Name: PhySetRadioTimerPrescalerReg
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhySetRadioTimerPrescalerReg
(
  uint8_t timeBase
)
{
  MC1324xDrv_IndirectAccessSPIWrite(TMR_PRESCALE,timeBase);
  return 0;
  
}

/*---------------------------------------------------------------------------
 * Name: PhyPlmeSetCurrentChannelRequestPAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeSetCurrentChannelRequestPAN0
(
  uint8_t channel
)
{
  //PHY_DEBUG_LOG(PDBG_PLME_SET_CHAN_REQUEST); TODO

#ifdef PHY_PARAMETERS_VALIDATION
  if((channel < 11) || (channel > 26)) 
  {
    return gPhyInvalidParam_c;
  }
#endif // PHY_PARAMETERS_VALIDATION

  /**/
  if( gIdle_c != PhyGetSeqState() )
  {
    return gPhyBusy_c;
  }
  
  gPhyCurrentChannelPAN0 = channel;
  MC1324xDrv_DirectAccessSPIWrite(PLL_INT0, pll_int[channel - 11]);
  MC1324xDrv_DirectAccessSPIMultiByteWrite(PLL_FRAC0_LSB, (uint8_t *) &pll_frac[channel - 11], 2);
  return gPhySuccess_c;
}

/*---------------------------------------------------------------------------
 * Name: PhyPlmeSetCurrentChannelRequestPAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeSetCurrentChannelRequestPAN1
(
  uint8_t channel
)
{
  //PHY_DEBUG_LOG(PDBG_PLME_SET_CHAN_REQUEST); TODO

#ifdef PHY_PARAMETERS_VALIDATION
  if((channel < 11) || (channel > 26)) 
  {
    return gPhyInvalidParam_c;
  }
#endif // PHY_PARAMETERS_VALIDATION

  if( gIdle_c != PhyGetSeqState() )
  {
    return gPhyBusy_c;
  }
  gPhyCurrentChannelPAN1 = channel;
  MC1324xDrv_DirectAccessSPIWrite(PLL_INT1, pll_int[channel - 11]);
  MC1324xDrv_DirectAccessSPIMultiByteWrite(PLL_FRAC1_LSB, (uint8_t *) &pll_frac[channel - 11], 2);
  return gPhySuccess_c;
}

/*---------------------------------------------------------------------------
 * Name: PhyPlmeGetCurrentChannelRequestPAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeGetCurrentChannelRequestPAN0
( 
  void
)
{
  return gPhyCurrentChannelPAN0;
}

/*---------------------------------------------------------------------------
 * Name: PhyPlmeGetCurrentChannelRequestPAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeGetCurrentChannelRequestPAN1
(
  void
)
{
  return gPhyCurrentChannelPAN1;
}

/*---------------------------------------------------------------------------
 * Name: PhyPlmeSetPwrLevelRequest
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeSetPwrLevelRequest
(
  uint8_t pwrStep
)
{
  //PHY_DEBUG_LOG(PDBG_PLME_SET_POWER_REQUEST); TODO
  
#ifdef PHY_PARAMETERS_VALIDATION
  if((pwrStep < 3) || (pwrStep > 31)) //-40 dBm to 16 dBm
  {
    return gPhyInvalidParam_c;
  }
#endif // PHY_PARAMETERS_VALIDATION
  
  MC1324xDrv_DirectAccessSPIWrite(PA_PWR, (uint8_t)(pwrStep & 0x1F));
  return gPhySuccess_c;
}