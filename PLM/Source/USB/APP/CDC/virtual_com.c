/******************************************************************************
 * Virtual Com Application
 * Freescale Semiconductor Inc.
 * (c) Copyright 2012 Freescale Semiconductor, Inc.
 * ALL RIGHTS RESERVED.
 *
 *****************************************************************************/
 
 

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "EmbeddedTypes.h"          /* User Defined Data Types */
#include "Utilities_Interface.h"
#include "Interrupt.h"
#include "USB_Interface.h"   
#include "USB.h"
#include "usb_cdc.h"        /* USB CDC Class Header File */
#include "CDC_Interface.h" 
#include "PortConfig.h"
#if gVirtualCOMPort_d
/*****************************************************************************
 * Constant and Macro's - None
 *****************************************************************************/
#define USB_Class_CDC_Interface_DIC_Send_Data( controller_ID, buff_ptr, size)  \
         USB_ClassSendData( controller_ID , gCDC_DIC_BulkInEndpoint_d , buff_ptr , size)
#define USB_Class_CDC_Interface_DIC_Recv_Data(controllerId, pBuff, size)  \
        USB_DeviceRecvData(controllerId, gCDC_DIC_BulkOutEndpoint_d, pBuff, size);        
 
#if(gUsbCdcEnabled_d == FALSE)
 #error "gUsbCdcEnabled_d in USB_Configuration.h must be TRUE"
#endif

#if (gUsbIncluded_d == FALSE)
 #error If gVirtualCOMPort_d == TRUE, gUsbIncluded_d must be TRUE
#endif

#if gCDC_RxFlowControlResume_d >= (gCDC_ReceiveBufferSize_c - gCDC_RxFlowControlSkew_d)
#error Deassert flow control before it is asserted?
#endif

#if gCDC_RxFlowControlSkew_d < gUsbCdcBulkOutEpSize_d
#error gCDC_RxFlowControlSkew_d must be >= gUsbCdcBulkOutEpSize_d
#endif
/*****************************************************************************
 * Global Functions Prototypes
 *****************************************************************************/
bool_t CDC_CommOpen(void);

/****************************************************************************
 * Global Variables
 ****************************************************************************/
 
/*****************************************************************************
 * Local Types - None
 *****************************************************************************/
typedef struct cdcTxBufRef_tag {
  unsigned char const *pBuf;
  void (*pfCallBack)(unsigned char const *pBuf);
} cdcTxBufRef_t;

/*****************************************************************************
 * Local Functions Prototypes
 *****************************************************************************/
static void CDC_DicInServiceCallback(usbDeviceEvent_t* event);
static void CDC_DicOutServiceCallback(usbDeviceEvent_t* event);
static void CDC_App_Callback(uint8_t controller_ID,
                        gUsbToAppEvent_t event_type, void* val);
//static bool_t CDC_CommOpen(void);                         

/*****************************************************************************
 * Local Variables
 *****************************************************************************/

//Tx variables
static cdcTxBufRef_t maCDCTxBufRefTable[gCDC_TransmitBuffers_c];
static index_t maCDCTxBufLenTable[gCDC_TransmitBuffers_c];
static index_t mCDCTxBufRefLeadingIndex;   /* Post-increment. */
static index_t mCDCTxPendingTxNum;
static index_t mCDCTxBufRefTrailingIndex;  /* Post-increment. */

//Rx variables
static   uint8_t maCDCRxBuf[gCDC_ReceiveBufferSize_c];
static   index_t mCDCRxBufLeadingIndex;      /* Post-increment. */
static   index_t mCDCRxBufTrailingIndex;     /* Post-increment. */
volatile index_t gCDCRxBufferByteCount;      /* # of bytes in buffer. */
static void (*pfCDCRxCallBack)(void);
#if gUsbAppNotificationOnSuspend_d  
static void (*pfUSBSuspendCallBack)(void);
#endif
volatile static bool_t gCDCDicRxMsgInUsbOueue;
volatile static bool_t gCDCDicTxMsgInUsbOueue;
volatile static bool_t gCDCDicRxEnabled;

