/**************************************************************************
 * Filename: TargetTWR-KW21D512.h
 *
 * Description: TWR-KW21D512 pins/ports mappings
 *
 * Copyright (c) 2012, Freescale Semiconductor, Inc. All rights reserved.
 *
 ***************************************************************************
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
 ***************************************************************************/

#ifndef TARGETTWRKW21D512_H_
#define TARGETTWRKW21D512_H_

//#include "MK21D5.h"
#include "IO_map.h"
/* 
 * Include the generic CPU header file 
 */


////////////////////////////////////////
/////// Target Global Definitions //////
////////////////////////////////////////


/*
 *  Name: gSystemClk24MHz_c
 *  Description: 24 MHz system clock 
 */
#ifndef gSystemClk24MHz_c
#define gSystemClk24MHz_c                        24
#endif

/*
 *  Name: gSystemClk48MHz_c
 *  Description: 48 MHz system clock 
 */
#ifndef gSystemClk48MHz_c
#define gSystemClk48MHz_c                        48
#endif

/*
 *  Name: gSystemClk96MHz_c
 *  Description: 96 MHz system clock 
 */
#ifndef gSystemClk96MHz_c
#define gSystemClk96MHz_c                        96
#endif

/*
 * Name: gSystemClock_d
 * Description: Peripheral Bus Clock (MHz units)
 */
#ifndef gSystemClock_c
#define gSystemClock_c                           gSystemClk48MHz_c                                                   
#endif

/*
 * Name: gMhzToKHzConvert_c
 * Description: Specifies how many KHz are in 1 MHz
 */
#ifndef gMhzToKHzConvert_c
#define gMhzToKHzConvert_c      (1000)
#endif

////////////////////////////////////////
///////////////// UART /////////////////
////////////////////////////////////////

/*
 * Name: UART_NR_INSTANCES
 * Description: number of UART peripherals on board
 */ 
#define  gUART_HW_MODULES_AVAILABLE_c            4

#define gUART_HW_MOD_0_c                         0
#define gUART_HW_MOD_1_c                         1
#define gUART_HW_MOD_2_c                         2
#define gUART_HW_MOD_3_c                         3


#define gUART0_BASE_PTR_c                        UART0_BASE_PTR
#define gUART1_BASE_PTR_c                        UART1_BASE_PTR
#define gUART2_BASE_PTR_c                        UART2_BASE_PTR
#define gUART3_BASE_PTR_c                        UART3_BASE_PTR


#define gUART0_LON_IRQ_NUM_c                     (30)  
#define gUART0_IRQ_NUM_c                         (31)
#define gUART0_ERR_IRQ_NUM_c                     (32)
#define gUART1_IRQ_NUM_c                         (33)
#define gUART1_ERR_IRQ_NUM_c                     (34)
#define gUART2_IRQ_NUM_c                         (35)
#define gUART2_ERR_IRQ_NUM_c                     (36)
#define gUART3_IRQ_NUM_c                         (37)
#define gUART3_ERR_IRQ_NUM_c                     (38)


#define gUART0_SIM_SCGC_REG_c                    SIM_SCGC4
#define gUART1_SIM_SCGC_REG_c                    SIM_SCGC4
#define gUART2_SIM_SCGC_REG_c                    SIM_SCGC4
#define gUART3_SIM_SCGC_REG_c                    SIM_SCGC4


#define gUART0_SIM_SCGC_BIT_c                    (10)
#define gUART1_SIM_SCGC_BIT_c                    (11)
#define gUART2_SIM_SCGC_BIT_c                    (12)
#define gUART3_SIM_SCGC_BIT_c                    (13)

/*Port A*/

