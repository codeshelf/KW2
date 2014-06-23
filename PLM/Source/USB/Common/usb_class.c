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
 * @file usb_class.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief The file contains USB stack Class module implementation.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "USB_Configuration.h"  
#include "usb_class.h"      /* USB class Header File */
#include "usb_dciapi.h"   
#include "usb_devapi.h"     /* USB device Header file */
#include "usb_framework.h"  /* USB framework module header file */
#include "USB.h"
#include "Interrupt.h"   

#if gUsbIncluded_d

/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/

/****************************************************************************
 * Global Variables
 ****************************************************************************/
 /* Class callback pointer */
 #define  gapfClassCallback(controller_ID) gpfClassFrameworkCallback
/* save the device state before device goes to suspend state */
static uint16_t gaDeviceStateBeforeSuspend[gNumUsbControllers_d];
/*****************************************************************************
 * Local Types - None
 *****************************************************************************/

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static void USB_Service_Suspend (usbDeviceEvent_t * event );
static void USB_Service_Resume (usbDeviceEvent_t * event );
static void USB_Service_Stall (usbDeviceEvent_t * event );
static void USB_Service_SOF (usbDeviceEvent_t * event );
static void USB_Service_Reset (usbDeviceEvent_t * event );
static void USB_Service_Error (usbDeviceEvent_t * event );

/*****************************************************************************
 * Local Variables
 *****************************************************************************/

 /*****************************************************************************
 * Local Functions - None
 *****************************************************************************/

/*****************************************************************************
 * Global Functions
 *****************************************************************************/
/**************************************************************************//*!
 *
 * @name  USB_Service_Suspend
 *
 * @brief The funtion is called when host suspends the USB port
 *
 * @param event     : Pointer to USB Event Structure
 *
 * @return None
 ******************************************************************************
 * Sets the device state as gUsbDeviceState_Suspend_d
 *****************************************************************************/
static void USB_Service_Suspend (
    usbDeviceEvent_t * event /* [IN] Pointer to USB Event Structure */
)
{
#if  gUsbAppNotificationOnSuspend_d
  usbMsg_t usbMsg;  
#endif
  /* Get the status of the device before suspend, so that on resume we can get back to the same state */
  (void)USB_DeviceGetStatus(event->controllerId,gUsbComponentStatus_DeviceState_d, &gaDeviceStateBeforeSuspend[event->controllerId]);
  /* Set the device state in the Device Layer to SUSPEND */
  (void)USB_DeviceSetStatus(event->controllerId, gUsbComponentStatus_DeviceState_d,gUsbDeviceState_Suspend_d);
#if  gUsbAppNotificationOnSuspend_d
  usbMsg.controllerId = event->controllerId;
  usbMsg.eventType = gUsbToAppEvent_Suspend_c;
  gapfClassCallback(event->controllerId)(event->controllerId, gUsbToAppEvent_Suspend_c, &usbMsg);
#endif  
}

/**************************************************************************//*!
 *
 * @name  USB_Service_Resume
 *
 * @brief The funtion is called when host resumes the USB port
 *
 * @param event     : Pointer to USB Event Structure
 *
 * @return None
 ******************************************************************************
 * Restore the state of the device before suspend
 *****************************************************************************/
static void USB_Service_Resume (
    usbDeviceEvent_t * event /* [IN] Pointer to USB Event Structure */
)
{
  uint16_t deviceState;
  (void)USB_DeviceGetStatus(event->controllerId, gUsbComponentStatus_DeviceState_d,&deviceState);
  if(deviceState == gUsbDeviceState_Suspend_d)
  {
    /*Set the device state in the Device Layer to the state before suspend */
    (void)USB_DeviceSetStatus(event->controllerId,
            gUsbComponentStatus_DeviceState_d, gaDeviceStateBeforeSuspend[event->controllerId]);
  }
}

/**************************************************************************//*!
 *
 * @name  USB_Service_Stall
 *
 * @brief The funtion is called when endpoint is stalled
 *
 * @param event: Pointer to USB Event Structure
 *
 * @return None
 ******************************************************************************
 * This function sends STALL Packet for the endpoint to be stalled. Also, sets
 * the status of Endpoint as STALLED
 *****************************************************************************/
static void USB_Service_Stall (
    usbDeviceEvent_t * event /* [IN] Pointer to USB Event Structure */
)
{
  if(event->epNum == gUsbControlEndpoint_d)
  {
  /* Update the Endpoint Status in the Device Layer to Idle */
  (void)USB_DeviceSetStatus(event->controllerId,
            (uint8_t)(gUsbComponentStatus_Endpoint_d | gUsbControlEndpoint_d |
            (event->direction << gUsbComponent_DirectionShift_d)),
            (uint16_t)gUsbStatus_Idle_d);
  }
  return;
}

