/******************************************************************************
 * This is a header file for USB Descriptors required for Virtual COM Application
 *        
 * Freescale Semiconductor Inc.
 * (c) Copyright 2012 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************/
 
#ifndef _USB_CDC_DESCRIPTOR_H
#define _USB_CDC_DESCRIPTOR_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "EmbeddedTypes.h"
#include "usb_class.h"
#include "usb_descriptor.h"
#include "CDC_Interface.h" 
/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/
#define gCDCRemoteWakeupSupport_d        (TRUE)


/* Communication Class SubClass Codes */
#define gUSB_CDC_SubclassCodes_DirectLineControlModel_d                (0x01)
#define gUSB_CDC_SubclassCodes_AbstractControlModel_d                  (0x02)
#define gUSB_CDC_SubclassCodes_TelephoneControlModel_d                 (0x03)
#define gUSB_CDC_SubclassCodes_MultiChannelControlModel_d              (0x04)
#define gUSB_CDC_SubclassCodes_CAPIControlModel_d                      (0x05)
#define gUSB_CDC_SubclassCodes_EthernetNetworkingControlModel_d        (0x06)
#define gUSB_CDC_SubclassCodes_ATMNetworkingControlModel_d             (0x07)
#define gUSB_CDC_SubclassCodes_WirelessHandsetControlModel_d           (0x08)
#define gUSB_CDC_SubclassCodes_DeviceManagement_d                      (0x09)
#define gUSB_CDC_SubclassCodes_MobileDirectLineModel_d                 (0x0A)
#define gUSB_CDC_SubclassCodes_OBEX_d                                  (0x0B)
#define gUSB_CDC_SubclassCodes_EthernetEmulationModel_d                (0x0C)

/* Communication Class Protocol Codes */
#define gUSB_CDC_CIC_ProtocolCodes_NoClassSpecificProtocol_d           (0x00)
#define gUSB_CDC_CIC_ProtocolCodes_AT250Protocol_d                     (0x01)
#define gUSB_CDC_CIC_ProtocolCodes_AT_PCCA101Protocol_d                (0x02)
#define gUSB_CDC_CIC_ProtocolCodes_AT_PCCA101AnnexOProtocol_d          (0x03)
#define gUSB_CDC_CIC_ProtocolCodes_AT_GSM7_07Protocol_d                (0x04)
#define gUSB_CDC_CIC_ProtocolCodes_AT_3GPP27_007Protocol_d             (0x05)
#define gUSB_CDC_CIC_ProtocolCodes_AT_TIA_CDMAProtocol_d               (0x06)
#define gUSB_CDC_CIC_ProtocolCodes_EthernetEmulationProtocol_d         (0x07)
#define gUSB_CDC_CIC_ProtocolCodes_ExternalProtocol_d                  (0xFE)
#define gUSB_CDC_CIC_ProtocolCodes_VendorSpecificProtocol_d            (0xFF)

/* Data Class Protocol Codes */
#define gUSB_CDC_DIC_ProtocolCodes_NoClassSpecificProtocol_d           (0x00) 
#define gUSB_CDC_DIC_ProtocolCodes_PhysicalInterfaceProtocol_d         (0x30) 
#define gUSB_CDC_DIC_ProtocolCodes_HDLCProtocol_d                      (0x31)  
#define gUSB_CDC_DIC_ProtocolCodes_TransparentProtocol_d               (0x32)  
#define gUSB_CDC_DIC_ProtocolCodes_ManagementProtocol_d                (0x50)   
#define gUSB_CDC_DIC_ProtocolCodes_DataLinkQ931Protocol_d              (0x51)
#define gUSB_CDC_DIC_ProtocolCodes_DataLinkQ921Protocol_d              (0x52) 
#define gUSB_CDC_DIC_ProtocolCodes_DataCompressionV42BisProtocol_d     (0x90)
#define gUSB_CDC_DIC_ProtocolCodes_EuroISDNProtocol_d                  (0x91)
#define gUSB_CDC_DIC_ProtocolCodes_RateAdaptationISDNV24Protocol_d     (0x92)
#define gUSB_CDC_DIC_ProtocolCodes_CAPICommandsProtocol_d              (0x93)
#define gUSB_CDC_DIC_ProtocolCodes_HostBasedDriverProtocol_d           (0xFD)
#define gUSB_CDC_DIC_ProtocolCodes_CDCUnitFunctionalProtocol_d         (0xFE)
#define gUSB_CDC_DIC_ProtocolCodes_VendorSpecificProtocol_d            (0xFF)



