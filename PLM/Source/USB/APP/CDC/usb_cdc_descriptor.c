/******************************************************************************
 *
 * Virtual Com Application descriptor file
 * Freescale Semiconductor Inc.
 * (c) Copyright 2012 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 **************************************************************************/
 

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "EmbeddedTypes.h"
#include "usb_class.h"
#include "USB.h"
#include "usb_descriptor.h"
#include "usb_cdc_descriptor.h"
#include "usb_dciapi.h"
#include "usb_cdc_pstn.h"
#include "CDC_Interface.h" 

#if gVirtualCOMPort_d
/*****************************************************************************
 * Constant and Macro's
 *****************************************************************************/
/* structure containing details of all the endpoints used by this device */
 const usbEpStruct_t  usbCdcDescEp[gNumUsbCdcNonControlEp_d]=
{
  {
    gCDC_DIC_BulkInEndpoint_d,
    gUsbEpType_Bulk_c,
    gUsbEpDirection_In_c,
    gUsbCdcBulkInEpSize_d,
  },
  {
    gCDC_DIC_BulkOutEndpoint_d,
    gUsbEpType_Bulk_c,
    gUsbEpDirection_Out_c,
    gUsbCdcBulkOutEpSize_d,
  },
  {
    gCDC_CIC_NotifyEndpoint_d,
    gUsbEpType_Interrupt_c,
    gUsbEpDirection_In_c,
    gUsbCdcInterruptInEpSize_d,
  }
};

uint8_t   gCDCDeviceDescriptor[gUSB_DescSize_DeviceDescriptorSize_d] =

{
  gUSB_DescSize_DeviceDescriptorSize_d,               /*  Device Dexcriptor Size         */
  gUSB_DescType_DeviceDescriptor_d,                /*  Device Type of descriptor      */
  0x00, 0x02,                           /*  BCD USB version                */
  0x02,                                 /*  Communications device class    */
  0x00,                                 /*  Device Subclass is indicated
                                            in the interface descriptors   */
  0x00,                                 /*  Device Protocol                */
  gUsbCdcControlEpSize_d,              /*  Max Packet size                */
  0x04,0x25,                            /*  Vendor ID                      */
  0x00,0x03,                            /*  0300 is our Product ID for CDC */
  0x02,0x00,                            /*  BCD Device version             */
  0x01,                                 /*  Manufacturer string index      */
  0x02,                                 /*  Product string index           */
  #if gVirtualCOMPortSerialNoEnable_d
  0x03,
  #else
  0x00,
  #endif                                 /*  Serial number string index     */
  0x01                                  /*  Number of configurations       */
};

