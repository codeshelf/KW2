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
 * @file usb_framework.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief The file contains USB stack framework module implementation.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "EmbeddedTypes.h"          /* Contains User Defined Data Types */
#include "USB_Configuration.h" 
#include "usb_class.h"      /* USB class Header File */
#include "usb_framework.h"  /* USB Framework Header File */
#include "usb_descriptor.h" /* USB descriptor Header File */
#include "USB.h"
#include "Utilities_Interface.h"

#if gUsbIncluded_d
/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/
/****************************************************************************
 * Global Variables
 ****************************************************************************/
static usbSetupPacket_t* gpaSetupPkt[gNumUsbControllers_d];
/*is used to store the value of data which needs to be sent to the USB
host in response to the standard requests.*/
static uint16_t gStdFrameworkData;
/*used to store the address received in Set Address in the standard request.*/
static uint8_t  gAssignedAddress;
/* Framework module callback pointer */
#define  gpfFrameworkCallback(controller_ID) gpfClassFrameworkCallback
#define  gpfOtherReqCallback(controller_ID) gpfOtherReqCallback_
static pfusbReqHandler_t gpfOtherReqCallback_ = NULL;

/* Other Requests Callback pointer */


bool_t const gValidateRequest[gMaxStrdReq_d][3] =
{
  {TRUE,TRUE,FALSE}, /*USB_StrdReq_GetStatus*/
         /* configured state: valid for existing interfaces/endpoints
            address state   : valid only for interface or endpoint 0
            default state   : not specified
         */
  {TRUE,TRUE,FALSE}, /* Clear Feature */
         /* configured state: valid only for device in configured state
            address state   : valid only for device (in address state),
                              interface and endpoint 0
            default state   : not specified
         */
  {FALSE,FALSE,FALSE}, /*reserved for future use*/
         /* configured state: request not supported
            address state   : request not supported
            default state   : request not supported
         */
  {TRUE,TRUE,FALSE}, /* Set Feature */
        /*  configured state: valid only for device in configured state
            address state   : valid only for interface or endpoint 0
            default state   : not specified
        */
        
  {FALSE,FALSE,FALSE}, /*reserved for future use*/
        /*  configured state: request not supported
            address state   : request not supported
            default state   : request not supported
         */
  {FALSE,TRUE,TRUE}, /*USB_StrdReq_SetAddress*/
        /*  configured state: not specified
            address state   : changes to default state if specified addr == 0,
                              but uses newly specified address
            default state   : changes to address state if specified addr != 0
         */
  {TRUE,TRUE,TRUE}, /*USB_StrdReq_GetDescriptor*/
         /* configured state: valid request
            address state   : valid request
            default state   : valid request
         */
  {FALSE,FALSE,FALSE}, /*Set Descriptor*/
         /* configured state: request not supported
            address state   : request not supported
            default state   : request not supported
         */
  {TRUE,TRUE,FALSE}, /*USB_StrdReq_GetConfig*/
         /* configured state: bConfiguration Value of current config returned
            address state   : value zero must be returned
            default state   : not specified
         */
  {TRUE,TRUE,FALSE}, /*USB_StrdReq_SetConfig*/
         /* configured state: If the specified configuration value is zero,
                              then the device enters the Address state.If the
                              specified configuration value matches the
                              configuration value from a config descriptor,
                              then that config is selected and the device
                              remains in the Configured state. Otherwise, the
                              device responds with a Request Error.

            address state   : If the specified configuration value is zero,
                              then the device remains in the Address state. If
                              the specified configuration value matches the
                              configuration value from a configuration
                              descriptor, then that configuration is selected
                              and the device enters the Configured state.
                              Otherwise,response is Request Error.
            default state   : not specified
         */
  {TRUE,FALSE,FALSE}, /*USB_StrdReq_GetInterface*/
         /* configured state: valid request
            address state   : request error
            default state   : not specified
         */
  {TRUE,FALSE,FALSE}, /*USB_StrdReq_SetInterface*/
         /* configured state: valid request
            address state   : request error
            default state   : not specified
         */
  {TRUE,FALSE,FALSE} /*USB_StrdReq_SyncFrame*/
         /* configured state: valid request
            address state   : request error
            default state   : not specified
         */
};
/*****************************************************************************
 * Global Functions Prototypes - None
 *****************************************************************************/

