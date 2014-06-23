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
 * @file usb_driver.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief The file contains S08 USB stack device layer implementation.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "USB_Interface.h"
#include "USB.h"
#include "usb_devapi.h" /* USB Device Layer API Header File */
#include "usb_dciapi.h" /* USB Controller API Header File */
#include "Utilities_Interface.h" 

#if gUsbIncluded_d
/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/


/****************************************************************************
 * Global Variables
 ****************************************************************************/

/* Array of USB Service pointers */
#define gUsbCB(controller_ID) gUsbCB_
static usbServiceCallback_t gUsbCB_[gUsbService_Max_d];  
/* Array of USB Component Status */
/* Test mode is the last service */
#define gUsbComponentStatus(controller_ID) gUsbComponentStatus_
static uint16_t gUsbComponentStatus_[gUsbComponentStatus_TestMode_d]; 
/* Array of USB Endpoint Status */
#define gUsbEpStatus(controller_ID) gUsbEpStatus_
static uint16_t gUsbEpStatus_[gNumMaxEnpNumber_d];
/* Current un-initialized non CONTROL Endpoint */
#define gEpNo(controller_ID) gEpNo_
static uint8_t gEpNo_=0;     
/* Maximum number of Non CONTROL Endpoint required by upper layer */
#define gEpNoMax(controller_ID) gEpNoMax_
static uint8_t gEpNoMax_=0; 

/*****************************************************************************
 * Local Types - None
 *****************************************************************************/

/*****************************************************************************
 * Local Functions Prototypes - None
 *****************************************************************************/
static void USB_DeviceInitParams(uint8_t    controllerId);

/*****************************************************************************
 * Local Variables - None
 *****************************************************************************/

/*****************************************************************************
 * Local Functions 
 *****************************************************************************/
/**************************************************************************//*!
 *
 * @name  USB_DeviceInitParams
 *
 * @brief The funtion initializes the Device Layer Structures
 *
 * @param  None
 *
 * @return None
 *
 ******************************************************************************
 * Initializes USB Device Layer Structures
 *****************************************************************************/
static void USB_DeviceInitParams(uint8_t    controllerId)
{
  uint8_t loopIndex=0;
  (void)controllerId;
  gEpNo(controllerId)= gEpNoMax(controllerId); /* 1 control endpoint */
    /* 
       Initialize gUsbComponentStatus_DeviceState_d, gUsbComponentStatus_Interface_d,
       gUsbComponentStatus_Address_d, gUsbComponentStatus_CurrentConfig_d, gUsbComponentStatus_SOFCount_d
       and gUsbComponentStatus_Device_d to gUsbStatus_Unknown_d 
    */
  for(loopIndex = 0; loopIndex < gUsbComponentStatus_TestMode_d; loopIndex++)
  {
    gUsbComponentStatus(controllerId)[loopIndex] = gUsbStatus_Unknown_d;
  }
  /* Initialize status of All Endpoints to gUsbStatus_Disabled_d */
  for(loopIndex = 0; loopIndex < gNumMaxEnpNumber_d; loopIndex++)
  {
    gUsbEpStatus(controllerId)[loopIndex] = gUsbStatus_Disabled_d;
  }
}



/*****************************************************************************
 * Global Functions
 *****************************************************************************/


/**************************************************************************//*!
 *
 * @name  USB_DeviceInit
 *
 * @brief The funtion initializes the Device and Controller layer
 *
 * @param controller_ID : Controller ID
 * @param endpoints     : Endpoint count of the application
 *
 * @return status
 *         gUsbErr_NoError_c                              : When Successfull
 *         gUsbErr_InvalidNumOfEndpoints_c     : When endpoints > max Supported
 ******************************************************************************
 * This function initializes the Device layer and the Controller layer of the
 * S08 USB stack. It initialised the variables used for this layer and then
 * calls the controller layer initialize function
 *****************************************************************************/
