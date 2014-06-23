/******************************************************************************
 * This is the generic descriptor file
 * Freescale Semiconductor Inc.
 * (c) Copyright 2012 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "EmbeddedTypes.h"
#include "usb_class.h"
#include "usb_descriptor.h"
#include "USB.h"
#include "USB_Configuration.h"

 #if gUsbIncluded_d
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

/*****************************************************************************
 * Local Variables - None
 *****************************************************************************/

 /*****************************************************************************
 * Local Functions - None
 *****************************************************************************/

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

/*****************************************************************************/
uint8_t USB_Desc_GetDescriptor(
     uint8_t controllerId,   /* [IN]  Controller ID */
     uint8_t type,            /* [IN]  Type of descriptor requested */
     uint8_t strNum,         /* [IN]  String index for string descriptor */
     uint16_t index,          /* [IN]  String descriptor language Id */
     uint8_t* *pDescriptor, /* [OUT] Output descriptor pointer */
     usbPacketSize_t *pSize   /* [OUT] Size of descriptor returned */
)
{
    uint8_t status = gUsbErr_InvalidReqType_c;
    pf_USB_Desc_GetDescriptor_t pfGD;
    pfGD = gpaUsbClassDescriptor[controllerId]->pfUSB_Desc_GetDescriptor;
    if(pfGD != NULL)
    {
       status = (*pfGD)(controllerId,type,strNum,index,pDescriptor,pSize);  
    }
     return status; 
    
}

/*****************************************************************************/
uint8_t USB_Desc_GetInterface
(
  uint8_t controllerId,     /* [IN] Controller ID */
  uint8_t interface,         /* [IN] Interface number */
  uint8_t* altInterface  /* [OUT] Output alternate interface */
)
{
  uint8_t status = gUsbErr_InvalidReqType_c;
  pf_USB_Desc_GetInterface_t pfGI;
  pfGI = gpaUsbClassDescriptor[controllerId]->pf_USB_Desc_GetInterface;
  if(pfGI != NULL)
  {
    status = (*pfGI)(controllerId,interface,altInterface);
  }
  return status;
}

/*****************************************************************************/
uint8_t USB_Desc_SetInterface
(
  uint8_t controllerId, /* [IN] Controller ID */
  uint8_t interface,     /* [IN] Interface number */
  uint8_t altInterface  /* [IN] Input alternate interface */
)
{
  uint8_t status = gUsbErr_InvalidReqType_c;
  pf_USB_Desc_SetInterface_t pfSI;
  pfSI = gpaUsbClassDescriptor[controllerId]->pf_USB_Desc_SetInterface;
  if(pfSI != NULL)
  {
    status = (*pfSI)(controllerId,interface,altInterface);
  }
  /* if interface valid */
  return status;
}

/*****************************************************************************/
bool_t USB_Desc_ValidConfiguration
(
  uint8_t controllerId,/*[IN] Controller ID */
  uint16_t configVal   /*[IN] Configuration value */
)
{
    
  bool_t status = FALSE;
  pf_USB_Desc_ValidConfiguration_t pfVC;
  pfVC = gpaUsbClassDescriptor[controllerId]->pf_USB_Desc_ValidConfiguration;
  if(pfVC != NULL)
  {
    status = (*pfVC)(controllerId,configVal); 
  }
  return status;  
}

/*****************************************************************************/
bool_t USB_Desc_ValidInterface
(
  uint8_t controllerId, /*[IN] Controller ID */
  uint8_t interface      /*[IN] Target interface */
)
{
  bool_t status = FALSE;
  pf_USB_Desc_ValidInterface_t pfVI;
  pfVI = gpaUsbClassDescriptor[controllerId]->pf_USB_Desc_ValidInterface;
  if(pfVI != NULL)
  {
    status = (*pfVI)(controllerId,interface);
  }
  return status;
}

/*****************************************************************************/
bool_t USB_Desc_RemoteWakeup
(
  uint8_t controllerId      /* [IN] Controller ID */
)
{
  bool_t status = FALSE;
  pf_USB_Desc_RemoteWakeup_t  pfRW;
  pfRW = gpaUsbClassDescriptor[controllerId]->pf_USB_Desc_RemoteWakeup;
  if(pfRW != NULL)
  {
    status = (*pfRW)(controllerId);
  }
  return status;
}

#endif
