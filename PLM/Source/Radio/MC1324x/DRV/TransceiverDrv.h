
#ifndef __TRANSCEIVER_DRV_H__
#define __TRANSCEIVER_DRV_H__

/*****************************************************************************
 *                               INCLUDED HEADERS                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the headers that this module needs to include.    *
 * Note that it is not a good practice to include header files into header   *
 * files, so use this section only if there is no other better solution.     *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

#include "PortConfig.h"

/*****************************************************************************
 *                             PRIVATE MACROS                                *
 *---------------------------------------------------------------------------*
 * Add to this section all the access macros, registers mappings, bit access *
 * macros, masks, flags etc ...
 *---------------------------------------------------------------------------*
 *****************************************************************************/

#define MC1324x_Irq_Number      Radio_Irq_Number

/****************************************************************************/
/* Transceiver SPI interface */
/****************************************************************************/

/* SPI Pin Control Registers */
#define MC1324x_SPI_SSEL_PCR       Radio_SPI_SSEL_PCR     
#define MC1324x_SPI_SCLK_PCR       Radio_SPI_SCLK_PCR     
#define MC1324x_SPI_MOSI_PCR       Radio_SPI_MOSI_PCR      
#define MC1324x_SPI_MISO_PCR       Radio_SPI_MISO_PCR       

#define cSPI_SSEL_PCR_Config_c         (PORT_PCR_MUX(2) | PORT_PCR_DSE_MASK)
#define cSPI_SCLK_PCR_Config_c         (PORT_PCR_MUX(2) | PORT_PCR_DSE_MASK)
#define cSPI_MOSI_PCR_Config_c         (PORT_PCR_MUX(2) | PORT_PCR_DSE_MASK)
#define cSPI_MISO_PCR_Config_c         (PORT_PCR_MUX(2) | PORT_PCR_DSE_MASK)

/* SPI PORT  Clock Gating Control Register */
#define MC1324x_SPI_PORT_SIM_SCG    Radio_SPI_PORT_SIM_SCG
#define cSPI_PORT_SIM_SCG_Config_c  cSPI_PORT_SIM_SCG_Mask_c

/* SPI Module Clock Gating Control Register */
#define MC1324x_SPI_SIM_SCG        Radio_SPI_SIM_SCG    
#define cSPI_SIM_SCG_Config_c      cSPI_SIM_SCG_Mask_c

/* SPI Registers */
#define MC1324x_SPI_MCR            Radio_SPI_MCR          
#define MC1324x_SPI_TCR            Radio_SPI_TCR          
#define MC1324x_SPI_CTAR0          Radio_SPI_CTAR0        
#define MC1324x_SPI_CTAR0_SLAVE    Radio_SPI_CTAR0_SLAVE  
#define MC1324x_SPI_CTAR1          Radio_SPI_CTAR1        
#define MC1324x_SPI_SR             Radio_SPI_SR           
#define MC1324x_SPI_RSER           Radio_SPI_RSER         
#define MC1324x_SPI_PUSHR          Radio_SPI_PUSHR        
#define MC1324x_SPI_PUSHR_SLAVE    Radio_SPI_PUSHR_SLAVE  
#define MC1324x_SPI_POPR           Radio_SPI_POPR         
#define MC1324x_SPI_TXFR0          Radio_SPI_TXFR0        
#define MC1324x_SPI_TXFR1          Radio_SPI_TXFR1        
#define MC1324x_SPI_TXFR2          Radio_SPI_TXFR2        
#define MC1324x_SPI_TXFR3          Radio_SPI_TXFR3        
#define MC1324x_SPI_RXFR0          Radio_SPI_RXFR0        
#define MC1324x_SPI_RXFR1          Radio_SPI_RXFR1        
#define MC1324x_SPI_RXFR2          Radio_SPI_RXFR2        
#define MC1324x_SPI_RXFR3          Radio_SPI_RXFR3        