#define gUART0_PORT_SIM_SCGC_REG_c               SIM_SCGC5       /* Port clock gating register */
#define gUART0_PORT_SIM_SGC_BIT_c                (1<<9)          /* Port clock gating bit */
#define gUART0_PORT_MUX_c                        (2)             /* Port MUX (alternate function) */
#define gUART0_RX_PCR_REG_c                      PORTA_PCR1      /* Port Control Register (PCR) associated with RX pin */
#define gUART0_TX_PCR_REG_c                      PORTA_PCR2      /* Port Control Register (PCR) associated with TX pin */
#define gUART0_RTS_PCR_REG_c                     PORTA_PCR3      /* Port Control Register (PCR) associated with RTS pin */
#define gUART0_CTS_PCR_REG_c                     PORTA_PCR0      /* Port Control Register (PCR) associated with CTS pin */

/*Port E*/
#define gUART1_PORT_SIM_SCGC_REG_c               SIM_SCGC5       /* Port clock gating register */
#define gUART1_PORT_SIM_SGC_BIT_c                (1<<13)         /* Port clock gating bit */
#define gUART1_PORT_MUX_c                        (3)             /* Port MUX (alternate function) */
#define gUART1_RX_PCR_REG_c                      PORTE_PCR1      /* Port Control Register (PCR) associated with RX pin */
#define gUART1_TX_PCR_REG_c                      PORTE_PCR0      /* Port Control Register (PCR) associated with TX pin */
#define gUART1_RTS_PCR_REG_c                     PORTE_PCR3      /* Port Control Register (PCR) associated with RTS pin */
#define gUART1_CTS_PCR_REG_c                     PORTE_PCR2      /* Port Control Register (PCR) associated with CTS pin */
/*Port E*/
#define gUART2_PORT_SIM_SCGC_REG_c               SIM_SCGC5       /* Port clock gating register */
#define gUART2_PORT_SIM_SGC_BIT_c                (1<<13)         /* Port clock gating bit */
#define gUART2_PORT_MUX_c                        (3)             /* Port MUX (alternate function) */
#define gUART2_RX_PCR_REG_c                      PORTE_PCR17      /* Port Control Register (PCR) associated with RX pin */
#define gUART2_TX_PCR_REG_c                      PORTE_PCR16      /* Port Control Register (PCR) associated with TX pin */
#define gUART2_RTS_PCR_REG_c                     PORTE_PCR19      /* Port Control Register (PCR) associated with RTS pin */
#define gUART2_CTS_PCR_REG_c                     PORTE_PCR18      /* Port Control Register (PCR) associated with CTS pin */

#define gUART3_PORT_SIM_SCGC_REG_c               SIM_SCGC5       /* Port clock gating register */
#define gUART3_PORT_SIM_SGC_BIT_c                (1<<11)         /* Port clock gating bit */
#define gUART3_PORT_MUX_c                        (3)             /* Port MUX (alternate function) */
#define gUART3_RX_PCR_REG_c                      PORTC_PCR16     /* Port Control Register (PCR) associated with RX pin */
#define gUART3_TX_PCR_REG_c                      PORTC_PCR17     /* Port Control Register (PCR) associated with TX pin */
#define gUART3_RTS_PCR_REG_c                     PORTC_PCR18     /* Port Control Register (PCR) associated with RTS pin */
#define gUART3_CTS_PCR_REG_c                     PORTC_PCR19     /* Port Control Register (PCR) associated with CTS pin */


/* Error flags masks */
#define gUART_PARITY_ERROR_FLAG_c                (0x1u)              
#define gUART_FRAME_ERROR_FLAG_c                 (0x2u)
#define gUART_NOISE_FLAG_c                       (0x4u)
#define gUART_OVERRUN_FLAG_c                     (0x8u)

/* Parity masks */
#define gUART_PARITY_EN_BIT_c                    (0x2u)
#define gUART_PARITY_EVEN_BIT_c                  (0x1u)

/* Data bits mask */
#define gUART_DATA_BITS_BIT_c                    (0x8u)

/* FIFO flush masks */
#define gUART_TX_FIFO_FLUSH_c                    (0x80u)
#define gUART_RX_FIFO_FLUSH_c                    (0x40u)

/* TX/RX enable bits mask */
#define gUART_TX_EN_BIT_c                        (0x8u)
#define gUART_RX_EN_BIT_c                        (0x4u)

