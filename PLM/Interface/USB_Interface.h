/*****************************************************************************
* Universal Serial Bus implementation interface
*
* Copyrigtht (c) 2011, Freescale, Inc. All rights reserved
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor
*
*****************************************************************************/

#ifndef _USB_INTERFACE_H
#define _USB_INTERFACE_H

#include "EmbeddedTypes.h"
#include "Interrupt.h"
#include "USB_Configuration.h"





/*****************************************************************************
******************************************************************************
* Public macros
******************************************************************************
*****************************************************************************/
#define gUsbInvalidControllerNumber_d  (0xff)
// USB Error codes
#define  gUsbErr_NoError_c                    (0x00)
#define  gUsbErr_Alloc_c                      (0x81)
#define  gUsbErr_BadStatus_c                  (0x82)
#define  gUsbErr_ClosedService_c              (0x83)
#define  gUsbErr_OpenService_c                (0x84)
#define  gUsbErr_TransferInProgress_c         (0x85)
#define  gUsbErr_EndpointStalled_c            (0x86)
#define  gUsbErr_AllocState_c                 (0x87)
#define  gUsbErr_DriverInstallFailed_c        (0x88)
#define  gUsbErr_DriverNotInstalled_c         (0x89)
#define  gUsbErr_InstallIsr_c                 (0x8A)
#define  gUsbErr_InvalidDeviceNum_c           (0x8B)
#define  gUsbErr_AllocService_c               (0x8C)
#define  gUsbErr_InitFailed_c                 (0x8D)
#define  gUsbErr_Shutdown_c                   (0x8E)
#define  gUsbErr_InvalidPipeHandle_c          (0x8F)
#define  gUsbErr_OpenPipeFailed_c             (0x90)
#define  gUsbErr_InitData_c                   (0x91)
#define  gUsbErr_SrpReqInvalidState_c         (0x92)
#define  gUsbErr_TxFailed_c                   (0x93)
#define  gUsbErr_RxFailed_c                   (0x94)
#define  gUsbErr_EpInitFailed_c               (0x95)
#define  gUsbErr_EpDeinitFailed_c             (0x96)
#define  gUsbErr_TrFailed_c                   (0x97)
#define  gUsbErr_BandwidthAllocFailed_c       (0x98)
#define  gUsbErr_InvalidNumOfEndpoints_c      (0x99)
#define  gUsbErr_NotSupported_c               (0x9A)

#define  gUsbErr_DeviceNotFound_c             (0xC0)
#define  gUsbErr_DeviceBusy_c                 (0xC1)
#define  gUsbErr_NoDeviceClass_c              (0xC3)
#define  gUsbErr_UnknownError_c               (0xC4)
#define  gUsbErr_InvalidBmreqType_c           (0xC5)
#define  gUsbErr_GetMemoryFailed_c            (0xC6)
#define  gUsbErr_InvalidMemType_c             (0xC7)
#define  gUsbErr_NoDescriptor_c               (0xC8)
#define  gUsbErr_NullCallback_c               (0xC9)
#define  gUsbErr_NoInterface_c                (0xCA)
#define  gUsbErr_InvalidCfigNum_c             (0xCB)
#define  gUsbErr_InvalidAnchor_c              (0xCC)
#define  gUsbErr_InvalidReqType_c             (0xCD)
#define  gUsbErr_NoMemory_c                   (0xCE)
#define  gUsbErr_ProcessLater_c               (0xCF)
#define  gUsbErr_InvalidParam_c               (0xD0)
#define  gUsbErr_HubNotMounted_c              (0xD1)
#define  gUsbErr_DevicesStillMounted_c        (0xD2)
#define  gUsbErr_ControllerNotAvailable_c     (0xD3)

/*****************************************************************************
******************************************************************************
* Public type definitions
******************************************************************************
*****************************************************************************/

/* Create a separate type for the status returned by the USB services */
typedef uint8_t  usbError_t;

/* USB PLM component is able to deal with usb packets no larger than 64K */
typedef uint16_t usbPacketSize_t; 

/* USB specific class types known by this implementation*/
typedef enum
{
  gUsbClassId_Cdc_c = 0x2,
  gUsbClassId_Hid_c = 0x3,
  gUsbClassId_Hub_c = 0x9
}usbClassId_t;

/* List of the USB events that the application can be notified about through
the UsbToAppEventHandler callback */
typedef enum
{
  gUsbToAppEvent_BusReset_c = 0x00,
  gUsbToAppEvent_ConfigChanged_c,
  gUsbToAppEvent_EnumComplete_c, 
  gUsbToAppEvent_SendComplete_c, 
  gUsbToAppEvent_DataReceived_c,
  gUsbToAppEvent_Error_c,
  gUsbToAppEvent_GetDataBuff_c,
  gUsbToAppEvent_EpStalled_c,
  gUsbToAppEvent_EpUnstalled_c,
  gUsbToAppEvent_GetTransferSize_c,
  gUsbToAppEvent_RequestPending_c,
  gUsbToAppEvent_ShortSendComplete_c,
  gUsbToAppEvent_Suspend_c,
  gUsbToAppEvent_Resume_c
}gUsbToAppEvent_t;


/* Structure of the 8 byte usb setup packet */
typedef struct 
{
  uint8_t      requestType;     /* Bmrequest type      */
  uint8_t      request;         /* Request code        */
  uint16_t     value;           /* Wvalue              */
  uint16_t     index;           /* Windex              */
  uint16_t     length;          /* Length of the data  */
}usbSetupPacket_t;
  
  
  
/* USB Endpoint Types */
typedef enum
{
  gUsbEpType_Control_c      = 0x00,
  gUsbEpType_Isochronous_c  = 0x01,
  gUsbEpType_Bulk_c         = 0x02,
  gUsbEpType_Interrupt_c    = 0x03
}usbEpType_t;