static volatile bool_t mCDCEnumerated;
#if gUSB_CDC_SendEventToAppOnCOMOpen_d 
static volatile bool_t mCDCChannelOpen;
#endif 
const usbClassDescriptor_t usbClassDescriptor_CDC= 
{
  gUsbClassId_Cdc_c, 
  CDC_App_Callback,
  NULL,
  NULL,
  USB_CDC_Desc_GetDescriptor,
  USB_CDC_Desc_GetInterface,
  USB_CDC_Desc_SetInterface,
  USB_CDC_Desc_ValidConfiguration,
  USB_CDC_Desc_ValidInterface,
  USB_CDC_Desc_RemoteWakeup,
  gUsbCdcControlEpSize_d,
  gNumUsbCdcNonControlEp_d,
  (usbEpStruct_t*)usbCdcDescEp
} ;




/*****************************************************************************
 * Global Functions
 *****************************************************************************/

/*****************************************************************************/

bool_t CDC_ModuleInit(void)
{  
  uint32_t ccr;
  uint8_t status = FALSE;
  ccr = IntDisableAll();
  if(USB_IsDeviceConnected(gUsbCdcControllerId_d) == FALSE)
  {
    usbError_t usbErr;
    MemorySet( maCDCTxBufLenTable , 0 , sizeof(maCDCTxBufLenTable));
    mCDCTxBufRefLeadingIndex = mCDCTxBufRefTrailingIndex = 0;
    mCDCTxPendingTxNum = 0;
    mCDCRxBufLeadingIndex = mCDCRxBufTrailingIndex = gCDCRxBufferByteCount = 0;
    gCDCDicRxMsgInUsbOueue = FALSE;
    gCDCDicTxMsgInUsbOueue = FALSE;
    gCDCDicRxEnabled = TRUE;
    USB_Class_CDC_SetDicInServiceCallback(CDC_DicInServiceCallback);
    USB_Class_CDC_SetDicOutServiceCallback(CDC_DicOutServiceCallback);
    usbErr =  USB_AddController(&usbClassDescriptor_CDC, gUsbCdcControllerId_d);   
    if(usbErr == gUsbErr_NoError_c)
    {
      status = TRUE;   
    }
  } 
  IntRestoreAll(ccr);
  return status;                               
}

/*****************************************************************************/

bool_t CDC_ModuleUninit(void)
{
   uint32_t ccr;
   uint8_t status = FALSE;
   usbError_t usbErr;
   ccr = IntDisableAll();
   usbErr = USB_RemoveController(gUsbCdcControllerId_d);   
   if(usbErr == gUsbErr_NoError_c)
   {
     pfCDCRxCallBack = NULL;
#if gUsbAppNotificationOnSuspend_d  
     pfUSBSuspendCallBack=NULL;
#endif     
     USB_Class_CDC_SetDicInServiceCallback(NULL);
     USB_Class_CDC_SetDicOutServiceCallback(NULL);
     mCDCEnumerated = FALSE;
#if gUSB_CDC_SendEventToAppOnCOMOpen_d            
     mCDCChannelOpen = FALSE;
#endif      
     status = TRUE;   
   }
   IntRestoreAll(ccr);
   return status;
}

/*****************************************************************************/

bool_t CDC_Transmit(unsigned char const *pBuf, index_t bufLen, void (*pfCallBack)(unsigned char const *pBuf))
{

  bool_t transmit = FALSE;
  bool_t status = FALSE;
  uint32_t ccr;
  unsigned char const *pUncalledBuf;

  void (*pfUncalledCallBack)(unsigned char const*) = NULL;
  /* Handle empty buffers. */
  if (!bufLen) 
  {
    if(pfCallBack)
      (*pfCallBack)(pBuf);
    return TRUE;
  }
  
  if(CDC_CommOpen() == FALSE)
  {
     if(pfCallBack)
      (*pfCallBack)(pBuf);
   return TRUE;    
  }
  ccr = IntDisableAll();
    /* Room for one more? */
  if(maCDCTxBufLenTable[mCDCTxBufRefLeadingIndex] == 0)
  {
    pfUncalledCallBack = maCDCTxBufRefTable[mCDCTxBufRefLeadingIndex].pfCallBack ; 
    pUncalledBuf = maCDCTxBufRefTable[mCDCTxBufRefLeadingIndex].pBuf;
    maCDCTxBufRefTable[mCDCTxBufRefLeadingIndex].pBuf = pBuf;
    maCDCTxBufRefTable[mCDCTxBufRefLeadingIndex].pfCallBack = pfCallBack;
    maCDCTxBufLenTable[mCDCTxBufRefLeadingIndex] = bufLen;
    if (++mCDCTxBufRefLeadingIndex >= NumberOfElements(maCDCTxBufRefTable)) 
    {
      mCDCTxBufRefLeadingIndex = 0;
    }
    mCDCTxPendingTxNum++;
    if(mCDCTxPendingTxNum == 1)
    {
      transmit = TRUE;
    }
    status = TRUE;
  }
  IntRestoreAll(ccr);
  if(transmit)
  {
    (void)USB_Class_CDC_Interface_DIC_Send_Data(gUsbCdcControllerId_d,(uint8_t*)pBuf, bufLen);    
  }
  if(pfUncalledCallBack != NULL)
  {
    pfUncalledCallBack(pUncalledBuf);
  }
  return status; 
    
}