/* UART RX/TX interrupt masks */
#define gUART_RX_INT_BIT_c                       (0x20u)
#define gUART_TX_INT_BIT_c                       (0x80u)

/* UART RX/TX interrupt enable masks */
#define gUART_TIE_BIT_c                          (0x40u)
#define gUART_RIE_BIT_c                          (0x20u)

/* UART HW flow control masks */
#define gUART_RXRTSE_BIT_c                       (0x8u)
#define gUART_TXRTSPOL_BIT_c                     (0x4u)
#define gUART_TXRTSE_BIT_c                       (0x2u)
#define gUART_TXCTSE_BIT_c                       (0x1u)                      

/* UART baudrate masks */
#define gUART_BDH_MASK_c                         (0x001Fu)
#define gUART_BDL_MASK_c                         (0x00FFu)
#define gUART_BRFA_MASK_c                        (0x1F)

#define gUART_TX_FIFO_EN_BIT_c                   (0x80)     /* transmit FIFO enable bit */  
#define gUART_TX_FIFO_MASK_c                     (0x70)     /* transmit FIFO buffer depth mask */

#define gUART_RX_FIFO_EN_BIT_c                   (0x8)      /* receive FIFO enable bit */
#define gUART_RX_FIFO_MASK_c                     (0x7)      /* receive FIFO buffer depth mask */

#define gUART_S1_TDRE_MASK_c                     (0x80)    /* TDRE bit mask */
#define gUART_S1_RDRF_MASK_c                     (0x20)    /* RDRF bit mask */  

/*
 * Name: see below
 * Description: UART Software Flow Control ports/pins mapping
 */
#define gUART1_SW_RTS_PDDR_c                    GPIOE_PDDR   /* GPIO port data direction register */
#define gUART1_SW_RTS_PDOR_c                    GPIOE_PDOR   /* GPIO port data output register */
#define gUART1_SW_RTS_PSOR_c                    GPIOE_PSOR   /* GPIO port set output register */
#define gUART1_SW_RTS_PCOR_c                    GPIOE_PCOR   /* GPIO port clear output register */
#define gUART1_SW_RTS_PIN_MASK_c                (1<<3)      /* the corresponding pin in the above registers */

#define gUART1_SW_CTS_PDDR_c                    GPIOE_PDDR   /* GPIO port data direction register */
#define gUART1_SW_CTS_PDIR_c                    GPIOE_PDIR   /* GPIO port input register */
#define gUART1_SW_CTS_PIN_MASK_c                (1<<2)      /* the corresponding pin in the above registers */

#define gUART2_SW_RTS_PDDR_c                    GPIOE_PDDR   /* GPIO port data direction register */
#define gUART2_SW_RTS_PDOR_c                    GPIOE_PDOR   /* GPIO port data output register */
#define gUART2_SW_RTS_PSOR_c                    GPIOE_PSOR   /* GPIO port set output register */ 
#define gUART2_SW_RTS_PCOR_c                    GPIOE_PCOR   /* GPIO port clear output register */
#define gUART2_SW_RTS_PIN_MASK_c                (1<<19)      /* the corresponding pin in the above registers */

#define gUART2_SW_CTS_PDDR_c                    GPIOE_PDDR   /* GPIO port data direction register */
#define gUART2_SW_CTS_PDIR_c                    GPIOE_PDIR   /* GPIO port input register */
#define gUART2_SW_CTS_PIN_MASK_c                (1<<18)      /* the corresponding pin in the above registers */

#define gUART1_SW_CTS_SIM_SCGC_REG_c            SIM_SCGC5    /* clock gating register for the port used by the CTS signal */
#define gUART1_SW_CTS_SIM_SCGC_BIT_c            13           /* bit position in the above register (clock gating register used by CTS) */

#define gUART1_SW_RTS_SIM_SCGC_REG_c            SIM_SCGC5    /* clock gating register for the port used by the RTS signal */
#define gUART1_SW_RTS_SIM_SCGC_BIT_c            13           /* bit position in the above register (clock gating register used by RTS) */

