/******************************************************************************
 * Source file for UART/SCI driver.
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
*******************************************************************************
* Includes
*******************************************************************************
******************************************************************************/
#include "Uart_Interface.h"
#include "Uart.h"
#include "PortConfig.h"
#include "Interrupt.h"
//#include "Platform.h"

/* if neither UART enabled, then no code or data */
#if gUart1_Enabled_d || gUart2_Enabled_d

/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/
 
/******************************************************************************
 * Name: UartBaseAddress_c
 * Description: UART memory map base addresses 
 * Valid ranges: see MCU reference manual
 *****************************************************************************/
const UART_MemMapPtr UartBaseAddress_c[] = {

#if gUART_HW_MODULES_AVAILABLE_c > 0
        gUART0_BASE_PTR_c,
#endif

#if gUART_HW_MODULES_AVAILABLE_c > 1        
        gUART1_BASE_PTR_c,
#endif

#if gUART_HW_MODULES_AVAILABLE_c > 2
        gUART2_BASE_PTR_c,
#endif

#if gUART_HW_MODULES_AVAILABLE_c > 3
        gUART3_BASE_PTR_c,
#endif

#if gUART_HW_MODULES_AVAILABLE_c > 4
        gUART4_BASE_PTR_c,
#endif

#if gUART_HW_MODULES_AVAILABLE_c > 5
        gUART5_BASE_PTR_c
#endif
};

/******************************************************************************
 * Name: mUartModule
 * Description: UART module internal status & configuration variable 
 * Valid ranges: see definition of UartModule_t
 *****************************************************************************/

static UartModule_t mUartModule[gUART_HW_MODULES_AVAILABLE_c];

/************************************
 * UART_1
 ************************************/
#if gUart1_Enabled_d

/******************************************************************************
 * Name: bUart1FlowConflict
 * Description: UART_1 flow conflict
 * Valid ranges: TRUE/FALSE
 *****************************************************************************/  
#if gUart1_Hw_Sw_FlowControl_d
  static bool_t bUart1FlowConflict = FALSE;
#endif

/******************************************************************************
 * Name: uart1_ungetFlag
 * Description: UART_1 unget byte flag
 * Valid ranges: TRUE/FALSE
 *****************************************************************************/
static bool_t uart1_ungetFlag = FALSE;

/******************************************************************************
 * Name: uart1_ungetByte
 * Description: UART_1 unget byte
 *              Each port can store one unget byte
 * Valid ranges: 0..0xFF
 *****************************************************************************/
static unsigned char uart1_ungetByte;

/******************************************************************************
 * Name: maSci1TxBufRefTable
 * Description: UART_1 TX buffer reference table
 * Valid ranges: see definition of SciTxBufRef_t
 *****************************************************************************/
static SciTxBufRef_t maSci1TxBufRefTable[gUart_TransmitBuffers_c];

/******************************************************************************
 * Name: maSci1TxBufLenTable
 * Description: UART_1 table for TX buffer lengths
 * Valid ranges: range of index_t
 *****************************************************************************/

static index_t maSci1TxBufLenTable[gUart_TransmitBuffers_c] = { 0, 0, 0 };

/******************************************************************************
 * Name: mSci1TxCurIndex
 * Description: UART_1 current index in TX buffers array
 * Valid ranges: range of index_t
 *****************************************************************************/

static index_t mSci1TxCurIndex = 0;

/******************************************************************************
 * Name: mSci1TxBufRefLeadingIndex
 * Description: The leading index is the next position to store 
 *              a buffer reference
 * Valid ranges: range of index_t
 *****************************************************************************/

static index_t mSci1TxBufRefLeadingIndex = 0;   /* Post-increment. */

/******************************************************************************
 * Name: mSci1TxBufRefTrailingIndex
 * Description: The trailing index is the buffer currently being transmitted
 * Valid ranges: range of index_t
 *****************************************************************************/

static index_t mSci1TxBufRefTrailingIndex = 0;  /* Post-increment. */

/******************************************************************************
 * Name: maSci1RxBuf
 * Description: UART_1 RX buffer
 *              Each port has a small local circular Rx buffer to store data until the
 *              application can retrieve it. In addition to the leading and trailing
 *              indexes, the Rx code keeps a count of the number of bytes in the buffer.
 * Valid ranges: 0..0xFF
 *****************************************************************************/

static unsigned char maSci1RxBuf[gUart1_ReceiveBufferSize_c];

/******************************************************************************
 * Name: pfSci1RxCallBack
 * Description: callback function for UART_1 RX
 * Valid ranges: -
 *****************************************************************************/
static void (*pfSci1RxCallBack)(void);

#endif // gUart1_Enabled_d

/************************************
 * UART_2
 ************************************/

#if gUart2_Enabled_d

/******************************************************************************
 * Name: bUart2FlowConflict
 * Description: UART_2 flow conflict
 * Valid ranges: TRUE/FALSE
 *****************************************************************************/

#if gUart2_Hw_Sw_FlowControl_d
  static bool_t bUart2FlowConflict = FALSE; 
#endif

/******************************************************************************
 * Name: uart2_ungetFlag
 * Description: UART_2 unget byte flag
 * Valid ranges: TRUE/FALSE
 *****************************************************************************/
static bool_t uart2_ungetFlag = FALSE;

/******************************************************************************
 * Name: uart2_ungetByte
 * Description: UART_2 unget byte
 *              Each port can store one unget byte
 * Valid ranges: 0..0xFF
 *****************************************************************************/
static unsigned char uart2_ungetByte;

/******************************************************************************
 * Name: maSci2TxBufRefTable
 * Description: UART_2 TX buffer reference table
 * Valid ranges: see definition of SciTxBufRef_t
 *****************************************************************************/
static SciTxBufRef_t maSci2TxBufRefTable[gUart_TransmitBuffers_c];

/******************************************************************************
 * Name: maSci2TxBufLenTable
 * Description: UART_2 table for TX buffer lengths
 * Valid ranges: range of index_t
 *****************************************************************************/

static index_t maSci2TxBufLenTable[gUart_TransmitBuffers_c];

/******************************************************************************
 * Name: mSci2TxCurIndex
 * Description: UART_2 current index in TX buffers array
 * Valid ranges: range of index_t
 *****************************************************************************/

static index_t mSci2TxCurIndex=0;

/******************************************************************************
 * Name: mSci2TxBufRefLeadingIndex
 * Description: The leading index is the next position to store 
 *              a buffer reference
 * Valid ranges: range of index_t
 *****************************************************************************/

static index_t mSci2TxBufRefLeadingIndex=0;   /* Post-increment. */

/******************************************************************************
 * Name: mSci2TxBufRefTrailingIndex
 * Description: The trailing index is the buffer currently being transmitted
 * Valid ranges: range of index_t
 *****************************************************************************/

static index_t mSci2TxBufRefTrailingIndex=0;  /* Post-increment. */

/******************************************************************************
 * Name: maSci2RxBuf
 * Description: UART_2 RX buffer
 *              Each port has a small local circular Rx buffer to store data until the
 *              application can retrieve it. In addition to the leading and trailing
 *              indexes, the Rx code keeps a count of the number of bytes in the buffer.
 * Valid ranges: 0..0xFF
 *****************************************************************************/

static unsigned char maSci2RxBuf[gUart2_ReceiveBufferSize_c];

/******************************************************************************
 * Name: pfSci2RxCallBack
 * Description: callback function for UART_2 RX
 * Valid ranges: -
 *****************************************************************************/

static void (*pfSci2RxCallBack)(void);

#endif  //gUart2_Enabled_d

/******************************************************************************
*******************************************************************************
* Public memory definitions
*******************************************************************************
******************************************************************************/
 
/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/
 
/******************************************************************************
 * Name: UartReceiverIsr
 * Description: UART RX interrupt handler. Called in interrupt context.
 * Parameters: [IN]: UartNumber - the UART peripheral number
 * Return: -
 *****************************************************************************/
static void UartReceiverIsr 
(
    uint8_t UartNumber
);

/******************************************************************************
 * Name: UartTransmitterIsr
 * Description: UART TX interrupt handler. Called in interrupt context.
 * Parameters: [IN]: UartNumber - the UART peripheral number
 * Return: -
 *****************************************************************************/
static void UartTransmitterIsr 
(
    uint8_t UartNumber
);

/******************************************************************************
 * Name: UartReadComplete
 * Description: UART read complete function.
 * Parameters: [IN]: UartNumber - the UART peripheral number
 * Return: -
 *****************************************************************************/
static void UartReadComplete
(
    uint8_t UartNumber
);

/******************************************************************************
 * Name: UartDivide
 * Description: Integer division by repeated substractions.
 * Parameters: [IN]: Number - the number to be divided
 *             [IN]: Divisor - the number to divide to
 * Return: the result of the division operation
 *****************************************************************************/
static uint16_t UartDivide
(
    int32_t Number, 
    uint32_t Divisor
);

/******************************************************************************
 * Name: UartOpen
 * Description: Open the specified UART port
 * Parameters: [IN]: UartNumber - the UART port number to be opened
 *             [IN]: PlatformClock - The platform clock in KHz
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartOpen
(
        uint8_t UartNumber, 
        uint32_t PlatformClock
);

/******************************************************************************
 * Name: UartSetConfig
 * Description: configure the specified UART port
 * Parameters: [IN]: UartNumber - the UART port number to be configured
 *             [IN]: pConfig - pointer to UART configuration data structure
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartSetConfig
(
    uint8_t UartNumber, 
    UartConfig_t* pConfig
);

/******************************************************************************
 * Name: UartGetConfig
 * Description: get the configuration of the specified UART port
 * Parameters: [IN]:  UartNumber - the UART port number to read the 
 *                               configuration from
 *             [OUT]: pConfig - pointer to UART configuration structure where
 *                            the configuration data shall be placed                  
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartGetConfig
(
    uint8_t UartNumber, 
    UartConfig_t* pConfig
);

/******************************************************************************
 * Name: UartSetReceiverThreshold
 * Description: sets the receiver threshold
 * Parameters: [IN]: UartNumber - the UART port number
 *             [IN]: Threshold - the desired threshold
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartSetReceiverThreshold
(
    uint8_t UartNumber,
    uint8_t Threshold
);

/******************************************************************************
 * Name: UartSetTransmitterThreshold
 * Description: sets the transmitter threshold
 * Parameters: [IN]: UartNumber - the UART port number
 *             [IN]: Threshold - the desired threshold
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartSetTransmitterThreshold
(
    uint8_t UartNumber,
    uint8_t Threshold
);

/******************************************************************************
 * Name: UartReadData
 * Description: read data from the specified UART port
 * Parameters: [IN]: UartNumber - the UART port number
 *             [OUT]: pBuf - pointer to the buffer where the read data shall be 
 *                         placed 
 *             [IN]: NumberBytes - number of bytes to be read
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartReadData
(
    uint8_t UartNumber, 
    uint8_t* pBuf,
    uint16_t NumberBytes
);

/******************************************************************************
 * Name: UartCancelReadData
 * Description: cancels the read data operation
 * Parameters: [IN]: UartNumber - the UART port number
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartCancelReadData
(
    uint8_t UartNumber
);

/******************************************************************************
 * Name: UartCancelWriteData
 * Description: cancels the write data operation
 * Parameters: [IN]: UartNumber - the UART port number
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartCancelWriteData
(
    uint8_t UartNumber
);

/******************************************************************************
 * Name: UartGetStatus
 * Description: gets the status of the specified UART port
 * Parameters: [IN]: UartNumber - the UART port number
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartGetStatus
(
    uint8_t UartNumber
);

/******************************************************************************
 * Name: UartClose
 * Description: closes the specified UART port
 * Parameters: [IN] UartNumber - the UART port number
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartClose
(
    uint8_t UartNumber
);

/******************************************************************************
 * Name: UartGetByteFromRxBuffer
 * Description: gets a byte from the HW RX buffer and appends it to RAM buffer
 * Parameters: [IN] UartNumber - the UART port number
 *             [OUT] pDst - pointer to the RAM buffer
 * Return: TRUE/FALSE
 *****************************************************************************/