/*****************************************************************************
 * Local Types - None
 *****************************************************************************/

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static void USB_Service_Control (usbDeviceEvent_t * event );
static void USB_ControlServiceHandler(uint8_t controller_ID,
                                uint8_t status,
                                usbSetupPacket_t * setup_packet,
                                uint8_t* *data,
                                usbPacketSize_t *size);
static uint8_t USB_StrdReq_GetStatus(uint8_t      controller_ID,
                                usbSetupPacket_t * setup_packet,
                                uint8_t* *data,
                                usbPacketSize_t *size);
static uint8_t USB_StrdReq_Feature(uint8_t     controller_ID,
                                usbSetupPacket_t * setup_packet,
                                uint8_t* *data,
                                usbPacketSize_t *size);
static uint8_t USB_StrdReq_SetAddress(uint8_t     controller_ID,
                                usbSetupPacket_t * setup_packet,
                                uint8_t* *data,
                                usbPacketSize_t *size);
static uint8_t USB_AssignAddress(uint8_t controller_ID);
static uint8_t USB_StrdReq_GetConfig(uint8_t      controller_ID,
                                usbSetupPacket_t * setup_packet,
                                uint8_t* *data,
                                usbPacketSize_t *size);
static uint8_t USB_StrdReq_SetConfig(uint8_t      controller_ID,
                                usbSetupPacket_t * setup_packet,
                                uint8_t* *data,
                                usbPacketSize_t *size);
static uint8_t USB_StrdReq_GetInterface(uint8_t   controller_ID,
                                usbSetupPacket_t * setup_packet,
                                uint8_t* *data,
                                usbPacketSize_t *size);
static uint8_t USB_StrdReq_SetInterface(uint8_t   controller_ID,
                                usbSetupPacket_t * setup_packet,
                                uint8_t* *data,
                                usbPacketSize_t *size);
static uint8_t USB_StrdReq_SyncFrame(uint8_t      controller_ID,
                                usbSetupPacket_t * setup_packet,
                                uint8_t* *data,
                                usbPacketSize_t *size);
static uint8_t USB_StrdReq_GetDescriptor(uint8_t      controller_ID,
                                usbSetupPacket_t * setup_packet,
                                uint8_t* *data,
                                usbPacketSize_t *size);


/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/

/*****************************************************************************
 * Local Variables
 *****************************************************************************/

static uint8_t extReqToDevice[32];/* used for extended OUT transactions on
                                     CONTROL ENDPOINT*/
/*****************************************************************************
 * Global Variables
 *****************************************************************************/
pfusbReqHandler_t const gapfStandardRequest[gMaxStrdReq_d] =
{
    USB_StrdReq_GetStatus,
    USB_StrdReq_Feature,
    NULL,
    USB_StrdReq_Feature,
    NULL,
    USB_StrdReq_SetAddress,
    USB_StrdReq_GetDescriptor,
    NULL,
    USB_StrdReq_GetConfig,
    USB_StrdReq_SetConfig,
    USB_StrdReq_GetInterface,
    USB_StrdReq_SetInterface,
    USB_StrdReq_SyncFrame
};

/**************************************************************************//*!
 *
 * @name  USB_Framework_Init
 *
 * @brief The funtion initializes the Class Module
 *
 * @param controller_ID     : Controller ID
 * @param class_callback    : Class callback pointer
 * @param other_req_callback: Other Request Callback
 *
 * @return status
 *         gUsbErr_NoError_c           : When Successfull
 *         Others           : Errors
 *
 ******************************************************************************
 * This fuction registers the service on the control endpoint
 *****************************************************************************/
uint8_t USB_Framework_Init (
    uint8_t              controllerId,       /* [IN] Controller ID */
    pfUsbToAppEventHandler_t  pfClassCallback,      /* Class Callback */
    pfusbReqHandler_t        pfOtherReqCallback   /* Other Request Callback */
)
{
  uint8_t error=gUsbErr_NoError_c;
  /* save input parameters */
  gpfFrameworkCallback(controllerId) = pfClassCallback;
  gpfOtherReqCallback(controllerId) = pfOtherReqCallback;
  /* Register CONTROL service */
  error = USB_DeviceRegisterService(controllerId, gUsbService_Ep0_d, USB_Service_Control);
  return error;
}

