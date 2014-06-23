/************************************************************************************
* This module implements the memory allocation, list, and message modules. The memory
* allocation is build around N (1-3) pools with various memory allocation unit (block)
* sizes. Each pool consists of an anchor with head and tail pointers. The memory blocks
* are all linked to the anchor using a single chained list. Thus each block has a next
* pointer. The user of the functions in this module never has to be concerned with the
* list overhead since this is handled transparently. The block pointer which the user
* receives when allocating memory is pointing to the address after the next-pointer.
*
* FIFO Queues are implemented using the same list functions as used by the memory
* (de)allocation functions. The queue data object is simply an anchor (anchor_t).
* List_AddTail is used for putting allocated blocks on the queue, and List_RemoveHead
* will detach the block from the queue. Before using a queue anchor it must have been
* initialized with List_ClearAnchor. No extra header is required in order to put a
* block in a queue. However, messages should contain type information beside the
* message data so that the message handler at the receiver can reckognize the message.
*
* Messages are sent by allocating a block using MM_Alloc, and using the MSG_Send macro
* to call the Service Access Point (SAP) of the receiver. If the SAP handles specific
* messages synchronously (returns with result immideately) then the block may be
* allocated on the stack of the calling function. The message types which allows this
* are specified in the design documents.
*
* (c) Copyright 2012, Freescale, Inc.  All rights reserved.
*
* No part of this document may be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/

#include "MsgSystem.h"
#include "Interrupt.h"

#if MsgTracking_d
#include "FunctionLib.h"
#endif

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
  #define MmAssert(condition)
  #define MmDebug_InitPool(pool)
  #define MmDebug_InitBlock(block)
  #define MM_DEBUG_LOG(fName)

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
// The heap for MAC, NWK and application memory blocks.
uint8_t maMacHeap[mMmTotalPoolSize_c];

// Memory pool info and anchors.
pools_t maMmPools[gMmNumPools_c];

// Const array used during initialization. Describes the memory layout.
// Pools must occur in the table in ascending order according to their size.
const poolInfo_t poolInfo[gMmNumPools_c] = {
  {gMmPoolSize0_c, mMmBlockSize0_c, mMmBlockSize1_c, 0},
#if gMmNumPools_c > 1
  {gMmPoolSize1_c, mMmBlockSize1_c, mMmBlockSize2_c, 0},
#if gMmNumPools_c > 2
  {gMmPoolSize2_c, mMmBlockSize2_c, 0, 0}
#endif // gMmNumPools_c > 2
#endif // gMmNumPools_c > 1
};


/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

const pools_t *pMacPool = NULL;
uint8_t gFreeMessagesCount;

#if MsgTracking_d
uint8_t NoOfWrongAddrs = 0;
uint32_t mLinkRegister; /* Used by Message Tracking feature*/
const uint8_t TotalNoOfMsgs_c = (gTotalBigMsgs_d + gTotalSmallMsgs_d + gTotalExtendedBigMsgs_d);
MsgTracking_t MsgTrackingArray [(gTotalBigMsgs_d + gTotalSmallMsgs_d + gTotalExtendedBigMsgs_d)] = {0};
#endif

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
  #define MmDebug_Alloc(pPool, numBytes)
  #define MmDebug_AllocFail(pPool, numBytes)
  #define MmDebug_Free(pPool)

int8_t MM_GetMsgIndex(const uint32_t BufferAddr);
bool_t MM_UpdateMsgTracking(const void *pBlock, const bool_t Alloc);
void   MM_InitMsgTracking(void);
void   MM_AddMsgToTrackingArray(uint8_t Index, uint32_t Addr);


