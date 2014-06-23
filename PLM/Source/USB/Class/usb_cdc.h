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
 * @file usb_cdc.h
 *
 * @author
 *
 * @version
 *
 * @date May-28-2009
 *
 * @brief The file contains USB stack CDC class layer API header function.
 *
 *****************************************************************************/

#ifndef _USB_CDC_H
#define _USB_CDC_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "EmbeddedTypes.h"          /* User Defined Data Types */
#include "usb_cdc_descriptor.h"
#include "usb_class.h"
#include "usb_devapi.h"

/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/
/* Class specific request Codes */
#define gUSB_CDC_ClassRequest_SendEncapsulatedCommand_d       (0x00)
#define gUSB_CDC_ClassRequest_GetEncapsulatedResponse_d       (0x01)
#define gUSB_CDC_ClassRequest_SetCommFeature_d                (0x02)
#define gUSB_CDC_ClassRequest_GetCommFeature_d                (0x03)
#define gUSB_CDC_ClassRequest_ClearCommFeature_d              (0x04)
#define gUSB_CDC_ClassRequest_SetAuxLineState_d               (0x10)
#define gUSB_CDC_ClassRequest_SetHookState_d                  (0x11)
#define gUSB_CDC_ClassRequest_PulseSetup_d                    (0x12)
#define gUSB_CDC_ClassRequest_SendPulse_d                     (0x13)
#define gUSB_CDC_ClassRequest_SetPulseTime_d                  (0x14)
#define gUSB_CDC_ClassRequest_RingAuxJack_d                   (0x15)
#define gUSB_CDC_ClassRequest_SetLineCoding_d                 (0x20)
#define gUSB_CDC_ClassRequest_GetLineCoding_d                 (0x21)
#define gUSB_CDC_ClassRequest_SetControlLineState_d           (0x22)
#define gUSB_CDC_ClassRequest_SendBreak_d                     (0x23)
#define gUSB_CDC_ClassRequest_SetRingerParams_d               (0x30)
#define gUSB_CDC_ClassRequest_GetRingerParams_d               (0x31)
#define gUSB_CDC_ClassRequest_SetOperationParams_d            (0x32)
#define gUSB_CDC_ClassRequest_GetOperationParams_d            (0x33)
#define gUSB_CDC_ClassRequest_SetLineParams_d                 (0x34)
#define gUSB_CDC_ClassRequest_GetLineParams_d                 (0x35)
#define gUSB_CDC_ClassRequest_DialDigits_d                    (0x36)
#define gUSB_CDC_ClassRequest_SetUnitParameter_d              (0x37)
#define gUSB_CDC_ClassRequest_GetUnitParameter_d              (0x38)
#define gUSB_CDC_ClassRequest_ClearUnitParameter_d            (0x39)
#define gUSB_CDC_ClassRequest_GetProfile_d                    (0x3A)
#define gUSB_CDC_ClassRequest_SetEthernetMulticastFilters_d   (0x40)
#define gUSB_CDC_ClassRequest_SetEthernetPowPatterFilter_d    (0x41)
#define gUSB_CDC_ClassRequest_GetEthernetPowPatterFilter_d    (0x42)
#define gUSB_CDC_ClassRequest_SetEthernetPacketFilter_d       (0x43)
#define gUSB_CDC_ClassRequest_GetEthernetStatistic_d          (0x44)
#define gUSB_CDC_ClassRequest_SetATMDataFormat_d              (0x50)
#define gUSB_CDC_ClassRequest_GetATMDeviceStatistics_d        (0x51)
#define gUSB_CDC_ClassRequest_SetATMDefaultVC_d               (0x52)
#define gUSB_CDC_ClassRequest_GetATMVCStatistics_d            (0x53)
#define gUSB_CDC_Mask_MDLMSpecificRequestMask_d               (0x7F)

/* Class Specific Notification Codes */
#define gUSB_CDC_ClassNotifCodes_NetworkConnection_d          (0x00)
#define gUSB_CDC_ClassNotifCodes_ResponseAvail_d              (0x01)
#define gUSB_CDC_ClassNotifCodes_AuxJackHookState_d           (0x08)
#define gUSB_CDC_ClassNotifCodes_RingDetect_d                 (0x09)
#define gUSB_CDC_ClassNotifCodes_SerialState_d                (0x20)
#define gUSB_CDC_ClassNotifCodes_CallStateChange_d            (0x28)
#define gUSB_CDC_ClassNotifCodes_LineStateChange_d            (0x29)
#define gUSB_CDC_ClassNotifCodes_ConnectionSpeedChange_d      (0x2A)
#define gUSB_CDC_Mask_MDLMSpecificNotifMask_d                 (0x5F)

/* Events to the Application */ /* 0 to 4 are reserved for class events */
#define gUsbToAppEvent_CDCCarrierDeactivated_c                (0x21)
#define gUsbToAppEvent_CDCCarrierActivated_c                  (0x22)
#define gUsbToAppEvent_CDC_DTEDeactivated_c                   (0x23)
#define gUsbToAppEvent_CDC_DTEActivated_c                     (0x24)
#define gUsbToAppEvent_CDC_COM_Open_c                         (0x25)
#define gUsbToAppEvent_CDC_COM_Closed_c                       (0x26)


/* other macros */
#define gUSB_CDC_Size_NotifPacketSize_d                       (0x08)
#define gUSB_CDC_NotifRequestType_d                           (0xA1)


#define gUSB_CDC_CIC_NotifElemSupport_d                       (TRUE)
#define gUSB_CDC_SendEventToAppOnCOMOpen_d                    (TRUE) 

/******************************************************************************
 * Types
 *****************************************************************************/
/******************************************************************************
 * Global Functions
 *****************************************************************************/
#if (gUsbCdcEnabled_d)
 extern uint8_t USB_Class_CDC_Init (uint8_t controllerId);
 extern uint8_t USB_Class_CDC_DeInit (uint8_t controllerId); 
 extern void USB_Class_CDC_SetDicInServiceCallback(void (*pf)(usbDeviceEvent_t* event));
 extern void USB_Class_CDC_SetDicOutServiceCallback(void (*pf)(usbDeviceEvent_t* event));
#else  //(gUsbCdcEnabled_d)
 #define USB_Class_CDC_Init(controllerId)   TRUE
 #define USB_Class_CDC_DeInit(controllerId) TRUE 
#endif


#endif