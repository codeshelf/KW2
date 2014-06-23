/******************************************************************************
 * Public header file for the UART/SCI driver.
 * 
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2011 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
*******************************************************************************
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
***************************************************************************//*!
******************************************************************************/

/****************************************************************************** 
 * On the receive side, this module keeps a small circular buffer, and calls
 * a callback function whenever a byte is received. The application can
 * retrieve bytes from the buffer at it's convenience, as long as it does so
 * before the driver's buffer fills up.
 *
 * On the transmit side, this module keeps a list of buffers to be
 * transmitted, and calls an application callback function when the entire
 * buffer has been sent. The application is responsible for ensuring that the
 * buffer is available until the callback function is called.
 *
 * If both SCI ports are enabled, each has it's own Rx circular buffer and
 * list of Tx buffers.
 *****************************************************************************/

#ifndef __UART_INTERFACE_H__
#define __UART_INTERFACE_H__

/******************************************************************************
*******************************************************************************
* Includes
*******************************************************************************
******************************************************************************/

#include "EmbeddedTypes.h"
#include "PortConfig.h"

/******************************************************************************
*******************************************************************************
* Public macros
*******************************************************************************
******************************************************************************/

/******************************************************************************
 * Name: gUart1_Enabled_d
 * Description: Specifies if the UART_1 HW interface is used
 *****************************************************************************/  
#ifdef gUARTInterface_d
#ifndef gUart1_Enabled_d
#if (gTargetBoard_d == gTargetTWR_K60N512_c)
  #define gUart1_Enabled_d    FALSE
#elif (gTargetBoard_d == gTargetTWR_KW21D512_c)
  #define gUart1_Enabled_d    TRUE
#endif  /*gTargetBoard_d*/
#endif  /*gUart1_Enabled_d*/


/******************************************************************************
 * Name: gUart2_Enabled_d
 * Description: Specifies if the UART_2 HW interface is used
 *****************************************************************************/ 

#ifndef gUart2_Enabled_d

#if (gTargetBoard_d == gTargetTWR_K60N512_c)
  #define gUart2_Enabled_d    TRUE
#elif (gTargetBoard_d == gTargetTWR_KW21D512_c)
  #define gUart2_Enabled_d    FALSE
#endif  /*gTargetBoard_d*/
#endif  /*gUart2_Enabled_d*/

/******************************************************************************
 * Name: gUart1_c
 * Description: hardware UART module associated to the first UART peripheral
 *****************************************************************************/ 
#ifndef gUart1_c
#if (gTargetBoard_d == gTargetTWR_K60N512_c)
#define  gUart1_c                gUART_HW_MOD_3_c  
#elif (gTargetBoard_d == gTargetTWR_KW21D512_c)
#define  gUart1_c                gUART_HW_MOD_1_c
#endif  /*gTargetBoard_d*/
#endif  /*gUart1_c*/

/******************************************************************************
 * Name: gUart2_c
 * Description: hardware UART module associated to the first UART peripheral
 *****************************************************************************/
#ifndef gUart2_c
#if (gTargetBoard_d == gTargetTWR_K60N512_c)
#define  gUart2_c                gUART_HW_MOD_5_c
#elif (gTargetBoard_d == gTargetTWR_KW21D512_c)
#define  gUart2_c                gUART_HW_MOD_2_c
#endif  /*gTargetBoard_d*/
#endif  /*gUart2_c*/

/******************************************************************************
 * Name: UART_USE_TX_FIFO
 * Description: enable / disable UART TX FIFO
 *****************************************************************************/
#ifndef gUART_USE_TX_FIFO_d
#define gUART_USE_TX_FIFO_d     (TRUE)
#endif

/******************************************************************************
 * Name: gUART_USE_RX_FIFO_d
 * Description: enable / disable UART RX FIFO 
 *****************************************************************************/
#ifndef gUART_USE_RX_FIFO_d
#define gUART_USE_RX_FIFO_d     (TRUE)
#endif

/******************************************************************************
 * Name: gUART_TX_THRESHOLD_c
 * Description: UART TX threshold value 
 *****************************************************************************/
#ifndef gUART_TX_THRESHOLD_c 
#define gUART_TX_THRESHOLD_c    4
#endif

/******************************************************************************
 * Name: gUART_RX_THRESHOLD_c
 * Description: UART RX threshold value 
 *****************************************************************************/
#ifndef gUART_RX_THRESHOLD_c 
#define gUART_RX_THRESHOLD_c    1
#endif

/******************************************************************************
 * Name: gUart_PortDefault_d
 * Description: specifies the default used port
 *****************************************************************************/
#ifndef gUart_PortDefault_d
#if (gUart1_Enabled_d == TRUE)
#define gUart_PortDefault_d     1
#elif (gUart2_Enabled_d == TRUE)
#define gUart_PortDefault_d     2
#endif 
#endif

/******************************************************************************
 * Name: gUart1_EnableHWFlowControl_d 
 * Description: Enable/Disable UART_1 hardware flow control 
 *****************************************************************************/
#ifndef gUart1_EnableHWFlowControl_d
#define gUart1_EnableHWFlowControl_d        FALSE
#endif

/******************************************************************************
 * Name: gUart2_EnableHWFlowControl_d 
 * Description: Enable/Disable UART_2 hardware flow control 
 *****************************************************************************/
#ifndef gUart2_EnableHWFlowControl_d
#define gUart2_EnableHWFlowControl_d        FALSE
#endif 

/******************************************************************************
 * Name: gUart1_Hw_Sw_FlowControl_d
 * Description: UART_1 flow control pins controlled software
 *              Used for UART with FIFO not activated 
 *****************************************************************************/
#ifndef gUart1_Hw_Sw_FlowControl_d
#define gUart1_Hw_Sw_FlowControl_d          FALSE
#endif
 
/******************************************************************************
 * Name: gUart2_Hw_Sw_FlowControl_d
 * Description: UART_2 flow control pins controlled software
 *              Used for UART with FIFO not activated 
 *****************************************************************************/

#ifndef gUart2_Hw_Sw_FlowControl_d
#define gUart2_Hw_Sw_FlowControl_d          FALSE
#endif

/******************************************************************************
 * Flow Control configuration check 
 *****************************************************************************/
#if (gUart1_EnableHWFlowControl_d == TRUE) && (gUart1_Hw_Sw_FlowControl_d == TRUE)
#error "Cannot set both software and hardware flow control at the same time"
#endif