/*****************************************************************************/

bool_t CDC_IsTxActive(void)
{
  return (maCDCTxBufLenTable[mCDCTxBufRefTrailingIndex] != 0);
}




/*****************************************************************************/

void CDC_SetRxCallBack(void (*pfCallBack)(void))
{
  uint32_t ccr;
  ccr = IntDisableAll();
  pfCDCRxCallBack = pfCallBack;
  if (pfCDCRxCallBack == NULL) 
  {
    mCDCRxBufLeadingIndex = mCDCRxBufTrailingIndex = gCDCRxBufferByteCount = 0;
  } 
  IntRestoreAll(ccr);
}
/*****************************************************************************/

void CDC_SetUsbSuspendCallBack(void (*pfCallBack)(void))
{
#if gUsbAppNotificationOnSuspend_d  
  pfUSBSuspendCallBack = pfCallBack;
#else
  (void)pfCallBack;
#endif  
}

/*****************************************************************************/

 bool_t CDC_GetByteFromRxBuffer(unsigned char *pDst)
 {
  uint32_t ccr;
  if (!gCDCRxBufferByteCount)
  {
    return FALSE;
  }
  ccr = IntDisableAll();
  *pDst = maCDCRxBuf[mCDCRxBufTrailingIndex];
  if (++mCDCRxBufTrailingIndex >= sizeof(maCDCRxBuf)) 
  {
    mCDCRxBufTrailingIndex = 0;
  }
  --gCDCRxBufferByteCount;
#if gCDC_EnableFlowControl_d
  if ((gCDCDicRxEnabled == FALSE) && (gCDCRxBufferByteCount <= gCDC_RxFlowControlResume_d)) 
  {
    gCDCDicRxEnabled = TRUE;
    (void)USB_Class_CDC_Interface_DIC_Recv_Data(gUsbCdcControllerId_d, NULL, 0);
  }
#endif
  IntRestoreAll(ccr);
  return TRUE;
 }

/*****************************************************************************
* Local Functions
*****************************************************************************/

 /*****************************************************************************/
//static bool_t CDC_CommOpen(void)  @SMAC
bool_t CDC_CommOpen(void) 
  {
    if(mCDCEnumerated 
#if gUSB_CDC_SendEventToAppOnCOMOpen_d     
       && mCDCChannelOpen
#endif       
      )
      {
        return TRUE;
      }
    return FALSE;  
  }
 

