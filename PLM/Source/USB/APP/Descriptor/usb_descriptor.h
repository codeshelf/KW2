/******************************************************************************
 * This is the generic descriptor file interface
 * Freescale Semiconductor Inc.
 * (c) Copyright 2012 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
****************************************************************************/

#ifndef _USB_DESCRIPTOR_H
#define _USB_DESCRIPTOR_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "EmbeddedTypes.h"
#include "USB_Interface.h"

/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/
#define gUSB_DescSize_DeviceDescriptorSize_d            (18)
#define gUSB_DescSize_StringDescriptorSize_d            (2)
#define gUSB_DescSize_DeviceQualifierDescriptorSize_d   (10)
#define gUSB_DescSize_ConfigOnlyDescriptorSize_d        (9)
#define gUSB_DescSize_InterfaceOnlyDescriptorSize_d     (9)
#define gUSB_DescSize_EndpOnlyDescriptorSize_d          (7)
 
#define gUSB_DescType_DeviceDescriptor_d                (1)
#define gUSB_DescType_ConfigDescriptor_d                (2)
#define gUSB_DescType_StringDescriptor_d                (3)
#define gUSB_DescType_InterfaceDescriptor_d             (4)
#define gUSB_DescType_EndpointDescriptor_d              (5)
#define gUSB_DescType_DevQualDescriptor_d               (6)

#define gUSB_RemoteWakeupShift_d                        (5)
 
/******************************************************************************
 * Types
 *****************************************************************************/
 
typedef const struct usbLanguage_tag
{
  uint16_t const languageId;      /* Language ID */
  uint8_t const ** pLangDesc;      /* Language Descriptor String */
  uint8_t const * pLangDescSize;  /* Language Descriptor Size */
} usbLanguage_t; 



/******************************************************************************
 * Global variables
 *****************************************************************************/


/******************************************************************************
 * Global Functions
 *****************************************************************************/
 
/*****************************************************************************
 * This function is used to pass the pointer to the requested descriptor
 *****************************************************************************/ 
extern uint8_t USB_Desc_GetDescriptor
(
  uint8_t controllerId,
  uint8_t type,
  uint8_t str_num,
  uint16_t index,
  uint8_t* *descriptor,
  usbPacketSize_t *size
);

/*****************************************************************************
 * This function is called by the framework module to get the current alternete interface
 *****************************************************************************/
extern uint8_t USB_Desc_GetInterface
(
  uint8_t controllerId,
  uint8_t interface,
  uint8_t* altInterface
);

/*****************************************************************************
 * This function is called by the framework module to set the alternate interface
 *****************************************************************************/
extern uint8_t USB_Desc_SetInterface
(
  uint8_t controllerId,
  uint8_t interface,
  uint8_t altInterface
);

/*****************************************************************************
 * This function checks whether the configuration is valid or not
 *****************************************************************************/
extern bool_t USB_Desc_ValidConfiguration
(
  uint8_t controllerId,
  uint16_t configVal
);

/*****************************************************************************
 * This function checks whether the interface is valid or not
 *****************************************************************************/
extern bool_t USB_Desc_ValidInterface
(
uint8_t controllerId,
uint8_t interface
);

/*****************************************************************************
 * The function checks whether the remote wakeup is supported or not
 *****************************************************************************/
extern bool_t USB_Desc_RemoteWakeup(uint8_t controllerId);


#endif