uint8_t    gCDCConfigDescriptor[gCDC_ConfigDescSize_d] =
{
  gUSB_DescSize_ConfigOnlyDescriptorSize_d,  /*  Configuration Descriptor Size */
  gUSB_DescType_ConfigDescriptor_d,  /* "Configuration" type of descriptor */
  gCDC_ConfigDescSize_d, 0x00, /*  Total length of the Configuration descriptor */
  (uint8_t)(2),/*NumInterfaces*/
  0x01,                      /*  Configuration Value */
  0x00,                      /*  Configuration Description String Index*/
  gbmUsbAttributes_BusPowered_d|gbmUsbAttributes_SelfPowered_d|(gCDCRemoteWakeupSupport_d<<gCDC_RemoteWakeupShift_d),
              /*  Attributes.support RemoteWakeup and self power*/
  0x32,                   /*  Current draw from bus -- 100mA*/
  /* CIC INTERFACE DESCRIPTOR */
  gUSB_DescSize_InterfaceOnlyDescriptorSize_d,
  gUSB_DescType_InterfaceDescriptor_d,
  0x00, /* bInterfaceNumber */
  0x00, /* bAlternateSetting */
  gCDC_CIC_EndpCount_d, /* management and notification(optional)element present */
  0x02, /* Communication Interface Class */
  gCDC_CIC_SubclassCode_d,
  gCDC_CIC_ProtocolCode_d,
  0x00, /* Interface Description String Index*/

  /* CDC Class-Specific descriptor */
  0x05,             /* size of Functional Desc in bytes */
  gCDC_DescType_CS_Interface_d,  /* descriptor type*/
  gUSB_CDC_DescType_HeaderFuncDesc_d,
  0x10, 0x01,  /* USB Class Definitions for CDC spec release number in BCD */
  0x05,             /* Size of this descriptor */
  gCDC_DescType_CS_Interface_d, /* descriptor type*/
  gUSB_CDC_DescType_CallManagementFuncDesc_d,
  0x01,/*may use 0x03 */  /* device handales call management itself(D0 set)
              and will process commands multiplexed over the data interface */
  0x01,      /* Indicates multiplexed commands are
                handled via data interface */

  0x04,             /* Size of this descriptor */
  gCDC_DescType_CS_Interface_d, /* descriptor type*/
  gUSB_CDC_DescType_AbstractControlFuncDesc_d,
  0x06, /*may use 0x0F */ /* Device Supports all commands for ACM - CDC
                              PSTN SubClass bmCapabilities */

  0x05,             /* size of Functional Desc in bytes */
  gCDC_DescType_CS_Interface_d,  /* descriptor type*/
  gUSB_CDC_DescType_UnionFuncDesc_d,
  0x00,           /* Interface Number of Control */
  0x01,           /* Interface Number of Subordinate (Data Class) Interface */
  gUSB_DescSize_EndpOnlyDescriptorSize_d,
  gUSB_DescType_EndpointDescriptor_d,
  gCDC_CIC_NotifyEndpoint_d|(gUsbEpDirection_In_c << 7),
  gUsbEpType_Interrupt_c,
  gUsbCdcInterruptInEpSize_d, 0x00,
  0x0A,
  gUSB_DescSize_InterfaceOnlyDescriptorSize_d,
  gUSB_DescType_InterfaceDescriptor_d,
  (uint8_t)(1), /* bInterfaceNumber */
  0x00, /* bAlternateSetting */
  gCDC_DIC_EndpCount_d, /* notification element included */
  0x0A, /* DATA Interface Class */
  0x00, /* Data Interface SubClass Code */
  gCDC_DIC_ProtocolCode_d,
  0x00, /* Interface Description String Index*/
  /*Endpoint descriptor */
  gUSB_DescSize_EndpOnlyDescriptorSize_d,
  gUSB_DescType_EndpointDescriptor_d,
  gCDC_DIC_BulkInEndpoint_d|(gUsbEpDirection_In_c << 7),
  gUsbEpType_Bulk_c,
  gUsbCdcBulkInEpSize_d, 0x00,
  0x00,/* This value is ignored for Bulk ENDPOINT */
   /*Endpoint descriptor */
  gUSB_DescSize_EndpOnlyDescriptorSize_d,
  gUSB_DescType_EndpointDescriptor_d,
  gCDC_DIC_BulkOutEndpoint_d|(gUsbEpDirection_Out_c << 7),
  gUsbEpType_Bulk_c,
  gUsbCdcBulkOutEpSize_d, 0x00,
  0x00 /* This value is ignored for Bulk ENDPOINT */
};

uint8_t   gCDC_Str0[gCDC_Str0DescSize_d+gUSB_DescSize_StringDescriptorSize_d] =
{
  sizeof(gCDC_Str0),
  gUSB_DescType_StringDescriptor_d,
  0x09,
  0x04/*equiavlent to 0x0409(English - United States)*/
};

uint8_t   gCDC_Str1[gCDC_Str1DescSize_d+gUSB_DescSize_StringDescriptorSize_d] = 
{
  sizeof(gCDC_Str1),
  gUSB_DescType_StringDescriptor_d,
  'F',0,
  'R',0,
  'E',0,
  'E',0,
  'S',0,
  'C',0,
  'A',0,
  'L',0,
  'E',0,
  ' ',0,
  'S',0,
  'E',0,
  'M',0,
  'I',0,
  'C',0,
  'O',0,
  'N',0,
  'D',0,
  'U',0,
  'C',0,
  'T',0,
  'O',0,
  'R',0,
  ' ',0,
  'I',0,
  'N',0,
  'C',0,
  '.',0
};


uint8_t   gCDC_Str2[gCDC_Str2DescSize_d+gUSB_DescSize_StringDescriptorSize_d] = 
{
  sizeof(gCDC_Str2),
  gUSB_DescType_StringDescriptor_d,
  'F',0,
  'R',0,
  'E',0,
  'E',0,
  'S',0,
  'C',0,
  'A',0,
  'L',0,
  'E',0,
  ' ',0,
  'V',0,
  'I',0,
  'R',0,
  'T',0,
  'U',0,
  'A',0,
  'L',0,
  ' ',0,
  'C',0,
  'O',0,
  'M',0,
  ' ',0
};
     
