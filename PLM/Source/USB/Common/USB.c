/************************************************************************************
* Source file of the application specific utility services
*
* (c) Copyright 2010, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/

#include "EmbeddedTypes.h"
#include "USB_Class.h"
#include "USB_Interface.h"
#include "USB_Configuration.h"
#include "USB_Framework.h"
#include "Utilities_Interface.h"
#include "USB.h"
#include "Interrupt.h"
#if gUsbHidEnabled_d
#include "usb_hid.h"
#endif
#if(gUsbCdcEnabled_d == TRUE)
#include "usb_cdc.h"    /* USB CDC Class Header File */
#endif
#include "UsbMsgQueue.h"
#include "usb_sim_settings.h"
#if gUsbIncluded_d
/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/
/* Events for USB task */
#define gUsbIrqEvent_c  (1<<0)


/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/
static usbError_t USB_AddControllerUnprotected( const usbClassDescriptor_t* pUsbClassDescriptor, uint8_t controllerId);
static usbError_t USB_RemoveControllerUnprotected(uint8_t controllerId);

/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/


/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/
/* Queue storing pending events from the USB ISR handler */
static uint8_t gUsbLockLevel = 0;

/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/
/* Id for USB task */
//tsTaskID_t gUsbTaskId; //@SMAC
const usbClassDescriptor_t* gpaUsbClassDescriptor[gNumUsbControllers_d];

pfUsbToAppEventHandler_t gpfClassFrameworkCallback = NULL;

/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/
static void USB_Task(void);

/************************************************************************************
* Usb_Init - 
*   
* Return value:
*    
* 
************************************************************************************/
void USB_Init(void)
{
  USB_SIM_Settings(); 
  /* Create the USB task */
  gUsbLockLevel = 0;
//  gUsbTaskId = TS_CreateTask(gTsUsbTaskPriority_c, USB_Task); //@SMAC
  /* Init the queue storing events from ISR routine */
  UsbMsgQueue_Init();
  /* Clear the table storing the class descriptor of each USB controller */
  MemorySet( gpaUsbClassDescriptor, 0, (uint16_t)sizeof(gpaUsbClassDescriptor));
 }


/************************************************************************************
* USB_AddController - 
*   
* Return value:
*    
* 
************************************************************************************/
usbError_t USB_AddController( const usbClassDescriptor_t* pUsbClassDescriptor, uint8_t controllerId)
{
  usbError_t  usbError;
  uint32_t ccr;
   //USB_Lock();
  ccr = IntDisableAll();
  usbError = USB_AddControllerUnprotected(pUsbClassDescriptor,controllerId); 
  // USB_UnLock();
  IntRestoreAll(ccr);
  return usbError;
}




/************************************************************************************
* USB_RemoveController - 
*   
* Return value:
*    
* 
************************************************************************************/
usbError_t USB_RemoveController(uint8_t controllerId) 
{
  usbError_t  usbError;
  uint32_t ccr;
  //USB_Lock();
  ccr = IntDisableAll();
  usbError = USB_RemoveControllerUnprotected(controllerId); 
  //USB_UnLock();
  IntRestoreAll(ccr);
  return usbError;
}



/************************************************************************************
* USB_GetFirstFreeController
*   
* Return value:
*    
* 
************************************************************************************/
uint8_t USB_GetFirstAvailableController(void)
{
  uint8_t ctrlNo;
  uint8_t firstAvailableController = gUsbInvalidControllerNumber_d;
  for(ctrlNo = 0 ; ctrlNo < gNumUsbControllers_d; ctrlNo++)
  {
    if(gpaUsbClassDescriptor[ctrlNo]  ==  NULL)    
    {
      firstAvailableController =  ctrlNo;
      break;     
    }
  }
  return firstAvailableController;    
}



/************************************************************************************
* USB_IsrEvent - 
*   
* Return value:
*    
* 
************************************************************************************/

void USB_IsrEvent(usbMsg_t* pUsbMsg)
{
  /* Add the event in the queue */
  usbMsgQueueErr_t usbMsgQueueErr;
  usbMsgQueueErr = UsbMsgQueue_Push(pUsbMsg);
  if(usbMsgQueueErr == usbMsgQueueErr_NoError)
  {
    /* Send message to USB task to process the event */
//    TS_SendEvent(gUsbTaskId, gUsbIrqEvent_c);  //@SMAC
    USB_Task();
    
  }
  #ifdef gUSB_Debug_d 
  else
  {
   // asm bgnd;
    while(TRUE);
  }
  #endif  
}          

/************************************************************************************
* USB_IsDeviceConnected 
*   
* Return value:
*    
* 
************************************************************************************/

bool_t USB_IsDeviceConnected(uint8_t controllerId)
{
  if( controllerId  >=  gNumUsbControllers_d )
  {
    return FALSE;
  }
  if(gpaUsbClassDescriptor[controllerId] == NULL) 
  {
    return FALSE;     
  }
  else
  {
    return TRUE;  
  }
}          
/************************************************************************************
* USB_Lock 
*   
* Return value:
*    
* 
************************************************************************************/