uint8_t USB_DeviceInit (
    uint8_t    controllerId, /* [IN] Controller ID */
    uint8_t    endpoints      /* [IN] Endpoint count of the application */
)
{

  /* validate endpoints param */
  if((endpoints > gNumMaxEnpNumber_d) || (endpoints < gNumMinEnpNumber_d))
  {
    return gUsbErr_InvalidNumOfEndpoints_c;
  }
  /*init variables */
  gEpNoMax(controllerId) = endpoints - 1;
  USB_DeviceInitParams(controllerId);
  /* Initialize all services to null value */
  MemorySet(gUsbCB(controllerId), 0, (uint16_t)(sizeof(usbServiceCallback_t) * gUsbService_Max_d));
    /* Call controller layer initialization function */
  return USB_DCI_Init(controllerId);

}
/**************************************************************************//*!
 *
 * @name  USB_DeviceDeinit
 *
 * @brief The funtion initializes the Device and Controller layer
 *
 * @param controller_ID : Controller ID
 * @param endpoints     : Endpoint count of the application
 *
 * @return status
 *         gUsbErr_NoError_c                              : When Successfull
 *         gUsbErr_InvalidNumOfEndpoints_c     : When endpoints > max Supported
 ******************************************************************************
 * This function initializes the Device layer and the Controller layer of the
 * S08 USB stack. It initialised the variables used for this layer and then
 * calls the controller layer initialize function
 *****************************************************************************/
uint8_t USB_DeviceDeinit (
    uint8_t    controllerId /* [IN] Controller ID */
)
{
  uint8_t error;
  error = USB_DCI_DeInit(controllerId );
  gEpNoMax(controllerId) = 0; //Keep this order
  USB_DeviceInitParams(controllerId);
  /* Initialize all services to null value */
  MemorySet(gUsbCB(controllerId), 0, (uint16_t)(sizeof(usbServiceCallback_t) * gUsbService_Max_d));
  return error;
}

/**************************************************************************//*!
 *
 * @name  USB_DeviceInitEndpoint
 *
 * @brief The funtion initializes the endpoint
 *
 * @param controller_ID : Controller ID
 * @param ep_ptr        : Pointer to endpoint detail structure
 * @param flag          : Zero termination flag
 *
 * @return status
 *         gUsbErr_NoError_c                              : When Successfull
 *         gUsbErr_EpInitFailed_c     : When endpoints > max Supported
 ******************************************************************************
 *
 * This function initializes an endpoint the Device layer and the Controller
 * layer in the S08 USB stack. It validate whether all endpoints have already
 * been initialized or not and then calls the controller layer endpoint
 *  initialize function
 *
 *****************************************************************************/
uint8_t USB_DeviceInitEndpoint (
    uint8_t               controllerId, /* [IN] Controller ID */
    usbEpStruct_t*        pUsbEpStruct,        /* [IN] Pointer to endpoint detail
                                            structure */
    uint8_t               flag           /* [IN] Zero termination flag */
)
{
  uint8_t status=gUsbErr_NoError_c;
  /* check if all endpoint have already been initialised */
  if((gEpNo(controllerId) == 0) && (pUsbEpStruct->number != gUsbControlEndpoint_d))
  {
    return gUsbErr_EpInitFailed_c;
  }
  /* call controller layer for initiazation */
  status = USB_DCI_InitEndpoint(controllerId, pUsbEpStruct, flag);
  /* if endpoint successfully initialised update counter */
  if ((pUsbEpStruct->number != gUsbControlEndpoint_d) && (status == gUsbErr_NoError_c))
  {
    gEpNo(controllerId)--;
  }

  return status;
}


/**************************************************************************//*!
 *
 * @name  USB_DeviceDeinitEndpoint
 *
 * @brief The funtion De-initializes the endpoint
 *
 * @param controller_ID : Controller ID
 * @param ep_ptr        : Pointer to endpoint detail structure
 * @param flag          : Zero termination flag
 *
 * @return status
 *         gUsbErr_NoError_c                              : When Successfull
 *         gUsbErr_EpDeinitFailed_c     : When endpoints > max Supported
 ******************************************************************************
 *
 * This function deinitializes an endpoint the Device layer and the Controller
 * layer in the S08 USB stack. It validate whether all endpoints have already
 * been deinitialized or not and then calls the controller layer endpoint
 * deinitialize function
 *
 *****************************************************************************/