/**************************************************************************//*!
 *
 * @name  USB_Framework_DeInit
 *
 * @brief The funtion De-initializes the Class Module
 *
 * @param controller_ID     : Controller ID
 *
 * @return status
 *         gUsbErr_NoError_c           : When Successfull
 *         Others           : Errors
 *
 ******************************************************************************
 * This fuction unregisters control service
 *****************************************************************************/
uint8_t USB_Framework_DeInit
(
    uint8_t controllerId       /* [IN] Controller ID */
) 
{
    uint8_t error;
    error =  USB_DeviceDeinit (controllerId);
    /* Free framwork_callback */
    gpfFrameworkCallback(controllerId) = NULL;
      /* Free other_req_callback */
    gpfOtherReqCallback(controllerId) = NULL;
      /* Return error */
    return error;
}

/**************************************************************************//*!
 *
 * @name  USB_Service_Control
 *
 * @brief Called upon a completed endpoint 0 transfer
 *
 * @param event : Pointer to USB Event Structure
 *
 * @return None
 *
 ******************************************************************************
 * This function handles the data sent or received on the control endpoint
 *****************************************************************************/
static void USB_Service_Control (
    usbDeviceEvent_t * event /* [IN] Pointer to USB Event Structure */
)
{
  uint16_t    deviceState = 0;
  uint8_t     status = gUsbErr_InvalidReqType_c;
  uint8_t* data = NULL;
  usbPacketSize_t size;
  usbSetupPacket_t* pSetupPkt = gpaSetupPkt[event->controllerId];
  /* get the device state  */
  (void)USB_DeviceGetStatus(event->controllerId , gUsbComponentStatus_DeviceState_d , &deviceState);
  if (event->setup == TRUE)
  {
    /* get the setup packet */
    pSetupPkt = gpaSetupPkt[event->controllerId] = (usbSetupPacket_t*)event->pBuffer;
    /* take care of endianess  of the 16 bt fields correctly */
#if (gLittle_Endian_d == 0)    
    pSetupPkt->index = USB_ByteSwap16(pSetupPkt->index);
    pSetupPkt->value = USB_ByteSwap16(pSetupPkt->value);
    pSetupPkt->length = USB_ByteSwap16(pSetupPkt->length);
#endif    
    /* if the request is standard request */
    if ((pSetupPkt->requestType & gUsbRequestClassMask_d) == gUsbRequestClass_Strd_d)
    {
      /* if callback is not NULL */
      if (gapfStandardRequest[pSetupPkt->request] != NULL)
      {
        /* if the request is valid in this device state */
        if((deviceState < gUsbDeviceState_Powered_d) && (gValidateRequest[pSetupPkt->request][deviceState] == TRUE))
        {
          /* Standard Request function pointers */
          status = gapfStandardRequest[pSetupPkt->request](event->controllerId, pSetupPkt, &data, (usbPacketSize_t *)&size);
        }
      }
    }
    else /* for Class/Vendor requests */
    {
      /*get the length from the setup_request*/
      size = (usbPacketSize_t)pSetupPkt->length;
      if( (size != 0) && ((pSetupPkt->requestType & gUsbDataDirMask_d) == gUsbDataDir_ToDevice_d) )
      {
        (void)MemoryCpy(extReqToDevice, pSetupPkt, gUsbSetupPacketSize_d);
        gpaSetupPkt[event->controllerId] = (usbSetupPacket_t*)extReqToDevice;
        /* expecting host to send data (OUT TRANSACTION)*/
        if(size > (sizeof(extReqToDevice) - gUsbSetupPacketSize_d))
        {
          size = sizeof(extReqToDevice) - gUsbSetupPacketSize_d;
        }
        (void)USB_DeviceRecvData(event->controllerId, gUsbControlEndpoint_d, extReqToDevice+gUsbSetupPacketSize_d,
        (usbPacketSize_t)(size));
        return;
      }
      else if(gpfOtherReqCallback(event->controllerId) != NULL)/*call class/vendor request*/
      {
        status = gpfOtherReqCallback(event->controllerId)(event->controllerId,
                    pSetupPkt, &data, (usbPacketSize_t*)&size);
      }
    }
    USB_ControlServiceHandler(event->controllerId , status , pSetupPkt , &data, (usbPacketSize_t*)&size);
  }
  /* if its not a setup request */
  else if(deviceState == gUsbDeviceState_PendingAddress_d)
  {
    /* Device state is PENDING_ADDRESS */
    /* Assign the new adddress to the Device */
    (void)USB_AssignAddress(event->controllerId);
    return;
  }
  else if( ((pSetupPkt->requestType & gUsbDataDirMask_d) == gUsbDataDir_ToDevice_d) &&
            (event->direction == gUsbEpDirection_Out_c) )
  {
    /* execution enters Control Service because of OUT transaction on gUsbControlEndpoint_d*/
    if(gpfOtherReqCallback(event->controllerId) != NULL)
    {/* class or vendor request */
      size = (usbPacketSize_t)(event->len + gUsbSetupPacketSize_d);
      status = gpfOtherReqCallback(event->controllerId)(event->controllerId,
                (usbSetupPacket_t*)(extReqToDevice), &data, 
                (usbPacketSize_t*)&size);
    }

    USB_ControlServiceHandler(event->controllerId , status , pSetupPkt , &data , (usbPacketSize_t*)&size);
  }
  else
  {
    if(event->shortSend && (event->direction == gUsbEpDirection_In_c))
    {
      usbMsg_t usbMsg;
      usbMsg.controllerId = event->controllerId;
      usbMsg.eventType = gUsbToAppEvent_ShortSendComplete_c;
      usbMsg.eventParam.shortSendCompleteEvent.epNum = event->epNum;
      usbMsg.eventParam.shortSendCompleteEvent.pBuffer = event->pBuffer;
      usbMsg.eventParam.shortSendCompleteEvent.len = event->len;
      USB_IsrEvent(&usbMsg);           
    }
  }
  return;
}