#if (gUart2_EnableHWFlowControl_d == TRUE) && (gUart2_Hw_Sw_FlowControl_d == TRUE)
#error "Cannot set both software and hardware flow control at the same time"
#endif

/******************************************************************************
 * Name: gUart_TransmitBuffers_c
 * Description: Number of entries in the transmit-buffers-in-waiting list
 *****************************************************************************/
#ifndef gUart_TransmitBuffers_c
#define gUart_TransmitBuffers_c         3
#endif /* gUart_TransmitBuffers_c */

/******************************************************************************
 * Name: gUart1_ReceiveBufferSize_c
 * Description: Size of the UART_1 driver Rx circular buffer. These buffers are used to
 *              hold received bytes until the application can retrieve them via the
 *              UartX_GetBytesFromRxBuffer() functions, and are not otherwise accessible
 *              from outside the driver. The size DOES NOT need to be a power of two.
 *****************************************************************************/
#ifndef gUart1_ReceiveBufferSize_c
#define gUart1_ReceiveBufferSize_c      250
#endif /* gUart1_ReceiveBufferSize_c */

/******************************************************************************
 * Name: gUart2_ReceiveBufferSize_c
 * Description: Size of the UART_2 driver Rx circular buffer. These buffers are used to
 *              hold received bytes until the application can retrieve them via the
 *              UartX_GetBytesFromRxBuffer() functions, and are not otherwise accessible
 *              from outside the driver. The size DOES NOT need to be a power of two.
 *****************************************************************************/
#ifndef gUart2_ReceiveBufferSize_c
#define gUart2_ReceiveBufferSize_c      250
#endif /* gUart2_ReceiveBufferSize_c */

/******************************************************************************
 * Name: gUart1_RxFlowControlSkew_d
 * Description: UART_1 SKEW Control.
 *              If flow control is used, there is a delay before telling the far side
 *              to stop and the far side actually stopping. When there are SKEW bytes
 *              remaining in the driver's Rx buffer, tell the far side to stop
 *              transmitting.
 *****************************************************************************/
#ifndef gUart1_RxFlowControlSkew_d
#define gUart1_RxFlowControlSkew_d      32
#endif

/******************************************************************************
 * Name: gUart2_RxFlowControlSkew_d
 * Description: UART_2 SKEW Control.
 *              If flow control is used, there is a delay before telling the far side
 *              to stop and the far side actually stopping. When there are SKEW bytes
 *              remaining in the driver's Rx buffer, tell the far side to stop
 *              transmitting.
 *****************************************************************************/
#ifndef gUart2_RxFlowControlSkew_d
#define gUart2_RxFlowControlSkew_d      32
#endif

/******************************************************************************
 * Name: gUart1_RxFlowControlResume_d
 * Description: Number of bytes left in the UART_1 RX buffer when hardware flow control is
 *              deasserted.
 *****************************************************************************/
#ifndef gUart1_RxFlowControlResume_d
#define gUart1_RxFlowControlResume_d    32
#endif

/******************************************************************************
 * Name: gUart2_RxFlowControlResume_d
 * Description: Number of bytes left in the UART_1 RX buffer when hardware flow control is
 *              deasserted.
 *****************************************************************************/
#ifndef gUart2_RxFlowControlResume_d
#define gUart2_RxFlowControlResume_d    32
#endif

/****************************************************************************** 
 * Name: Baud Rates
 * Description: UART standard baud rates definitions
 *****************************************************************************/

#define gUARTBaudRate1200_c	        	1200UL
#define gUARTBaudRate2400_c 	        2400UL
#define gUARTBaudRate4800_c 	        4800UL
#define gUARTBaudRate9600_c 	        9600UL
#define gUARTBaudRate19200_c 	        19200UL
#define gUARTBaudRate38400_c 	        38400UL
#define gUARTBaudRate57600_c 	        57600UL
#define gUARTBaudRate115200_c 	        115200UL
#define gUARTBaudRate230400_c           230400UL

/****************************************************************************** 
 * UART_1 configuration 
 * 
 * gUART1_Parity_c - must be one of the following: gUartParityNone_c, gUartParityEven_c, gUartParityOdd_c
 * gUART1_DataBits_c - must be one of the following: gUart_8bits_c, gUart_9bits_c
 * gUART1_Baudrate_c - any standard or non standard baudrate (bps)
 * gUart1_EnableHWFlowControl_c - must be one of the following: gNoFlowControl_c, gFlowEnabled_c 
 * gUart1_RTS_ActiveHigh_c - must be one of the following: gRtsActiveLow_c, gRtsActiveHi_c
 *****************************************************************************/
#if gUart1_Enabled_d

#ifndef gUART1_Parity_c
#define gUART1_Parity_c                     gUartParityNone_c
#endif

#ifndef gUART1_DataBits_c
#define gUART1_DataBits_c                   gUart_8bits_c
#endif

#ifndef gUART1_Baudrate_c
#define gUART1_Baudrate_c                   gUARTBaudRate115200_c
#endif

#if (gUart1_EnableHWFlowControl_d == TRUE)
#define gUart1_EnableHWFlowControl_c        gFlowEnabled_c   /* DO NOT EDIT */
#else
#define gUart1_EnableHWFlowControl_c        gNoFlowControl_c /* DO NOT EDIT */
#endif

#ifndef gUart1_RTS_ActiveState_c
#define gUart1_RTS_ActiveState_c            gRtsActiveLow_c
#endif

#endif


/******************************************************************************
 * UART_2 configuration 
 * 
 * gUART2_Parity_c - must be one of the following: gUartParityNone_c, gUartParityEven_c, gUartParityOdd_c
 * gUART2_DataBits_c - must be one of the following: gUart_8bits_c, gUart_9bits_c
 * gUART2_Baudrate_c - any standard or non standard baudrate (bps)
 * gUart2_EnableHWFlowControl_c - must be one of the following: gNoFlowControl_c, gFlowEnabled_c 
 * gUart2_RTS_ActiveHigh_c - must be one of the following: gRtsActiveLow_c, gRtsActiveHi_c
 *****************************************************************************/

#if gUart2_Enabled_d

#ifndef gUART2_Parity_c
#define gUART2_Parity_c                     gUartParityNone_c
#endif

#ifndef gUART2_DataBits_c
#define gUART2_DataBits_c                   gUart_8bits_c
#endif

