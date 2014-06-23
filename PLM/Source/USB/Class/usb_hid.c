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
 * @file usb_hid.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief The file contains USB stack HID layer implementation.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

 
#include "USB_Interface.h" 
#include "USB.h"
#include "usb_hid.h"    /* USB HID Class Header File */
#include "usb_descriptor.h"
#include "usb_devapi.h" /* USB device Header File */

#if gUsbHidEnabled_d 
 
#if (gUsbIncluded_d == FALSE)
 #error If gUsbHidEnabled_d == TRUE, gUsbIncluded_d must be TRUE
#endif

/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/

/****************************************************************************
 * Global Variables
 ****************************************************************************/


/*****************************************************************************
 * Local Types - None
 *****************************************************************************/

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static uint8_t USB_HID_OtherRequests
(
  uint8_t controllerId,
  usbSetupPacket_t * pSetupPacket,
  uint8_t* *pData,
  usbPacketSize_t *pSize
);

/*****************************************************************************
 * Local Variables - None
 *****************************************************************************/

/*****************************************************************************
 * Local Functions
 *****************************************************************************/

/**************************************************************************//*!
 *
 * @name  USB_Class_HID_ServiceIn
 *
 * @brief The funtion is a callback function of HID endpoint
 *
 * @param event : Pointer to USB Event Structure
 *
 * @return None
 *
 ******************************************************************************
 * This function is called from lower layer when data is transfer is completed
 * on HID endpoint (non control endpoint)
 *****************************************************************************/
static void USB_Class_HID_ServiceIn
(
  usbDeviceEvent_t * pEvent /* [IN] Pointer to USB Event Structure */
)
{
  pfUsbToAppEventHandler_t hidClassCallback;
  hidClassCallback = gpaUsbClassDescriptor[pEvent->controllerId]->pfUsbToAppEventHandler;
  if(hidClassCallback != NULL)
  {
    /* notify the app of the send complete */
    usbMsg_t usbMsg;
    usbMsg.controllerId = pEvent->controllerId;
    if(pEvent->errors != 0)
    {
      usbMsg.eventType = gUsbToAppEvent_Error_c;
      usbMsg.eventParam.errorEvent.errors = pEvent->errors;
    }
    else if(pEvent->shortSend)
    {
      usbMsg.eventType = gUsbToAppEvent_ShortSendComplete_c;
      usbMsg.eventParam.shortSendCompleteEvent.epNum = pEvent->epNum;
      usbMsg.eventParam.shortSendCompleteEvent.pBuffer = pEvent->pBuffer;
      usbMsg.eventParam.shortSendCompleteEvent.len = pEvent->len;
    }
    else
    {
      usbMsg.eventType = gUsbToAppEvent_SendComplete_c;
      usbMsg.eventParam.sendCompleteEvent.epNum = pEvent->epNum;
      usbMsg.eventParam.sendCompleteEvent.pBuffer = pEvent->pBuffer;
      usbMsg.eventParam.sendCompleteEvent.len = pEvent->len;
    }
    USB_IsrEvent(&usbMsg);
  }
}

/**************************************************************************//*!
 *
 * @name  USB_Class_HID_ServiceOut
 *
 * @brief The funtion is a callback function of HID endpoint
 *
 * @param event : Pointer to USB Event Structure
 *
 * @return None
 *
 ******************************************************************************
 * This function is called from lower layer when data is transfer is completed
 * on HID endpoint (non control endpoint)
 *****************************************************************************/
static void USB_Class_HID_ServiceOut
(
  usbDeviceEvent_t * pEvent /* [IN] Pointer to USB Event Structure */
)
{
  pfUsbToAppEventHandler_t hidClassCallback;
  hidClassCallback = gpaUsbClassDescriptor[pEvent->controllerId]->pfUsbToAppEventHandler;
  if(hidClassCallback != NULL)
  {
    /* notify the app of the send complete */
    usbMsg_t usbMsg;
    usbMsg.controllerId = pEvent->controllerId;
    if(pEvent->errors != 0)
    {
      usbMsg.eventType = gUsbToAppEvent_Error_c;
      usbMsg.eventParam.errorEvent.errors = pEvent->errors;
    }
    else
    {
      usbMsg.eventType = gUsbToAppEvent_DataReceived_c;
      usbMsg.eventParam.dataReceivedEvent.epNum = pEvent->epNum;
      usbMsg.eventParam.dataReceivedEvent.pBuffer = pEvent->pBuffer;
      usbMsg.eventParam.dataReceivedEvent.len = pEvent->len;
    }
    USB_IsrEvent(&usbMsg);
  }
}

/**************************************************************************//*!
 *
 * @name  USB_Class_HID_Event
 *
 * @brief The funtion initializes HID endpoint
 *
 * @param controller_ID     : Controller ID
 * @param event             : Event Type
 * @param val               : Pointer to configuration Value
 *
 * @return None
 *
 ******************************************************************************
 * The funtion initializes the HID endpoints when Enumeration complete event is
 * received
 *****************************************************************************/
