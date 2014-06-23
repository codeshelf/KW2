/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2009 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 ******************************************************************************
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
 **************************************************************************//*!
 *
 * @file usb_dciapi.h
 *
 * @author 
 *
 * @version 
 *
 * @date Jun-05-2009
 *
 * @brief The file contains DCI api function definetions .
 *
 *****************************************************************************/

#ifndef _USB_DCIAPI_H
#define _USB_DCIAPI_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "EmbeddedTypes.h"
#include "usb_devapi.h"

/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/
#define gNumMaxEnpNumber_d     16 
#define gNumMinEnpNumber_d     1

/******************************************************************************
 * Types
 *****************************************************************************/


 /*****************************************************************************
 * Global Functions
 *****************************************************************************/
extern uint8_t USB_DCI_Init (uint8_t    controllerId);
extern uint8_t USB_DCI_DeInit(uint8_t    controllerId);
extern uint8_t USB_DCI_InitEndpoint(uint8_t controller_ID, usbEpStruct_t* pEpStruct, bool_t flag);
extern uint8_t USB_DCI_CancelTransfer( uint8_t controllerId, uint8_t endpNumber, uint8_t direction);
extern uint8_t USB_DCI_DeinitEndpoint(uint8_t controllerId, uint8_t epNum, uint8_t direction);
extern void USB_DCI_StallEndpoint( uint8_t controllerId, uint8_t endpNum, uint8_t direction);
extern void USB_DCI_UnstallEndpoint( uint8_t controllerId, uint8_t endpNumber, uint8_t direction);
extern void USB_DCI_GetSetupData(uint8_t controllerId, uint8_t  endpNumber, uint8_t* pBuff);
extern uint8_t USB_DCI_GetTransferStatus( uint8_t controllerId, uint8_t endpNumber, uint8_t direction);
extern uint8_t USB_DCI_RecvData(uint8_t controllerId, uint8_t endpNumber, uint8_t* pBuff, usbPacketSize_t size);
extern uint8_t USB_DCI_SendData(uint8_t controllerId, uint8_t endpNumber, uint8_t* pBuff, usbPacketSize_t size, bool_t sendShort);
extern void  USB_DCI_SetAddress(uint8_t controllerId, uint8_t address);
extern void USB_DCI_AssertResume(uint8_t controllerId);


#define USB_DCI_CancelTransfer USB_DeviceCancelTransfer  

#define USB_DCI_RecvData USB_DeviceRecvData  

#define USB_DCI_SendData USB_DeviceSendData    

#define USB_DCI_StallEndpoint USB_DeviceStallEndpoint  

#define USB_DCI_UnstallEndpoint USB_DeviceUnstallEndpoint  

#define USB_DCI_GetTransferStatus USB_Device_GetTransferStatus       

#define USB_DCI_GetSetupData USB_Device_ReadSetupData 

#define USB_DCI_SetAddress USB_DeviceSetAddress  

#define USB_DCI_AssertResume USB_DeviceAssertResume

#endif