#define gUART2_SW_CTS_SIM_SCGC_REG_c            SIM_SCGC5    /* clock gating register for the port used by the CTS signal */
#define gUART2_SW_CTS_SIM_SCGC_BIT_c            13           /* bit position in the above register (clock gating register used by CTS) */

#define gUART2_SW_RTS_SIM_SCGC_REG_c            SIM_SCGC5    /* clock gating register for the port used by the RTS signal */
#define gUART2_SW_RTS_SIM_SCGC_BIT_c            13           /* bit position in the above register (clock gating register used by RTS) */

#define gUART1_SW_CTS_PCR_REG_c                 PORTE_PCR13  /* UART1 CTS Pin Control Register */
#define gUART1_SW_CTS_MUX_c                     (1)          /* UART1 CTS pin alternate function (MUX) */
#define gUART1_SW_RTS_PCR_REG_c                 PORTE_PCR12  /* UART1 RTS Pin Control Register */
#define gUART1_SW_RTS_MUX_c                     (1)          /* UART1 RTS pin alternate function (MUX) */

#define gUART2_SW_CTS_PCR_REG_c                 PORTE_PCR19  /* UART2 CTS Pin Control Register */
#define gUART2_SW_CTS_MUX_c                     (1)          /* UART2 CTS pin alternate function (MUX) */
#define gUART2_SW_RTS_PCR_REG_c                 PORTE_PCR18  /* UART2 RTS Pin Control Register */
#define gUART2_SW_RTS_MUX_c                     (1)          /* UART2 RTS pin alternate function (MUX) */    


////////////////////////////////////////
/////// Define LED port mapping ////////
////////////////////////////////////////

/*
 * LED Port Data Output register mapping
 */
#define gLED_PortDataOutReg_c                    GPIOD_PDOR

/*
 * LED Port Data Direction register mapping
 */
#define gLED_PortDataDirReg_c                    GPIOD_PDDR

/*
 * LED Port Data Set register mapping 
 */
#define gLED_PortDataSetReg_c                    GPIOD_PSOR

/*
 * LED Port Data Clear register mapping
 */
#define gLED_PortDataClearReg_c                  GPIOD_PCOR

/*
 * LED Port Data Toggle register mapping
 */
#define gLED_PortDataToggleReg_c                 GPIOD_PTOR

/*
 * LED 1 pin definition
 */
#define gLED_Led1Pin_c                           (1<<4)

/*
 * LED 2 pin definition
 */
#define gLED_Led2Pin_c                           (1<<5)

/*
 * LED 3 pin definition
 */
#define gLED_Led3Pin_c                           (1<<6)

/*
 * LED 4 pin definition
 */
#define gLED_Led4Pin_c                           (1<<7)

/*
 * LED 1 Pin Control register mapping
 */
#define gLED_Led1PinCtrlReg_c                    PORTD_PCR4

/*
 * LED 2 Pin Control register mapping
 */
#define gLED_Led2PinCtrlReg_c                    PORTD_PCR5

/*
 * LED 3 Pin Control register mapping
 */
#define gLED_Led3PinCtrlReg_c                    PORTD_PCR6

/*
 * LED 4 Pin Control register mapping
 */
#define gLED_Led4PinCtrlReg_c                    PORTD_PCR7

/*
 * LED port clock gating register mapping
 */
#define gLED_ClkGatingReg_c                      SIM_SCGC5

/*
 * LED port clock gating bit 
 */
#define gLED_ClkGatingBit_c                      (1<<12)

/*
 * LED Pin Control Alternate Function
 */
#define gLED_PinCtrlAltFunc_c                    1

/*
 * LEDs count on target board
 */
#define gLEDsOnTargetBoardCnt_c                  4

////////////////////////////////////////
/////// Define Keyboard port mapping ////////
////////////////////////////////////////

#define gKeyboard_Irq_Number            (61)