static void USB_Class_HID_Event
(
  uint8_t controllerId,   /* [IN] Controller ID */
  gUsbToAppEvent_t event,           /* [IN] Event Type */
  void* pVal               /* [IN] Pointer to configuration Value */
)
{
  pfUsbToAppEventHandler_t hidClassCallback;
  uint8_t index;

  if(event == gUsbToAppEvent_EnumComplete_c)
  {
    usbEpStruct_t* pHidNCEpDesc = gpaUsbClassDescriptor[controllerId]->usbNonControlEpDescriptor;    
    uint8_t count = gpaUsbClassDescriptor[controllerId]->numNonControlEp;
		index = count;
		while(index > 0) 
		{ 
		  index--;  
			(void)USB_DeviceDeinitEndpoint(controllerId,pHidNCEpDesc[index].number, pHidNCEpDesc[index].direction);
		}
		  
    while( index < count )
    {
      usbEpStruct_t* pEpStruct = pHidNCEpDesc + index;
      (void)USB_DeviceInitEndpoint(controllerId, pEpStruct, TRUE);
      // register callback service for the endpoint 
      if(pEpStruct->direction == gUsbEpDirection_In_c) 
      {
        (void)USB_DeviceRegisterService(controllerId,(uint8_t)(gUsbService_Ep0_d+pEpStruct->number),USB_Class_HID_ServiceIn);  
      } 
      else
      {
        (void)USB_DeviceRegisterService(controllerId,(uint8_t)(gUsbService_Ep0_d+pEpStruct->number),USB_Class_HID_ServiceOut);
      }
      // set the EndPoint Status as Idle in the device layer 
      (void)USB_DeviceSetStatus(controllerId,
            (uint8_t)(gUsbComponentStatus_Endpoint_d | pEpStruct->number |(pEpStruct->direction << gUsbComponent_DirectionShift_d)),
            gUsbStatus_Idle_d);
      index++;
    }

  }//(event == gUsbToAppEvent_EnumComplete_c)
  
  hidClassCallback = gpaUsbClassDescriptor[controllerId]->pfUsbToAppEventHandler;
  if(hidClassCallback != NULL)
  {
    USB_IsrEvent((usbMsg_t*) pVal);    
  }
}

/**************************************************************************//*!
 *
 * @name  USB_Other_Requests
 *
 * @brief   The funtion provides flexibilty to add class and vendor specific
 *              requests
 *
 * @param controller_ID     : Controller ID
 * @param setup_packet      : Setup packet received
 * @param data              : Data to be send back
 * @param size              : Size to be returned
 *
 * @return status:
 *                        gUsbErr_NoError_c        : When Successfull
 *                        Others        : When Error
 *
 ******************************************************************************
 * Handles HID Class requests and forwards vendor specific request to the
 * application
 *****************************************************************************/
static uint8_t USB_HID_OtherRequests
(
  uint8_t controllerId,               /* [IN] Controller ID */
  usbSetupPacket_t * pSetupPacket,    /*[IN] Setup packet received */
  uint8_t* *pData,                   /* [OUT] Data to be send back */
  usbPacketSize_t *pSize               /* [OUT] Size to be returned*/
)
{
  uint8_t status = gUsbErr_InvalidReqType_c;
  if((pSetupPacket->requestType & gUsbRequestClassMask_d) == gUsbRequestClass_Class_d)
  {
    pfClassSpecificReqHandler_t pfClassSpecificReqAppHandler ;
    pfClassSpecificReqAppHandler =  gpaUsbClassDescriptor[controllerId]->pfClassSpecificReqHandler;
    if(pfClassSpecificReqAppHandler != NULL)
    {
      /* handle callback if the application has supplied it  */
      *pSize = (usbPacketSize_t)(pSetupPacket->length + gUsbSetupPacketSize_d); 
      *pData = ((uint8_t*)pSetupPacket);
       status = pfClassSpecificReqAppHandler(controllerId,pSetupPacket->request,/* request type */
                pSetupPacket->value,
                pData,/* pointer to the data */
                pSize);/* size of the transfer */
    }
  }
  else if((pSetupPacket->requestType & gUsbRequestClassMask_d) == gUsbRequestClass_Vendor_d)
  {
    /* vendor specific request  */
    pfusbReqHandler_t       pfVendorReqCallback;
    pfVendorReqCallback =  gpaUsbClassDescriptor[controllerId]->pfVendorReqHandler;
    if(pfVendorReqCallback != NULL)
    {
      status = pfVendorReqCallback(controllerId, pSetupPacket,pData, pSize);
    }
  }
  return status;
}

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

/**************************************************************************//*!
 *
 * @name  USB_Class_HID_Init
 *
 * @brief   The funtion initializes the Device and Controller layer
 *
 * @param   controller_ID               : Controller ID
 * @param   hid_class_callback          : HID Class Callback
 * @param   vendor_req_callback         : Vendor Request Callback
 * @param   param_callback              : Class requests Callback
 *
 * @return status:
 *                        gUsbErr_NoError_c        : When Successfull
 *                        Others        : When Error
 *
 ******************************************************************************
 *This function initializes the HID Class layer and layers it is dependent on
 *****************************************************************************/
uint8_t USB_Class_HID_Init 
(
  uint8_t    controllerId                /* [IN] Controller ID */
)
{
  uint8_t status;
  uint8_t count = gpaUsbClassDescriptor[controllerId]->numNonControlEp;
  /* Initialize the device layer*/
  status = USB_DeviceInit(controllerId,count + 1);
  if(status == gUsbErr_NoError_c)
  {
    /* Initialize the generic class functions */
    status = USB_ClassInit(controllerId,USB_Class_HID_Event,USB_HID_OtherRequests);
  }
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_Class_HID_DeInit
 *
 * @brief   The funtion de-initializes the Device and Controller layer
 *
 * @param   controller_ID               : Controller ID
 *
 * @return status:
 *                        gUsbErr_NoError_c        : When Successfull
 *                        Others        : When Error
 *
 ******************************************************************************
 *This function de-initializes the HID Class layer
 *****************************************************************************/
uint8_t USB_Class_HID_DeInit 
(
  uint8_t controllerId              /* [IN] Controller ID */
) 
{
  uint8_t status;
  /* Call common class deinit function */
  status = USB_ClassDeInit(controllerId);
  return status;
}

#endif