static bool_t UartGetByteFromRxBuffer
(
    uint8_t UartNumber, 
    uint8_t *pDst
);

/******************************************************************************
 * Name: UartOpenCloseTransceiver
 * Description: opens or closes the UART transceiver
 * Parameters: [IN] UartNumber - the UART port number
 *             [IN] Pin - the bit in the control register that 
 *                  enables/disables the transmiter and the receiver
 *             [IN] Open - if TRUE, it opens the receiver/trasmitter; if FALSE
 *                         it closes the receiver/transmiter
 * Return: -
 *****************************************************************************/
static void UartOpenCloseTransceiver
(
    uint8_t UartNumber, 
    uint8_t Pin, 
    bool_t Open
);

/******************************************************************************
 * Name: UartClearErrors
 * Description: clears the UART receive errors
 * Parameters: [IN] UartNumber - the UART port number
 * Return: -
 *****************************************************************************/
static void UartClearErrors
(
    uint8_t UartNumber
);

/******************************************************************************
 * Name: UartExecPendingCallbacks
 * Description: execute any pending UART transmit callbacks
 * Parameters: [IN] UartNumber - the UART port number
 * Return: -
 *****************************************************************************/
static void UartExecPendingCallbacks
(
        uint8_t UartNumber
);

/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/
 
 /*****************************************************************************
 * Name: UartReceiverIsr
 * Description: UART RX interrupt handler. Called in interrupt context.
 * Parameters: [IN]: UartNumber - the UART peripheral number
 * Return: -
 *****************************************************************************/
static void UartReceiverIsr 
(
    uint8_t UartNumber
)
{
  UartModule_t *pModule = &mUartModule[UartNumber];
#if UART_USE_RX_FIFO  
    	if(mUartModule[UartNumber].UartRxFifoSize > 1)
	{				
		while(mUartModule[UartNumber].UartRegs->RCFIFO)
		{ 
			/* read one character from RX FIFO */    					
			pModule->pUartRxBuffer[pModule->UartBufferLeadIndex] = pModule->UartRegs->D;
                        
			if (++pModule->UartBufferLeadIndex >= pModule->UartNumberCharsToReceive) 
			{
				pModule->UartBufferLeadIndex = 0;
			}

			if (pModule->UartUnreadBytesNumber < pModule->UartNumberCharsToReceive) 
			{
				++pModule->UartUnreadBytesNumber;
			}
			else 
			{
				/* A new character was received while the buffer is already full. 
				 * The oldest character in the buffer has been overwritten. 
				 * The trailing index must be also incremented to compensate the overwritten characters
				 */
				if(++pModule->UartBufferTrailIndex >= pModule->UartNumberCharsToReceive) 
				{
					pModule->UartBufferTrailIndex = 0;
				}
			}                                                
		}
	}    	    
	else  
#endif /* UART_USE_RX_FIFO */
   	{
		pModule->pUartRxBuffer[pModule->UartBufferLeadIndex] = pModule->UartRegs->D;    		

		if (++pModule->UartBufferLeadIndex >= pModule->UartNumberCharsToReceive) 
		{
			pModule->UartBufferLeadIndex = 0;
		}
		if (pModule->UartUnreadBytesNumber < pModule->UartNumberCharsToReceive) 
		{
			++pModule->UartUnreadBytesNumber;
		}
		else 
		{
			/* A new character was received while the buffer is already full. 
			 * The oldest character in the buffer has been overwritten. 
			 * The trailing index must be also incremented to compensate the overwritten characters
			 */
			if(++pModule->UartBufferTrailIndex >= pModule->UartNumberCharsToReceive) 
			{
				pModule->UartBufferTrailIndex = 0;
			}
		}
	}
	
	/* Let the application know that one (or more) byte(s) have (has) been received */
#if (TRUE == gUart1_Enabled_d)	
	if(UartNumber == gUart1_c)
	{
#if gUart1_Hw_Sw_FlowControl_d
        if (pModule->UartNumberCharsToReceive - pModule->UartUnreadBytesNumber <= gUart1_RxFlowControlSkew_d) 
        {
                Uart1_DisableFlow();
        }
#endif
            if(pfSci1RxCallBack != NULL)
            {
              pfSci1RxCallBack();
            }
        }
#endif /* gUart1_Enabled_d */
        
#if (TRUE == gUart2_Enabled_d)        
        if(UartNumber==gUart2_c)
        {
#if gUart2_Hw_Sw_FlowControl_d
            if (pModule->UartNumberCharsToReceive - pModule->UartUnreadBytesNumber <= gUart2_RxFlowControlSkew_d) 
            {
                    Uart2_DisableFlow();
            }
#endif
             if(pfSci2RxCallBack != NULL)
             {  
                pfSci2RxCallBack();
             }  
        } 
#endif /* gUart2_Enabled_d */
}  

/******************************************************************************
 * Name: UartTransmitterIsr
 * Description: UART TX interrupt handler. Called in interrupt context.
 * Parameters: [IN]: UartNumber - the UART peripheral number
 * Return: -
 *****************************************************************************/
static void UartTransmitterIsr 
(
    uint8_t UartNumber
)
{	/*************************
	 ******** UART_1 *********
	 *************************/
	
#if (gUart1_Enabled_d == TRUE)

    if(gUart1_c == UartNumber)
    {
        volatile uint8_t dummy;
                
#if gUART_USE_TX_FIFO_d
        
        if((mUartModule[gUart1_c].UartTxFifoSize > 1) && (mUartModule[gUart1_c].UartRegs->PFIFO & gUART_TX_FIFO_EN_BIT_c))
        {
        	if(mUartModule[gUart1_c].UartRegs->S1 & gUART_S1_TDRE_MASK_c)
        	{        		        	
        		while(mUartModule[gUart1_c].UartTxFifoSize - mUartModule[gUart1_c].UartRegs->TCFIFO > 0)
        		{
        			if(FALSE == mUartModule[gUart1_c].UartWriteOnGoing)
        				break;
        			
        			/* Write a byte. */
        			mUartModule[gUart1_c].UartRegs->D = maSci1TxBufRefTable[mSci1TxBufRefTrailingIndex].pBuf[mSci1TxCurIndex];        	

        			/* Finished with this buffer? */
        			if (++mSci1TxCurIndex >= maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex]) 
        			{

        				/* Mark this one as done */
        				maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex] = 0;                
        				if ( maSci1TxBufRefTable[mSci1TxBufRefTrailingIndex].pfCallBack) 
        				{
                                                UartExecPendingCallbacks(gUart1_c);
        				}                

        				/* Reset the current buffer index */
        				mSci1TxCurIndex = 0;

        				/* Increment to the next buffer. */
        				if (++mSci1TxBufRefTrailingIndex >= NumberOfElements(maSci1TxBufRefTable)) 
        				{
        					mSci1TxBufRefTrailingIndex = 0;
        				}

        				/* If there is no more data to send, turn off the transmit interrupt. */
        				if (!maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex]) 
        				{
        					mUartModule[gUart1_c].UartRegs->C2 &= ~(gUART_TIE_BIT_c);
        					/* clear the write ongoing flag */
        					mUartModule[gUart1_c].UartWriteOnGoing = FALSE;
        				}
        			} 
        		}
        		
        		/* this read will clear the TDRE flag */
        		dummy = mUartModule[gUart1_c].UartRegs->S1;
        	}
        }
        else
#endif
        {
#if gUart1_Hw_Sw_FlowControl_d
        	if(Uart1_FlowConflict())
        	{
        		/* Disable the TX interrupt */
        		mUartModule[gUart1_c].UartRegs->C2 &= ~(gUART_TIE_BIT_c);
        		bUart1FlowConflict = TRUE;
        	}
        	else
#endif	
        	{
        		/* Dummy read - required before write to data register. */
        		dummy = mUartModule[gUart1_c].UartRegs->S1;

        		/* Write a byte. */
        		mUartModule[gUart1_c].UartRegs->D = maSci1TxBufRefTable[mSci1TxBufRefTrailingIndex].pBuf[mSci1TxCurIndex];        	

        		/* Finished with this buffer? */
        		if (++mSci1TxCurIndex >= maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex]) 
        		{

        			/* Mark this one as done */
        			maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex] = 0;                
        			if ( maSci1TxBufRefTable[mSci1TxBufRefTrailingIndex].pfCallBack) 
        			{
                                        UartExecPendingCallbacks(gUart1_c);                  
        			}                

        			/* Reset the current buffer index */
        			mSci1TxCurIndex = 0;

        			/* Increment to the next buffer. */
        			if (++mSci1TxBufRefTrailingIndex >= NumberOfElements(maSci1TxBufRefTable)) 
        			{
        				mSci1TxBufRefTrailingIndex = 0;
        			}

        			/* If there is no more data to send, turn off the transmit interrupt. */
        			if (!maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex]) 
        			{
        				mUartModule[gUart1_c].UartRegs->C2 &= ~(gUART_TIE_BIT_c);
        				/* clear the write ongoing flag */
        				mUartModule[gUart1_c].UartWriteOnGoing = FALSE;
        			}
        		} 
        	}
       	
        }
        (void)dummy;
    }   
#endif // gUart1_Enabled_d   
    
    /*************************
     ******** UART_2 *********
     *************************/
    
#if (gUart2_Enabled_d == TRUE)

    if(gUart2_c == UartNumber)
    {
        volatile uint8_t dummy;
                
#if gUART_USE_TX_FIFO_d
        
        if((mUartModule[gUart2_c].UartTxFifoSize > 1) && (mUartModule[gUart2_c].UartRegs->PFIFO & gUART_TX_FIFO_EN_BIT_c))
        {
        	if(mUartModule[gUart2_c].UartRegs->S1 & gUART_S1_TDRE_MASK_c)
        	{        		        	
        		while(mUartModule[gUart2_c].UartTxFifoSize - mUartModule[gUart2_c].UartRegs->TCFIFO > 0)
        		{
        			if(FALSE == mUartModule[gUart2_c].UartWriteOnGoing)
        				break;
        			
        			/* Write a byte. */
        			mUartModule[gUart2_c].UartRegs->D = maSci2TxBufRefTable[mSci2TxBufRefTrailingIndex].pBuf[mSci2TxCurIndex];        	

        			/* Finished with this buffer? */
        			if (++mSci2TxCurIndex >= maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex]) 
        			{

        				/* Mark this one as done */
        				maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex] = 0;                
        				if ( maSci2TxBufRefTable[mSci2TxBufRefTrailingIndex].pfCallBack) 
        				{
                                                UartExecPendingCallbacks(gUart2_c);
        				}                

        				/* Reset the current buffer index */
        				mSci2TxCurIndex = 0;

        				/* Increment to the next buffer. */
        				if (++mSci2TxBufRefTrailingIndex >= NumberOfElements(maSci2TxBufRefTable)) 
        				{
        					mSci2TxBufRefTrailingIndex = 0;
        				}

        				/* If there is no more data to send, turn off the transmit interrupt. */
        				if (!maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex]) 
        				{
        					mUartModule[gUart2_c].UartRegs->C2 &= ~(gUART_TIE_BIT_c);
        					/* clear the write ongoing flag */
        					mUartModule[gUart2_c].UartWriteOnGoing = FALSE;
        				}
        			} 
        		}
        		
        		/* this read will clear the TDRE flag */
        		dummy = mUartModule[gUart2_c].UartRegs->S1;
        	}
        }
        else