/* Descriptor SubType in Communications Class Functional Descriptors */
#define gUSB_CDC_DescType_HeaderFuncDesc_d              (0x00)
#define gUSB_CDC_DescType_CallManagementFuncDesc_d      (0x01)
#define gUSB_CDC_DescType_AbstractControlFuncDesc_d     (0x02)
#define gUSB_CDC_DescType_DirectLineFuncDesc_d          (0x03)
#define gUSB_CDC_DescType_TelephoneRingerFuncDesc_d     (0x04)
#define gUSB_CDC_DescType_TelephoneReportFuncDesc_d     (0x05)
#define gUSB_CDC_DescType_UnionFuncDesc_d               (0x06)
#define gUSB_CDC_DescType_CountrySelectFuncDesc_d       (0x07)
#define gUSB_CDC_DescType_TelephoneModesFuncDesc_d      (0x08)
#define gUSB_CDC_DescType_USBTerminalFuncDesc_d         (0x09)
#define gUSB_CDC_DescType_NetworkChannelFuncDesc_d      (0x0A)
#define gUSB_CDC_DescType_ProtocolUnitFuncDesc_d        (0x0B)
#define gUSB_CDC_DescType_ExtensionUnitFuncDesc_d       (0x0C)
#define gUSB_CDC_DescType_MultiChannelFuncDesc_d        (0x0D)
#define gUSB_CDC_DescType_CAPIControlFuncDesc_d         (0x0E)
#define gUSB_CDC_DescType_EthernetNetworkingFuncDesc_d  (0x0F)
#define gUSB_CDC_DescType_ATMNetworkingFuncDesc_d       (0x10)
#define gUSB_CDC_DescType_WirelessControlFuncDesc_d     (0x11)
#define gUSB_CDC_DescType_MobileDirectLineFuncDesc_d    (0x12)
#define gUSB_CDC_DescType_MDLMDetailFuncDesc_d          (0x13)
#define gUSB_CDC_DescType_DeviceManagementFuncDesc_d    (0x14)
#define gUSB_CDC_DescType_OBEXFuncDesc_d                (0x15)
#define gUSB_CDC_DescType_CommandSetFuncDesc_d          (0x16)
#define gUSB_CDC_DescType_CommandSetDetailFuncDesc_d    (0x17)
#define gUSB_CDC_DescType_TelephoneControlFuncDesc_d    (0x18)
#define gUSB_CDC_DescType_OBEXServiceIdFuncDesc_d       (0x19)


#define gCDC_CIC_SubclassCode_d          gUSB_CDC_SubclassCodes_AbstractControlModel_d
#define gCDC_CIC_ProtocolCode_d          gUSB_CDC_CIC_ProtocolCodes_NoClassSpecificProtocol_d
#define gCDC_DIC_ProtocolCode_d          gUSB_CDC_DIC_ProtocolCodes_NoClassSpecificProtocol_d
#define gCDC_CIC_EndpCount_d                   (1)
#define gCDC_DIC_EndpCount_d                   (2)


#define gCDC_CIC_NotifyEndpoint_d              (3)
#define gCDC_DIC_BulkInEndpoint_d              (1)
#define gCDC_DIC_BulkOutEndpoint_d             (2)


#define gCDC_RemoteWakeupShift_d               (5)

#define gCDC_ConfigDescSize_d                  (gUSB_DescSize_ConfigOnlyDescriptorSize_d + 28 + 7 +  23)


/* Max descriptors provided by the Application */
#define gCDC_Num_StdDescriptorsNum_d           (7)

/* Max configuration supported by the Application */
#define gCDC_Num_SupportedConfigNum_d          (1)

/* Max string descriptors supported by the Application */
#if gVirtualCOMPortSerialNoEnable_d 
#define gCDC_Num_StringDescriptorsNum_d        (4)
#else
#define gCDC_Num_StringDescriptorsNum_d        (3)
#endif
/* Max language codes supported by the USB */
#define gCDC_Num_SupportedLanguagesNum_d       (1)

/* string descriptors sizes */

#define gCDC_Str0DescSize_d                    (2)
#define gCDC_Str1DescSize_d                    (56)
//#define gCDC_Str2DescSize_d                    (28)
#define gCDC_Str2DescSize_d                    (44)
#define gCDC_Str3DescSize_d                    (16)
#define gCDC_StrNDescSize_d                    (32)

/* descriptors codes */
#define gCDC_DescType_CS_Interface_d           (0x24)
#define gCDC_DescType_CS_Endpoint_d            (0x25)

#define gCDC_Num_MaxSupportedDataInterfaces_d     (1)

/* Implementation Specific Macros */
#define gCDC_LineCodingSize_d                  (0x07)

#define gCDC_LineCodeDTERateIface0_d           (115200) /*e.g 9600 is 0x00002580 */
#define gCDC_LineCodeCharFormatIface0_d        (0x00)   /* 1 stop bit */
#define gCDC_LineCodeParityTypeIface0_d        (0x00)   /* No Parity */
#define gCDC_LineCodeDataBitsIface0_d          (0x08)   /* Data Bits Format */


#define gCDC_StatusAbstractStateIface0_d       (0x0000) /* Disable Multiplexing
                                                  ENDP in this interface will
                                                 continue to accept/offer data*/
#define gCDC_CountrySettingIface0_d            (0x0000) /* Country Code in the format as
                                                  defined in [ISO3166] */

/******************************************************************************
 * Types
 *****************************************************************************/
