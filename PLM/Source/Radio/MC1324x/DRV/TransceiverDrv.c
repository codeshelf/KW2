/*****************************************************************************
 *                               INCLUDED HEADERS                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the headers that this module needs to include.    *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
#include "EmbeddedTypes.h"
#include "TransceiverDrv.h"
#include "TransceiverReg.h"
#include "Interrupt.h"
//#include "NVIC.h"

/*****************************************************************************
 *                               PRIVATE VARIABLES                           *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have local       *
 * (file) scope.                                                             *
 * Each of this declarations shall be preceded by the 'static' keyword.      *
 * These variables / constants cannot be accessed outside this module.       *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

//uint8_t mSpiDMABuff[512];
  uint32_t mSpiDMA_PUSH_Reg;
  uint32_t mLastSpiDMA_PUSH_Reg;


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
 * Name: MC1324xDrv_SPIInit
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_SPIInit
(
  void
)
{
  /* Enable SPI PORT clock */ 
  MC1324x_SPI_PORT_SIM_SCG |= cSPI_PORT_SIM_SCG_Config_c;
  
  /* Enable SPI clock */ 
  MC1324x_SPI_SIM_SCG |= cSPI_SIM_SCG_Config_c;
  
  /* Initialize SPI Pin Control registers */
  
  MC1324x_SPI_SSEL_PCR &= ~PORT_PCR_MUX_MASK;
  MC1324x_SPI_SSEL_PCR |= PORT_PCR_MUX(2);
  MC1324x_SPI_SCLK_PCR &= ~PORT_PCR_MUX_MASK;
  MC1324x_SPI_SCLK_PCR |= PORT_PCR_MUX(2);
  MC1324x_SPI_MOSI_PCR &= ~PORT_PCR_MUX_MASK;
  MC1324x_SPI_MOSI_PCR |= PORT_PCR_MUX(2);
  MC1324x_SPI_MISO_PCR &= ~PORT_PCR_MUX_MASK;
  MC1324x_SPI_MISO_PCR |= PORT_PCR_MUX(2);
  
  /* Initialize SPI module */
  MC1324x_SPI_MCR = cSPI_MCR_Config_c;
  MC1324x_SPI_CTAR0 = cSPI_CTAR0_Config_c;
  MC1324x_SPI_CTAR1 = cSPI_CTAR1_Config_c;
  MC1324x_SPI_RSER = cSPI_RSER_Config_c;
  
}