/*****************************************************************************/
static void CDC_DicInServiceCallback(usbDeviceEvent_t* event)
{
  void (*pfCallBack)(unsigned char const *pBuf);
  uint32_t ccr;
  
  if(event->errors != 0)
  {
    return;    
  }

    /* Mark this one as done, and call the callback. */
  pfCallBack = maCDCTxBufRefTable[mCDCTxBufRefTrailingIndex].pfCallBack;
  ccr = IntDisableAll();
  mCDCTxPendingTxNum--;
  IntRestoreAll(ccr);
  if (pfCallBack)
  {
    if(gCDCDicTxMsgInUsbOueue == FALSE)
    {
      usbMsg_t usbMsg;
      usbMsg.controllerId = event->controllerId;
      usbMsg.eventType = gUsbToAppEvent_SendComplete_c;
      usbMsg.eventParam.sendCompleteEvent.epNum = event->epNum;
      usbMsg.eventParam.sendCompleteEvent.pBuffer = event->pBuffer;
      usbMsg.eventParam.sendCompleteEvent.len = event->len;
      USB_IsrEvent(&usbMsg);
      gCDCDicTxMsgInUsbOueue = TRUE;
    }
  }
  maCDCTxBufLenTable[mCDCTxBufRefTrailingIndex] = 0;
      /* Increment to the next buffer. */
  if (++mCDCTxBufRefTrailingIndex >= NumberOfElements(maCDCTxBufRefTable)) 
  {
    mCDCTxBufRefTrailingIndex = 0;
  }

  /* If there is no more data to send, turn off the transmit interrupt. */
  if (maCDCTxBufLenTable[mCDCTxBufRefTrailingIndex]) 
  {
     uint8_t* pBuf = (uint8_t*)maCDCTxBufRefTable[mCDCTxBufRefTrailingIndex].pBuf;
     index_t bufLen = maCDCTxBufLenTable[mCDCTxBufRefTrailingIndex]; 
     (void)USB_Class_CDC_Interface_DIC_Send_Data(gUsbCdcControllerId_d,pBuf, bufLen); 
  }
}
/*****************************************************************************/
static void CDC_DicOutServiceCallback(usbDeviceEvent_t* event)
{
  uint8_t *pSrc;
  usbPacketSize_t  len;
  uint32_t ccr;
  if(pfCDCRxCallBack != NULL)
  {
    pSrc = event->pBuffer;
    len = event->len;
    while(len)
    {
      ccr = IntDisableAll();
      maCDCRxBuf[mCDCRxBufLeadingIndex] = *pSrc++;
      if (++mCDCRxBufLeadingIndex >= sizeof(maCDCRxBuf)) 
      {
        mCDCRxBufLeadingIndex = 0;
      }
      if (gCDCRxBufferByteCount < sizeof(maCDCRxBuf)) 
      {
        ++gCDCRxBufferByteCount;
      }
      else 
      { 
        /* A new character was received while the buffer is already full. 
        * The oldest character in the buffer has been overwritten. 
        * The trailing index must be also incremented to compensate the overwritten characters
        */
        #ifdef gUSB_Debug_d  
        // debug start
        while(TRUE);  
        //debug end
        #endif 
        if(++mCDCRxBufTrailingIndex >= sizeof(maCDCRxBuf)) 
        {
          mCDCRxBufTrailingIndex = 0;
        }
      }
      IntRestoreAll(ccr);
      len--;                                           
    }
    #if gCDC_EnableFlowControl_d
    ccr = IntDisableAll();
    if (sizeof(maCDCRxBuf) - gCDCRxBufferByteCount >= gCDC_RxFlowControlSkew_d) 
    {
      (void)USB_Class_CDC_Interface_DIC_Recv_Data(event->controllerId, NULL, 0);
    }
    else
    {
      gCDCDicRxEnabled = FALSE;
    }
    IntRestoreAll(ccr);
    #else
    (void)USB_Class_CDC_Interface_DIC_Recv_Data(event->controllerId, NULL, 0);
    #endif
    if(event->len)
    {
      /* Let the application know a byte has been received. */
      if(gCDCDicRxMsgInUsbOueue == FALSE)
      {
        usbMsg_t usbMsg;
        usbMsg.controllerId = event->controllerId;
        usbMsg.eventType = gUsbToAppEvent_DataReceived_c;
        usbMsg.eventParam.dataReceivedEvent.epNum = event->epNum;
        usbMsg.eventParam.dataReceivedEvent.pBuffer = event->pBuffer;
        usbMsg.eventParam.dataReceivedEvent.len = event->len;
        USB_IsrEvent(&usbMsg); 
        //gCDCDicRxMsgInUsbOueue = TRUE; //@SMAC
      }
    }
  }
  else //(pfCDCRxCallBack == NULL)
  {
    (void)USB_Class_CDC_Interface_DIC_Recv_Data(event->controllerId, NULL, 0); 
  }
}

