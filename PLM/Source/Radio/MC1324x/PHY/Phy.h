#ifndef __PHY_H__
#define __PHY_H__

/*****************************************************************************
 *                               INCLUDED HEADERS                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the headers that this module needs to include.    *
 * Note that it is not a good practice to include header files into header   *
 * files, so use this section only if there is no other better solution.     *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
#include "EmbeddedTypes.h"

/*****************************************************************************
 *                             PUBLIC MACROS, DEFINITIONS                    *
 *---------------------------------------------------------------------------*
 * Add to this section all the access macros, registers mappings, bit access *
 * macros, masks, flags etc ...
 *---------------------------------------------------------------------------*
 *****************************************************************************/

#define MAC_PHY_DEBUG

#define MC1324x_Irq_Priority (0xau)
#define gPhyMaxDataLength_c   127

// PHY states
enum {
  gIdle_c = 0,
  gRX_c,
  gTX_c,
  gCCA_c,
  gTR_c,
  gCCCA_c
};

// PHY channel state
enum {
  gChannelIdle_c = 0,
  gChannelBusy_c
};

// PHY requests exit states
enum {
  gPhySuccess_c = 0,
  gPhyBusy_c,
  gPhyInvalidParam_c
};

// PANCORDNTR bit in PP
enum {
  gMacRole_DeviceOrCoord_c,
  gMacRole_PanCoord_c
};

// Cca types
enum {
  gCcaED_c,            // energy detect - CCA bit not active, not to be used for T and CCCA sequences
  gCcaCCA_MODE1_c,     // energy detect - CCA bit ACTIVE
  gCcaCCA_MODE2_c,     // 802.15.4 compliant signal detect - CCA bit ACTIVE
  gCcaCCA_MODE3_c,     // 
  gInvalidCcaType_c    // illegal type
};

//MC1323X: Cca modes
enum {
  gNormalCca_c,
  gContinuousCca_c    
};

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// PhyPdDataRequest and PhyPlmeCcaEdRequest parameter bit map:
//  |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
//  slottedEn   x       x    ackReq   ccaEn  cont.En     ccaType
//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
// slottedEn == 1 -> slotted
// slottedEn == 0 -> unslotted
//   ackReq  == 1 -> TxRxAck
//   ackReq  == 0 -> Tx
//   ccaEn   == 1 -> CcaTx or CcaTxRxAck depending on AckReq
//   ccaEn   == 0 -> no CCA before Tx  or ED scan
//   cont.En == 1 -> Continuous CCA
//   cont.En == 0 -> normal CCA
//   ccaType == 3 -> do not use !
//   ccaType == 2 ->  CCA mode 2
//   ccaType == 1 ->  CCA mode 1
//   ccaType == 0 ->  ED

#define gSlottedEnPos_c    7
#define gAckReqPos_c       4
#define gCcaEnPos_c        3
#define gContinuousEnPos_c 2
#define gCcaTypePos_c      0

#define gSlottedEnMask_c    (1 << gSlottedEnPos_c)
#define gAckReqMask_c       (1 << gAckReqPos_c)
#define gCcaEnMask_c        (1 << gCcaEnPos_c)
#define gContinuousEnMask_c (1 << gContinuousEnPos_c)
#define gCcaTypeMask_c      (3 << gCcaTypePos_c)

// argument definitions for PhyPlmeCcaRequest()

#define gCcaReq_Continuous_Mode1_c             ((gContinuousEnMask_c)| (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Continuous_Mode2_c             ((gContinuousEnMask_c)| (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gCcaReq_Slotted_Mode1_c                ( (gSlottedEnMask_c)  | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Slotted_Mode2_c                ( (gSlottedEnMask_c)  | (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gCcaReq_Unslotted_Mode1_c              (                       (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Unslotted_Mode2_c              (                       (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gCcaReq_Ed_c                           (                       (gCcaEnMask_c) | (gCcaED_c        << gCcaTypePos_c) )
#define gCcaReq_Mode1_c                        (                       (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Mode2_c                        (                       (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gCcaReq_Continuous_Default_c           ((gContinuousEnMask_c)| (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Slotted_Default_c              ( (gSlottedEnMask_c)  | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gCcaReq_Default_c                      (                       (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )

