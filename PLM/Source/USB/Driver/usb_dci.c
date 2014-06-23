/******************************************************************************
 *
 * Freescale Semiconductor Inc.
 * (c) Copyright 2004-2012 Freescale Semiconductor, Inc.
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
 * @file usb_dci_kinetis.c
 *
 * @author
 *
 * @version
 *
 * @date
 *
 * @brief The file contains Kinetis USB stack controller layer implementation.
 *
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <string.h>
#include "USB_Interface.h"   
#include "usb_dciapi.h" /* USB DCI API Header File */
#include "usb_devapi.h" /* USB Device API Header File */
#include "usb_dci.h"    /* USB DCI Header File */
#include "usb_bdt.h"    /* USB BDT Structure Header File */
#include "usb_class.h"
#include "USB.h"
#include "Utilities_Interface.h"
#if gUsbIncluded_d   
/*****************************************************************************
 * Constant and Macro's - None
 *****************************************************************************/
/*****************************************************************************
 * Local Types - None
 *****************************************************************************/

/****************************************************************************
 * Global Variables
 ****************************************************************************/
/* location for BDT Table and buff */
#if (defined(__CWCC__)||defined(__GNUC__))
	__attribute__((__aligned__(512)))
#elif defined(__IAR_SYSTEMS_ICC__)
	#pragma data_alignment = 512
#endif

/* BDT Map Structure */
#if defined __CC_ARM
   __align(512) static bdtMap_t gBDTMap; 
#else
   static bdtMap_t gBDTMap;      
#endif
/* endpoint buffers allocated by default */
static uint8_t gaEndpBuffers[gSizeOfUsbRam_d];   
/* Table of bdtElem_t used for endpoint management */
static bdtElem_t gBdtElem[gNumUsbEp_d];

/* structure  used for endpoint buffers allocation and for binding the bdt map with bdt elem */
static struct  endpCtl_tag
{
	uint32_t            endpBuffAddrAlloc;
  uint8_t             bdtElemIndexMap[gUsbMaxBdtIndex_d>>1];
	uint8_t             bdtElemIndexAlloc;            
} endpCtl;
/* stores Controller ID */
static uint8_t gDciControllerId = 0;
/* Transfer direction */
static uint8_t gTrfDirection = gUsbTrfDirUnknown_d;

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static void USB_DCI_BusResetHandler(void);

static uint8_t USB_DCI_GetBDTMapIndex(uint8_t ep_num,
                                    uint8_t direction,
                                    bool_t odd);
static uint8_t USB_DCI_GetBDTElemIndex (uint8_t bdtMapIndex);
static uint8_t USB_DCI_ValidateParam(uint8_t ep_num,
                                    uint8_t direction,
                                    bool_t odd);
static void USB_DCI_PrepareSendData(pBuffDsc_t pBuffDsc,
                                    pBdtElem_t pBdtElem);
static void USB_DCI_BusTokenCplHandler(uint8_t stat,
                                    usbDeviceEvent_t* event);

#ifdef USB_LOWPOWERMODE
	static void Enter_StopMode(STOP_MODE stop_mode);
#endif
/*****************************************************************************
 * Local Variables - None
 *****************************************************************************/

/*****************************************************************************
 * Local Functions
 *****************************************************************************/

/**************************************************************************//*!
 *
 * @name  USB_DCI_BusResetHandler
 *
 * @brief The function handles Bus Reset Interrupt
 *
 * @param  None
 *
 * @return None
 *
 ******************************************************************************
 * This functions is called when USB Bus Reset event is received on USB Bus.
 * This function clears all the errors conditions and reinit Global data
 * structures. Also resets USB device controller.
 *****************************************************************************/
  static void USB_DCI_BusResetHandler (void)
  {
    USB0_ERRSTAT = gUsbDciErrStat_ClearAll_c;  /* clear USB error flag */
    USB0_CTL |= USB_CTL_ODDRST_MASK;                /* Reset to Even buffer */
    USB0_ADDR = 0;                       /* reset to default address */
    USB0_USBCTRL = 0x00;
    
    MemorySet( gBdtElem,  (uint8_t)gUninitialisedVal_d, (uint16_t)sizeof(gBdtElem));
    endpCtl.bdtElemIndexAlloc = 0;
    endpCtl.endpBuffAddrAlloc = (uint32_t)gaEndpBuffers;
    MemorySet(endpCtl.bdtElemIndexMap, (uint8_t)gUsbInvalidBdtIndex_d, (uint16_t)sizeof(endpCtl.bdtElemIndexMap));
    MemorySet(&gBDTMap, 0, (uint16_t)sizeof(gBDTMap));     
    gTrfDirection = gUsbTrfDirUnknown_d;
    USB0_CTL &= ~USB_CTL_ODDRST_MASK;
    USB0_USBTRC0 |= 0x40;            
    USB0_ERREN = gUsbDciErrEnb_EnableAll_c;   /* Enable All Error Interrupts */
    USB0_INTEN = gUsbDciIntEnb_IntEnbAtBusReset_c; /* Enable All Interrupts except RESUME */
    USB0_CTL &= ~USB_CTL_TXSUSPENDTOKENBUSY_MASK;
  }
        
/**************************************************************************//*!
 *
 * @name  USB_DCI_GetBDTMapIndex
 *
 * @brief The function maps endpoint number and direction to bdt index
 *
 * @param  ep_num   : Endpoint Number
 * @param  direction: Endpoint direction
 * @param  odd      : Odd or even buffer
 *
 * @return bdt index         : Mapped bdt index
 *         gUsbInvalidBdtIndex_d : In case of error
 *
 ******************************************************************************
 * This function returns BDT Index from Endpoint number, direction,
 * odd/even buffer
 *****************************************************************************/
  static uint8_t USB_DCI_GetBDTMapIndex 
    (
     uint8_t epNum,     /* [IN] Endpoint Number */
     uint8_t direction,  /* [IN] Endpoint direction */
     bool_t odd        /* [IN] Odd or even buffer */
    )
    {
      uint8_t bdtMapIndex = gUsbInvalidBdtIndex_d;
      
      if(epNum < gNumMaxEnpNumber_d)
      {
        /* per endpoint 4 bdt_index -- 2 for recv 2 for send */
        bdtMapIndex = (epNum <<2) ;
        if(direction == gUsbEpDirection_In_c)
        {
          bdtMapIndex += 2;
        }
        if(odd)
        {
          bdtMapIndex++;
        }
      }
      return bdtMapIndex;
    }