/*****************************************************************************/
static void CDC_App_Callback (
    uint8_t controller_ID,   /* [IN] Controller ID */
    gUsbToAppEvent_t event_type,      /* [IN] value of the event */
    void* val               /* [IN] gives the configuration value */
)
{

  usbMsg_t* pUsbMsg = val;
  (void)(val);
  (void)(controller_ID);
    
  switch(event_type)
  {
    case gUsbToAppEvent_BusReset_c :
           mCDCEnumerated = FALSE;
#if gUSB_CDC_SendEventToAppOnCOMOpen_d            
           mCDCChannelOpen = FALSE;
#endif           
    break;
        
    case gUsbToAppEvent_ConfigChanged_c: 
           if(pUsbMsg->eventParam.configEvent.configValue == 0)
           {
             mCDCEnumerated = FALSE;
           }
           
    break;
        
    case gUsbToAppEvent_EnumComplete_c: 
           mCDCEnumerated = TRUE;
    break;
        
    case gUsbToAppEvent_SendComplete_c:
           if(pUsbMsg->eventParam.sendCompleteEvent.epNum == gCDC_DIC_BulkInEndpoint_d)
           {
               void (*pfCallBack)(unsigned char const *pBuf);    
               unsigned char const *pBuf;
               uint32_t ccr;
               index_t txBufRefTableIndex;
               gCDCDicTxMsgInUsbOueue = FALSE;
               txBufRefTableIndex = mCDCTxBufRefLeadingIndex;
               do
               {
                 pfCallBack = NULL; 
                 ccr = IntDisableAll();
                 if((maCDCTxBufLenTable[txBufRefTableIndex] == 0) && (maCDCTxBufRefTable[txBufRefTableIndex].pfCallBack != NULL))
                 {
                   pfCallBack = maCDCTxBufRefTable[txBufRefTableIndex].pfCallBack;
                   pBuf= maCDCTxBufRefTable[txBufRefTableIndex].pBuf;
                   maCDCTxBufRefTable[txBufRefTableIndex].pfCallBack = NULL;
                 } 
                 IntRestoreAll(ccr);
                 if (++txBufRefTableIndex >= NumberOfElements(maCDCTxBufRefTable)) 
                 {
                   txBufRefTableIndex = 0;
                 }
                 if(pfCallBack != NULL)
                 {
                   pfCallBack(pBuf);
                 }
               }
               while(txBufRefTableIndex != mCDCTxBufRefTrailingIndex);
          
           }
           
    break;
         
    case gUsbToAppEvent_DataReceived_c:
           gCDCDicRxMsgInUsbOueue = FALSE;
           {
             uint32_t ccr;
             void (*pfRxCallBack)(void);
             ccr = IntDisableAll();
             pfRxCallBack = pfCDCRxCallBack;
             IntRestoreAll(ccr);
             if(pfRxCallBack != NULL)
             {
               pfRxCallBack();  
             }
           }
    break;
        
    case gUsbToAppEvent_Resume_c:
    break;
    case gUsbToAppEvent_Suspend_c:
    break;
    case gUsbToAppEvent_ShortSendComplete_c:
    break;
    case gUsbToAppEvent_RequestPending_c:
    break;
    case gUsbToAppEvent_Error_c:
    break;
    case gUsbToAppEvent_GetDataBuff_c:
    break;
    case gUsbToAppEvent_EpStalled_c:
    break;
    case gUsbToAppEvent_EpUnstalled_c:
    break;
    case gUsbToAppEvent_GetTransferSize_c:
    break;
#if gUSB_CDC_SendEventToAppOnCOMOpen_d     
    case gUsbToAppEvent_CDC_COM_Open_c:
           mCDCChannelOpen = TRUE;
    break;
    case gUsbToAppEvent_CDC_COM_Closed_c:
           mCDCChannelOpen = FALSE;
    break;
#endif 
#if gUsbAppNotificationOnSuspend_d    
    case gUsbToAppEvent_Suspend_c:
      if(mCDCEnumerated)
      {
        uint32_t ccr;
        void (*pfSuspendCallBack)(void);
        ccr = IntDisableAll();
        pfSuspendCallBack = pfUSBSuspendCallBack;
         IntRestoreAll(ccr);
        if(pfSuspendCallBack != NULL)
        {
          pfSuspendCallBack();  
        }
      }
      
           
    break;   
#endif      
    
  }
}

#endif //gVirtualCOMPort_d
/* EOF */
