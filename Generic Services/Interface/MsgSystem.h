/************************************************************************************
*
* This is the header file for the memory and message module.
*
* (c) Copyright 2012, Freescale, Inc.  All rights reserved.
*
* No part of this document may be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#ifndef _MSG_SYSTEM_INTERFACE_H_
#define _MSG_SYSTEM_INTERFACE_H_

#include "EmbeddedTypes.h"
#include "PortConfig.h"

/******************************************************************************
*******************************************************************************
* Public macros
*******************************************************************************
******************************************************************************/

#ifndef MsgTracking_d
#define MsgTracking_d 0
#endif

#ifndef gMmNumPools_c
#define gMmNumPools_c                     3         // Number of pools
#endif

// gMmNumPools_c must be in {1,2,3}
#if ((gMmNumPools_c > 3) || (gMmNumPools_c < 1))
#error The number of pools must be al least one (1) and at most three (3)
#endif

#ifndef gTotalSmallMsgs_d
#define gTotalSmallMsgs_d                 5         // Number of control messages
#endif

#ifndef gSmallMsgSize_d
#define gSmallMsgSize_d                  48         // default blocksize for pool0
#endif

#ifndef gTotalBigMsgs_d
#define gTotalBigMsgs_d                  10         // Number of data messages
#endif

#ifndef gBigMsgSize_d
#define gBigMsgSize_d                   180         // default blocksize for pool1
#endif

#ifndef gTotalExtendedBigMsgs_d
#define gTotalExtendedBigMsgs_d           3         // Number of extended big messages
#endif

#ifndef gExtendedBigMsgSize_d
#define gExtendedBigMsgSize_d           252         // default blocksize for pool2
#endif


/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

#if (gMmNumPools_c == 3)
  #if ((gTotalSmallMsgs_d == 0) || (gTotalBigMsgs_d == 0) || (gTotalExtendedBigMsgs_d == 0))
    #error One of the pool contain an invalid number of buffers!
  #endif

  #if ((gSmallMsgSize_d == 0) || (gBigMsgSize_d == 0) || (gExtendedBigMsgSize_d == 0))
    #error One or more pools contain an invalid buffer size!
  #endif

  #if ((gExtendedBigMsgSize_d < gBigMsgSize_d) || (gBigMsgSize_d < gSmallMsgSize_d))
  #error Pool sizes must be sorted ascending!
  #endif

  #ifndef gMaxPacketBufferSize_c
  #define gMaxPacketBufferSize_c         (gBigMsgSize_d)
  #endif

#elif (gMmNumPools_c == 2)
#if ((gTotalSmallMsgs_d == 0) || (gTotalBigMsgs_d == 0))
    #error One of the pool`s dimension is missing!
  #endif

  #if ((gSmallMsgSize_d == 0) || (gBigMsgSize_d == 0))
    #error One or more pools contain an invalid message size!
  #endif

  #if (gBigMsgSize_d < gSmallMsgSize_d)
  #error Pool sizes must be sorted ascending!
  #endif

  #ifndef gMaxPacketBufferSize_c
  #define gMaxPacketBufferSize_c         (gBigMsgSize_d)
  #endif

#else
  #if (gTotalSmallMsgs_d == 0)
  #error The pool has no buffers!
  #endif

  #if (gSmallMsgSize_d == 0)
    #error The pool contain an invalid message size!
  #endif

  #ifndef gMaxPacketBufferSize_c
  #define gMaxPacketBufferSize_c         (gSmallMsgSize_d)
  #endif
#endif

  #ifndef gMaxMsduDataLength_c
  #define gMaxMsduDataLength_c         (118)
  #endif

  #ifndef gMaxRxTxDataLength_c                       // Max length of MPDU/PSDU without CRC
    #ifdef gMAC2006_d
      #define gMaxRxTxDataLength_c     (158)
    #else
      #define gMaxRxTxDataLength_c     (143)
    #endif
  #endif

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

typedef PACKED_STRUCT listHeader_tag
  {
  struct listHeader_tag *pNext;
  struct pools_tag *pParentPool;
  } listHeader_t;

// List anchor with head and tail elements. Used
// for both memory pools and message queues.
typedef PACKED_STRUCT anchor_tag
  {
  listHeader_t *pHead;
  listHeader_t *pTail;
  } anchor_t, msgQueue_t;

// Buffer pool handle. An array of this type is filled
// in by MM_Init(), and used by MM_AllocPool().