/**************************************************************************//*!
 *
 * @name  USB_DCI_GetBDTElemIndex
 *
 * @brief The function returns the index of the associated gBdtElem
 *
 * @param  bdtMapIndex   : the index in the buffer descriptor table
 * 
 *
 * @return bdtElemIndex          : bdtElemIndex associated with the bdtMapIndex received as param
 *         gUsbInvalidBdtIndex_d : In case of error
 *
 ******************************************************************************
 * This function returns BDT Index from Endpoint number, direction,
 * odd/even buffer
 *****************************************************************************/
  static uint8_t USB_DCI_GetBDTElemIndex (uint8_t bdtMapIndex)
  {
    uint8_t bdtElemIndex;
    if(bdtMapIndex >= gUsbMaxBdtIndex_d)
    {
      return gUsbInvalidBdtIndex_d;
    } 
    bdtElemIndex = endpCtl.bdtElemIndexMap[bdtMapIndex>>1];
    if(bdtElemIndex < gNumUsbEp_d)
    {
      return bdtElemIndex;
    }
    return gUsbInvalidBdtIndex_d;
  }
/**************************************************************************//*!
 *
 * @name  USB_DCI_ValidateParam
 *
 * @brief The function validates endpoint number & direction parameters
 *        and returns bdt index.
 *
 * @param  ep_num   : Endpoint Number
 * @param  direction: Endpoint direction
 * @param  odd      : odd or even buffer
 *
 * @return bdt index         : mapped bdt index
 *         gUsbInvalidBdtIndex_d : incase of error
 *
 ******************************************************************************
 * This function validates endpoint parameters and returns bdt index
 *****************************************************************************/
  static uint8_t USB_DCI_ValidateParam 
    (
     uint8_t epNum,     /* [IN] Endpoint Number */
     uint8_t direction,  /* [IN] Endpoint direction */
     bool_t odd        /* [IN] Odd or even buffer */
       )
    {
      /* Get bdt index mapped to endpoint number-direction and odd/even buffer */
      uint8_t bdtElemIndex;
      uint8_t bdtMapIndex = USB_DCI_GetBDTMapIndex(epNum, direction, odd);
      
      if(bdtMapIndex  == gUsbInvalidBdtIndex_d)
        return gUsbInvalidBdtIndex_d;
      bdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex);
      if(bdtElemIndex  == gUsbInvalidBdtIndex_d)
        return gUsbInvalidBdtIndex_d;
      if(gBdtElem[bdtElemIndex].epSize == (uint16_t)gUninitialisedVal_d)
      {
        return gUsbInvalidBdtIndex_d;
      } 
      
      return bdtMapIndex;
    }

/**************************************************************************//*!
 *
 * @name  USB_DCI_PrepareSendData
 *
 * @brief The function sets up the BDT for Send
 *
 * @param  buffer_dsc   : Pointer to buffer descriptor element in USB_RAM
 * @param  bdt_elem     : Pointer to per endpoint/direction structure
 *
 * @return None
 *
 ******************************************************************************
 * This functions configures Buffer Descriptor (Address and Count)
 *****************************************************************************/
static void USB_DCI_PrepareSendData (
    pBuffDsc_t pBuffDsc,  /* [OUT] Pointer to buffer descriptor
                                element in USB_RAM */
    pBdtElem_t pBdtElem     /* [IN] Pointer to per endpoint/direction
                                structure */
)
{
    uint8_t* pBuff = pBdtElem->pAppBuffer + pBdtElem->currOffset;
    uint16_t currentCount = 0;

    /* adjust size based on the input at the init endpoint */
    if((pBdtElem->appLen - pBdtElem->currOffset) > pBdtElem->epSize)
    {
        /* If size of packet is greater than endpoint buffer size */
        currentCount = pBdtElem->epSize;
    }
    else
    {
        /* If size of packet is smaller than endpoint buffer size */
        currentCount = (uint16_t)(pBdtElem->appLen - pBdtElem->currOffset);
    }
    pBuffDsc->cnt = currentCount;
    pBuffDsc->addr = (uint32_t)pBuff;
}

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

/**************************************************************************//*!
 *
 * @name  USB_DCI_Init
 *
 * @brief The function initializes the Controller layer
 *
 * @param controller_ID : Controller ID
 *
 * @return status
 *         gUsbErr_NoError_c    : Always
 ******************************************************************************
 * Initializes the USB controller
 *****************************************************************************/ 
