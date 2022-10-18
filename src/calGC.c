/**CFile***********************************************************************

  FileName    [calGC.c]

  PackageName [cal]

  Synopsis    [Garbage collection routines]

  Description [optional]

  SeeAlso     [optional]

  Author      [Jagesh Sanghavi (sanghavi@eecs.berkeley.edu)
               Rajeev Ranjan (rajeev@eecs.berkeley.edu)]

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

  Revision    [$Id: calGC.c,v 1.2 1998/09/16 16:08:40 ravi Exp $]

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

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int CeilLog2(int number);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Sets the garbage collection mode, 0 means the garbage
  collection should be turned off, 1 means garbage collection should
  be on.]

  Description [Sets the garbage collection mode, 0 means the garbage
  collection should be turned off, 1 means garbage collection should
  be on.]

  SideEffects [None.]

******************************************************************************/
void
Cal_BddSetGCMode(
  Cal_BddManager bddManager,
  int  gcMode)
{
  bddManager->gcMode = gcMode;
}


/**Function********************************************************************

  Synopsis    [Invokes the garbage collection at the manager level.]

  Description [For each variable in the increasing id free nodes with reference
  count equal to zero freeing a node results in decrementing reference count of
  then and else nodes by one.]

  SideEffects [None.]

******************************************************************************/
int
Cal_BddManagerGC(Cal_BddManager bddManager)
{
  Cal_BddIndex_t index;
  Cal_BddId_t id;
  int numNodesFreed;
  /* unsigned long origNodes = bddManager->numNodes; */
  
  if (bddManager->numPeakNodes < (bddManager->numNodes +
                                  bddManager->numForwardedNodes)){
    bddManager->numPeakNodes = bddManager->numNodes +
        bddManager->numForwardedNodes ;
  }
  
  CalHashTableGC(bddManager, bddManager->uniqueTable[0]);
  for(index = 0; index < bddManager->numVars; index++){
    id = bddManager->indexToId[index];
    numNodesFreed = CalHashTableGC(bddManager, bddManager->uniqueTable[id]);
    bddManager->numNodes -= numNodesFreed;
    bddManager->numNodesFreed += numNodesFreed;
  }
  /* Free the cache entries related to unused BDD nodes */
  /* The assumption is that during CalHashTableGC, the freed BDD nodes
     are marked. However, since they are not touched after being put
     on the free list, the mark should be unaffected and can be used
     for cleaning up the cache table.
     */
  CalCacheTableTwoGCFlush(bddManager->cacheTable);
  bddManager->numGC++;
  return 0;
}

