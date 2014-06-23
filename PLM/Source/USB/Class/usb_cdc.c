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
 * @file usb_cdc.c
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief The file contains USB stack CDC layer implementation.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "USB.h" 
#include "usb_cdc.h"    /* USB CDC Class Header File */
#include "usb_cdc_pstn.h"
/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/

/****************************************************************************
 * Global Variables
***************************************************************************/
#if gUsbCdcEnabled_d

#if (gUsbIncluded_d == FALSE)
 #error If gUsbCdcEnabled_d == TRUE, gUsbIncluded_d must be TRUE
#endif

static void (*pfCDCDicInServiceCallBack)(usbDeviceEvent_t* event);
static void (*pfCDCDicOutServiceCallBack)(usbDeviceEvent_t* event);
extern uint8_t gDTEStatus;
/*****************************************************************************
 * Local Types - None
 *****************************************************************************/
#if gUSB_CDC_CIC_NotifElemSupport_d
extern uint8_t gNotifyEndpoint;
#endif
/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
#if gUSB_CDC_CIC_NotifElemSupport_d
static void USB_Class_CDC_ServiceCicNotify(usbDeviceEvent_t* event);
#endif

static void USB_Class_CDC_ServiceDicBulkIn(usbDeviceEvent_t* event);
static void USB_Class_CDC_ServiceDicBulkOut(usbDeviceEvent_t* event);
static uint8_t USB_CDC_OtherRequests
(
  uint8_t controllerId,
  usbSetupPacket_t * setupPacket,
  uint8_t* *data,
  usbPacketSize_t *size
);
static void USB_CDC_ClassEvent
(
  uint8_t controllerId,
  gUsbToAppEvent_t event,
  void* val
);

/*****************************************************************************
 * Local Variables - None
 *****************************************************************************/

 /*****************************************************************************
 * Local Functions
 *****************************************************************************/
#if gUSB_CDC_CIC_NotifElemSupport_d
/**************************************************************************//*!
 *
 * @name  USB_Class_CDC_Service_Cic_Notify
 *
 * @brief The funtion is callback function of CIC Notification endpoint
 *
 * @param event : Pointer to USB Event Structure
 *
 * @return None
 *
 ******************************************************************************
 * Called by Lower layer when data on CIC Endpoint is sent
 *****************************************************************************/
static void USB_Class_CDC_ServiceCicNotify 
(
    usbDeviceEvent_t* event  /* [IN] Pointer to USB Event Structure */
)
{
  pfUsbToAppEventHandler_t cdcClassCallback;
  cdcClassCallback = gpaUsbClassDescriptor[event->controllerId]->pfUsbToAppEventHandler;
  if(cdcClassCallback != NULL)
  {
    usbMsg_t usbMsg;
    usbMsg.controllerId = event->controllerId;
    if(event->errors != 0)
    {
      usbMsg.eventType = gUsbToAppEvent_Error_c;
      usbMsg.eventParam.errorEvent.errors = event->errors;
    }
    else
    {
      usbMsg.eventType = gUsbToAppEvent_SendComplete_c;
      usbMsg.eventParam.sendCompleteEvent.epNum = event->epNum;
      usbMsg.eventParam.sendCompleteEvent.pBuffer = event->pBuffer;
      usbMsg.eventParam.sendCompleteEvent.len = event->len;
    }
    USB_IsrEvent(&usbMsg);
  }
}
#endif


/**************************************************************************//*!
 *
 * @name  USB_Class_CDC_Service_Dic_Bulk_In
 *
 * @brief The funtion is callback function of DIC Bulk In Endpoint
 *
 * @param event : Pointer to USB Event Structure
 *
 * @return None
 *
 ******************************************************************************
 * Called by Lower Layer when Data on DIC SEND Interface is sent
 *****************************************************************************/
