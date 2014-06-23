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
#define PHY_IRQSTS1_INDEX_c     0x00
#define PHY_IRQSTS2_INDEX_c     0x01
#define PHY_IRQSTS3_INDEX_c     0x02
#define PHY_CTRL1_INDEX_c       0x03
#define PHY_CTRL2_INDEX_c       0x04
#define PHY_CTRL3_INDEX_c       0x05
#define PHY_RX_FRM_LEN_INDEX_c  0x06
#define PHY_CTRL4_INDEX_c       0x07

static phyRxParams_t * mpRxParams = NULL;
static phyPacket_t * mpRxData = NULL;
uint8_t mStatusAndControlRegs[10];

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
 * Name: PhyPassRxParams()
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPassRxParams
(
  phyRxParams_t * pRxParam
)
{
  mpRxParams = pRxParam;
}

/*---------------------------------------------------------------------------
 * Name: PhyPassRxDataPtr()
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPassRxDataPtr
(
  phyPacket_t * pRxData
)
{
  mpRxData = pRxData;
}

/*---------------------------------------------------------------------------
 * Name: PhyIsrSeqCleanup
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyIsrSeqCleanup
(
  void
)
{
  mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] &= 0xF0;
  mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] &= (uint8_t) ~( cIRQSTS3_TMR3MSK ); // unmask TMR3 interrupt
  mStatusAndControlRegs[PHY_CTRL2_INDEX_c]   |= (uint8_t)  ( cPHY_CTRL2_CCAMSK | \
                                                             cPHY_CTRL2_RXMSK | \
                                                             cPHY_CTRL2_TXMSK | \
                                                             cPHY_CTRL2_SEQMSK);
  mStatusAndControlRegs[PHY_CTRL1_INDEX_c]   &= (uint8_t) ~( cPHY_CTRL1_XCVSEQ);
  
  // clear transceiver interrupts, mask SEQ, RX, TX and CCA interrupts and set the PHY sequencer back to IDLE
  MC1324xDrv_DirectAccessSPIMultiByteWrite(IRQSTS1, mStatusAndControlRegs, 5);
  
}

/*---------------------------------------------------------------------------
 * Name: PhyIsrTimeoutCleanup
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyIsrTimeoutCleanup
(
  void
)
{
  mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] &= 0xF0;
  mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] |= (uint8_t)  ( cIRQSTS3_TMR3MSK | \
                                                             cIRQSTS3_TMR3IRQ); // mask and clear TMR3 interrupt
  mStatusAndControlRegs[PHY_CTRL2_INDEX_c]   |= (uint8_t)  ( cPHY_CTRL2_CCAMSK | \
                                                             cPHY_CTRL2_RXMSK | \
                                                             cPHY_CTRL2_TXMSK | \
                                                             cPHY_CTRL2_SEQMSK);
  mStatusAndControlRegs[PHY_CTRL1_INDEX_c]   &= (uint8_t) ~( cPHY_CTRL1_XCVSEQ);
  
  // disable TMR3 comparator and timeout
  mStatusAndControlRegs[PHY_CTRL3_INDEX_c]   &= (uint8_t) ~( cPHY_CTRL3_TMR3CMP_EN);
  mStatusAndControlRegs[PHY_CTRL4_INDEX_c]   &= (uint8_t) ~( cPHY_CTRL4_TC3TMOUT);
  
  MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL3, mStatusAndControlRegs[PHY_CTRL3_INDEX_c]);
  MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL4, mStatusAndControlRegs[PHY_CTRL4_INDEX_c]);
  
  // clear transceiver interrupts, mask mask SEQ, RX, TX, TMR3 and CCA interrupts interrupts and set the PHY sequencer back to IDLE
  MC1324xDrv_DirectAccessSPIMultiByteWrite(IRQSTS1, mStatusAndControlRegs, 5);
}

/*---------------------------------------------------------------------------
 * Name: Phy_GetEnergyLevel
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t Phy_GetEnergyLevel
(
  void
)
{
  uint8_t energyLevel;
  energyLevel = MC1324xDrv_DirectAccessSPIRead((uint8_t) CCA1_ED_FNL);
  // TODO to convert energy level based on spec.
  return energyLevel;
}

/*---------------------------------------------------------------------------
 * Name: PHY_InterruptHandler
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PHY_InterruptHandler
(
  void
)
{
  uint8_t xcvseqCopy;
  
  // disable and clear transceiver(IRQ_B) interrupt
  MC1324xDrv_IRQ_Disable();
  MC1324xDrv_IRQ_Clear();
  
  // read transceiver interrupt status and control registers
  MC1324xDrv_DirectAccessSPIMultiByteRead(IRQSTS1, mStatusAndControlRegs, 8);
  
  xcvseqCopy = mStatusAndControlRegs[PHY_CTRL1] & cPHY_CTRL1_XCVSEQ;
  
  /* debugg */
  if(   ((mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c] & cIRQSTS2_WAKE_IRQ) == cIRQSTS2_WAKE_IRQ)
      &&((mStatusAndControlRegs[PHY_CTRL3_INDEX_c] & cPHY_CTRL3_WAKE_MSK) != cPHY_CTRL3_WAKE_MSK) )
  {
    // clear transceiver interrupts
    MC1324xDrv_DirectAccessSPIMultiByteWrite(IRQSTS1, mStatusAndControlRegs, 3);
    //PhyUnexpectedTransceiverReset();
    MC1324xDrv_IRQ_Enable();
    return;
  }
  /* ----- */
 
  // sequencer interrupt, the autosequence has completed
  if( (mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] & cIRQSTS1_SEQIRQ) == cIRQSTS1_SEQIRQ )
  {
    // PLL unlock, the autosequence has been aborted due to PLL unlock
    if( (mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] & cIRQSTS1_PLL_UNLOCK_IRQ) == cIRQSTS1_PLL_UNLOCK_IRQ )
    {
      PhyIsrSeqCleanup();
      PhyPlmeSyncLossIndication();
      MC1324xDrv_IRQ_Enable();
      return;
    }
    // TMR3 timeout, the autosequence has been aborted due to TMR3 timeout
    if(  ((mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] & cIRQSTS3_TMR3IRQ) == cIRQSTS3_TMR3IRQ)
       &&((mStatusAndControlRegs[PHY_IRQSTS1_INDEX_c] & cIRQSTS1_RXIRQ)   != cIRQSTS1_RXIRQ) )
    {
      PhyIsrTimeoutCleanup();
      PhyTimeRxTimeoutIndication();
      MC1324xDrv_IRQ_Enable();
      return;
    }
    
    PhyIsrSeqCleanup();
    
    switch(xcvseqCopy)
    {
      case gTX_c:
      {
        if(  ((mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c] & cIRQSTS2_CCA)        == cIRQSTS2_CCA)
           &&((mStatusAndControlRegs[PHY_CTRL1_INDEX_c]   & cPHY_CTRL1_CCABFRTX) == cPHY_CTRL1_CCABFRTX) )
        {
          PhyPlmeCcaConfirm(gChannelBusy_c);
        }
        else
        {
          PhyPdDataConfirm();
        }
      }
      break;
      case gTR_c:
      {
        if(  ((mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c] & cIRQSTS2_CCA)        == cIRQSTS2_CCA)
           &&((mStatusAndControlRegs[PHY_CTRL1_INDEX_c]   & cPHY_CTRL1_CCABFRTX) == cPHY_CTRL1_CCABFRTX) )
        {
          PhyPlmeCcaConfirm(gChannelBusy_c);
        }
        else
        {
          if(NULL != mpRxParams)
          {
            // TODO to convert LQI based on spec.
            mpRxParams->linkQuality = MC1324xDrv_DirectAccessSPIRead((uint8_t) LQI_VALUE);
            MC1324xDrv_DirectAccessSPIMultiByteRead( (uint8_t) TIMESTAMP_LSB, (uint8_t *) mpRxParams->timeStamp, 3);
          }
          PhyPdDataConfirm();
        }
      }
      break;
      case gRX_c:
      {
        if(NULL != mpRxParams)
        {
          // TODO to convert LQI based on spec.
          mpRxParams->linkQuality = MC1324xDrv_DirectAccessSPIRead((uint8_t) LQI_VALUE);
          MC1324xDrv_DirectAccessSPIMultiByteRead( (uint8_t) TIMESTAMP_LSB, (uint8_t *) mpRxParams->timeStamp, 3);
        }
        if(NULL != mpRxData)
        {
          MC1324xDrv_PB_SPIBurstRead((uint8_t *) (&mpRxData->data[0]), (uint8_t)(mStatusAndControlRegs[PHY_RX_FRM_LEN_INDEX_c] - 2));
          mpRxData->frameLength = (uint8_t)(mStatusAndControlRegs[PHY_RX_FRM_LEN_INDEX_c] - 2);
        }
        
        PhyPdDataIndication();
      }
      break;
      case gCCA_c:
      {
        if( (mStatusAndControlRegs[PHY_CTRL4_INDEX_c] & (cPHY_CTRL4_CCATYPE << cPHY_CTRL4_CCATYPE_Shift_c)) == (gCcaED_c << cPHY_CTRL4_CCATYPE_Shift_c) )
        {
          // Ed
          PhyPlmeEdConfirm(Phy_GetEnergyLevel());
        } 
        else
        {
          // CCA
          if( ((mStatusAndControlRegs[PHY_IRQSTS2_INDEX_c] & cIRQSTS2_CCA) == cIRQSTS2_CCA) )
          {                      
            PhyPlmeCcaConfirm(gChannelBusy_c);
          }
          else
          {
            PhyPlmeCcaConfirm(gChannelIdle_c);
          }
        } 
      }
      break;
      case gCCCA_c:
      {
        PhyPlmeCcaConfirm(gChannelIdle_c);
      }
      break;
      default:
      {
        PhyPlmeSyncLossIndication();
      }
      break;
    }
  }
  // timers interrupt
  else
  {
    if((mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] & cIRQSTS3_TMR2IRQ) == cIRQSTS3_TMR2IRQ)
    {
      // mask and clear TMR2 interrupt
      mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] &= 0xF0;
      mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] |= (uint8_t)  ( cIRQSTS3_TMR2MSK | \
                                                                 cIRQSTS3_TMR2IRQ);
      
      MC1324xDrv_DirectAccessSPIWrite(IRQSTS3, mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c]);
      
      // disable TMR2 comparator and time triggered action
      mStatusAndControlRegs[PHY_CTRL3_INDEX_c]   &= (uint8_t) ~( cPHY_CTRL3_TMR2CMP_EN);
      mStatusAndControlRegs[PHY_CTRL1_INDEX_c]   &= (uint8_t) ~( cPHY_CTRL1_TMRTRIGEN);
      

      
      MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL3, mStatusAndControlRegs[PHY_CTRL3_INDEX_c]);
      MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL4, mStatusAndControlRegs[PHY_CTRL4_INDEX_c]);
      MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL1, mStatusAndControlRegs[PHY_CTRL1_INDEX_c]);
      
      PhyTimeStartEventIndication();
      
    }
    if((mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] & cIRQSTS3_TMR3IRQ) == cIRQSTS3_TMR3IRQ)
    {
      // mask and clear TMR3 interrupt
      mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] &= 0xF0;
      mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] |= (uint8_t)  ( cIRQSTS3_TMR3MSK | \
                                                                 cIRQSTS3_TMR3IRQ);
      
      MC1324xDrv_DirectAccessSPIWrite(IRQSTS3, mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c]);
      
      // disable TMR3 comparator and timeout
      mStatusAndControlRegs[PHY_CTRL3_INDEX_c]   &= (uint8_t) ~( cPHY_CTRL3_TMR3CMP_EN);
      mStatusAndControlRegs[PHY_CTRL4_INDEX_c]   &= (uint8_t) ~( cPHY_CTRL4_TC3TMOUT);
      
      MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL3, mStatusAndControlRegs[PHY_CTRL3_INDEX_c]);
      MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL4, mStatusAndControlRegs[PHY_CTRL4_INDEX_c]);
      
      /* Ensure that we're not issuing TimeoutIndication while the Automated sequence is still in progress */
      /* TMR3 can expire during R-T turnaround for example, case in which the sequence is not interrupted */
      if(gIdle_c == PhyGetSeqState())
      {      
        PhyTimeRxTimeoutIndication();
      }
    }
    if((mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] & cIRQSTS3_TMR1IRQ) == cIRQSTS3_TMR1IRQ)
    {
      // mask and clear TMR1 interrupt
      mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] &= 0xF0;
      mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] |= (uint8_t)  ( cIRQSTS3_TMR1MSK | \
                                                                 cIRQSTS3_TMR1IRQ);
      
      MC1324xDrv_DirectAccessSPIWrite(IRQSTS3, mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c]);
      
      // disable TMR1 comparator
      mStatusAndControlRegs[PHY_CTRL3_INDEX_c]   &= (uint8_t) ~( cPHY_CTRL3_TMR1CMP_EN);
      
      MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL3, mStatusAndControlRegs[PHY_CTRL3_INDEX_c]);

    }
    if((mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] & cIRQSTS3_TMR4IRQ) == cIRQSTS3_TMR4IRQ)
    {
      // mask and clear TMR4 interrupt
      mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] &= 0xF0;
      mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c] |= (uint8_t)  ( cIRQSTS3_TMR4MSK | \
                                                                 cIRQSTS3_TMR4IRQ);
      
      MC1324xDrv_DirectAccessSPIWrite(IRQSTS3, mStatusAndControlRegs[PHY_IRQSTS3_INDEX_c]);
      
      // disable TMR4 comparator
      mStatusAndControlRegs[PHY_CTRL3_INDEX_c]   &= (uint8_t) ~( cPHY_CTRL3_TMR4CMP_EN);
      
      MC1324xDrv_DirectAccessSPIWrite(PHY_CTRL3, mStatusAndControlRegs[PHY_CTRL3_INDEX_c]);
    }
  }
  MC1324xDrv_IRQ_Enable();
}