/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/************************************************************************************
* This function initializes the message module private variables. Must be
* called at boot time, or if device is reset. Currently the module supports
* up to 3 memory pools.
*
* The gMmPoolSize*_c constants determine the number of blocks in a pool, and
* gMmBlockSize*_c is the number of bytes in each block for the corresponding pool.
* The number of bytes is rounded up to a value so that each block is aligned to
* a machine dependant boundary in order to avoid bus errors during pool access.
*
* The total amount of heap required is given by the constant mMmTotalPoolSize_c.
*
************************************************************************************/
void MM_Init
  (
  uint8_t *pHeap,               // IN: Memory heap. Caller must be sure to make this big enough
  const poolInfo_t *pPoolInfo,  // IN: Memory layout information
  pools_t *pPools               // OUT: Will be initialized with requested memory pools.
  )
{
  uintn8_t poolN;
  uint8_t nBuffer = 0;
  gFreeMessagesCount = 0;

  MM_DEBUG_LOG(MM_Init);
  MM_InitMsgTracking();

  pMacPool = &maMmPools[mMmBigMsgPoolIdx_c];


  for(;;) {
    poolN = pPoolInfo->poolSize;

    List_ClearAnchor(&pPools->anchor);

    MmDebug_InitPool(pPools);

    while(poolN) {
      MmDebug_InitBlock(pHeap);
        // Add block to list of free memory.
      ((listHeader_t *)pHeap)->pParentPool = pPools;
      MM_AddToPool(pPools, pHeap);

      gFreeMessagesCount++;

      MM_AddMsgToTrackingArray(nBuffer, (uint32_t) pHeap  + 8);
      nBuffer++;

        // Add block size (without list header)
      pHeap += pPoolInfo->blockSize + sizeof(listHeader_t);
      poolN--;
    }

    pPools->blockSize = pPoolInfo->blockSize;

    pPools->nextBlockSize = pPoolInfo->nextBlockSize;
    if(pPools->nextBlockSize == 0)
      break;

    pPools++;
    pPoolInfo++;
  }
}


/************************************************************************************
* Initialize a list anchor with a NULL list header. Used for preparing an anchor for
* first time use
*
************************************************************************************/
void List_ClearAnchor
  (
  anchor_t *pAnchor // IN: Anchor of list to reset
  )
{
  pAnchor->pHead = NULL;
}

/************************************************************************************
* Links a list element to the tail of the list given by the anchor argument.
* This function is amongst other useful for FIFO queues if combined with
* List_RemoveHead.
*
************************************************************************************/
void List_AddTail
  (
  anchor_t *pAnchor, //IN: Anchor of list to add element to
  void *pBlock       //IN: Element to add to tail of list
  )
{
  uint32_t saveInt;

  MmAssert(pAnchor && pBlock);

  MM_DEBUG_LOG(List_AddTail);
  if ((NULL == pBlock) || (NULL == pAnchor))
    return;

    // Get pointer to header portion of the list element
  pBlock = ((listHeader_t *)pBlock)-1;
    // The new element must terminate the list.
  ((listHeader_t *)pBlock)->pNext = NULL;

  saveInt = IntDisableAll();

    // Update current tail element with pointer to new element
  if(pAnchor->pHead) {
      // Make old tail point to new element.
    pAnchor->pTail->pNext = pBlock;
  }
  else {
      // Nothing in list yet, so initialize head pointer.
    pAnchor->pHead = pBlock;
  }
    // Let new element be the tail.
  pAnchor->pTail = pBlock;

  IntRestoreAll(saveInt);
}


/************************************************************************************
*
* Links a list element to the head of the list given by the anchor argument.
* Useful for FILO buffers (push/pop stacks).
*
************************************************************************************/
void List_AddHead
  (
  anchor_t *pAnchor, //IN: Anchor of list to add element to
  void *pBlock       //IN: Element to add to head of list
  )
{
  listHeader_t *pNewBlock;
  uint32_t saveInt;

  MmAssert(pAnchor && pBlock);
  MM_DEBUG_LOG(List_AddHead);

  if (NULL == pBlock || NULL == pAnchor)
	return;

    // Get pointer to header portion of the list element
  pNewBlock = ((listHeader_t *)pBlock)-1;

  saveInt = IntDisableAll();

   // If list is empty, then the element will also be the tail of the list.
  if(pAnchor->pHead == NULL) {
    pAnchor->pTail = pNewBlock;
  }
    // Let the new element point to the old head element (NULL if list is empty).
  pNewBlock->pNext = pAnchor->pHead;
    // The new element becomes the new list header.
  pAnchor->pHead = pNewBlock;

  IntRestoreAll(saveInt);
}