static void USB_Class_CDC_ServiceDicBulkIn 
(
  usbDeviceEvent_t* event /* [IN] Pointer to USB Event Structure */
)
{
  if(pfCDCDicInServiceCallBack != NULL)
  {
    pfCDCDicInServiceCallBack(event);  
  }
  else
  {
    pfUsbToAppEventHandler_t cdcClassCallback;
    cdcClassCallback = gpaUsbClassDescriptor[event->controllerId]->pfUsbToAppEventHandler;
    if(cdcClassCallback != NULL)
    {
      usbMsg_t usbMsg;
      usbMsg.controllerId = event->controllerId;
      if(event->errors != 0)
      {
        usbMsg.eventType = gUsbToAppEvent_Error_c;
        usbMsg.eventParam.errorEvent.errors = event->errors;
      }
      else
      {
        usbMsg.eventType = gUsbToAppEvent_SendComplete_c;
        usbMsg.eventParam.sendCompleteEvent.epNum = event->epNum;
        usbMsg.eventParam.sendCompleteEvent.pBuffer = event->pBuffer;
        usbMsg.eventParam.sendCompleteEvent.len = event->len;
      }
      USB_IsrEvent(&usbMsg);
    }
  }
}

/**************************************************************************//*!
 *
 * @name  USB_Class_CDC_Service_Dic_Bulk_Out
 *
 * @brief The funtion is callback function of DIC Bulk Out Endpoint
 *
 * @param event : Pointer to USB Event Structure
 *
 * @return None
 *
 ******************************************************************************
 * Called by Lower Layer when Data on DIC RECV Interface is received
 *****************************************************************************/
static void USB_Class_CDC_ServiceDicBulkOut 
(
    usbDeviceEvent_t* event  /* [IN] Pointer to USB Event Structure */
)
{   
  if(pfCDCDicOutServiceCallBack != NULL)
  {
    pfCDCDicOutServiceCallBack(event);  
  }
  else
  {
    pfUsbToAppEventHandler_t cdcClassCallback;
    cdcClassCallback = gpaUsbClassDescriptor[event->controllerId]->pfUsbToAppEventHandler;
    if(cdcClassCallback != NULL)
    {
      usbMsg_t usbMsg;
      usbMsg.controllerId = event->controllerId;
      if(event->errors != 0)
      {
        usbMsg.eventType = gUsbToAppEvent_Error_c;
        usbMsg.eventParam.errorEvent.errors = event->errors;
      }
      else
      {
        usbMsg.eventType = gUsbToAppEvent_DataReceived_c;
        usbMsg.eventParam.dataReceivedEvent.epNum = event->epNum;
        usbMsg.eventParam.dataReceivedEvent.pBuffer = event->pBuffer;
        usbMsg.eventParam.dataReceivedEvent.len = event->len;
      }
        USB_IsrEvent(&usbMsg);
    }   
  }
}


/**************************************************************************//*!
 *
 * @name  USB_Class_CDC_Event
 *
 * @brief The funtion initializes CDC endpoints
 *
 * @param controller_ID : Controller ID
 * @param event         : Event Type
 * @param val           : Pointer to configuration Value
 *
 * @return None
 *
 ******************************************************************************
 *
 *****************************************************************************/