#define StartSPITransfer()             MC1324x_SPI_MCR &= (uint32_t)(~(SPI_MCR_HALT_MASK))
#define StopSPITransferAndFlushFifo()  MC1324x_SPI_MCR |= (uint32_t)((SPI_MCR_HALT_MASK | SPI_MCR_CLR_RXF_MASK | SPI_MCR_CLR_TXF_MASK))
#define StopSPITransfer()              MC1324x_SPI_MCR |= (uint32_t)((SPI_MCR_HALT_MASK))
#define FlushSPIFifo()                 MC1324x_SPI_MCR |= (uint32_t)((SPI_MCR_CLR_RXF_MASK | SPI_MCR_CLR_TXF_MASK))


/* SPI PUSHR register masks */
#define cSPI_PUSHR_CONT_enable_c        SPI_PUSHR_CONT_MASK
#define cSPI_PUSHR_PCS_select_c         cSPI_PUSHR_PCS_target_c        
#define cSPI_PUSHR_CTAS_read_select_c   cSPI_PUSHR_CTAS_read_target_c  
#define cSPI_PUSHR_CTAS_write_select_c  cSPI_PUSHR_CTAS_write_target_c 
#define cSPI_PUSHR_PCS0_select_c        SPI_PUSHR_PCS(1)  
#define cSPI_PUSHR_PCS1_select_c        SPI_PUSHR_PCS(2) 
#define cSPI_PUSHR_CTAS0_select_c       SPI_PUSHR_CTAS(0) 
#define cSPI_PUSHR_CTAS1_select_c       SPI_PUSHR_CTAS(1) 
#define cSPI_PUSHR_EOQ_assert_c         SPI_PUSHR_EOQ_MASK

/* SPI Status Register masks */

#define cSPI_SR_RFDF_MASK_c             SPI_SR_RFDF_MASK 
#define cSPI_SR_RFOF_MASK_c             SPI_SR_RFOF_MASK
#define cSPI_SR_TFFF_MASK_c             SPI_SR_TFFF_MASK
#define cSPI_SR_TFUF_MASK_c             SPI_SR_TFUF_MASK
#define cSPI_SR_EOQF_MASK_c             SPI_SR_EOQF_MASK
#define cSPI_SR_TXRXS_MASK_c            SPI_SR_TXRXS_MASK
#define cSPI_SR_TCF_MASK_c              SPI_SR_TCF_MASK

#define SPI_DummyRead()                 {char dummy = MC1324x_SPI_POPR;}

#define WaitSPI_TransferCompleteFlag()  while( (MC1324x_SPI_SR & SPI_SR_TCF_MASK) != SPI_SR_TCF_MASK )
#define ClearSPI_TransferCompleteFlag() MC1324x_SPI_SR |= (uint32_t) (SPI_SR_TCF_MASK)

#define SPI_TxFIFO_IsNotFull()         ((MC1324x_SPI_SR & SPI_SR_TFFF_MASK) == SPI_SR_TFFF_MASK )
#define WaitSPI_TxFIFO_NotFullFlag()    while( (MC1324x_SPI_SR & SPI_SR_TFFF_MASK) != SPI_SR_TFFF_MASK )
#define ClearSPI_TxFIFO_NotFullFlag()   MC1324x_SPI_SR |= (uint32_t) (SPI_SR_TFFF_MASK)

#define SPI_RxFIFO_IsNotEmpty()         ( (MC1324x_SPI_SR & SPI_SR_RFDF_MASK) == SPI_SR_RFDF_MASK )
#define WaitSPI_RxFIFO_NotEmptyFlag()   while( (MC1324x_SPI_SR & SPI_SR_RFDF_MASK) != SPI_SR_RFDF_MASK )
#define ClearSPI_RxFIFO_NotEmptyFlag()  MC1324x_SPI_SR |= (uint32_t) (SPI_SR_RFDF_MASK)