/************************************************************************************
* Unlinks a list element from the head of the list given by the anchor argument.
*
************************************************************************************/
void *List_RemoveHead
  (
  anchor_t *pAnchor  //IN: Anchor of list to remove head from
  )
{
  listHeader_t *pBlock;
  uint32_t saveInt;

  MmAssert(pAnchor);
  MM_DEBUG_LOG(List_RemoveHead);
  if (NULL == pAnchor)
	return NULL;

  saveInt = IntDisableAll();

    // If any head element, then remove it by setting
    // the head to the next element in the list.
    // If we just removed the last element, then the
    // tail ponter must be set to 0.
  pBlock = pAnchor->pHead;
  if(pBlock) {
    pAnchor->pHead = pBlock->pNext;
  }

  IntRestoreAll(saveInt);

    // Skip header, and return pointer to data area if pointer is not NULL.
  return pBlock ? pBlock+1 : pBlock;
}


/************************************************************************************
* Removes a list element from anywhere in the list.
*
* This function should be used somewhat like in the following example since it
* requires a pointer to the previous block relative to the current block:
*
************************************************************************************/
void List_Remove
  (
  anchor_t *pAnchor, //IN: List to remove block from
  void *pPrevBlock,  //IN: Previous List element used to fix the list after modification.
  void *pBlock       //IN: List element used to reference the next element in the list.
  )
{
  uint32_t saveInt;

#define prevBlock (((listHeader_t *)pPrevBlock)-1)

  listHeader_t *nextBlock =  (listHeader_t *)pBlock;

  if (NULL == pBlock || NULL == pAnchor)
	return;
  MM_DEBUG_LOG(List_Remove);

  saveInt = IntDisableAll();

    // Use current element as temp var to point at next element.
  nextBlock = ((nextBlock)-1)->pNext;

  if(pPrevBlock) {
      // Let previous block point to next block.
    prevBlock->pNext = nextBlock;
      // Adjust tail pointer if pBlock was the last element in the list.
    if(prevBlock->pNext == 0) {
      pAnchor->pTail = prevBlock;
    }
  }
  else {
      // No previous element => remove from head.
      // Set head to point at second element to remove pBlock from list.
    pAnchor->pHead = nextBlock;
  }

  IntRestoreAll(saveInt);

#undef prevBlock
}


/************************************************************************************
* Returns a pointer to the first element in the list.
*
************************************************************************************/
void *List_GetFirst
  (
  anchor_t *pAnchor  //IN: The list where the first element will be returned from.
  )
{
  listHeader_t *pBlock;
  uint32_t saveInt;

  MmAssert(pAnchor);

  if (NULL == pAnchor)
    return NULL;

  saveInt = IntDisableAll();

  pBlock = pAnchor->pHead;

  IntRestoreAll(saveInt);

    // Skip header, and return pointer to data area if pointer is not NULL.
  return pBlock ? pBlock+1 : pBlock;
}

/************************************************************************************
* Returns a pointer to the next list element in the list.
*
************************************************************************************/
void *List_GetNext
  (
  void *pBlock  //IN: List element used to reference the next element in the list.
  )
{
  uint32_t saveInt;

  MmAssert(pBlock);

  if (NULL == pBlock)
    return NULL;

  saveInt = IntDisableAll();

  pBlock = (((listHeader_t *)pBlock)-1)->pNext;

  IntRestoreAll(saveInt);

    // Skip header, and return pointer to data area if pointer is not NULL.
  return pBlock ? ((listHeader_t *)pBlock)+1 : pBlock;
}


/************************************************************************************
* This function can be used to add a memory block to the specified pool during
* runtime.
************************************************************************************/
void MM_AddToPool(pools_t *pPool, void *pBlock)
{
  ((listHeader_t *)pBlock)->pParentPool = pPool;
  pBlock = ((listHeader_t *)pBlock) + 1;
  List_AddTail(&pPool->anchor, ((listHeader_t *)pBlock));
}


