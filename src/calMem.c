/**CFile*****************************************************************

  FileName    [calMem.c]

  PackageName [cal]

  Synopsis    [Routines for memory management.]

  Description [Contains allocation, free, resize routines. Also has
  routines for managing records of fixed size.]

  SeeAlso     [optional]

  Author      [Rajeev K. Ranjan (rajeev@eecs.berkeley.edu). Originally
  written by David Long.]

  Copyright   [Copyright (c) 1994-1996 The Regents of the Univ. of California.
  All rights reserved.

  Permission is hereby granted, without written agreement and without license
  or royalty fees, to use, copy, modify, and distribute this software and its
  documentation for any purpose, provided that the above copyright notice and
  the following two paragraphs appear in all copies of this software.

  IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
  DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
  CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
  FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN
  "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO PROVIDE
  MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.]

  Revision   [$Id: calMem.c,v 1.1.1.3 1998/05/04 00:58:59 hsv Exp $]

******************************************************************************/

#include "calMem.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct BlockStruct *Block;
typedef struct BlockStruct Block_t;
typedef struct SegmentStruct *Segment;
typedef struct SegmentStruct Segment_t;
typedef struct ListStruct *List;
typedef struct ListStruct List_t;
typedef struct Cal_RecMgrStruct Cal_RecMgr_t;

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/* #define DEBUG_MEM */
#define MAGIC_COOKIE 0x34f21ab3l
#define MAGIC_COOKIE1 0x432fa13bl
struct SegmentStruct
{
  Cal_Pointer_t baseAddress;
  Cal_Address_t limit;
};

struct BlockStruct
{
  int used;
  int sizeIndex;
  unsigned long dummy;
  Block_t *next;
  Block_t *prev;
  Segment seg;
};
#define HEADER_SIZE ((Cal_Address_t)CAL_ROUNDUP(sizeof(Block_t)))
#define MAX_SIZEINDEX (8*sizeof(Cal_Address_t)-2)
#define MAX_SEG_SIZE ((Cal_Address_t)1 << MAX_SIZEINDEX)
#define MAX_SIZE ((Cal_Address_t)(MAX_SEG_SIZE-HEADER_SIZE))
#define NICE_BLOCK_SIZE ((Cal_Address_t)PAGE_SIZE-CAL_ROUNDUP(sizeof(Block_t)))
#define ALLOC_SIZE NICE_BLOCK_SIZE
#define MIN_ALLOC_SIZEINDEX 15


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/
struct ListStruct
{
  List_t *next;
};

struct Cal_RecMgrStruct
{
  int size;
  int recsPerBlock;
  List free;
  List blocks;
  int numBlocks;
};


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
static Cal_Address_t blockAllocation;
static Block avail[MAX_SIZEINDEX+1];


/* Bogus segment for initialization */

static Segment_t dummySeg={(Cal_Pointer_t)0, (Cal_Address_t)0};


/* Current segment */