#ifndef gUART2_Baudrate_c
#define gUART2_Baudrate_c                   gUARTBaudRate115200_c
#endif

#if (gUart2_EnableHWFlowControl_d == TRUE)
#define gUart2_EnableHWFlowControl_c        gFlowEnabled_c    /* DO NOT EDIT */
#else
#define gUart2_EnableHWFlowControl_c        gNoFlowControl_c  /* DO NOT EDIT */
#endif

#ifndef gUart2_RTS_ActiveState_c
#define gUart2_RTS_ActiveState_c            gRtsActiveLow_c
#endif

#endif

/******************************************************************************
 * READ ONLY SECTION: THE USER SHALL NOT MODIFY THIS SECTION
 * Registers configuration is done by re-mapping the ports defined in specific
 * target header file 
 ******************************************************************************/

/********** 
 * UART_1 * 
 **********/
#if (gUart1_c == gUART_HW_MOD_0_c)
#define gUART_1_SIM_SCG_c                   gUART0_SIM_SCGC_REG_c           /* UART0 clock gating register */
#define gUART_1_SIM_SCG_BIT_c               gUART0_SIM_SCGC_BIT_c           /* UART0 clock gating bit */
#define gUART_1_PORT_SIM_SCG_c              gUART0_PORT_SIM_SCGC_REG_c      /* UART0 port clock gating register */
#define gUART_1_PORT_SIM_SGC_BIT_c          gUART0_PORT_SIM_SGC_BIT_c       /* UART0 port clock gating bit*/
#define gUART_1_RX_PCR_c                    gUART0_RX_PCR_REG_c             /* UART0 RX Pin Control register*/
#define gUART_1_TX_PCR_c                    gUART0_TX_PCR_REG_c             /* UART0 TX Pin Control register */
#define gUART_1_RTS_PCR_c                   gUART0_RTS_PCR_REG_c            /* UART0 RTS Pin Control register */
#define gUART_1_CTS_PCR_c                   gUART0_CTS_PCR_REG_c            /* UART0 CTS Pin Control register*/
#define gUART_1_MUX_c                       gUART0_PORT_MUX_c               /* UART0 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_1_IRQ_NUM_c                   gUART0_IRQ_NUM_c                /* UART0 interrupt source number */
#define gUART_1_ERR_IRQ_NUM_c               gUART0_ERR_IRQ_NUM_c            /* UART0 error interrupt source number */
#elif (gUart1_c == gUART_HW_MOD_1_c)
#define gUART_1_SIM_SCG_c                   gUART1_SIM_SCGC_REG_c           /* UART1 clock gating register */
#define gUART_1_SIM_SCG_BIT_c               gUART1_SIM_SCGC_BIT_c           /* UART1 clock gating bit */
#define gUART_1_PORT_SIM_SCG_c              gUART1_PORT_SIM_SCGC_REG_c      /* UART1 port clock gating register */
#define gUART_1_PORT_SIM_SGC_BIT_c          gUART1_PORT_SIM_SGC_BIT_c       /* UART1 port clock gating bit*/
#define gUART_1_RX_PCR_c                    gUART1_RX_PCR_REG_c             /* UART1 RX Pin Control register*/
#define gUART_1_TX_PCR_c                    gUART1_TX_PCR_REG_c             /* UART1 TX Pin Control register */
#define gUART_1_RTS_PCR_c                   gUART1_RTS_PCR_REG_c            /* UART1 RTS Pin Control register */
#define gUART_1_CTS_PCR_c                   gUART1_CTS_PCR_REG_c            /* UART1 CTS Pin Control register*/
#define gUART_1_MUX_c                       gUART1_PORT_MUX_c               /* UART1 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_1_IRQ_NUM_c                   gUART1_IRQ_NUM_c                /* UART1 interrupt source number */
#define gUART_1_ERR_IRQ_NUM_c               gUART1_ERR_IRQ_NUM_c            /* UART1 error interrupt source number */
#elif (gUart1_c == gUART_HW_MOD_2_c)
#define gUART_1_SIM_SCG_c                   gUART2_SIM_SCGC_REG_c           /* UART2 clock gating register */
#define gUART_1_SIM_SCG_BIT_c               gUART2_SIM_SCGC_BIT_c           /* UART2 clock gating bit */
#define gUART_1_PORT_SIM_SCG_c              gUART2_PORT_SIM_SCGC_REG_c      /* UART2 port clock gating register */
#define gUART_1_PORT_SIM_SGC_BIT_c          gUART2_PORT_SIM_SGC_BIT_c       /* UART2 port clock gating bit*/
#define gUART_1_RX_PCR_c                    gUART2_RX_PCR_REG_c             /* UART2 RX Pin Control register*/
#define gUART_1_TX_PCR_c                    gUART2_TX_PCR_REG_c             /* UART2 TX Pin Control register*/
#define gUART_1_RTS_PCR_c                   gUART2_RTS_PCR_REG_c            /* UART2 RTS Pin Control register */
#define gUART_1_CTS_PCR_c                   gUART2_CTS_PCR_REG_c            /* UART2 CTS Pin Control register */
#define gUART_1_MUX_c                       gUART2_PORT_MUX_c               /* UART2 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_1_IRQ_NUM_c                   gUART2_IRQ_NUM_c                /* UART2 interrupt source number */
#define gUART_1_ERR_IRQ_NUM_c               gUART2_ERR_IRQ_NUM_c            /* UART2 error interrupt source number */
#elif (gUart1_c == gUART_HW_MOD_3_c)
#define gUART_1_SIM_SCG_c                   gUART3_SIM_SCGC_REG_c           /* UART3 clock gating register */
#define gUART_1_SIM_SCG_BIT_c               gUART3_SIM_SCGC_BIT_c           /* UART3 clock gating bit */
#define gUART_1_PORT_SIM_SCG_c              gUART3_PORT_SIM_SCGC_REG_c      /* UART3 port clock gating register */
#define gUART_1_PORT_SIM_SGC_BIT_c          gUART3_PORT_SIM_SGC_BIT_c       /* UART3 port clock gating bit*/
#define gUART_1_RX_PCR_c                    gUART3_RX_PCR_REG_c             /* UART3 RX Pin Control register*/
#define gUART_1_TX_PCR_c                    gUART3_TX_PCR_REG_c             /* UART3 TX Pin Control register*/
#define gUART_1_RTS_PCR_c                   gUART3_RTS_PCR_REG_c            /* UART3 RTS Pin Control register */
#define gUART_1_CTS_PCR_c                   gUART3_CTS_PCR_REG_c            /* UART3 CTS Pin Control register */
#define gUART_1_MUX_c                       gUART3_PORT_MUX_c               /* UART3 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_1_IRQ_NUM_c                   gUART3_IRQ_NUM_c                /* UART3 interrupt source number */
#define gUART_1_ERR_IRQ_NUM_c               gUART3_ERR_IRQ_NUM_c            /* UART3 error interrupt source number */
#elif (gUart1_c == gUART_HW_MOD_4_c)
#define gUART_1_SIM_SCG_c                   gUART4_SIM_SCGC_REG_c           /* UART4 clock gating register */
#define gUART_1_SIM_SCG_BIT_c               gUART4_SIM_SCGC_BIT_c           /* UART4 clock gating bit */
#define gUART_1_PORT_SIM_SCG_c              gUART4_PORT_SIM_SCGC_REG_c      /* UART4 port clock gating register */
#define gUART_1_PORT_SIM_SGC_BIT_c          gUART4_PORT_SIM_SGC_BIT_c       /* UART4 port clock gating bit*/
#define gUART_1_RX_PCR_c                    gUART4_RX_PCR_REG_c             /* UART4 RX Pin Control register*/
#define gUART_1_TX_PCR_c                    gUART4_TX_PCR_REG_c             /* UART4 TX Pin Control register*/
#define gUART_1_RTS_PCR_c                   gUART4_RTS_PCR_REG_c            /* UART4 RTS Pin Control register */
#define gUART_1_CTS_PCR_c                   gUART4_CTS_PCR_REG_c            /* UART4 CTS Pin Control register */
#define gUART_1_MUX_c                       gUART4_PORT_MUX_c               /* UART4 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_1_IRQ_NUM_c                   gUART4_IRQ_NUM_c                /* UART4 interrupt source number */
#define gUART_1_ERR_IRQ_NUM_c               gUART4_ERR_IRQ_NUM_c            /* UART4 error interrupt source number */
#else
#define gUART_1_SIM_SCG_c                   gUART5_SIM_SCGC_REG_c           /* UART5 clock gating register */
#define gUART_1_SIM_SCG_BIT_c               gUART5_SIM_SCGC_BIT_c           /* UART5 clock gating bit */
#define gUART_1_PORT_SIM_SCG_c              gUART5_PORT_SIM_SCGC_REG_c      /* UART5 port clock gating register */
#define gUART_1_PORT_SIM_SGC_BIT_c          gUART5_PORT_SIM_SGC_BIT_c       /* UART5 port clock gating bit*/
#define gUART_1_RX_PCR_c                    gUART5_RX_PCR_REG_c             /* UART5 RX Pin Control register*/
#define gUART_1_TX_PCR_c                    gUART5_TX_PCR_REG_c             /* UART5 TX Pin Control register*/
#define gUART_1_RTS_PCR_c                   gUART5_RTS_PCR_REG_c            /* UART5 RTS Pin Control register */
#define gUART_1_CTS_PCR_c                   gUART5_CTS_PCR_REG_c            /* UART5 CTS Pin Control register */
#define gUART_1_MUX_c                       gUART5_PORT_MUX_c               /* UART5 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_1_IRQ_NUM_c                   gUART5_IRQ_NUM_c                /* UART5 interrupt source number */
#define gUART_1_ERR_IRQ_NUM_c               gUART5_ERR_IRQ_NUM_c            /* UART5 error interrupt source number */
#endif

