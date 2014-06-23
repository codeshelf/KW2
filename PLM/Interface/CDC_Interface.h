/*****************************************************************************
* Virtual Com Application interface.
*
* Copyright (c) 2012, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

/* On the receive side, this module keeps a small circular buffer, and calls
 * a callback function whenever a byte is received. The application can
 * retrieve bytes from the buffer at its convenience, as long as it does so
 * before the driver's buffer fills up.
 *
 * On the transmit side, this module keeps a list of buffers to be
 * transmitted, and calls an application callback function when the entire
 * buffer has been sent. The application is responsible for ensuring that the
 * buffer is available until the callback function is called.
*/

#ifndef _CDC_Interface_h_
#define _CDC_Interface_h_


/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/

/* Default setting for debugging code (includes high-water mark checking). */
#ifndef gCDC_Debug_d
#define gCDC_Debug_d       FALSE
#endif

/*****************************************************************************/


/* if gVirtualCOMPort_d == FALSE the entire virtual com application is excluded from build */

 #ifndef gVirtualCOMPort_d
  #define gVirtualCOMPort_d    TRUE
 #endif

/* if gVirtualCOMPortSerialNoEnable_d == TRUE the Virtual Com device contains a Serial Number String Descriptor */

 #ifndef gVirtualCOMPortSerialNoEnable_d
  #define gVirtualCOMPortSerialNoEnable_d                       FALSE
 #endif

/* Please keep unchanged the length and the structure (every Serial No character must be followed by a 0x00 character ) of this define. Modify its value only. */
/* Eg:for a serial number = 12345678 the define value is  0x31,0x00,0x32,0x00,0x33,0x00,0x34,0x00,0x35,0x00,0x36,0x00,0x37,0x00,0x38,0x00*/ 
#ifndef gDefaultValueOfVirtualCOMPortSerialNo_c
                                                            /*00000001*/
 #define gDefaultValueOfVirtualCOMPortSerialNo_c            0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x31,0x00
#endif

/* Keep this define TRUE. When FALSE the RX buffer may be overwritten */
#ifndef gCDC_EnableFlowControl_d
#define gCDC_EnableFlowControl_d     TRUE
#endif

/*****************************************************************************/

/* Tuning definitions. */

/* Number of entries in the transmit-buffers-in-waiting list. */
#ifndef gCDC_TransmitBuffers_c
#define gCDC_TransmitBuffers_c     3
#endif

/* Size of the driver's Rx circular buffer. These buffers are used to */
/* hold received bytes until the application can retrieve them via the */
/* CDC_GetByteFromRxBuffer() functions, and are not otherwise accessible */
/* from outside the driver.  */
#define gCDC_ReceiveBufferSize_c   (gUsbCdcBulkOutEpSize_d*4)

/* If flow control is used, there is a delay before telling the far side */
/* to stop and the far side actually stopping. When there are SKEW free bytes */
/* remaining in the driver's Rx buffer, tell the far side to stop */
/* transmitting. */
#define gCDC_RxFlowControlSkew_d   (gUsbCdcBulkOutEpSize_d)

/* Number of bytes left in the Rx buffer when hardware flow control is */
/* deasserted. */
#ifndef gCDC_RxFlowControlResume_d
#define gCDC_RxFlowControlResume_d 8
#endif

/* Dummy Baud Rate Definition for consistency across serial interfaces*/ 
#ifndef gUartDefaultBaud_c
  #define gUartDefaultBaud_c  0x0000  
#endif
#define gCDCDefaultBaud_c 115200UL

/*****************************************************************************/

/*****************************************************************************
******************************************************************************
* Public memory declarations
******************************************************************************
*****************************************************************************/

/* number of bytes in the Rx buffer. */ 
extern volatile index_t gCDCRxBufferByteCount;       



/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/

#if !gVirtualCOMPort_d
/* stub functions if disabled */

#define CDC_ModuleInit()   TRUE
#define CDC_ModuleUninit() TRUE
#define CDC_Transmit(pBuf, bufLen, pfCallBack) FALSE
#define CDC_IsTxActive() FALSE
#define CDC_SetRxCallBack(pfCallBack)
#define CDC_SetUsbSuspendCallBack(pfCallBack)
#define CDC_GetByteFromRxBuffer(pDst) FALSE
#else //!gVirtualCOMPort_d



/*****************************************************************************
* Initialize and attach the virtual com device
*****************************************************************************/
extern bool_t CDC_ModuleInit(void);

/*****************************************************************************
* Detach the virtual com device 
*****************************************************************************/
extern bool_t CDC_ModuleUninit(void);

/*****************************************************************************
* Transmit bufLen bytes of data from pBuffer over the virtual com port.  
* Call *pfCallBack() when the entire buffer has been sent. Returns FALSE 
*if there are no more available Tx buffer slots, TRUE otherwise. The caller 
* must ensure that the buffer remains available until the callback function 
*is called. 
*****************************************************************************/
extern bool_t CDC_Transmit(unsigned char const *pBuf, index_t bufLen, void (*pfCallBack)(unsigned char const *pBuf));

/*****************************************************************************
* Return TRUE if there are any bytes in transmit buffers that have not 
* yet been sent. Return FALSE otherwise. 
*****************************************************************************/
extern bool_t CDC_IsTxActive(void);

/*****************************************************************************
* Set the receive side callback function. This function will be called 
* whenever a byte is received by the driver. 
* The callback function will be called in non interrupt context.  
* If the callback pointer is set to NULL, received characters will be discarded. 
*****************************************************************************/
extern void CDC_SetRxCallBack(void (*pfCallBack)(void));

/*****************************************************************************
* Set the USB suspend callback function. This function will be called 
* whenever the Host suspends the CDC device. 
* The callback function will be called in non interrupt context.  
* When the host suspends it, the device should enter in low power.
*****************************************************************************/
extern void CDC_SetUsbSuspendCallBack(void (*pfCallBack)(void));

/*****************************************************************************
* Retrieve one byte from the driver's Rx buffer and store it at *pDst. 
* Return TRUE if a byte was retrieved; FALSE if the Rx buffer is empty.
*****************************************************************************/
extern bool_t CDC_GetByteFromRxBuffer(unsigned char *pDst);

/*****************************************************************************
*Verify CDC is Open
*****************************************************************************/
extern bool_t CDC_CommOpen(void);

#ifndef NumberOfElements
#define NumberOfElements(array)     ((sizeof(array) / (sizeof(array[0]))))
#endif

#endif //!gVirtualCOMPort_d


#endif //_CDC_Interface_h_