#if 0

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_SPI_DMA_Init
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_SPI_DMA_Init
(
  void
)
{
  /* DMAMUX clock gate enabled */
  MC1324x_DMA_MUX_SIM_SCG |= (uint32_t)(cDMA_MUX_SIM_SCG_Config_c);
  /* DMA clock gate enabled */
  MC1324x_eDMA_SIM_SCG |= (uint32_t)(ceDMA_SIM_SCG_Config_c);
  
  /* MEM2MEM DMAMUX config */
  MC1324x_DMAMUX_MEM2MEM = 0x00;
  MC1324x_DMAMUX_MEM2MEM = (0
  | DMAMUX_CHCFG_SOURCE(54)
  | DMAMUX_CHCFG_ENBL_MASK);
  
  /* MEM2SPI DMAMUX config */
  MC1324x_DMAMUX_MEM2SPI = 0x00;
  MC1324x_DMAMUX_MEM2SPI = (0
  | DMAMUX_CHCFG_SOURCE(19)
  | DMAMUX_CHCFG_ENBL_MASK );
  
    /* SPI2MEM DMAMUX config */
  MC1324x_DMAMUX_SPI2MEM = 0x00;
  MC1324x_DMAMUX_SPI2MEM = (0
  | DMAMUX_CHCFG_SOURCE(18)
  | DMAMUX_CHCFG_ENBL_MASK );
  
  /* DMA MEM2MEM default config */
  MC1324x_DMA_MEM2MEM_TCD_SOFF = 1;
  MC1324x_DMA_MEM2MEM_TCD_ATTR = (0
  | DMA_ATTR_DSIZE(0)
  | DMA_ATTR_DMOD(0)
  | DMA_ATTR_SSIZE(0)
  | DMA_ATTR_SMOD(0) );
  MC1324x_DMA_MEM2MEM_TCD_NBYTES_MLNO = DMA_NBYTES_MLNO_NBYTES(1);
  MC1324x_DMA_MEM2MEM_TCD_SLAST = DMA_SLAST_SLAST(0);
  MC1324x_DMA_MEM2MEM_TCD_DOFF = 0;
  MC1324x_DMA_MEM2MEM_TCD_CITER_ELINKNO = DMA_CITER_ELINKNO_CITER(1);
  MC1324x_DMA_MEM2MEM_TCD_DLASTSGA = DMA_DLAST_SGA_DLASTSGA(0);
  MC1324x_DMA_MEM2MEM_TCD_CSR = (0
  | DMA_CSR_DREQ_MASK
  | DMA_CSR_BWC(0) );
  MC1324x_DMA_MEM2MEM_TCD_BITER_ELINKNO = DMA_BITER_ELINKNO_BITER(1);
  
  /* DMA MEM2SPI default config */
  MC1324x_DMA_MEM2SPI_TCD_SADDR = (uint32_t) &mSpiDMA_PUSH_Reg;
  MC1324x_DMA_MEM2SPI_TCD_SOFF = 0;
  MC1324x_DMA_MEM2SPI_TCD_ATTR = (0
  | DMA_ATTR_DSIZE(2)
  | DMA_ATTR_DMOD(0)
  | DMA_ATTR_SSIZE(2)
  | DMA_ATTR_SMOD(0) );
  MC1324x_DMA_MEM2SPI_TCD_SLAST = DMA_SLAST_SLAST(0);
  MC1324x_DMA_MEM2SPI_TCD_NBYTES_MLNO = DMA_NBYTES_MLNO_NBYTES(4);
  MC1324x_DMA_MEM2SPI_TCD_DADDR = (uint32_t) &(MC1324x_SPI_PUSHR);
  MC1324x_DMA_MEM2SPI_TCD_DOFF = 0;
  MC1324x_DMA_MEM2SPI_TCD_DLASTSGA = DMA_DLAST_SGA_DLASTSGA(0);
  MC1324x_DMA_MEM2SPI_TCD_CSR = (0
  | DMA_CSR_INTMAJOR_MASK
  | DMA_CSR_DREQ_MASK
  | DMA_CSR_BWC(0) );
  
  NVIC_EnableIRQ(14);
  NVIC_SetPriority(14, 2);
  
  /* DMA SPI2MEM default config */
  MC1324x_DMA_SPI2MEM_TCD_SADDR = (uint32_t) &(MC1324x_SPI_POPR);
  MC1324x_DMA_SPI2MEM_TCD_SOFF = 0;
  MC1324x_DMA_SPI2MEM_TCD_ATTR = (0
  | DMA_ATTR_DSIZE(0)
  | DMA_ATTR_DMOD(0)
  | DMA_ATTR_SSIZE(0)
  | DMA_ATTR_SMOD(0) );
  MC1324x_DMA_SPI2MEM_TCD_SLAST = DMA_SLAST_SLAST(0);
  MC1324x_DMA_SPI2MEM_TCD_NBYTES_MLNO = DMA_NBYTES_MLNO_NBYTES(1);
  MC1324x_DMA_SPI2MEM_TCD_DOFF = 1;
  MC1324x_DMA_SPI2MEM_TCD_DLASTSGA = DMA_DLAST_SGA_DLASTSGA(0);
  MC1324x_DMA_SPI2MEM_TCD_CSR = (0
  | DMA_CSR_INTMAJOR_MASK
  | DMA_CSR_DREQ_MASK
  | DMA_CSR_BWC(0) );
  
  NVIC_EnableIRQ(14);
  NVIC_SetPriority(14, 2);
  
  //NVIC_EnableIRQ(13);
  //NVIC_SetPriority(14, 1);
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_Start_PB_DMA_SPI_Write
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_Start_PB_DMA_SPI_Write
(
  uint8_t * srcAddress,
  uint8_t numOfBytes
)
{
  MC1324x_SPI_PUSHR = (uint32_t) ( 0
  | TransceiverSPI_PacketBuffAccessSelect
  | TransceiverSPI_PacketBuffBurstModeSelect
  | cSPI_PUSHR_CONT_enable_c
  | cSPI_PUSHR_PCS_select_c
  | cSPI_PUSHR_CTAS0_select_c);
  
  MC1324x_SPI_PUSHR = (uint32_t) (uint32_t) ( 0
  | numOfBytes
  | cSPI_PUSHR_CONT_enable_c
  | cSPI_PUSHR_PCS_select_c
  | cSPI_PUSHR_CTAS0_select_c);
  
  mSpiDMA_PUSH_Reg = (uint32_t) ( 0
  | srcAddress[0]
  | cSPI_PUSHR_CONT_enable_c
  | cSPI_PUSHR_PCS_select_c
  | cSPI_PUSHR_CTAS0_select_c);
  
  mLastSpiDMA_PUSH_Reg = (uint32_t) ( 0
  | srcAddress[(numOfBytes - 1)]
  | cSPI_PUSHR_EOQ_assert_c
  | cSPI_PUSHR_PCS_select_c
  | cSPI_PUSHR_CTAS0_select_c);

  MC1324x_DMA_MEM2MEM_TCD_SADDR = (uint32_t) (srcAddress + 1);
  MC1324x_DMA_MEM2MEM_TCD_DADDR = (uint32_t) &mSpiDMA_PUSH_Reg;
  
  MC1324x_DMA_MEM2SPI_TCD_CITER_ELINKYES = (0 
  | DMA_CITER_ELINKYES_CITER((numOfBytes - 1))                              
  | DMA_CITER_ELINKYES_LINKCH(15)
  | DMA_CITER_ELINKYES_ELINK_MASK
  );
  MC1324x_DMA_MEM2SPI_TCD_BITER_ELINKYES = (0 
  | DMA_BITER_ELINKYES_BITER((numOfBytes - 1))                              
  | DMA_BITER_ELINKYES_LINKCH(15)
  | DMA_BITER_ELINKYES_ELINK_MASK
  );
  
  DMA_ERQ |= (uint32_t) (1 << 14);
  
  MC1324x_SPI_RSER |= 0x03000000;
  
  StartSPITransfer();
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_Start_PB_DMA_SPI_Read
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_Start_PB_DMA_SPI_Read
(
  uint8_t * dstAddress,
  uint8_t numOfBytes
)
{
  MC1324x_DMA_MEM2MEM_TCD_SOFF = 0;
  MC1324x_DMA_MEM2MEM_TCD_DADDR = (uint32_t) &(MC1324x_SPI_PUSHR);
  MC1324x_DMA_MEM2MEM_TCD_SADDR = (uint32_t) &mSpiDMA_PUSH_Reg;
  
  MC1324x_DMA_SPI2MEM_TCD_DADDR = (uint32_t) (dstAddress);
  
  MC1324x_DMA_SPI2MEM_TCD_CITER_ELINKYES = (0 
  | DMA_CITER_ELINKYES_CITER((numOfBytes))                              
  | DMA_CITER_ELINKYES_LINKCH(15)
  | DMA_CITER_ELINKYES_ELINK_MASK
  );
  MC1324x_DMA_SPI2MEM_TCD_BITER_ELINKYES = (0 
  | DMA_BITER_ELINKYES_BITER((numOfBytes))                              
  | DMA_BITER_ELINKYES_LINKCH(15)
  | DMA_BITER_ELINKYES_ELINK_MASK
  );
  
  MC1324x_SPI_PUSHR = (uint32_t) ( 0
  | TransceiverSPI_PacketBuffAccessSelect
  | TransceiverSPI_PacketBuffBurstModeSelect
  | cSPI_PUSHR_CONT_enable_c
  | cSPI_PUSHR_PCS_select_c
  | cSPI_PUSHR_CTAS0_select_c);
  
  mSpiDMA_PUSH_Reg = (uint32_t) ( 0
  | cSPI_PUSHR_CONT_enable_c
  | cSPI_PUSHR_PCS_select_c
  | cSPI_PUSHR_CTAS0_select_c);

  DMA_ERQ |= (uint32_t) (1 << 13);
  
  StartSPITransfer();
  
  WaitSPI_TransferCompleteFlag();
  ClearSPI_TransferCompleteFlag();  
  
  SPI_DummyRead();

  MC1324x_SPI_RSER |= 0x00030000;
}

/*---------------------------------------------------------------------------
 * Name: DMA_Channel_15_Isr
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void DMA_Channel_15_Isr
(
  void
)
{
  DMA_INT |= (uint16_t) (1 << 15);
  DMA_ERQ |= (uint32_t) (1 << 14);
  
  StartSPITransfer();
  
  MC1324x_SPI_RSER |= 0x03000000;
}

/*---------------------------------------------------------------------------
 * Name: DMA_Channel_14_Isr
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void DMA_Channel_14_Isr
(
  void
)
{
  DMA_INT |= (uint16_t) (1 << 14);
  WaitSPI_TxFIFO_NotFullFlag();
  MC1324x_SPI_PUSHR = mLastSpiDMA_PUSH_Reg;
  
}
#endif

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_DirectAccessSPIWrite
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_DirectAccessSPIWrite
(
 uint8_t address,
 uint8_t value
)
{
  uint32_t SPIPushRegister;
  uint32_t irqMaskRegister;
  
  irqMaskRegister = IntDisableAll();
    
  FlushSPIFifo();
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);

  SPIPushRegister = (uint32_t) (address & TransceiverSPI_DirectRegisterAddressMask);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  SPIPushRegister = (uint32_t) (value);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_EOQ_assert_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);

  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  StartSPITransfer();
  
  WaitSPI_EndOfQueueFlag();
  
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  StopSPITransferAndFlushFifo();
  
  IntRestoreAll(irqMaskRegister);
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_DirectAccessSPIMultiByteWrite
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_DirectAccessSPIMultiByteWrite
(
 uint8_t startAddress,
 uint8_t * byteArray,
 uint8_t numOfBytes
)
{
  uint32_t SPIPushRegister;
  uint8_t fifoIdx = 0;
  uint32_t irqMaskRegister;

  if( (numOfBytes == 0) || (byteArray == NULL) )
  {
    return;
  }

  irqMaskRegister = IntDisableAll();
    
  FlushSPIFifo();
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  SPIPushRegister = (uint32_t) (startAddress & TransceiverSPI_DirectRegisterAddressMask);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  fifoIdx++;
  
  while( (numOfBytes > 1)  && (fifoIdx < 4) )
  {
    SPIPushRegister = (uint32_t) *(byteArray++);
    SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                   cSPI_PUSHR_PCS_select_c |
                                   cSPI_PUSHR_CTAS0_select_c);
    MC1324x_SPI_PUSHR = SPIPushRegister;
    numOfBytes--;
    fifoIdx++;
  }
  
  StartSPITransfer();
  
  while((numOfBytes > 1))
  {
    SPIPushRegister = (uint32_t) *(byteArray++);
    SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                   cSPI_PUSHR_PCS_select_c |
                                   cSPI_PUSHR_CTAS0_select_c);
    WaitSPI_TransferCompleteFlag();
    ClearSPI_TransferCompleteFlag();
    
    MC1324x_SPI_PUSHR = SPIPushRegister;
    numOfBytes--;
  }
  
  SPIPushRegister = (uint32_t) *(byteArray);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_EOQ_assert_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);
  WaitSPI_TransferCompleteFlag();
  ClearSPI_TransferCompleteFlag();
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  WaitSPI_EndOfQueueFlag();
  
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  StopSPITransferAndFlushFifo();

  IntRestoreAll(irqMaskRegister);
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_PB_SPIBurstWrite
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_PB_SPIBurstWrite
(
 uint8_t * byteArray,
 uint8_t numOfBytes
)
{
  uint32_t SPIPushRegister;
  uint8_t fifoIdx = 0;
  uint32_t irqMaskRegister;

  if( (numOfBytes == 0) || (byteArray == NULL) )
  {
    return;
  }

  irqMaskRegister = IntDisableAll();
    
  FlushSPIFifo();
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  SPIPushRegister = (uint32_t) ( TransceiverSPI_WriteSelect
                                |TransceiverSPI_PacketBuffAccessSelect
                                |TransceiverSPI_PacketBuffBurstModeSelect);
  
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  fifoIdx++;
  
  while( (numOfBytes > 1)  && (fifoIdx < 4) )
  {
    SPIPushRegister = (uint32_t) *(byteArray++);
    SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                   cSPI_PUSHR_PCS_select_c |
                                   cSPI_PUSHR_CTAS0_select_c);
    MC1324x_SPI_PUSHR = SPIPushRegister;
    numOfBytes--;
    fifoIdx++;
  }
  
  StartSPITransfer();
  
  while((numOfBytes > 1))
  {
    SPIPushRegister = (uint32_t) *(byteArray++);
    SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                   cSPI_PUSHR_PCS_select_c |
                                   cSPI_PUSHR_CTAS0_select_c);
    WaitSPI_TransferCompleteFlag();
    ClearSPI_TransferCompleteFlag();
    
    MC1324x_SPI_PUSHR = SPIPushRegister;
    numOfBytes--;
  }
  
  SPIPushRegister = (uint32_t) *(byteArray);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_EOQ_assert_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);
  WaitSPI_TransferCompleteFlag();
  ClearSPI_TransferCompleteFlag();
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  WaitSPI_EndOfQueueFlag();
  
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c);

  StopSPITransferAndFlushFifo();

  IntRestoreAll(irqMaskRegister);
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_DirectAccessSPIRead
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/

uint8_t MC1324xDrv_DirectAccessSPIRead
(
 uint8_t address
)
{
  uint32_t SPIPushRegister, SPIPopRegister;
  uint32_t irqMaskRegister;
  
  irqMaskRegister = IntDisableAll();
    
  FlushSPIFifo();
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);

  SPIPushRegister = (uint32_t) ((address & TransceiverSPI_DirectRegisterAddressMask) |
                                TransceiverSPI_ReadSelect);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS1_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  SPIPushRegister = (uint32_t) (cSPI_PUSHR_EOQ_assert_c |
                                cSPI_PUSHR_PCS_select_c |
                                cSPI_PUSHR_CTAS1_select_c);

  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  StartSPITransfer();
  
  WaitSPI_EndOfQueueFlag();
  
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  StopSPITransfer();
  
  SPIPopRegister = MC1324x_SPI_RXFR1;

  FlushSPIFifo();

  IntRestoreAll(irqMaskRegister);
  
  return (uint8_t)SPIPopRegister;
  
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_DirectAccessSPIMultyByteRead
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_DirectAccessSPIMultiByteRead
(
 uint8_t startAddress,
 uint8_t * byteArray,
 uint8_t numOfBytes
)
{
  uint32_t SPIPushRegister;
  uint32_t irqMaskRegister;
  
  if( (numOfBytes == 0) || (byteArray == NULL) )
  {
    return;
  }
  
  //asm(" MRS %0,PRIMASK" : "=r" (irqMaskRegister) );
  //asm(" CPSID i");
  irqMaskRegister = IntDisableAll();
     
  FlushSPIFifo();
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);

  SPIPushRegister = (uint32_t) ((startAddress & TransceiverSPI_DirectRegisterAddressMask) |
                                TransceiverSPI_ReadSelect);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS1_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  StartSPITransfer();
  
  WaitSPI_TransferCompleteFlag();
  ClearSPI_TransferCompleteFlag();
  
  //Flush RX FIFO
  MC1324x_SPI_MCR |= (uint32_t)((SPI_MCR_CLR_RXF_MASK));

  while( (numOfBytes > 1) )
  {
    SPIPushRegister = (uint32_t) 0;
    SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                   cSPI_PUSHR_PCS_select_c |
                                   cSPI_PUSHR_CTAS1_select_c);
    
    MC1324x_SPI_PUSHR = SPIPushRegister;
    
    WaitSPI_TransferCompleteFlag();
    ClearSPI_TransferCompleteFlag();
    
    *byteArray++ = MC1324x_SPI_POPR;
    
    numOfBytes--;
  }
  
  SPIPushRegister = (uint32_t) 0;
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_EOQ_assert_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS1_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  WaitSPI_EndOfQueueFlag();
  
  *byteArray++ = MC1324x_SPI_POPR;
  
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  StopSPITransfer();
  
  FlushSPIFifo();

  //asm(" MSR PRIMASK, %0" : : "r" (irqMaskRegister) );
  IntRestoreAll(irqMaskRegister);
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_PB_SPIBurstRead
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_PB_SPIBurstRead
(
 uint8_t * byteArray,
 uint8_t numOfBytes
)
{
  uint32_t SPIPushRegister;
  uint32_t irqMaskRegister;

  if( (numOfBytes == 0) || (byteArray == NULL) )
  {
    return;
  }
  
  //asm(" MRS %0,PRIMASK" : "=r" (irqMaskRegister) );
  //asm(" CPSID i");
  irqMaskRegister = IntDisableAll();
     
  FlushSPIFifo();
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  SPIPushRegister = (uint32_t) ( TransceiverSPI_ReadSelect
                                |TransceiverSPI_PacketBuffAccessSelect
                                |TransceiverSPI_PacketBuffBurstModeSelect);
  
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS1_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  StartSPITransfer();
  
  WaitSPI_TransferCompleteFlag();
  ClearSPI_TransferCompleteFlag();
  
  //Flush RX FIFO
  MC1324x_SPI_MCR |= (uint32_t)((SPI_MCR_CLR_RXF_MASK));
  
  while( (numOfBytes > 1) )
  {
    SPIPushRegister = (uint32_t) 0;
    SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                   cSPI_PUSHR_PCS_select_c |
                                   cSPI_PUSHR_CTAS1_select_c);
    
    MC1324x_SPI_PUSHR = SPIPushRegister;
    
    WaitSPI_TransferCompleteFlag();
    ClearSPI_TransferCompleteFlag();
    
    *byteArray++ = MC1324x_SPI_POPR;
    
    numOfBytes--;
  }
  
  SPIPushRegister = (uint32_t) 0;
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_EOQ_assert_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS1_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  WaitSPI_EndOfQueueFlag();
  
  *byteArray++ = MC1324x_SPI_POPR;
  
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  StopSPITransfer();
  
  FlushSPIFifo();
  
  //asm(" MSR PRIMASK, %0" : : "r" (irqMaskRegister) );
  IntRestoreAll(irqMaskRegister);
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IndirectAccessSPIWrite
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_IndirectAccessSPIWrite
(
 uint8_t address,
 uint8_t value
)
{
  uint32_t SPIPushRegister = (uint8_t) TransceiverSPI_IARIndexReg;
  uint32_t irqMaskRegister;
  
  //asm(" MRS %0,PRIMASK" : "=r" (irqMaskRegister) );
  //asm(" CPSID i");
  irqMaskRegister = IntDisableAll();
    
  FlushSPIFifo();
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  SPIPushRegister = (uint32_t) (address);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;

  SPIPushRegister = (uint32_t) (value);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_EOQ_assert_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  StartSPITransfer();
  
  WaitSPI_EndOfQueueFlag();
  
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  StopSPITransferAndFlushFifo();
  
  //asm(" MSR PRIMASK, %0" : : "r" (irqMaskRegister) );
  IntRestoreAll(irqMaskRegister);
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IndirectAccessSPIMultiByteWrite
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_IndirectAccessSPIMultiByteWrite
(
 uint8_t startAddress,
 uint8_t * byteArray,
 uint8_t numOfBytes
)
{
  uint32_t SPIPushRegister = (uint8_t) TransceiverSPI_IARIndexReg;
  uint8_t fifoIdx = 0;
  uint32_t irqMaskRegister;
  
  if( (numOfBytes == 0) || (byteArray == NULL) )
  {
    return;
  }
  
  //asm(" MRS %0,PRIMASK" : "=r" (irqMaskRegister) );
  //asm(" CPSID i");
  irqMaskRegister = IntDisableAll();
    
  FlushSPIFifo();
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);

  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  fifoIdx++;
  
  SPIPushRegister = (uint32_t) (startAddress);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  fifoIdx++;
  
  while( (numOfBytes > 1)  && (fifoIdx < 4) )
  {
    SPIPushRegister = (uint32_t) *(byteArray++);
    SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                   cSPI_PUSHR_PCS_select_c |
                                   cSPI_PUSHR_CTAS0_select_c);
    MC1324x_SPI_PUSHR = SPIPushRegister;
    fifoIdx++;
    numOfBytes--;
  }
  
  StartSPITransfer();
  
  while((numOfBytes > 1))
  {
    SPIPushRegister = (uint32_t) *(byteArray++);
    SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                   cSPI_PUSHR_PCS_select_c |
                                   cSPI_PUSHR_CTAS0_select_c);
    WaitSPI_TransferCompleteFlag();
    ClearSPI_TransferCompleteFlag();
    
    MC1324x_SPI_PUSHR = SPIPushRegister;
    numOfBytes--;
  }
  
  SPIPushRegister = (uint32_t) *(byteArray);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_EOQ_assert_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS0_select_c);
  WaitSPI_TransferCompleteFlag();
  ClearSPI_TransferCompleteFlag();
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  WaitSPI_EndOfQueueFlag();
  
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  StopSPITransferAndFlushFifo();
  
  //asm(" MSR PRIMASK, %0" : : "r" (irqMaskRegister) );
  IntRestoreAll(irqMaskRegister);
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IndirectAccessSPIRead
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t MC1324xDrv_IndirectAccessSPIRead
(
 uint8_t address
)
{
  uint32_t SPIPushRegister = (uint8_t) (TransceiverSPI_IARIndexReg | TransceiverSPI_ReadSelect), SPIPopRegister;
  uint32_t irqMaskRegister;
  
  //asm(" MRS %0,PRIMASK" : "=r" (irqMaskRegister) );
  //asm(" CPSID i");
  irqMaskRegister = IntDisableAll();
    
  FlushSPIFifo();
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS1_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  SPIPushRegister = (uint32_t) (address);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS1_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;

  SPIPushRegister = (uint32_t) (cSPI_PUSHR_EOQ_assert_c |
                                cSPI_PUSHR_PCS_select_c |
                                cSPI_PUSHR_CTAS1_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  StartSPITransfer();
  
  WaitSPI_EndOfQueueFlag();
 
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  StopSPITransfer();
  
  SPIPopRegister = MC1324x_SPI_RXFR2;

  FlushSPIFifo();
  
  //asm(" MSR PRIMASK, %0" : : "r" (irqMaskRegister) );
  IntRestoreAll(irqMaskRegister);

  return (uint8_t)SPIPopRegister;
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IndirectAccessSPIMultiByteRead
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_IndirectAccessSPIMultiByteRead
(
 uint8_t startAddress,
 uint8_t * byteArray,
 uint8_t numOfBytes
)
{
  uint32_t SPIPushRegister = (uint8_t) (TransceiverSPI_IARIndexReg | TransceiverSPI_ReadSelect);
  uint32_t irqMaskRegister;
  
  if( (numOfBytes == 0) || (byteArray == NULL) )
  {
    return;
  }
  
  //asm(" MRS %0,PRIMASK" : "=r" (irqMaskRegister) );
  //asm(" CPSID i");
  irqMaskRegister = IntDisableAll();
    
  FlushSPIFifo();
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS1_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  SPIPushRegister = (uint32_t) (startAddress);
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS1_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  

  StartSPITransfer();
  
  WaitSPI_TransferCompleteFlag();
  ClearSPI_TransferCompleteFlag();
  
  WaitSPI_TransferCompleteFlag();
  ClearSPI_TransferCompleteFlag();
  
  //Flush RX FIFO
  MC1324x_SPI_MCR |= (uint32_t)((SPI_MCR_CLR_RXF_MASK));
  
  while( (numOfBytes > 1) )
  {
    SPIPushRegister = (uint32_t) 0;
    SPIPushRegister |= (uint32_t) (cSPI_PUSHR_CONT_enable_c |
                                   cSPI_PUSHR_PCS_select_c |
                                   cSPI_PUSHR_CTAS1_select_c);
    
    MC1324x_SPI_PUSHR = SPIPushRegister;
    
    WaitSPI_TransferCompleteFlag();
    ClearSPI_TransferCompleteFlag();
    
    *byteArray++ = MC1324x_SPI_POPR;
    
    numOfBytes--;
  }
  
  SPIPushRegister = (uint32_t) 0;
  SPIPushRegister |= (uint32_t) (cSPI_PUSHR_EOQ_assert_c |
                                 cSPI_PUSHR_PCS_select_c |
                                 cSPI_PUSHR_CTAS1_select_c);
  
  MC1324x_SPI_PUSHR = SPIPushRegister;
  
  WaitSPI_EndOfQueueFlag();
  
  *byteArray++ = MC1324x_SPI_POPR;
  
  MC1324x_SPI_SR |= (uint32_t)(cSPI_SR_EOQF_MASK_c | cSPI_SR_TCF_MASK_c | cSPI_SR_RFDF_MASK_c);
  
  StopSPITransfer();
  
  FlushSPIFifo();

  //asm(" MSR PRIMASK, %0" : : "r" (irqMaskRegister) );
  IntRestoreAll(irqMaskRegister);
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IRQ_PortConfig
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_IRQ_PortConfig
(
  void
)
{
  MC1324x_IRQ_PORT_SIM_SCG |= cIRQ_PORT_SIM_SCG_Config_c;
  MC1324x_IRQ_PCR |= cIRQ_PCR_Config_c;
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IsIrqPending
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
bool_t  MC1324xDrv_IsIrqPending
(
  void
)
{
  bool_t retVal = TRUE;
  if((MC1324x_IRQ_PDIR & (uint32_t )Radio_IRQ_PinNumberMask_c) == (uint32_t)Radio_IRQ_PinNumberMask_c)
  {
    retVal = FALSE;
  }
  return retVal;
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IRQ_Disable
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_IRQ_Disable
(
  void
)
{
  MC1324x_IRQ_PCR &= ~PORT_PCR_IRQC_MASK;
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IRQ_Enable
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_IRQ_Enable
(
  void
)
{
  MC1324x_IRQ_PCR |= PORT_PCR_ISF_MASK;
  MC1324x_IRQ_PCR |= PORT_PCR_IRQC(8);
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IRQ_Clear
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_IRQ_Clear
(
  void
)
{
  MC1324x_IRQ_PCR |= PORT_PCR_ISF_MASK;
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_RST_PortConfig
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_RST_B_PortConfig
(
  void
)
{
  MC1324x_RST_PORT_SIM_SCG |= cRST_PORT_SIM_SCG_Config_c;
  MC1324x_RST_PCR |= cRST_PCR_Config_c;
  MC1324x_RST_PSOR |= (uint32_t)Radio_RST_PinNumberMask_c;
  MC1324x_RST_PDDR |= (uint32_t)Radio_RST_PinNumberMask_c;
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_RST_Assert
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_RST_B_Assert
(
  void
)
{
  MC1324x_RST_PCOR |= (uint32_t)Radio_RST_PinNumberMask_c;
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_RST_Deassert
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_RST_B_Deassert
(
  void
)
{
  MC1324x_RST_PSOR |= (uint32_t)Radio_RST_PinNumberMask_c;
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_SoftRST_Assert
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_SoftRST_Assert
(
  void
)
{
  MC1324xDrv_IndirectAccessSPIWrite(SOFT_RESET, (0x80));
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_SoftRST_Deassert
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_SoftRST_Deassert
(
  void
)
{
  MC1324xDrv_IndirectAccessSPIWrite(SOFT_RESET, (0x00));
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_RESET
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_RESET
(
  void
)
{
  volatile uint32_t delay = 1000;
  MC1324x_RST_PCOR |= Radio_RST_PinNumberMask_c;
// TO DO
  while(delay--);
  MC1324x_RST_PSOR |= Radio_RST_PinNumberMask_c;
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_Soft_RESET
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_Soft_RESET
(
  void
)
{
  //assert SOG_RST
  MC1324xDrv_IndirectAccessSPIWrite(SOFT_RESET, (0x80)); 
  //deassert SOG_RST
  MC1324xDrv_IndirectAccessSPIWrite(SOFT_RESET, (0x00));
}

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_Set_CLK_OUT_Freq
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_Set_CLK_OUT_Freq
(
  uint8_t freqDiv
)
{
  uint8_t clkOutCtrlReg;
  clkOutCtrlReg = (uint8_t) ( (freqDiv & cCLK_OUT_DIV_Mask) | \
                               cCLK_OUT_EN | \
                               cCLK_OUT_EXTEND );
  if(freqDiv == gCLK_OUT_FREQ_DISABLE)
  {
    clkOutCtrlReg = (uint8_t) (cCLK_OUT_EXTEND | gCLK_OUT_FREQ_4_MHz); //reset value with clock out disabled
  }

  MC1324xDrv_DirectAccessSPIWrite((uint8_t) CLK_OUT_CTRL, clkOutCtrlReg);
}