static Segment currSeg= &dummySeg;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define SBRK(size) ((Cal_Pointer_t)sbrk((long)(size)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int CeilingLog2(Cal_Address_t i);
static int BlockSizeIndex(Cal_Address_t size);
static void AddToFreeList(Block b);
static Block RemoveFromFreeList(Block b);
static Block Buddy(Block b);
static void TrimToSize(Block b, int sizeIndex);
static void MergeAndFree(Block b);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/



/**Function********************************************************************
******************************************************************************/
void
Cal_MemFatal(char *message)
{
  fprintf(stderr, "Memory management library: error: %s\n", message);
  exit(1);
}

/**Function********************************************************************
******************************************************************************/
Cal_Address_t
Cal_MemAllocation(void)
{
  return (blockAllocation);
}

/**Function********************************************************************
******************************************************************************/
Cal_Pointer_t
Cal_MemGetBlock(Cal_Address_t size)
{
  int i;
  int sizeIndex;
  int allocSizeIndex;
  int newSeg;
  Cal_Address_t allocSize;
  Cal_Pointer_t sbrkRet;
  Block b;

  if ((sizeIndex = BlockSizeIndex(size)) < 0) return ((Cal_Pointer_t)0);

  /* Find smallest free block which is large enough. */
  for (i = sizeIndex; i <= MAX_SIZEINDEX && !avail[i]; ++i);
  if (i > MAX_SIZEINDEX) {
    /* We must get more storage; don't allocate less than */
    /* 2^MIN_ALLOC_SIZE_INDEX */
    if (sizeIndex < MIN_ALLOC_SIZEINDEX) allocSizeIndex=MIN_ALLOC_SIZEINDEX;
    else allocSizeIndex=sizeIndex;
    allocSize=((Cal_Address_t)1 << allocSizeIndex);

    /* Pad current segment to be a multiple of 2^allocSizeIndex in */
    /* length. */
    allocSize += ((currSeg->limit + allocSize - 1) &
                  ~(allocSize - 1)) - currSeg->limit;
    if ((sbrkRet=(Cal_Pointer_t)SBRK(0)) !=
        (Cal_Pointer_t)((Cal_Address_t)currSeg->baseAddress+currSeg->limit) ||
        allocSize+currSeg->limit > MAX_SEG_SIZE) {

      /* Segment is too large or someone else has moved the break. */
      /* Pad to get to appropriate boundary. */
      allocSize=CAL_ROUNDUP((Cal_Address_t)sbrkRet)-(Cal_Address_t)sbrkRet;

      /* Pad allocation request with storage for new segment */
      /* information and indicate that a new segment must be */
        /* created. */
      allocSize+=((Cal_Address_t)1 << allocSizeIndex)+CAL_ROUNDUP(sizeof(Segment_t));
      newSeg=1;
    }
    else newSeg=0;
    sbrkRet=(Cal_Pointer_t)SBRK(allocSize);
    if (sbrkRet == (Cal_Pointer_t) -1) Cal_MemFatal("Cal_MemGetBlock: allocation failed");
    blockAllocation += allocSize;
    if (newSeg){
      currSeg = (Segment) CAL_ROUNDUP((Cal_Address_t)sbrkRet);
      currSeg->baseAddress=(Cal_Pointer_t)((Cal_Address_t)currSeg+CAL_ROUNDUP(sizeof(Segment_t)));
      currSeg->limit=0;
      /* Readjust allocation size. */
      allocSize=(1l << allocSizeIndex);
      }
    /* Carve allocated space up into blocks and add to free lists. */
    while (allocSize){
      size = allocSize - (allocSize & (allocSize-1));
      b = (Block) ((Cal_Address_t)currSeg->baseAddress+currSeg->limit);
      b->sizeIndex = CeilingLog2(size);
      b->seg = currSeg;
      AddToFreeList(b);
        currSeg->limit += size;
        allocSize -= size;
    }
    /* Find free block of appropriate size. */
    for (i=sizeIndex; i <= MAX_SIZEINDEX && !avail[i]; ++i);
  }
  b = RemoveFromFreeList(avail[i]);
  TrimToSize(b, sizeIndex);
  return ((Cal_Pointer_t)((Cal_Address_t)b + HEADER_SIZE));
}


/**Function********************************************************************
******************************************************************************/void
Cal_MemFreeBlock(Cal_Pointer_t p)
{
  Block b;

  if (!p) return;
  b = (Block) ((Cal_Address_t)p-HEADER_SIZE);
  if (!b->used) Cal_MemFatal("Cal_MemFreeBlock: block not in use");
  if (b->sizeIndex < 0 || b->sizeIndex > MAX_SIZEINDEX) Cal_MemFatal("Cal_MemFreeBlock: invalid block header");
  MergeAndFree(b);
}



/**Function********************************************************************
******************************************************************************/
Cal_Pointer_t
Cal_MemResizeBlock(Cal_Pointer_t p, Cal_Address_t newSize)
{
  int newSizeIndex;
  Block b;
  Block bb;
  Cal_Pointer_t q;
  Cal_Address_t oldSize;

  if (!p) return (Cal_MemGetBlock(newSize));
  b = (Block) ((Cal_Address_t)p - HEADER_SIZE);
  if (!b->used) Cal_MemFatal("Cal_MemResizeBlock: block not in use");
  if (b->sizeIndex < 0 || b->sizeIndex > MAX_SIZEINDEX){
    Cal_MemFatal("Cal_MemResizeBlock: invalid block header");
  }
  if ((newSizeIndex = BlockSizeIndex(newSize)) < 0){
    Cal_MemFreeBlock(p);
    return ((Cal_Pointer_t)0);
  }
  if (b->sizeIndex >= newSizeIndex){
    /* Shrink block. */
    TrimToSize(b, newSizeIndex);
    return (p);
  }
  oldSize=(1l << b->sizeIndex) - HEADER_SIZE;
  /* Try to expand by adding buddies at higher addresses. */
  for (bb=Buddy(b);
       bb && (Cal_Address_t)b < (Cal_Address_t)bb && !bb->used && bb->sizeIndex == b->sizeIndex;
       bb=Buddy(b)) {
    RemoveFromFreeList(bb);
    if (++(b->sizeIndex) == newSizeIndex) return (p);
  }
  /* Couldn't expand all the way to needed size; allocate a new block */
  /* and move the contents of the old one. */
  q = (Cal_Pointer_t) Cal_MemGetBlock(newSize);
  Cal_MemCopy(q, p, oldSize);
  MergeAndFree(b);
  return (q);
}

/**Function********************************************************************
******************************************************************************/
Cal_Pointer_t
Cal_MemNewRec(Cal_RecMgr mgr)
{
  int i;
  Cal_Pointer_t p;
  List new;

  if (!mgr->free) {
    /* Allocate a new block. */
    new = (List) Cal_MemGetBlock(ALLOC_SIZE);
    mgr->numBlocks++;
    new->next=mgr->blocks;
    mgr->blocks=new;
    mgr->free=(List)((Cal_Address_t)new+CAL_ROUNDUP(sizeof(List_t)));
    p=(Cal_Pointer_t)(mgr->free);
    /* Carve the block into pieces. */
    for (i=1; i < mgr->recsPerBlock; ++i) {
      ((List)p)->next=(List)((Cal_Address_t)p+mgr->size);
#if defined(DEBUG_MEM)
      if (mgr->size >= sizeof(long)+sizeof(List_t))
        *(long *)(sizeof(List_t)+(Cal_Address_t)p)=MAGIC_COOKIE;
#endif
      p=(Cal_Pointer_t)((Cal_Address_t)p+mgr->size);
    }
    ((List)p)->next=0;
#if defined(DEBUG_MEM)
    if (mgr->size >= sizeof(long)+sizeof(List_t)){
      *(long *)(sizeof(List_t)+(Cal_Address_t)p)=MAGIC_COOKIE;
    }
#endif
  }
  new = mgr->free;
#if defined(DEBUG_MEM)
  if (mgr->size >= sizeof(long)+sizeof(List_t)){
    if (*(long *)(sizeof(List_t)+(Cal_Address_t)new) != MAGIC_COOKIE)
      fprintf(stderr, "record at 0x%lx may be in use\n", (Cal_Address_t)new);
    else
      *(long *)(sizeof(struct
                       list_)+(Cal_Address_t)new)=MAGIC_COOKIE1;
  }
#endif
  mgr->free = mgr->free->next;
  return ((Cal_Pointer_t)new);
}


/**Function********************************************************************
******************************************************************************/
void
Cal_MemFreeRec(Cal_RecMgr mgr, Cal_Pointer_t rec)
{
#if defined(DEBUG_MEM)
  if (mgr->size >= sizeof(long)+sizeof(List_t))
    if (*(long *)(sizeof(List_t)+(Cal_Address_t)rec) == MAGIC_COOKIE)
      fprintf(stderr, "record at 0x%lx may already be freed\n", (Cal_Address_t)rec);
#endif
  ((List)rec)->next=mgr->free;
#if defined(DEBUG_MEM)
  if (mgr->size >= sizeof(long)+sizeof(List_t))
    *(long *)(sizeof(List_t)+(Cal_Address_t)rec)=MAGIC_COOKIE;
#endif
  mgr->free=(List)rec;
}



/**Function********************************************************************
******************************************************************************/
Cal_RecMgr
Cal_MemNewRecMgr(int size)
{
  Cal_RecMgr mgr;

  if (size < sizeof(List_t)) size=sizeof(List_t);
  size=CAL_ROUNDUP(size);
  if (size > ALLOC_SIZE-CAL_ROUNDUP(sizeof(List_t)))
    Cal_MemFatal("Cal_MemNewRecMgr: record size too large");
  mgr = (Cal_RecMgr)Cal_MemGetBlock((Cal_Address_t)sizeof(Cal_RecMgr_t));
  mgr->size=size;
  mgr->recsPerBlock=(ALLOC_SIZE-CAL_ROUNDUP(sizeof(List_t)))/size;
  mgr->free=0;
  mgr->blocks=0;
  mgr->numBlocks = 0;
  return (mgr);
}

/**Function********************************************************************
******************************************************************************/
void
Cal_MemFreeRecMgr(Cal_RecMgr mgr)
{
  List p, q;
  for (p=mgr->blocks; p; p=q){
    q=p->next;
    Cal_MemFreeBlock((Cal_Pointer_t)p);
  }
  Cal_MemFreeBlock((Cal_Pointer_t)mgr);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************
  Compute the binary logarithm (rounded up).
******************************************************************************/
static int
CeilingLog2(Cal_Address_t i)
{
  Cal_Address_t j;
  int result;

  for (result=0, j=1; j < i; ++result, j*=2);
  return (result);
}

/**Function********************************************************************
  BlockSizeIndex(size) return the coded size for a block.
******************************************************************************/
static int
BlockSizeIndex(Cal_Address_t size)
{
  if (size < 1)
    return (-1);
  if (size > MAX_SIZE)
    Cal_MemFatal("BlockSizeIndex: block size too large");
  else
    size+=HEADER_SIZE;
  return (CeilingLog2(size));
}


/**Function********************************************************************
  AddToFreeList(b) adds b to the appropriate free list.
******************************************************************************/
static void
AddToFreeList(Block b)
{
  int i;

  i=b->sizeIndex;
  if (!avail[i]){
      b->next=b;
      b->prev=b;
      avail[i]=b;
  }
  else {
    b->next=avail[i]->next;
    avail[i]->next->prev=b;
    avail[i]->next=b;
    b->prev=avail[i];
  }
  b->used=0;
}


/**Function********************************************************************
  RemoveFromFreeList(b) removes b from the free list which it is on.
******************************************************************************/
static Block
RemoveFromFreeList(Block b)
{
  int i;

  i=b->sizeIndex;
  if (b->next == b)
    avail[i]=0;
  else {
    b->next->prev=b->prev;
    b->prev->next=b->next;
    if (avail[i] == b) avail[i]=b->next;
  }
  b->used=1;
  return (b);
}


/**Function********************************************************************
  Buddy(b) returns the Buddy block of b, or null if there is no  Buddy.
******************************************************************************/

static Block
Buddy(Block b)
{
  Cal_Address_t Buddy_offset;

  Buddy_offset=(Cal_Address_t)(((Cal_Address_t)b-(Cal_Address_t)b->seg->baseAddress) ^
                               ((Cal_Address_t)1 << b->sizeIndex));
  if (Buddy_offset < b->seg->limit)
    return ((Block)((Cal_Address_t)b->seg->baseAddress+Buddy_offset));
  else
    return ((Block)0);
}


/**Function********************************************************************
  TrimToSize(b, sizeIndex) repeatedly splits b until it has the indicated size.
  Blocks which are split off are added to the appropriate free list.
******************************************************************************/
static void
TrimToSize(Block b, int sizeIndex)
{
  Block bb;

  while (b->sizeIndex > sizeIndex) {
    b->sizeIndex--;
    bb=Buddy(b);
    bb->sizeIndex=b->sizeIndex;
    bb->seg=b->seg;
    AddToFreeList(bb);
  }
}


/**Function********************************************************************
  MergeAndFree(b) repeatedly merges b its Buddy until b has no Buddy or the
  Buddy isn't free, then adds the result to the appropriate free list.
******************************************************************************/
static void
MergeAndFree(Block b)
{
  Block bb;

  for (bb=Buddy(b); bb && !bb->used && bb->sizeIndex == b->sizeIndex;
       bb=Buddy(b)) {
    RemoveFromFreeList(bb);
    if ((Cal_Address_t)bb < (Cal_Address_t)b) b=bb;
    b->sizeIndex++;
  }
  AddToFreeList(b);
}