// argument definitions for Tx settings when calling PhyPdDataRequest()
#define gDataReq_NoAck_NoCca_Slotted_c         ( (gSlottedEnMask_c) |                                    (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_NoCca_Unslotted_c       (                                                         (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Slotted_c           ( (gSlottedEnMask_c) |                   (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Unslotted_c         (                                        (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Slotted_c           ( (gSlottedEnMask_c) | (gAckReqMask_c) |                  (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Unslotted_c         (                      (gAckReqMask_c) |                  (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Slotted_c             ( (gSlottedEnMask_c) | (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Unslotted_c           (                      (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )

#define gDataReq_NoAck_NoCca_Slotted_Mode1_c   ( (gSlottedEnMask_c) |                                    (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_NoCca_Unslotted_Mode1_c (                                                         (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Slotted_Mode1_c     ( (gSlottedEnMask_c) |                   (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Unslotted_Mode1_c   (                                        (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Slotted_Mode1_c     ( (gSlottedEnMask_c) | (gAckReqMask_c) |                  (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Unslotted_Mode1_c   (                      (gAckReqMask_c) |                  (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Slotted_Mode1_c       ( (gSlottedEnMask_c) | (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Unslotted_Mode1_c     (                      (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE1_c << gCcaTypePos_c) )

#define gDataReq_NoAck_NoCca_Slotted_Mode2_c   ( (gSlottedEnMask_c) |                                    (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_NoAck_NoCca_Unslotted_Mode2_c (                                                         (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Slotted_Mode2_c     ( (gSlottedEnMask_c) |                   (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_NoAck_Cca_Unslotted_Mode2_c   (                                        (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Slotted_Mode2_c     ( (gSlottedEnMask_c) | (gAckReqMask_c) |                  (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_Ack_NoCca_Unslotted_Mode2_c   (                      (gAckReqMask_c) |                  (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Slotted_Mode2_c       ( (gSlottedEnMask_c) | (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )
#define gDataReq_Ack_Cca_Unslotted_Mode2_c     (                      (gAckReqMask_c) | (gCcaEnMask_c) | (gCcaCCA_MODE2_c << gCcaTypePos_c) )


#define gPHY_IRQ_SEQ_Flag_c  0x00000001
#define gPHY_IRQ_RX_Flag_c   0x00000002
#define gPHY_IRQ_TX_Flag_c   0x00000004
#define gPHY_IRQ_FF_Flag_c   0x00000008

  
#define gRadioPartId_2p1        (0x19)
#define gRadioPartId_2p0        (0x18)
#define gRadioPartId_TV1        (0x48)
#define gRadioPartId_TV2        (0x88)
#define gRadioPartId_TV3        (0xC8)

/* 
        Mfr          Dev        Mask
        ID           Ver         Set
1.0     00           001         000 (1xVCO)
1.0     00           010         000 (2xVCO)
TV1     01           001         000 
TV2     10           001         000 
TV3     11           001         000 
1.1     00           001         001
2.0     00           011         000
*/

/*****************************************************************************
*                             Public type definitions                        *
*****************************************************************************/

typedef struct phyPacket_tag {
  uint8_t frameLength;
  uint8_t data[gPhyMaxDataLength_c];
} phyPacket_t;

typedef struct phyRxParams_tag {
  zbClock24_t timeStamp;
  uint8_t     linkQuality;
} phyRxParams_t;

/*****************************************************************************
*                             Public macros                                  *
*****************************************************************************/

#define PhyGetSeqState()                     PhyPpGetState()
#define PhyPlmeForceTrxOffRequest()          PhyAbort()

#define PhyPpSetPanId(arg)                   PhyPpSetPanIdPAN0(arg)
#define PhyPpSetLongAddr(arg)                PhyPpSetLongAddrPAN0(arg)
#define PhyPpSetShortAddr(arg)               PhyPpSetShortAddrPAN0(arg)
#define PhyPpSetMacRole(arg)                 PhyPpSetMacRolePAN0(arg)