#if gVirtualCOMPortSerialNoEnable_d                          
uint8_t   gCDC_Str3[gCDC_Str3DescSize_d+gUSB_DescSize_StringDescriptorSize_d] = 
{
  sizeof(gCDC_Str3),
  gUSB_DescType_StringDescriptor_d,
  gDefaultValueOfVirtualCOMPortSerialNo_c
};
#endif

uint8_t   gCDC_Strn[gCDC_StrNDescSize_d+gUSB_DescSize_StringDescriptorSize_d] =
{
  sizeof(gCDC_Strn),
  gUSB_DescType_StringDescriptor_d,
  'B',0,
  'A',0,
  'D',0,
  ' ',0,
  'S',0,
  'T',0,
  'R',0,
  'I',0,
  'N',0,
  'G',0,
  ' ',0,
  'I',0,
  'N',0,
  'D',0,
  'E',0,
  'X',0
};


usbPacketSize_t const gCDC_StdDescSize[gCDC_Num_StdDescriptorsNum_d+1] =
{
  0,
  gUSB_DescSize_DeviceDescriptorSize_d,
  gCDC_ConfigDescSize_d,
  0, /* string */
  0, /* Interface */
  0, /* Endpoint */
  0, /* Device Qualifier */
  0 /* other speed config */
};

uint8_t* const gCDC_StdDescriptors[gCDC_Num_StdDescriptorsNum_d+1] =
{
  NULL,
  (uint8_t*)gCDCDeviceDescriptor,
  (uint8_t*)gCDCConfigDescriptor,
  NULL, /* string */
  NULL, /* Interface */
  NULL, /* Endpoint */
  NULL, /* Device Qualifier */
  NULL  /* other speed config*/
};

uint8_t const gCDC_StringDescSize[gCDC_Num_StringDescriptorsNum_d+1] =
{
  sizeof(gCDC_Str0),
  sizeof(gCDC_Str1),
  sizeof(gCDC_Str2),
#if gVirtualCOMPortSerialNoEnable_d  
  sizeof(gCDC_Str3),
#endif  
  sizeof(gCDC_Strn)
};

uint8_t* const gCDC_StringDescriptors[gCDC_Num_StringDescriptorsNum_d+1] =
{
  (uint8_t*)gCDC_Str0,
  (uint8_t*)gCDC_Str1,
  (uint8_t*)gCDC_Str2,
#if gVirtualCOMPortSerialNoEnable_d  
  (uint8_t*)gCDC_Str3,
#endif  
  (uint8_t*)gCDC_Strn
};

usbCDCAllLanguages_t gCDC_Languages = 
{ 
gCDC_Str0, sizeof(gCDC_Str0),
  { 
    {
      (uint16_t )0x0409,
      (const uint8_t **)gCDC_StringDescriptors,
      gCDC_StringDescSize
    }
  }
};

uint8_t const gCDC_ValidConfigValues[gCDC_Num_SupportedConfigNum_d+1]={0,1};

/****************************************************************************
 * Global Variables
 ****************************************************************************/


static uint8_t gLineCoding[gCDC_Num_MaxSupportedDataInterfaces_d][gCDC_LineCodingSize_d] =
{
  {
    (gCDC_LineCodeDTERateIface0_d>> 0) & 0x000000FF,
    (gCDC_LineCodeDTERateIface0_d>> 8) & 0x000000FF,
    (gCDC_LineCodeDTERateIface0_d>>16) & 0x000000FF,
    (gCDC_LineCodeDTERateIface0_d>>24) & 0x000000FF,
    /*e.g. 0x00,0xC2,0x01,0x00 : 0x0001C200 is 115200 bits per second */
    gCDC_LineCodeCharFormatIface0_d,
    gCDC_LineCodeParityTypeIface0_d,
    gCDC_LineCodeDataBitsIface0_d
  }
};

static uint8_t gAbstractState[gCDC_Num_MaxSupportedDataInterfaces_d][gUSB_CDC_CommFeatureDataSize_d] =
{
  { 
    (gCDC_StatusAbstractStateIface0_d>>0) & 0x00FF,
    (gCDC_StatusAbstractStateIface0_d>>8) & 0x00FF
  }
};

static uint8_t gCountryCode[gCDC_Num_MaxSupportedDataInterfaces_d][gUSB_CDC_CommFeatureDataSize_d] =
{
  { 
    (gCDC_CountrySettingIface0_d>>0) & 0x00FF,
    (gCDC_CountrySettingIface0_d>>8) & 0x00FF
  }
};

static uint8_t gAlternateInterface[gCDC_Num_MaxSupportedDataInterfaces_d + 1];

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