#define WaitSPI_EndOfQueueFlag()        while( (MC1324x_SPI_SR & SPI_SR_EOQF_MASK) != SPI_SR_EOQF_MASK )
#define ClearSPI_EndOfQueuelag()        MC1324x_SPI_SR |= (uint32_t) (SPI_SR_EOQF_MASK)

#if (gTargetBoard_d == gTargetTWR_K60N512_c)
  #define cSPI_MCR_Config_c             (SPI_MCR_MSTR_MASK | SPI_MCR_ROOE_MASK | SPI_MCR_PCSIS(2) | \
                                         SPI_MCR_DIS_RXF_MASK | SPI_MCR_DIS_TXF_MASK | SPI_MCR_CLR_TXF_MASK | \
                                         SPI_MCR_CLR_RXF_MASK | SPI_MCR_HALT_MASK)
#elif (gTargetBoard_d == gTargetTWR_KW21D512_c)
  #define cSPI_MCR_Config_c             (SPI_MCR_MSTR_MASK | SPI_MCR_ROOE_MASK | SPI_MCR_PCSIS(1) | \
                                         SPI_MCR_DIS_RXF_MASK | SPI_MCR_DIS_TXF_MASK | SPI_MCR_CLR_TXF_MASK | \
                                         SPI_MCR_CLR_RXF_MASK | SPI_MCR_HALT_MASK)
#endif /*gTargetBoard_d*/

#define cSPI_CTAR0_Config_c      (SPI_CTAR_FMSZ(7) | SPI_CTAR_DT(1) | SPI_CTAR_BR(2))  // BR = 6 SPI write access 4 MHz
                                // (SPI_CTAR_DBR_MASK | SPI_CTAR_FMSZ(7) | SPI_CTAR_PBR(1) | SPI_CTAR_DT(1)) /* 16MHz */
                                  
#define cSPI_CTAR1_Config_c        (SPI_CTAR_FMSZ(7) | SPI_CTAR_PBR(1) | SPI_CTAR_DT(1) | SPI_CTAR_BR(2)) // TODO BR = 6 SPI read access 2.66 MHz 
                                // (SPI_CTAR_FMSZ(7) | SPI_CTAR_PBR(1) | SPI_CTAR_DT(1)) /* 8Mhz */

#define cSPI_RSER_Config_c             (0x00000000UL)

/****************************************************************************/
/* Transceiver DMA */
/****************************************************************************/

/* DMA MUX Clock Gating Control Register  */
#define MC1324x_DMA_MUX_SIM_SCG     Radio_DMA_MUX_SIM_SCG
#define cDMA_MUX_SIM_SCG_Config_c   cDMA_MUX_SIM_SCG_Mask_c

/* eDMA Clock Gating Control Register  */
#define MC1324x_eDMA_SIM_SCG      Radio_eDMA_SIM_SCG
#define ceDMA_SIM_SCG_Config_c    ceDMA_SIM_SCG_Mask_c

/* DMA MUX control registers */

#define MC1324x_DMAMUX_SPI2MEM     Radio_DMAMUX_SPI2MEM
#define MC1324x_DMAMUX_MEM2SPI     Radio_DMAMUX_MEM2SPI
#define MC1324x_DMAMUX_MEM2MEM     Radio_DMAMUX_MEM2MEM

/* eDMA TCD words*/