/********** 
 * UART_2 * 
 **********/
#if (gUart2_c == gUART_HW_MOD_0_c)
#define gUART_2_SIM_SCG_c                   gUART0_SIM_SCGC_REG_c           /* UART0 clock gating register */
#define gUART_2_SIM_SCG_BIT_c               gUART0_SIM_SCGC_BIT_c           /* UART0 clock gating bit */
#define gUART_2_PORT_SIM_SCG_c              gUART0_PORT_SIM_SCGC_REG_c      /* UART0 port clock gating register */
#define gUART_2_PORT_SIM_SGC_BIT_c          gUART0_PORT_SIM_SGC_BIT_c       /* UART0 port clock gating bit*/
#define gUART_2_RX_PCR_c                    gUART0_RX_PCR_REG_c             /* UART0 RX Pin Control register*/
#define gUART_2_TX_PCR_c                    gUART0_TX_PCR_REG_c             /* UART0 TX Pin Control register */
#define gUART_2_RTS_PCR_c                   gUART0_RTS_PCR_REG_c            /* UART0 RTS Pin Control register */
#define gUART_2_CTS_PCR_c                   gUART0_CTS_PCR_REG_c            /* UART0 CTS Pin Control register*/
#define gUART_2_MUX_c                       gUART0_PORT_MUX_c               /* UART0 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_2_IRQ_NUM_c                   gUART0_IRQ_NUM_c                /* UART0 interrupt source number */
#define gUART_2_ERR_IRQ_NUM_c               gUART0_ERR_IRQ_NUM_c            /* UART0 error interrupt source number */
#elif (gUart2_c == gUART_HW_MOD_1_c)
#define gUART_2_SIM_SCG_c                   gUART1_SIM_SCGC_REG_c           /* UART1 clock gating register */
#define gUART_2_SIM_SCG_BIT_c               gUART1_SIM_SCGC_BIT_c           /* UART1 clock gating bit */
#define gUART_2_PORT_SIM_SCG_c              gUART1_PORT_SIM_SCGC_REG_c      /* UART1 port clock gating register */
#define gUART_2_PORT_SIM_SGC_BIT_c          gUART1_PORT_SIM_SGC_BIT_c       /* UART1 port clock gating bit*/
#define gUART_2_RX_PCR_c                    gUART1_RX_PCR_REG_c             /* UART1 RX Pin Control register*/
#define gUART_2_TX_PCR_c                    gUART1_TX_PCR_REG_c             /* UART1 TX Pin Control register */
#define gUART_2_RTS_PCR_c                   gUART1_RTS_PCR_REG_c            /* UART1 RTS Pin Control register */
#define gUART_2_CTS_PCR_c                   gUART1_CTS_PCR_REG_c            /* UART1 CTS Pin Control register*/
#define gUART_2_MUX_c                       gUART1_PORT_MUX_c               /* UART1 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_2_IRQ_NUM_c                   gUART1_IRQ_NUM_c                /* UART1 interrupt source number */
#define gUART_2_ERR_IRQ_NUM_c               gUART1_ERR_IRQ_NUM_c            /* UART1 error interrupt source number */
#elif (gUart2_c == gUART_HW_MOD_2_c)
#define gUART_2_SIM_SCG_c                   gUART2_SIM_SCGC_REG_c           /* UART2 clock gating register */
#define gUART_2_SIM_SCG_BIT_c               gUART2_SIM_SCGC_BIT_c           /* UART2 clock gating bit */
#define gUART_2_PORT_SIM_SCG_c              gUART2_PORT_SIM_SCGC_REG_c      /* UART2 port clock gating register */
#define gUART_2_PORT_SIM_SGC_BIT_c          gUART2_PORT_SIM_SGC_BIT_c       /* UART2 port clock gating bit*/
#define gUART_2_RX_PCR_c                    gUART2_RX_PCR_REG_c             /* UART2 RX Pin Control register*/
#define gUART_2_TX_PCR_c                    gUART2_TX_PCR_REG_c             /* UART2 TX Pin Control register*/
#define gUART_2_RTS_PCR_c                   gUART2_RTS_PCR_REG_c            /* UART2 RTS Pin Control register */
#define gUART_2_CTS_PCR_c                   gUART2_CTS_PCR_REG_c            /* UART2 CTS Pin Control register */
#define gUART_2_MUX_c                       gUART2_PORT_MUX_c               /* UART2 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_2_IRQ_NUM_c                   gUART2_IRQ_NUM_c                /* UART2 interrupt source number */
#define gUART_2_ERR_IRQ_NUM_c               gUART2_ERR_IRQ_NUM_c            /* UART2 error interrupt source number */
#elif (gUart2_c == gUART_HW_MOD_3_c)
#define gUART_2_SIM_SCG_c                   gUART3_SIM_SCGC_REG_c           /* UART3 clock gating register */
#define gUART_2_SIM_SCG_BIT_c               gUART3_SIM_SCGC_BIT_c           /* UART3 clock gating bit */
#define gUART_2_PORT_SIM_SCG_c              gUART3_PORT_SIM_SCGC_REG_c      /* UART3 port clock gating register */
#define gUART_2_PORT_SIM_SGC_BIT_c          gUART3_PORT_SIM_SGC_BIT_c       /* UART3 port clock gating bit*/
#define gUART_2_RX_PCR_c                    gUART3_RX_PCR_REG_c             /* UART3 RX Pin Control register*/
#define gUART_2_TX_PCR_c                    gUART3_TX_PCR_REG_c             /* UART3 TX Pin Control register*/
#define gUART_2_RTS_PCR_c                   gUART3_RTS_PCR_REG_c            /* UART3 RTS Pin Control register */
#define gUART_2_CTS_PCR_c                   gUART3_CTS_PCR_REG_c            /* UART3 CTS Pin Control register */
#define gUART_2_MUX_c                       gUART3_PORT_MUX_c               /* UART3 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_2_IRQ_NUM_c                   gUART3_IRQ_NUM_c                /* UART3 interrupt source number */
#define gUART_2_ERR_IRQ_NUM_c               gUART3_ERR_IRQ_NUM_c            /* UART3 error interrupt source number */
#elif (gUart2_c == gUART_HW_MOD_4_c)
#define gUART_2_SIM_SCG_c                   gUART4_SIM_SCGC_REG_c           /* UART4 clock gating register */
#define gUART_2_SIM_SCG_BIT_c               gUART4_SIM_SCGC_BIT_c           /* UART4 clock gating bit */
#define gUART_2_PORT_SIM_SCG_c              gUART4_PORT_SIM_SCGC_REG_c      /* UART4 port clock gating register */
#define gUART_2_PORT_SIM_SGC_BIT_c          gUART4_PORT_SIM_SGC_BIT_c       /* UART4 port clock gating bit*/
#define gUART_2_RX_PCR_c                    gUART4_RX_PCR_REG_c             /* UART4 RX Pin Control register*/
#define gUART_2_TX_PCR_c                    gUART4_TX_PCR_REG_c             /* UART4 TX Pin Control register*/
#define gUART_2_RTS_PCR_c                   gUART4_RTS_PCR_REG_c            /* UART4 RTS Pin Control register */
#define gUART_2_CTS_PCR_c                   gUART4_CTS_PCR_REG_c            /* UART4 CTS Pin Control register */
#define gUART_2_MUX_c                       gUART4_PORT_MUX_c               /* UART4 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_2_IRQ_NUM_c                   gUART4_IRQ_NUM_c                /* UART4 interrupt source number */
#define gUART_2_ERR_IRQ_NUM_c               gUART4_ERR_IRQ_NUM_c            /* UART4 error interrupt source number */
#else
#define gUART_2_SIM_SCG_c                   gUART5_SIM_SCGC_REG_c           /* UART5 clock gating register */
#define gUART_2_SIM_SCG_BIT_c               gUART5_SIM_SCGC_BIT_c           /* UART5 clock gating bit */
#define gUART_2_PORT_SIM_SCG_c              gUART5_PORT_SIM_SCGC_REG_c      /* UART5 port clock gating register */
#define gUART_2_PORT_SIM_SGC_BIT_c          gUART5_PORT_SIM_SGC_BIT_c       /* UART5 port clock gating bit*/
#define gUART_2_RX_PCR_c                    gUART5_RX_PCR_REG_c             /* UART5 RX Pin Control register*/
#define gUART_2_TX_PCR_c                    gUART5_TX_PCR_REG_c             /* UART5 TX Pin Control register*/
#define gUART_2_RTS_PCR_c                   gUART5_RTS_PCR_REG_c            /* UART5 RTS Pin Control register */
#define gUART_2_CTS_PCR_c                   gUART5_CTS_PCR_REG_c            /* UART5 CTS Pin Control register */
#define gUART_2_MUX_c                       gUART5_PORT_MUX_c               /* UART5 RX, TX, RTS and CTS pins alternate function (see MUX) */
#define gUART_2_IRQ_NUM_c                   gUART5_IRQ_NUM_c                /* UART5 interrupt source number */
#define gUART_2_ERR_IRQ_NUM_c               gUART5_ERR_IRQ_NUM_c            /* UART5 error interrupt source number */
#endif