uint8_t USB_CDC_Desc_GetDescriptor 
(
  uint8_t controllerId,   /* [IN] Controller ID */
  uint8_t type,            /* [IN] type of descriptor requested */
  uint8_t strNum,         /* [IN] string index for string descriptor */
  uint16_t index,          /* [IN] string descriptor language Id */
  uint8_t* *pDescriptor, /* [OUT] output descriptor pointer */
  usbPacketSize_t *pSize   /* [OUT] size of descriptor returned */
)
{
  (void) (controllerId);
   /* string descriptors are handled saperately */
  if (type == gUSB_DescType_StringDescriptor_d)
  {
    if(index == 0)
    {
      /* return the string and size of all languages */
      *pDescriptor = (uint8_t*)gCDC_Languages.pLanguagesSupportedString;
      *pSize = gCDC_Languages.languagesSupportedSize;
    }
    else
    {
      uint8_t langId ;
      uint8_t langIndex = gCDC_Num_SupportedLanguagesNum_d;
      for(langId = 0;langId< gCDC_Num_SupportedLanguagesNum_d;langId++)
        {
          /* check whether we have a string for this language */
          if(index == gCDC_Languages.usbLanguage[langId].languageId)
            {   /* check for max descriptors */
              if(strNum < gCDC_Num_StringDescriptorsNum_d)
              {   /* setup index for the string to be returned */
                langIndex=strNum;
              }

              break;
            }

        }

        /* set return val for descriptor and size */
        *pDescriptor = (uint8_t*)gCDC_Languages.usbLanguage[langId].pLangDesc[langIndex];
        *pSize = gCDC_Languages.usbLanguage[langId].pLangDescSize[langIndex];
    }

  }
  else if (type < gCDC_Num_StdDescriptorsNum_d+1)
  {
    /* set return val for descriptor and size*/
    *pDescriptor = (uint8_t*)gCDC_StdDescriptors [type];
    /* if there is no descriptor then return error */
    if(*pDescriptor == NULL)
    {
      return gUsbErr_InvalidReqType_c;
    }

    *pSize = gCDC_StdDescSize[type];
  }
  else /* invalid descriptor */
  {
    return gUsbErr_InvalidReqType_c;
  }

  return gUsbErr_NoError_c;
}

/*****************************************************************************/
uint8_t USB_CDC_Desc_GetInterface 
(
  uint8_t controllerId,     /* [IN] Controller ID */
  uint8_t interface,         /* [IN] interface number */
  uint8_t* pAltInterface  /* [OUT] output alternate interface */
)
{
  (void) (controllerId);
  /* if interface valid */
  if(interface < gCDC_Num_MaxSupportedDataInterfaces_d + 1)
  {
    /* get alternate interface*/
    *pAltInterface = gAlternateInterface[interface];
    return gUsbErr_NoError_c;
  }
  return gUsbErr_InvalidReqType_c;
}

/*****************************************************************************/
uint8_t USB_CDC_Desc_SetInterface
(
  uint8_t controllerId, /* [IN] Controller ID */
  uint8_t interface,     /* [IN] interface number */
  uint8_t altInterface  /* [IN] input alternate interface */
)
{
  (void) (controllerId);
  /* if interface valid */
  if(interface < gCDC_Num_MaxSupportedDataInterfaces_d + 1)
  {
    /* set alternate interface*/
    gAlternateInterface[interface]=altInterface;
    return gUsbErr_NoError_c;
  }
    return gUsbErr_InvalidReqType_c;
}

/*****************************************************************************/
bool_t USB_CDC_Desc_ValidConfiguration 
(
  uint8_t controllerId,/*[IN] Controller ID */
  uint16_t configVal   /*[IN] configuration value */
)
{
  uint8_t loopIndex=0;
  (void) (controllerId);
  /* check with only supported val right now */
  while(loopIndex < (gCDC_Num_SupportedConfigNum_d+1))
  {
    if(configVal == gCDC_ValidConfigValues[loopIndex])
    {
      return TRUE;
    }
    loopIndex++;
  }
  return FALSE;
}

/*****************************************************************************/
bool_t USB_CDC_Desc_ValidInterface 
(
  uint8_t controllerId, /*[IN] Controller ID */
  uint8_t interface      /*[IN] target interface */
)
{
  uint8_t loopIndex=0;
  (void) (controllerId);
  /* check with only supported val right now */
  while(loopIndex < gCDC_Num_MaxSupportedDataInterfaces_d)
  {
    if(interface == gAlternateInterface[loopIndex])
    {
      return TRUE;
    }
    loopIndex++;
  }

  return FALSE;
}

