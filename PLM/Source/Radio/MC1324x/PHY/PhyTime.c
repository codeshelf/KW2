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

/*****************************************************************************
 *                               PRIVATE VARIABLES                           *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have local       *
 * (file) scope.                                                             *
 * Each of this declarations shall be preceded by the 'static' keyword.      *
 * These variables / constants cannot be accessed outside this module.       *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

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
 * Name: PhyTimeSetEventTrigger
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyTimeSetEventTrigger
(
  uint16_t startTime
)
{
  uint8_t phyCtrl1Reg, phyCtrl3Reg, irqSts3Reg;
  uint16_t startTimeTmp = startTime;
  
  phyCtrl1Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL1);
  phyCtrl3Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL3);
  
  
  phyCtrl3Reg &= ~(cPHY_CTRL3_TMR2CMP_EN);// disable TMR2 compare
  phyCtrl1Reg |= cPHY_CTRL1_TMRTRIGEN;    // enable autosequence start by TC2 match
  
  
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL3, phyCtrl3Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL1, phyCtrl1Reg);
  
  
  MC1324xDrv_DirectAccessSPIMultiByteWrite( (uint8_t) T2PRIMECMP_LSB, (uint8_t *) &startTimeTmp, 2);
  
  irqSts3Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS3);
  
  // TC2PRIME_EN must be enabled in PHY_CTRL4 register
  phyCtrl3Reg |= cPHY_CTRL3_TMR2CMP_EN;   // enable TMR2 compare
  irqSts3Reg &= ~(cIRQSTS3_TMR2MSK);      // unmask TMR2 interrupt
  irqSts3Reg &= 0xF0;                            // do not change IRQ others status
  irqSts3Reg |= (cIRQSTS3_TMR2IRQ);       // aknowledge TMR2 IRQ 
  
  
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL3, phyCtrl3Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS3, irqSts3Reg);
  
  //PHY_DEBUG_LOG2(PDBG_SET_EVENT_TRIGGER, PP_PHY_TC2PRIMEH, PP_PHY_TC2PRIMEL); //TODO
}

/*---------------------------------------------------------------------------
 * Name: PhyTimeSetEventTimeout
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyTimeSetEventTimeout
(
  zbClock24_t *pEndTime
)
{
  uint8_t phyCtrl4Reg, phyCtrl3Reg, irqSts3Reg;
  
#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pEndTime)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION
  
  phyCtrl4Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL4);
  phyCtrl3Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL3);
  
  
  phyCtrl3Reg &= ~(cPHY_CTRL3_TMR3CMP_EN);// disable TMR3 compare
  phyCtrl4Reg |= cPHY_CTRL4_TC3TMOUT;     // enable autosequence stop by TC3 match
  
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL3, phyCtrl3Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL4, phyCtrl4Reg);
  
  MC1324xDrv_DirectAccessSPIMultiByteWrite( (uint8_t) T3CMP_LSB, (uint8_t *) pEndTime, 3);
  
  irqSts3Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS3);
  
  phyCtrl3Reg |= cPHY_CTRL3_TMR3CMP_EN;   // enable TMR3 compare
  irqSts3Reg &= ~(cIRQSTS3_TMR3MSK);      // unmask TMR3 interrupt
  irqSts3Reg &= 0xF0;                     // do not change IRQ status
  irqSts3Reg |= (cIRQSTS3_TMR3IRQ);       // aknowledge TMR3 IRQ
  
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL3, phyCtrl3Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS3, irqSts3Reg);
  
  //PHY_DEBUG_LOG3(PDBG_SET_EVENT_TIMEOUT, PP_PHY_T3CMP16, PP_PHY_T3CMP8, PP_PHY_T3CMP0);; //TODO
}

/*---------------------------------------------------------------------------
 * Name: PhyTimeDisableEventTimeout
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyTimeDisableEventTimeout
(
  void
)
{
  uint8_t phyCtrl4Reg, phyCtrl3Reg, irqSts3Reg;
  
  //PHY_DEBUG_LOG(PDBG_DISABLE_EVENT_TIMEOUT); TODO
  
  phyCtrl4Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL4);
  phyCtrl3Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL3);
  
  phyCtrl4Reg &= ~(cPHY_CTRL4_TC3TMOUT);  // disable autosequence stop by TC3 match
  phyCtrl3Reg &= ~(cPHY_CTRL3_TMR3CMP_EN);// disable TMR3 compare
  
  irqSts3Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS3);
  irqSts3Reg |= cIRQSTS3_TMR3IRQ;         // aknowledge TMR3 IRQ
  
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL3, phyCtrl3Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL4, phyCtrl4Reg);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) IRQSTS3, irqSts3Reg);
}

/*---------------------------------------------------------------------------
 * Name: PhyTimeReadClock
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyTimeReadClock
(
  zbClock24_t *pRetClk
)
{
#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pRetClk)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION
  
  //PHY_DEBUG_LOG(PDBG_READ_CLOCK); TODO
  
  MC1324xDrv_DirectAccessSPIMultiByteRead( (uint8_t) EVENT_TMR_LSB, (uint8_t *) pRetClk, 3);
  *(((uint8_t *)pRetClk) + 3) = 0;
}

/*---------------------------------------------------------------------------
 * Name: PhyTimeInitEventTimer
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyTimeInitEventTimer
(
  zbClock24_t *pAbsTime
)
{
  uint8_t phyCtrl4Reg;
    
#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pAbsTime)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION

  //PHY_DEBUG_LOG(PDBG_INIT_EVENT_TIMER); TODO

  phyCtrl4Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL4);
  phyCtrl4Reg |= cPHY_CTRL4_TMRLOAD; // self clearing bit
  
  MC1324xDrv_DirectAccessSPIMultiByteWrite( (uint8_t) T1CMP_LSB, (uint8_t *) pAbsTime, 3);
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL4, phyCtrl4Reg);

}