typedef PACKED_STRUCT pools_tag
  {
  anchor_t anchor; // MUST be first element in pools_t struct
  uint8_t nextBlockSize;
  uint8_t blockSize;
  uint8_t padding[2];
  } pools_t;

// Buffer pool description. Used by MM_Init()
// for creating the buffer pools.
typedef PACKED_STRUCT poolInfo_tag
  {
  uint8_t poolSize;
  uint8_t blockSize;
  uint8_t nextBlockSize;
  uint8_t padding;
  } poolInfo_t;

typedef PACKED_STRUCT MsgTracking_tag
  {
  uint32_t MsgAddr;        /*Addr of Msg, not that this pointer is 4 byte bigger than
                            the addr in the pool has the header of the msg is 4 bytes */
  uint32_t AllocAddr;      /*Return address of last Alloc made */
  uint32_t FreeAddr;       /*Return address of last Free made */
  uint16_t  AllocCounter;   /*No of time this msg has been allocated */
  uint16_t  FreeCounter;    /*No of time this msg has been freed */
  uint8_t  AllocStatus;    /*1 if currently allocated, 0 if currently free */
  } MsgTracking_t;


/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

extern uint8_t maMacHeap[];
extern pools_t maMmPools[];
extern const poolInfo_t poolInfo[];

/******************************************************************************
*******************************************************************************
* Private function prototypes
*******************************************************************************
******************************************************************************/
void List_ClearAnchor(anchor_t *pAnchor);
void List_AddTail(anchor_t *pAnchor, void *pBlock);
void *List_RemoveHead(anchor_t *pAnchor);

/******************************************************************************
*******************************************************************************
* Public function prototypes
*******************************************************************************
******************************************************************************/
// Free a message
#define MSG_Free(msg) MM_Free(msg)
// Put a message in a queue at the head.
#define MSG_QueueHead(anchor, element) List_AddHead((anchor), (element))
// Get a message from a queue. Returns NULL if no messages in queue.
#define MSG_DeQueue(anchor) List_RemoveHead(anchor)
// Check if a message is pending in a queue. Returns
// TRUE if any pending messages, and FALSE otherwise.
#define MSG_Pending(anchor) ((anchor)->pHead != 0)
#define MSG_InitQueue(anchor) List_ClearAnchor(anchor)

#define MSG_FreeQueue(anchor)  while(MSG_Pending(anchor)) { MSG_Free(MSG_DeQueue(anchor)); }

  // Allocate a message of a certain type
  #define MSG_AllocType(type) MM_Alloc(sizeof(type))
  // Allocate a message of a certain size
  #define MSG_Alloc(size) MM_Alloc(size)
  // Sending a message is equal to calling a Service Access Point function
  // If the sap argument is e.g. MLME, then a function called MLME_SapHandler
  // must exist that takes a message pointer as argument.
  #define MSG_Send(sap, msg)  ((sap##_SapHandler)((void *)(msg)))
  // Put a message in a queue.
  #define MSG_Queue(anchor, element) List_AddTail((anchor), (element))
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

#if (gMmNumPools_c == 1)
  #define gMmPoolSize0_c   (gTotalSmallMsgs_d)
  #define gMmBlockSize0_c  (gSmallMsgSize_d)
  #define gMmPoolSize1_c   (0)
  #define gMmBlockSize1_c  (0)
  #define gMmPoolSize2_c   (0)
  #define gMmBlockSize2_c  (0)
  #define mMmBigMsgPoolIdx_c 0                  // Index of the pool containing BigMsgs (used in MM_Alloc)
#elif (gMmNumPools_c == 2)
  #define gMmPoolSize0_c   (gTotalSmallMsgs_d)
  #define gMmBlockSize0_c  (gSmallMsgSize_d)
  #define gMmPoolSize1_c   (gTotalBigMsgs_d)
  #define gMmBlockSize1_c  (gBigMsgSize_d)
  #define gMmPoolSize2_c   (0)
  #define gMmBlockSize2_c  (0)
  #define mMmBigMsgPoolIdx_c 1                  // Index of the pool containing BigMsgs (used in MM_Alloc)