/* END OF SECTION: READ ONLY */

/******************************************************************************
 * Name: see below
 * Description: UART Software Flow Control ports/pins mapping
 *****************************************************************************/

#define gUART_1_SW_RTS_PDDR_c           gUART1_SW_RTS_PDDR_c      /* GPIO port data direction register */
#define gUART_1_SW_RTS_PDOR_c           gUART1_SW_RTS_PDOR_c      /* GPIO port data output register */
#define gUART_1_SW_RTS_PSOR_c           gUART1_SW_RTS_PSOR_c      /* GPIO port set output register */
#define gUART_1_SW_RTS_PCOR_c           gUART1_SW_RTS_PCOR_c      /* GPIO port clear output register */
#define gUART_1_SW_RTS_PIN_c            gUART1_SW_RTS_PIN_MASK_c  /* the corresponding pin in the above registers */

#define gUART_1_SW_CTS_PDDR_c           gUART1_SW_CTS_PDDR_c      /* GPIO port data direction register */
#define gUART_1_SW_CTS_PDIR_c           gUART1_SW_CTS_PDIR_c      /* GPIO port input register */
#define gUART_1_SW_CTS_PIN_c            gUART1_SW_CTS_PIN_MASK_c  /* the corresponding pin in the above registers */

#define gUART_2_SW_RTS_PDDR_c           gUART2_SW_RTS_PDDR_c      /* GPIO port data direction register */
#define gUART_2_SW_RTS_PDOR_c           gUART2_SW_RTS_PDOR_c      /* GPIO port data output register */
#define gUART_2_SW_RTS_PSOR_c           gUART2_SW_RTS_PSOR_c      /* GPIO port set output register */ 
#define gUART_2_SW_RTS_PCOR_c           gUART2_SW_RTS_PCOR_c      /* GPIO port clear output register */
#define gUART_2_SW_RTS_PIN_c            gUART2_SW_RTS_PIN_MASK_c  /* the corresponding pin in the above registers */