#endif
        {
#if gUart2_Hw_Sw_FlowControl_d
        	if(Uart2_FlowConflict())
        	{
        		/* Disable the TX interrupt */
        		mUartModule[gUart2_c].UartRegs->C2 &= ~(gUART_TIE_BIT_c);
        		bUart2FlowConflict = TRUE;
        	}
        	else
#endif
        	{
        		/* Dummy read - required before write to data register. */
        		dummy = mUartModule[gUart2_c].UartRegs->S1;

        		/* Write a byte. */
        		mUartModule[gUart2_c].UartRegs->D = maSci2TxBufRefTable[mSci2TxBufRefTrailingIndex].pBuf[mSci2TxCurIndex];        	

        		/* Finished with this buffer? */
        		if (++mSci2TxCurIndex >= maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex]) 
        		{

        			/* Mark this one as done */
        			maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex] = 0;                
        			if ( maSci2TxBufRefTable[mSci2TxBufRefTrailingIndex].pfCallBack) 
        			{
                                        UartExecPendingCallbacks(gUart2_c); 
        			}                

        			/* Reset the current buffer index */
        			mSci2TxCurIndex = 0;

        			/* Increment to the next buffer. */
        			if (++mSci2TxBufRefTrailingIndex >= NumberOfElements(maSci2TxBufRefTable)) 
        			{
        				mSci2TxBufRefTrailingIndex = 0;
        			}

        			/* If there is no more data to send, turn off the transmit interrupt. */
        			if (!maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex]) 
        			{
        				mUartModule[gUart2_c].UartRegs->C2 &= ~(gUART_TIE_BIT_c);
        				/* clear the write ongoing flag */
        				mUartModule[gUart2_c].UartWriteOnGoing = FALSE;
        			}
        		} 
        	}       	
        }
    }   
#endif // gUart2_Enabled_d  }    
}

/******************************************************************************
 * Name: UartReadComplete
 * Description: UART read complete function.
 * Parameters: [IN]: UartNumber - the UART peripheral number
 * Return: -
 *****************************************************************************/
static void UartReadComplete
(
    uint8_t UartNumber
)
{    
	/* Reset the internal write flag because the reception is over */
	mUartModule[UartNumber].UartReadOnGoing = FALSE;
	
	/* Disable RX interrupt */ 
	mUartModule[UartNumber].UartRegs->C2 &= ~(gUART_RIE_BIT_c);
   
    /* Call to the read pCallback function */
        if (UartNumber == gUart1_c)
        {
          #if (TRUE == gUart1_Enabled_d)
                  if(pfSci1RxCallBack != NULL)
                  {
                    pfSci1RxCallBack();
                  }
          #endif
        }
        else
        {
          #if (TRUE == gUart2_Enabled_d)
                if(pfSci2RxCallBack != NULL)
                {
                  pfSci2RxCallBack();
                } 
          #endif    
        }
}

/******************************************************************************
 * Name: UartDivide
 * Description: Integer division by repeated substractions.
 * Parameters: [IN]: Number - the number to be divided
 *             [IN]: Divisor - the number to divide to
 * Return: the result of the division operation
 *****************************************************************************/
static uint16_t UartDivide
(
    int32_t Number, 
    uint32_t Divisor
)
{
    /* Variable to store result of the operation */
    uint16_t Result=0;

    while(1)
    {    
        Number-=Divisor;
        if(Number<0) 
        {
            return Result;
        }
        Result++;
    }
}

/******************************************************************************
 * Name: UartOpen
 * Description: Open the specified UART port
 * Parameters: [IN]: UartNumber - the UART port number to be opened
 *             [IN]: PlatformClock - The platform clock in KHz
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartOpen
(
    uint8_t UartNumber, 
    uint32_t PlatformClock
)
{
   
    /* Variable used to clear the UART status register */
    register uint32_t dummy;

    /* Check if the first parameter received is valid  */
    if (UartNumber >= gUART_HW_MODULES_AVAILABLE_c)
    {
        return gUartErrWrongUartNumber_c;
    }

    /* Check if UART is open */
    if (mUartModule[UartNumber].UartIsOpen)
    {
        return gUartErrUartAlreadyOpen_c;
    }

    if(NULL == mUartModule[UartNumber].UartRegs)
    {
        return gUartErrNullPointer_c;
    }

    /* Disable transceiver */
    mUartModule[UartNumber].UartRegs->C2 &= ~(gUART_TX_EN_BIT_c | gUART_RX_EN_BIT_c);

    /* Clear all bits in control registers; disable RX and TX interrupts */
    mUartModule[UartNumber].UartRegs->C1 = 0x00;
    mUartModule[UartNumber].UartRegs->C2 &= ~(gUART_TIE_BIT_c | gUART_RIE_BIT_c); 
    mUartModule[UartNumber].UartRegs->C3 = 0x00;
    mUartModule[UartNumber].UartRegs->C4 = 0x00;
    mUartModule[UartNumber].UartRegs->C5 = 0x00;

    /* Initialize the internal variables */
    mUartModule[UartNumber].UartIsOpen = TRUE;
    mUartModule[UartNumber].UartReadOnGoing = FALSE;
    mUartModule[UartNumber].UartWriteOnGoing = FALSE;
    mUartModule[UartNumber].UartPlatformClock = PlatformClock;    

    /* Read status registers in order to clear error bits and interrupt receive bits */
    dummy = mUartModule[UartNumber].UartRegs->S1;   
    (void)dummy;
    dummy = mUartModule[UartNumber].UartRegs->S2;
    (void)dummy;

    /* Enable transceiver */
    mUartModule[UartNumber].UartRegs->C2 |=  (gUART_TX_EN_BIT_c | gUART_RX_EN_BIT_c);

    /* Exit without error */
    return gUartErrNoError_c;
}

/******************************************************************************
 * Name: UartSetConfig
 * Description: configure the specified UART port
 * Parameters: [IN]: UartNumber - the UART port number to be configured
 *             [IN]: pConfig - pointer to UART configuration data structure
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartSetConfig
(
    uint8_t UartNumber, 
    UartConfig_t* pConfig
)
{/* variables used for temporary store values for INC and MOD UART registers */
    register uint16_t mUartSBR;
    register uint32_t mUartBRDF;
    register uint8_t mUartBRFA;    

    /* check if the first parameter received is valid */
    if (UartNumber >= gUART_HW_MODULES_AVAILABLE_c)    
        return gUartErrWrongUartNumber_c;

    /* check if the second parameter received is valid */
    if (NULL == pConfig)    
        return gUartErrNullPointer_c;

    /* check if UART has been opened */
    if (!mUartModule[UartNumber].UartIsOpen)    
        return gUartErrUartNotOpen_c;

    /* check the possibility to set the specified baudrate considering the platform clock  (platform clock/baudrate >=8) */
    if ((((mUartModule[UartNumber].UartPlatformClock * 1000) >>3) < pConfig->UartBaudRate))    
        return gUartErrInvalidBaudrate_c;

    /* check if the read operation is ongoing */
    if (mUartModule[UartNumber].UartReadOnGoing)    
        return gUartErrReadOngoing_c;

    /* check if the write operation is ongoing */
    if (mUartModule[UartNumber].UartWriteOnGoing)
        return gUartErrWriteOngoing_c;

    /* set parity - set/reset PE and PT bits from UARTx_C2 */
    switch(pConfig->UartParity)
    {
    case gUartParityNone_c:
        /* reset the PE (parity enable) bit */
        mUartModule[UartNumber].UartRegs->C1 &= ~gUART_PARITY_EN_BIT_c;
        break;
    case gUartParityEven_c:
        /* set the PE (parity enable) bit */
        mUartModule[UartNumber].UartRegs->C1 |= gUART_PARITY_EN_BIT_c;
        /* reset the PT (parity type) bit */
        mUartModule[UartNumber].UartRegs->C1 &= ~gUART_PARITY_EVEN_BIT_c;
        break;
    case gUartParityOdd_c:
        /* set the PE (parity enable) bit */
        mUartModule[UartNumber].UartRegs->C1 |=  gUART_PARITY_EN_BIT_c;
        /* set the PT (parity type) bit */
        mUartModule[UartNumber].UartRegs->C1 |= gUART_PARITY_EVEN_BIT_c;
        break;
    default:
        /* on default case, return error */
        return gUartErrInvalidParity_c;
    }

    /* set the number of data bits - set/reset M bit from UARTx_C1 register */
    switch(pConfig->UartDataBits)
    {
    case gUart_8bits_c:
        /* M = 0 (8 bits) */
        mUartModule[UartNumber].UartRegs->C1 &= ~gUART_DATA_BITS_BIT_c;
        break;
    case gUart_9bits_c:
        /* M = 1 (9 bits) */
        mUartModule[UartNumber].UartRegs->C1 |=  gUART_DATA_BITS_BIT_c;
        break;
    default:
        /* on default case, return error */
        return gUartErrInvalidStop_c;
    }

    /* configure the HW flow control, if enabled */
    if(gFlowEnabled_c == pConfig->UartHWFlowCtrl)
    {
    	
    	if(gUart1_c == UartNumber)
    	{
    		/* Setup Port Control Register (PCR) for RTS pin */
    		gUART_1_RTS_PCR_c = PORT_PCR_MUX(gUART_1_MUX_c);
    		/* Setup Port Control Register (PCR) for CTS pin */
    		gUART_1_CTS_PCR_c = PORT_PCR_MUX(gUART_1_MUX_c);
    	} else if(gUart2_c == UartNumber)
    	{
    		/* Setup Port Control Register (PCR) for RTS pin */
    		gUART_2_RTS_PCR_c = PORT_PCR_MUX(gUART_2_MUX_c);
    		/* Setup Port Control Register (PCR) for CTS pin */
    		gUART_2_CTS_PCR_c = PORT_PCR_MUX(gUART_2_MUX_c);
    	}
    	    	
        /* set the transmitter RTS polarity */
        if(gRtsActiveHi_c == pConfig->UartRTSActiveState) {
            /* TXRTSPOL = 1 */
            mUartModule[UartNumber].UartRegs->MODEM |= gUART_TXRTSPOL_BIT_c;
        } else {
            /* TXRTSPOL = 0 */
            mUartModule[UartNumber].UartRegs->MODEM &= ~gUART_TXRTSPOL_BIT_c;
        }

        /* set the receiver RTS bit */
        mUartModule[UartNumber].UartRegs->MODEM |= gUART_RXRTSE_BIT_c;

        /* set the transmitter CTS bit */
        mUartModule[UartNumber].UartRegs->MODEM |= gUART_TXCTSE_BIT_c;     
    } 
    else 
    {
        /* HW flow control is disabled */
        mUartModule[UartNumber].UartRegs->MODEM &= ~(gUART_RXRTSE_BIT_c | gUART_TXRTSPOL_BIT_c | gUART_TXCTSE_BIT_c);
    }
    
    /* default FIFO sizes */
    mUartModule[UartNumber].UartRxFifoSize = 1;
    mUartModule[UartNumber].UartTxFifoSize = 1;
    
#if (gUART_USE_RX_FIFO_d == TRUE)    
    /* get RX FIFO size */
    {
    	register uint8_t fifoSz = (uint8_t)(mUartModule[UartNumber].UartRegs->PFIFO & gUART_RX_FIFO_MASK_c);    	
    	fifoSz+=1;
    	
    	while(fifoSz-->0)
    	{
    		mUartModule[UartNumber].UartRxFifoSize = mUartModule[UartNumber].UartRxFifoSize << 1;    		
    	};    	
    }
        
    /* enable RX FIFO and set RX threshold, if FIFO is implemented by hardware */
    if(mUartModule[UartNumber].UartRxFifoSize > 1)
    {
    	UartSetReceiverThreshold(UartNumber, gUART_RX_THRESHOLD_c);    	
    }
#endif
    
