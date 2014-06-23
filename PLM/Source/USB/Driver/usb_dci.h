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
 * @file usb_dci_kinetis.h
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief The file contains Macro's and functions needed by the DCI layer.
 *
 *****************************************************************************/

#ifndef _USB_DCI_H
#define _USB_DCI_H
/******************************************************************************
 * Includes
 *****************************************************************************/
#include"EmbeddedTypes.h"   
#include "PortConfig.h"   
#include "usb_devapi.h"

/******************************************************************************
 * Constants - None
 *****************************************************************************/

/******************************************************************************
 * Macro's
 *****************************************************************************/


#define gUsbEndpt_Out_c                          (0x08)            /* Cfg OUT only pipe for this endpoint*/
#define gUsbEndpt_In_c                           (0x04)            /* Cfg IN only pipe for this endpoint */
#define gUsbEndpt_HshkEn_c                       (0x01)            /* Enable handshake packet            */
                                                                   /* Handshake should be disable for
                                                                    isochorous transfer                */
#define gUsbEndpt_Disable_c                       (0)

/* Macro's to check whether corresponding INT_STAT bit is set */
#define UsbDciIntFlag_BusReset(x)                ((x) & 1)
#define UsbDciIntFlag_Error(x)                   ((x) & 2)
#define UsbDciIntFlag_SofToken(x)                ((x) & 4)
#define UsbDciIntFlag_Sleep(x)                   ((x) & 0x10)
#define UsbDciIntFlag_Resume(x)                  ((x) & 0x20)
#define UsbDciIntFlag_Stall(x)                   ((x) & 0x80)
#define UsbDciIntFlag_TokenComplete(x)           ((x) & 8)

/* control endpoint transfer types */
#define gUsbTrfDirUnknown_d      			           (0xFF)
#define gUsbInvalidBdtIndex_d                    (0xff)/* invalid bdt index */
#define gUsbStat_EndpNumberShift_c               (4)   /* endpoint shift & mask to  */
#define gUsbStat_EndpNumberMask_c                (0xf0)/* use in setting and getting status */
#define gUsbStat_EndpDirShift_c                  (3)   /* direction shift & mask to */
#define gUsbStat_EndpDirMask_c                   (0x08)
#define gUsbToken_Setup_c                        (0x0d)/* Setup Token PID */

#define gUsbDciIntStat_ClearAll_c                (0xbf)/* Value to clear all Interrupts */
#define gUsbDciErrStat_ClearAll_c                (0xbf)/* Value to clear all Errors */
#define gUsbDciErrEnb_EnableAll_c                (0xbf)/* Value to enable all Error Interrupts */
#define gUsbDciIntEnb_IntEnbAtBusReset_c         (0x9f)/* Value to enable Interrupts at Bus Reset */

#define gUsbDci_AssertResumeDelayCount_c         (40000)/* Delay for assert resume */
#define gUsbBdtEvenBuffer_c                      (0)

/******************************************************************************
 * Types
 *****************************************************************************/
#if defined(__CWCC__)
#pragma align_array_members on
#elif defined(__IAR_SYSTEMS_ICC__)   
#pragma pack(1)
#endif

/* This structure is used to hold endpoint paramaetes and the
   transaction parameters on the IO's happening on them */
typedef struct  bdtElem_tag
{
	uint16_t            epSize;             /* endpoint max buffer len */
	uint32_t            addr;            /* endpoint buffer addr in USB_RAM */
  uint8_t*            pAppBuffer;      /* application buffer pointer */
  usbPacketSize_t     appLen;         /* application buffer len */
  usbPacketSize_t     currOffset;     /* current offset for long transactions */
  uint8_t             flag;                /* zero termination flag */
  uint8_t             bdtMapIndex;        /* Corresponding to the buffer */
  bool_t              shortSend;
  bool_t              noTransferInProgress;
} bdtElem_t, *pBdtElem_t;
#if defined(__CWCC__)
	#pragma options align = reset
#elif defined(__IAR_SYSTEMS_ICC__)
	#pragma pack()
#endif
 /*****************************************************************************
 * Global Functions
 *****************************************************************************/

#endif
