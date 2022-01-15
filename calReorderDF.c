/**CFile***********************************************************************

  FileName    [calReorderDF.c]

  PackageName [cal]

  Synopsis    [Routines for dynamic reordering of variables.]

  Description [This method is based on traditional dynamic reordering
  technique found in depth-first based packages. The data structure is
  first converted to conform to traditional one and then reordering is
  performed. At the end the nodes are arranged back on the pages. The
  computational overheads are in terms of converting the data
  structure back and forth and the memory overhead due to the extra
  space needed to arrange the nodes. This overhead can be eliminated
  by proper implementation. For details, please refer to the work by
  Rajeev K. Ranjan et al - "Dynamic variable reordering in a
  breadth-first manipulation based package: Challenges and Solutions"-
  Proceedings of ICCD'97.]

  SeeAlso     [calReorderBF.c calReorderUtil.c]

  Author      [Rajeev K. Ranjan   (rajeev@@ic. eecs.berkeley.edu)]

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
static   CalNodeManager_t *nodeManager; 
static   int freeListId;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/* These macros are needed because we are dealing with a new data
   structures of the BDD nodes */

#define BddNodeIcrRefCount(f)                   \
{                                               \
  CalBddNode_t *_bddNode = CAL_BDD_POINTER(f);  \
  if (_bddNode->elseBddId < CAL_MAX_REF_COUNT){ \
    _bddNode->elseBddId++;                      \
  }                                             \
}

#define BddNodeDcrRefCount(f) \
{ \
  CalBddNode_t *_bddNode = CAL_BDD_POINTER(f); \
  if ((_bddNode->elseBddId < CAL_MAX_REF_COUNT) && (_bddNode->elseBddId)){ \
    _bddNode->elseBddId--; \
  } \
  else if (_bddNode->elseBddId == 0){ \
    CalBddWarningMessage("Trying to decrement reference count below zero"); \
  } \
}

#define BddGetCofactors(bddManager, f, id, fThen, fElse)                \
{                                                                       \
  CalBddNode_t *_bddNode = CAL_BDD_POINTER(f);                          \
  Cal_Assert(bddManager->idToIndex[_bddNode->thenBddId] <=              \
             bddManager->idToIndex[id]);                                \
  if (bddManager->idToIndex[_bddNode->thenBddId] ==                     \
      bddManager->idToIndex[id]){                                       \
    fThen = _bddNode->thenBddNode;                                      \
    fElse = _bddNode->elseBddNode;                                      \
  }                                                                     \
  else{                                                                 \
    fThen = fElse = f;                                                  \
  }                                                                     \
}

#define BddNodeGetThenBddNode(bddNode)                    \
((CalBddNode_t*) ((CalAddress_t)                          \
                  (CAL_BDD_POINTER(bddNode)->thenBddNode) \
                  ^ (CAL_TAG0(bddNode))))

#define BddNodeGetElseBddNode(bddNode)                    \
((CalBddNode_t*) ((CalAddress_t)                          \
                  (CAL_BDD_POINTER(bddNode)->elseBddNode) \
                  ^ (CAL_TAG0(bddNode))))
  
    
    