#define gUART_2_SW_CTS_PDDR_c           gUART2_SW_CTS_PDDR_c      /* GPIO port data direction register */
#define gUART_2_SW_CTS_PDIR_c           gUART2_SW_CTS_PDIR_c      /* GPIO port input register */
#define gUART_2_SW_CTS_PIN_c            gUART2_SW_CTS_PIN_MASK_c  /* the corresponding pin in the above registers */

#define gUART_1_SW_CTS_SIM_SCG_c        gUART1_SW_CTS_SIM_SCGC_REG_c  /* clock gating register for the port used by the CTS signal */
#define gUART_1_SW_CTS_SIM_SCG_BIT_c    gUART1_SW_CTS_SIM_SCGC_BIT_c  /* bit position in the above register (clock gating register used by CTS) */

#define gUART_1_SW_RTS_SIM_SCG_c        gUART1_SW_RTS_SIM_SCGC_REG_c  /* clock gating register for the port used by the CTS signal */
#define gUART_1_SW_RTS_SIM_SCG_BIT_c    gUART1_SW_RTS_SIM_SCGC_BIT_c  /* bit position in the above register (clock gating register used by CTS) */

#define gUART_2_SW_CTS_SIM_SCG_c        gUART2_SW_CTS_SIM_SCGC_REG_c  /* clock gating register for the port used by the CTS signal */
#define gUART_2_SW_CTS_SIM_SCG_BIT_c    gUART2_SW_CTS_SIM_SCGC_BIT_c  /* bit position in the above register (clock gating register used by CTS) */

#define gUART_2_SW_RTS_SIM_SCG_c        gUART2_SW_RTS_SIM_SCGC_REG_c  /* clock gating register for the port used by the CTS signal */
#define gUART_2_SW_RTS_SIM_SCG_BIT_c    gUART2_SW_RTS_SIM_SCGC_BIT_c  /* bit position in the above register (clock gating register used by CTS) */

#define gUART_1_SW_CTS_PCR_c            gUART1_SW_CTS_PCR_REG_c   /* UART1 CTS Pin Control Register */
#define gUART_1_SW_CTS_MUX_c            gUART1_SW_CTS_MUX_c       /* UART1 CTS pin alternate function (MUX) */
#define gUART_1_SW_RTS_PCR_c            gUART1_SW_RTS_PCR_REG_c	  /* UART1 RTS Pin Control Register */
#define gUART_1_SW_RTS_MUX_c            gUART1_SW_RTS_MUX_c       /* UART1 RTS pin alternate function (MUX) */

#define gUART_2_SW_CTS_PCR_c            gUART2_SW_CTS_PCR_REG_c   /* UART2 CTS Pin Control Register */
#define gUART_2_SW_CTS_MUX_c            gUART2_SW_CTS_MUX_c       /* UART2 CTS pin alternate function (MUX) */
#define gUART_2_SW_RTS_PCR_c            gUART2_SW_RTS_PCR_REG_c	  /* UART2 RTS Pin Control Register */
#define gUART_2_SW_RTS_MUX_c            gUART2_SW_RTS_MUX_c       /* UART2 RTS pin alternate function (MUX) */


/******************************************************************************
 * Name: gUart1_InterruptPriority_c
 * Description: UART_1 IRQ priority 
 *****************************************************************************/

#define gUart1_InterruptPriority_c          (0x9u) 

/******************************************************************************
 * Name: gUart2_InterruptPriority_c
 * Description: UART_2 IRQ priority
 *****************************************************************************/

#define gUart2_InterruptPriority_c          (0x9u)

/******************************************************************************
 * Name: gUart1_ErrInterruptPriority_c
 * Description: UART_1 Error IRQ priority
 *****************************************************************************/

#define gUart1_ErrInterruptPriority_c       (0x8u) 

/******************************************************************************
 * Name: gUart2_ErrInterruptPriority_c
 * Description: UART_2 Error IRQ priority
 *****************************************************************************/

#define gUart2_ErrInterruptPriority_c       (0x8u)

/******************************************************************************
*******************************************************************************
* Public type definitions
*******************************************************************************
******************************************************************************/

/******************************************************************************
 * Name: UartBaudRate_t
 * Description: data type definition for UART baudrate
 *****************************************************************************/ 

typedef uint16_t UartBaudRate_t;