#if (gUART_USE_TX_FIFO_d == TRUE)
    /* get TX FIFO size */
    {
    	register uint8_t fifoSz = (uint8_t)((mUartModule[UartNumber].UartRegs->PFIFO >> 4 ) & gUART_RX_FIFO_MASK_c);    	
    	fifoSz+=1;

    	while(fifoSz-->0)
    	{
    		mUartModule[UartNumber].UartTxFifoSize = mUartModule[UartNumber].UartTxFifoSize << 1;    		
    	};    	
    }    
    
    /* enable TX FIFO and set TX threshold, if FIFO is implemented by hardware */
    if(mUartModule[UartNumber].UartTxFifoSize > 1)
    {
    	UartSetTransmitterThreshold(UartNumber, gUART_TX_THRESHOLD_c);    	
    }    
#endif

    /* setup the baud rate registers */
    if(0 != pConfig->UartBaudRate) {

        mUartSBR = UartDivide( (mUartModule[UartNumber].UartPlatformClock * 1000), pConfig->UartBaudRate << 4);
        /* setup the baud rate registers BDH and BDL */
        mUartModule[UartNumber].UartRegs->BDH = 0x00;
        mUartModule[UartNumber].UartRegs->BDH |= (uint8_t)((mUartSBR >> 8) & gUART_BDH_MASK_c);
        mUartModule[UartNumber].UartRegs->BDL = (uint8_t) (mUartSBR & gUART_BDL_MASK_c);
        /* setup the baud rate fine adjust registers */
        mUartBRDF = UartDivide((mUartModule[UartNumber].UartPlatformClock * 100000), (pConfig->UartBaudRate<<4)) - (mUartSBR * 100);        
        mUartBRFA = UartDivide((mUartBRDF << 5), 100);
        mUartModule[UartNumber].UartRegs->C4 |= (uint8_t)(mUartBRFA & gUART_BRFA_MASK_c); 
    }

    /* enable UART error(s) interrupts */
    mUartModule[UartNumber].UartRegs->C3 |= (gUART_PARITY_ERROR_FLAG_c | gUART_FRAME_ERROR_FLAG_c | gUART_NOISE_FLAG_c | gUART_OVERRUN_FLAG_c);

    /* enable UART transceiver */
    mUartModule[UartNumber].UartRegs->C2 |=  (gUART_TX_EN_BIT_c | gUART_RX_EN_BIT_c);

    /* exit function without error */
    return gUartErrNoError_c;
}

/******************************************************************************
 * Name: UartGetConfig
 * Description: get the configuration of the specified UART port
 * Parameters: [IN]:  UartNumber - the UART port number to read the 
 *                               configuration from
 *             [OUT]: pConfig - pointer to UART configuration structure where
 *                            the configuration data shall be placed                  
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartGetConfig
(
    uint8_t UartNumber, 
    UartConfig_t* pConfig
)
{
    /* check if the first parameter received is valid */
    if (UartNumber >= gUART_HW_MODULES_AVAILABLE_c)
        return gUartErrWrongUartNumber_c;
    /* check if the second parameter received is valid */
    if (NULL == pConfig)
        return gUartErrNullPointer_c;
    /* check if UART has been opened */
    if (!mUartModule[UartNumber].UartIsOpen)
        return gUartErrUartNotOpen_c;

    /* get parity - look for the PE and PT bits in UARTx_C1 register */
    if (!(mUartModule[UartNumber].UartRegs->C1 & gUART_PARITY_EN_BIT_c))
    {
        pConfig->UartParity=gUartParityNone_c;
    }
    else
    {
        if (mUartModule[UartNumber].UartRegs->C1 & gUART_PARITY_EVEN_BIT_c)
        {
            pConfig->UartParity=gUartParityEven_c;
        }
        else
        {
            pConfig->UartParity=gUartParityOdd_c;
        }       
    }

    /* get the number of data bits - look for M bit in UARTx_C1 register */
    if (mUartModule[UartNumber].UartRegs->C1 & gUART_DATA_BITS_BIT_c)
        pConfig->UartDataBits=gUart_9bits_c;
    else
        pConfig->UartDataBits=gUart_8bits_c;

    /* get the flow control */
    if((mUartModule[UartNumber].UartRegs->MODEM & (gUART_RXRTSE_BIT_c | gUART_TXCTSE_BIT_c)))
    {
        pConfig->UartHWFlowCtrl = gFlowEnabled_c; /* HW flow control enabled */

        if(mUartModule[UartNumber].UartRegs->MODEM & gUART_TXRTSPOL_BIT_c) 
        {
            pConfig->UartRTSActiveState = gRtsActiveHi_c;
        } else 
        {
            pConfig->UartRTSActiveState = gRtsActiveLow_c;
        }       
    }
    else 
    {
        pConfig->UartHWFlowCtrl = gNoFlowControl_c;  /* HW flow control disabled */
    }

    /* exit function without error */
    return gUartErrNoError_c;    
}

/******************************************************************************
 * Name: UartSetReceiverThreshold
 * Description: sets the receiver threshold
 * Parameters: [IN]: UartNumber - the UART port number
 *             [IN]: Threshold - the desired threshold
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartSetReceiverThreshold
(
    uint8_t UartNumber,
    uint8_t Threshold
)
{	
#if (gUART_USE_RX_FIFO_d == FALSE)
	(void)UartNumber;
	(void)Threshold;
    return gUartRXFIFONotInUse_c;
#else

    UartConfig_t UartCfg;

    UartGetConfig(UartNumber, &UartCfg);

    if (UartNumber >= gUART_HW_MODULES_AVAILABLE_c)
        return gUartErrWrongUartNumber_c;
     
    if (Threshold > mUartModule[UartNumber].UartRxFifoSize)
        return gUartErrInvalidThreshold_c;
    
    if (!mUartModule[UartNumber].UartIsOpen)
        return gUartErrUartNotOpen_c;
    
    if (mUartModule[UartNumber].UartReadOnGoing)
        return gUartErrReadOngoing_c;

    /* disable transceiver*/
    mUartModule[UartNumber].UartRegs->C2 &= ~(gUART_TX_EN_BIT_c | gUART_RX_EN_BIT_c);     
    
    /* enable RX FIFO */
    mUartModule[UartNumber].UartRegs->PFIFO |= gUART_RX_FIFO_EN_BIT_c;
    
    /* write the threshold to UART FIFO Receive Water-mark register */
    mUartModule[UartNumber].UartRegs->RWFIFO = Threshold;
    
    /* Flush the FIFOs */
    mUartModule[UartNumber].UartRegs->CFIFO |= (gUART_TX_FIFO_FLUSH_c | gUART_RX_FIFO_FLUSH_c);

    /* enable transceiver */    
    mUartModule[UartNumber].UartRegs->C2 |= (gUART_TX_EN_BIT_c | gUART_RX_EN_BIT_c);

    /* memorize internally the value of the threshold */     
    mUartModule[UartNumber].UartRxThreshold = Threshold;

    /* exit function without error */
    return gUartErrNoError_c;
#endif
}

/******************************************************************************
 * Name: UartSetTransmitterThreshold
 * Description: sets the transmitter threshold
 * Parameters: [IN]: UartNumber - the UART port number
 *             [IN]: Threshold - the desired threshold
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartSetTransmitterThreshold
(
    uint8_t UartNumber,
    uint8_t Threshold
)
{
#if (gUART_USE_TX_FIFO_d == FALSE)      
    (void)UartNumber;
    (void)Threshold;
    return gUartTXFIFONotInUse_c;
#else

    UartConfig_t UartCfg;

    UartGetConfig(UartNumber, &UartCfg);
    
    if (UartNumber >= gUART_HW_MODULES_AVAILABLE_c) 
        return gUartErrWrongUartNumber_c;
        
    if (Threshold > mUartModule[UartNumber].UartTxFifoSize)
        return gUartErrInvalidThreshold_c;
    
    if (!mUartModule[UartNumber].UartIsOpen)
        return gUartErrUartNotOpen_c;
    
    if (mUartModule[UartNumber].UartWriteOnGoing)
        return gUartErrWriteOngoing_c;

    /* disable transceiver*/
    mUartModule[UartNumber].UartRegs->C2 &= ~(gUART_TX_EN_BIT_c | gUART_RX_EN_BIT_c);   
    
    /* enable TX FIFO */
    mUartModule[UartNumber].UartRegs->PFIFO |= gUART_TX_FIFO_EN_BIT_c;

    /* write the threshold to UART FIFO Transmit Water-mark register */
    mUartModule[UartNumber].UartRegs->TWFIFO = Threshold;
    
    /* Flush the FIFOs */
    mUartModule[UartNumber].UartRegs->CFIFO |= (gUART_TX_FIFO_FLUSH_c | gUART_RX_FIFO_FLUSH_c);

    /* enable transceiver */
    mUartModule[UartNumber].UartRegs->C2 |=  (gUART_TX_EN_BIT_c | gUART_RX_EN_BIT_c);

    /* memorize internally the value of the threshold */
    mUartModule[UartNumber].UartTxThreshold = Threshold;

    /* exit function without error */
    return gUartErrNoError_c;
#endif  
}

/******************************************************************************
 * Name: UartReadData
 * Description: read data from the specified UART port
 * Parameters: [IN]: UartNumber - the UART port number
 *             [OUT]: pBuf - pointer to the buffer where the read data shall be 
 *                         placed 
 *             [IN]: NumberBytes - number of bytes to be read 
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartReadData
(
    uint8_t UartNumber, 
    uint8_t* pBuf,
    uint16_t NumberBytes
)
{
    /* Variable used for clear the Rx hardware FIFO */
    register uint8_t dummy;

    /* Check if the first parameter received is valid */ 
    if (UartNumber >= gUART_HW_MODULES_AVAILABLE_c) 
        return gUartErrWrongUartNumber_c;
    
    /* Check if the pointer received is not null */
    if (NULL == pBuf)   
        return gUartErrNullPointer_c;
    
    /* Check if the buffer length is not zero */
    if (NumberBytes==0) 
        return gUartErrInvalidNrBytes_c;
    
    /* Check if UART has been opened */
    if (!mUartModule[UartNumber].UartIsOpen)    
        return gUartErrUartNotOpen_c;
    
    /* Check if the read operation is ongoing  */
    if (mUartModule[UartNumber].UartReadOnGoing)    
        return gUartErrReadOngoing_c;
    

    /* Set the read status */
    mUartModule[UartNumber].UartRxStatus.UartReadStatus = gUartReadStatusComplete_c;
    mUartModule[UartNumber].UartRxStatus.UartNumberBytesReceived = 0;
    
    /* Set the read error flags to false */
    mUartModule[UartNumber].UartRxStatus.UartReadError.UartRecvOverrunError = FALSE;
    mUartModule[UartNumber].UartRxStatus.UartReadError.UartParityError = FALSE;
    mUartModule[UartNumber].UartRxStatus.UartReadError.UartFrameError = FALSE;
    mUartModule[UartNumber].UartRxStatus.UartReadError.UartNoiseError = FALSE;
    
    /* Set the internal variables */
    mUartModule[UartNumber].pUartRxBuffer = pBuf;
    mUartModule[UartNumber].UartNumberCharsToReceive = NumberBytes;
    
    /* read ongoing */
    mUartModule[UartNumber].UartReadOnGoing = TRUE; 

    /* Read the status registers in order to clear the error flags */
    dummy = mUartModule[UartNumber].UartRegs->S1;
    dummy = mUartModule[UartNumber].UartRegs->D;    
    (void)dummy;

#if gUART_USE_RX_FIFO_d     
    /* flush the RX FIFO */
    mUartModule[UartNumber].UartRegs->CFIFO |= gUART_RX_FIFO_FLUSH_c;  
#endif

    mUartModule[UartNumber].UartBufferLeadIndex = 0;
    mUartModule[UartNumber].UartBufferTrailIndex = 0;
    mUartModule[UartNumber].UartUnreadBytesNumber = 0;

    /* Enable the RX interrupt */
    mUartModule[UartNumber].UartRegs->C2 |= gUART_RIE_BIT_c;

    /* Exit function without error */
    return gUartErrNoError_c;
}