#define gKeyboard_PortDataOutReg_c               GPIOC_PDOR
#define gKeyboard_PortDataDirReg_c               GPIOC_PDDR
#define gKeyboard_PortDataSetReg_c               GPIOC_PSOR
#define gKeyboard_PortDataClearReg_c             GPIOC_PCOR
#define gKeyboard_PortDataToggleReg_c            GPIOC_PTOR

#define gKeyboard_Switch1Pin_c                   (1<<4)
#define gKeyboard_Switch2Pin_c                   (1<<5)
#define gKeyboard_Switch3Pin_c                   (1<<6)
#define gKeyboard_Switch4Pin_c                   (1<<7)

/*
 * LED 1 Pin Control register mapping
 */
#define gKeyboard_Switch1PinCtrlReg_c            PORTC_PCR4
#define gKeyboard_Switch2PinCtrlReg_c            PORTC_PCR5
#define gKeyboard_Switch3PinCtrlReg_c            PORTC_PCR6
#define gKeyboard_Switch4PinCtrlReg_c            PORTC_PCR7

#define gKeyboard_ClkGatingReg_c                 SIM_SCGC5

#define gKeyboard_ClkGatingBit_c                 (1<<SIM_SCGC5_PORTC_SHIFT)

#define gSwitch_PinCtrlAltFunc_c                    1

#define gSwitchsOnTargetBoardCnt_c                  4
   

////////////////////////////////////////
///////// GPIO port mapping ///////////
////////////////////////////////////////   
/*
 * GPIO 5 Pin Control register mapping
 */
#define gGPIO_Gpio1PinCtrlReg_c                    PORTD_PCR1

/*
 * GPIO port clock gating register mapping
 */
#define gGPIO_ClkGatingReg_c                      SIM_SCGC5

/*
 * GPIO port clock gating bit 
 */
#define gGPIO_ClkGatingBit_c                      (1<<12)

/*
 * GPIO 5 pin definition
 */
#define gGPIO_Gpio1Pin_c                           (1<<1)

/*
 * GPIO Pin Control Alternate Function
 */
#define gGPIO_PinCtrlAltFunc_c                    1   
   
   
/*
 * GPIO Port Data Output register mapping
 */
#define gGPIO_PortDataOutReg_c                    GPIOD_PDOR

/*
 * GPIO Port Data Direction register mapping
 */
#define gGPIO_PortDataDirReg_c                    GPIOD_PDDR

/*
 * GPIO Port Data Set register mapping 
 */
#define gGPIO_PortDataSetReg_c                    GPIOD_PSOR

/*
 * GPIO Port Data Clear register mapping
 */
#define gGPIO_PortDataClearReg_c                  GPIOD_PCOR

/*
 * GPIO Port Data Toggle register mapping
 */
#define gGPIO_PortDataToggleReg_c                 GPIOD_PTOR

   
#define GpioOn()		(gGPIO_PortDataSetReg_c |= gGPIO_Gpio1Pin_c)

#define GpioOff()	        (gGPIO_PortDataClearReg_c |= gGPIO_Gpio1Pin_c)


////////////////////////////////////////
///////// Radio port mapping ///////////
////////////////////////////////////////   

/* Interrupt Pin Control Registers */
#define Radio_IRQ_PCR                   PORTB_PCR3        /* Port B Pin Control Register 3 */   
#define Radio_IRQ_PinNumberMask_c       (1<<3)

#define Radio_Irq_Number        60   

/* IRQ GPIO PORT  Clock Gating Control Register */
#define Radio_IRQ_PORT_SIM_SCG      SIM_SCGC5
#define cIRQ_PORT_SIM_SCG_Mask_c    (1<<10)           /*Port B Clock Gate Control enable*/

/* IRQ GPIO Registers */ 
#define Radio_IRQ_PDOR          GPIOB_PDOR        /* Port Data Output Register */
#define Radio_IRQ_PSOR          GPIOB_PSOR        /* Port Set Output Register */
#define Radio_IRQ_PCOR          GPIOB_PCOR        /* Port Clear Output Register */
#define Radio_IRQ_PTOR          GPIOB_PTOR        /* Port Toggle Output Register */
#define Radio_IRQ_PDIR          GPIOB_PDIR        /* Port Data Input Register */
#define Radio_IRQ_PDDR          GPIOB_PDDR        /* Port Data Direction Register */   