/*****************************************************************************/
bool_t USB_CDC_Desc_RemoteWakeup 
(
  uint8_t controllerId  /* [IN] Controller ID */
)
{
  (void) (controllerId);
  return gCDCRemoteWakeupSupport_d;
}


/*****************************************************************************/
uint8_t USB_CDC_Desc_GetLineCoding 
(
  uint8_t controllerId,       /* [IN] Controller ID */
  uint8_t interface,           /* [IN] Interface Number */
  uint8_t* *pCodingData     /* [OUT] Line Coding Data */
)
{
  (void) (controllerId);
  /* if interface valid */
  if(interface < gCDC_Num_MaxSupportedDataInterfaces_d)
  {
    /* get line coding data*/
    *pCodingData = gLineCoding[interface];
    return gUsbErr_NoError_c;
  }
  return gUsbErr_InvalidReqType_c;
}

/*****************************************************************************/
uint8_t USB_CDC_Desc_SetLineCoding
(
  uint8_t controllerId,       /* [IN] Controller ID */
  uint8_t interface,           /* [IN] Interface Number */
  uint8_t* *pCodingData     /* [IN] Line Coding Data */
)
{
  uint8_t count;
  (void)(controllerId);
   /* if interface valid */
  if(interface < gCDC_Num_MaxSupportedDataInterfaces_d)
  {
  /* set line coding data*/
    for (count = 0; count < gCDC_LineCodingSize_d; count++)
    {
      gLineCoding[interface][count] = *(*pCodingData + count);
    }
    return gUsbErr_NoError_c;
  }
  return gUsbErr_InvalidReqType_c;
}

/*****************************************************************************/
uint8_t USB_CDC_Desc_GetAbstractState 
(
  uint8_t controllerId,       /* [IN] Controller ID */
  uint8_t interface,           /* [IN] Interface Number */
  uint8_t* *pFeatureData    /* [OUT] Output Comm Feature Data */
)
{
  (void)(controllerId);
  /* if interface valid */
  if(interface < gCDC_Num_MaxSupportedDataInterfaces_d)
  {
    /* get line coding data*/
    *pFeatureData = gAbstractState[interface];
    return gUsbErr_NoError_c;
  }
  return gUsbErr_InvalidReqType_c;
}


/*****************************************************************************/
uint8_t USB_CDC_Desc_GetCountrySetting
(
  uint8_t controllerId,       /* [IN] Controller ID */
  uint8_t interface,           /* [IN] Interface Number */
  uint8_t* *pFeatureData    /* [OUT] Output Comm Feature Data */
)
{
  (void)(controllerId);
  /* if interface valid */
  if(interface < gCDC_Num_MaxSupportedDataInterfaces_d)
  {
    /* get line coding data*/
    *pFeatureData = gCountryCode[interface];
    return gUsbErr_NoError_c;
  }
  return gUsbErr_InvalidReqType_c;
}

/*****************************************************************************/
uint8_t USB_CDC_Desc_SetAbstractState
(
  uint8_t controllerId,       /* [IN] Controller ID */
  uint8_t interface,           /* [IN] Interface Number */
  uint8_t* *pFeatureData    /* [OUT] Output Comm Feature Data */
)
{
  uint8_t count;
  (void)(controllerId);
  /* if interface valid */
  if(interface < gCDC_Num_MaxSupportedDataInterfaces_d)
  {
    /* set Abstract State Feature*/
    for (count = 0; count < gUSB_CDC_CommFeatureDataSize_d; count++)
    {
      gAbstractState[interface][count] = *(*pFeatureData + count);
    }
    return gUsbErr_NoError_c;
  }
  return gUsbErr_InvalidReqType_c;
}

/*****************************************************************************/
uint8_t USB_CDC_Desc_SetCountrySetting
(
  uint8_t controllerId,       /* [IN] Controller ID */
  uint8_t interface,           /* [IN] Interface Number */
  uint8_t* *pFeatureData    /* [OUT] Output Comm Feature Data */
)
{
  uint8_t count;
  (void)(controllerId);
  /* if interface valid */
  if(interface < gCDC_Num_MaxSupportedDataInterfaces_d)
  {
    for (count = 0; count < gUSB_CDC_CommFeatureDataSize_d; count++)
    {
      gCountryCode[interface][count] = *(*pFeatureData + count);
    }
    return gUsbErr_NoError_c;
  }
  return gUsbErr_InvalidReqType_c;
}



  
#endif//gVirtualCOMPort_d