/************************************************************************************
* This function returns a pointer to the buffer pool which corresponds exactly to
* the size argument. This is used with the MM_AddToPool() function to add buffers
* to the buffer pool.
*
************************************************************************************/
pools_t *MM_GetPool(uint8_t size)
{
  pools_t *pPools = maMmPools;

  while(size) {
    if(size == pPools->blockSize)
      return pPools;
    if(pPools->nextBlockSize == 0)
      return NULL;
    pPools++;
  }
  return NULL;
}

/************************************************************************************
* Allocate a block from the specified memory pool. The function uses the size
* argument to look up a pool with adequate block sizes.
*
************************************************************************************/
void *MM_AllocPool
  (
  pools_t *pPools, // IN: Pool to allocate from
  uint8_t numBytes // IN: Minimum number of bytes to allocate
  )
{
  listHeader_t *pBlock;

  while(numBytes)
  {
    if(numBytes <= pPools->blockSize)
    {
      /* Do not allow to allocate the last message from the MAC Big Message Pool.
         The last message is only accessible via MM_AllocFast() */
      if ((pPools == pMacPool) && (pPools->anchor.pHead == pPools->anchor.pTail))
        pBlock = NULL;
      else
        pBlock = List_RemoveHead(&pPools->anchor);

      if(NULL != pBlock)
      {

        gFreeMessagesCount--;

        (void)MM_UpdateMsgTracking(pBlock, TRUE);

        return pBlock;
      }
      else
          // No more blocks of that size, try next size.
        numBytes = pPools->nextBlockSize;
    }
      // Try next pool
    if(pPools->nextBlockSize)
      pPools++;
    else
      break;
  }

  MmDebug_AllocFail(pPools, numBytes);

  return NULL;
}


/************************************************************************************
* Allocate a block from the MAC memory pool. The function uses the size argument to
* look up a pool with adequate block sizes.
*
************************************************************************************/
void *MM_Alloc
  (
  uint8_t numBytes // IN: Minimum number of bytes to allocate
  )
{
#if MsgTracking_d
  /* Save the Link Register */
  volatile uint32_t savedLR;

  __asm("push {r1}  ");
  __asm("push {r14} ");
  __asm("pop  {r1} ");
  __asm("str  r1, [SP, #4]");
  __asm("pop {r1}");
  mLinkRegister = savedLR;
#endif
  return MM_AllocPool(maMmPools, numBytes);
}


/************************************************************************************
* Deallocate a memory block by putting it in the corresponding pool of free blocks.
*
************************************************************************************/
void MM_Free
  (
  void *pBlock // IN: Block of memory to free
  )
{
#if MsgTracking_d
  /* Save the Link Register */
  volatile uint32_t savedLR;

  __asm("push {r1}  ");
  __asm("push {r14} ");
  __asm("pop  {r1} ");
  __asm("str  r1, [SP, #4]");
  __asm("pop {r1}");
  mLinkRegister = savedLR;
#endif
  MM_DEBUG_LOG(MM_Free);

    // Freeing a NULL pointer will be ignored.
  if(pBlock) {
    pools_t *pParentPool = (((listHeader_t *)pBlock)-1)->pParentPool;

    gFreeMessagesCount++;

    (void)MM_UpdateMsgTracking(pBlock, FALSE);
    List_AddTail(&pParentPool->anchor, pBlock);
  }
}


/************************************************************************************
* Allocate a block from the largest memory pool. This function should only be called
* from interrupt context since it is not protected. First the function tries to
* allocate from the MAC private pool. If it fails it tries to allocate from the
* public buffer pool.
*
************************************************************************************/