/* RESET Pin Control Registers */
#define Radio_RST_PCR           PORTB_PCR19        /* Port B Pin Control Register 9 */
#define Radio_RST_PinNumberMask_c       (1<<19)

/* RESET PORT  Clock Gating Control Register */
#define Radio_RST_PORT_SIM_SCG          SIM_SCGC5
#define cRST_PORT_SIM_SCG_Mask_c    (1<<10)            /*PORT B Clock Gate Control enable*/  

/* RESET GPIO Registers */
#define Radio_RST_PDOR          GPIOB_PDOR        /* Port Data Output Register */
#define Radio_RST_PSOR          GPIOB_PSOR        /* Port Set Output Register */
#define Radio_RST_PCOR          GPIOB_PCOR        /* Port Clear Output Register */
#define Radio_RST_PTOR          GPIOB_PTOR        /* Port Toggle Output Register */
#define Radio_RST_PDIR          GPIOB_PDIR        /* Port Data Input Register */
#define Radio_RST_PDDR          GPIOB_PDDR        /* Port Data Direction Register */   
   
/* SPI Pin Control Registers */
#define Radio_SPI_SSEL_PCR       PORTB_PCR10       /* Port B Pin Control Register 15 */
#define Radio_SPI_SCLK_PCR       PORTB_PCR11       /* Port B Pin Control Register 12 */
#define Radio_SPI_MOSI_PCR       PORTB_PCR16       /* Port B Pin Control Register 13 */
#define Radio_SPI_MISO_PCR       PORTB_PCR17       /* Port B Pin Control Register 14 */   
   
/* SPI PORT  Clock Gating Control Register */
#define Radio_SPI_PORT_SIM_SCG   SIM_SCGC5
#define cSPI_PORT_SIM_SCG_Mask_c     (1<<10)       /* Port B Clock Gate Control enable*/   

/* SPI Module Clock Gating Control Register */

#define Radio_SPI_SIM_SCG        SIM_SCGC6         /* System Clock Gating Control Register 6 */
#define cSPI_SIM_SCG_Mask_c      (1<<13)           /*SPI1 Clock Gate Control enable*/   

/* SPI Registers */
#define Radio_SPI_MCR            SPI1_MCR          /* DSPI Module Configuration Register */
#define Radio_SPI_TCR            SPI1_TCR          /* DSPI Transfer Count Register */
#define Radio_SPI_CTAR0          SPI1_CTAR0        /* DSPI Clock and Transfer Attributes Register in Master Mode*/
#define Radio_SPI_CTAR0_SLAVE    SPI1_CTAR0_SLAVE  /* DSPI Clock and Transfer Attributes Register in Slave Mode*/
#define Radio_SPI_CTAR1          SPI1_CTAR1        /* DSPI Clock and Transfer Attributes Register in Master Mode*/
#define Radio_SPI_SR             SPI1_SR           /* DSPI Status Register */
#define Radio_SPI_RSER           SPI1_RSER         /* DSPI DMA/Interrupt Request Select and Enable Register */
#define Radio_SPI_PUSHR          SPI1_PUSHR        /* DSPI PUSH TX FIFO Register In Master Mode */
#define Radio_SPI_PUSHR_SLAVE    SPI1_PUSHR_SLAVE  /* DSPI PUSH TX FIFO Register In Slave Mode */
#define Radio_SPI_POPR           SPI1_POPR         /* DSPI POP RX FIFO Register */
#define Radio_SPI_TXFR0          SPI1_TXFR0        /* DSPI Transmit FIFO Registers 0 */
#define Radio_SPI_TXFR1          SPI1_TXFR1        /* DSPI Transmit FIFO Registers 1 */
#define Radio_SPI_TXFR2          SPI1_TXFR2        /* DSPI Transmit FIFO Registers 2 */
#define Radio_SPI_TXFR3          SPI1_TXFR3        /* DSPI Transmit FIFO Registers 3 */
#define Radio_SPI_RXFR0          SPI1_RXFR0        /* DSPI Receive FIFO Registers 0 */
#define Radio_SPI_RXFR1          SPI1_RXFR1        /* DSPI Receive FIFO Registers 1 */
#define Radio_SPI_RXFR2          SPI1_RXFR2        /* DSPI Receive FIFO Registers 2 */
#define Radio_SPI_RXFR3          SPI1_RXFR3        /* DSPI Receive FIFO Registers 3 */   