uint8_t USB_DCI_Init 
(
uint8_t    controllerId   /* [IN] Controller ID */
)
{
	/* Select System Clock and Disable Weak Pull Downs */
	USB0_USBCTRL = 0x00;
	/* save the controller_ID for future use */
  gDciControllerId = controllerId;
  /* Clear bdt elem structure */
  MemorySet( gBdtElem,  (uint8_t)gUninitialisedVal_d, (uint16_t)sizeof(gBdtElem));
  endpCtl.bdtElemIndexAlloc = 0;
  endpCtl.endpBuffAddrAlloc = (uint32_t)gaEndpBuffers;
  MemorySet(endpCtl.bdtElemIndexMap, (uint8_t)gUsbInvalidBdtIndex_d, (uint16_t)sizeof(endpCtl.bdtElemIndexMap));
  /* Clear Memory for BDT and buffer Data */
  MemorySet(&gBDTMap, 0, (uint16_t)sizeof(gBDTMap));        
  gTrfDirection = gUsbTrfDirUnknown_d;
  USB0_USBTRC0 |= 0x40;
  /* Set the BDT Table address, Need to be on 512 byte boundary */
  /* D8 Bit is masked in BDT_PAGE_01 */
  USB0_BDTPAGE1 = (uint8_t)(((uint32_t)&gBDTMap >> 8)& 0xFE);
  USB0_BDTPAGE2 = (uint8_t)((uint32_t)&gBDTMap >> 16);
  USB0_BDTPAGE3 = (uint8_t)((uint32_t)&gBDTMap >> 24);
  /* Pull Up configuration */
  USB0_CONTROL = USB_CONTROL_DPPULLUPNONOTG_MASK;
  USB0_CTL = USB_CTL_USBENSOFEN_MASK; 	/* Enable USB module */
  USB0_ISTAT = gUsbDciIntStat_ClearAll_c;      	/* Clear USB interrupts*/
  /* Remove suspend state */
  USB0_USBCTRL &= ~USB_USBCTRL_SUSP_MASK;
  /* Enable USB RESET Interrupt */
  USB0_INTEN |= USB_INTEN_USBRSTEN_MASK;
  /* Enable USB Sleep Interrupt */
  USB0_INTEN |= USB_INTEN_SLEEPEN_MASK;
  // USB0_OTGCTL = USB_OTGCTL_DPHIGH_MASK | USB_OTGCTL_OTGEN_MASK;
  return gUsbErr_NoError_c;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_DeInit
 *
 * @brief The function de-initializes the Controller layer
 *
 * @param controller_ID : Controller ID
 *
 * @return status
 *         gUsbErr_NoError_c    : Always
 ******************************************************************************
 * Initializes the USB controller
 *****************************************************************************/
uint8_t USB_DCI_DeInit(uint8_t    controllerId)
{
  uint8_t i;
  (void)controllerId;
  
  USB0_INTEN = 0;
  USB0_ERREN = 0;
  USB0_CONTROL = 0;
  USB0_USBCTRL = 0xc0;
  for(i=0;i<gNumMaxEnpNumber_d;i++)
  {
    USB_ENDPT_REG(USB0_BASE_PTR,0) =0;
  }
	USB0_USBTRC0 = 0;
  USB0_ADDR = 0;
  /* Clear USB interrupts*/
	USB0_ISTAT = gUsbDciIntStat_ClearAll_c;
  USB0_ERRSTAT = gUsbDciErrStat_ClearAll_c;
  USB0_CTL =0;
	
	
	
  
  
  return gUsbErr_NoError_c;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_Init_EndPoint
 *
 * @brief The function initializes an endpoint
 *
 * @param controller_ID : Controller ID
 * @param ep_ptr        : Pointer to EndPoint Structures
 * @param flag          : Zero Termination
 *
 * @return status
 *         gUsbErr_NoError_c                    : When Successfull
 *         USBERR_EP_INIT_FAILED     : When Error
 ******************************************************************************
 *
 * This function initializes an endpoint and the Bufffer Descriptor Table
 * entry associated with it. Incase the input parameters are invalid it will
 * return USBERR_EP_INIT_FAILED error.
 *
 *****************************************************************************/
uint8_t USB_DCI_InitEndpoint
(
uint8_t               controller_ID,/* [IN] Controller ID */
usbEpStruct_t*    pEpStruct,       /* [IN] Pointer to Endpoint structure,
                                      (endpoint number,
                                      endpoint type,
                                      endpoint direction,
                                      max packet size) */
bool_t              flag          /* [IN] Zero Termination */
)
{
  
  uint8_t bdtMapIndex;
  uint8_t bdtElemIndex;
  pBuffDsc_t  pBuffDsc;
  pBdtElem_t pBdtElem;    
  uint8_t epNum = pEpStruct->number;
  uint8_t direction = pEpStruct->direction;
  uint32_t epCtrl[2] = {gUsbEndpt_Out_c, gUsbEndpt_In_c};
  
  if((pEpStruct->type > gUsbEpType_Interrupt_c) || (pEpStruct->direction > gUsbEpDirection_In_c)) 
  {
    return gUsbErr_EpInitFailed_c;
  } 
  bdtMapIndex = USB_DCI_GetBDTMapIndex(epNum, direction, gUsbBdtEvenBuffer_c);
  if(bdtMapIndex  == gUsbInvalidBdtIndex_d)
  {
    return gUsbErr_EpInitFailed_c;
  }
  bdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex); 
  if((bdtElemIndex  != gUsbInvalidBdtIndex_d) &&
     (gBdtElem[bdtElemIndex].epSize != (uint16_t)gUninitialisedVal_d))
  {
    return gUsbErr_EpInitFailed_c;
  }
  /* if the max packet size is greater than the max buffer size */    
  if(pEpStruct->size > gUsbMaxEpBufferSize_d)
  {
    pEpStruct->size = gUsbMaxEpBufferSize_d;
  }
  
  if(bdtElemIndex == gUsbInvalidBdtIndex_d)
  {
    if((endpCtl.bdtElemIndexAlloc >= gNumUsbEp_d) ||
       ((endpCtl.endpBuffAddrAlloc + pEpStruct->size) > ((uint32_t)gaEndpBuffers + sizeof(gaEndpBuffers))))         
    {
      return gUsbErr_EpInitFailed_c;
    }
    bdtElemIndex = endpCtl.bdtElemIndexMap[bdtMapIndex>>1] = endpCtl.bdtElemIndexAlloc;
    endpCtl.bdtElemIndexAlloc++;
    gBdtElem[bdtElemIndex].bdtMapIndex = bdtMapIndex;
    gBdtElem[bdtElemIndex].addr = endpCtl.endpBuffAddrAlloc;
    endpCtl.endpBuffAddrAlloc += pEpStruct->size;
  }
  pBdtElem = &gBdtElem[bdtElemIndex];
  /* update bdt element structure */
  pBdtElem->epSize = pEpStruct->size;
  pBdtElem->flag = flag;
  /* Update BDTMAP  */
  pBuffDsc = &gBDTMap.epDsc[pBdtElem->bdtMapIndex]; 
  pBuffDsc->cnt = pEpStruct->size;
  /* preserving even/odd buffer bit and address*/
  pBuffDsc->addr = pBdtElem->addr;
  pBuffDsc->stat.byte = (gUsbBdtSCtl_CPU_d | gUsbBdtSCtl_Data0_d | gUsbBdtSCtl_DTS_d);
  
  pBuffDsc = &gBDTMap.epDsc[(uint8_t)((pBdtElem->bdtMapIndex) ^ 1)];
  pBuffDsc->cnt = pEpStruct->size;
  pBuffDsc->addr = pBdtElem->addr;
  pBuffDsc->stat.byte = (gUsbBdtSCtl_CPU_d | gUsbBdtSCtl_Data1_d | gUsbBdtSCtl_DTS_d);
  if(direction == gUsbEpDirection_Out_c)
  {
    /* For Recv Endpoints Give SIE Control to DATA0 */
    pBuffDsc = &gBDTMap.epDsc[pBdtElem->bdtMapIndex];
    pBdtElem->noTransferInProgress = FALSE;
    pBuffDsc->stat.byte |= gUsbBdtSCtl_SIE_d;
  }
  
  /* enable handshake for Non-Isochronous Endpoints */
  epCtrl[direction] |= ((pEpStruct->type != gUsbEpType_Isochronous_c) ? gUsbEndpt_HshkEn_c:0x00);
  /* set the EndPoint Control MCU Register*/
  *((&USB0_ENDPT0) + (4 * epNum)) |= epCtrl[direction];
  return gUsbErr_NoError_c;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_CancelTransfer
 *
 * @brief The function cancels any pending Transfers which ahve not been sent
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param direction       : Endpoint direction
 *
 * @return status
 *         gUsbErr_NotSupported_c : Always
 ******************************************************************************
 * This function just returns Error Code not supported
 *****************************************************************************/
uint8_t USB_DCI_CancelTransfer 
(
uint8_t  controllerId,                                 
uint8_t  endpNumber,    /* [IN] Endpoint number */
uint8_t  direction  /* [IN] Endpoint direction */
)
{
  uint8_t bdtElemIndex;
  uint8_t status= gUsbErr_UnknownError_c;
  /* validate params and get the bdt index */
  uint8_t bdtMapIndex = USB_DCI_ValidateParam (endpNumber, direction, gUsbBdtEvenBuffer_c);
  bdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex);
  /* Check for valid bdt index */
  if(bdtMapIndex != gUsbInvalidBdtIndex_d)
  {
    pBdtElem_t pBdtElem = &gBdtElem[bdtElemIndex];
    pBuffDsc_t pBuffDsc = &gBDTMap.epDsc[pBdtElem->bdtMapIndex];
    pBuffDsc_t pBuffDscAlt = &gBDTMap.epDsc[pBdtElem->bdtMapIndex ^ 1];
    /* Clear SIE Control Bit for both buffers*/
    pBuffDsc->stat.byte &= ~gUsbBdtSCtl_SIE_d;
    pBuffDscAlt->stat.byte &= ~gUsbBdtSCtl_SIE_d;
    pBdtElem->appLen = (usbPacketSize_t)gUninitialisedVal_d;
    pBdtElem->noTransferInProgress = TRUE;
    status = gUsbErr_NoError_c;
  }
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_DeinitEndpoint
 *
 * @brief The function de initializes an endpoint
 *
 * @param controller_ID : Controller ID
 * @param ep_num        : Endpoint number
 * @param direction     : Endpoint direction
 *
 * @return status
 *         gUsbErr_NoError_c                   : When successfull
 *         gUsbErr_EpDeinitFailed_c  : When unsuccessfull
 ******************************************************************************
 *
 * This function un-intializes the endpoint by clearing the corresponding
 * endpoint control register and then clearing the bdt elem.
 *
 *****************************************************************************/
uint8_t USB_DCI_DeinitEndpoint 
(
uint8_t    controllerId,   /* [IN] Controller ID */
uint8_t    epNum,          /* [IN] Endpoint number */
uint8_t    direction        /* [IN] Endpoint direction */
)
{
  /* validate params and get the bdt index */
  uint8_t bdtElemIndex;
  uint8_t bdtMapIndex = USB_DCI_ValidateParam (epNum, direction, gUsbBdtEvenBuffer_c);
  /* in case the bdt_index is invalid*/
  if(bdtMapIndex  == gUsbInvalidBdtIndex_d)
  {
    return gUsbErr_EpDeinitFailed_c;
  }
  bdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex);
  USB_DCI_CancelTransfer(controllerId, epNum, direction);
  /* Disable endpoint */
  *((&USB0_ENDPT0) + (4 * epNum)) = gUsbEndpt_Disable_c;
  /* un-initialize the bdt_elem structure for this endpoint */
  gBdtElem[bdtElemIndex].epSize = (uint16_t)gUninitialisedVal_d;
  return gUsbErr_NoError_c;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_StallEndpoint
 *
 * @brief The function stalls an endpoint
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param direction       : Endpoint direction
 *
 * @return None
 *
 ******************************************************************************
 * This function stalls the endpoint by setting Endpoint BDT
 *****************************************************************************/
void USB_DCI_StallEndpoint 
(
uint8_t  controllerId,                         
uint8_t endpNum,    /* [IN] Endpoint number to stall */
uint8_t direction   /* [IN] Direction to stall */
)
{
  uint8_t bdtElemIndex;
  /* validate params and get the bdt index */
  uint8_t bdtMapIndex = USB_DCI_ValidateParam (endpNum, direction, gUsbBdtEvenBuffer_c);
  if(bdtMapIndex == gUsbInvalidBdtIndex_d)
  {
    return;
  }
  bdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex); 
  bdtMapIndex = gBdtElem[bdtElemIndex].bdtMapIndex;
  (void)USB_DCI_CancelTransfer(controllerId, endpNum, direction);
  gBDTMap.epDsc[bdtMapIndex].stat.byte |= (gUsbBdtSCtl_SIE_d | gUsbBdtSCtl_Stall_d);
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_UnstallEndpoint
 *
 * @brief The function unstalls an endpoint
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param direction       : Endpoint direction
 *
 * @return None
 *
 ******************************************************************************
 * This function unstalls the endpoint by clearing Endpoint Control Register
 * and BDT
 *****************************************************************************/
void USB_DCI_UnstallEndpoint
(
uint8_t  controllerId,                           
uint8_t  endpNumber,    /* [IN] Endpoint number to unstall */
uint8_t  direction           /* [IN] Direction to unstall */
)
{
  uint8_t bdtElemIndex;
  /* validate params and get the bdt index */
  uint8_t bdtMapIndex = USB_DCI_ValidateParam (endpNumber, direction, gUsbBdtEvenBuffer_c);
  /* Check for valid bdt index */
  if(bdtMapIndex == gUsbInvalidBdtIndex_d)
  {
    return;
  }
  bdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex); 
  bdtMapIndex = gBdtElem[bdtElemIndex].bdtMapIndex;
  /* We Require DATA0 PID to be zero on unstall */
  gBDTMap.epDsc[bdtMapIndex].stat.byte = gUsbBdtSCtl_Data0_d;
  if(direction == gUsbEpDirection_Out_c)
  {
    /* Initiate Next receive Transfer */
    USB_DCI_RecvData(controllerId, endpNumber, NULL, 0);
  } 	
  return;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_GetSetupData
 *
 * @brief The function copies Setup Packet from USB RAM to application buffer
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param buffer_ptr      : Application buffer pointer
 *
 * @return None
 *
 ******************************************************************************
 * Copies setup packet from USB RAM to Application Buffer
 *****************************************************************************/
void USB_DCI_GetSetupData 
(
uint8_t  controllerId,                             
uint8_t  endpNumber,    /* [IN] Endpoint number for the transaction */
uint8_t* pBuff          /* [IN] Pointer to the buffer into which to read data */
)
{
	uint8_t* pSetupAddr;
  uint8_t bdtElemIndex;
  /* validate params and get the bdt index */
  uint8_t bdtMapIndex = USB_DCI_ValidateParam (endpNumber, gUsbEpDirection_Out_c, gUsbBdtEvenBuffer_c);
  if(bdtMapIndex == gUsbInvalidBdtIndex_d)
  {
    return;
  }
  bdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex);
  bdtMapIndex = gBdtElem[bdtElemIndex].bdtMapIndex;
  /* address correponding to the endpoint */
  pSetupAddr = (uint8_t*)(gBDTMap.epDsc[bdtMapIndex].addr);
  /* copy bdt buffer to application buffer */
  MemoryCpy( pBuff,   pSetupAddr,   gUsbSetupPacketSize_d     );
  return;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_GetTransferStatus
 *
 * @brief The function retrieves the Transfer status of an endpoint
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param direction       : Endpoint direction
 *
 * @return status
 *         USBERR_TR_FAILED                : When unsuccessful
 *         gUsbStatus_Idle_d                 : No transfer on endpoint
 *         gUsbStatus_Disabled_d             : endpoint is disabled
 *         gUsbStatus_Stalled_d              : endpoint is stalled
 *         gUsbStatus_TransferInProgress_d : When SIE has control of BDT
 ******************************************************************************
 *
 * This function retrieves the transfer status of the endpoint by checking the
 * BDT as well as the endpoint control register
 *
 *****************************************************************************/
uint8_t USB_DCI_GetTransferStatus 
(
uint8_t  controllerId ,                                    
uint8_t  endpNumber,    /* [IN] Endpoint number */
uint8_t  direction           /* [IN] Endpoint direction */
)
{
  uint8_t bdtElemIndex;  
  uint8_t status = gUsbStatus_Disabled_d;
  /* validate params and get the bdt index */
  uint8_t bdtMapIndex = USB_DCI_ValidateParam (endpNumber, direction, gUsbBdtEvenBuffer_c);
  /* Check for valid bdt index */
  if(bdtMapIndex != gUsbInvalidBdtIndex_d)
  {
    uint8_t epCtrl = (uint8_t)(*((&USB0_ENDPT0)+4*endpNumber));
    bdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex);
    bdtMapIndex = gBdtElem[bdtElemIndex].bdtMapIndex;
    status = gUsbStatus_Idle_d;
    /* Check for direction in endpoint control register */
    if((epCtrl & (gUsbEndpt_In_c|gUsbEndpt_Out_c)) == gUsbEndpt_Disable_c)
    {
      status = gUsbStatus_Disabled_d;
    }
    /* Check for stall bit in endpoint control register */
    else if(
            (gBDTMap.epDsc[bdtMapIndex].stat.mcuCtlBit.bdtstall == 1)
              &&(gBDTMap.epDsc[bdtMapIndex].stat.sieCtlBit.own == 1)
                )
    {
      status = gUsbStatus_Stalled_d ;
    }
    /* Check whether SIE has control of BDT */
    else if (gBdtElem[bdtElemIndex].noTransferInProgress == FALSE)
    {
      status = gUsbStatus_TransferInProgress_d;
    }
  }
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_RecvData
 *
 * @brief The function retrieves data received on an RECV endpoint
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param buffer_ptr      : Application buffer pointer
 * @param size            : Size of the buffer
 *
 * @return status
 *         gUsbErr_NoError_c                          : When successful
 *         gUsbErr_RxFailed_c                : When unsuccessful
 ******************************************************************************
 * This function retrieves data received data on a RECV endpoint by copying it
 * from USB RAM to application buffer
 *****************************************************************************/
uint8_t USB_DCI_RecvData 
(
uint8_t  controllerId,                           
uint8_t  endpNumber,    /* [IN] Endpoint number for the transaction */
uint8_t* pBuff,         /* [OUT] Pointer to the buffer into which to receive data */
usbPacketSize_t size
)
{
  uint8_t status = gUsbErr_RxFailed_c;
  uint8_t bdtElemIndex;
  pBdtElem_t pBdtElem;
  pBuffDsc_t pBuffDsc;
  uint32_t ccr;
  /* validate params and get the bdt index */
  uint8_t bdtMapIndex = USB_DCI_ValidateParam (endpNumber, gUsbEpDirection_Out_c, gUsbBdtEvenBuffer_c);
  (void)(controllerId);
  /* Check for valid bdt index */
  if(bdtMapIndex == gUsbInvalidBdtIndex_d)
  {
    return status;
  }
  ccr = IntDisableAll();
  bdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex);
  pBdtElem = &gBdtElem[bdtElemIndex];
  /* For selecting even/odd buffer */
  bdtMapIndex = pBdtElem->bdtMapIndex;
  pBuffDsc = &gBDTMap.epDsc[bdtMapIndex];
  /* Does MCU owns it */
  if(pBdtElem->noTransferInProgress)
  {
    if(size == 0)
    {
      pBuffDsc->cnt = pBdtElem->epSize;
      pBuffDsc->addr = pBdtElem->addr;
      
      /* Give the ownership to SIE and TOGGLE DATA BIT */
    }
    /* adjust size based on the input at the init endpoint */
    else
    {
      /* Initialise transfer */
      pBdtElem->appLen = size;
      pBdtElem->pAppBuffer = pBuff;
      if(size > pBdtElem->epSize)
      {
        size = pBdtElem->epSize;
      }
      pBdtElem->currOffset = 0;
      pBuffDsc->cnt = size;
      pBuffDsc->addr = (uint32_t)pBuff;
    } 
    pBdtElem->noTransferInProgress = FALSE;
    pBuffDsc->stat.byte = (uint8_t)(
                                       (pBuffDsc->stat.mcuCtlBit.data << 6) |
                                         gUsbBdtSCtl_SIE_d | gUsbBdtSCtl_DTS_d);
    status = gUsbErr_NoError_c;
  }
  IntRestoreAll(ccr);
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_SendData
 *
 * @brief The function configures Controller to send data on an SEND endpoint
 *
 * @param handle          : USB Device handle
 * @param endpoint_number : Endpoint number
 * @param buffer_ptr      : Application buffer pointer
 * @param size            : Size of the buffer
 *
 * @return status
 *         gUsbErr_NoError_c           : When successfull
 *         gUsbErr_TxFailed_c : When unsuccessfull
 ******************************************************************************
 * This function configures Controller to send data on a SEND endpoint by
 * setting the BDT to send data.
 *****************************************************************************/
uint8_t USB_DCI_SendData 
(
 uint8_t          controllerId,                           
 uint8_t          endpNumber,    /* [IN] Endpoint number */
 uint8_t*         pBuff,         /* [IN] Application buffer pointer */
 usbPacketSize_t  size    ,            /* [IN] Size of the buffer */
 bool_t           shortSend  
 )
{
  
  uint8_t status = gUsbErr_TxFailed_c;
  pBuffDsc_t pBuffDsc;
  pBdtElem_t pBdtElem;
  uint32_t ccr;
  /* validate params and get the bdt index */
  uint8_t bdtMapIndex = USB_DCI_ValidateParam (endpNumber, gUsbEpDirection_In_c, gUsbBdtEvenBuffer_c);
  (void)(controllerId);
  if(bdtMapIndex == gUsbInvalidBdtIndex_d)
  {
    return status;
  }
  ccr = IntDisableAll();
  pBdtElem = &gBdtElem[USB_DCI_GetBDTElemIndex(bdtMapIndex)];
  bdtMapIndex = pBdtElem->bdtMapIndex;
  pBuffDsc = &gBDTMap.epDsc[bdtMapIndex];
  /* Does MCU owns it and it is not stalled */
    if(
     pBdtElem->noTransferInProgress 
     && (pBuffDsc->stat.mcuCtlBit.bdtstall == 0)
       )
  {
    /* Initialize transfer */
    pBdtElem->appLen = size;
    pBdtElem->pAppBuffer = pBuff;
    pBdtElem->currOffset = 0;
    pBdtElem->shortSend = shortSend;
    /* Prepare for send */
    USB_DCI_PrepareSendData(pBuffDsc, pBdtElem);
    pBdtElem->noTransferInProgress = FALSE;
    pBuffDsc->stat.byte = (uint8_t)((pBuffDsc->stat.mcuCtlBit.data << 6) | gUsbBdtSCtl_SIE_d | gUsbBdtSCtl_DTS_d);
    status = gUsbErr_NoError_c;
  } /* Does MCU own IN BDT */
  IntRestoreAll(ccr);
  return status;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_SetAddress
 *
 * @brief The function configures Controller to send data on an SEND endpoint
 *
 * @param handle  : USB Device handle
 * @param address : Controller Address
 *
 * @return None
 *
 ******************************************************************************
 * Assigns the Address to the Controller
 *****************************************************************************/
void  USB_DCI_SetAddress 
(
uint8_t controllerId,                           
uint8_t address    /* [IN] Address of the USB device */
)
{
  (void)(controllerId);
	/* set the address */
  USB0_ADDR = address;
  USB_DeviceSetStatus(gDciControllerId, gUsbComponentStatus_DeviceState_d,gUsbDeviceState_Address_d);
  return;
}
/**************************************************************************//*!
 *
 * @name  USB_DCI_AssertResume
 *
 * @brief The function makes the Controller start USB RESUME signaling
 *
 * @param handle : USB Device handle
 *
 * @return None
 *
 ******************************************************************************
 *
 * This function starts RESUME signalling and then stops it after some delay.
 * In this delay make sure that COP is reset.
 *
 *****************************************************************************/
void USB_DCI_AssertResume
(
uint8_t controllerId                            
)
{
  uint16_t delayCount;
  (void)controllerId;
  /* Clear SUSP Bit from USB_CTRL */
  USB0_USBCTRL &= ~USB_USBCTRL_SUSP_MASK;
  /* Reset Low Power RESUME enable */
  USB0_USBTRC0 &= ~USB_USBTRC0_USBRESMEN_MASK;
  /* Setup the controller for Remote Wakeup */
	USB0_ISTAT |= USB_ISTAT_RESUME_MASK; 
	USB0_INTEN &= ~USB_INTEN_RESUMEEN_MASK;
	USB0_CTL &= ~USB_CTL_TXSUSPENDTOKENBUSY_MASK;
  USB0_CTL |= USB_CTL_RESUME_MASK;   /* Start RESUME signaling and make SUSPEND bit 0*/
  delayCount = gUsbDci_AssertResumeDelayCount_c; /* Set RESUME line for 1-15 ms*/
  do
  {
    delayCount--;
    //  Watchdog_Reset();    /* Reset the COP */
  }while(delayCount);
  USB0_CTL &= ~USB_CTL_RESUME_MASK;          /* Stop RESUME signalling */
  return;
}

/**************************************************************************//*!
 *
 * @name  USB_DCI_BusTokenCplHandler
 *
 * @brief The function handles Token Complete USB interrupts on the bus.
 *
 * @param stat  : BDT stat byte
 * @param event : Pointer to USB EVENT Structure
 *
 * @return None
 ******************************************************************************
 * This function handles Token Complete USB interrupts on the bus.
 *****************************************************************************/

void USB_DCI_BusTokenCplHandler 
(
uint8_t stat,            /* [IN] Value of STAT register */
usbDeviceEvent_t* event /* [IN] Pointer to USB EVENT Structure */
)
{
  uint8_t bdtMapIndex = 0 ;
  uint8_t bdtElemIndex;
  pBuffDsc_t pBuffDsc = NULL;
  pBuffDsc_t pBuffDscAlt = NULL;/* stores data of alternate buffer */
  pBdtElem_t pBdtElem = NULL;
  /* Get the direction from STAT register */
  event->direction = (uint8_t)((stat & gUsbStat_EndpDirMask_c) >>gUsbStat_EndpDirShift_c);
  /* Get bdt index from STAT register*/
  bdtMapIndex =  stat>>2;
  pBuffDsc = &gBDTMap.epDsc[bdtMapIndex];
  pBuffDscAlt = &gBDTMap.epDsc[bdtMapIndex ^ 1];
  bdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex);
#ifdef gUSB_Debug_d 
  if(bdtElemIndex == gUsbInvalidBdtIndex_d)
  {
    while(TRUE);
  }
#endif
  pBdtElem = &gBdtElem[bdtElemIndex];
  /* Get address from BDT */
  event->pBuffer = (uint8_t*)(pBuffDsc->addr);
  /* Get len from BDT */
  event->len = pBuffDsc->cnt;
  event->shortSend = pBdtElem->shortSend;
  /* Prepare for Next USB Transaction */
  pBdtElem->bdtMapIndex = bdtMapIndex^1;
  /* Toggle Data PID*/
  pBuffDscAlt->stat.byte = (uint8_t)((pBuffDsc->stat.mcuCtlBit.data ^ 1) << 6);
  if(event->direction == gUsbEpDirection_In_c)
  {
    if(event->epNum == gUsbControlEndpoint_d)
    {
      /* for Control Endpoint */
      /* For Transfer Direction Host to Device */
      if(gTrfDirection == gUsbEpDirection_Out_c)
      {
        uint8_t ctrlOutBdtElemIndex;
        pBdtElem_t pCtrlOutBdtElem;
        pBuffDsc_t pCtrlOutBufferDsc;
        /* make Transfer Direction UNKNOWN */                
        gTrfDirection = gUsbTrfDirUnknown_d;
        ctrlOutBdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex^2);
#ifdef gUSB_Debug_d 
        if(ctrlOutBdtElemIndex == gUsbInvalidBdtIndex_d)
        {
          while(TRUE);
        }
#endif
        pCtrlOutBdtElem = &gBdtElem[ctrlOutBdtElemIndex];
        pCtrlOutBufferDsc = &gBDTMap.epDsc[pCtrlOutBdtElem->bdtMapIndex];
        
        pCtrlOutBufferDsc->stat.byte = gUsbBdtSCtl_Data0_d;
        pCtrlOutBdtElem->noTransferInProgress = TRUE;
        USB_DCI_RecvData((event->controllerId), gUsbControlEndpoint_d, NULL,0);
        
      }
    }/* ep_num is CONTROL ENDPOINT */
    
    if((pBdtElem->shortSend == FALSE)&& ((gTrfDirection == gUsbEpDirection_In_c) ||
                                         (event->epNum != gUsbControlEndpoint_d)))
    {
      /* update the request */
      pBdtElem->currOffset += event->len;
      /*
      Initiate next USB SEND if:
      1. More Data is still pending OR
      2. Send Data == Endpoint Size AND
      3. Zero Termination Flag is TRUE
      */
      if ((pBdtElem->appLen > pBdtElem->currOffset) ||
          (((uint8_t)event->len == pBdtElem->epSize) && (pBdtElem->flag == TRUE))
            )
      {
        /* send next Req */
        USB_DCI_PrepareSendData(pBuffDscAlt, pBdtElem);
        
        /* give the ownership to SIE and TOGGLE DATA BIT */
        pBuffDscAlt->stat.byte = (uint8_t)(
                                            ((pBuffDscAlt->stat.mcuCtlBit.data) << 6) |
                                              gUsbBdtSCtl_SIE_d | gUsbBdtSCtl_DTS_d);;
                                              return;
      }
      else
      {
        event->pBuffer = pBdtElem->pAppBuffer;
        event->len = pBdtElem->currOffset;
      }
    }
    
  }/* End of SEND loop */
  else /* direction IS gUsbEpDirection_Out_c */
  {
    if(event->epNum == gUsbControlEndpoint_d)
    {
      /* for Control Endpoint */
      if(pBuffDsc->stat.recPid.pid == gUsbToken_Setup_c)
      {
        uint8_t ctrlInBdtElemIndex;
        pBdtElem_t pCtrlInBdtElem;
        pBuffDsc_t pCtrlInBufferDsc;
        /* set setup phase */
        event->setup = TRUE;
        /* Transfer direction of next packet */
        gTrfDirection = event->pBuffer[0] >> 7;
        ctrlInBdtElemIndex = USB_DCI_GetBDTElemIndex(bdtMapIndex^2);
#ifdef gUSB_Debug_d 
        if(ctrlInBdtElemIndex == gUsbInvalidBdtIndex_d)
        {
          while(TRUE);
        }
#endif
        pCtrlInBdtElem = &gBdtElem[ctrlInBdtElemIndex];
        pCtrlInBufferDsc = &gBDTMap.epDsc[pCtrlInBdtElem->bdtMapIndex];
        pCtrlInBufferDsc->stat.byte = gUsbBdtSCtl_Data1_d;
        pCtrlInBdtElem->noTransferInProgress = TRUE;
        
      }
      else if(gTrfDirection == gUsbEpDirection_In_c)
      {
        /* make Transfer Direction UNKNOWN */
        gTrfDirection = gUsbTrfDirUnknown_d;
        /* We Require DATA0 PID for Setup Token */
        pBuffDscAlt->stat.byte = gUsbBdtSCtl_Data0_d;
        pBdtElem->noTransferInProgress = TRUE;
        /* Prepare for Next SETUP PACKET Receive */
        USB_DCI_RecvData((event->controllerId), gUsbControlEndpoint_d, NULL,0);
        return;
      }
    }   /* ep_num is CONTROL ENDPOINT */
    
    if(pBdtElem->appLen != (usbPacketSize_t)gUninitialisedVal_d)
    {
      /* on control endpoint the data is only 8 bytes */
      usbPacketSize_t size = event->len;
      pBdtElem->currOffset += size;
      /*
      Initiate next USB RECV if:
      1. More Data is still pending OR
      2. Received Data == Endpoint Size AND
      3. Zero Termination Flag is TRUE
      */
      if(
         (size == pBdtElem->epSize) &&
           ((pBdtElem->appLen > pBdtElem->currOffset) || ((pBdtElem->appLen == pBdtElem->currOffset) && (pBdtElem->flag == TRUE)))
             ) 
      {
        /* send next IO */
        uint16_t count;
        count = (uint16_t)(((pBdtElem->appLen - pBdtElem->currOffset)
                            > pBdtElem->epSize) ? pBdtElem->epSize :
                               (pBdtElem->appLen - pBdtElem->currOffset));
				if(count == 0)
				{
          /* For Zero byte Packet Receive */
					pBuffDscAlt->addr = pBdtElem->addr;
          pBuffDscAlt->cnt = 0;
				}
				else
				{
          pBuffDscAlt->addr = (uint32_t)(pBdtElem->pAppBuffer + pBdtElem->currOffset);
          pBuffDscAlt->cnt = count;
				}
        /* give the ownership to SIE and Toggle DATA bit*/
        pBuffDscAlt->stat.byte = (uint8_t)((
                                             (pBuffDscAlt->stat.mcuCtlBit.data) << 6) |
                                            gUsbBdtSCtl_SIE_d | gUsbBdtSCtl_DTS_d);
        return;
      }
      else /* request completed */
      {
        /* populate buffer structure */
        event->pBuffer = pBdtElem->pAppBuffer;
        event->len = pBdtElem->currOffset;
        pBdtElem->appLen = (usbPacketSize_t)gUninitialisedVal_d;
      }
    }
  } /* End of RECV loop */
  pBdtElem->noTransferInProgress = TRUE;
  /* Notify Device Layer of Data Received or Sent Event */
  (void)USB_DeviceCallService(event->epNum, event);
  return;
}




/**************************************************************************//*!
 *
 * @name  USB_ISR
 *
 * @brief The function handles USB interrupts on the bus.
 *
 * @param None
 *
 * @return None
 *
 ******************************************************************************
 * This function is hooked onto interrupt 69 and handles the USB interrupts.
 * After handling the interrupt it calls the Device Layer to notify it about
 * the event.
 *****************************************************************************/

void USB_ISR(void)
{
  usbDeviceEvent_t event;
  uint16_t devState = gUsbStatus_Unknown_d;
  uint8_t stat = (uint8_t)USB0_STAT;
  /* Which interrupt occured and also was enabled */
  uint8_t intrStat =  USB0_ISTAT;
  intrStat &= USB0_INTEN;
  /* initialize event structure */
  event.controllerId = gDciControllerId;
  event.setup = FALSE;
  event.pBuffer = NULL;
  event.len = 0;
  event.direction = gUsbEpDirection_Out_c;
  event.errors = 0;
  event.shortSend = FALSE;
  event.epNum = (uint8_t)gUninitialisedVal_d;
  /* Get the device state from the Device Layer */
  (void)USB_DeviceGetStatus(gDciControllerId, gUsbComponentStatus_DeviceState_d,&devState);
  /* if current device state is SUSPEND and Low Power Resume Flag set */
  if((USB0_USBTRC0 & USB_USBTRC0_USB_RESUME_INT_MASK) && (devState == gUsbDeviceState_Suspend_d))
  {
    /* Clear SUSP Bit from USB_CTRL */
    USB0_USBCTRL &= ~USB_USBCTRL_SUSP_MASK;
    /* Reset Low Power RESUME enable */
    USB0_USBTRC0 &= ~USB_USBTRC0_USBRESMEN_MASK;
  }
  
  /* SOF received */
  if(UsbDciIntFlag_SofToken(intrStat))
  {
    uint16_t sofCount;
    sofCount = USB0_FRMNUMH;
    sofCount <<= 8;
    sofCount += USB0_FRMNUML;
    /* Clear SOF Interrupt */
    USB0_ISTAT = USB_ISTAT_SOFTOK_MASK;
    /*address of Lower byte of Frame number*/
    event.pBuffer = (uint8_t*)(&sofCount);
    /* Notify Device Layer of SOF Event */
    (void)USB_DeviceCallService(gUsbService_SOF_d, &event);
  }
  
  if(UsbDciIntFlag_BusReset(intrStat))
  {
    /* Clear Reset Flag */
    USB0_ISTAT = USB_ISTAT_USBRST_MASK;
    /* Handle RESET Interrupt */
    USB_DCI_BusResetHandler();
    /* Notify Device Layer of RESET Event */
    (void)USB_DeviceCallService(gUsbService_BusReset_d, &event);
    /* Clearing this bit allows the SIE to continue token processing and clear suspend condition */
    USB0_CTL &= ~USB_CTL_TXSUSPENDTOKENBUSY_MASK;
    /* No need to process other interrupts */
    return;
  }
  
  if(UsbDciIntFlag_TokenComplete(intrStat))
  {
    /* Clear TOKEN Interrupt */
    USB0_ISTAT = USB_ISTAT_TOKDNE_MASK;
    event.epNum = (uint8_t)((stat & gUsbStat_EndpNumberMask_c) >> gUsbStat_EndpNumberShift_c);
    USB_DCI_BusTokenCplHandler(stat, &event);
    /* Clearing this bit allows the SIE to continue token processing and clear suspend condition */
    USB0_CTL &= ~USB_CTL_TXSUSPENDTOKENBUSY_MASK;
  }
  
  if(UsbDciIntFlag_Error(intrStat))
  {
    /* Clear ERROR Interrupt */
    USB0_ISTAT = USB_ISTAT_ERROR_MASK;
    event.errors = USB0_ERRSTAT;
    event.errors &= USB0_ERREN;
    /* Notify Device Layer of ERROR Event to error service */
    (void)USB_DeviceCallService(gUsbService_Error_d, &event);
    USB0_ERRSTAT = gUsbDciErrStat_ClearAll_c;  /*clear all errors*/
    /* Clearing this bit allows the SIE to continue token processing and clear suspend condition */
    USB0_CTL &= ~USB_CTL_TXSUSPENDTOKENBUSY_MASK;
  }
  
  if(UsbDciIntFlag_Sleep(intrStat))
  {
    /* Clear RESUME Interrupt if Pending */
    USB0_ISTAT = USB_ISTAT_RESUME_MASK;
    /* Clear SLEEP Interrupt */
  	USB0_ISTAT = USB_ISTAT_SLEEP_MASK;
    /* Notify Device Layer of SLEEP Event */
    (void)USB_DeviceCallService(gUsbService_Sleep_d, &event);
    /* Set Low Power RESUME enable */
    USB0_USBTRC0 |= USB_USBTRC0_USBRESMEN_MASK;
    /* Set SUSP Bit in USB_CTRL */
    USB0_USBCTRL |= USB_USBCTRL_SUSP_MASK;
    /* Enable RESUME Interrupt */
    USB0_INTEN |= USB_INTEN_RESUMEEN_MASK;
  }
  
  if(UsbDciIntFlag_Resume(intrStat))
  {
    /* Clear RESUME Interrupt */
    USB0_ISTAT = USB_ISTAT_RESUME_MASK;
    /* Notify Device Layer of RESUME Event */
    (void)USB_DeviceCallService(gUsbService_Resume_d, &event);
    /* Disable RESUME Interrupt */
    USB0_INTEN &= ~USB_INTEN_RESUMEEN_MASK;
  }
  
  if(UsbDciIntFlag_Stall(intrStat))
  {
    uint16_t endpStatus;
    event.epNum = (uint8_t)gUninitialisedVal_d;
    /* If Control Endpoint is stalled then unstall it.
    For other endpoints host issues clear endpoint feature request
    to unstall them */
    /* Get Control Endpoint Status*/
    (void)USB_DeviceGetStatus((event.controllerId),
                              (gUsbComponentStatus_Endpoint_d|gUsbControlEndpoint_d),
                              &endpStatus);
     if(endpStatus == gUsbStatus_Stalled_d)
     {
       event.epNum = gUsbControlEndpoint_d;
       event.direction = gUsbEpDirection_In_c;
     }
     /* Clear STALL Interrupt */
     USB0_ISTAT = USB_ISTAT_STALL_MASK;
     /* Notify Device Layer of STALL Event */
     (void)USB_DeviceCallService(gUsbService_Stall_d, &event);
     /* Clearing this bit allows the SIE to continue token processing and clear suspend condition */
     USB0_CTL &= ~USB_CTL_TXSUSPENDTOKENBUSY_MASK;
  }
  return;
}

#endif//gUsbIncluded_d