/******************************************************************************
 * Name: UartCancelReadData
 * Description: cancels the read data operation
 * Parameters: [IN]: UartNumber - the UART port number
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartCancelReadData
(
    uint8_t UartNumber
)
{
     /*Check if the first parameter received is valid*/ 
    if (UartNumber >= gUART_HW_MODULES_AVAILABLE_c) 
        return gUartErrWrongUartNumber_c;
    
    /* Check if UART has been opened */
    if (!mUartModule[UartNumber].UartIsOpen)    
        return gUartErrUartNotOpen_c;
    
    /* Check if the read operation is ongoing  */
    if (!mUartModule[UartNumber].UartReadOnGoing)   
        return gUartErrNoError_c;
    
    /* Set the status operation to canceled */
    mUartModule[UartNumber].UartRxStatus.UartReadStatus = gUartReadStatusCanceled_c;
    
    /*Call UartReadComplete */
    UartReadComplete(UartNumber); 
    
    /* Exit function without error */
    return gUartErrNoError_c;
}

/******************************************************************************
 * Name: UartCancelWriteData
 * Description: cancels the write data operation
 * Parameters: [IN]: UartNumber - the UART port number
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartCancelWriteData
(
    uint8_t UartNumber
)
{
    /* check if the first parameter received is valid */ 
    if (UartNumber >= gUART_HW_MODULES_AVAILABLE_c) 
        return gUartErrWrongUartNumber_c;
    
    /* Check if UART has been opened */
    if (!mUartModule[UartNumber].UartIsOpen)    
        return gUartErrUartNotOpen_c;
    
    /* Check if the read operation is ongoing */ 
    if (!mUartModule[UartNumber].UartWriteOnGoing)  
        return gUartErrNoError_c;
    
    /* set TX FIFO threshold to 0 in order for the next TX interrupt to be 
    triggered when all the characters in FIFO had been sent*/
    mUartModule[UartNumber].UartRegs->TWFIFO = 0;
        
    /* Exit function without error*/
    return gUartErrNoError_c;
}

/******************************************************************************
 * Name: UartGetStatus
 * Description: gets the status of the specified UART port
 * Parameters: [IN]: UartNumber - the UART port number
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartGetStatus
(
    uint8_t UartNumber
)
{
        /* Check if the parameter received is valid  */
    if (UartNumber >= gUART_HW_MODULES_AVAILABLE_c)     
        return gUartErrWrongUartNumber_c;
    
    /* Check if UART has been opened */ 
    if (!mUartModule[UartNumber].UartIsOpen)    
        return gUartErrUartNotOpen_c;
    
    /* Verify if the read is ongoing */
    if(mUartModule[UartNumber].UartReadOnGoing) 
        return gUartErrReadOngoing_c;
    
    /* Verify if the write is ongoing */ 
    if(mUartModule[UartNumber].UartWriteOnGoing)    
        return gUartErrWriteOngoing_c;
    
    /* no error  */
    return gUartErrNoError_c;
}

/******************************************************************************
 * Name: UartClose
 * Description: closes the specified UART port
 * Parameters: [IN] UartNumber - the UART port number
 * Return: see definition of UartErr_t
 *****************************************************************************/
static UartErr_t UartClose
(
    uint8_t UartNumber
)
{
    /* Check if the parameter received is valid */ 
    if (UartNumber >= gUART_HW_MODULES_AVAILABLE_c) 
        return gUartErrWrongUartNumber_c;
    
    /* Check if UART has been opened */
    if (mUartModule[UartNumber].UartIsOpen==FALSE)  
        return gUartErrNoError_c;
    
    /* Verify if the read is ongoing */ 
    if(TRUE == mUartModule[UartNumber].UartReadOnGoing) 
        return gUartErrReadOngoing_c;
    
    /* Verify if the write is ongoing */ 
    if(mUartModule[UartNumber].UartWriteOnGoing)    
        return gUartErrWriteOngoing_c;
    
    /* Disable UART interrupt enable flags */
    mUartModule[UartNumber].UartRegs->C2 &= ~(gUART_TIE_BIT_c | gUART_RIE_BIT_c);
    /* Disable transceiver*/
    mUartModule[UartNumber].UartRegs->C2 &= ~(gUART_TX_EN_BIT_c | gUART_RX_EN_BIT_c) ;
    /* Clear the internal variable */ 
    mUartModule[UartNumber].UartIsOpen = FALSE;
    /* Exit function with no error status */
    return gUartErrNoError_c;
}

/******************************************************************************
 * Name: UartGetByteFromRxBuffer
 * Description: gets a byte from the HW RX buffer and appends it to RAM buffer
 * Parameters: [IN] UartNumber - the UART port number
 *             [OUT] pDst - pointer to the RAM buffer
 * Return: TRUE/FALSE
 *****************************************************************************/
static bool_t UartGetByteFromRxBuffer
(
    uint8_t UartNumber, 
    uint8_t *pDst
)
{
   UartModule_t *Module = &mUartModule[UartNumber];
    
    if(0 == Module->UartUnreadBytesNumber)
    {
        return FALSE;
    }

    /* Disable the RX interrupt while the counter is updated */    
    mUartModule[UartNumber].UartRegs->C2 &= ~gUART_RIE_BIT_c;

    *pDst = mUartModule[UartNumber].pUartRxBuffer[mUartModule[UartNumber].UartBufferTrailIndex];
    if (++mUartModule[UartNumber].UartBufferTrailIndex >= mUartModule[UartNumber].UartNumberCharsToReceive) 
    {
        mUartModule[UartNumber].UartBufferTrailIndex = 0;
    }
    --mUartModule[UartNumber].UartUnreadBytesNumber;  
    
#if gUart1_Hw_Sw_FlowControl_d
    if(gUart1_c == UartNumber)
    {
        if (mUartModule[UartNumber].UartUnreadBytesNumber <= gUart1_RxFlowControlResume_d) 
        {
            Uart1_EnableFlow();
        } 
    }
#endif

#if gUart2_Hw_Sw_FlowControl_d
    if(gUart2_c == UartNumber)
    {
        if (mUartModule[UartNumber].UartUnreadBytesNumber <= gUart2_RxFlowControlResume_d) 
        {
            Uart2_EnableFlow();
        } 
    }
#endif  
       
    /* Enable the RX interrupt */   
    mUartModule[UartNumber].UartRegs->C2 |= gUART_RIE_BIT_c;

    return TRUE;
}

/******************************************************************************
 * Name: UartOpenCloseTransceiver
 * Description: opens or closes the UART transceiver
 * Parameters: [IN] UartNumber - the UART port number
 *             [IN] Pin - the bit in the control register that 
 *                  enables/disables the transmiter and the receiver
 *             [IN] Open - if TRUE, it opens the receiver/trasmitter; if FALSE
 *                         it closes the receiver/transmiter
 * Return: -
 *****************************************************************************/
static void UartOpenCloseTransceiver
(
    uint8_t UartNumber, 
    uint8_t Pin, 
    bool_t Open
)
{  
    if (Open)
    {
        mUartModule[UartNumber].UartRegs->C2 |= Pin;
    }
    else
    {
        mUartModule[UartNumber].UartRegs->C2 &= ~(uint32_t)Pin;
    }
}

/******************************************************************************
 * Name: UartClearErrors
 * Description: clears the UART receive errors
 * Parameters: [IN] UartNumber - the UART port number
 * Return: -
 *****************************************************************************/
static void UartClearErrors
(
    uint8_t UartNumber
)
{  
    register uint8_t status;
    status = mUartModule[UartNumber].UartRegs->S1;
    /* Check for errors */
    if(status & (gUART_PARITY_ERROR_FLAG_c | gUART_FRAME_ERROR_FLAG_c | gUART_NOISE_FLAG_c | gUART_OVERRUN_FLAG_c))
    {
        /* dummy read to clear UART errors */
        status = mUartModule[UartNumber].UartRegs->D;
        
#if gUART_USE_RX_FIFO_d     
    /* flush the RX FIFO */
    mUartModule[UartNumber].UartRegs->CFIFO |= gUART_RX_FIFO_FLUSH_c;  
#endif
    
#if gUART_USE_TX_FIFO_d     
    /* flush the TX FIFO */
    mUartModule[UartNumber].UartRegs->CFIFO |= gUART_TX_FIFO_FLUSH_c;  
#endif
    }
}

/******************************************************************************
 * Name: UartExecPendingCallbacks
 * Description: execute any pending UART transmit callbacks
 * Parameters: [IN] UartNumber - the UART port number
 * Return: -
 *****************************************************************************/
static void UartExecPendingCallbacks
(
        uint8_t UartNumber
)
{   

    uint8_t loopCnt;
    
#if (gUart1_Enabled_d == TRUE)
    if(UartNumber == gUart1_c)
    {
        for(loopCnt = 0; loopCnt < NumberOfElements(maSci1TxBufRefTable); loopCnt++ )
        {
            if(maSci1TxBufLenTable[loopCnt] == 0 && (NULL != maSci1TxBufRefTable[loopCnt].pfCallBack) )
            {
                /* Call callback with buffer info as parameter */
                (maSci1TxBufRefTable[loopCnt].pfCallBack)(maSci1TxBufRefTable[loopCnt].pBuf);
                /* reset the callback */
                maSci1TxBufRefTable[loopCnt].pfCallBack = NULL;
            }
        }
    }
#endif //gUart1_Enabled_d
    
#if (gUart2_Enabled_d == TRUE)        
    if (UartNumber == gUart2_c)
    {
        for(loopCnt = 0; loopCnt < NumberOfElements(maSci2TxBufRefTable); loopCnt++ )
        {
            if(maSci2TxBufLenTable[loopCnt] == 0 && (NULL != maSci2TxBufRefTable[loopCnt].pfCallBack) )
            {
                /*Call callback with buffer info as parameter*/
                (maSci2TxBufRefTable[loopCnt].pfCallBack)(maSci2TxBufRefTable[loopCnt].pBuf);
                /* reset the callback */
                maSci2TxBufRefTable[loopCnt].pfCallBack = NULL;
            }
        }
    }
#endif //gUart2_Enabled_d
}
 
/*****************************************************************************
******************************************************************************
* Public functions
******************************************************************************
*****************************************************************************/

/******************************************************************************
 * Name: Uart_ModuleInit
 * Description: Initialize the serial port(s) and module internal variables
 * Parameters: -
 * Return: -
 *****************************************************************************/