/* Channel 15 */
#define MC1324x_DMA_MEM2MEM_TCD_SADDR           Radio_DMA_MEM2MEM_TCD_SADDR          
#define MC1324x_DMA_MEM2MEM_TCD_SOFF            Radio_DMA_MEM2MEM_TCD_SOFF           
#define MC1324x_DMA_MEM2MEM_TCD_ATTR            Radio_DMA_MEM2MEM_TCD_ATTR           
#define MC1324x_DMA_MEM2MEM_TCD_NBYTES_MLNO     Radio_DMA_MEM2MEM_TCD_NBYTES_MLNO    
#define MC1324x_DMA_MEM2MEM_TCD_SLAST           Radio_DMA_MEM2MEM_TCD_SLAST          
#define MC1324x_DMA_MEM2MEM_TCD_DADDR           Radio_DMA_MEM2MEM_TCD_DADDR          
#define MC1324x_DMA_MEM2MEM_TCD_DOFF            Radio_DMA_MEM2MEM_TCD_DOFF           
#define MC1324x_DMA_MEM2MEM_TCD_DLASTSGA        Radio_DMA_MEM2MEM_TCD_DLASTSGA       
#define MC1324x_DMA_MEM2MEM_TCD_CITER_ELINKNO   Radio_DMA_MEM2MEM_TCD_CITER_ELINKNO  
#define MC1324x_DMA_MEM2MEM_TCD_CITER_ELINKYES  Radio_DMA_MEM2MEM_TCD_CITER_ELINKYES 
#define MC1324x_DMA_MEM2MEM_TCD_CSR             Radio_DMA_MEM2MEM_TCD_CSR            
#define MC1324x_DMA_MEM2MEM_TCD_BITER_ELINKNO   Radio_DMA_MEM2MEM_TCD_BITER_ELINKNO  
#define MC1324x_DMA_MEM2MEM_TCD_BITER_ELINKYES  Radio_DMA_MEM2MEM_TCD_BITER_ELINKYES 

/* Channel 14 */
#define MC1324x_DMA_MEM2SPI_TCD_SADDR           Radio_DMA_MEM2SPI_TCD_SADDR         
#define MC1324x_DMA_MEM2SPI_TCD_SOFF            Radio_DMA_MEM2SPI_TCD_SOFF          
#define MC1324x_DMA_MEM2SPI_TCD_ATTR            Radio_DMA_MEM2SPI_TCD_ATTR          
#define MC1324x_DMA_MEM2SPI_TCD_NBYTES_MLNO     Radio_DMA_MEM2SPI_TCD_NBYTES_MLNO   
#define MC1324x_DMA_MEM2SPI_TCD_SLAST           Radio_DMA_MEM2SPI_TCD_SLAST         
#define MC1324x_DMA_MEM2SPI_TCD_DADDR           Radio_DMA_MEM2SPI_TCD_DADDR         
#define MC1324x_DMA_MEM2SPI_TCD_DOFF            Radio_DMA_MEM2SPI_TCD_DOFF          
#define MC1324x_DMA_MEM2SPI_TCD_DLASTSGA        Radio_DMA_MEM2SPI_TCD_DLASTSGA      
#define MC1324x_DMA_MEM2SPI_TCD_CITER_ELINKNO   Radio_DMA_MEM2SPI_TCD_CITER_ELINKNO 
#define MC1324x_DMA_MEM2SPI_TCD_CITER_ELINKYES  Radio_DMA_MEM2SPI_TCD_CITER_ELINKYES
#define MC1324x_DMA_MEM2SPI_TCD_CSR             Radio_DMA_MEM2SPI_TCD_CSR           
#define MC1324x_DMA_MEM2SPI_TCD_BITER_ELINKNO   Radio_DMA_MEM2SPI_TCD_BITER_ELINKNO 
#define MC1324x_DMA_MEM2SPI_TCD_BITER_ELINKYES  Radio_DMA_MEM2SPI_TCD_BITER_ELINKYES