/* USB Endpoint Directions */
typedef enum
{
  gUsbEpDirection_Out_c = 0x00,
  gUsbEpDirection_In_c = 0x01
}usbEpDirection_t;

 

typedef struct _usbEpStruct_t
{
  uint8_t          number;      /* endpoint number         */
  uint8_t          type;        /* type of endpoint        */
  uint8_t          direction;   /* direction of endpoint   */
  usbPacketSize_t size;        /* buffer size of endpoint */
}usbEpStruct_t;

  
/* Define a pointer to the handler in the application that will process the events from USB component */
typedef void(* pfUsbToAppEventHandler_t)(uint8_t controllerId, gUsbToAppEvent_t usbToAppEvent, void* pValue);

/* callback function pointer structure to handle USB framework request */
typedef usbError_t(* pfusbReqHandler_t)
(
  uint8_t controllerId,
  usbSetupPacket_t* pUsbSetupPacket,
  uint8_t** ppData,
  usbPacketSize_t* pDataLength
);                                              

/* Define a pointer to the handler in the application that will process the class specific requests
received from USB component */
typedef usbError_t (* pfClassSpecificReqHandler_t)
(
  uint8_t controllerId,
  uint8_t request,
  uint16_t value,
  uint8_t** ppData,
  usbPacketSize_t* pDataLength
);

/* Define a pointer to a function that enables the application layer
to provide the requested descriptor in interrupt context. */                           
typedef uint8_t (*pf_USB_Desc_GetDescriptor_t)
(
  uint8_t controllerId,
  uint8_t type,
  uint8_t strNum,
  uint16_t index,
  uint8_t* *pDescriptor,
  usbPacketSize_t *pSize
);

/*Define a pointer to a function that enables the application layer
 to provide the current alternate settings of an interface in interrupt context. */     
typedef uint8_t (*pf_USB_Desc_GetInterface_t)
(
  uint8_t controllerId,
  uint8_t interface,
  uint8_t* altInterface
);

/*Define a pointer to a function that enables the application layer 
to be noticed in interrupt context when a new alternate setting 
has been set for one of its interfaces.*/
typedef uint8_t (*pf_USB_Desc_SetInterface_t)
(
  uint8_t controllerId,
  uint8_t interface,
  uint8_t altInterface
);
    
/*Define a pointer to a function that is called in interrupt context
 to check whether a configuration value is valid for the respective device or not.*/
typedef bool_t (*pf_USB_Desc_ValidConfiguration_t)
(
  uint8_t controllerId,
  uint16_t configVal
);

typedef bool_t (*pf_USB_Desc_ValidInterface_t)
(
  uint8_t controllerId,
  uint8_t interface
);

/* Define a pointer to a function that is called to check whether
 a device has remote wakeup capabilities or not.*/
typedef bool_t (*pf_USB_Desc_RemoteWakeup_t)(uint8_t controllerId);    





/* Structure describing all the details of a USB Class instance */
typedef  struct 
{
  uint8_t                           classId;
  pfUsbToAppEventHandler_t          pfUsbToAppEventHandler;
  pfusbReqHandler_t                 pfVendorReqHandler;
  pfClassSpecificReqHandler_t       pfClassSpecificReqHandler;
  pf_USB_Desc_GetDescriptor_t       pfUSB_Desc_GetDescriptor;
  pf_USB_Desc_GetInterface_t        pf_USB_Desc_GetInterface;
  pf_USB_Desc_SetInterface_t        pf_USB_Desc_SetInterface;
  pf_USB_Desc_ValidConfiguration_t  pf_USB_Desc_ValidConfiguration;
  pf_USB_Desc_ValidInterface_t      pf_USB_Desc_ValidInterface;
  pf_USB_Desc_RemoteWakeup_t        pf_USB_Desc_RemoteWakeup;
  uint8_t                           controlEpSize;
  uint8_t                           numNonControlEp;
  usbEpStruct_t*                    usbNonControlEpDescriptor;
}usbClassDescriptor_t;
  

/*****************************************************************************
******************************************************************************
* Public memory declaration
******************************************************************************
*****************************************************************************/

/*****************************************************************************
******************************************************************************
* Public prototypes
******************************************************************************
*****************************************************************************/
#if gUsbIncluded_d

/*This function must be called prior to any other calls of the USB Platform
 Component functions. It creates the USB task and initializes the message queue
 used to transport USB messages from USB ISR to USB task.*/
 extern void USB_Init(void);
 
 /*This function mounts the device characterized by the pUsbClassDescriptor 
 initialization structure on the controllerId controller number.*/
 extern usbError_t USB_AddController(const usbClassDescriptor_t* pUsbClassDescriptor, uint8_t controllerId);
 
 /*This function detaches the device mounted on the controllerId controller number.*/
 extern usbError_t USB_RemoveController(uint8_t controllerId);
 
 /* This function returns the first free controller number that can be used to mount a new device */
 extern uint8_t USB_GetFirstAvailableController(void);
 
 /* This function returns TRUE if a device is already mounted on controllerId
  controller number and FALSE otherwise*/
 extern bool_t USB_IsDeviceConnected(uint8_t controllerId);
#else
 #define USB_Init() 
 #define USB_AddController(pUsbClassDescriptor, controllerId) gUsbErr_NoError_c
 #define USB_RemoveController(controllerId)                   gUsbErr_NoError_c
 #define USB_GetFirstAvailableController()                           0
 #define USB_IsDeviceConnected(controllerId)                       FALSE 
#endif

extern void USB_ISR(void);


#endif /* _USB_INTERFACE_H */