static void USB_CDC_ClassEvent 
(
    uint8_t controllerId,   /* [IN] Controller ID */
    gUsbToAppEvent_t event,           /* [IN] Event Type */
    void* val               /* [OUT] Pointer to configuration Value */
)
{
  pfUsbToAppEventHandler_t cdcClassCallback;
  uint8_t index;
  usbEpStruct_t* usbNCEpDptr = gpaUsbClassDescriptor[controllerId]->usbNonControlEpDescriptor;    
  uint8_t count = gpaUsbClassDescriptor[controllerId]->numNonControlEp;
  if(event == gUsbToAppEvent_EnumComplete_c)
  {
	  /* deinitialize all endpoints in case they were initialized */
		index = count;
		while(index > 0) 
		{   
		  index--;
			(void)USB_DeviceDeinitEndpoint(controllerId,usbNCEpDptr[index].number, usbNCEpDptr[index].direction);
		}
        
   /* intialize all non control endpoints */
    while(index < count)
    {
      usbEpStruct_t* epStruct = usbNCEpDptr + index;
      (void)USB_DeviceInitEndpoint(controllerId, epStruct, TRUE);
      /* register callback service for Non Control EndPoints */
      switch(epStruct->type)
      {
        #if gUSB_CDC_CIC_NotifElemSupport_d
        case gUsbEpType_Interrupt_c :
               gNotifyEndpoint = epStruct->number;  
               (void)USB_DeviceRegisterService(controllerId,
                 (uint8_t)(gUsbService_Ep0_d+epStruct->number),USB_Class_CDC_ServiceCicNotify);
        break;
        #endif
        case gUsbEpType_Bulk_c :
               if(epStruct->direction == gUsbEpDirection_Out_c)
               {
                 (void)USB_DeviceRegisterService(controllerId,
                    (uint8_t)(gUsbService_Ep0_d+epStruct->number),USB_Class_CDC_ServiceDicBulkOut);
               }
               else
               {
                 (void)USB_DeviceRegisterService(controllerId,
                   (uint8_t)(gUsbService_Ep0_d+epStruct->number),USB_Class_CDC_ServiceDicBulkIn);
               }
        break;

        default: 
        break;
      }
        /* set the EndPoint Status as Idle in the device layer */
      (void)USB_DeviceSetStatus(controllerId,
                (uint8_t)(gUsbComponentStatus_Endpoint_d | epStruct->number |
                (epStruct->direction << gUsbComponent_DirectionShift_d)),
                (uint8_t)gUsbStatus_Idle_d);
      index++;
    }
  }
  else if(event == gUsbToAppEvent_BusReset_c)
  {
    gDTEStatus = (uint8_t)gUninitialisedVal_d;
  }
      
  cdcClassCallback = gpaUsbClassDescriptor[controllerId]->pfUsbToAppEventHandler;
  if(cdcClassCallback != NULL)
  {
    USB_IsrEvent((usbMsg_t*) val);    
  }
}

/**************************************************************************//*!
 *
 * @name  USB_Other_Requests
 *
 * @brief The funtion provides flexibilty to add class and vendor specific
 *        requests
 *
 * @param controller_ID : Controller ID
 * @param setup_packet  : Setup packet received
 * @param data:         : Data to be send back
 * @param size:         : Size to be returned
 *
 * @return status:
 *                        gUsbErr_NoError_c : When Successfull
 *                        Others : When Error
 *
 ******************************************************************************
 * Handles CDC Class requests and forwards vendor specific request to the
 * application
 *****************************************************************************/