/**************************************************************************//*!
 *
 * @name  USB_Service_SOF
 *
 * @brief The funtion is called when SOF flag is set (from ISR)
 *
 * @param event: Pointer to USB Event Structure
 *
 * @return None
 ******************************************************************************
 * This function is called when SOF token is received by controller. Updates
 * SOF Count status.
 *****************************************************************************/
static void USB_Service_SOF (
    usbDeviceEvent_t * event     /* [IN] Pointer to USB Event Structure */
)
{
  uint16_t sofCount,deviceState;
  uint8_t i;
  /* update SOF */
  sofCount = *((uint16_t*)event->pBuffer);
  /* write SOF to status */
  for(i=0 ; i< gNumUsbControllers_d ; i++)
  {
    (void)USB_DeviceGetStatus( i , gUsbComponentStatus_DeviceState_d, &deviceState);
    if(deviceState < gUsbDeviceState_Suspend_d)
    {
      (void)USB_DeviceSetStatus( i , gUsbComponentStatus_SOFCount_d , sofCount);   
    }
  }
  return;
}
/**************************************************************************//*!
 *
 * @name  USB_Service_Reset
 *
 * @brief The funtion is called upon a bus reset event.
              Initializes the control endpoint.
 *
 * @param event: Pointer to USB Event Structure
 *
 * @return None
 ******************************************************************************
 * Reset Callback function. This function re-initializes CONTROL Endpoint
 *****************************************************************************/


static void USB_Service_Reset 
(
    usbDeviceEvent_t * event  /* [IN] Pointer to USB Event Structure */
)
{
  usbEpStruct_t epStruct;
  usbMsg_t usbMsg;

  /* Initialize the endpoint 0 in both directions */
  epStruct.direction = gUsbEpDirection_In_c;
  epStruct.number = gUsbControlEndpoint_d;
  epStruct.size = gpaUsbClassDescriptor[event->controllerId]->controlEpSize;
  epStruct.type = gUsbEpType_Control_c;
  /* Deinit Endpoint in case its already initialized */
  (void)USB_DeviceDeinitEndpoint(event->controllerId , epStruct.number, epStruct.direction);
  epStruct.direction = gUsbEpDirection_Out_c;
  (void)USB_DeviceDeinitEndpoint(event->controllerId,	epStruct.number, epStruct.direction);
   /* now initialize the endpoint */
  epStruct.direction = gUsbEpDirection_In_c;	
  (void)USB_DeviceInitEndpoint(event->controllerId, &epStruct, TRUE);
  epStruct.direction = gUsbEpDirection_Out_c;
  (void)USB_DeviceInitEndpoint(event->controllerId, &epStruct, TRUE);
  /* set the default device state */
  (void)USB_DeviceSetStatus(event->controllerId , gUsbComponentStatus_DeviceState_d , gUsbDeviceState_Default_d);
  /* set the default device state */
  (void)USB_DeviceSetStatus(event->controllerId, gUsbComponentStatus_Device_d, gbmUsbAttributes_SelfPowered_d >> gUsbSelfPoweredBitShift_d);
  // gbmUsbAttributes_SelfPowered_d must be moved in usbClassDescriptor_t
  /* set the EndPoint Status as Idle in the device layer */
  (void)USB_DeviceSetStatus(event->controllerId,
        (uint8_t)(gUsbComponentStatus_Endpoint_d | gUsbControlEndpoint_d |
        (gUsbEpDirection_In_c << gUsbComponent_DirectionShift_d)),
        gUsbStatus_Idle_d);
  /* let the application know that bus reset has taken place */
  usbMsg.controllerId = event->controllerId;
  usbMsg.eventType = gUsbToAppEvent_BusReset_c;
  gapfClassCallback(event->controllerId)(event->controllerId, gUsbToAppEvent_BusReset_c, &usbMsg);
  return;
}

/**************************************************************************//*!
 *
 * @name  USB_Service_Error
 *
 * @brief The funtion is called when an error has been detected
 *
 * @param event: Pointer to USB Event Structure
 *
 * @return None
 ******************************************************************************
 * Calls application with the error code received from the lower layer
 *****************************************************************************/
static void USB_Service_Error (
    usbDeviceEvent_t * event /* [IN] Pointer to USB Event Structure */
)
{
  /* notify the application of the error */
  usbMsg_t usbMsg;
  usbMsg.controllerId = event->controllerId;
  usbMsg.eventType = gUsbToAppEvent_Error_c;
  usbMsg.eventParam.errorEvent.errors = event->errors;
  gapfClassCallback(event->controllerId)(event->controllerId, gUsbToAppEvent_Error_c,&usbMsg);
  return;
}