#define PhyPlmeGetCurrentChannelRequest()    PhyPlmeGetCurrentChannelRequestPAN0()
#define PhyPlmeSetCurrentChannelRequest(arg) PhyPlmeSetCurrentChannelRequestPAN0(arg)

#define PhyPlmeEdRequest()                   PhyPlmeCcaEdRequest(gCcaED_c << gCcaTypePos_c)
#define PhyPlmeCcaRequest(arg)               PhyPlmeCcaEdRequest(arg)


/*****************************************************************************
*                             Public prototypes                              *
*****************************************************************************/

// PHY Packet Processor

/*---------------------------------------------------------------------------
 * Name: PhyPpSetPromiscuous
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetPromiscuous
(
  bool_t mode
);

/*---------------------------------------------------------------------------
 * Name: PhyPpSetPanIdPAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetPanIdPAN0
(
  uint8_t *pPanId
);

/*---------------------------------------------------------------------------
 * Name: PhyPpSetPanIdPAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetPanIdPAN1
(
  uint8_t *pPanId
);

/*---------------------------------------------------------------------------
 * Name: PhyPpSetShortAddrPAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetShortAddrPAN0
(
  uint8_t *pShortAddr
);

/*---------------------------------------------------------------------------
 * Name: PhyPpSetShortAddrPAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetShortAddrPAN1
(
  uint8_t *pShortAddr
);

/*---------------------------------------------------------------------------
 * Name: PhyPpSetLongAddrPAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetLongAddrPAN0
(
  uint8_t *pLongAddr
);

/*---------------------------------------------------------------------------
 * Name: PhyPpSetLongAddrPAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetLongAddrPAN1
(
  uint8_t *pLongAddr
);

/*---------------------------------------------------------------------------
 * Name: PhyPpSetMacRolePAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetMacRolePAN0
(
  bool_t macRole
);

/*---------------------------------------------------------------------------
 * Name: PhyPpSetMacRolePAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetMacRolePAN1
(
  bool_t macRole
);

/*---------------------------------------------------------------------------
 * Name: PhyPpIsTxAckDataPending
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
bool_t PhyPpIsTxAckDataPending
(
  void
);

/*---------------------------------------------------------------------------
 * Name: PhyPpIsRxAckDataPending
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
bool_t PhyPpIsRxAckDataPending
(
  void
);

/*---------------------------------------------------------------------------
 * Name: PhyPpIsPollIndication
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
bool_t PhyPpIsPollIndication
(
  void
);

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
);

/*---------------------------------------------------------------------------
 * Name: PhyPp_RemoveFromIndirect
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPp_RemoveFromIndirect
(
  uint8_t index
);

/*---------------------------------------------------------------------------
 * Name: PhyPpReadLatestIndex
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPpReadLatestIndex
(
  void
);

/*---------------------------------------------------------------------------
 * Name: PhyPpGetState
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPpGetState
(
  void
);


/*---------------------------------------------------------------------------
 * Name: PhyAbort
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyAbort
(
  void
);

/*---------------------------------------------------------------------------
 * Name: PhyInit
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyInit
(
  void
);

// PHY PLME & DATA primitives

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
);

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
);

/*---------------------------------------------------------------------------
 * Name: PhyPlmeCcaEdRequest
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeCcaEdRequest
(
  uint8_t ccaParam
);

/*---------------------------------------------------------------------------
 * Name: PhyPlmeSetCurrentChannelRequestPAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeSetCurrentChannelRequestPAN0
(
  uint8_t channel
);

/*---------------------------------------------------------------------------
 * Name: PhyPlmeSetCurrentChannelRequestPAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeSetCurrentChannelRequestPAN1
(
  uint8_t channel
);

/*---------------------------------------------------------------------------
 * Name: PhyPlmeGetCurrentChannelRequestPAN0
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeGetCurrentChannelRequestPAN0
( 
  void
);

/*---------------------------------------------------------------------------
 * Name: PhyPlmeGetCurrentChannelRequestPAN1
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeGetCurrentChannelRequestPAN1
( 
  void
);

/*---------------------------------------------------------------------------
 * Name: PhyPlmeSetPwrLevelRequest
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeSetPwrLevelRequest
(
  uint8_t pwrStep
);

/*---------------------------------------------------------------------------
 * Name: PhyPpSetCcaThreshold
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPpSetCcaThreshold
(
  uint8_t ccaThreshold
);

/*---------------------------------------------------------------------------
 * Name: PhyAdjustCcaOffsetCmpReg
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyAdjustCcaOffsetCmpReg
(
  uint8_t ccaOffset
);

/*---------------------------------------------------------------------------
 * Name: PhyPlmeSetClockOutRateReg
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhyPlmeSetClockOutRateReg
(
  uint8_t clockOutRate
);

/*---------------------------------------------------------------------------
 * Name: PhySetRadioTimerPrescalerReg
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PhySetRadioTimerPrescalerReg
(
  uint8_t timeBase
);




// PHY Time

/*---------------------------------------------------------------------------
 * Name: PhyTimeSetEventTrigger
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyTimeSetEventTrigger
(
  uint16_t startTime
);

/*---------------------------------------------------------------------------
 * Name: PhyTimeSetEventTimeout
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyTimeSetEventTimeout
(
  zbClock24_t *pEndTime
);

/*---------------------------------------------------------------------------
 * Name: PhyTimeReadClock
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyTimeReadClock
(
  zbClock24_t *pRetClk
);

/*---------------------------------------------------------------------------
 * Name: PhyTimeDisableEventTimeout
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyTimeDisableEventTimeout
(
  void
);

// PHY ISR

/*---------------------------------------------------------------------------
 * Name: PhyPassRxParams()
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPassRxParams
(
  phyRxParams_t * pRxParam
);

/*---------------------------------------------------------------------------
 * Name: PhyPassRxDataPtr()
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyPassRxDataPtr
(
  phyPacket_t * pRxData
);

/*---------------------------------------------------------------------------
 * Name: PhyIsrTimeoutCleanup
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyIsrTimeoutCleanup
(
  void
);

/*---------------------------------------------------------------------------
 * Name: PhyIsrSeqCleanup
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PhyIsrSeqCleanup
(
  void
);

// PHY 2 MAC
/*****************************************************************************
* PhyPlmeSyncLossIndication function
*
* Interface assumptions:
*
* Return Value:
* None
*****************************************************************************/
void PhyPlmeSyncLossIndication
(
  void
);