#define cSPI_PUSHR_PCS_target_c         SPI_PUSHR_PCS(1)    /* PCS 0 selected */ 
#define cSPI_PUSHR_CTAS_read_target_c   SPI_PUSHR_CTAS(0)   /* CTAS 0 selected */
#define cSPI_PUSHR_CTAS_write_target_c  SPI_PUSHR_CTAS(0)   /* CTAS 0 selected */
   
/* DMA MUX Clock Gating Control Register  */
#define Radio_DMA_MUX_SIM_SCG   SIM_SCGC6
#define cDMA_MUX_SIM_SCG_Mask_c     (1<<1)           /*DMA MUX Clock Gate Control enable*/

/* eDMA Clock Gating Control Register  */
#define Radio_eDMA_SIM_SCG      SIM_SCGC7
#define ceDMA_SIM_SCG_Mask_c        (1<<1)            /*eDMA module Clock Gate Control enable*/

/* DMA MUX control registers */
#define Radio_DMAMUX_SPI2MEM     DMAMUX_CHCFG13
#define Radio_DMAMUX_MEM2SPI     DMAMUX_CHCFG14
#define Radio_DMAMUX_MEM2MEM     DMAMUX_CHCFG15   

/* eDMA TCD words*/

/* Channel 15 */
#define Radio_DMA_MEM2MEM_TCD_SADDR           DMA_TCD15_SADDR
#define Radio_DMA_MEM2MEM_TCD_SOFF            DMA_TCD15_SOFF
#define Radio_DMA_MEM2MEM_TCD_ATTR            DMA_TCD15_ATTR
#define Radio_DMA_MEM2MEM_TCD_NBYTES_MLNO     DMA_TCD15_NBYTES_MLNO
#define Radio_DMA_MEM2MEM_TCD_SLAST           DMA_TCD15_SLAST
#define Radio_DMA_MEM2MEM_TCD_DADDR           DMA_TCD15_DADDR
#define Radio_DMA_MEM2MEM_TCD_DOFF            DMA_TCD15_DOFF
#define Radio_DMA_MEM2MEM_TCD_DLASTSGA        DMA_TCD15_DLASTSGA
#define Radio_DMA_MEM2MEM_TCD_CITER_ELINKNO   DMA_TCD15_CITER_ELINKNO
#define Radio_DMA_MEM2MEM_TCD_CITER_ELINKYES  DMA_TCD15_CITER_ELINKYES
#define Radio_DMA_MEM2MEM_TCD_CSR             DMA_TCD15_CSR
#define Radio_DMA_MEM2MEM_TCD_BITER_ELINKNO   DMA_TCD15_BITER_ELINKNO
#define Radio_DMA_MEM2MEM_TCD_BITER_ELINKYES  DMA_TCD15_BITER_ELINKYES