/******************************************************************************
 * Name: UartParityMode_t
 * Description: This data type enumerates the possible values of UART parity modes
 *****************************************************************************/ 

typedef enum UartParity_tag 
{
	gUartParityNone_c = 0,
	gUartParityEven_c,
	gUartParityOdd_c,
	gUartParityMax_c
} UartParityMode_t;

/******************************************************************************
 * Name: UartDataBits_t
 * Description: This data type enumerates the possible values of UART data bits 
 *****************************************************************************/
typedef enum UartDataBits_tag 
{
	gUart_8bits_c = 0,
	gUart_9bits_c,
	gUartDataBitsMax_c
} UartDataBits_t;

typedef enum UartHwFlowControl_tag
{
	gNoFlowControl_c = 0,
	gFlowEnabled_c
} UartHwFlowControl_t;

typedef enum UartRTSActiveState_tag
{
	gRtsActiveLow_c = 0,
	gRtsActiveHi_c
} UartRTSActiveState_t;

/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/

#if (gUart1_Enabled_d == FALSE) && (gUart2_Enabled_d == FALSE)

/******************************************************************************
 * Name: see section below
 * Description: stub functions if both UART ports are disabled 
 *****************************************************************************/

#define Uart_ModuleInit()
#define Uart_ModuleUninit()
#define Uart_ClearErrors()
#define Uart1_SetBaud(baudRate)
#define Uart2_SetBaud(baudRate)
#define Uart1_Transmit(pBuf, bufLen, pfCallBack)    FALSE
#define Uart2_Transmit(pBuf, bufLen, pfCallBack)    FALSE
#define Uart1_IsTxActive()                          FALSE
#define Uart2_IsTxActive()                          FALSE
#define Uart1_SetRxCallBack(pfCallBack)
#define Uart2_SetRxCallBack(pfCallBack)
#define Uart1_GetByteFromRxBuffer(pDst)             FALSE
#define Uart2_GetByteFromRxBuffer(pDst)             FALSE
#define Uart1_UngetByte(byte)
#define Uart2_UngetByte(byte)
#define Uart1_RxBufferByteCount                     0
#define Uart2_RxBufferByteCount                     0

#else /* (gUart1_Enabled_d == FALSE) && (gUart2_Enabled_d == FALSE) */

/******************************************************************************
* Uart_ModuleInit initializes the serial port(s) and module internal variables
*     
* Interface assumptions:
*   It should be called before any the other driver function. It needs to be
*   called first before use UART driver.
*
* return value:
*   NONE
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/

extern void Uart_ModuleInit
(
    void
);

/******************************************************************************
* Uart_ModuleUnInit shut down the UART module by clearing the SCI Control 
* registers. 
*     
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart_ModuleUninit
(
    void
);

/******************************************************************************
* Uart_ClearErrors clears any error flags seen by the UART driver. In rare 
* conditions, an error flag can be set without triggering an error interrupt, 
* and will prevent Rx and/or Tx interrupts from occurring. The most likely cause 
* is a breakpoint set during debugging, when a UART port is active.     
* Calling this function occasionally allows the UART code to recover    
* from these errors. 
*     
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart_ClearErrors
(
    void
);

/******************************************************************************
* Uart1_GetByteFromRxBuffer Retrieve one byte from 
* the driver's Rx buffer and store it at *pDst.
*     
* Interface assumptions:
*   NONE
*
* return value:
*   TRUE -  If a byte was retrieved
*   FALSE - If the Rx buffer is empty.
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern bool_t Uart1_GetByteFromRxBuffer
(
    unsigned char *pDst
);

/******************************************************************************
* Uart2_GetByteFromRxBuffer Retrieve one byte from 
* the driver's Rx buffer and store it at *pDst.
*     
* Interface assumptions:
*   NONE
*
* return value:
*   TRUE -  If a byte was retrieved
*   FALSE - If the Rx buffer is empty.
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern bool_t Uart2_GetByteFromRxBuffer
(
    unsigned char *pDst
);

/******************************************************************************
* Uart1_IsTxActive checks if a transmission is active.
*     
*
* Interface assumptions:
*   NONE
*
* return value:
*   TRUE - If there are any bytes in the transmit buffers that have not be sent.
*   FALSE - If there are no bytes to be sent.
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern bool_t Uart1_IsTxActive
(
    void
);

/******************************************************************************
* Uart2_IsTxActive checks if a transmission is active.
*     
*
* Interface assumptions:
*   NONE
*
* return value:
*   TRUE - If there are any bytes in the transmit buffers that have not be sent.
*   FALSE - If there are no bytes to be sent.
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern bool_t Uart2_IsTxActive
(
    void
);

/******************************************************************************
* Uart1_SetRxCallBack sets the receive side callback function. 
* This function will be called whenever a byte is received by the driver. 
* The callback function will be called in interrupt context, so it should be 
* kept very short. If the callback pointer is set to NULL, the Rx interrupt 
* will be turned off, and all data in the driver's Rx buffer will be discarded. 
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart1_SetRxCallBack
(
    void (*pfCallBack)(void)  /*IN: Pointer to that callback function to be executed 
                              when any of the receiving conditions is reached*/
);

/******************************************************************************
* Uart2_SetRxCallBack sets the receive side callback function. 
* This function will be called whenever a byte is received by the driver. 
* The callback function will be called in interrupt context, so it should be 
* kept very short. If the callback pointer is set to NULL, the Rx interrupt 
* will be turned off, and all data in the driver's Rx buffer will be discarded. 
*
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart2_SetRxCallBack
(
    void (*pfCallBack)(void)  /*IN: Pointer to that callback function to be executed 
                              when any of the receiving conditions is reached*/
);

/******************************************************************************
* Uart1_SetBaud set the selected baud rate specified in the given parameter.
*     
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart1_SetBaud
(
    uint32_t baudRate    /* IN: Selected baud rate */
);

/******************************************************************************
* Uart2_SetBaud set the selected baud rate specified in the given parameter.
*     
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart2_SetBaud
(
	uint32_t baudRate   /* IN: Selected baud rate */
);