void Uart_ModuleInit
(
    void
) 
{
   uint8_t res, loopCnt;
   UartConfig_t pConfig;    


/************************
 ******** UART_1 ********
 ************************/
#if gUart1_Enabled_d
    
    /* Initialize the UART peripheral base address */
    mUartModule[gUart1_c].UartRegs = (UART_MemMapPtr)UartBaseAddress_c[gUart1_c];
    
    if(NULL == mUartModule[gUart1_c].UartRegs)
    	return;
    
    /* Enable system clock gating for UART_1 */
    gUART_1_SIM_SCG_c |= (1 << gUART_1_SIM_SCG_BIT_c);
        
    /* Enable clock gating for UART port in use */
    gUART_1_PORT_SIM_SCG_c |= gUART_1_PORT_SIM_SGC_BIT_c;
    /* Setup Port Control Register (PCR) for RX pin */
    gUART_1_RX_PCR_c = PORT_PCR_MUX(gUART_1_MUX_c);
    /* Setup Port Control Register (PCR) for TX pin */
    gUART_1_TX_PCR_c = PORT_PCR_MUX(gUART_1_MUX_c);

#if (gUart1_Hw_Sw_FlowControl_d == TRUE)
    /* Enable clock gating of the port used by CTS pin */
    gUART_1_SW_CTS_SIM_SCG_c |= (1<<gUART_1_SW_CTS_SIM_SCG_BIT_c);
    /* Setup Port Control Register (PCR) for CTS pin*/
    gUART_1_SW_CTS_PCR_c = PORT_PCR_MUX(gUART_1_SW_CTS_MUX_c);  
    
    /* Enable clock gating of the port used by RTS pin */
    gUART_1_SW_RTS_SIM_SCG_c |= (1<<gUART_1_SW_RTS_SIM_SCG_BIT_c);
    /* Setup Port Control Register (PCR) for CTS pin*/
    gUART_1_SW_RTS_PCR_c = PORT_PCR_MUX(gUART_1_SW_RTS_MUX_c);
    
    /* CTS set as input pin */
    gUART_1_SW_CTS_PDDR_c &= ~gUART_1_SW_CTS_PIN_c;     
    /* RTS set as output pin */
    gUART_1_SW_RTS_PDDR_c |= gUART_1_SW_RTS_PIN_c; 
#endif
    
    /* Open the UART_1 port */               
    res =  UartOpen(gUart1_c, gSystemClock_c * gMhzToKHzConvert_c);
    
    /* Set UART_1 communication parameters */ 
    pConfig.UartParity = gUART1_Parity_c;
    pConfig.UartDataBits = gUART1_DataBits_c;
    pConfig.UartBaudRate = gUART1_Baudrate_c;
    pConfig.UartHWFlowCtrl = gUart1_EnableHWFlowControl_c;   
    pConfig.UartRTSActiveState = gUart1_RTS_ActiveState_c;
    
    /* Apply UART_1 configuration */    
    res |= UartSetConfig(gUart1_c, &pConfig);  
    
    for(loopCnt = 0; loopCnt < gUart_TransmitBuffers_c; loopCnt++)
    {
        /* initialize TX callbacks to NULL */
        maSci1TxBufRefTable[loopCnt].pfCallBack = NULL;
    }

    /* Enables the interrupts corresponding to UART_1 driver */      
    NVIC_EnableIRQ(gUART_1_IRQ_NUM_c);
    NVIC_SetPriority(gUART_1_IRQ_NUM_c, gUart1_InterruptPriority_c);

    /* Enables the interrupts corresponding to UART_1 driver error */      
    NVIC_EnableIRQ(gUART_1_ERR_IRQ_NUM_c);
    NVIC_SetPriority(gUART_1_ERR_IRQ_NUM_c, gUart1_ErrInterruptPriority_c);

    /* Enable reception */
    UartReadData(gUart1_c, maSci1RxBuf, sizeof(maSci1RxBuf));
    
#endif /* gUart1_Enabled_d */
    

/************************
 ******** UART_2 ********
 ************************/
#if gUart2_Enabled_d
    
    /* Initialize the UART peripheral base address */
    mUartModule[gUart2_c].UartRegs = UartBaseAddress_c[gUart2_c];
    
    if(NULL == mUartModule[gUart2_c].UartRegs)
    	return;
    
    /* Enable system clock gating for UART_2 */
    gUART_2_SIM_SCG_c |= (1 << gUART_2_SIM_SCG_BIT_c);       
    
    /* Enable clock gating for UART port in use */
    gUART_2_PORT_SIM_SCG_c |= gUART_2_PORT_SIM_SGC_BIT_c;
    /* Setup Port Control Register (PCR) for RX pin */
    gUART_2_RX_PCR_c = PORT_PCR_MUX(gUART_2_MUX_c);
    /* Setup Port Control Register (PCR) for TX pin*/
    gUART_2_TX_PCR_c = PORT_PCR_MUX(gUART_2_MUX_c);
    
#if (gUart2_Hw_Sw_FlowControl_d == TRUE)
    /* Enable clock gating of the port used by CTS pin */
    gUART_2_SW_CTS_SIM_SCG_c |= (1<<gUART_2_SW_CTS_SIM_SCG_BIT_c);
    /* Setup Port Control Register (PCR) for CTS pin*/
    gUART_2_SW_CTS_PCR_c = PORT_PCR_MUX(gUART_2_SW_CTS_MUX_c);  
    
    /* Enable clock gating of the port used by RTS pin */
    gUART_2_SW_RTS_SIM_SCG_c |= (1<<gUART_2_SW_RTS_SIM_SCG_BIT_c);
    /* Setup Port Control Register (PCR) for CTS pin*/
    gUART_2_SW_RTS_PCR_c = PORT_PCR_MUX(gUART_2_SW_RTS_MUX_c);
    
    /* CTS set as input pin */
    gUART_2_SW_CTS_PDDR_c &= ~gUART_2_SW_CTS_PIN_c;     
    /* RTS set as output pin */
    gUART_2_SW_RTS_PDDR_c |= gUART_2_SW_RTS_PIN_c;
#endif
            
    /* Open the UART_2 port */            
    res =  UartOpen(gUart2_c, gSystemClock_c * gMhzToKHzConvert_c);
    
    /* Set UART_2 communication parameters */ 
    pConfig.UartParity = gUART2_Parity_c;
    pConfig.UartDataBits = gUART2_DataBits_c;
    pConfig.UartBaudRate = gUART2_Baudrate_c;
    pConfig.UartHWFlowCtrl = gUart2_EnableHWFlowControl_c;   
    pConfig.UartRTSActiveState = gUart2_RTS_ActiveState_c;
    
    /* Apply UART_2 configuration */ 
    res |= UartSetConfig(gUart2_c, &pConfig);

    for(loopCnt = 0; loopCnt < gUart_TransmitBuffers_c; loopCnt++)
    {
        /* initialize TX callbacks to NULL */
        maSci2TxBufRefTable[loopCnt].pfCallBack = NULL;
    }

    /* Enables the interrupts corresponding to UART_2 driver */      
    NVIC_EnableIRQ(gUART_2_IRQ_NUM_c);
    NVIC_SetPriority(gUART_2_IRQ_NUM_c, gUart2_InterruptPriority_c);

    /* Enables the interrupts corresponding to UART_2 driver error */      
    NVIC_EnableIRQ(gUART_2_ERR_IRQ_NUM_c);
    NVIC_SetPriority(gUART_2_ERR_IRQ_NUM_c, gUart2_ErrInterruptPriority_c);

    UartReadData(gUart2_c, maSci2RxBuf, sizeof(maSci2RxBuf));    
#endif /* gUart2_Enabled_d */

    /* Initialize the UART task */
   // gUartTaskId = TS_CreateTask(gTsUartTaskPriority_c, Uart_Task);
    
    (void)res;
}

/******************************************************************************
 * Name: Uart_ModuleUninit
 * Description: Shut down the serial port(s)
 * Parameters: -
 * Return: -
 *****************************************************************************/
void Uart_ModuleUninit
(
    void
)
{
#if gUart1_Enabled_d
    UartClose(gUart1_c);
#endif /* gUart1_Enabled_d */

#if gUart2_Enabled_d
    UartClose(gUart2_c);
#endif /* gUart2_Enabled_d */
}


/******************************************************************************
 * Name: Uart_ClearErrors
 * Description: Clear any error flags seen by the UART driver. In rare 
 *              conditions, an error flag can be set without triggering an 
 *              error interrupt, and will prevent Rx and/or Tx interrupts 
 *              from occuring. The most likely cause is a breakpoint set 
 *              during debugging, when a UART port is active.
 *              Calling this function occasionaly allows the UART code 
 *              to recover from these errors.
 * Parameters: -
 * Return: -
 ******************************************************************************/
void Uart_ClearErrors
(
    void
)
{
#if gUart1_Enabled_d
    UartClearErrors(gUart1_c);
#endif

#if gUart2_Enabled_d
    UartClearErrors(gUart2_c);
#endif
}

/******************************************************************************
 * Name: Uart1_GetByteFromRxBuffer
 * Description: Retrieve a byte from the driver's UART1 Rx buffer and store 
 *              it at *pDst
 * Parameters: [OUT] pDst - destination pointer
 * Return: TRUE if a byte was retrieved / FALSE if the Rx buffer is empty
 *****************************************************************************/
bool_t Uart1_GetByteFromRxBuffer
(
    unsigned char *pDst
) 
{
#if !gUart1_Enabled_d
    (void) pDst;
    return FALSE;
#else  
    if (uart1_ungetFlag) 
    {
        uart1_ungetFlag = FALSE;
        *pDst = uart1_ungetByte;
        return TRUE;
    } 
    else
    {  
        return UartGetByteFromRxBuffer(gUart1_c, pDst);
    }
#endif
}

/******************************************************************************
 * Name: Uart2_GetByteFromRxBuffer
 * Description: Retrieve a byte from the driver's UART2 Rx buffer and store 
 *              it at *pDst
 * Parameters: [OUT] pDst - destination pointer
 * Return: TRUE if a byte was retrieved / FALSE if the Rx buffer is empty
 *****************************************************************************/
bool_t Uart2_GetByteFromRxBuffer
(
    unsigned char *pDst
) 
{
#if !gUart2_Enabled_d
    (void) pDst;
    return 0;
#else
    if (uart2_ungetFlag)
    {
        uart2_ungetFlag = FALSE;
        *pDst = uart2_ungetByte;
        return TRUE;
    }
    else
    {
        return UartGetByteFromRxBuffer(gUart2_c, pDst);
    }
#endif
}

/******************************************************************************
 * Name: Uart1_IsTxActive
 * Description: checks if UART_1 trasmitter is active
 * Parameters: -
 * Return: TRUE if there is data still not sent
 *****************************************************************************/
bool_t Uart1_IsTxActive
(
    void
) 
{
#if !gUart1_Enabled_d
    return FALSE;
#else
    
#if gUart1_Hw_Sw_FlowControl_d
    bool_t status;

    status = (maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex] != 0) || 
    (!(mUartModule[gUart1_c].UartRegs->S1 & 0x40));

    if(TRUE == bUart1FlowConflict)
    {
    	pfSci1RxCallBack();
    	if(0 == Uart1_FlowConflict())
    	{
    		bUart1FlowConflict = FALSE;
    		if(status)
    		{
    			mUartModule[gUart1_c].UartRegs->C2 |= (gUART_TIE_BIT_c);
    		}
    	}
    }
    return status ;
#else     
    return (maSci1TxBufLenTable[mSci1TxBufRefTrailingIndex] != 0) || 
    (!(mUartModule[gUart1_c].UartRegs->S1 & 0x40));
#endif
#endif
}

/******************************************************************************
 * Name: Uart2_IsTxActive
 * Description: checks if UART_1 trasmitter is active
 * Parameters: -
 * Return: TRUE if there is data still not sent
 *****************************************************************************/
bool_t Uart2_IsTxActive
(
    void
) 
{
#if !gUart2_Enabled_d
    return FALSE;
#else   

#if gUart2_Hw_Sw_FlowControl_d
    bool_t status;

    status = (maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex] != 0) || 
    (!(mUartModule[gUart2_c].UartRegs->S1 & 0x40));

    if(TRUE == bUart2FlowConflict)
    {
        pfSci2RxCallBack();
        if(0 == Uart2_FlowConflict())
        {
            bUart2FlowConflict = FALSE;
            if(status)
            {
                mUartModule[gUart2_c].UartRegs->C2 |= (gUART_TIE_BIT_c);
            }
        }
    }
    return status ;
#else    
    return (maSci2TxBufLenTable[mSci2TxBufRefTrailingIndex] != 0) || 
    (!(mUartModule[gUart2_c].UartRegs->S1 & 0x40));
#endif
#endif
}

/******************************************************************************
 * Name: Uart_ModuleEnableLowPowerWakeup
 * Description: Enables the Receive input active edge, RXEDGIF, to generate interrupt requests.
 *              The receive input active edge detect circuit is still active on low power modes
 *              (wait and stop). An active edge on the receive input brings the CPU out of low 
 *              power mode if the interrupt is not masked.
 * Parameters: -
 * Return: -
 *****************************************************************************/
