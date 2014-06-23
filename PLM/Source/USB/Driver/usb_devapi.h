/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2010 Freescale Semiconductor, Inc.
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
 * @file usb_devapi.h
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief This file contains S08 USB stack device layer API header function.
 *
 *****************************************************************************/

#ifndef _USB_DEVAPI_H
#define _USB_DEVAPI_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "EmbeddedTypes.h"          /* User Defined Data Types */
#include "PortConfig.h"
/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/

#define gUsbMaxEpBufferSize_d     (512)
#define gUsbControlEndpoint_d     (0)     /* control endpoint number */
#define gUsbSetupPacketSize_d     (8)     /* Setup Packet Size */
        
/* Device States */
#define  gUsbDeviceState_Unknown_d                   (0xff)
#define  gUsbDeviceState_PendingAddress_d            (0x04)
#define  gUsbDeviceState_Powered_d                   (0x03)
#define  gUsbDeviceState_Default_d                   (0x02)
#define  gUsbDeviceState_Address_d                   (0x01)
#define  gUsbDeviceState_Config_d                    (0x00)
#define  gUsbDeviceState_Suspend_d                   (0x80)

/* possible values of Status */
#define  gUsbStatus_Idle_d                     (0)
#define  gUsbStatus_TransferAccepted_d         (6)
#define  gUsbStatus_TransferPending_d          (2)
#define  gUsbStatus_TransferInProgress_d       (3)
#define  gUsbStatus_Error_d                    (4)
#define  gUsbStatus_Disabled_d                 (5)
#define  gUsbStatus_Stalled_d                  (1)
#define  gUsbStatus_TransferQueued_d          (7)
#define  gUsbStatus_Unknown_d                  (0xFFFF)

/* Available service types */

#define  gUsbService_Ep0_d                     (0x00)
#define  gUsbService_Ep1_d                     (0x01)
#define  gUsbService_Ep2_d                     (0x02)
#define  gUsbService_Ep3_d                     (0x03)
#define  gUsbService_Ep4_d                     (0x04)
#define  gUsbService_Ep5_d                     (0x05)
#define  gUsbService_Ep6_d                     (0x06)
#define  gUsbService_Ep7_d                     (0x07)
#define  gUsbService_Ep8_d                     (0x08)
#define  gUsbService_Ep9_d                     (0x09)
#define  gUsbService_Ep10_d                    (0x0a)
#define  gUsbService_Ep11_d                    (0x0b)
#define  gUsbService_Ep12_d                    (0x0c)
#define  gUsbService_Ep13_d                    (0x0d)
#define  gUsbService_Ep14_d                    (0x0e)
#define  gUsbService_Ep15_d                    (0x0f)
#define  gUsbService_EpMax_d                   (gNumMaxEnpNumber_d - 1)
#define  gUsbService_BusReset_d                (gUsbService_EpMax_d + 1)
#define  gUsbService_SOF_d                     (gUsbService_BusReset_d + 1)
#define  gUsbService_Resume_d                  (gUsbService_SOF_d + 1)
#define  gUsbService_Sleep_d                   (gUsbService_Resume_d + 1)
#define  gUsbService_Error_d                   (gUsbService_Sleep_d + 1)
#define  gUsbService_Stall_d                   (gUsbService_Error_d + 1)
#define  gUsbService_Max_d                     (gUsbService_Stall_d + 1)

/* Informational Request/Set Types */
/* component parameter in USB_Device_Get/Set_Status */
#define  gUsbComponent_DirectionShift_d                 (7)
#define  gUsbComponent_DirectionMask_d                 (0x01)
#define  gUsbComponentStatus_DeviceState_d             (0x01)
#define  gUsbComponentStatus_Interface_d               (0x02)
#define  gUsbComponentStatus_Address_d                 (0x03)
#define  gUsbComponentStatus_CurrentConfig_d           (0x04)
#define  gUsbComponentStatus_SOFCount_d                (0x05)
#define  gUsbComponentStatus_Device_d                  (0x06)
#define  gUsbComponentStatus_TestMode_d                (0x07)
#define  gUsbComponentStatus_Endpoint_d                (0x10)
#define  gUsbComponentStatus_EpNumberMask_d            (0x0F)