void *MM_AllocFast(void)
{
#if MsgTracking_d
  volatile uint32_t savedLR;
#endif
  anchor_t *pAnchor;
  listHeader_t *pBlock;

#if MsgTracking_d
  /* Save the Link Register */
  __asm("push {r1}  ");
  __asm("push {r14} ");
  __asm("pop  {r1}  ");
  __asm("str  r1, [SP, #4]");
  __asm("pop  {r1}  ");
  mLinkRegister = savedLR;
#endif
  pAnchor = (anchor_t *)&(pMacPool->anchor);
  pBlock = pAnchor->pHead;

  if (NULL == pBlock)
    return pBlock;


  pAnchor->pHead = pBlock->pNext;

  MmDebug_Alloc(((listHeader_t *)pBlock)->pParentPool, 0);

  gFreeMessagesCount--;

  (void)MM_UpdateMsgTracking(pBlock + 1, TRUE);

  return pBlock + 1;
}

/************************************************************************************
* Initialize Msg tracking array. It is called by the Msg system when the MAC is reset
*
* Interface assumptions:
*   None
*
* Return value:
*   None.
*
************************************************************************************/
void MM_InitMsgTracking(void) {
#if MsgTracking_d
   FLib_MemSet16((uint8_t* )&MsgTrackingArray[0],0,(sizeof(MsgTrackingArray[0]) * TotalNoOfMsgs_c));
   NoOfWrongAddrs = 0;
#endif
}
/************************************************************************************
* Initialize Msg tracking array a current index with a Msg address.
* This functions is called for once for every Msg when the MAC is reset.
*
* Interface assumptions:
*   None
*
* Return value:
*   None.
*
************************************************************************************/
void   MM_AddMsgToTrackingArray(uint8_t Index, uint32_t Addr) {
#if MsgTracking_d
      MsgTrackingArray[Index].MsgAddr = Addr;
#else
  /*To prevent compiler warinngs:*/
  (void) Index;
  (void) Addr;
#endif
}
/************************************************************************************
* This function finds the index of a Msg in the Tracking array
* -1 is returned if Msg not found and the NoOfWrongAddrs is incremented.
*
* Interface assumptions:
*   None
*
* Return value:
*   Index of message in tracking array
*
************************************************************************************/
int8_t MM_GetMsgIndex(const uint32_t MsgAddr)
{
#if MsgTracking_d
  uint8_t i;
  /*Search through the Tracking array*/
  for (i = 0; i < TotalNoOfMsgs_c; i++)
  {
    /*If Msg Addr found then exit with Index*/
    if (MsgAddr == MsgTrackingArray[i].MsgAddr)
      return i;
  }
  /*Msg addr not found, increment error counter and return*/
  NoOfWrongAddrs++;
  return -1;
#else
  /*To prevent compiler warinngs:*/
 (void) MsgAddr;
 return -1;
#endif
}

/************************************************************************************
* Provide the mLinkRegister value to Message Tracking module
************************************************************************************/
#if MsgTracking_d
uint32_t MM_GetLinkRegister(void)
{
  return mLinkRegister;
}
#endif

/************************************************************************************
* This function is called when ever a Msg is freed or allocated and updates
* the tracking information for that particular Msg
*
*
* Interface assumptions:
*   None
*
* Return value:
*   Success status
*
************************************************************************************/
bool_t MM_UpdateMsgTracking(const void *pBlock, const bool_t Alloc)
{
#if MsgTracking_d
  int8_t index = MM_GetMsgIndex((uint32_t)pBlock);
  if (index >= 0)
  {
    if (MsgTrackingArray[index].AllocStatus == Alloc)
    {
     return FALSE;
    }

    MsgTrackingArray[index].AllocStatus = Alloc;

    /*Update MsgTracking array counters and return address of alloc or free */
    if (Alloc) {
      MsgTrackingArray[index].AllocCounter++;
      MsgTrackingArray[index].AllocAddr = MM_GetLinkRegister();
    }
    else {
      MsgTrackingArray[index].FreeCounter++;
      MsgTrackingArray[index].FreeAddr = MM_GetLinkRegister();
    }
    return TRUE;

  }
  else
    return FALSE;
#else
  /*To prevent compiler warinngs:*/
  (void) pBlock;
  (void) Alloc;
  return TRUE;
#endif
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