void USB_Lock(void)
{
  if(gUsbLockLevel==0)
  {
    USB_SIM_UsbIntDisable();
  }
  gUsbLockLevel++;
}
/************************************************************************************
* USB_Unlock 
*   
* Return value:
*    
* 
************************************************************************************/

void USB_UnLock(void)
{
  if(gUsbLockLevel)
  {
    gUsbLockLevel--;
    if(gUsbLockLevel == 0) 
    {
      USB_SIM_UsbIntEnable();
    }
  }
}
/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
*******************************************************************************/

/************************************************************************************
* USB_AddControllerUnprotected - 
*   
* Return value:
*    
* 
************************************************************************************/
static usbError_t USB_AddControllerUnprotected( const usbClassDescriptor_t* pUsbClassDescriptor, uint8_t controllerId)
{
  usbError_t  usbError = gUsbErr_NoError_c;
  if( controllerId  >=  gNumUsbControllers_d )
  {
    return gUsbErr_InvalidParam_c ;
  }
  if( gpaUsbClassDescriptor[controllerId]  !=  NULL)
  {
    return gUsbErr_InvalidParam_c ;
  }
  if(pUsbClassDescriptor == NULL)
  {
    return gUsbErr_InvalidParam_c ;      
  }
  
  switch(pUsbClassDescriptor->classId)
  {
    case gUsbClassId_Cdc_c:
    #if(gUsbCdcEnabled_d == FALSE)
      return gUsbErr_InvalidParam_c ;
    #else  
      gpaUsbClassDescriptor[controllerId] = pUsbClassDescriptor;
      usbError = USB_Class_CDC_Init(controllerId);                    
    #endif
    break;
    
    case gUsbClassId_Hid_c:
    #if(gUsbHidEnabled_d == FALSE)
      return gUsbErr_InvalidParam_c ;
    #else
      gpaUsbClassDescriptor[controllerId] = pUsbClassDescriptor;
      usbError = USB_Class_HID_Init(controllerId);                    
    #endif
    break;
      
  default:
    return gUsbErr_InvalidParam_c ;
  }
  /* Check that controller 0 is a HUB. If not, return error as a new controller can only be added
  on a HUB */ 
   if(gUsbErr_NoError_c != usbError)
  {
    gpaUsbClassDescriptor[controllerId]  =  NULL ;
  }

  return usbError;
}


/************************************************************************************
* USB_RemoveControllerUnprotected - 
*   
* Return value:
*    
* 
************************************************************************************/
static usbError_t USB_RemoveControllerUnprotected(uint8_t controllerId)
{
  usbError_t  usbError = gUsbErr_NoError_c;
  
  if( controllerId  >=  gNumUsbControllers_d )
  {
    return gUsbErr_InvalidParam_c ;
  }
  if( gpaUsbClassDescriptor[controllerId]  ==  NULL)
  {
    return gUsbErr_InvalidParam_c ;
  }

  switch(gpaUsbClassDescriptor[controllerId]->classId)
  {
    case gUsbClassId_Cdc_c:
      #if(gUsbCdcEnabled_d == TRUE)
      usbError = USB_Class_CDC_DeInit(controllerId);
      #else
      return gUsbErr_InvalidParam_c ;
      #endif
    break;
    case gUsbClassId_Hid_c:
     #if(gUsbHidEnabled_d == TRUE)
      usbError = USB_Class_HID_DeInit(controllerId);                    
     #else 
      return gUsbErr_InvalidParam_c ;
     #endif
    break;
    default:
    return gUsbErr_InvalidParam_c ;
    }
 
    gpaUsbClassDescriptor[controllerId] = NULL;
    return usbError;  
}


/************************************************************************************
* USB_Task - 
*   
* Return value:
*    
* 
************************************************************************************/
static void USB_Task(void)  //@SMAC
{ 
  usbMsg_t* pUsbMsg;
  pfUsbToAppEventHandler_t pfUsbToAppEventHandler = NULL;
  
  
  if(gUsbIrqEvent_c)
  {
    /* Check if there are pending events in the ISR queue */
    if(UsbMsgQueue_MsgPending())
    {
      /* Get event out of queue */
      pUsbMsg = UsbMsgQueue_GetFirstMsg();
      #ifdef gUSB_Debug_d 
      //debug start
      if(pUsbMsg->controllerId >= gNumUsbControllers_d)
       {
         while(TRUE);
       }
      //debug end
      #endif
      if(gpaUsbClassDescriptor[pUsbMsg->controllerId] != NULL)
      {
        pfUsbToAppEventHandler = gpaUsbClassDescriptor[pUsbMsg->controllerId]->pfUsbToAppEventHandler;
      }
      
      if(pfUsbToAppEventHandler != NULL)
        {
         pfUsbToAppEventHandler(pUsbMsg->controllerId , pUsbMsg->eventType , pUsbMsg ) ;
        }
      
      (void)UsbMsgQueue_Pop();
    }
  }
   
  /* If there are unprocessed events in the queue, send a message to the USB
  task to run again  */
  if(UsbMsgQueue_MsgPending())
  {
    //TS_SendEvent(gUsbTaskId, gUsbIrqEvent_c); //@SMAC
    USB_Task();
  }
}


#endif // gUsbIncluded_d
