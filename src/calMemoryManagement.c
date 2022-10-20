/**CFile***********************************************************************

  FileName    [calMemoryManagement.c]

  PackageName [cal]

  Synopsis    [Special memory management routines specific to CAL.]

  Description [Functions for managing the system memory using a set of 
              nodeManagers. Each nodeManager manages a set of fixed size
              nodes obtained from a set of pages. When additional memory
              is required, nodeManager obtains a new page from the pageManager.
              The new page is divided into ( PAGE_SIZE/NODE_SIZE ) number of
              nodes.]

  SeeAlso     []

  Author      [Jagesh Sanghavi (sanghavi@eecs.berkeley.edu)
               Rajeev Ranjan   (rajeev@eecs.berkeley.edu)
              ] 
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

  Revision    [$Id: calMemoryManagement.c,v 1.3 1998/09/18 15:34:37 fabio Exp $]

******************************************************************************/
#include "calInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#ifndef HAVE_VALLOC
#define __NOVALLOC__
#else
#if HAVE_VALLOC != 1
#define __NOVALLOC__
#endif
#endif

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int PageManagerExpandStorage(CalPageManager_t * pageManager);
static CalAddress_t * PageAlign(CalAddress_t * p);
static int SegmentToPageList(CalAddress_t * segment, int numPages, CalAddress_t * lastPointer);

/**AutomaticEnd***************************************************************/