void Uart_ModuleEnableLowPowerWakeup
(
    void
)
{
#if (gUart1_Enabled_d == FALSE)
#else
  mUartModule[gUart1_c].UartRegs->BDH   &= ~(UART_BDH_RXEDGIE_MASK);
  mUartModule[gUart1_c].UartRegs->S2    &= ~(UART_S2_RXINV_MASK);
  mUartModule[gUart1_c].UartRegs->S2    |= UART_S2_RXEDGIF_MASK;
  mUartModule[gUart1_c].UartRegs->BDH   |= UART_BDH_RXEDGIE_MASK;
#endif
  
#if (gUart2_Enabled_d == FALSE)
#else
  mUartModule[gUart2_c].UartRegs->BDH   &= ~(UART_BDH_RXEDGIE_MASK);
  mUartModule[gUart2_c].UartRegs->S2    &= ~(UART_S2_RXINV_MASK);
  mUartModule[gUart2_c].UartRegs->S2    |= UART_S2_RXEDGIF_MASK;
  mUartModule[gUart2_c].UartRegs->BDH   |= UART_BDH_RXEDGIE_MASK;
#endif
}

/******************************************************************************
 * Name: Uart_ModuleDisableLowPowerWakeup
 * Description: Disables the Receive input active edge, RXEDGIF, to generate interrupt requests.
 * Parameters: -
 * Return: -
 *****************************************************************************/

void Uart_ModuleDisableLowPowerWakeup
(
    void
)
{
#if (gUart1_Enabled_d == FALSE)
#else
  mUartModule[gUart1_c].UartRegs->BDH   &= ~(UART_BDH_RXEDGIE_MASK);
  mUartModule[gUart1_c].UartRegs->S2    |= UART_S2_RXEDGIF_MASK;
#endif
  
#if (gUart2_Enabled_d == FALSE)
#else
  mUartModule[gUart2_c].UartRegs->BDH   &= ~(UART_BDH_RXEDGIE_MASK);
  mUartModule[gUart2_c].UartRegs->S2    |= UART_S2_RXEDGIF_MASK;
#endif
}

/******************************************************************************
 * Name: Uart_IsWakeUpSource
 * Description: -
 * Parameters: -
 * Return: -
 *****************************************************************************/

bool_t Uart_IsWakeUpSource
(
    void
)
{
  bool_t res = FALSE;
  uint32_t irqMaskRegister;
  irqMaskRegister = IntDisableAll();
  
#if (gUart1_Enabled_d == FALSE)
#else
  if( ( mUartModule[gUart1_c].UartRegs->S2 & UART_S2_RXEDGIF_MASK ) == UART_S2_RXEDGIF_MASK )
  {
    mUartModule[gUart1_c].UartRegs->S2 |= UART_S2_RXEDGIF_MASK;
    res = TRUE;
  }
#endif
  
#if (gUart2_Enabled_d == FALSE)
#else
  if( ( mUartModule[gUart2_c].UartRegs->S2 & UART_S2_RXEDGIF_MASK ) == UART_S2_RXEDGIF_MASK )
  {
    mUartModule[gUart2_c].UartRegs->S2 |= UART_S2_RXEDGIF_MASK;
    res = TRUE;
  }
#endif
  
  IntRestoreAll(irqMaskRegister);
  return res;
}

/******************************************************************************
 * Name: Uart1_SetRxCallBack
 * Description: Set the receive side callback function. This function will 
 *              be called whenever a byte is received by the driver. 
 *              The callback's bytesWaiting parameter is the number of bytes 
 *              available in the driver's Rx buffer.
 *              The callback function will be called in interrupt context, 
 *              so it should be kept very short.
 *              If the callback pointer is set to NULL, the Rx interrupt will 
 *              be turned off, and all data in the driver's Rx buffer will be 
 *              discarded.
 * Parameters: [IN] pfCallBack - pointer to callback function
 * Return: -
 *****************************************************************************/
void Uart1_SetRxCallBack
(
    void (*pfCallBack)(void)
)
{
#if (gUart1_Enabled_d == FALSE)
    (void) pfCallBack;
#else
    pfSci1RxCallBack = pfCallBack;
    UartOpenCloseTransceiver(gUart1_c, gUART_RX_EN_BIT_c, NULL != pfSci1RxCallBack);  
#endif
}

/******************************************************************************
 * Name: Uart2_SetRxCallBack
 * Description: Set the receive side callback function. This function will 
 *              be called whenever a byte is received by the driver. 
 *              The callback's bytesWaiting parameter is the number of bytes 
 *              available in the driver's Rx buffer.
 *              The callback function will be called in interrupt context, 
 *              so it should be kept very short.
 *              If the callback pointer is set to NULL, the Rx interrupt will 
 *              be turned off, and all data in the driver's Rx buffer will be 
 *              discarded.
 * Parameters: [IN] pfCallBack - pointer to callback function
 * Return: -
 *****************************************************************************/
void Uart2_SetRxCallBack
(
    void (*pfCallBack)(void)
) 
{
#if (gUart2_Enabled_d == FALSE)
    (void) pfCallBack;
#else
    pfSci2RxCallBack = pfCallBack;  
    UartOpenCloseTransceiver(gUart2_c, gUART_RX_EN_BIT_c, NULL != pfSci2RxCallBack);  
#endif
}

/******************************************************************************
 * Name: Uart1_SetBaud
 * Description: Sets the UART_1 baud rate
 * Parameters: [IN] baudRate - baudrate value (bps)
 * Return: -
 *****************************************************************************/
void Uart1_SetBaud
(
    uint32_t baudRate
) 
{  
#if (gUart1_Enabled_d == FALSE)
    (void) baudRate;
#else  
    UartConfig_t Cfg;
    bool_t readStopped = FALSE;
    register uint32_t i;
    UartErr_t err;

    err = UartGetStatus(gUart1_c);  
    if(err != gUartErrUartNotOpen_c)
    {  
        for (i = 0; i < 2; i++)
        {  
            if(err == gUartErrReadOngoing_c) 
            {  
                UartCancelReadData(gUart1_c);
                readStopped = TRUE;
            }
            else
            {  
                if(err == gUartErrWriteOngoing_c)
                {  
                    UartCancelWriteData(gUart1_c);
                }  
                else
                {
                    break;
                }
            }
            /* UartGetStatus is returning one error code at a time. Read back the status to check again */
            err = UartGetStatus(gUart1_c);
        }
    }
    
    UartGetConfig(gUart1_c, &Cfg);
    Cfg.UartBaudRate = baudRate;
    UartSetConfig(gUart1_c, &Cfg);

    if(TRUE == readStopped)
    {
        UartReadData(gUart1_c, maSci1RxBuf, sizeof(maSci1RxBuf));
    }    
#endif
}

/******************************************************************************
 * Name: Uart2_SetBaud
 * Description: Sets the UART_1 baud rate
 * Parameters: [IN] baudRate - baudrate value (bps)
 * Return: -
 *****************************************************************************/
void Uart2_SetBaud
(
    uint32_t baudRate
) 
{
#if (gUart2_Enabled_d == FALSE)
    (void) baudRate;
#else
    UartConfig_t Cfg;
    bool_t readStopped = FALSE;
    register uint32_t i;

    UartErr_t err = UartGetStatus(gUart2_c);  
    if(err != gUartErrUartNotOpen_c)
    {  
        for (i = 0; i < 2; i++)
        {  
            if(err == gUartErrReadOngoing_c) 
            {  
                UartCancelReadData(gUart2_c);
                readStopped = TRUE;
            }
            else
            {  
                if(err == gUartErrWriteOngoing_c)
                {  
                    UartCancelWriteData(gUart2_c);
                }  
                else
                {
                    break;
                }
            }
            /* UartGetStatus is returning one error code at a time. Read back the status to check again */
            err = UartGetStatus(gUart2_c);
        }
    }
    
	UartGetConfig(gUart2_c, &Cfg);
	Cfg.UartBaudRate = baudRate;
	UartSetConfig(gUart2_c, &Cfg);

    if(TRUE == readStopped)
    {
        UartReadData(gUart2_c, maSci2RxBuf, sizeof(maSci2RxBuf));
    } 
#endif
}

/******************************************************************************
 * Name: Uart1_Transmit
 * Description: Transmit bufLen bytes of data from pBuffer over UART_1 port. 
 *              It calls *pfCallBack() when the entire buffer has been sent. 
 *              Returns FALSE if there are no more available Tx buffer slots, 
 *              TRUE otherwise. The caller must ensure that the buffer remains
 *              available until the call back function is called.
 *              pfCallBack must not be NULL.
 *              The callback function will be called in interrupt context, 
 *              so it should be kept very short.
 * Parameters: [IN] pBuf - pointer to the buffer to be transmitted
 *             [IN] bufLen - buffer length
 *             [IN] pfCallBack - pointer to the callback function
 * Return: Returns FALSE if there are no more slots in the buffer reference 
 *         table
 *****************************************************************************/
bool_t Uart1_Transmit
(
    unsigned char const *pBuf, 
    index_t bufLen, 
    void (*pfCallBack)(unsigned char const *pBuf)
) 
{
#if (gUart1_Enabled_d == FALSE)
    (void) pBuf;
    (void) bufLen;
    (void) pfCallBack;
    return FALSE;
#else
    /* Handle empty buffers */
    if (!bufLen) 
    {
    	if(pfCallBack)
    		(*pfCallBack)(pBuf);    		 
    	
    	return TRUE;
    }
    
    if (maSci1TxBufLenTable[mSci1TxBufRefLeadingIndex])
    	return FALSE;
    
    /* Room for one more */
    
    /* Handle the case when a previous message was transmitted but the callback was not yet executed. 
     * The buffer length of the new message must be stored before executing the callback because
     * the transmit function (this function) can be called again from that callback */
      
    maSci1TxBufLenTable[mSci1TxBufRefLeadingIndex] = bufLen;
    
    if(maSci1TxBufRefTable[mSci1TxBufRefLeadingIndex].pfCallBack)
    	maSci1TxBufRefTable[mSci1TxBufRefLeadingIndex].pfCallBack(maSci1TxBufRefTable[mSci1TxBufRefLeadingIndex].pBuf);

    /* store the new message to be transmitted */
    maSci1TxBufRefTable[mSci1TxBufRefLeadingIndex].pBuf = pBuf;
    maSci1TxBufRefTable[mSci1TxBufRefLeadingIndex].pfCallBack = pfCallBack;     
    
    if (++mSci1TxBufRefLeadingIndex >= NumberOfElements(maSci1TxBufRefTable)) 
    {
    	mSci1TxBufRefLeadingIndex = 0;
    }

    /* Set the write ongoing flag */
    mUartModule[gUart1_c].UartWriteOnGoing = TRUE;
    
    /* 
     * Enable the transmit interrupt. 
     * Harmless if the interrupt is already enabled. 
     */
    mUartModule[gUart1_c].UartRegs->C2 |= gUART_TIE_BIT_c;
           
    return TRUE;
#endif /* gUart1_Enabled_d */
}

/******************************************************************************
 * Name: Uart2_Transmit
 * Description: Transmit bufLen bytes of data from pBuffer over UART_2 port. 
 *              It calls *pfCallBack() when the entire buffer has been sent. 
 *              Returns FALSE if there are no more available Tx buffer slots, 
 *              TRUE otherwise. The caller must ensure that the buffer remains
 *              available until the call back function is called.
 *              pfCallBack must not be NULL.
 *              The callback function will be called in interrupt context, 
 *              so it should be kept very short.
 * Parameters: [IN] pBuf - pointer to the buffer to be transmitted
 *             [IN] bufLen - buffer length
 *             [IN] pfCallBack - pointer to the callback function
 * Return: Returns FALSE if there are no more slots in the buffer reference 
 *         table
 *****************************************************************************/