/*****************************************************************************
* PhyTimeRxTimeoutIndication function
*
* Interface assumptions:
*
* Return Value:
* None
*****************************************************************************/
void PhyTimeRxTimeoutIndication
(
  void
);

/*****************************************************************************
* PhyTimeStartEventIndication function
*
* Interface assumptions:
*
* Return Value:
* None
*****************************************************************************/
void PhyTimeStartEventIndication
(
  void
);

/*****************************************************************************
* PhyPlmeCcaConfirm function
*
* Interface assumptions:
*
* Return Value:
* None
*****************************************************************************/
void PhyPlmeCcaConfirm
(
  bool_t channelInUse
);

/*****************************************************************************
* PhyPlmeEdConfirm function
*
* Interface assumptions:
*
* Return Value:
* None
*****************************************************************************/
void PhyPlmeEdConfirm
(
  uint8_t energyLevel
);

/*****************************************************************************
* PhyPdDataConfirm function
*
* Interface assumptions:
*
* Return Value:
* None
*****************************************************************************/
void PhyPdDataConfirm
(
  void
);

/*****************************************************************************
* PhyPdDataIndication function
*
* Interface assumptions:
*
* Return Value:
* None
*****************************************************************************/
void PhyPdDataIndication
(
void
);

/*****************************************************************************
* PhyUnexpectedTransceiverReset function
*
* Interface assumptions:
*
* Return Value:
* None
*****************************************************************************/
void PhyUnexpectedTransceiverReset
(
  void
);

/*****************************************************************************
* PhyPlmeFilterFailRx function
*
* Interface assumptions:
*
* Return Value:
* None
*****************************************************************************/
void PhyPlmeFilterFailRx
(
  void
);

/*****************************************************************************
* PhyPlmeRxSfdDetect function
*
* Interface assumptions:
*
* Return Value:
* None
*****************************************************************************/
void PhyPlmeRxSfdDetect
(
  void
);

#endif /* __PHY_H__ */