/* Channel 13 */
#define MC1324x_DMA_SPI2MEM_TCD_SADDR           Radio_DMA_SPI2MEM_TCD_SADDR          
#define MC1324x_DMA_SPI2MEM_TCD_SOFF            Radio_DMA_SPI2MEM_TCD_SOFF           
#define MC1324x_DMA_SPI2MEM_TCD_ATTR            Radio_DMA_SPI2MEM_TCD_ATTR           
#define MC1324x_DMA_SPI2MEM_TCD_NBYTES_MLNO     Radio_DMA_SPI2MEM_TCD_NBYTES_MLNO    
#define MC1324x_DMA_SPI2MEM_TCD_SLAST           Radio_DMA_SPI2MEM_TCD_SLAST          
#define MC1324x_DMA_SPI2MEM_TCD_DADDR           Radio_DMA_SPI2MEM_TCD_DADDR          
#define MC1324x_DMA_SPI2MEM_TCD_DOFF            Radio_DMA_SPI2MEM_TCD_DOFF           
#define MC1324x_DMA_SPI2MEM_TCD_DLASTSGA        Radio_DMA_SPI2MEM_TCD_DLASTSGA       
#define MC1324x_DMA_SPI2MEM_TCD_CITER_ELINKNO   Radio_DMA_SPI2MEM_TCD_CITER_ELINKNO  
#define MC1324x_DMA_SPI2MEM_TCD_CITER_ELINKYES  Radio_DMA_SPI2MEM_TCD_CITER_ELINKYES 
#define MC1324x_DMA_SPI2MEM_TCD_CSR             Radio_DMA_SPI2MEM_TCD_CSR            
#define MC1324x_DMA_SPI2MEM_TCD_BITER_ELINKNO   Radio_DMA_SPI2MEM_TCD_BITER_ELINKNO  
#define MC1324x_DMA_SPI2MEM_TCD_BITER_ELINKYES  Radio_DMA_SPI2MEM_TCD_BITER_ELINKYES 


/****************************************************************************/
/* Transceiver GPIO pins mapping */
/****************************************************************************/

/* GPIO Pin Control Registers */

#define MC1324x_GPIO3_PCR          Radio_GPIO3_PCR 
#define MC1324x_GPIO4_PCR          Radio_GPIO4_PCR 
#define MC1324x_GPIO5_PCR          Radio_GPIO5_PCR 

/* GPIO PORT  Clock Gating Control Register */
#define MC1324x_GPIO_PORT_SIM_SCG      Radio_GPIO_PORT_SIM_SCG   
#define cGPIO3_PORT_SIM_SCG_Config_c   cGPIO3_PORT_SIM_SCG_Mask_c
#define cGPIO4_PORT_SIM_SCG_Config_c   cGPIO4_PORT_SIM_SCG_Mask_c
#define cGPIO5_PORT_SIM_SCG_Config_c   cGPIO5_PORT_SIM_SCG_Mask_c

#define cGPIO3_PCR_Config_c            (PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK)
#define cGPIO4_PCR_Config_c            (PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK)
#define cGPIO5_PCR_Config_c            (PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK)

/* GPIO Registers */
#define MC1324x_GPIO_PDOR          Radio_GPIO_PDOR 
#define MC1324x_GPIO_PSOR          Radio_GPIO_PSOR 
#define MC1324x_GPIO_PCOR          Radio_GPIO_PCOR 
#define MC1324x_GPIO_PTOR          Radio_GPIO_PTOR 
#define MC1324x_GPIO_PDIR          Radio_GPIO_PDIR 
#define MC1324x_GPIO_PDDR          Radio_GPIO_PDDR 

#define MC1324xDrv_GPIO3_PortConfig()                         \
{                                                                 \
  MC1324x_GPIO_PORT_SIM_SCG |= cGPIO3_PORT_SIM_SCG_Config_c; \
  MC1324x_GPIO3_PCR |= cGPIO3_PCR_Config_c;                   \
}

#define MC1324xDrv_GPIO4_PortConfig()                         \
{                                                                 \
  MC1324x_GPIO_PORT_SIM_SCG |= cGPIO4_PORT_SIM_SCG_Config_c; \
  MC1324x_GPIO4_PCR |= cGPIO4_PCR_Config_c;                   \
}