bool_t Uart2_Transmit
(
    unsigned char const *pBuf, 
    index_t bufLen, 
    void (*pfCallBack)(unsigned char const *pBuf)
)
{
#if (gUart2_Enabled_d == FALSE)
    (void) pBuf;
    (void) bufLen;
    (void) pfCallBack;
    return FALSE;
#else

    /* Handle empty buffers */
    if (!bufLen) 
    {
    	if(pfCallBack)    	
    		(*pfCallBack)(pBuf);
    		
    	return TRUE;
    }
    
    if (maSci2TxBufLenTable[mSci2TxBufRefLeadingIndex])
    	return FALSE;
    
    /* Room for one more */
    
    /* Handle the case when a previous message was transmitted but the callback was not yet executed. 
     * The buffer length of the new message must be stored before executing the callback because
     * the transmit function (this function) can be called again from that callback */
    
    maSci2TxBufLenTable[mSci2TxBufRefLeadingIndex] = bufLen;
        
    if(maSci2TxBufRefTable[mSci2TxBufRefLeadingIndex].pfCallBack)
    	maSci2TxBufRefTable[mSci2TxBufRefLeadingIndex].pfCallBack(maSci2TxBufRefTable[mSci2TxBufRefLeadingIndex].pBuf);

    /* store the new message to be transmitted */
    maSci2TxBufRefTable[mSci2TxBufRefLeadingIndex].pBuf = pBuf;
    maSci2TxBufRefTable[mSci2TxBufRefLeadingIndex].pfCallBack = pfCallBack;     
    

    if (++mSci2TxBufRefLeadingIndex >= NumberOfElements(maSci2TxBufRefTable)) 
    {
    	mSci2TxBufRefLeadingIndex = 0;
    }

    /* Set the write ongoing flag */
    mUartModule[gUart2_c].UartWriteOnGoing = TRUE;

    /* 
     * Enable the transmit interrupt. 
     * Harmless if the interrupt is already enabled. 
     */
    mUartModule[gUart2_c].UartRegs->C2 |= gUART_TIE_BIT_c; 
   
    return TRUE;
#endif  /* gUart2_Enabled_d */
}                                      

/******************************************************************************
 * Name: Uart1_UngetByte
 * Description: Unget a byte: store the byte so that it will be returned by 
 *              the next call to the get byte function. 
 *              Also call the Rx callback
 * Parameters: [IN] byte - the byte to be stored
 * Return: -
 *****************************************************************************/
void Uart1_UngetByte
(
    unsigned char byte
) 
{
#if (gUart1_Enabled_d == FALSE)
    (void) byte;
#else
    uart1_ungetFlag = TRUE;
    uart1_ungetByte = byte;

    /* Likewise, it is the caller's responsibility to not call this function */
    /* if the callback pointer is not valid. */
    if(pfSci1RxCallBack != NULL)
    {  
        pfSci1RxCallBack();
    } 
#endif /* gUart1_Enabled_d */
}


/******************************************************************************
 * Name: Uart2_UngetByte
 * Description: Unget a byte: store the byte so that it will be returned by 
 *              the next call to the get byte function. 
 *              Also call the Rx callback
 * Parameters: [IN] byte - the byte to be stored
 * Return: -
 *****************************************************************************/
void Uart2_UngetByte
(
    unsigned char byte
) 
{
#if (gUart2_Enabled_d == FALSE)
    (void) byte;
#else
    uart2_ungetFlag = TRUE;
    uart2_ungetByte = byte;

    /* Likewise, it is the caller's responsibility to not call this function */
    /* if the callback pointer is not valid. */
    if(pfSci2RxCallBack != NULL)
    {  
        pfSci2RxCallBack();
    } 
#endif  /* gUart2_Enabled_d */
}

/******************************************************************************
 * Name: Uart1_RxBufferByteCount
 * Description: Return unread bytes number from UART_1 buffer
 * Parameters: -
 * Return: the number of unread bytes 
 *****************************************************************************/
uint32_t Uart1_RxBufferByteCount
(
    void
) 
{
    return mUartModule[gUart1_c].UartUnreadBytesNumber;
}

/******************************************************************************
 * Name: Uart2_RxBufferByteCount
 * Description: Return unread bytes number from UART_2 buffer
 * Parameters: -
 * Return: the number of unread bytes 
 *****************************************************************************/
uint32_t Uart2_RxBufferByteCount
(
    void
) 
{
    	return mUartModule[gUart2_c].UartUnreadBytesNumber;
}

/******************************************************************************
 * Name: Uart1Isr
 * Description: UART_1 Interrupt Service Routine
 * Parameters: -
 * Return: -
 *****************************************************************************/
void Uart1Isr
(
    void
)
{
     /* Store the status register in an internal variable, because a read of this register clears all the error and interrupt flags */
    mUartModule[gUart1_c].UartUstatRegister1 = mUartModule[gUart1_c].UartRegs->S1;
    mUartModule[gUart1_c].UartUstatRegister2 = mUartModule[gUart1_c].UartRegs->S2;
    if( (mUartModule[gUart1_c].UartUstatRegister2 & UART_S2_RXEDGIF_MASK) &&
        (mUartModule[gUart1_c].UartRegs->BDH & UART_BDH_RXEDGIE_MASK) )
    {
      mUartModule[gUart1_c].UartRegs->S2 |= UART_S2_RXEDGIF_MASK;
    }

    /*If Rx module is enabled and Rx interrupt pending flag is set and Rx interrupt is enabled */
    if((mUartModule[gUart1_c].UartRegs->C2 & gUART_RX_EN_BIT_c) &&
            (mUartModule[gUart1_c].UartUstatRegister1 & gUART_RX_INT_BIT_c)  && 
            (mUartModule[gUart1_c].UartRegs->C2 & gUART_RIE_BIT_c))
    {
        UartReceiverIsr(gUart1_c);
    }
    /*If Tx module is enabled and Tx interrupt pending flag is set and Tx interrupt is enabled */
    if((mUartModule[gUart1_c].UartRegs->C2 & gUART_TX_EN_BIT_c) &&
            (mUartModule[gUart1_c].UartUstatRegister1 & gUART_TX_INT_BIT_c)  &&
            (mUartModule[gUart1_c].UartRegs->C2 & gUART_TIE_BIT_c))
    {
        UartTransmitterIsr(gUart1_c);
    }
}

/******************************************************************************
 * Name: Uart2Isr
 * Description: UART_2 Interrupt Service Routine
 * Parameters: -
 * Return: -
 *****************************************************************************/
void Uart2Isr
(
    void
)
{
     /* Store the status register in an internal variable, because a read of this register clears all the error and interrupt flags */
    mUartModule[gUart2_c].UartUstatRegister1 = mUartModule[gUart2_c].UartRegs->S1;
    mUartModule[gUart2_c].UartUstatRegister2 = mUartModule[gUart2_c].UartRegs->S2;
    if( (mUartModule[gUart2_c].UartUstatRegister2 & UART_S2_RXEDGIF_MASK) &&
        (mUartModule[gUart2_c].UartRegs->BDH & UART_BDH_RXEDGIE_MASK) )
    {
      mUartModule[gUart2_c].UartRegs->S2 |= UART_S2_RXEDGIF_MASK;
    }

    /* If Rx module is enabled and Rx interrupt pending flag is set and Rx interrupt is enabled */
    if((mUartModule[gUart2_c].UartRegs->C2 & gUART_RX_EN_BIT_c) &&
            (mUartModule[gUart2_c].UartUstatRegister1 & gUART_RX_INT_BIT_c) && 
            (mUartModule[gUart2_c].UartRegs->C2 & gUART_RIE_BIT_c))
    {                    
        UartReceiverIsr(gUart2_c);
    }
    /* If Tx module is enabled and Tx interrupt pending flag is set and Tx interrupt is enabled*/
    if((mUartModule[gUart2_c].UartRegs->C2 & gUART_TX_EN_BIT_c) &&
            (mUartModule[gUart2_c].UartUstatRegister1 & gUART_TX_INT_BIT_c) &&
            (mUartModule[gUart2_c].UartRegs->C2 & gUART_TIE_BIT_c))
    {
        UartTransmitterIsr(gUart2_c);
    }
}

/******************************************************************************
 * Name: Uart1ErrorIsr
 * Description: Clear enabled UART_1 errors
 * Parameters: -
 * Return: -
 *****************************************************************************/
void Uart1ErrorIsr
(
    void
) 
{
	/* Since the flag checking mechanism will clear any flags in a register
	 * that are set, we have to save the value of the status register
	 * and then check against the saved value to be able to detect all of
	 * the flags that were set (if you read the status register over and 
	 * over again, then you'll only capture the first one that was set.
	 */

	uint8_t status, dummy;

	status = mUartModule[gUart1_c].UartRegs->S1;

	if(status & (gUART_PARITY_ERROR_FLAG_c | gUART_FRAME_ERROR_FLAG_c | gUART_NOISE_FLAG_c | gUART_OVERRUN_FLAG_c))
	{
		/* dummy read to clear UART errors */
		dummy = mUartModule[gUart1_c].UartRegs->D;
	}

	if (mUartModule[gUart1_c].UartRegs->SFIFO & (UART_SFIFO_TXOF_MASK))
	{
		mUartModule[gUart1_c].UartRegs->SFIFO |= UART_SFIFO_TXOF_MASK;
	}

	if (mUartModule[gUart1_c].UartRegs->SFIFO & (UART_SFIFO_RXUF_MASK))
	{    	
		mUartModule[gUart1_c].UartRegs->SFIFO |= UART_SFIFO_RXUF_MASK;    	
	}
	
#if gUART_USE_RX_FIFO_d     
    /* flush the RX FIFO */
    mUartModule[gUart1_c].UartRegs->CFIFO |= gUART_RX_FIFO_FLUSH_c;  
#endif
    
#if gUART_USE_TX_FIFO_d     
    /* flush the TX FIFO */
    mUartModule[gUart1_c].UartRegs->CFIFO |= gUART_TX_FIFO_FLUSH_c;  
#endif
        
    (void)dummy;
}

/******************************************************************************
 * Name: Uart2ErrorIsr
 * Description: Clear enabled UART_2 errors
 * Parameters: -
 * Return: -
 *****************************************************************************/
void Uart2ErrorIsr
(
    void
) 
{	
	/* Since the flag checking mechanism will clear any flags in a register
	 * that are set, we have to save the value of the status register
	 * and then check against the saved value to be able to detect all of
	 * the flags that were set (if you read the status register over and 
	 * over again, then you'll only capture the first one that was set.
	 */
    
	uint8_t status, dummy;

    status = mUartModule[gUart2_c].UartRegs->S1;
   
    if(status & (gUART_PARITY_ERROR_FLAG_c | gUART_FRAME_ERROR_FLAG_c | gUART_NOISE_FLAG_c | gUART_OVERRUN_FLAG_c))
    {
        /* dummy read to clear UART errors */
    	dummy = mUartModule[gUart2_c].UartRegs->D;
    }
            
    if (mUartModule[gUart2_c].UartRegs->SFIFO & (UART_SFIFO_TXOF_MASK))
    {
    	mUartModule[gUart2_c].UartRegs->SFIFO |= UART_SFIFO_TXOF_MASK;
    }
    
    if (mUartModule[gUart2_c].UartRegs->SFIFO & (UART_SFIFO_RXUF_MASK))
    {    	
    	mUartModule[gUart2_c].UartRegs->SFIFO |= UART_SFIFO_RXUF_MASK;    	
    }
    
#if gUART_USE_RX_FIFO_d     
    /* flush the RX FIFO */
    mUartModule[gUart2_c].UartRegs->CFIFO |= gUART_RX_FIFO_FLUSH_c;  
#endif
    
#if gUART_USE_TX_FIFO_d     
    /* flush the TX FIFO */
    mUartModule[gUart2_c].UartRegs->CFIFO |= gUART_TX_FIFO_FLUSH_c;  
#endif
    
    (void)dummy;
}

#endif /* gUart1_Enabled_d || gUart2_Enabled_d */
