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

// Address mode indentifiers. Used for both network and MAC interfaces
#define gPhyAddrModeNoAddr_c        (0)
#define gPhyAddrModeInvalid_c       (1)
#define gPhyAddrMode16BitAddr_c     (2)
#define gPhyAddrMode64BitAddr_c     (3)

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
 * Name: PhyPpSetPromiscuous
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetPromiscuous
(
  bool_t mode
)
{
  uint8_t phyCtrl4Reg;
  
  phyCtrl4Reg = MC1324xDrv_DirectAccessSPIRead( (uint8_t) PHY_CTRL4);
  
  if(TRUE == mode)
  {
    phyCtrl4Reg |= cPHY_CTRL4_PROMISCUOUS;
  }
  else
  {
    phyCtrl4Reg &= ~cPHY_CTRL4_PROMISCUOUS;
  }
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL4, phyCtrl4Reg);
}

/*---------------------------------------------------------------------------
 * Name: PhyPpSetPanIdPAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetPanIdPAN0
(
  uint8_t *pPanId
)
{
  //PHY_DEBUG_LOG(PDBG_PP_SET_PANID_PAN0); TODO

#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pPanId)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION
  
  MC1324xDrv_IndirectAccessSPIMultiByteWrite((uint8_t) MACPANID0_LSB, pPanId, 2);
}

/*---------------------------------------------------------------------------
 * Name: PhyPpSetPanIdPAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetPanIdPAN1
(
  uint8_t *pPanId
)
{
  //PHY_DEBUG_LOG(PDBG_PP_SET_PANID_PAN1); TODO

#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pPanId)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION

  MC1324xDrv_IndirectAccessSPIMultiByteWrite((uint8_t) MACPANID1_LSB, pPanId, 2);
}

/*---------------------------------------------------------------------------
 * Name: PhyPpSetShortAddrPAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetShortAddrPAN0
(
  uint8_t *pShortAddr
)
{
  //PHY_DEBUG_LOG(PDBG_PP_SET_SHORTADDR_PAN0); TODO
  
#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pShortAddr)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION

  MC1324xDrv_IndirectAccessSPIMultiByteWrite((uint8_t) MACSHORTADDRS0_LSB, pShortAddr, 2);
}

/*---------------------------------------------------------------------------
 * Name: PhyPpSetShortAddrPAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetShortAddrPAN1
(
  uint8_t *pShortAddr
)
{
  //PHY_DEBUG_LOG(PDBG_PP_SET_SHORTADDR_PAN1); TODO
  
#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pShortAddr)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION

  MC1324xDrv_IndirectAccessSPIMultiByteWrite((uint8_t) MACSHORTADDRS1_LSB, pShortAddr, 2);
}

/*---------------------------------------------------------------------------
 * Name: PhyPpSetLongAddrPAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetLongAddrPAN0
(
  uint8_t *pLongAddr
)
{
  //PHY_DEBUG_LOG(PDBG_PP_SET_LONGADDR_PAN0); TODO

#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pLongAddr)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION

  MC1324xDrv_IndirectAccessSPIMultiByteWrite((uint8_t) MACLONGADDRS0_0, pLongAddr, 8);
}

/*---------------------------------------------------------------------------
 * Name: PhyPpSetLongAddrPAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetLongAddrPAN1
(
  uint8_t *pLongAddr
)
{
  //PHY_DEBUG_LOG(PDBG_PP_SET_LONGADDR_PAN1); TODO

#ifdef PHY_PARAMETERS_VALIDATION
  if(NULL == pLongAddr)
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION

  MC1324xDrv_IndirectAccessSPIMultiByteWrite((uint8_t) MACLONGADDRS1_0, pLongAddr, 8);
}

/*---------------------------------------------------------------------------
 * Name: PhyPpSetMacRolePAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetMacRolePAN0
(
  bool_t macRole
)
{
  uint8_t phyCtrl4Reg;
  
  //PHY_DEBUG_LOG(PDBG_PP_SET_MACROLE_PAN0); TODO
  
  phyCtrl4Reg = MC1324xDrv_DirectAccessSPIRead( (uint8_t) PHY_CTRL4);
  
  if(gMacRole_PanCoord_c == macRole)
  {
    phyCtrl4Reg |=  cPHY_CTRL4_PANCORDNTR0;
  }
  else
  {
    phyCtrl4Reg &= ~cPHY_CTRL4_PANCORDNTR0;
  }
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) PHY_CTRL4, phyCtrl4Reg);
}

/*---------------------------------------------------------------------------
 * Name: PhyPpSetMacRolePAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetMacRolePAN1
(
  bool_t macRole
)
{
  uint8_t dualPanCtrlReg;
  
  //PHY_DEBUG_LOG(PDBG_PP_SET_MACROLE_PAN1); TODO
  
  dualPanCtrlReg = MC1324xDrv_IndirectAccessSPIRead( (uint8_t) DUAL_PAN_CTRL);
  
  if(gMacRole_PanCoord_c == macRole)
  {
    dualPanCtrlReg |=  cDUAL_PAN_CTRL_PANCORDNTR1;
  }
  else
  {
    dualPanCtrlReg &= ~cDUAL_PAN_CTRL_PANCORDNTR1;
  }
  MC1324xDrv_IndirectAccessSPIWrite( (uint8_t) DUAL_PAN_CTRL, dualPanCtrlReg);
}

/*---------------------------------------------------------------------------
 * Name: PhyPpIsTxAckDataPending
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
bool_t PhyPpIsTxAckDataPending
(
  void
)
{
  uint8_t irqsts2Reg;
  irqsts2Reg = MC1324xDrv_DirectAccessSPIRead((uint8_t) IRQSTS2);
  if(irqsts2Reg & cIRQSTS2_SRCADDR)
  {
    return TRUE;
  }
  return FALSE;
}

/*---------------------------------------------------------------------------
 * Name: PhyPpIsRxAckDataPending
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
bool_t PhyPpIsRxAckDataPending
(
  void
)
{
  uint8_t irqsts1Reg;
  irqsts1Reg = MC1324xDrv_DirectAccessSPIRead((uint8_t) IRQSTS1);
  if(irqsts1Reg & cIRQSTS1_RX_FRM_PEND)
  {
    return TRUE;
  }
  return FALSE;
}

/*---------------------------------------------------------------------------
 * Name: PhyPpIsPollIndication
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
bool_t PhyPpIsPollIndication
(
  void
)
{
  uint8_t irqsts2Reg;
  irqsts2Reg = MC1324xDrv_DirectAccessSPIRead((uint8_t) IRQSTS2);
  if(irqsts2Reg & cIRQSTS2_PI)
  {
    return TRUE;
  }
  return FALSE;
}

/*---------------------------------------------------------------------------
 * Name: PhyPpSetCcaThreshold
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetCcaThreshold(uint8_t ccaThreshold)
{
  MC1324xDrv_IndirectAccessSPIWrite((uint8_t) CCA1_THRESH, (uint8_t) ccaThreshold);
}

/*---------------------------------------------------------------------------
 * Name: PhyPp_AddToIndirect
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPp_AddToIndirect
(
  uint8_t index,
  uint8_t *pPanId,
  uint8_t *pAddr,
  uint8_t AddrMode
)
{
  uint16_t srcAddressCheckSum;
  uint8_t srcCtrlReg;
  
  //PHY_DEBUG_LOG1(PDBG_PP_ADD_INDIRECT, index); TODO
  
#ifdef PHY_PARAMETERS_VALIDATION
  if((NULL == pPanId) || (NULL == pAddr))
  {
    return;
  }
  if( (gPhyAddrMode64BitAddr_c != AddrMode) && (gPhyAddrMode16BitAddr_c != AddrMode) )
  {
    return;
  }
#endif // PHY_PARAMETERS_VALIDATION

  srcAddressCheckSum = (uint16_t ) ( (uint16_t) pPanId[0] | (uint16_t) (pPanId[1] << 8) );
      srcAddressCheckSum += (uint16_t ) ( (uint16_t) pAddr[0] | (uint16_t) (pAddr[1] << 8) );

      if(AddrMode == gPhyAddrMode64BitAddr_c)
      {
          srcAddressCheckSum += (uint16_t ) ( (uint16_t) pAddr[2] | (uint16_t) (pAddr[3] << 8) );
          srcAddressCheckSum += (uint16_t ) ( (uint16_t) pAddr[4] | (uint16_t) (pAddr[5] << 8) );
          srcAddressCheckSum += (uint16_t ) ( (uint16_t) pAddr[6] | (uint16_t) (pAddr[7] << 8) );
      }

      srcCtrlReg = (uint8_t) ( (index & cSRC_CTRL_INDEX) << cSRC_CTRL_INDEX_Shift_c );

      MC1324xDrv_DirectAccessSPIWrite( (uint8_t) SRC_CTRL, srcCtrlReg);

      MC1324xDrv_DirectAccessSPIMultiByteWrite( (uint8_t) SRC_ADDRS_SUM_LSB, (uint8_t *) &srcAddressCheckSum, 2);

      srcCtrlReg |= ( cSRC_CTRL_SRCADDR_EN | cSRC_CTRL_INDEX_EN );

      MC1324xDrv_DirectAccessSPIWrite( (uint8_t) SRC_CTRL, srcCtrlReg);
}

/*---------------------------------------------------------------------------
 * Name: PhyPp_RemoveFromIndirect
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPp_RemoveFromIndirect
(
  uint8_t index
)
{
  uint8_t srcCtrlReg;
  
  //PHY_DEBUG_LOG1(PDBG_PP_REMOVE_INDIRECT, index); TODO
  
  srcCtrlReg = (uint8_t)( ( (index & cSRC_CTRL_INDEX) << cSRC_CTRL_INDEX_Shift_c )
                             |( cSRC_CTRL_SRCADDR_EN )
                                 |( cSRC_CTRL_INDEX_DISABLE) );
  
  MC1324xDrv_DirectAccessSPIWrite( (uint8_t) SRC_CTRL, srcCtrlReg);
}

/*---------------------------------------------------------------------------
 * Name: PhyPpReadLatestIndex
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPpReadLatestIndex
(
  void
)
{
	uint8_t srcCtrlReg;

	    srcCtrlReg = MC1324xDrv_DirectAccessSPIRead( (uint8_t) SRC_CTRL);

	    srcCtrlReg = (srcCtrlReg >> cSRC_CTRL_INDEX_Shift_c);

	    //PHY_DEBUG_LOG1(PDBG_PP_READ_LATEST_INDEX, srcCtrlReg); //TODO

	    return srcCtrlReg;
}

/*---------------------------------------------------------------------------
 * Name: PhyPpGetState
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPpGetState
(
  void
)
{
  return (uint8_t)( MC1324xDrv_DirectAccessSPIRead( (uint8_t) PHY_CTRL1) & cPHY_CTRL1_XCVSEQ );
}

/*---------------------------------------------------------------------------
 * Name: PhyAbort
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyAbort(void)
{

  uint8_t irqSts1Reg, irqSts2Reg, irqSts3Reg, phyCtrl1Reg, phyCtrl2Reg, phyCtrl3Reg, phyCtrl4Reg;
  volatile uint8_t time = 0;
  uint32_t irqMaskRegister;

  asm(" MRS %0,PRIMASK" : "=r" (irqMaskRegister) );
  asm(" CPSID i");
  
  //PHY_DEBUG_LOG(PDBG_PHY_ABORT); TODO

  
  // disable timer trigger of the already programmed packet processor sequence, if any
  phyCtrl1Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL1);
  phyCtrl1Reg &= (uint8_t) ~(cPHY_CTRL1_TMRTRIGEN);
  MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL1, phyCtrl1Reg);
  
  // give the FSM enough time to start if it was triggered
  time = MC1324xDrv_DirectAccessSPIRead(EVENT_TMR_LSB);
  while(MC1324xDrv_DirectAccessSPIRead(EVENT_TMR_LSB) < (uint8_t) (time + 6));//TODO
  
  // make sure that we abort in HW only if the sequence was actually started (tmr triggered)
  if((gIdle_c != PhyGetSeqState()) && ((MC1324xDrv_DirectAccessSPIRead(SEQ_STATE)&0x1F) != 0))
  {
    phyCtrl1Reg &= (uint8_t) ~(cPHY_CTRL1_XCVSEQ);
    MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL1, phyCtrl1Reg);
    while ((MC1324xDrv_DirectAccessSPIRead(SEQ_STATE) & 0x1F) != 0);
  }
  
 
  phyCtrl2Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL2);
  phyCtrl3Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL3);
  phyCtrl4Reg = MC1324xDrv_DirectAccessSPIRead(PHY_CTRL4);

  // mask SEQ interrupt
  phyCtrl2Reg |= (uint8_t) (cPHY_CTRL2_SEQMSK);
  MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL2, phyCtrl2Reg);
 
  // stop timers
  phyCtrl4Reg &= (uint8_t) ~(cPHY_CTRL4_TC3TMOUT);
  phyCtrl3Reg &= (uint8_t) ~(cPHY_CTRL3_TMR2CMP_EN | cPHY_CTRL3_TMR3CMP_EN);
  MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL3, phyCtrl3Reg);
  
  PhyPassRxParams(NULL);
  PhyPassRxDataPtr(NULL);
  
  // clear all PP IRQ bits to avoid unexpected interrupts
  irqSts1Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS1);
  irqSts2Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS2);
  irqSts3Reg = MC1324xDrv_DirectAccessSPIRead(IRQSTS3);
  
  irqSts3Reg |= (uint8_t) (cIRQSTS3_TMR2MSK | cIRQSTS3_TMR3MSK);
  
  MC1324xDrv_DirectAccessSPIWrite(IRQSTS1, irqSts1Reg);
  MC1324xDrv_DirectAccessSPIWrite(IRQSTS2, irqSts2Reg);
  MC1324xDrv_DirectAccessSPIWrite(IRQSTS3, irqSts3Reg);

  asm(" MSR PRIMASK, %0" : : "r" (irqMaskRegister) );
}

/*---------------------------------------------------------------------------
 * Name: PhyInit
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyInit(void)
{
  uint8_t u8PartId = 0;
  PhyPassRxParams(NULL);
  PhyPassRxDataPtr(NULL);
  
  //@AC added: Verify the Radio IC version Indirect register 0x00
  
  u8PartId = MC1324xDrv_IndirectAccessSPIRead(PART_ID);
  
  if((gRadioPartId_2p0 == u8PartId)|| (gRadioPartId_2p1 == u8PartId))
  {
      // Direct Registers
      MC1324xDrv_DirectAccessSPIWrite(OVERWRITE_VER, 0x0C);  //version 0C: new value for ACKDELAY targeting 198us (23 May, 2013, Larry Roshak)
      MC1324xDrv_DirectAccessSPIWrite(PA_PWR,0x17);          // 9:PA_PWR new default Power Step is "23"  (24 Jan, 2013, Doug Shade)   
      
      MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL4,0x4B);       // TC3TMOUT, CCATYPE, PROMISCUOUS, TC2PRIME_EN bits set on PHYCTRL4 reg (@SMAC)
      MC1324xDrv_DirectAccessSPIWrite(SRC_CTRL,0x01);        // disable Index for source address table feature (@SMAC)
      
      // Indirect Registers  
      MC1324xDrv_IndirectAccessSPIWrite(RX_FRAME_FILTER, 0x1F); 
      
      /* Overwrites section start */
      MC1324xDrv_IndirectAccessSPIWrite(0x31, 0x02);        //clear MISO_HIZ_EN (for single SPI master/slave pair) and SPI_PUL_EN (minimize HIB currents) (17 Apr 2012, D. Brown)
      MC1324xDrv_IndirectAccessSPIWrite(0x91, 0xB3);        //VCO_CTRL1: override VCOALC_REF_TX to 3 (13 Sep 2012, D. Brown)
      MC1324xDrv_IndirectAccessSPIWrite(0x92, 0x07);        //VCO_CTRL2: override VCOALC_REF_RX to 3, keep VCO_BUF_BOOST = 1 (13 Sep 2012, D. Brown)
      MC1324xDrv_IndirectAccessSPIWrite(0x8A, 0x71);        //PA_TUNING: override PA_COILTUNING to 001 (27 Nov 2012, D. Brown, on behalf of S. Eid)
      MC1324xDrv_IndirectAccessSPIWrite(0x79, 0x2F);        //CHF_IBUF  Adjust the gm-C filter gain (+/- 6dB)         (21 Dec, 2012, on behalf of S. Soca) 
      MC1324xDrv_IndirectAccessSPIWrite(0x7A, 0x2F);        //CHF_QBUF  Adjust the gm-C filter gain (+/- 6dB)         (21 Dec, 2012, on behalf of S. Soca) 
      MC1324xDrv_IndirectAccessSPIWrite(0x7B, 0x24);        //CHF_IRIN  Adjust the filter bandwidth (+/- 0.5MHz)      (21 Dec, 2012, on behalf of S. Soca) 
      MC1324xDrv_IndirectAccessSPIWrite(0x7C, 0x24);        //CHF_QRIN  Adjust the filter bandwidth (+/- 0.5MHz)      (21 Dec, 2012, on behalf of S. Soca) 
      MC1324xDrv_IndirectAccessSPIWrite(0x7D, 0x24);        //CHF_IL    Adjust the filter bandwidth (+/- 0.5MHz)      (21 Dec, 2012, on behalf of S. Soca) 
      MC1324xDrv_IndirectAccessSPIWrite(0x7E, 0x24);        //CHF_QL    Adjust the filter bandwidth (+/- 0.5MHz)      (21 Dec, 2012, on behalf of S. Soca)
      MC1324xDrv_IndirectAccessSPIWrite(0x7F, 0x32);        //CHF_CC1   Adjust the filter center frequency (+/- 1MHz) (21 Dec, 2012, on behalf of S. Soca) 
      MC1324xDrv_IndirectAccessSPIWrite(0x80, 0x1D);        //CHF_CCL   Adjust the filter center frequency (+/- 1MHz) (21 Dec, 2012, on behalf of S. Soca) 
      MC1324xDrv_IndirectAccessSPIWrite(0x81, 0x2D);        //CHF_CC2   Adjust the filter center frequency (+/- 1MHz) (21 Dec, 2012, on behalf of S. Soca)  
      MC1324xDrv_IndirectAccessSPIWrite(0x82, 0x24);        //CHF_IROUT Adjust the filter bandwidth (+/- 0.5MHz)      (21 Dec, 2012, on behalf of S. Soca) 
      MC1324xDrv_IndirectAccessSPIWrite(0x83, 0x24);        //CHF_QROUT Adjust the filter bandwidth (+/- 0.5MHz)      (21 Dec, 2012, on behalf of S. Soca) 
      MC1324xDrv_IndirectAccessSPIWrite(0x64, 0x28);        //PA_CAL_DIS=1  Disabled PA calibration (3 Jan 2013, Andy Chiang)
      MC1324xDrv_IndirectAccessSPIWrite(0x52, 0x55);        //AGC_THR1 RSSI tune up (24 Jan 2013, Andy Chiang)
      MC1324xDrv_IndirectAccessSPIWrite(0x53, 0x2D);        //AGC_THR2 RSSI tune up (7 Jan 2013, Andy Chiang)
      MC1324xDrv_IndirectAccessSPIWrite(0x66, 0x5F);        //ATT_RSSI1 tune up     (7 Jan 2013, Andy Chiang)
      MC1324xDrv_IndirectAccessSPIWrite(0x67, 0x8F);        //ATT_RSSI2 tune up     (7 Jan 2013, Andy Chiang)
      MC1324xDrv_IndirectAccessSPIWrite(0x68, 0x61);        //RSSI_OFFSET (9 Jan 2013, Andy Chiang)
      MC1324xDrv_IndirectAccessSPIWrite(0x78, 0x03);        //CHF_PMAGAIN (9 Jan 2013, Andy Chiang)
      MC1324xDrv_IndirectAccessSPIWrite(0x22, 0x50);        //CCA1_THRESH (11 Feb 2013, Andy Chiang)
      MC1324xDrv_IndirectAccessSPIWrite(0x4D, 0x13);        //Moved from 0x14 to 0x13 for 0.5 dB improved Rx Sensitivity (01 Apr 2013, D. Shade)
      MC1324xDrv_IndirectAccessSPIWrite(0x39, 0x3D);        //ACKDELAY new value targeting a delay of 198us (23 May, 2013, Larry Roshak)
     /* Overwrites section ends */
  }
  else
  {
      if((gRadioPartId_TV1 == u8PartId) || (gRadioPartId_TV2 == u8PartId) || (gRadioPartId_TV3 == u8PartId))
      {
          // Direct Registers
          MC1324xDrv_DirectAccessSPIWrite(OVERWRITE_VER, 0x05);   
          MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL4,0x4B);
          MC1324xDrv_DirectAccessSPIWrite(SRC_CTRL,0x01); // disable Index for source address table feature
         
          // Indirect Registers  
          MC1324xDrv_IndirectAccessSPIWrite(RX_FRAME_FILTER, 0x1F); 
          
          /* Overwrites section start */
          MC1324xDrv_IndirectAccessSPIWrite(0x31, 0x02); //clear MISO_HIZ_EN (for single SPI master/slave pair) and SPI_PUL_EN (minimize HIB currents) (17 Apr 2012, D. Brown)
          MC1324xDrv_IndirectAccessSPIWrite(0x79, 0x1B); //fine tune I-Channel gm-C filter gain (+/- 6dB) CHF_IBUF[5:0]=0x1B  (23 Apr, 2012, Jalaleddine)
          MC1324xDrv_IndirectAccessSPIWrite(0x7A, 0x1B); //fine tune Q-Channel gm-C filter gain (+/- 6dB) CHF_QBUF[5:0]=0x1B  (23 Apr, 2012, Jalaleddine)
          MC1324xDrv_IndirectAccessSPIWrite(0x7B, 0x20); //fine tune I-Channel CHF_IRIN filter bandwidth (+/- 0.5MHz)  CHF_IRIN[5:0]=0x20  (23 Apr, 2012, Jalaleddine)
          MC1324xDrv_IndirectAccessSPIWrite(0x7C, 0x20); //fine tune Q-Channel CHF_QRIN filter bandwidth (+/- 0.5MHz)  CHF_QRIN[5:0]=0x20  (23 Apr, 2012, Jalaleddine)
          MC1324xDrv_IndirectAccessSPIWrite(0x7D, 0x20); //fine tune I-Channel CHF_IL filter bandwidth (+/- 0.5MHz)  CHF_IL[5:0]=0x20  (23 Apr, 2012, Jalaleddine)
          MC1324xDrv_IndirectAccessSPIWrite(0x7E, 0x20); //fine tune Q-Channel CHF_QL filter bandwidth (+/- 0.5MHz)  CHF_QL[5:0]=0x20  (23 Apr, 2012, Jalaleddine)
          MC1324xDrv_IndirectAccessSPIWrite(0x82, 0x20); //fine tune I-Channel CHF_IROUT filter bandwidth (+/- 0.5MHz)  CHF_IROUT[5:0]=0x20  (23 Apr, 2012, Jalaleddine)
          MC1324xDrv_IndirectAccessSPIWrite(0x83, 0x20); //fine tune Q-Channel CHF_QROUT filter bandwidth (+/- 0.5MHz)  CHF_QROUT[5:0]=0x20  (23 Apr, 2012, Jalaleddine)
          MC1324xDrv_IndirectAccessSPIWrite(0x7F, 0x30); //fine tune I-Channel CHF_CC1 center frequency (+/- 1MHz)  CHF_CC1[5:0]=0x30  (23 Apr, 2012, Jalaleddine)
          MC1324xDrv_IndirectAccessSPIWrite(0x80, 0x1B); //fine tune Q-Channel CHF_CCL center frequency (+/- 1MHz)  CHF_CCL[5:0]=0x1B  (23 Apr, 2012, Jalaleddine)
          MC1324xDrv_IndirectAccessSPIWrite(0x81, 0x2B); //fine tune I-Channel CHF_CC2 center frequency (+/- 1MHz)  CHF_CC2[5:0]=0x2B  (23 Apr, 2012, Jalaleddine)
          MC1324xDrv_IndirectAccessSPIWrite(0x67, 0x11); //ATT_RSSI2 register: Disable RSSI Auto Calibration (see TKT116800)
          MC1324xDrv_IndirectAccessSPIWrite(0x64, 0x28); //set PA_CAL_DIS=1 to reduce current consumption of the PA output stage (25 Apr, 2012, A.Chiang)
          MC1324xDrv_IndirectAccessSPIWrite(0x92, 0x01); //VCO_BUF_BOOST = 1
          MC1324xDrv_IndirectAccessSPIWrite(0x75, 0x00); //LNA_BIAS_BOOST[1:0] = 00
          MC1324xDrv_IndirectAccessSPIWrite(0x52, 0x3C); //AGC_THR1 RSSI response tune up (06 Aug, 2012, Murthy/Delbecq)
          MC1324xDrv_IndirectAccessSPIWrite(0x53, 0xFF); //AGC_THR2 RSSI response tune up, basically disables LNA high attenuation mode (06 Aug, 2012, Murthy/Delbecq)
          MC1324xDrv_IndirectAccessSPIWrite(0x66, 0x5F); //ATT_RSS1 RSSI response tune up (06 Aug, 2012, Murthy/Delbecq)
          MC1324xDrv_IndirectAccessSPIWrite(0x23, 0x6D); //CCA1_ED_OFFSET_COM RSSI response tune up (06 Aug, 2012, Murthy/Delbecq)
          MC1324xDrv_IndirectAccessSPIWrite(0x24, 0x24); //LQI_OFFSET_COMP    RSSI response tune up (06 Aug, 2012, Murthy/Delbecq)
          MC1324xDrv_IndirectAccessSPIWrite(0x61, 0x36); //VCO_BIAS Disables ALC, sets VCO_CAL to 0x16 (06 Aug, 2012, Jalaleddine)
         /* Overwrites section ends */
      }
  }
           
      PhyPlmeSetCurrentChannelRequest(0x0B);        // Set Initial freq. at 2405 MHz
      PhyPlmeSetPwrLevelRequest(0x17);              // Set Initial power level to 0dBm
      PhyPpSetCcaThreshold(0x4B);                   // set CCA1 threshold to -75 dBm
      
      MC1324xDrv_IRQ_Clear();
      NVIC_ClearPendingIRQ(MC1324x_Irq_Number);
  
}