/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int UniqueTableForIdFindOrAdd(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, CalBddNode_t *thenBdd, CalBddNode_t *elseBdd, CalBddNode_t **bddPtr);
static void HashTableAddDirect(CalHashTable_t * hashTable, CalBddNode_t *bddNode);
static int HashTableFindOrAdd(Cal_BddManager_t *bddManager, CalHashTable_t *hashTable, CalBddNode_t *thenBdd, CalBddNode_t *elseBdd, CalBddNode_t **bddPtr);
static void BddConvertDataStruct(Cal_BddManager_t *bddManager);
static void BddConvertDataStructBack(Cal_BddManager_t *bddManager);
static void BddReallocateNodes(Cal_BddManager_t *bddManager);
static void BddExchangeAux(Cal_BddManager_t *bddManager, CalBddNode_t *f, int id, int nextId);
static int CheckValidityOfNodes(Cal_BddManager_t *bddManager, long id);
static void SweepVarTable(Cal_BddManager_t *bddManager, long id);
static void BddExchange(Cal_BddManager_t *bddManager, long id);
static void BddExchangeVarBlocks(Cal_BddManager_t *bddManager, Cal_Block parent, long blockIndex);
static int BddReorderWindow2(Cal_BddManager_t *bddManager, Cal_Block block, long i);
static int BddReorderWindow3(Cal_BddManager_t *bddManager, Cal_Block block, long i);
static void BddReorderStableWindow3Aux(Cal_BddManager_t *bddManager, Cal_Block block, char *levels);
static void BddReorderStableWindow3(Cal_BddManager_t *bddManager);
static void BddSiftBlock(Cal_BddManager_t *bddManager, Cal_Block block, long startPosition, double maxSizeFactor);
static void BddReorderSiftAux(Cal_BddManager_t *bddManager, Cal_Block block, Cal_Block *toSift, double maxSizeFactor);
static void BddReorderSift(Cal_BddManager_t *bddManager, double maxSizeFactor);
static int CeilLog2(int number);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalBddReorderAuxDF(Cal_BddManager_t *bddManager)
{
  CalHashTableGC(bddManager, bddManager->uniqueTable[0]);
  /*Cal_BddManagerGC(bddManager);Cal_Assert(CalCheckAllValidity(bddManager));*/
  /* If we want to check the validity, we need to garbage collect */
  CalInitInteract(bddManager); /* Initialize the interaction matrix
                                  before changing the data structure */
  nodeManager = CalNodeManagerInit(bddManager->pageManager2);
  freeListId = 1;
#ifdef _CAL_QUANTIFY_
  quantify_start_recording_data();
#endif
  BddConvertDataStruct(bddManager);
  if (bddManager->reorderTechnique == CAL_REORDER_WINDOW){
    BddReorderStableWindow3(bddManager);
  }
  else {
    BddReorderSift(bddManager, bddManager->maxSiftingGrowth);
  }
  BddReallocateNodes(bddManager);
  BddConvertDataStructBack(bddManager);
#ifdef _CAL_QUANTIFY_
  quantify_stop_recording_data();
#endif
  nodeManager->numPages = 0; /* Since these pages have already been
                                freed */
  CalNodeManagerQuit(nodeManager);
  Cal_Assert(CalCheckAllValidity(bddManager));
  Cal_MemFree(bddManager->interact);
  bddManager->numReorderings++;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Definition of static functions                                          */
/*---------------------------------------------------------------------------*/
static void
NodeManagerAllocNode(Cal_BddManager_t *bddManager, CalBddNode_t **nodePtr) 
{
  /* First check the free list of bddManager */
  if (nodeManager->freeNodeList){
    *nodePtr = nodeManager->freeNodeList;
    nodeManager->freeNodeList =
        ((CalBddNode_t *)(*nodePtr))->nextBddNode;
  }
  else{
    if (freeListId < bddManager->numVars){
      /* Find the next id with free list */
      for (; freeListId <= bddManager->numVars; freeListId++){
        CalNodeManager_t *nodeManagerForId =
            bddManager->nodeManagerArray[freeListId]; 
        if (nodeManagerForId->freeNodeList){
          *nodePtr = nodeManagerForId->freeNodeList;
          nodeManagerForId->freeNodeList = (CalBddNode_t *)0;
          nodeManager->freeNodeList =
              ((CalBddNode_t *)(*nodePtr))->nextBddNode;
          break;
        }
      }
    }
  }
  if (!(*nodePtr)){
    /* Create a new page */
    CalBddNode_t *_freeNodeList, *_nextNode, *_node;                        
    _freeNodeList =                                                         
        (CalBddNode_t *)CalPageManagerAllocPage(nodeManager->pageManager);  
    for(_node = _freeNodeList + NUM_NODES_PER_PAGE - 1, _nextNode =0;       
        _node != _freeNodeList; _nextNode = _node--){                       
      _node->nextBddNode = _nextNode;                                       
    }                                                                       
    nodeManager->freeNodeList = _freeNodeList + 1;                          
    *nodePtr = _node;
    if (nodeManager->numPages == nodeManager->maxNumPages){             
      nodeManager->maxNumPages *= 2;                                      
      nodeManager->pageList =                                            
          Cal_MemRealloc(CalAddress_t *, nodeManager->pageList, 
                         nodeManager->maxNumPages);                       
    }                                                                       
    nodeManager->pageList[nodeManager->numPages++] =
        (CalAddress_t *)_freeNodeList;     
  }
}

/**Function********************************************************************

  Synopsis    [find or add in the unique table for id.]

  Description [optional]

  SideEffects [If a new BDD node is created (found == false), then the
  numNodes field of the manager needs to be incremented.]

  SeeAlso     [optional]

******************************************************************************/
static int
UniqueTableForIdFindOrAdd(Cal_BddManager_t * bddManager,
                          CalHashTable_t * hashTable,
                          CalBddNode_t *thenBdd,
                          CalBddNode_t *elseBdd,
                          CalBddNode_t **bddPtr)
{
  int found = 0; 
  if (thenBdd == elseBdd){
    *bddPtr = thenBdd;
    found = 1;
  }
  else if(CalBddNodeIsOutPos(thenBdd)){
    found = HashTableFindOrAdd(bddManager, hashTable, thenBdd, elseBdd, bddPtr);
  }
  else{
    found = HashTableFindOrAdd(bddManager, hashTable,
                               CalBddNodeNot(thenBdd),
                               CalBddNodeNot(elseBdd), bddPtr); 
    *bddPtr = CalBddNodeNot(*bddPtr);
  }
  if (!found) bddManager->numNodes++;
  return found;
}

/**Function********************************************************************

  Synopsis    [Directly insert a BDD node in the hash table.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
HashTableAddDirect(CalHashTable_t * hashTable, CalBddNode_t *bddNode)
{
  int hashValue;
  CalBddNode_t *thenBddNode, *elseBddNode;

  hashTable->numEntries++;
  if(hashTable->numEntries >= hashTable->maxCapacity){
    CalHashTableRehash(hashTable, 1);
  }
  thenBddNode = bddNode->thenBddNode;
  Cal_Assert(CalBddNodeIsOutPos(thenBddNode));
  elseBddNode = bddNode->elseBddNode;
  hashValue = CalDoHash2(thenBddNode, elseBddNode, hashTable);
  bddNode->nextBddNode = hashTable->bins[hashValue];
  hashTable->bins[hashValue] = bddNode;
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static int
HashTableFindOrAdd(Cal_BddManager_t *bddManager, CalHashTable_t
                   *hashTable,  CalBddNode_t *thenBdd,
                   CalBddNode_t *elseBdd, CalBddNode_t **bddPtr) 
{
  CalBddNode_t *ptr;
  int hashValue;
  
  Cal_Assert(CalBddNodeIsOutPos(thenBdd));
  hashValue = CalDoHash2(thenBdd, elseBdd, hashTable);
  for (ptr = hashTable->bins[hashValue]; ptr; ptr = ptr->nextBddNode){
    if ((ptr->thenBddNode == thenBdd) &&
        (ptr->elseBddNode == elseBdd)){
      *bddPtr = ptr;
      return 1;
    }
  }
  hashTable->numEntries++;
  if(hashTable->numEntries > hashTable->maxCapacity){
    CalHashTableRehash(hashTable,1);
    hashValue = CalDoHash2(thenBdd, elseBdd, hashTable);
  }

  NodeManagerAllocNode(bddManager, &ptr);

  ptr->thenBddNode = thenBdd;
  ptr->elseBddNode = elseBdd;
  ptr->nextBddNode = hashTable->bins[hashValue];
  ptr->thenBddId = hashTable->bddId;
  ptr->elseBddId = 0;
  hashTable->bins[hashValue] = ptr;
  *bddPtr = ptr;
  return 0;
}


/**Function********************************************************************

  Synopsis           [Changes the data structure of the bdd nodes.]

  Description        [New data structure: thenBddId -> id 
                                          elseBddId -> ref count]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddConvertDataStruct(Cal_BddManager_t *bddManager)
{
  CalBddNode_t *bddNode, **bins, *thenBddNode, *elseBddNode,
      *next = Cal_Nil(CalBddNode_t); 
  CalBddNode_t *last;
  long numBins;
  int i, refCount, id, index;
  long oldNumEntries;
  CalHashTable_t *uniqueTableForId;

  if (bddManager->numPeakNodes < bddManager->numNodes){
    bddManager->numPeakNodes = bddManager->numNodes;
  }

  for(index = 0; index < bddManager->numVars; index++){
    id = bddManager->indexToId[index];
    uniqueTableForId = bddManager->uniqueTable[id];
    oldNumEntries = uniqueTableForId->numEntries;
    numBins = uniqueTableForId->numBins;
    bins = uniqueTableForId->bins;
    for(i = 0; i < numBins; i++){
      last = NULL;
      bddNode = uniqueTableForId->bins[i];
      while(bddNode != Cal_Nil(CalBddNode_t)){
        next = CalBddNodeGetNextBddNode(bddNode);
        CalBddNodeGetRefCount(bddNode, refCount);
        thenBddNode = CalBddNodeGetThenBddNode(bddNode);
        elseBddNode = CalBddNodeGetElseBddNode(bddNode);
        if(refCount == 0){
          if (last == NULL){
            uniqueTableForId->bins[i] = next;
          }
          else{
            last->nextBddNode = next;
          }
          CalBddNodeDcrRefCount(CAL_BDD_POINTER(thenBddNode));
          CalBddNodeDcrRefCount(CAL_BDD_POINTER(elseBddNode));
          CalNodeManagerFreeNode(nodeManager, bddNode);
          uniqueTableForId->numEntries--;
        }
        else {
          bddNode->thenBddId = id;
          bddNode->elseBddId = refCount;
          bddNode->nextBddNode = next;
          bddNode->thenBddNode = thenBddNode;
          bddNode->elseBddNode = elseBddNode;
          last = bddNode; 
        }
        bddNode = next;
      }
    }
    if ((uniqueTableForId->numBins > uniqueTableForId->numEntries) &&
        (uniqueTableForId->sizeIndex > HASH_TABLE_DEFAULT_SIZE_INDEX)){
      CalHashTableRehash(uniqueTableForId, 0);
    }
    bddManager->numNodes -= oldNumEntries - uniqueTableForId->numEntries;
    bddManager->numNodesFreed += oldNumEntries - uniqueTableForId->numEntries;
  }
  id = 0;
  uniqueTableForId = bddManager->uniqueTable[id];
  numBins = uniqueTableForId->numBins;
  bins = uniqueTableForId->bins;
  for(i = 0; i < numBins; i++){
    bddNode = uniqueTableForId->bins[i];
    while(bddNode != Cal_Nil(CalBddNode_t)){
      next = CalBddNodeGetNextBddNode(bddNode);
      CalBddNodeGetRefCount(bddNode, refCount);
      Cal_Assert(refCount);
      thenBddNode = CalBddNodeGetThenBddNode(bddNode);
      elseBddNode = CalBddNodeGetElseBddNode(bddNode);
      bddNode->thenBddId = id;
      bddNode->elseBddId = refCount;
      bddNode->nextBddNode = next;
      bddNode->thenBddNode = thenBddNode;
      bddNode->elseBddNode = elseBddNode;
      bddNode = next;
    }
  }
  bddNode = bddManager->bddOne.bddNode;
  CalBddNodeGetRefCount(bddNode, refCount);
  Cal_Assert(refCount);
  thenBddNode = CalBddNodeGetThenBddNode(bddNode);
  elseBddNode = CalBddNodeGetElseBddNode(bddNode);
  bddNode->thenBddId = id;
  bddNode->elseBddId = refCount;
  bddNode->nextBddNode = next;
  bddNode->thenBddNode = thenBddNode;
  bddNode->elseBddNode = elseBddNode;
}


/**Function********************************************************************

  Synopsis           [Changes the data structure of the bdd nodes to
  the original one.]

  Description        [Data structure conversion: thenBddId -> id 
  elseBddId -> ref count]
  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddConvertDataStructBack(Cal_BddManager_t *bddManager)
{
  Cal_Bdd_t thenBdd, elseBdd;
  
  CalBddNode_t *bddNode, *nextBddNode, **bins;
  long numBins;
  int i, id, index;
  CalHashTable_t *uniqueTableForId;
  uniqueTableForId = bddManager->uniqueTable[0];
  numBins = uniqueTableForId->numBins;
  bins = uniqueTableForId->bins;
  for(i = 0; i < numBins; i++) {
    for(bddNode = bins[i];
        bddNode != Cal_Nil(CalBddNode_t);
        bddNode = nextBddNode) {
      nextBddNode = CalBddNodeGetNextBddNode(bddNode);
      CalBddNodePutRefCount(bddNode, bddNode->elseBddId);
      bddNode->thenBddId = CAL_BDD_POINTER(bddNode->thenBddNode)->thenBddId;
      bddNode->elseBddId = CAL_BDD_POINTER(bddNode->elseBddNode)->thenBddId;
    }
  }
  for(index = 0; index < bddManager->numVars; index++){
    id = bddManager->indexToId[index];
    uniqueTableForId = bddManager->uniqueTable[id];
    numBins = uniqueTableForId->numBins;
    bins = uniqueTableForId->bins;
    for(i = 0; i < numBins; i++) {
      for(bddNode = bins[i];
          bddNode != Cal_Nil(CalBddNode_t);
          bddNode = nextBddNode) {
        nextBddNode = CalBddNodeGetNextBddNode(bddNode);
        CalBddNodePutRefCount(bddNode, bddNode->elseBddId);
        bddNode->thenBddId = CAL_BDD_POINTER(bddNode->thenBddNode)->thenBddId;
        bddNode->elseBddId = CAL_BDD_POINTER(bddNode->elseBddNode)->thenBddId;
      Cal_Assert(!CalBddNodeIsForwarded(bddNode));
      Cal_Assert(!CalBddNodeIsRefCountZero(bddNode));
      CalBddNodeGetThenBdd(bddNode, thenBdd);
      CalBddNodeGetElseBdd(bddNode, elseBdd);
      Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
      Cal_Assert(CalBddIsForwarded(elseBdd) == 0);
      }
    }
  }
  bddNode = bddManager->bddOne.bddNode;
  CalBddNodePutRefCount(bddNode, bddNode->elseBddId);
  bddNode->thenBddId = 0;
  bddNode->elseBddId = 0;
}

#ifdef _FOO_
/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddReallocateNodesInPlace(Cal_BddManager_t *bddManager)
{
  Cal_Address_t  *pageSegment;
  CalPageManager_t *pageManager;
  CalHashTable_t *uniqueTable;
  CalNodeManager_t *nodeManager;
  int index, id, i, pageCounter, numUsefulSegments, segmentCounter;
  
  /* Initialize and set up few things */
  pageManager = bddManager->pageManager;
  uniqueTable = bddManager->uniqueTable;
  for (id = 1; id <= bddManager->numVars; id++){
    numPagesRequired =
        uniqueTable[id]->numEntries/NUM_NODES_PER_PAGE+1;
    nodeManager = uniqueTable[id]->nodeManager;
    /* Clear out the page list of the node manager */
    for (i=0; i<nodeManager->maxNumPages; i++){
      nodeManager->pageList[i] = 0;
    }
    nodeManager->freeNodeList = (CalBddNode_t *)0;
    nodeManager->numPages = numPagesRequired;
    Cal_Assert(nodeManager->maxNumPages >= numPagesRequired);
    for (i = 0; i<numPagesRequired; i++){
      if (++pageCounter ==
              pageManager->numPagesArray[segmentCounter]){
        pageCounter = 0;
        segmentCounter++;
        pageSegment = pageManager->pageSegmentArray[segmentCounter];
      }
      nodeManager->pageList[i] = pageSegment[pageCounter];
    }
  }
  numUsefulSegments = segmentCounter+1;
  numUsefulPagesInLastSegment = pageCounter+1;
  
  /* Traverse all the nodes belonging in each page */
  /* Put the destination addresses in the next pointer */
  for (numSegment=0; numSegment < pageManager->numSegments; 
       numSegment++){
    for (numPage = 0, page = pageManager->pageSegmentArray[numSegment]; 
         numPage < pageManager->numPagesArray[numSegment];
         page += PAGE_SIZE, numPage++){
      for (bddNode = (CalBddNode_t*) page, numNode = 0;
           numNode < NUM_NODES_PER_PAGE; numNode++, bddNode += 1){
        /* If the node is not useful, continue */
        if (bddNode->elseBddId == 0) continue; 
        /* Find out the destination address */
        bddId = bddNode->thenBddId;
        nodeCounter[bddId]++;
        if (nodeCounter[bddId] == NUM_NODES_PER_PAGE){
          pageCounter[bddId]++;
          nodePointerArray[bddId] =
              pageListArray[bddId][pageCounter[bddId]];
          nodeCounter[bddId] = 0;
        }
        bddNode->nextBddNode = nodePointerArray[bddId];
        nodePointerArray[bddId] += 1;
      }
    }
  }
  /* Traverse all the nodes belonging in each page */
  /* Update the contents */
  for (numSegment=0; numSegment < pageManager->totalNumSegments; 
       numSegment++){
    for (numPage = 0, page = pageManager->pageSegmentArray[numSegment]; 
         numPage < pageManager->numPagesArray[numSegment];
         page += PAGE_SIZE, numPage++){
      for (bddNode = (CalBddNode_t*) page, numNode = 0;
           numNode < NUM_NODES_PER_PAGE; numNode++, bddNode += 1){
        /* If the node is not useful, continue */
        if (bddNode->elseBddId == 0) continue; 
        /* If the node has been visited, continue */
        if ((CalAddress_t)bddNode->nextBddNode & 01) continue;
        /* If the nodes is supposed to remain at the same place,
           update the then and else pointers and continue */
        if (((CalAddress_t) bddNode->nextBddNode &~01) ==
            ((CalAddress_t) bddNode & ~01)){
          CalBddNodeUpdatebddNode(bddNode);
          continue;
        }
        origNode = bddNode; /* Remember the address */
        /* Update the contents */
        thenBddNode = bddNode->thenBddNode;
        elseBddNode = bddNode->elseBddNode;
        thenBddId = bddNode->thenBddId;
        elseBddId = bddNode->elseBddId;
        do{
          thenBddNode = UpdateThenBddNode(thenBddNode);
          elseBddNode = UpdateElseBddNode(elseBddNode);
          destinationNode = bddNode->nextBddNode;
          /* Mark the node visited */
          bddNode->nextBddNode = (CalBddNode_t *)
              ((CalAddress_t)bddNode->nextBddNode | 01);
          thenBddNode2 = destinationNode->thenBddNode;
          elseBddNode2 = destinationNode->elseBddNode;
          thenBddId2 = destinationNode->thenBddId;
          elseBddId2 = destinationNode->elseBddId;
          destinationNode->thenBddNode = thenBddNode;
          destinationNode->elseBddNode = elseBddNode;
          destinationNode->thenBddId = thenBddId;
          destinationNode->elseBddId = elseBddId;
          bddNode = destinationNode;
          thenBddNode = thenBddNode2;
          elseBddNode = elseBddNode2;
          thenBddId = thenBddId2;
          elseBddId = elseBddId2;
        } while ((elseBddId != 0) && (bddNode != origNode) &&
                 !((CalAddress_t)(bddNode->nextBddNode) & 01));
      }
    }
  }
  /* Fix the handles to the nodes being moved */
  for (id = 1; id <= bddManager->numVars; id++){
    /* Fix the varBdd array */
  }
/* Need to update the handles to the nodes being moved */
  if (bddManager->pipelineState == CREATE){
    /* There are some results computed in pipeline */
    CalBddReorderFixProvisionalNodesAfterReallocation(bddManager);
  }
  
  /* Fix the user BDDs */
  CalBddReorderFixUserBddPtrsAfterReallocation(bddManager);

  /* Fix the association */
  CalReorderAssociationFixAfterReallocation(bddManager);

  Cal_Assert(CalCheckAssoc(bddManager));
  

  /* Update the next pointers */
  /* Since the pages for the ids are distributed in the uniform
     manner, we can scan the pages on id by id basis without any
     disadvantage */
  for (id = 1; id <= bddManager->numVars; id++){
    nodeManager = uniqueTable[id]->nodeManager;
    freeNodeList = Cal_Nil(CalBddNode_t);
    for (i=0; i<nodeManager->numPages; i++){
      page = nodeManager->pageList[i];
      for (bddNode = (CalBddNode_t*) page, numNode = 0;
           numNode < NUM_NODES_PER_PAGE; numNode++, bddNode += 1){
        /* If the node is not useful, put it in the free list */
        if ((bddNode->elseBddId == 0) || (bddNode->elseBddNode == 0)){
          bddNode->nextBddNode = freeNodeList;
          freeNodeList = bddNode;
        }
      }
    }
    nodeManager->freeNodeList = freeNodeList;
  }
  /* We should put the unused pages in the free page list */
  pageSegment = pageManager->pageSegmentArray[numUsefulSegments-1];
  for (pageCounter = numUsefulPagesInLastSegment;
       pageCounter < pageSegment->numPages ; pageCounter++){
    CalPageManagerFreePage(pageManager, pageSegment[pageCounter]);
  }
  /* We have to free up the unnecessary page segments;*/
  for (i = numUsefulSegments; i < pageManager->numSegments; i++){
    free(pageManager->pageSegmentArray[i]);
    pageManager->pageSegmentArray[i] = 0;
  }
  pageManager->numSegments = numUsefulSegments;
}
/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalAlignCollisionChains(Cal_BddManager_t *bddManager)
{
  /* First find out the pages corresponding to each variable */
  Cal_Address_t ***pageListArray = Cal_MemAlloc(Cal_Address_t **,
                                                bddManager->numVars+1);
  for (id = 1; id <= bddManager->numVars; id++){
    nodeManager = bddManager->nodeManagerArray[id];
    numPages = nodeManager->numPages;
    pageListArray[id] = Cal_MemAlloc(Cal_Address_t *, numPages);
    for (i=0; i<numPages; i++){
      pageListArray[id][i] = nodeManager->pageList[i];
    }
  }
    
  /* Bottom up traversal */
  for (index = bddManager->numVars-1; index >= 0; index--){
    id = bddManager->indexToId[index];
    uniqueTableForId = bddManager->uniqueTable[id];
    nodeManager = uniqueTableForId->nodeManager;
    /* Calculate the collision lengths */
    collisionLengthArray = CalculateCollisionLength(uniqueTableForId);
    /* Initialize the bins */
    bins = uniqueTableForId->bins;
    numBins = uniqueTableForId->numBins;
    numNodes = 0;
    pageNum = 0;
    for (i=0; i<numBins; i++){
      numNodes += collisionLengthArray[i];
      if (numNodes < NUM_NODES_PER_PAGE){
        nodePointer[i] += collisionLengthArray[i];
      }
      else if (numNodes == NUM_NODES_PER_PAGE){
        nodePointer[i] = pageListArray[id][++pageNum];
        numNodes = 0;
      }
      else {
        /* put the rest of the nodes from this page in a free list */
        nodePointer[i]->nextBddNode = nodeManager->freeNodeList;
        nodeManager->freeNodeList = nodePointer;
        nodePointer[i] = pageListArray[id][++pageNum]+collisionLengthArray[i];
        numNodes = collisionLengthArray[i];
      }
    }
  }
}
#endif

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddReallocateNodes(Cal_BddManager_t *bddManager)
{
  int i;
  int index;
  CalNodeManager_t *nodeManager;
  CalPageManager_t *pageManager;
  int numSegments;
  CalAddress_t **pageSegmentArray;
  
  pageManager = bddManager->pageManager2;
  numSegments = pageManager->numSegments;
  pageSegmentArray = pageManager->pageSegmentArray;
  
  /* Reinitialize the page manager */
  pageManager->totalNumPages = 0;
  pageManager->numSegments = 0;
  pageManager->maxNumSegments = MAX_NUM_SEGMENTS;
  pageManager->pageSegmentArray 
      = Cal_MemAlloc(CalAddress_t *, pageManager->maxNumSegments);
  pageManager->freePageList = Cal_Nil(CalAddress_t);
  
  /* Do a bottom up traversal */

  for (index = bddManager->numVars-1; index >= 0; index--){
    int id;
    CalHashTable_t *uniqueTableForId;
    int numPagesRequired, newSizeIndex;
    CalBddNode_t *bddNode, *dupNode, *thenNode, *elseNode, **oldBins;
    long hashValue, oldNumBins;
    
    id = bddManager->indexToId[index];
    uniqueTableForId = bddManager->uniqueTable[id];
    nodeManager = bddManager->nodeManagerArray[id];
    oldBins = uniqueTableForId->bins;
    oldNumBins = uniqueTableForId->numBins;
    nodeManager->freeNodeList = Cal_Nil(CalBddNode_t);
    nodeManager->numPages = 0;
    numPagesRequired =
        uniqueTableForId->numEntries/NUM_NODES_PER_PAGE;
    nodeManager->maxNumPages =
        2*(numPagesRequired ? numPagesRequired : 1);
    Cal_MemFree(nodeManager->pageList);
    nodeManager->pageList = Cal_MemAlloc(CalAddress_t *,
                                         nodeManager->maxNumPages); 
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
    memset((char *)uniqueTableForId->bins, 0, 
           uniqueTableForId->numBins*sizeof(CalBddNode_t *)); 
    for (i=0; i<oldNumBins; i++){
      for (bddNode = oldBins[i]; bddNode; bddNode = bddNode->nextBddNode){ 
        CalNodeManagerAllocNode(nodeManager, dupNode);
        thenNode = bddNode->thenBddNode;
        CalBddNodeIsForwardedTo(thenNode);
        Cal_Assert(thenNode);
        Cal_Assert(!CalBddNodeIsForwarded(thenNode));
        elseNode = bddNode->elseBddNode;
        CalBddNodeIsForwardedTo(elseNode);
        Cal_Assert(elseNode);
        Cal_Assert(!CalBddNodeIsForwarded(CAL_BDD_POINTER(elseNode)));
        Cal_Assert(bddManager->idToIndex[bddNode->thenBddId] <
                   bddManager->idToIndex[thenNode->thenBddId]); 
        Cal_Assert(bddManager->idToIndex[bddNode->thenBddId] <
                   bddManager->idToIndex[CAL_BDD_POINTER(elseNode)->thenBddId]);
        dupNode->thenBddNode = thenNode;
        dupNode->elseBddNode = elseNode;
        dupNode->thenBddId = bddNode->thenBddId;
        dupNode->elseBddId = bddNode->elseBddId;
        hashValue = CalDoHash2(thenNode, elseNode, uniqueTableForId);
        dupNode->nextBddNode = uniqueTableForId->bins[hashValue];
        uniqueTableForId->bins[hashValue] = dupNode;
        bddNode->thenBddNode = dupNode;
        bddNode->elseBddNode = (CalBddNode_t *)0;
        bddNode->thenBddId = id;
        Cal_Assert(bddManager->idToIndex[dupNode->thenBddId] <
                   bddManager->idToIndex[thenNode->thenBddId]); 
        Cal_Assert(bddManager->idToIndex[dupNode->thenBddId] <
                   bddManager->idToIndex[CAL_BDD_POINTER(elseNode)->thenBddId]);
      }
    }
    Cal_MemFree(oldBins);
    CalBddIsForwardedTo(bddManager->varBdds[id]);
  }

  if (bddManager->pipelineState == CREATE){
    /* There are some results computed in pipeline */
    CalBddReorderFixProvisionalNodes(bddManager);
  }
  
  /* Fix the user BDDs */
  CalBddReorderFixUserBddPtrs(bddManager);

  /* Fix the association */
  CalReorderAssociationFix(bddManager);

  Cal_Assert(CalCheckAssoc(bddManager));
  
  /* Free the page manager related stuff*/
  for(i = 0; i < numSegments; i++){
    free(pageSegmentArray[i]);
  }
  Cal_MemFree(pageSegmentArray);
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddExchangeAux(Cal_BddManager_t *bddManager, CalBddNode_t *f,
               int id, int nextId)
{
  CalBddNode_t *f0, *f1;
  CalBddNode_t *f00, *f01, *f10, *f11;
  CalBddNode_t *newF0, *newF1;
  int f0Found, f1Found;
  int fIndex;
  
  f0 = f->elseBddNode;
  f1 = f->thenBddNode;

  if (CAL_BDD_POINTER(f0)->thenBddId == nextId){
    f00 = BddNodeGetElseBddNode(f0);
    f01 = BddNodeGetThenBddNode(f0);
  }
  else {
    f00 = f01 = f0;
  }
  if (CAL_BDD_POINTER(f1)->thenBddId == nextId){
    f10 = BddNodeGetElseBddNode(f1);
    f11 = BddNodeGetThenBddNode(f1);
  }
  else {
    f10 = f11 = f1;
  }
  
  if (f00 == f10){
    newF0 = f00;
    f0Found = 1;
  }
  else{
    f0Found = UniqueTableForIdFindOrAdd(bddManager,
                                        bddManager->uniqueTable[id], f10,
                                        f00, &newF0);
  }
  BddNodeIcrRefCount(newF0);
  if (f01 == f11){
    newF1 = f11;
    f1Found = 1;
  }
  else{
    f1Found = UniqueTableForIdFindOrAdd(bddManager,
                                        bddManager->uniqueTable[id], f11,
                                        f01, &newF1);
  }
  BddNodeIcrRefCount(newF1);

  f->thenBddId = nextId;
  f->elseBddNode = newF0;
  f->thenBddNode = newF1;

  fIndex = bddManager->idToIndex[id];
  Cal_Assert(fIndex <
             bddManager->idToIndex[CAL_BDD_POINTER(f00)->thenBddId]);
  Cal_Assert(fIndex <
             bddManager->idToIndex[CAL_BDD_POINTER(f10)->thenBddId]);
  Cal_Assert(fIndex <
             bddManager->idToIndex[CAL_BDD_POINTER(f01)->thenBddId]);
  Cal_Assert(fIndex <
             bddManager->idToIndex[CAL_BDD_POINTER(f11)->thenBddId]);
  Cal_Assert(CAL_BDD_POINTER(f00)->thenBddId != nextId);
  Cal_Assert(CAL_BDD_POINTER(f01)->thenBddId != nextId);
  Cal_Assert(CAL_BDD_POINTER(f10)->thenBddId != nextId);
  Cal_Assert(CAL_BDD_POINTER(f11)->thenBddId != nextId);
  
  if (!f0Found){
    BddNodeIcrRefCount(f00);
    BddNodeIcrRefCount(f10);
  }

  if (!f1Found){
    BddNodeIcrRefCount(f01);
    BddNodeIcrRefCount(f11);
  }

  BddNodeDcrRefCount(f0);
  BddNodeDcrRefCount(f1);
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
CheckValidityOfNodes(Cal_BddManager_t *bddManager, long id)
{
  CalHashTable_t *table = bddManager->uniqueTable[id];
  int i;
  CalBddNode_t *bddNode;
  int index = bddManager->idToIndex[id];
  for(i = 0; i < table->numBins; ++i){
    for (bddNode = table->bins[i]; bddNode; bddNode = bddNode->nextBddNode){
      int thenIndex = bddManager->idToIndex[bddNode->thenBddNode->thenBddId];
      int elseIndex =
          bddManager->idToIndex[CAL_BDD_POINTER(bddNode->elseBddNode)->thenBddId]; 
      assert((thenIndex > index) && (elseIndex > index));
    }
  }
  return 1;
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
SweepVarTable(Cal_BddManager_t *bddManager, long id)
{
  CalHashTable_t *table = bddManager->uniqueTable[id];
  long numNodesFreed, oldNumEntries;
  CalBddNode_t **ptr, *bddNode;
  int i;
  
  oldNumEntries = table->numEntries;
  for(i = 0; i < table->numBins; ++i){
    for (ptr = &table->bins[i], bddNode = *ptr; bddNode;
         bddNode = *ptr){
      if (bddNode->elseBddId == 0){
        *ptr = bddNode->nextBddNode;
        CalNodeManagerFreeNode(nodeManager, bddNode);
        BddNodeDcrRefCount(bddNode->thenBddNode);
        BddNodeDcrRefCount(bddNode->elseBddNode);
        table->numEntries--;
      }
      else{
        ptr = &bddNode->nextBddNode;
      }
    }
  }
  numNodesFreed = oldNumEntries - table->numEntries;
  bddManager->numNodes -= numNodesFreed;
  bddManager->numNodesFreed += numNodesFreed;
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddExchange(Cal_BddManager_t *bddManager, long id)
{
  Cal_BddId_t  nextId;
  CalBddNode_t **ptr, *bddNode, *nodeList;
  CalHashTable_t *table, *nextTable;
  Cal_BddIndex_t index, nextIndex;
  int i;
  CalBddNode_t  *f1, *f2;
  CalAssociation_t *p;
  CalNodeManager_t *nodeManager;
  
  index = bddManager->idToIndex[id];
  nextIndex = index+1;
  nextId = bddManager->indexToId[nextIndex];

  if (CalTestInteract(bddManager, id, nextId)){
    bddManager->numSwaps++;
    nodeManager = bddManager->nodeManagerArray[id];
    table = bddManager->uniqueTable[id];
    nextTable = bddManager->uniqueTable[nextId];
    nodeList = (CalBddNode_t*)0;
    for(i = 0; i < table->numBins; i++){
      for (ptr = &table->bins[i], bddNode = *ptr; bddNode;
           bddNode = *ptr){
        Cal_Assert(bddNode->elseBddId != 0);
        f1 = bddNode->elseBddNode;
        f2 = bddNode->thenBddNode;
        if ((CAL_BDD_POINTER(f1)->thenBddId != nextId) &&
            (CAL_BDD_POINTER(f2)->thenBddId != nextId)){ 
          ptr = &bddNode->nextBddNode;
        }
        else{
          *ptr = bddNode->nextBddNode;
          bddNode->nextBddNode = nodeList;
          nodeList = bddNode;
        }
      }
    }
    for (bddNode = nodeList; bddNode ; bddNode = nodeList){
      BddExchangeAux(bddManager, bddNode, id, nextId);
      nodeList = bddNode->nextBddNode;
      HashTableAddDirect(nextTable, bddNode);
      table->numEntries--;
    }
    SweepVarTable(bddManager, nextId);
  }
  else {
    bddManager->numTrivialSwaps++;
  }
  
  CalFixupAssoc(bddManager, id, nextId, bddManager->tempAssociation);
  for(p = bddManager->associationList; p; p = p->next){
    CalFixupAssoc(bddManager, id, nextId, p);
  }

  bddManager->idToIndex[id] = nextIndex;
  bddManager->idToIndex[nextId] = index;
  bddManager->indexToId[index] = nextId;
  bddManager->indexToId[nextIndex] = id;

  Cal_Assert(CheckValidityOfNodes(bddManager, id));
  Cal_Assert(CheckValidityOfNodes(bddManager, nextId));
  Cal_Assert(CalCheckAssoc(bddManager));
#ifdef _CAL_VERBOSE
  /*fprintf(stdout,"Variable order after swap:\n");*/
  for (i=0; i<bddManager->numVars; i++){
    fprintf(stdout, "%3d ", bddManager->indexToId[i]);
  }
  fprintf(stdout, "%8d\n", bddManager->numNodes);
#endif
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddExchangeVarBlocks(Cal_BddManager_t *bddManager, Cal_Block parent,
                     long blockIndex)
{
  Cal_Block b1, b2, temp;
  long i, j, k, l, firstBlockWidth, secondBlockWidth;

  b1 = parent->children[blockIndex];
  b2 = parent->children[blockIndex+1];
  /* This slides the blocks past each other in a kind of interleaving */
  /* fashion. */
  firstBlockWidth = b1->lastIndex - b1->firstIndex;
  secondBlockWidth = b2->lastIndex - b2->firstIndex;
  
  for (i=0; i <= firstBlockWidth + secondBlockWidth; i++){
    j = i - firstBlockWidth;
    if (j < 0) j=0;
    k = ((i > secondBlockWidth) ? secondBlockWidth : i);
    while (j <= k) {
	  l = b2->firstIndex + j - i + j;
	  BddExchange(bddManager, bddManager->indexToId[l-1]);
	  ++j;
	}
  }
  CalBddBlockDelta(b1, secondBlockWidth+1);
  CalBddBlockDelta(b2, -(firstBlockWidth+1));
  temp = parent->children[blockIndex];
  parent->children[blockIndex] = parent->children[blockIndex+1];
  parent->children[blockIndex+1] = temp;
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
BddReorderWindow2(Cal_BddManager_t *bddManager, Cal_Block block, long i)
{
  long size, bestSize;

  /* 1 2 */
  bestSize = bddManager->numNodes;
  BddExchangeVarBlocks(bddManager, block, i);
  /* 2 1 */
  size = bddManager->numNodes;
  if (size < bestSize) return (1);
  BddExchangeVarBlocks(bddManager, block, i);
  return (0);
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
BddReorderWindow3(Cal_BddManager_t *bddManager, Cal_Block block, long i)
{
  int best;
  long size, bestSize;
  long origSize;
  
  origSize = bddManager->numNodes;
  best = 0;
  /* 1 2 3 */
  bestSize = bddManager->numNodes;
  BddExchangeVarBlocks(bddManager, block, i);
  /* 2 1 3 */
  size=bddManager->numNodes;
  if (size < bestSize) {
    best=1;
    bestSize=size;
  }
  BddExchangeVarBlocks(bddManager, block, i+1);
  /* 2 3 1 */
  size=bddManager->numNodes;
  if (size < bestSize) {
    best=2;
    bestSize=size;
  }
  BddExchangeVarBlocks(bddManager, block, i);
  /* 3 2 1 */
  size=bddManager->numNodes;
  if (size <= bestSize) {
    best=3;
    bestSize=size;
  }
  BddExchangeVarBlocks(bddManager, block, i+1);
  /* 3 1 2 */
  size=bddManager->numNodes;
  if (size <= bestSize) {
    best=4;
    bestSize=size;
  }
  BddExchangeVarBlocks(bddManager, block, i);
  /* 1 3 2 */
  size=bddManager->numNodes;
  if (size <= bestSize) {
    best=5;
    bestSize=size;
  }
  switch (best){
      case 0:
        BddExchangeVarBlocks(bddManager, block, i+1);
        break;
      case 1:
        BddExchangeVarBlocks(bddManager, block, i+1);
        BddExchangeVarBlocks(bddManager, block, i);
        break;
      case 2:
        BddExchangeVarBlocks(bddManager, block, i+1);
        BddExchangeVarBlocks(bddManager, block, i);
        BddExchangeVarBlocks(bddManager, block, i+1);
        break;
      case 3:
        BddExchangeVarBlocks(bddManager, block, i);
        BddExchangeVarBlocks(bddManager, block, i+1);
        break;
      case 4:
        BddExchangeVarBlocks(bddManager, block, i);
        break;
      case 5:
        break;
  }
  return ((best > 0) && (origSize > bestSize));
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddReorderStableWindow3Aux(Cal_BddManager_t *bddManager, Cal_Block block,
                           char *levels) 
{
  long i;
  int moved;
  int anySwapped;

  if (block->reorderable) {
    for (i=0; i < block->numChildren-1; ++i) levels[i]=1;
    do {
	  anySwapped=0;
	  for (i=0; i < block->numChildren-1; i++){
	    if (levels[i]){
#ifdef _CAL_VERBOSE
          fprintf(stdout,"Moving block %3d -- %3d\n",
                  bddManager->indexToId[block->children[i]-> firstIndex],
                  bddManager->indexToId[block->children[i]->lastIndex]);
          fflush(stdout);
          for (j=0; j<bddManager->numVars; j++){
            fprintf(stdout, "%3d ", bddManager->indexToId[j]);
          }
          fprintf(stdout, "%8d\n", bddManager->numNodes);
#endif
          if (i < block->numChildren-2){
            moved = BddReorderWindow3(bddManager, block, i);
          }
          else{
            moved = BddReorderWindow2(bddManager, block, i);
          }
          if (moved){
		    if (i > 0) {
              levels[i-1]=1;
              if (i > 1)
                levels[i-2]=1;
            }
		    levels[i]=1;
		    levels[i+1]=1;
		    if (i < block->numChildren-2){
              levels[i+2]=1;
              if (i < block->numChildren-3) {
			    levels[i+3]=1;
			    if (i < block->numChildren-4) levels[i+4]=1;
			  }
            }
		    anySwapped=1;
          }
          else levels[i]=0;
        }
      }
    } while (anySwapped);
  }
  for (i=0; i < block->numChildren; ++i){
    BddReorderStableWindow3Aux(bddManager, block->children[i], levels);
  }
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddReorderStableWindow3(Cal_BddManager_t *bddManager)
{
  char *levels;
  levels =  Cal_MemAlloc(char, bddManager->numVars);
  bddManager->numSwaps = 0;
  BddReorderStableWindow3Aux(bddManager, bddManager->superBlock, levels);
  Cal_MemFree(levels);
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddSiftBlock(Cal_BddManager_t *bddManager, Cal_Block block, long
             startPosition, double maxSizeFactor)
{
  long startSize;
  long bestSize;
  long bestPosition;
  long currentSize;
  long currentPosition;
  long maxSize;
  
  startSize = bddManager->numNodes;
  bestSize = startSize;
  bestPosition = startPosition;
  currentSize = startSize;
  currentPosition = startPosition;
  maxSize = maxSizeFactor*startSize;
  if (bddManager->nodeLimit && maxSize > bddManager->nodeLimit)
    maxSize = bddManager->nodeLimit;

  /* Need to do optimization here */
  if (startPosition > (block->numChildren >> 1)){
    while (currentPosition < block->numChildren-1 && currentSize <= maxSize){
      BddExchangeVarBlocks(bddManager, block, currentPosition);
      ++currentPosition;
      currentSize = bddManager->numNodes;
      if (currentSize < bestSize){
        bestSize = currentSize;
        bestPosition=currentPosition;
      }
    }
    while (currentPosition != startPosition){
      --currentPosition;
      BddExchangeVarBlocks(bddManager, block, currentPosition);
    }
    currentSize = startSize;
    while (currentPosition && currentSize <= maxSize){
      --currentPosition;
      BddExchangeVarBlocks(bddManager, block, currentPosition);
      currentSize = bddManager->numNodes;
      if (currentSize <= bestSize){
        bestSize = currentSize;
        bestPosition = currentPosition;
      }
    }
    while (currentPosition != bestPosition){
      BddExchangeVarBlocks(bddManager, block, currentPosition);
      ++currentPosition;
    }
  }
  else{
    while (currentPosition && currentSize <= maxSize){
      --currentPosition;
      BddExchangeVarBlocks(bddManager, block, currentPosition);
      currentSize = bddManager->numNodes;
      if (currentSize < bestSize){
        bestSize = currentSize;
        bestPosition = currentPosition;
      }
    }
    while (currentPosition != startPosition){
      BddExchangeVarBlocks(bddManager, block, currentPosition);
      ++currentPosition;
    }
    currentSize = startSize;
    while (currentPosition < block->numChildren-1 && currentSize <= maxSize){
      BddExchangeVarBlocks(bddManager, block, currentPosition);
      currentSize = bddManager->numNodes;
      ++currentPosition;
      if (currentSize <= bestSize){
        bestSize = currentSize;
        bestPosition = currentPosition;
      }
    }
    while (currentPosition != bestPosition){
      --currentPosition;
      BddExchangeVarBlocks(bddManager, block, currentPosition);
    }
  }
}



/**Function********************************************************************

  Synopsis    [Reorder variables using "sift" algorithm.]

  Description [Reorder variables using "sift" algorithm.]

  SideEffects [None]

******************************************************************************/
static void
BddReorderSiftAux(Cal_BddManager_t *bddManager, Cal_Block block,
                     Cal_Block *toSift, double maxSizeFactor) 
{
  long i, j, k;
  long width;
  long maxWidth;
  long widest;
  long numVarsShifted = 0;
  bddManager->numSwaps = 0;
  if (block->reorderable) {
    for (i=0; i < block->numChildren; ++i){
      toSift[i] = block->children[i];
    }
    while (i &&
           (numVarsShifted <=
            bddManager->maxNumVarsSiftedPerReordering) &&
           (bddManager->numSwaps <=
            bddManager->maxNumSwapsPerReordering)){ 
	  i--;
      numVarsShifted++;
	  maxWidth = 0;
	  widest = 0;
	  for (j=0; j <= i; ++j) {
        for (width=0, k=toSift[j]->firstIndex; k <= toSift[j]->lastIndex; ++k){
          width +=
              bddManager->uniqueTable[bddManager->indexToId[k]]->numEntries; 
        }
        width /= toSift[j]->lastIndex - toSift[j]->firstIndex+1;
        if (width > maxWidth) {
		  maxWidth = width;
		  widest = j;
		}
      }
	  if (maxWidth > 1) {
        for (j=0; block->children[j] != toSift[widest]; ++j);
#ifdef _CAL_VERBOSE
        fprintf(stdout,"Moving block %3d -- %3d\n",
                bddManager->indexToId[block->children[j]-> firstIndex],
                bddManager->indexToId[block->children[j]->lastIndex]);
        fflush(stdout);
        for (l=0; l<bddManager->numVars; l++){
          fprintf(stdout, "%3d ", bddManager->indexToId[l]);
        }
        fprintf(stdout, "%8d\n", bddManager->numNodes);
#endif
        BddSiftBlock(bddManager, block, j, maxSizeFactor);
        toSift[widest] = toSift[i];
      }
	  else {
        break;
      }
	}
  }
  for (i=0; i < block->numChildren; ++i)
    BddReorderSiftAux(bddManager, block->children[i], toSift,
                      maxSizeFactor);  
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddReorderSift(Cal_BddManager_t *bddManager, double maxSizeFactor)
{
  Cal_Block *toSift;

  toSift = Cal_MemAlloc(Cal_Block, bddManager->numVars);
  BddReorderSiftAux(bddManager, bddManager->superBlock, toSift,
                       maxSizeFactor); 
  Cal_MemFree(toSift);
}




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