/**Function********************************************************************

  Synopsis    [Sets the limit of the garbage collection.]

  Description [It tries to set the limit at twice the number of nodes
  in the manager at the current point. However, the limit is not
  allowed to fall below the MIN_GC_LIMIT or to exceed the value of
  node limit (if one exists).]

  SideEffects [None.]

******************************************************************************/
void
Cal_BddManagerSetGCLimit(Cal_BddManager manager)
{
  manager->uniqueTableGCLimit = ((manager->numNodes) << 1);
  if(manager->uniqueTableGCLimit < CAL_MIN_GC_LIMIT){
    manager->uniqueTableGCLimit = CAL_MIN_GC_LIMIT;
  }
  if (manager->nodeLimit && (manager->uniqueTableGCLimit >
                             manager->nodeLimit)){
    manager->uniqueTableGCLimit = manager->nodeLimit;
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalBddManagerGCCheck(Cal_BddManager_t * bddManager)
{
  if (bddManager->gcMode == 0) return;
  if (bddManager->gcCheck > 0) return;
  bddManager->gcCheck = CAL_GC_CHECK;
  if(bddManager->numNodes > bddManager->uniqueTableGCLimit){
    Cal_BddManagerGC(bddManager);
    Cal_BddManagerSetGCLimit(bddManager);
  }
}

/**Function********************************************************************

  Synopsis    [This function performs the garbage collection operation
  for a particular index.]

  Description [The input is the hash table containing the nodes
  belonging to that level. Each bin of the hash table is traversed and
  the Bdd nodes with 0 reference count are put at the appropriate
  level in the processing que of the manager.]

  SideEffects [The number of nodes in the hash table can possibly decrease.]

  SeeAlso     [optional]

******************************************************************************/
int
CalHashTableGC(Cal_BddManager_t *bddManager, CalHashTable_t *hashTable)
{
  CalBddNode_t *last, *next, *ptr, *thenBddNode, *elseBddNode;
  int i;
  int oldNumEntries;
  
  oldNumEntries = hashTable->numEntries;
  for(i = 0; i < hashTable->numBins; i++){
    last = NULL;
    ptr = hashTable->bins[i];
    while(ptr != Cal_Nil(CalBddNode_t)){
      next = CalBddNodeGetNextBddNode(ptr);
      if(CalBddNodeIsRefCountZero(ptr)){
        if (last == NULL){
          hashTable->bins[i] = next;
        }
        else{
          CalBddNodePutNextBddNode(last,next);
        }
        thenBddNode = CAL_BDD_POINTER(CalBddNodeGetThenBddNode(ptr));
        elseBddNode = CAL_BDD_POINTER(CalBddNodeGetElseBddNode(ptr));
        CalBddNodeDcrRefCount(thenBddNode);
        CalBddNodeDcrRefCount(elseBddNode);
        CalNodeManagerFreeNode(hashTable->nodeManager, ptr);
        /* Mark the freed node for cache table clean up */
        /* We have to make sure that the clean up routine is called */
        /* right after this function (so that the marking remains */
        /* valid) */
        CalBddNodeMark(ptr);
        hashTable->numEntries--;
      }
      else {
        last = ptr;
      }
      ptr = next;
    }
  }
  return oldNumEntries - hashTable->numEntries;
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalRepackNodesAfterGC(Cal_BddManager_t *bddManager)
{
  int index, id, numPagesRequired, packingFlag, pageNum, nodeNum;
  int rehashFlag = 0;
  int newSizeIndex, hashValue;
  CalNodeManager_t *nodeManager;
  CalHashTable_t *uniqueTableForId;
  CalBddNode_t *bddNode, *thenBddNode, *elseBddNode, *freeNodeList;
  CalBddNode_t *newNode;
  Cal_Bdd_t thenBdd, elseBdd;
  
  packingFlag = 0;
  
  for (index = bddManager->numVars-1; index >= 0; index--){
    id = bddManager->indexToId[index];
    uniqueTableForId = bddManager->uniqueTable[id];
    nodeManager = uniqueTableForId->nodeManager;
    if (CalBddIdNeedsRepacking(bddManager, id) == 0){
      if (packingFlag == 0) continue; /* nothing needs to be done */
      /* We just need to update the cofactors and continue; */
      for (pageNum=0; pageNum < nodeManager->numPages; pageNum++){
        for(nodeNum = 0,
                bddNode = (CalBddNode_t *)nodeManager->pageList[pageNum]; 
            nodeNum < NUM_NODES_PER_PAGE; nodeNum++, bddNode += 1){
          if (CalBddNodeIsRefCountZero(bddNode) ||
              CalBddNodeIsForwarded(bddNode)) continue;
          thenBddNode = CalBddNodeGetThenBddNode(bddNode);
          elseBddNode = CalBddNodeGetElseBddNode(bddNode);
          CalBddNodeGetThenBdd(bddNode, thenBdd);
          CalBddNodeGetElseBdd(bddNode, elseBdd);
          if (CalBddIsForwarded(thenBdd)){
            CalBddForward(thenBdd);
            CalBddNodePutThenBdd(bddNode, thenBdd);
            rehashFlag = 1;
          }
          if (CalBddIsForwarded(elseBdd)){
            CalBddForward(elseBdd);
            CalBddNodePutElseBdd(bddNode, elseBdd);
            rehashFlag = 1;
          }
          Cal_Assert(!CalBddIsRefCountZero(thenBdd));
          Cal_Assert(!CalBddIsRefCountZero(elseBdd));
          Cal_Assert(bddManager->idToIndex[id] <
                     bddManager->idToIndex[bddNode->thenBddId]);   
          Cal_Assert(bddManager->idToIndex[id] < 
                     bddManager->idToIndex[bddNode->elseBddId]);
          if (rehashFlag){
            CalUniqueTableForIdRehashNode(uniqueTableForId, bddNode,
                                          thenBddNode, elseBddNode);
          }
        }
      }
      continue; /* move to next higher index */
    }
    packingFlag = 1;
    if ((uniqueTableForId->numBins > uniqueTableForId->numEntries) &&
        (uniqueTableForId->sizeIndex > HASH_TABLE_DEFAULT_SIZE_INDEX)){
      /* Free the old bins */
      Cal_MemFree(uniqueTableForId->bins);
      /* Create the new set of bins */
      newSizeIndex =
          CeilLog2(uniqueTableForId->numEntries/HASH_TABLE_DEFAULT_MAX_DENSITY); 
      if (newSizeIndex < HASH_TABLE_DEFAULT_SIZE_INDEX){
        newSizeIndex = HASH_TABLE_DEFAULT_SIZE_INDEX;
      }
      uniqueTableForId->sizeIndex = newSizeIndex;
      uniqueTableForId->numBins =  TABLE_SIZE(uniqueTableForId->sizeIndex);
      uniqueTableForId->maxCapacity =
          uniqueTableForId->numBins * HASH_TABLE_DEFAULT_MAX_DENSITY; 
      uniqueTableForId->bins = Cal_MemAlloc(CalBddNode_t *,
                                            uniqueTableForId->numBins); 
      if(uniqueTableForId->bins == Cal_Nil(CalBddNode_t *)){
        CalBddFatalMessage("out of memory");
      }
    }
    /* Clear the unique table bins */
    memset((char *)uniqueTableForId->bins, 0,
           uniqueTableForId->numBins*sizeof(CalBddNode_t *));
    numPagesRequired =
        uniqueTableForId->numEntries/NUM_NODES_PER_PAGE+1;
    /* Traverse the first numPagesRequired pages of this nodeManager */
    /* Create the new free list */
    nodeManager->freeNodeList = freeNodeList = Cal_Nil(CalBddNode_t);
    for (pageNum = 0; pageNum < nodeManager->numPages; pageNum++){
      for(nodeNum = 0,
              bddNode = (CalBddNode_t *)nodeManager->pageList[pageNum]; 
          nodeNum < NUM_NODES_PER_PAGE; nodeNum++, bddNode += 1){
        if(CalBddNodeIsRefCountZero(bddNode) ||
           CalBddNodeIsForwarded(bddNode)){
          if (pageNum < numPagesRequired){
            bddNode->nextBddNode = freeNodeList;
            freeNodeList = bddNode;
          }
          continue;
        }
        CalBddNodeGetThenBdd(bddNode, thenBdd);
        CalBddNodeGetElseBdd(bddNode, elseBdd);
        if (CalBddIsForwarded(thenBdd)){
          CalBddForward(thenBdd);
          CalBddNodePutThenBdd(bddNode, thenBdd);
        }
        if (CalBddIsForwarded(elseBdd)){
          CalBddForward(elseBdd);
          CalBddNodePutElseBdd(bddNode, elseBdd); 
        }
        if (pageNum < numPagesRequired){
          /* Simply insert the node in the unique table */
          hashValue = CalDoHash2(thenBdd.bddNode, elseBdd.bddNode,
                                 uniqueTableForId); 
          CalBddNodePutNextBddNode(bddNode, uniqueTableForId->bins[hashValue]);
          uniqueTableForId->bins[hashValue] = bddNode;
        }
        else {
          /* Create a new node */
          newNode = freeNodeList;
          freeNodeList = newNode->nextBddNode;
          newNode->thenBddNode = bddNode->thenBddNode;
          newNode->elseBddNode = bddNode->elseBddNode;
          newNode->thenBddId = bddNode->thenBddId;
          newNode->elseBddId = bddNode->elseBddId;
          newNode->nextBddNode = bddNode->nextBddNode;
          bddNode->elseBddNode = FORWARD_FLAG;
          bddNode->thenBddId = id;
          bddNode->thenBddNode = newNode;
          hashValue = CalDoHash2(thenBdd.bddNode, elseBdd.bddNode,
                                 uniqueTableForId); 
          CalBddNodePutNextBddNode(newNode, uniqueTableForId->bins[hashValue]);
          uniqueTableForId->bins[hashValue] = newNode;
        }
      }
      if (pageNum >= numPagesRequired){
        /* Free this page. I am assuming that there would not be any
           call to PageManagerAllocPage, until this function finishes */
        /* Also, CalPageManagerFreePage overwrites only the first field of
           the bdd node (the nextBddNode field), hence no relevant
           information is lost */
        CalPageManagerFreePage(nodeManager->pageManager,
                               nodeManager->pageList[pageNum]);
        nodeManager->pageList[pageNum] = 0;
      }
    }
#ifdef _CAL_VERBOSE    
    printf("Recycled %4d pages for %3d id\n",
           nodeManager->numPages-numPagesRequired, id);
#endif
    nodeManager->numPages = numPagesRequired;
    nodeManager->freeNodeList = freeNodeList;
  }
  /* Need to update the handles to the nodes being moved */
  if (bddManager->pipelineState == CREATE){
    /* There are some results computed in pipeline */
    CalBddReorderFixProvisionalNodes(bddManager);
  }
  
  CalCacheTableTwoRepackUpdate(bddManager->cacheTable);

  /* Fix the user BDDs */
  CalBddReorderFixUserBddPtrs(bddManager);

  /* Fix the association */
  CalReorderAssociationFix(bddManager);

  Cal_Assert(CalCheckAssoc(bddManager));
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Returns the smallest integer greater than or equal to log2 of a
  number]

  Description [Returns the smallest integer greater than or equal to log2 of a
  number (The assumption is that the number is >= 1)]

  SideEffects [None]

******************************************************************************/
static int
CeilLog2(int  number)
{
  int num, count;
  for (num=number, count=0; num > 1; num >>= 1, count++);
  if ((1 << count) != number) count++;
  return count;
}