/* Channel 14 */
#define Radio_DMA_MEM2SPI_TCD_SADDR           DMA_TCD14_SADDR
#define Radio_DMA_MEM2SPI_TCD_SOFF            DMA_TCD14_SOFF
#define Radio_DMA_MEM2SPI_TCD_ATTR            DMA_TCD14_ATTR
#define Radio_DMA_MEM2SPI_TCD_NBYTES_MLNO     DMA_TCD14_NBYTES_MLNO
#define Radio_DMA_MEM2SPI_TCD_SLAST           DMA_TCD14_SLAST
#define Radio_DMA_MEM2SPI_TCD_DADDR           DMA_TCD14_DADDR
#define Radio_DMA_MEM2SPI_TCD_DOFF            DMA_TCD14_DOFF
#define Radio_DMA_MEM2SPI_TCD_DLASTSGA        DMA_TCD14_DLASTSGA
#define Radio_DMA_MEM2SPI_TCD_CITER_ELINKNO   DMA_TCD14_CITER_ELINKNO
#define Radio_DMA_MEM2SPI_TCD_CITER_ELINKYES  DMA_TCD14_CITER_ELINKYES
#define Radio_DMA_MEM2SPI_TCD_CSR             DMA_TCD14_CSR
#define Radio_DMA_MEM2SPI_TCD_BITER_ELINKNO   DMA_TCD14_BITER_ELINKNO
#define Radio_DMA_MEM2SPI_TCD_BITER_ELINKYES  DMA_TCD14_BITER_ELINKYES

/* Channel 13 */
#define Radio_DMA_SPI2MEM_TCD_SADDR           DMA_TCD13_SADDR
#define Radio_DMA_SPI2MEM_TCD_SOFF            DMA_TCD13_SOFF
#define Radio_DMA_SPI2MEM_TCD_ATTR            DMA_TCD13_ATTR
#define Radio_DMA_SPI2MEM_TCD_NBYTES_MLNO     DMA_TCD13_NBYTES_MLNO
#define Radio_DMA_SPI2MEM_TCD_SLAST           DMA_TCD13_SLAST
#define Radio_DMA_SPI2MEM_TCD_DADDR           DMA_TCD13_DADDR
#define Radio_DMA_SPI2MEM_TCD_DOFF            DMA_TCD13_DOFF
#define Radio_DMA_SPI2MEM_TCD_DLASTSGA        DMA_TCD13_DLASTSGA
#define Radio_DMA_SPI2MEM_TCD_CITER_ELINKNO   DMA_TCD13_CITER_ELINKNO
#define Radio_DMA_SPI2MEM_TCD_CITER_ELINKYES  DMA_TCD13_CITER_ELINKYES
#define Radio_DMA_SPI2MEM_TCD_CSR             DMA_TCD13_CSR
#define Radio_DMA_SPI2MEM_TCD_BITER_ELINKNO   DMA_TCD13_BITER_ELINKNO
#define Radio_DMA_SPI2MEM_TCD_BITER_ELINKYES  DMA_TCD13_BITER_ELINKYES
   
/* GPIO Pin Control Registers */
#define Radio_GPIO3_PCR          PORTC_PCR3        /* Port C Pin Control Register 3 */
#define Radio_GPIO4_PCR          PORTC_PCR1       /* Port C Pin Control Register 1 */
#define Radio_GPIO5_PCR          PORTC_PCR0       /* Port C Pin Control Register 0 */

/* GPIO PORT  Clock Gating Control Register */
#define Radio_GPIO_PORT_SIM_SCG     SIM_SCGC5
#define cGPIO3_PORT_SIM_SCG_Mask_c    (1<<11)          /*Port C Clock Gate Control enable*/
#define cGPIO4_PORT_SIM_SCG_Mask_c    (1<<11)           /*Port C Clock Gate Control enable*/ 
#define cGPIO5_PORT_SIM_SCG_Mask_c    (1<<11)          /*Port C Clock Gate Control enable*/  

/* GPIO Registers */

#define Radio_GPIO_PDOR          GPIOC_PDOR        /* Port Data Output Register */
#define Radio_GPIO_PSOR          GPIOC_PSOR        /* Port Set Output Register */
#define Radio_GPIO_PCOR          GPIOC_PCOR        /* Port Clear Output Register */
#define Radio_GPIO_PTOR          GPIOC_PTOR        /* Port Toggle Output Register */
#define Radio_GPIO_PDIR          GPIOC_PDIR        /* Port Data Input Register */
#define Radio_GPIO_PDDR          GPIOC_PDDR        /* Port Data Direction Register */   

   
   



#endif /* TARGETTWRKW21D512_H_ */