static uint8_t USB_CDC_OtherRequests 
(
    uint8_t controllerId,           /* [IN] Controller ID */
    usbSetupPacket_t * setupPacket,/* [IN] Pointer to Setup Packet Received */
    uint8_t* *pData,           /* [OUT] Pointer to Data Buffer to be sent */
    usbPacketSize_t *pSize           /* [OUT] Size of Data buffer */
)
{
  uint8_t status = gUsbErr_InvalidReqType_c;
  if((setupPacket->requestType & gUsbRequestClassMask_d) == gUsbRequestClass_Class_d)
  {  
    /* class request so handle it here */
    status=gUsbErr_NoError_c;

    /* call for class/subclass specific requests */
    switch(setupPacket->request)
    {
      case gUSB_CDC_ClassRequest_SendEncapsulatedCommand_d :
             /* Add code to transfer Request and Acknowledgement */
             *pSize=0;
      break;
      case gUSB_CDC_ClassRequest_GetEncapsulatedResponse_d :
             /* Add code for handling Transfer Response/Requests and Notification        */
             *pSize=0;
      break;
      case gUSB_CDC_ClassRequest_SetCommFeature_d :
             *pData = (uint8_t*)setupPacket + gUsbSetupPacketSize_d; 
             status = USB_Class_CDC_PSTN_SetCommFeature(controllerId,setupPacket, pData, pSize);
             
      break;
      case gUSB_CDC_ClassRequest_GetCommFeature_d :
             status = USB_Class_CDC_PSTN_GetCommFeature(controllerId, setupPacket, pData, pSize);
      break;
      case gUSB_CDC_ClassRequest_ClearCommFeature_d : 
             {
               uint8_t data[2] = {0,0};
               *pData = data;
               status = USB_Class_CDC_PSTN_SetCommFeature(controllerId,setupPacket, pData, pSize);
             }
      break;
      case gUSB_CDC_ClassRequest_GetLineCoding_d :
             status = USB_Class_CDC_PSTN_GetLineCoding(controllerId, setupPacket, pData, pSize);
      break;
      case gUSB_CDC_ClassRequest_SetLineCoding_d :
             status = USB_Class_CDC_PSTN_SetLineCoding(controllerId, setupPacket, pData, pSize);
      break;
      case gUSB_CDC_ClassRequest_SetControlLineState_d :
             status = USB_Class_CDC_PSTN_SetCtrlLineState(controllerId, setupPacket, pData, pSize);
      break;
      case gUSB_CDC_ClassRequest_SendBreak_d :
             status = USB_Class_CDC_PSTN_SendBreak(controllerId, setupPacket, pData, pSize);
      break;
      default:  
             *pSize=0;
    }
  }
  else if((setupPacket->requestType & gUsbRequestClassMask_d) == gUsbRequestClass_Vendor_d)
  {
    /* vendor specific request  */
    pfusbReqHandler_t       pfVendorReqCallback;
    pfVendorReqCallback =  gpaUsbClassDescriptor[controllerId]->pfVendorReqHandler;
    if(pfVendorReqCallback != NULL)
    {
      status = pfVendorReqCallback(controllerId, setupPacket, pData, pSize);
    }   
  }
  return status;
}


/*****************************************************************************
 * Global Functions
 *****************************************************************************/
/**************************************************************************//*!
 *
 * @name  USB_Class_CDC_Init
 *
 * @brief The funtion initializes the Device and Controller layer
 *
 * @param controller_ID:        Controller ID
 * @param cdc_class_callback:   CDC Class Callback
 * @param vendor_req_callback:  vendor specific class request callback
 * @param param_callback:       PSTN Callback
 *
 * @return status
 *         gUsbErr_NoError_c           : When Successfull
 *         Others           : Errors
 ******************************************************************************
 * This function initializes the CDC Class layer and layers it is dependent upon
 *****************************************************************************/
uint8_t USB_Class_CDC_Init 
(
    uint8_t    controllerId                /* [IN] Controller ID */
)
{
        
  uint8_t count = gpaUsbClassDescriptor[controllerId]->numNonControlEp;
  /* Initialize the device layer*/
  uint8_t status = USB_DeviceInit(controllerId, (uint8_t)(count+1));
  /* +1 is for Control Endpoint */
  gDTEStatus = (uint8_t)gUninitialisedVal_d;
  gNotifyEndpoint = 0;
  if(status == gUsbErr_NoError_c)
  {
    /* Initialize the generic class functions */
    status = USB_ClassInit(controllerId, USB_CDC_ClassEvent, USB_CDC_OtherRequests);
  }
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_Class_CDC_DeInit
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
 *This function de-initializes the CDC Class layer
 *****************************************************************************/
uint8_t USB_Class_CDC_DeInit 
(
  uint8_t controllerId              /* [IN] Controller ID */
) 
{
  uint8_t status;
  status = USB_ClassDeInit(controllerId); 
  return status;
}


/******************************************************************************
 *
 * @name  USB_Class_CDC_SetDicInServiceCallback
 *
 * @brief 
   
 ******************************************************************************/
 
 void USB_Class_CDC_SetDicInServiceCallback(void (*pf)(usbDeviceEvent_t* event))
 {
   pfCDCDicInServiceCallBack = pf;
 }
 /******************************************************************************
 *
 * @name  USB_Class_CDC_SetDicOutServiceCallback
 *
 * @brief 
   
 ******************************************************************************/

 void USB_Class_CDC_SetDicOutServiceCallback(void (*pf)(usbDeviceEvent_t* event))
 {
   pfCDCDicOutServiceCallBack = pf;
 }

/* EOF */
#endif