uint8_t USB_DeviceDeinitEndpoint (
    uint8_t    controllerId,  /* [IN] Controller ID */
    uint8_t    epNum,         /* [IN] Endpoint number */
    uint8_t    direction       /* [IN] Direction */
)
{
  uint8_t status=gUsbErr_NoError_c;
  /* check if all endpoint have already been initialised */
  if((gEpNo(controllerId) == gEpNoMax(controllerId)) && (epNum != gUsbControlEndpoint_d))
  {
    return gUsbErr_EpDeinitFailed_c;
  }
  /* call controller layer for initiazation */
  status = USB_DCI_DeinitEndpoint(controllerId, epNum, direction);
  /* if endpoint successfully deinitialised update counter */
  if ((epNum != gUsbControlEndpoint_d) && (status == gUsbErr_NoError_c))
  {
    gEpNo(controllerId)++;
  }

    return status;
}

/**************************************************************************//*!
 *
 * @name  USB_DeviceGetStatus
 *
 * @brief The funtion retrieves various endpoint as well as USB component status
 *
 * @param controller_ID : Controller ID
 * @param component     : USB component
 * @param status        : Pointer to 16 bit return value
 *
 * @return status
 *         gUsbErr_NoError_c                : When Successfull
 *         gUsbErr_BadStatus_c     : When error
 *
 ******************************************************************************
 * This function retrieves the endpoint as well USB component status which is
 * stored by calling USB_DeviceSetStatus. This function can be called by Ap-
 * plication as well as the DCI layer.
 *****************************************************************************/
uint8_t USB_DeviceGetStatus (
    uint8_t        controllerId,  /* [IN]  Controller ID  */
    uint8_t        component,      /* [IN]  USB component */
    uint16_t*   status          /* [OUT] Pointer to 16 bit return value */
)
{
  /* get the endpoint number from component input variable */
  uint8_t epNum = (uint8_t)(component & gUsbComponentStatus_EpNumberMask_d);
  (void)controllerId;
  
  if((component <= gUsbComponentStatus_TestMode_d) && (component >= gUsbComponentStatus_DeviceState_d))
  {
  /* Get the corresponding component status  -1 as components start from 1 */
    *status = gUsbComponentStatus(controllerId)[component-1];
  }
  else if ((component & gUsbComponentStatus_Endpoint_d) && (epNum < gNumMaxEnpNumber_d))
  {
    /* Get the corresponding endpoint status */
    *status = gUsbEpStatus(controllerId)[epNum];
  }
  else
  {
    return gUsbErr_BadStatus_c;
  }

  return gUsbErr_NoError_c;
}

/**************************************************************************//*!
 *
 * @name  USB_DeviceSetStatus
 *
 * @brief The funtion saves status of endpoints as well as USB components.
 *
 * @param controller_ID : Controller ID
 * @param component     : USB component
 * @param setting       : Value to be set
 *
 * @return status
 *         gUsbErr_NoError_c                : When Successfull
 *         gUsbErr_BadStatus_c     : When error
 *
 ******************************************************************************
 * This function sets the endpoint as well USB component status which can be
 * retrieved by calling USB_DeviceGetStatus. This function can be called by
 * Application as well as the DCI layer.
 *****************************************************************************/
uint8_t USB_DeviceSetStatus(
    uint8_t    controllerId,  /* [IN] Controller ID */
    uint8_t    component,      /* [IN] USB component */
    uint16_t   setting         /* [IN] Value to be set */
)
{
  /* get the endpoint number from component input variable */
  uint8_t epNum = (uint8_t)(component & gUsbComponentStatus_EpNumberMask_d);
  if((component <= gUsbComponentStatus_TestMode_d) && (component >= gUsbComponentStatus_DeviceState_d))
  {
    /* Set the corresponding component setting  -1 as components start from 1   */
    gUsbComponentStatus(controllerId)[component-1] = setting;
  }
  else if ((component & gUsbComponentStatus_Endpoint_d) && (epNum < gNumMaxEnpNumber_d))
  {
    uint8_t direction = (uint8_t)((component >> gUsbComponent_DirectionShift_d) & gUsbComponent_DirectionMask_d);
    /* HALT Endpoint */
    if(setting == gUsbStatus_Stalled_d)
    {
      USB_DeviceStallEndpoint(controllerId, epNum, direction);
    }
    else if((setting == gUsbStatus_Idle_d) && (gUsbEpStatus(controllerId)[epNum] == gUsbStatus_Stalled_d))
    {
      USB_DeviceUnstallEndpoint(controllerId, epNum, direction);
      if(epNum == gUsbControlEndpoint_d)
      {
        direction = (uint8_t)((direction == gUsbEpDirection_In_c)?(gUsbEpDirection_Out_c):(gUsbEpDirection_In_c));
        USB_DeviceUnstallEndpoint(controllerId, epNum, direction);
      }
    }
        /* Set the corresponding endpoint setting */
    gUsbEpStatus(controllerId)[epNum] = setting;
  }
  else
  {
    return gUsbErr_BadStatus_c;
  }
  return gUsbErr_NoError_c;
}

