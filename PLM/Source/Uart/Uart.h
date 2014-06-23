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
 
#ifndef __UART_H__
#define __UART_H__

/******************************************************************************
*******************************************************************************
* Includes
*******************************************************************************
******************************************************************************/
 
#include "UART_Interface.h"
#ifdef gUARTInterface_d
/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/

 
/******************************************************************************
 * Name: NumberOfElements()
 * Description: Number of elements in an array
 *****************************************************************************/
#ifndef NumberOfElements
#define NumberOfElements(array)     ((sizeof(array) / (sizeof(array[0]))))
#endif

/******************************************************************************
 * Name: see below
 * Description: Software Flow Control macros
 *****************************************************************************/
#if gUart1_Hw_Sw_FlowControl_d
  #define  Uart1_FlowConflict()  (gUART_1_SW_CTS_PDIR_c &  gUART_1_SW_CTS_PIN_c)
  #define  Uart1_DisableFlow()   (gUART_1_SW_RTS_PSOR_c |= gUART_1_SW_RTS_PIN_c)
  #define  Uart1_EnableFlow()    (gUART_1_SW_RTS_PCOR_c |= gUART_1_SW_RTS_PIN_c)
#endif

#if gUart2_Hw_Sw_FlowControl_d
  #define  Uart2_FlowConflict()   (gUART_2_SW_CTS_PDIR_c &  gUART_2_SW_CTS_PIN_c)
  #define  Uart2_DisableFlow()    (gUART_2_SW_RTS_PSOR_c |= gUART_2_SW_RTS_PIN_c)
  #define  Uart2_EnableFlow()     (gUART_2_SW_RTS_PCOR_c |= gUART_2_SW_RTS_PIN_c)
#endif

/*****************************************************************************
******************************************************************************
* Private type definitions
******************************************************************************
*****************************************************************************/


/******************************************************************************
 * Name: UartReadStatus_t
 * Description: This data type enumerates the possible read operation status
 *****************************************************************************/
typedef enum 
{
    gUartReadStatusComplete_c = 0,
    gUartReadStatusCanceled_c ,
    gUartReadStatusError_c,
    gUartReadStatusMax_c
} UartReadStatus_t;

/******************************************************************************
 * Name: UartErr_t
 * Description: This data type enumerates the UART API calls return values 
 *****************************************************************************/
typedef enum {
    gUartErrNoError_c = 0,                 
    gUartErrUartAlreadyOpen_c,                 
    gUartErrUartNotOpen_c,
    gUartErrReadOngoing_c,
    gUartErrWriteOngoing_c,
    gUartErrInvalidClock_c,
    gUartErrNullPointer_c,
    gUartErrInvalidNrBytes_c,
    gUartErrInvalidBaudrate_c,
    gUartErrInvalidParity_c,
    gUartErrInvalidStop_c,
    gUartErrInvalidCTS_c,
    gUartErrInvalidThreshold_c,
    gUartErrWrongUartNumber_c,
    gUartRXFIFONotInUse_c,
    gUartTXFIFONotInUse_c,
    gUartErrMax_c
} UartErr_t;

/******************************************************************************
 * Name: UartConfig_t
 * Description: This data type describes the configuration of the UART module
 *****************************************************************************/
typedef struct 
{    
    uint32_t	            UartBaudRate;
    UartParityMode_t 	    UartParity;
    UartDataBits_t   	    UartDataBits;
    UartHwFlowControl_t     UartHWFlowCtrl;       
    UartRTSActiveState_t    UartRTSActiveState;    
} UartConfig_t;


/******************************************************************************
 * Name: UartReadErrorFlags_t
 * Description: This data type describes the possible errors passed to the read callback function
 *****************************************************************************/
typedef struct 
{
    uint32_t   UartRecvOverrunError:1;
    uint32_t   UartParityError:1;
    uint32_t   UartFrameError:1;
    uint32_t   UartNoiseError:1;
    uint32_t   Reserved:4;
} UartReadErrorFlags_t;

/******************************************************************************
 * Name: UartReadCallbackArgs_t
 * Description: This data type describes the parameter passed to the read callback function 
 * Note: Do not modify the order of the members in the structure,  because it is optimized for lower space consumption
 *****************************************************************************/
typedef struct 
{
    UartReadStatus_t            UartReadStatus;
    uint16_t                    UartNumberBytesReceived;    
    UartReadErrorFlags_t        UartReadError;
} UartRxStatus_t;


/******************************************************************************
 * Name: SciTxBufRef_t
 * Description: The transmit code keeps a circular list of buffers to be sent.
 *              Each SCI port has it's own list.
 *****************************************************************************/
typedef struct SciTxBufRef_tag 
{
    unsigned char const *pBuf;
    void (*pfCallBack)(unsigned char const *pBuf);
} SciTxBufRef_t;

/******************************************************************************
 * Name: UartModule_t
 * Description: This data type describes the internal variables used for one UART driver instance
 *****************************************************************************/

typedef struct {
    /* Variable which indicates if the UART peripheral is opened */ 
    uint32_t UartIsOpen:1; 
    /* variable which indicates if a write operation is ongoing */ 
    volatile uint32_t UartWriteOnGoing:1;
    /* variable which indicates if a read operation is ongoing */ 
    volatile uint32_t UartReadOnGoing:1;
    /* reserved for alignment purposes */
    uint32_t Reserved_01:5;
    /* variable which indicates the RX FIFO threshold */
    uint32_t UartRxThreshold:8; 
    /* variable which indicates the TX FIFO threshold */
    uint32_t UartTxThreshold:8;   
    /* pointer to a structure which maps over the UART hardware register */
    UART_MemMapPtr  UartRegs;
    /* variable which contains status of the RX operation(s) */ 
    UartRxStatus_t UartRxStatus;    
    /* pointer to UART driver read buffer */
    uint8_t* pUartRxBuffer;    
    /* variable which indicates the number of characters the UART driver should receive */
    uint32_t UartNumberCharsToReceive;    
    /* variable which contains the value of the UART status register */
    uint32_t UartUstatRegister1;
    uint32_t UartUstatRegister2;
    /* variable which contains the value of the platform clock in KHz */
    uint32_t UartPlatformClock;    
    /* Rx circular buffer leading (head) index */
    uint32_t UartBufferLeadIndex;
    /* Rx circular buffer trailing (tail) index */
    uint32_t UartBufferTrailIndex;
    /* unread bytes count within the Rx circular buffer */
    uint32_t UartUnreadBytesNumber;    
    /* UART RX FIFO size */
    uint8_t  UartRxFifoSize;
    /* UART TX FIFO size */
    uint8_t	 UartTxFifoSize;
} UartModule_t;

#endif /*gUARTInterface_d*/
#endif /* __UART_H__ */