/****************************************************************************/
/* Transceiver RESET pin mapping*/
/****************************************************************************/

/* RESET Pin Control Registers */
#define MC1324x_RST_PCR           Radio_RST_PCR

/* RESET PORT  Clock Gating Control Register */
#define MC1324x_RST_PORT_SIM_SCG      Radio_RST_PORT_SIM_SCG   
#define cRST_PORT_SIM_SCG_Config_c    cRST_PORT_SIM_SCG_Mask_c 

#define cRST_PCR_Config_c    (PORT_PCR_MUX(1) | PORT_PCR_DSE_MASK)

/* GPIO Registers */
#define MC1324x_RST_PDOR      Radio_RST_PDOR 
#define MC1324x_RST_PSOR      Radio_RST_PSOR 
#define MC1324x_RST_PCOR      Radio_RST_PCOR 
#define MC1324x_RST_PTOR      Radio_RST_PTOR 
#define MC1324x_RST_PDIR      Radio_RST_PDIR 
#define MC1324x_RST_PDDR      Radio_RST_PDDR 

/****************************************************************************/
/* Transceiver IRQ pin mapping */
/****************************************************************************/

/* Interrupt Pin Control Registers */
#define MC1324x_IRQ_PCR   Radio_IRQ_PCR

#define cIRQ_PCR_Config_c    (PORT_PCR_MUX(1))

/* GPIO PORT  Clock Gating Control Register */
#define MC1324x_IRQ_PORT_SIM_SCG      Radio_IRQ_PORT_SIM_SCG   
#define cIRQ_PORT_SIM_SCG_Config_c    cIRQ_PORT_SIM_SCG_Mask_c 

#define MC1324x_IRQ_PDOR     Radio_IRQ_PDOR   
#define MC1324x_IRQ_PSOR     Radio_IRQ_PSOR   
#define MC1324x_IRQ_PCOR     Radio_IRQ_PCOR   
#define MC1324x_IRQ_PTOR     Radio_IRQ_PTOR   
#define MC1324x_IRQ_PDIR     Radio_IRQ_PDIR   
#define MC1324x_IRQ_PDDR     Radio_IRQ_PDDR   


/*****************************************************************************
 *                            PUBLIC FUNCTIONS                               *
 *---------------------------------------------------------------------------*
 * Add to this section all the global functions prototype preceded (as a     *
 * good practice) by the keyword 'extern'                                    *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_SPIInit
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
extern void MC1324xDrv_SPIInit
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_SPI_DMA_Init
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_SPI_DMA_Init
(
  void
);

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
);

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
);

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
);

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
);

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
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_DirectAccessSPIRead
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t MC1324xDrv_DirectAccessSPIRead
(
 uint8_t address
);

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
);

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
);

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
);

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
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IndirectAccessSPIRead
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t MC1324xDrv_IndirectAccessSPIRead
(
 uint8_t address
);
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
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IRQ_PortConfig
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_IRQ_PortConfig
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IsIrqPending
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
bool_t  MC1324xDrv_IsIrqPending
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IRQ_Disable
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_IRQ_Disable
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IRQ_Enable
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_IRQ_Enable
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_IRQ_Clear
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_IRQ_Clear
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_RST_PortConfig
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_RST_B_PortConfig
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_RST_Assert
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_RST_B_Assert
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_RST_Deassert
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_RST_B_Deassert
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_SoftRST_Assert
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_SoftRST_Assert
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_SoftRST_Deassert
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_SoftRST_Deassert
(
  void
);


/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_RESET
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_RESET
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_Soft_RESET
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_Soft_RESET
(
  void
);

/*---------------------------------------------------------------------------
 * Name: MC1324xDrv_Set_CLK_OUT_Freq
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void MC1324xDrv_Set_CLK_OUT_Freq
(
  uint8_t freqDiv
);

#endif /* __TRANSCEIVER_DRV_H__ */