#else
  #define gMmPoolSize0_c   (gTotalSmallMsgs_d)
  #define gMmBlockSize0_c  (gSmallMsgSize_d)
  #define gMmPoolSize1_c   (gTotalBigMsgs_d)
  #define gMmBlockSize1_c  (gBigMsgSize_d)
  #define gMmPoolSize2_c   (gTotalExtendedBigMsgs_d)
  #define gMmBlockSize2_c  (gExtendedBigMsgSize_d)
  #define mMmBigMsgPoolIdx_c 1                  // Index of the pool containing BigMsgs (used in MM_Alloc)
#endif





#define mRoundUp(a,s) (((a + (s-1)) / s) * s)
#define mMmBlockSize0_c mRoundUp(gMmBlockSize0_c, sizeof(uint8_t *))
#define mMmBlockSize1_c mRoundUp(gMmBlockSize1_c, sizeof(uint8_t *))
#define mMmBlockSize2_c mRoundUp(gMmBlockSize2_c, sizeof(uint8_t *))

// The total number of bytes in each pool including list headers
#define mMmPoolByteSize0_c ((gMmPoolSize0_c) * (mMmBlockSize0_c + sizeof(listHeader_t)))
#define mMmPoolByteSize1_c ((gMmPoolSize1_c) * (mMmBlockSize1_c + sizeof(listHeader_t)))
#define mMmPoolByteSize2_c ((gMmPoolSize2_c) * (mMmBlockSize2_c + sizeof(listHeader_t)))

// Total number of bytes in all pools together
#define mMmTotalPoolSize_c (mMmPoolByteSize0_c + mMmPoolByteSize1_c + mMmPoolByteSize2_c)

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
* Deallocate a memory block by putting it in the corresponding pool of free blocks.
*
* Interface assumptions:
*   None
*
* Return value:
*   None.
*
************************************************************************************/
void MM_Free
  (
  void *pBlock // IN: Block of memory to free
  );

/************************************************************************************
*
* Interface assumptions:
*   None
*
* Return value:
*   None
*
************************************************************************************/
void MM_Init
  (
  uint8_t *pHeap, // IN: Memory heap. Caller must be sure to make this big enough
  const poolInfo_t *pPoolInfo,  // IN: Memory layout information
  pools_t *pPools // OUT: Will be initialized with requested memory pools.
  );

/************************************************************************************
* This function can be used to add a memory block to the specified pool during
* runtime.
*
* Interface assumptions:
*   None
*
* Return value:
*   None
*
************************************************************************************/
void MM_AddToPool
  (
  pools_t *pPool,
  void *pBlock
  );

/************************************************************************************
* This function returns a pointer to the buffer pool which corresponds exactly
* to the size argument. This is used with the MM_AddToPool() function to add buffers
* to the buffer pool.
*
* Interface assumptions:
*   None
*
* Return value:
*   Pointer to the requested buffer pool
*
************************************************************************************/
pools_t *MM_GetPool
  (
  uint8_t size // IN: Exact size which represents the requested buffer pool.
  );

void List_AddHead
  (
  anchor_t *pAnchor, //IN: Anchor of list to add element to
  void *pBlock       //IN: Element to add to head of list
  );

void *List_GetFirst
  (
  anchor_t *pAnchor  //IN: The list where the first element will be returned from.
  );

void *List_GetNext
  (
  void *pBlock  //IN: List element used to reference the next element in the list.
  );

void List_Remove
  (
  anchor_t *pAnchor, //IN: List to remove block from
  void *pPrevBlock,  //IN: Previous List element used to fix the list after modification.
  void *pBlock       //IN: List element used to reference the next element in the list.
  );

void *MM_AllocFast
  (
  void
  );

/************************************************************************************
* Allocate a block from the specified memory pool. The function uses the size
* argument to look up a pool with adequate block sizes.
*
* Interface assumptions:
*   None
*
* Return value:
*   Pointer to the allocated memory block or NULL if no blocks available.
*
************************************************************************************/
void *MM_AllocPool
  (
  pools_t *pPools, // IN: Pool array to allocate from
  uint8_t numBytes // IN: Minimum number of bytes to allocate
  );

/************************************************************************************
* Allocate a block from the memory pool. The function uses the size argument to
* look up a pool with adequate block sizes.
*
* Interface assumptions:
*   None
*
* Return value:
*   Pointer to the allocated memory block or NULL if no blocks available.
*
************************************************************************************/
void *MM_Alloc
  (
  uint8_t numBytes // IN: Minimum number of bytes to allocate
  );

#endif  // #ifndef _MSG_SYSTEM_INTERFACE_H_