/*
 * object: pageManager
 * operations: Init, Quit, AllocPage, FreePage, Print
 */


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Name        [CalPageMangerInit]

  Synopsis    [Initializes a pageManager.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
CalPageManager_t *
CalPageManagerInit(int numPagesPerSegment)
{
  CalPageManager_t *pageManager;
  pageManager = Cal_MemAlloc(CalPageManager_t, 1);
  pageManager->totalNumPages = 0;
  pageManager->numSegments = 0;
  pageManager->numPagesPerSegment = numPagesPerSegment;
  pageManager->maxNumSegments = MAX_NUM_SEGMENTS;
  pageManager->pageSegmentArray 
      = Cal_MemAlloc(CalAddress_t *, pageManager->maxNumSegments);
  pageManager->numPagesArray 
      = Cal_MemAlloc(int, pageManager->maxNumSegments);
  pageManager->freePageList = Cal_Nil(CalAddress_t);
  if(PageManagerExpandStorage(pageManager) == FALSE){
    Cal_MemFree(pageManager->pageSegmentArray);
    Cal_MemFree(pageManager);
    return Cal_Nil(CalPageManager_t);
  }
  return pageManager;
}


/**Function********************************************************************

  Name        [CalPageMangerQuit]

  Synopsis    [Frees pageManager and associated pages.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalPageManagerQuit(
  CalPageManager_t * pageManager)
{
  int i;
  if(pageManager == Cal_Nil(CalPageManager_t)){
    return 1;
  }
  for(i = 0; i < pageManager->numSegments; i++){
    // BUG: double free!
    free(pageManager->pageSegmentArray[i]);
  }
  Cal_MemFree(pageManager->pageSegmentArray);
  Cal_MemFree(pageManager->numPagesArray);
  Cal_MemFree(pageManager);
  return 0;
}


/**Function********************************************************************

  Name        [CalPageMangerPrint]

  Synopsis    [Prints address of each memory segment and address of each page.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalPageManagerPrint(
  CalPageManager_t * pageManager)
{
  int i;
  CalAddress_t *page;
  printf("****************** pageManager ********************\n");
  printf("allocationList:\n");
  for(i = 0; i < pageManager->numSegments; i++){
    page = pageManager->pageSegmentArray[i];
    printf("%lx%c", (CalAddress_t)page, (i+1)%5?' ':'\n');
  }
  printf("\n");
  printf("freePageList:\n");
  i = 0;
  page = pageManager->freePageList;
  while(page){
    printf("%lx%c", (CalAddress_t)page, (i+1)%5?' ':'\n');
    i++;
    page = (CalAddress_t *)*page;
  }
  printf("\n");
}


/**Function********************************************************************

  Name        [CalNodeManagerInit]

  Synopsis    [Initializes a node manager.]

  Description [optional]

  SideEffects []

  SeeAlso     [optional]

******************************************************************************/
CalNodeManager_t *
CalNodeManagerInit(CalPageManager_t * pageManager)
{
  CalNodeManager_t *nodeManager;
  nodeManager = Cal_MemAlloc(CalNodeManager_t, 1);
  nodeManager->freeNodeList = Cal_Nil(CalBddNode_t);
  nodeManager->pageManager = pageManager;
  nodeManager->numPages = 0;
  nodeManager->maxNumPages = 10;
  nodeManager->pageList = Cal_MemAlloc(CalAddress_t *,
                                        nodeManager->maxNumPages);
  return nodeManager;
}


/**Function********************************************************************

  Name        [CalNodeManagerQuit]

  Synopsis    [Frees a node manager.]

  Description [optional]

  SideEffects [The associated nodes are lost.]

  SeeAlso     [optional]

******************************************************************************/
int
CalNodeManagerQuit(CalNodeManager_t * nodeManager)
{
  if(nodeManager == Cal_Nil(CalNodeManager_t)){
    return 1;
  }
  else{
    int i;
    for (i = 0; i < nodeManager->numPages; i++){
      CalPageManagerFreePage(nodeManager->pageManager,
                             nodeManager->pageList[i]);
    }
    Cal_MemFree(nodeManager->pageList);
    Cal_MemFree(nodeManager);
    return 0;
  }
}



/**Function********************************************************************

  Name        [CalNodeManagerPrint]

  Synopsis    [Prints address of each free node.]

  Description [optional]

  SideEffects []

  SeeAlso     [optional]

******************************************************************************/
void
CalNodeManagerPrint(
  CalNodeManager_t * nodeManager)
{
  int i;
  CalBddNode_t *node;
  printf("****************** nodeManager ********************\n");
  printf("freeNodeList:\n");
  i = 0;
  node = nodeManager->freeNodeList;
  while(node){
    printf("%lx%c", (CalAddress_t)node, (i+1)%5?' ':'\n');
    i++;
    node = node->nextBddNode;
  }
  printf("\n");
}


/**Function********************************************************************

  Name        [PageMangerAllocPage]

  Synopsis    [Allocs a new page.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
CalAddress_t *
CalPageManagerAllocPage(CalPageManager_t * pageManager)
{
  CalAddress_t *page;
  char buffer[512];
  if(pageManager->freePageList == Cal_Nil(CalAddress_t)){
    if(PageManagerExpandStorage(pageManager) == FALSE){
      sprintf(buffer,
              "out of memory : Number of pages allocated = %d\n", 
              pageManager->totalNumPages);
      CalBddFatalMessage(buffer);
    }
  }
  page = pageManager->freePageList;
  pageManager->freePageList = (CalAddress_t *)*page;
  return page;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Name        [PageMangerFreePage]

  Synopsis    [Free a page.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalPageManagerFreePage(CalPageManager_t * pageManager, CalAddress_t * page)
{
  *page = (CalAddress_t)(pageManager->freePageList);
  pageManager->freePageList = page;
}


/**Function********************************************************************

  Name        [PageManagerExpandStorage]

  Synopsis    [Allocates a segment of memory to expand the storage managed by
              pageManager. The allocated segment is divided into free pages
              which are linked as a freePageList.]

  Description [optional]

  SideEffects [The size of the segment is stored in one of the fields
              of page manager - numPagesPerSegment. If a memory
              segment of a specific size cannot be allocated, the
              routine calls itself recursively by reducing
              numPagesPerSegment by a factor of 2.]

  SeeAlso     [optional]

******************************************************************************/
static int
PageManagerExpandStorage(CalPageManager_t * pageManager)
{
  CalAddress_t *p;
  CalAddress_t *segment;
  int numUsefulPages;

  int numPagesPerSegment = pageManager->numPagesPerSegment;

#ifdef __NOVALLOC__
  p = (CalAddress_t *) malloc(numPagesPerSegment*PAGE_SIZE);
#else
  p = (CalAddress_t *) valloc(numPagesPerSegment*PAGE_SIZE);
#endif
  /* Just check the page boundary correctness */
  // TODO: This check fails!
  // Cal_Assert(((CalAddress_t)p & ((1 << LG_PAGE_SIZE)-1)) == 0);
  if(p == Cal_Nil(CalAddress_t)){
    numPagesPerSegment = numPagesPerSegment / 2;
    if(numPagesPerSegment < MIN_NUM_PAGES_PER_SEGMENT){
      return FALSE;
    }
    pageManager->numPagesPerSegment = numPagesPerSegment;
    return PageManagerExpandStorage(pageManager);
  }

#ifdef __NOVALLOC__
  /* No need to do it anymore, since I am using valloc */
  /* align the memory segment to a page boundary */
  segment = PageAlign(p);

  /* if memory segment is already page aligned, all pages in the memory
   * segment are useful, otherwise, one page is wasted
   */
  if(segment == p){
    numUsefulPages = numPagesPerSegment;
  }
  else{
    numUsefulPages = numPagesPerSegment - 1;
  }
#else
  segment = p;
  numUsefulPages = numPagesPerSegment;
#endif

  /* Initialize the pages  */
  memset((char *)segment, 0, numUsefulPages*PAGE_SIZE);

  /* Keep track of the number of pages allocated */
  pageManager->totalNumPages += numUsefulPages;

  /* increase the size of the allocation list if neccessary */
  if(pageManager->numSegments == pageManager->maxNumSegments){
    pageManager->maxNumSegments = pageManager->maxNumSegments * 2;
    pageManager->pageSegmentArray = Cal_MemRealloc(CalAddress_t *,
                                                   pageManager->pageSegmentArray,
                                                   pageManager->maxNumSegments);
    pageManager->numPagesArray = Cal_MemRealloc(int,
                                                pageManager->numPagesArray,
                                                pageManager->maxNumSegments);

  }

  pageManager->pageSegmentArray[pageManager->numSegments] = segment;
  pageManager->numPagesArray[pageManager->numSegments++] =
      numUsefulPages;
  
  SegmentToPageList(segment, numUsefulPages, pageManager->freePageList);
  pageManager->freePageList = segment;
  return TRUE;
}


/**Function********************************************************************

  Name        [PageAlign]

  Synopsis    [Return page aligned address greater than or equal to
  the pointer.]

  Description [optional]

  SideEffects []

  SeeAlso     [optional]

******************************************************************************/
static CalAddress_t *
PageAlign(
  CalAddress_t * p)
{
  if((CalAddress_t)p & (PAGE_SIZE - 1)){
    p = (CalAddress_t *)( (CalAddress_t)p >> LG_PAGE_SIZE );
    p = (CalAddress_t *)( ((CalAddress_t)p << LG_PAGE_SIZE) + PAGE_SIZE );
  }
  return p;
}


/**Function********************************************************************

  Name        [SegmentToPageList]

  Synopsis    [Converts a memory segment into a linked list of pages.
              if p is a pointer to a page, *p contains address of the next page
              if p is a pointer to the last page, *p contains lastPointer.]

  Description [optional]

  SideEffects []

  SeeAlso     [optional]

******************************************************************************/
static int
SegmentToPageList(CalAddress_t * segment,
                  int  numPages,
                  CalAddress_t * lastPointer)
{
  int i;
  unsigned long thisPageOffset, nextPageOffset;

  if(numPages > 0){
    for(i = 0; i < numPages - 1; i++){
      thisPageOffset = (i<<LG_PAGE_SIZE)/sizeof(CalAddress_t);
      nextPageOffset = ((i+1)<<LG_PAGE_SIZE)/sizeof(CalAddress_t);
      *(segment + thisPageOffset) =
          (CalAddress_t)(segment + nextPageOffset);
    }
    thisPageOffset = ((numPages - 1)<<LG_PAGE_SIZE)/sizeof(CalAddress_t);
    *(segment + thisPageOffset) = (CalAddress_t)lastPointer;
  }
  else{
    CalBddFatalMessage("out of memory");
  }
  return 0;
}


/*---------------------------------------------------------------------------*/
/* Module Testing 
/*---------------------------------------------------------------------------*/
#ifdef PAGE_MANAGER
main(argc, argv)
int argc;
char **argv;
{
  CalPageManager_t *pageManager;
  CalAddress_t *page, *pageArray[10];
  int i;

  pageManager = CalPageManagerInit();
  CalPageManagerPrint(pageManager);

  page = CalPageManagerAllocPage(pageManager);
  printf("PAGE - %x\n", (CalAddress_t)page);
  CalPageManagerPrint(pageManager);

  PageManagerFreePage(pageManager, page);
  CalPageManagerPrint(pageManager);

  printf("Cal_MemAllocATING PAGES\n");
  for(i = 0; i < 10; i++){
    pageArray[i] = CalPageManagerAllocPage(pageManager);
    CalPageManagerPrint(pageManager);
    printf("\n");
  }
  printf("\n");

  printf("FREEING PAGES\n");
  for(i = 0; i < 10; i++){
    PageManagerFreePage(pageManager, pageArray[i] );
    CalPageManagerPrint(pageManager);
    printf("\n");
  }    
  printf("\n");
  CalPageManagerQuit(pageManager);
}
#endif

#ifdef NODE_MANAGER
main(argc, argv)
int argc;
char **argv;
{
  CalPageManager_t *pageManager;
  CalNodeManager_t *nodeManagerArray[5], *nodeManager;
  CalBddNode_t *node, *nodeArray[5][10];
  int numNodeManagers = 5;
  int numNodes = 10;
  int i,j;
  

  pageManager = CalPageManagerInit();
  /*CalNodeManagerPrint(nodeManager);*/

  printf("Allocating Nodes\n");
  for(i = 0; i < numNodeManagers; i++){
    nodeManagerArray[i] = CalNodeManagerInit(pageManager);
    for (j=0; j < numNodes; j++){
      CalNodeManagerAllocNode(nodeManagerArray[i], nodeArray[i][j]);
      CalBddNodePutRefCount(nodeArray[i][j], i+j);
    }
  }
  for(i = 0; i < numNodeManagers; i++){
    printf("i = %3d\n", i);
    for (j=0; j < numNodes; j++){
      CalBddNodePrint(nodeArray[i][j]);
    }
    printf("\n");
  }

  printf("FREEING NODES\n");
  for(i = 0; i < numNodeManagers; i++){
    for(j = 0; j < numNodes; j++){
      CalNodeManagerFreeNode(nodeManagerArray[i], nodeArray[i][j] );
    }    
    CalNodeManagerQuit(nodeManagerArray[i]);
  }
  CalPageManagerQuit(pageManager);
}
#endif