/**************************************************************************//*!
 *
 * @name  USB_DeviceRegisterService
 *
 * @brief The funtion registers a callback function from the Application layer
 *
 * @param controller_ID : Controller ID
 * @param type          : event type or endpoint number
 * @param service       : callback function pointer
 *
 * @return status
 *         gUsbErr_NoError_c                   : When Successfull
 *         gUsbErr_AllocService_c     : When invalid type or already registered
 *
 ******************************************************************************
 * This function registers a callback function from the application if it is
 * called not already registered so that the regitered callback function can
 * be if the event of that type occurs
 *****************************************************************************/
uint8_t USB_DeviceRegisterService(
    uint8_t                    controllerId, /* [IN] Controller ID           */
    uint8_t                    type,          /* [IN] type of event or endpoint
                                                     number to service       */
    usbServiceCallback_t      service        /* [IN] pointer to callback
                                                     function                */
)
{
  (void)controllerId;
  /* check if the type is valid and callback for the type is not already registered */
  if(((type  <= gUsbService_EpMax_d) ||
        ((type < gUsbService_Max_d) && (type >= gUsbService_BusReset_d))) &&
       (gUsbCB(controllerId)[type] == NULL))
  {
    /* register the callback function */
    gUsbCB(controllerId)[type] = service;
    return gUsbErr_NoError_c;
  }
  else
  {
    return gUsbErr_AllocService_c;
  }
}

/**************************************************************************//*!
 *
 * @name  USB_DeviceUnregisterService
 *
 * @brief The funtion unregisters an event or endpoint callback function
 *
 * @param controller_ID : Controller ID
 * @param type          : event type or endpoint number
 *
 * @return status
 *         gUsbErr_NoError_c                   : When Successfull
 *         gUsbErr_UnknownError_c     : When invalid type or not registered
 *
 *****************************************************************************
 * This function un registers a callback function which has been previously
 * registered by the application layer
 *****************************************************************************/
uint8_t USB_DeviceUnregisterService(
      uint8_t controllerId, /* [IN] Controller ID */
      uint8_t type           /* [IN] type of event or endpoint number
                                to service */
)
{
   (void)controllerId;
  /* check if the type is valid and callback for the type is already registered */
  if(((type  <= gUsbService_EpMax_d) ||
     ((type < gUsbService_Max_d) && (type >= gUsbService_BusReset_d))) &&
     (gUsbCB(controllerId)[type] != NULL))
  {
    /* unregister the callback */
    gUsbCB(controllerId)[type] = NULL;
    return gUsbErr_NoError_c;
  }
  else
  {
    return gUsbErr_UnknownError_c;
  }
}

/**************************************************************************//*!
 *
 * @name  USB_DeviceCallService
 *
 * @brief The funtion is a device layer event handler
 *
 * @param type  : Type of service or endpoint
 * @param event : Pointer to event structure
 *
 * @return status
 *         gUsbErr_NoError_c      : Always
 *
 *****************************************************************************
 *
 * This function calls the registered service callback function of the applic-
 * ation layer based on the type of event received. This function is called
 * from the DCI layer.
 *
 *****************************************************************************/
uint8_t USB_DeviceCallService(
    uint8_t             type,    /* [IN] Type of service or endpoint */
    usbDeviceEvent_t *  pUsbDeviceEvent    /* [IN] Pointer to event structure  */
)
{
  if(type == gUsbService_BusReset_d)
  {    /* if it is an reset interrupt then reset all status structures */
    USB_DeviceInitParams(pUsbDeviceEvent->controllerId);
  }

  /* check if the callback is registered or not */
  if(gUsbCB(pUsbDeviceEvent->controllerId)[type] != NULL)
  {
    /* call the callback function */
    gUsbCB(pUsbDeviceEvent->controllerId)[type](pUsbDeviceEvent);
  }

  return gUsbErr_NoError_c;
}

#endif// gUsbIncluded_d