/**************************************************************************//*!
 *
 * @name  USB_ControlServiceHandler
 *
 * @brief The function is used to send a response to the Host based.
 *
 * @param controller_ID : Controller ID
 * @param status        : Status of request
 * @param setup_packet  : Setup packet received
 * @param data          : Data to be send back
 * @param size          : Size to be returned
 *
 * @return None
 *
 ******************************************************************************
 * This function sends a response to the data received on the control endpoint.
 * the request is decoded in the control service
 *****************************************************************************/
static void USB_ControlServiceHandler (
    uint8_t    controllerId,            /* [IN] Controller ID */
    uint8_t status,                      /* [IN] Device Status */
    usbSetupPacket_t * pSetupPacket,    /* [IN] Setup packet received */
    uint8_t* *data,                   /* [OUT] Data to be send back */
    usbPacketSize_t *size               /* [OUT] Size to be returned */
)
{
  if(status == gUsbErr_InvalidReqType_c)
  {
    /* incase of error Stall endpoint */
    (void)USB_DeviceSetStatus(controllerId,
            (uint8_t)(gUsbComponentStatus_Endpoint_d | gUsbControlEndpoint_d |
            (gUsbEpDirection_In_c << gUsbComponent_DirectionShift_d)),
            gUsbStatus_Stalled_d);
  }
  else if(status == gUsbErr_ProcessLater_c)
  {
    usbMsg_t usbMsg;
    usbMsg.controllerId = controllerId;
    usbMsg.eventType = gUsbToAppEvent_RequestPending_c;
    usbMsg.eventParam.requestPendingEvent.pSetupPacket = pSetupPacket;
    USB_IsrEvent(&usbMsg);  
  }
  else /* Need to send Data to the USB Host */
  {
    /* send the data prepared by the handlers.*/
    if(*size > pSetupPacket->length)
    {
      *size = (usbPacketSize_t)pSetupPacket->length;
    }
    /* send the data to the host */
    (void)USB_ClassSendData(controllerId , gUsbControlEndpoint_d, *data, *size);
    if((pSetupPacket->requestType & gUsbDataDirMask_d) ==
            gUsbDataDir_ToHost_d)
        {   /* Request was to Get Data from device */
            /* setup rcv to get status from host */
            (void)USB_DeviceRecvData(controllerId,
                gUsbControlEndpoint_d,NULL,0);
        }
  }
  return;
}
/*************************************************************************//*!
 *
 * @name  USB_StrdReq_GetStatus
 *
 * @brief  This function is called in response to Get Status request
 *
 * @param controller_ID : Controller ID
 * @param setup_packet  : Setup packet received
 * @param data          : Data to be send back
 * @param size          : Size to be returned
 *
 * @return status:
 *               gUsbErr_NoError_c : When Successfull
 *               Others : When Error
 *
 ******************************************************************************
 * this is a ch9 request and is called by the host to know the status of the
 * device, the interface and the endpoint
 *****************************************************************************/