#define gUninitialisedVal_d                    (0xffffffff)

/******************************************************************************
 * Types
 *****************************************************************************/


typedef struct _usbDeviceEvent_t
{
  
  uint8_t          controllerId;      /* controller ID           */
  uint8_t          epNum;
  bool_t           setup;              /* is setup packet         */
  bool_t           direction;          /* direction of endpoint   */
  uint8_t*         pBuffer;         /* pointer to buffer       */
  usbPacketSize_t  len;                /* buffer size of endpoint */
  uint8_t          errors;             /* Any errors              */
  bool_t           shortSend;
} usbDeviceEvent_t ;

typedef void(* usbServiceCallback_t)(usbDeviceEvent_t *);


/******************************************************************************
 * Global Functions
 *****************************************************************************/
extern uint8_t USB_DeviceInit (
    uint8_t    controllerId,
    uint8_t    endpoints
);

uint8_t USB_DeviceDeinit (
    uint8_t    controllerId /* [IN] Controller ID */
);

extern uint8_t USB_DeviceInitEndpoint(
    uint8_t           controllerId,
    usbEpStruct_t*    pUsbEpStruct,
    uint8_t           flag
);

extern uint8_t USB_DeviceCancelTransfer (
    uint8_t    controllerId,
    uint8_t    epNum,
    uint8_t    direction
);

extern void  USB_DeviceDisableController (
    uint8_t    controllerId    /* [IN] Controller ID */
   ) ;
extern void  USB_DeviceEnableController (
    uint8_t    controllerId    /* [IN] Controller ID */
   ) ;   
extern uint8_t USB_DeviceDeinitEndpoint (
    uint8_t    controllerId,
    uint8_t    epNum,
    uint8_t    direction
);

extern uint8_t USB_DeviceRecvData (
    uint8_t           controllerId,
    uint8_t           epNum,
    uint8_t*          pBuff,
    usbPacketSize_t   size
);

extern uint8_t USB_DeviceSendData (
    uint8_t           controllerId,
    uint8_t           epNum,
    uint8_t*          pBuff,
    usbPacketSize_t   size,
    bool_t            sendShort
);

extern void USB_DeviceShutdown(uint8_t  controllerId);

extern void USB_DeviceStallEndpoint (
    uint8_t    controllerId,
    uint8_t    epNum,
    uint8_t    direction
);

extern void USB_DeviceUnstallEndpoint (
    uint8_t    controllerId,
    uint8_t    epNum,
    uint8_t    direction
);

extern void USB_DeviceReadSetupData (
    uint8_t    controllerId,
    uint8_t    epNum,
    uint8_t*   pBuff
);

extern uint8_t USB_DeviceGetStatus (
    uint8_t    controllerId,
    uint8_t    component,
    uint16_t*  status
);

extern uint8_t USB_DeviceSetStatus (
    uint8_t     controllerId,
    uint8_t     component,
    uint16_t    setting
);

extern void USB_DeviceAssertResume(uint8_t  controllerId);

extern uint8_t USB_DeviceRegisterService (
    uint8_t                  controllerId,
    uint8_t                  type,
    usbServiceCallback_t     service
);

extern uint8_t USB_DeviceUnregisterService (
    uint8_t    controllerId,
    uint8_t    type
);

extern uint8_t USB_DeviceGetTransferStatus (
    uint8_t    controllerId,
    uint8_t    epNum,
    uint8_t    direction
);

extern void  USB_DeviceSetAddress (
    uint8_t    controllerId,
    uint8_t    address
);

extern uint8_t USB_DeviceCallService(
    uint8_t             type,
    usbDeviceEvent_t *  pUsbDeviceEvent
);

#endif