/******************************************************************************
* Uart1_Transmit Transmits bufLen bytes of data from pBuffer 
* over a port. Call *pfCallBack() when the entire buffer has been sent.  
* The caller must ensure that the buffer remains available until the call back 
* function is called. pfCallBack must not be NULL.  This function is executed 
* as part of an interrupt service routine, as a result follow all the 
* recommendations for interrupt execution timing and context
*
* Interface assumptions:
*   NONE
*
* return value:
*   FALSE - If there are no more available Tx buffer slots.
*   TRUE - If there are slots to transmit.
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern bool_t Uart1_Transmit
(
    unsigned char const *pBuf,                      /* IN: Pointer to the buffer that is required to be 
                                                    transmitted */
    index_t bufLen,                                 /* IN: Number of bytes to be transmitted */
    void (*pfCallBack)(unsigned char const *pBuf)   /*IN: Pointer to that callback function to be executed 
					            when the entire buffer has been sent*/
);

/******************************************************************************
* Uart2_Transmit Transmits bufLen bytes of data from pBuffer 
* over a port. Call *pfCallBack() when the entire buffer has been sent.  
* The caller must ensure that the buffer remains available until the call back 
* function is called. pfCallBack must not be NULL.  This function is executed 
* as part of an interrupt service routine, as a result follow all the 
* recommendations for interrupt execution timing and context
*
* Interface assumptions:
*   NONE
*
* return value:
*   FALSE - If there are no more available Tx buffer slots.
*   TRUE - If there are slots to transmit.
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern bool_t Uart2_Transmit
(
    unsigned char const *pBuf,                      /* IN: Pointer to the buffer that is required to be 
                                                    transmitted */
    index_t bufLen,                                 /* IN: Number of bytes to be transmitted */
    void (*pfCallBack)(unsigned char const *pBuf)   /*IN: Pointer to that callback function to be executed 
					            when the entire buffer has been sent*/
);                                

/******************************************************************************
* Uart1_UngetByte If a caller reads  many 
* characters, it can unget it. Calling the unget routine will store 
* the byte to be read by the next call to Uart1_GetByteFromRxBuffer(), 
* and will also call the Rx call back. 
*     
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart1_UngetByte
(
    unsigned char byte    /* IN: Unget byte to be read by 
		          the Uart1_GetByteFromRxBuffer() in its next call*/
);

/******************************************************************************
* Uart2_UngetByte If a caller reads  many 
* characters, it can unget it. Calling the unget routine will store 
* the byte to be read by the next call to Uart1_GetByteFromRxBuffer(), 
* and will also call the Rx call back. 
*     
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart2_UngetByte
(
    unsigned char byte    /* IN: Unget byte to be read by 
			  the Uart2_GetByteFromRxBuffer() in its next call*/
);

/******************************************************************************
* Uart1_RxBufferByteCount returns unread bytes number from UART_1 buffer 
*     
* Interface assumptions:
*   NONE
*
* return value:
*   uint32_t - the number of unread bytes
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern uint32_t Uart1_RxBufferByteCount
(
    void
); 

/******************************************************************************
* Uart2_RxBufferByteCount returns unread bytes number from UART_1 buffer 
*     
* Interface assumptions:
*   NONE
*
* return value:
*   uint32_t - the number of unread bytes
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern uint32_t Uart2_RxBufferByteCount
(
    void
);


/******************************************************************************
* Uart1Isr is the UART_1 Interrupt Service Routine
*     
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart1Isr
(
    void
);

/******************************************************************************
* Uart2Isr is the UART_2 Interrupt Service Routine
*     
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart2Isr
(
    void
);

/******************************************************************************
* Uart1ErrorIsr clears enabled UART_1 errors
*     
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart1ErrorIsr
(
    void
); 

/******************************************************************************
* Uart2ErrorIsr clears enabled UART_2 errors
*     
* Interface assumptions:
*   NONE
*
* return value:
*   NONE
*   
* Effects on global data:
*   NONE
*
*******************************************************************************/
extern void Uart2ErrorIsr
(
    void
);

/*---------------------------------------------------------------------------
 * Name: Uart_ModuleEnableLowPowerWakeup
 * Description: Enables the Receive input active edge, RXEDGIF, to generate interrupt requests.
 *              The receive input active edge detect circuit is still active on low power modes
 *              (wait and stop). An active edge on the receive input brings the CPU out of low 
 *              power mode if the interrupt is not masked.
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
extern void Uart_ModuleEnableLowPowerWakeup
(
    void
);

/*---------------------------------------------------------------------------
 * Name: Uart_ModuleDisableLowPowerWakeup
 * Description: Disables the Receive input active edge, RXEDGIF, to generate interrupt requests.
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
extern void Uart_ModuleDisableLowPowerWakeup
(
    void
);

/*---------------------------------------------------------------------------
 * Name: Uart_IsWakeUpSource
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
extern bool_t Uart_IsWakeUpSource
(
    void
);

/****************************************************************************** 
 * Declare the default API. 
 * If gUart_PortDefault_d is not defined, there is 
 * no default API. 
 *****************************************************************************/
 
#if gUart_PortDefault_d == 1
#define UartX_Transmit              Uart1_Transmit  
#define UartX_IsTxActive            Uart1_IsTxActive
#define UartX_SetRxCallBack         Uart1_SetRxCallBack
#define UartX_GetByteFromRxBuffer   Uart1_GetByteFromRxBuffer
#define UartX_SetBaud               Uart1_SetBaud
#define UartX_UngetByte             Uart1_UngetByte
#define UartX_TxCompleteFlag        Uart1_TxCompleteFlag
#define UartX_RxBufferByteCount     Uart1_RxBufferByteCount
#define UartX_DefaultBaud           gUART1_Baudrate_c
#endif

#if gUart_PortDefault_d == 2
#define UartX_Transmit              Uart2_Transmit
#define UartX_IsTxActive            Uart2_IsTxActive
#define UartX_SetRxCallBack         Uart2_SetRxCallBack
#define UartX_GetByteFromRxBuffer   Uart2_GetByteFromRxBuffer
#define UartX_SetBaud               Uart2_SetBaud
#define UartX_UngetByte             Uart2_UngetByte
#define UartX_TxCompleteFlag        Uart2_TxCompleteFlag
#define UartX_RxBufferByteCount     Uart2_RxBufferByteCount
#define UartX_DefaultBaud           gUART2_Baudrate_c
#endif

#endif /* (gUart1_Enabled_d == FALSE) && (gUart2_Enabled_d == FALSE) */

#endif  /* gUARTInterface_d*/
#endif /* UART_INTERFACE_H_ */