/**************************************************************************//*!
 *
 * @name  USB_ClassInit
 *
 * @brief The funtion initializes the Class Module
 *
 * @param controller_ID         : Controller ID
 * @param class_callback        : Class callback
 * @param other_req_callback    : Other Requests Callback
 *
 * @return status
 *         gUsbErr_NoError_c           : When Successfull
 *         Others           : Errors
 ******************************************************************************
 * Initializes USB Class Module
 *****************************************************************************/
uint8_t USB_ClassInit (
    uint8_t    controllerId,                /* [IN] Controller ID */
    pfUsbToAppEventHandler_t classCallback,      /* [IN] Class Callback */
    pfusbReqHandler_t       otherReqCallback   /* [IN] Other Requests Callback */
)
{
  uint8_t status = USB_Framework_Init(controllerId , classCallback , otherReqCallback);
  /* save callback address */
  gapfClassCallback(controllerId) = classCallback;
  if(status == gUsbErr_NoError_c)
  {
    /* Register all the services here */
    status |= USB_DeviceRegisterService(controllerId , gUsbService_BusReset_d , USB_Service_Reset);
    status |= USB_DeviceRegisterService(controllerId , gUsbService_SOF_d , USB_Service_SOF);
    status |= USB_DeviceRegisterService(controllerId , gUsbService_Sleep_d , USB_Service_Suspend);
    status |= USB_DeviceRegisterService(controllerId , gUsbService_Resume_d,USB_Service_Resume);
    status |= USB_DeviceRegisterService(controllerId , gUsbService_Stall_d , USB_Service_Stall);
    status |= USB_DeviceRegisterService(controllerId , gUsbService_Error_d , USB_Service_Error);
     /* set the device state as powered */
    (void)USB_DeviceSetStatus(controllerId , gUsbComponentStatus_DeviceState_d , gUsbDeviceState_Powered_d);
    gaDeviceStateBeforeSuspend[controllerId] = gUsbDeviceState_Powered_d;
  }
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_ClassDeInit
 *
 * @brief The funtion De-initializes the Class Module
 *
 * @param controller_ID         : Controller ID
 *
 * @return status
 *         gUsbErr_NoError_c           : When Successfull
 *         Others           : Errors
 ******************************************************************************
 * De-initializes USB Class Module
 *****************************************************************************/
uint8_t USB_ClassDeInit 
(
    uint8_t    controllerId          /* [IN] Controller ID */
) 
{
  uint8_t status = gUsbErr_NoError_c;
  status = USB_Framework_DeInit(controllerId);    
  /* Free class_callback */
  gapfClassCallback(controllerId) = NULL;
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_ClassSendData
 *
 * @brief The funtion calls the device to send data upon recieving an IN token
 *
 * @param controller_ID     : Controller ID
 * @param ep_num            : Endpoint number
 * @param buff_ptr          : Buffer to send
 * @param size              : Length of transfer
 *
 * @return status
 *         gUsbErr_NoError_c           : When Successfull
 *         Others           : Errors
 ******************************************************************************
 * Used by Application to send Data on USB Bus if not suspended
 *****************************************************************************/
uint8_t USB_ClassSendData_ (
    uint8_t           controllerId, /* [IN] Controller ID */
    uint8_t           epNum,        /* [IN] Endpoint number */
    uint8_t*          pBuff,      /* [IN] Buffer to send */
    usbPacketSize_t   size  ,         /* [IN] Length of the transfer */
    bool_t            sendShort
)
{
  uint8_t status = gUsbErr_NoError_c;
  uint16_t deviceState, state;
  uint32_t ccr;
  if(controllerId >= gNumUsbControllers_d)
  {
    return gUsbErr_InvalidParam_c;
  }
  //USB_Lock();  
  ccr = IntDisableAll();
  (void)USB_DeviceGetStatus(controllerId , gUsbComponentStatus_DeviceState_d , &deviceState);
  (void)USB_DeviceGetStatus(controllerId , gUsbComponentStatus_Device_d , &state);
  if((deviceState == gUsbDeviceState_Suspend_d) && 
           (state & gUsbRemoteWakeupStatusMask_d ) && 
           (USB_Framework_RemoteWakeup(controllerId) == TRUE))
  {
    /* Resume the bus */
      USB_DeviceAssertResume(controllerId);    
      deviceState = gaDeviceStateBeforeSuspend[controllerId];
    /* Set the device state in the Device Layer to DEFAULT */
      (void)USB_DeviceSetStatus(controllerId,gUsbComponentStatus_DeviceState_d, gaDeviceStateBeforeSuspend[controllerId]);
    
	}
  if(deviceState != gUsbDeviceState_Suspend_d)
  {
    /* if not suspended */
    status = USB_DeviceSendData(controllerId, epNum, pBuff, size, sendShort);
  }
 // USB_UnLock();
  IntRestoreAll(ccr);
  return status;
 }



#endif // gUsbIncluded_d