static uint8_t USB_StrdReq_GetStatus (
    uint8_t    controllerId,            /* [IN] Controller ID */
    usbSetupPacket_t * pSetupPacket,    /* [IN] Setup packet received */
    uint8_t* *data,                   /* [OUT] Data to be send back */
    usbPacketSize_t *size               /* [OUT] Size to be returned */
)
{
  uint8_t interface, endpoint;
  uint8_t status = gUsbErr_InvalidReqType_c;

  if((pSetupPacket->requestType & gUsbRequestSrcMask_d) == gUsbRequestSrc_Device_d)
  {    
         /* request for Device */
  status = USB_DeviceGetStatus(controllerId , gUsbComponentStatus_Device_d , &gStdFrameworkData);
  gStdFrameworkData &= gUsbGetStatusDeviceMask_d;
#if (gLittle_Endian_d == 0)
  gStdFrameworkData = USB_ByteSwap16(gStdFrameworkData);
#endif
  *size = gDeviceStatusSize_d;
  }
  else if ((pSetupPacket->requestType & gUsbRequestSrcMask_d) == gUsbRequestSrc_Interface_d)
  {    
    /* request for Interface */
    interface  = (uint8_t) pSetupPacket->index;
    status =  USB_Desc_GetInterface(controllerId , interface , (uint8_t*)&gStdFrameworkData);
    *size = gUsbInterfaceStatusSize_d;
  }
  else if ((pSetupPacket->requestType & gUsbRequestSrcMask_d) == gUsbRequestSrc_Endpoint_d)
  {   
    /* request for Endpoint */
    endpoint  = (uint8_t)(((uint8_t) pSetupPacket->index) | gUsbComponentStatus_Endpoint_d);
    status =  USB_DeviceGetStatus(controllerId , endpoint , &gStdFrameworkData);
#if (gLittle_Endian_d == 0)    
    gStdFrameworkData = USB_ByteSwap16(gStdFrameworkData);
#endif    
    *size = gUsbEndpStatusSize_d;
  }
  *data = (uint8_t*)&gStdFrameworkData;
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_StrdReq_Feature
 *
 * @brief  This function is called in response to Clear or Set Feature request
 *
 * @param controller_ID : Controller ID
 * @param setup_packet  : Setup packet received
 * @param data          : Data to be send back
 * @param size          : Size to be returned
 *
 * @return status:
 *               gUsbErr_NoError_c : When Successfull
 *               Others : When Error
 *
 ******************************************************************************
 * This is a ch9 request, used to set/clear a feature on the device
 * (device_remote_wakeup and test_mode) or on the endpoint(ep halt)
 *****************************************************************************/
static uint8_t USB_StrdReq_Feature (
    uint8_t    controllerId,            /* [IN] Controller ID */
    usbSetupPacket_t * pSetupPacket,    /* [IN] Setup packet received */
    uint8_t* *data,                   /* [OUT] Data to be send back */
    usbPacketSize_t *size               /* [OUT] Size to be returned */
)
{
  uint16_t deviceStatus;
  uint8_t  setRequest;
  uint8_t  status = gUsbErr_InvalidReqType_c;
  uint8_t  epinfo;
  gUsbToAppEvent_t event;
  usbMsg_t usbMsg;
  (void) (data);
  *size=0;
  /* find whether its a clear feature request or a set feature request */
  setRequest = (uint8_t)((pSetupPacket->request & gUsbSetRequestMask_d) >> 1 );
  if((pSetupPacket->requestType & gUsbRequestSrcMask_d) == gUsbRequestSrc_Device_d)
  {
    if(setRequest == TRUE)
    { 
      uint16_t device_set_feature_bitfield = gUsbDeviceSetFeatureMask_d;
      if(((uint16_t)(1 << (uint8_t)pSetupPacket->value)) & device_set_feature_bitfield)
      {
        status = gUsbErr_NoError_c;
        if(pSetupPacket->value == gUsbDeviceFeatureRemoteWakeup_d)
        {
          status = USB_DeviceGetStatus(controllerId , gUsbComponentStatus_Device_d , &deviceStatus);
          /* add the request to be cleared from device_status */
          deviceStatus |= (uint16_t)(1 << (uint8_t)pSetupPacket->value);                   
          /* set the status on the device */
          status = USB_DeviceSetStatus(controllerId , gUsbComponentStatus_Device_d, deviceStatus);
        }
      }
      else
      {
        status = gUsbErr_InvalidReqType_c;    
      }
    }
    else//(set_request == FALSE) it is a clear feature request
    {
      if(((uint16_t)(1 << (uint8_t)pSetupPacket->value)) & gUsbDeviceClearFeatureMask_d) 
      {
        status = gUsbErr_NoError_c;
        if(pSetupPacket->value == gUsbDeviceFeatureRemoteWakeup_d)
        {
          status = USB_DeviceGetStatus(controllerId , gUsbComponentStatus_Device_d , &deviceStatus);
          /* remove the request to be cleared from device_status */
          deviceStatus &= (uint16_t)~(1 << (uint8_t)pSetupPacket->value);
          status = USB_DeviceSetStatus(controllerId , gUsbComponentStatus_Device_d, deviceStatus);
        }
      }
      else
      {
        status = gUsbErr_InvalidReqType_c;   
      }
    }
  }
  else if ((pSetupPacket->requestType & gUsbRequestSrcMask_d) == gUsbRequestSrc_Endpoint_d)
  {
    /* request for Endpoint */
    epinfo = (uint8_t)(pSetupPacket->index & 0x00FF);
	  status = USB_DeviceSetStatus(controllerId , (uint8_t)(epinfo|gUsbComponentStatus_Endpoint_d) , setRequest);
    event = setRequest ? gUsbToAppEvent_EpStalled_c : gUsbToAppEvent_EpUnstalled_c;      
    /* inform the upper layers of stall/unstall */
    usbMsg.controllerId = controllerId;
    usbMsg.eventType = event;
    usbMsg.eventParam.stallEvent.stallEp = epinfo;
    gpfFrameworkCallback(controllerId)(controllerId,event,&usbMsg);
  }
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_StrdReq_SetAddress
 *
 * @brief  This function is called in response to Set Address request
 *
 * @param controller_ID : Controller ID
 * @param setup_packet  : Setup packet received
 * @param data          : Data to be send back
 * @param size          : Size to be returned
 *
 * @return status:
 *               gUsbErr_NoError_c : When Successfull
 *               Others : When Error
 *
 ******************************************************************************
 * This is a ch9 request, saves the new address in a global variable. this
 * address is assigned to the device after this transaction completes
 *****************************************************************************/
static uint8_t USB_StrdReq_SetAddress (
    uint8_t    controllerId,            /* [IN] Controller ID */
    usbSetupPacket_t * pSetupPacket,    /* [IN] Setup packet received */
    uint8_t* *data,                   /* [OUT] Data to be send back */
    usbPacketSize_t *size               /* [OUT] Size to be returned */
)
{
  (void) (data);
  *size=0;
  /* update device stae */
  (void)USB_DeviceSetStatus(controllerId , gUsbComponentStatus_DeviceState_d , gUsbDeviceState_PendingAddress_d);
  /*store the address from setup_packet into assigned_address*/
  gAssignedAddress = (uint8_t)pSetupPacket->value;
  return gUsbErr_NoError_c;
}

/**************************************************************************//*!
 *
 * @name  USB_AssignAddress
 *
 * @brief  This function assigns the address to the Device
 *
 * @param controller_ID : Controller ID
 *
 * @return status
 *                       gUsbErr_NoError_c: Always
 *
 ******************************************************************************
 * This function assigns the new address and is called (from the control
 * service) after the set address transaction completes
 *****************************************************************************/
static uint8_t USB_AssignAddress (
    uint8_t    controllerId         /* [IN] Controller ID */
)
{
  /* Set Device Address */
  (void)USB_DeviceSetAddress(controllerId, gAssignedAddress);
  /* Set Device state */
  (void)USB_DeviceSetStatus(controllerId , gUsbComponentStatus_DeviceState_d , gUsbDeviceState_Address_d);
    /* Set Device state */
  (void)USB_DeviceSetStatus(controllerId , gUsbComponentStatus_Address_d , gAssignedAddress);
  return gUsbErr_NoError_c;
}

/**************************************************************************//*!
 *
 * @name  USB_StrdReq_GetConfig
 *
 * @brief  This function is called in response to Get Config request
 *
 * @param controller_ID : Controller ID
 * @param setup_packet  : Setup packet received
 * @param data          : Data to be send back
 * @param size          : Size to be returned
 *
 * @return status:
 *               gUsbErr_NoError_c : Always
 *
 ******************************************************************************
 * This is a ch9 request and is used to know the currently used configuration
 *****************************************************************************/
static uint8_t USB_StrdReq_GetConfig (
    uint8_t    controllerId,            /* [IN] Controller ID */
    usbSetupPacket_t * pSetupPacket,    /* [IN] Setup packet received */
    uint8_t* *data,                   /* [OUT] Data to be send back */
    usbPacketSize_t *size               /* [OUT] Size to be returned */
)
{
  (void)(pSetupPacket);
  *size = gUsbConfigSize_d;
  (void)USB_DeviceGetStatus(controllerId , gUsbComponentStatus_CurrentConfig_d , &gStdFrameworkData);
 #if (gLittle_Endian_d == 0)
  gStdFrameworkData = USB_ByteSwap16(gStdFrameworkData);
#endif
  *data = (uint8_t*)(&gStdFrameworkData);
  return gUsbErr_NoError_c;
}

/**************************************************************************//*!
 *
 * @name  USB_StrdReq_SetConfig
 *
 * @brief  This function is called in response to Set Config request
 *
 * @param controller_ID : Controller ID
 * @param setup_packet  : Setup packet received
 * @param data          : Data to be send back
 * @param size          : Size to be returned
 *
 * @return status:
 *               gUsbErr_NoError_c : When Successfull
 *               Others : When Error
 *
 ******************************************************************************
 * This is a ch9 request and is used by the host to set the new configuration
 *****************************************************************************/
static uint8_t USB_StrdReq_SetConfig (
    uint8_t    controllerId,            /* [IN] Controller ID */
    usbSetupPacket_t * pSetupPacket,    /* [IN] Setup packet received */
    uint8_t* *data,                   /* [OUT] Data to be send back */
    usbPacketSize_t *size               /* [OUT] Size to be returned */
)
{
  uint8_t status = gUsbErr_InvalidReqType_c;
  uint16_t configVal;
  usbMsg_t usbMsg;
  (void) (data);
  *size = 0;
  status = gUsbStatus_Error_d;
  configVal = pSetupPacket->value;
  if (USB_Desc_ValidConfiguration(controllerId, configVal))
  /*if valid configuration (fn returns bool_t value)*/
  {
    uint16_t deviceState = gUsbDeviceState_Config_d;
    /* if config_val is 0 */
    if (!configVal) 
    {
      deviceState = gUsbDeviceState_Address_d ;
    }
    status = USB_DeviceSetStatus(controllerId, gUsbComponentStatus_DeviceState_d, deviceState);
    status = USB_DeviceSetStatus(controllerId, gUsbComponentStatus_CurrentConfig_d, configVal);
    /* Callback to the app. to let the application know about the new Configuration  */
    usbMsg.controllerId = controllerId;
    usbMsg.eventType = gUsbToAppEvent_ConfigChanged_c;
    usbMsg.eventParam.configEvent.configValue = configVal;
    gpfFrameworkCallback(controllerId)(controllerId,gUsbToAppEvent_ConfigChanged_c,&usbMsg);
    if(configVal)
    {
    usbMsg.eventType = gUsbToAppEvent_EnumComplete_c;
    gpfFrameworkCallback(controllerId)(controllerId,gUsbToAppEvent_EnumComplete_c, &usbMsg);
    }
  }
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_StrdReq_GetInterface
 *
 * @brief  This function is called in response to Get Interface request
 *
 * @param controller_ID : Controller ID
 * @param setup_packet  : Setup packet received
 * @param data          : Data to be send back
 * @param size          : Size to be returned
 *
 * @return status:
 *               gUsbErr_NoError_c : When Successfull
 *               Others : When Error
 *
 ******************************************************************************
 * This is a ch9 request and is used to know the current interface
 *****************************************************************************/
static uint8_t USB_StrdReq_GetInterface (
    uint8_t    controllerId,            /* [IN] Controller ID */
    usbSetupPacket_t * pSetupPacket,    /* [IN] Setup packet received */
    uint8_t* *data,                   /* [OUT] Data to be send back */
    usbPacketSize_t *size               /* [OUT] Size to be returned */
)
{
  uint8_t status;
  *size = gUsbInterfaceStatusSize_d;
  status = USB_Desc_GetInterface(controllerId , (uint8_t)pSetupPacket->index , (uint8_t*)&gStdFrameworkData);
  *data = (uint8_t*)&gStdFrameworkData;
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_StrdReq_SetInterface
 *
 * @brief  This function is called in response to Set Interface request
 *
 * @param controller_ID : Controller ID
 * @param setup_packet  : Setup packet received
 * @param data          : Data to be send back
 * @param size          : Size to be returned
 *
 * @return status:
 *               gUsbErr_NoError_c : Always
 *
 ******************************************************************************
 * This is a ch9 request and is used by the  host to set the interface
 *****************************************************************************/
static uint8_t USB_StrdReq_SetInterface (
    uint8_t    controllerId,            /* [IN] Controller ID */
    usbSetupPacket_t * pSetupPacket,    /* [IN] Setup packet received */
    uint8_t* *data,                   /* [OUT] Data to be send back */
    usbPacketSize_t *size               /* [OUT] Size to be returned */
)
{
  (void) (data);
  *size=0;
  /* Request type not for interface */
  if ((pSetupPacket->requestType & 0x03) != 0x01)
  {
    return gUsbStatus_Error_d;
  }
  /* Get Interface and alternate interface from setup_packet */
  (void)USB_Desc_SetInterface(controllerId , (uint8_t)pSetupPacket->index , (uint8_t)pSetupPacket->value);
  return gUsbErr_NoError_c;
}

/**************************************************************************//*!
 *
 * @name  USB_StrdReq_SyncFrame
 *
 * @brief  This function is called in response to Sync Frame request
 *
 * @param controller_ID : Controller ID
 * @param setup_packet  : Setup packet received
 * @param data          : Data to be send back
 * @param size          : Size to be returned
 *
 * @return status:
 *               gUsbErr_NoError_c : When Successfull
 *               Others : When Error
 *
 ******************************************************************************
 * This req is used to set and then report an ep's synchronization frame
 *****************************************************************************/
static uint8_t USB_StrdReq_SyncFrame (
    uint8_t    controllerId,            /* [IN] Controller ID */
    usbSetupPacket_t * pSetupPacket,    /* [IN] Setup packet received */
    uint8_t* *data,                   /* [OUT] Data to be send back */
    usbPacketSize_t *size               /* [OUT] Size to be returned */
)
{
  uint8_t status;
  (void) (pSetupPacket);
  *size=gUsbFrameSize_d;
  /* Get the frame number */
  status = USB_DeviceGetStatus(controllerId , gUsbComponentStatus_SOFCount_d , &gStdFrameworkData);
  *data = (uint8_t*)&gStdFrameworkData;
  return status;
}


/**************************************************************************//*!
 *
 * @name  USB_Std_Req_Get_Descriptor
 *
 * @brief  This function is called in response to Get Descriptor request
 *
 * @param controller_ID : Controller ID
 * @param setup_packet  : Setup packet received
 * @param data          : Data to be send back
 * @param size          : Size to be returned
 *
 * @return status:
 *               gUsbErr_NoError_c : When Successfull
 *               Others : When Error
 *
 ******************************************************************************
 * This is a ch9 request and is used to send the descriptor requested by the
 * host
 *****************************************************************************/
static uint8_t USB_StrdReq_GetDescriptor (
    uint8_t    controllerId,            /* [IN] Controller ID */
    usbSetupPacket_t * pSetupPacket,    /* [IN] Setup packet received */
    uint8_t* *data,                   /* [OUT] Data to be send back */
    usbPacketSize_t *size               /* [OUT] Size to be returned */
)
{
  uint8_t type = USB_UInt16High(pSetupPacket->value);
  uint16_t index = (uint8_t)gUninitialisedVal_d;
  uint8_t strNum = (uint8_t)gUninitialisedVal_d;
  uint8_t status;
  /* for string descriptor set the language and string number */
  index = pSetupPacket->index;
  /*g_setup_pkt.lValue*/
  strNum = USB_UInt16Low(pSetupPacket->value);
  /* Call descriptor class to get descriptor */
  status = USB_Desc_GetDescriptor(controllerId , type , strNum , index , data , size);
  return status;
}
#endif // gUsbIncluded_d