typedef const struct usbCDCAllLanguages_tag
{
  /* Pointer to Supported Language String */
  uint8_t const *pLanguagesSupportedString;
  /* Size of Supported Language String */
  uint8_t const languagesSupportedSize;
  /* Array of Supported Languages */
  usbLanguage_t usbLanguage[gCDC_Num_MaxSupportedDataInterfaces_d];
}usbCDCAllLanguages_t;


#if gVirtualCOMPort_d
/******************************************************************************
 * Global Variables
 *****************************************************************************/

extern  const  usbEpStruct_t usbCdcDescEp[gNumUsbCdcNonControlEp_d];
/******************************************************************************
 * Global Functions
 *****************************************************************************/
 
/*****************************************************************************
 * This function is used to pass the pointer to the requested descriptor
 *****************************************************************************/ 
extern uint8_t USB_CDC_Desc_GetDescriptor(
    uint8_t controllerId,
    uint8_t type,
    uint8_t strNum,
    uint16_t index,
    uint8_t**pDescriptor,
    usbPacketSize_t *pSize);

/*****************************************************************************
 * This function is called by the framework module to get the current alternete interface
 *****************************************************************************/
extern uint8_t USB_CDC_Desc_GetInterface(
    uint8_t controllerId,
    uint8_t interface,
    uint8_t* pAltInterface);

/*****************************************************************************
 * This function is called by the framework module to set the alternate interface
 *****************************************************************************/
extern uint8_t USB_CDC_Desc_SetInterface(
    uint8_t controllerId,
    uint8_t interface,
    uint8_t altInterface);

/*****************************************************************************
 * This function checks whether the configuration is valid or not
 *****************************************************************************/
extern bool_t USB_CDC_Desc_ValidConfiguration(
    uint8_t controllerId,
    uint16_t configVal);

/*****************************************************************************
 * This function checks whether the interface is valid or not
 *****************************************************************************/
extern bool_t USB_CDC_Desc_ValidInterface(
    uint8_t controllerId,
    uint8_t interface);

/*****************************************************************************
 * The function checks whether the remote wakeup is supported or not
 *****************************************************************************/
extern bool_t USB_CDC_Desc_RemoteWakeup(uint8_t controllerId);

/*****************************************************************************
* Returns current Line Coding Parameters
*****************************************************************************/
extern uint8_t USB_CDC_Desc_GetLineCoding(
    uint8_t controllerId,
    uint8_t interface,
    uint8_t* *pCodingData);

/*****************************************************************************
 * Sets Line Coding Structure with the HOST specified values
 *****************************************************************************/
extern uint8_t USB_CDC_Desc_SetLineCoding(
    uint8_t controllerId,
    uint8_t interface,
    uint8_t* *pCodingData);

/*****************************************************************************
 * Returns ABSTRACT STATE Communication Feature to the Host
 *****************************************************************************/
extern uint8_t USB_CDC_Desc_GetAbstractState(
    uint8_t controllerId,
    uint8_t interface,
    uint8_t* *pFeatureData);
    
/*****************************************************************************
 * The function gets the current setting for communication feature (COUNTRY_CODE)
 *****************************************************************************/    
extern uint8_t USB_CDC_Desc_GetCountrySetting(
    uint8_t controllerId,
    uint8_t interface,
    uint8_t* *pFeatureData);
    
/*****************************************************************************
 * The function sets the current setting for communication feature (ABSTRACT_STATE)
 * specified by the Host
 *****************************************************************************/
extern uint8_t USB_CDC_Desc_SetAbstractState(
    uint8_t controllerId,
    uint8_t interface,
    uint8_t* *pFeatureData);
    
/*****************************************************************************
 *The function sets the current setting for communication feature (COUNTRY_CODE)
 * specified by the HOST
 *****************************************************************************/
extern uint8_t USB_CDC_Desc_SetCountrySetting(
    uint8_t controllerId,
    uint8_t interface,
    uint8_t* *pFeatureData);
#else//#if gVirtualCOMPort_d

#define   USB_CDC_Desc_GetLineCoding(controllerId, interface, pCodingData)        ((void)controllerId, (void*)pCodingData, gUsbErr_InvalidReqType_c)
#define   USB_CDC_Desc_SetLineCoding(controllerId, interface, pCodingData)        ((void)controllerId, gUsbErr_InvalidReqType_c)
#define   USB_CDC_Desc_GetAbstractState(controllerId, interface, pFeatureData)    ((void)controllerId, (void*)pFeatureData, gUsbErr_InvalidReqType_c)
#define   USB_CDC_Desc_GetCountrySetting(controllerId, interface, pFeatureData)   ((void)controllerId, (void*)pFeatureData, gUsbErr_InvalidReqType_c)
#define   USB_CDC_Desc_SetAbstractState(controllerId, interface, pFeatureData)    ((void)controllerId, (void*)pFeatureData, gUsbErr_InvalidReqType_c)
#define   USB_CDC_Desc_SetCountrySetting(controllerId, interface, pFeatureData)   ((void)controllerId, (void*)pFeatureData, gUsbErr_InvalidReqType_c)



#endif//#if gVirtualCOMPort_d


#endif//#ifndef _USB_CDC_DESCRIPTOR_H

