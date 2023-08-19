/**CFile***********************************************************************

  FileName    [calHashTable.c]

  PackageName [cal]

  Synopsis    [Functions to manage the hash tables that are a part of
                  1. unique table
                  2. request queue
               ]

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

  Revision    [$Id: calHashTable.c,v 1.8 1998/09/16 00:32:36 ravi Exp $]

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


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************
  Initialize a hash table using default parameters.
******************************************************************************/
CalHashTable_t *
CalHashTableInit(Cal_BddManager_t *bddManager, Cal_BddId_t  bddId)
{
  CalHashTable_t *hashTable;

  hashTable = Cal_MemAlloc(CalHashTable_t, 1);
  /*hashTable = CAL_BDD_NEW_REC(bddManager, CalHashTable_t);*/
  if(hashTable == Cal_Nil(CalHashTable_t)){
    CalBddFatalMessage("out of memory");
  }
  hashTable->sizeIndex = HASH_TABLE_DEFAULT_SIZE_INDEX;
  hashTable->numBins = TABLE_SIZE(hashTable->sizeIndex);
  hashTable->maxCapacity = hashTable->numBins*HASH_TABLE_DEFAULT_MAX_DENSITY;
  hashTable->bins = Cal_MemAlloc(CalBddNode_t *, hashTable->numBins);
  if(hashTable->bins == Cal_Nil(CalBddNode_t *)){
    CalBddFatalMessage("out of memory");
  }
  memset((char *)hashTable->bins, 0,
         hashTable->numBins*sizeof(CalBddNode_t *));
  hashTable->bddId = bddId;
  hashTable->nodeManager = bddManager->nodeManagerArray[bddId];
  hashTable->requestNodeList = Cal_Nil(CalRequestNode_t);
  memset((char *)(&(hashTable->startNode)), 0, sizeof(CalBddNode_t));
  hashTable->endNode = &(hashTable->startNode);
  hashTable->numEntries = 0;
  return hashTable;
}


/**Function********************************************************************
  Free a hash table along with the associated storage.
******************************************************************************/
int
CalHashTableQuit(Cal_BddManager_t *bddManager, CalHashTable_t * hashTable)
{
  if(hashTable == Cal_Nil(CalHashTable_t))return 1;
  /*
  for(i = 0; i < hashTable->numBins; i++){
    ptr = hashTable->bins[i];
    while(ptr != Cal_Nil(CalBddNode_t)){
      next = CalBddNodeGetNextBddNode(ptr);
      CalNodeManagerFreeNode(hashTable->nodeManager, ptr);
      ptr = next;
    }
  }
  There is no need to free the nodes individually. They will be taken
  care of by the PageManagerQuit.
  We need to make sure that this function is called only during the global quitting.
  If it need be called at some intermediate point, we need to free the BDD nodes
  appropriately.
  */

  Cal_MemFree(hashTable->bins);
  Cal_MemFree(hashTable);
  /*CAL_BDD_FREE_REC(bddManager, hashTable, CalHashTable_t);*/
  return 0;
}



/**Function********************************************************************
  Directly insert a BDD node in the hash table.
******************************************************************************/
void
CalHashTableAddDirect(CalHashTable_t * hashTable, CalBddNode_t * bddNode)
{
  int hashValue;
  CalBddNode_t *thenBddNode, *elseBddNode;

  hashTable->numEntries++;
  if(hashTable->numEntries >= hashTable->maxCapacity){
    CalHashTableRehash(hashTable, 1);
  }
  thenBddNode = CalBddNodeGetThenBddNode(bddNode);
  elseBddNode = CalBddNodeGetElseBddNode(bddNode);
  hashValue = CalDoHash2(thenBddNode, elseBddNode, hashTable);
  CalBddNodePutNextBddNode(bddNode, hashTable->bins[hashValue]);
  hashTable->bins[hashValue] = bddNode;
}


/**Function********************************************************************
******************************************************************************/
int
CalHashTableFindOrAdd(CalHashTable_t * hashTable,
                      Cal_Bdd_t  thenBdd,
                      Cal_Bdd_t  elseBdd,
                      Cal_Bdd_t * bddPtr)
{
  CalBddNode_t *ptr;
  Cal_Bdd_t tmpBdd;
  int hashValue;

  hashValue = CalDoHash2(CalBddGetBddNode(thenBdd),
      CalBddGetBddNode(elseBdd), hashTable);
  ptr = hashTable->bins[hashValue];
  while(ptr != Cal_Nil(CalBddNode_t)){
    CalBddNodeGetThenBdd(ptr, tmpBdd);
    if(CalBddIsEqual(thenBdd, tmpBdd)){
      CalBddNodeGetElseBdd(ptr, tmpBdd);
      if(CalBddIsEqual(elseBdd, tmpBdd)){
        CalBddPutBddId(*bddPtr, hashTable->bddId);
        CalBddPutBddNode(*bddPtr, ptr);
        return 1;
      }
    }
    ptr = CalBddNodeGetNextBddNode(ptr);
  }
  hashTable->numEntries++;
  if(hashTable->numEntries > hashTable->maxCapacity){
    CalHashTableRehash(hashTable,1);
    hashValue = CalDoHash2(CalBddGetBddNode(thenBdd),
        CalBddGetBddNode(elseBdd), hashTable);
  }
  CalNodeManagerInitBddNode(hashTable->nodeManager, thenBdd, elseBdd,
      hashTable->bins[hashValue], ptr);
  hashTable->bins[hashValue] = ptr;
  CalBddPutBddId(*bddPtr, hashTable->bddId);
  CalBddPutBddNode(*bddPtr, ptr);
  return 0;
}

/**Function********************************************************************
******************************************************************************/
int
CalHashTableAddDirectAux(CalHashTable_t * hashTable, Cal_Bdd_t
                         thenBdd, Cal_Bdd_t  elseBdd, Cal_Bdd_t *
                         bddPtr)
{
  CalBddNode_t *ptr;
  int hashValue;

  hashTable->numEntries++;
  if(hashTable->numEntries >= hashTable->maxCapacity){
    CalHashTableRehash(hashTable, 1);
  }
  hashValue = CalDoHash2(CalBddGetBddNode(thenBdd), CalBddGetBddNode(elseBdd),
                         hashTable);
  CalNodeManagerInitBddNode(hashTable->nodeManager, thenBdd, elseBdd,
      hashTable->bins[hashValue], ptr);
  hashTable->bins[hashValue] = ptr;
  CalBddPutBddId(*bddPtr, hashTable->bddId);
  CalBddPutBddNode(*bddPtr, ptr);
  return 0;
}

/**Function********************************************************************
******************************************************************************/
void
CalHashTableCleanUp(CalHashTable_t * hashTable)
{
  CalNodeManager_t *nodeManager;

  nodeManager = hashTable->nodeManager;
  hashTable->endNode->nextBddNode = nodeManager->freeNodeList;
  nodeManager->freeNodeList = hashTable->startNode.nextBddNode;
  hashTable->endNode = &(hashTable->startNode);
  hashTable->numEntries = 0;
  hashTable->startNode.nextBddNode = NULL;
  Cal_Assert(!(hashTable->requestNodeList));
  hashTable->requestNodeList = Cal_Nil(CalRequestNode_t);
  return;
}


/**Function********************************************************************
******************************************************************************/
int
CalHashTableLookup(
  CalHashTable_t * hashTable,
  Cal_Bdd_t  thenBdd,
  Cal_Bdd_t  elseBdd,
  Cal_Bdd_t * bddPtr)
{
  CalBddNode_t *ptr;
  Cal_Bdd_t tmpBdd;
  int hashValue;

  hashValue = CalDoHash2(CalBddGetBddNode(thenBdd),
      CalBddGetBddNode(elseBdd), hashTable);
  ptr = hashTable->bins[hashValue];
  while(ptr != Cal_Nil(CalBddNode_t)){
    CalBddNodeGetThenBdd(ptr, tmpBdd);
    if(CalBddIsEqual(thenBdd, tmpBdd)){
      CalBddNodeGetElseBdd(ptr, tmpBdd);
      if(CalBddIsEqual(elseBdd, tmpBdd)){
        CalBddPutBddId(*bddPtr, hashTable->bddId);
        CalBddPutBddNode(*bddPtr, ptr);
        return 1;
      }
    }
    ptr = CalBddNodeGetNextBddNode(ptr);
  }
  return 0;
}

/**Function********************************************************************
  Deletes a BDD node in the hash table.
******************************************************************************/
void
CalHashTableDelete(CalHashTable_t * hashTable, CalBddNode_t * bddNode)
{
  int hashValue;
  Cal_Bdd_t thenBdd, elseBdd;
  CalBddNode_t  *ptr, *last;

  CalBddNodeGetThenBdd(bddNode, thenBdd);
  CalBddNodeGetElseBdd(bddNode, elseBdd);
  hashValue =
      CalDoHash2(CalBddGetBddNode(thenBdd), CalBddGetBddNode(elseBdd), hashTable);

  last = Cal_Nil(CalBddNode_t);
  ptr = hashTable->bins[hashValue];
  while(ptr != Cal_Nil(CalBddNode_t)){
    if(ptr == bddNode){
      if(last == Cal_Nil(CalBddNode_t)){
        hashTable->bins[hashValue] = CalBddNodeGetNextBddNode(ptr);
      }
      else{
        CalBddNodePutNextBddNode(last, CalBddNodeGetNextBddNode(ptr));
      }
      hashTable->numEntries--;
      CalNodeManagerFreeNode(hashTable->nodeManager, ptr);
      return;
    }
    last = ptr;
    ptr = CalBddNodeGetNextBddNode(ptr);
  }
  CalBddWarningMessage("Trying to delete a non-existent node\n");
}


/**Function********************************************************************
  Lookup unique table for id.
******************************************************************************/
int
CalUniqueTableForIdLookup(
  Cal_BddManager_t * bddManager,
  CalHashTable_t * hashTable,
  Cal_Bdd_t  thenBdd,
  Cal_Bdd_t  elseBdd,
  Cal_Bdd_t * bddPtr)
{
  CalBddNode_t *ptr;
  Cal_Bdd_t tmpBdd;
  int hashValue;

  hashValue = CalDoHash2(CalBddGetBddNode(thenBdd),
      CalBddGetBddNode(elseBdd), hashTable);
  ptr = hashTable->bins[hashValue];
  if(CalBddIsOutPos(thenBdd)){
    while(ptr != Cal_Nil(CalBddNode_t)){
      CalBddNodeGetThenBdd(ptr, tmpBdd);
      if(CalBddIsEqual(thenBdd, tmpBdd)){
        CalBddNodeGetElseBdd(ptr, tmpBdd);
        if(CalBddIsEqual(elseBdd, tmpBdd)){
          CalBddPutBddId(*bddPtr, hashTable->bddId);
          CalBddPutBddNode(*bddPtr, ptr);
          return 1;
        }
      }
      ptr = CalBddNodeGetNextBddNode(ptr);
    }
  }
  else{
    CalBddNot(thenBdd, thenBdd);
    CalBddNot(elseBdd, elseBdd);
    while(ptr != Cal_Nil(CalBddNode_t)){
      CalBddNodeGetThenBdd(ptr, tmpBdd);
      if(CalBddIsEqual(thenBdd, tmpBdd)){
        CalBddNodeGetElseBdd(ptr, tmpBdd);
        if(CalBddIsEqual(elseBdd, tmpBdd)){
          CalBddPutBddId(*bddPtr, hashTable->bddId);
          CalBddPutBddNode(*bddPtr, CalBddNodeNot(ptr));
          return 1;
        }
      }
      ptr = CalBddNodeGetNextBddNode(ptr);
    }
  }
  return 0;
}


/**Function********************************************************************
  Find or add in the unique table for id.

  If a new BDD node is created (found == false), then the numNodes field of the
  manager needs to be incremented.
******************************************************************************/
int
CalUniqueTableForIdFindOrAdd(
  Cal_BddManager_t * bddManager,
  CalHashTable_t * hashTable,
  Cal_Bdd_t  thenBdd,
  Cal_Bdd_t  elseBdd,
  Cal_Bdd_t * bddPtr)
{
  int found = 0;
  if (CalBddIsEqual(thenBdd, elseBdd)){
    *bddPtr = thenBdd;
    found = 1;
  }
  else if(CalBddIsOutPos(thenBdd)){
    found = CalHashTableFindOrAdd(hashTable, thenBdd, elseBdd, bddPtr);
  }
  else{
    CalBddNot(thenBdd, thenBdd);
    CalBddNot(elseBdd, elseBdd);
    found = CalHashTableFindOrAdd(hashTable, thenBdd, elseBdd, bddPtr);
    CalBddNot(*bddPtr, *bddPtr);
  }
  if (!found) bddManager->numNodes++;
  return found;
}

/**Function********************************************************************
******************************************************************************/
void
CalHashTableRehash(CalHashTable_t *hashTable,int grow)
{
  CalBddNode_t *ptr, *next;
  CalBddNode_t **oldBins = hashTable->bins;
  int i, hashValue;
  int oldNumBins = hashTable->numBins;

  if(grow){
    hashTable->sizeIndex++;
  }
  else{
    if (hashTable->sizeIndex <= HASH_TABLE_DEFAULT_SIZE_INDEX){/* No need to rehash */
      return;
    }
    hashTable->sizeIndex--;
  }

  hashTable->numBins = TABLE_SIZE(hashTable->sizeIndex);
  hashTable->maxCapacity = hashTable->numBins * HASH_TABLE_DEFAULT_MAX_DENSITY;
  hashTable->bins = Cal_MemAlloc(CalBddNode_t *, hashTable->numBins);
  if(hashTable->bins == Cal_Nil(CalBddNode_t *)){
    CalBddFatalMessage("out of memory");
  }
  /*
  for(i = 0; i < hashTable->numBins; i++){
    hashTable->bins[i] = Cal_Nil(CalBddNode_t);
  }
  */
  memset((char *)hashTable->bins, 0,
         hashTable->numBins*sizeof(CalBddNode_t *));

  for(i = 0; i < oldNumBins; i++){
    ptr = oldBins[i];
    while(ptr != Cal_Nil(CalBddNode_t)){
      next = CalBddNodeGetNextBddNode(ptr);
      hashValue = CalDoHash2(CalBddNodeGetThenBddNode(ptr),
          CalBddNodeGetElseBddNode(ptr), hashTable);
      CalBddNodePutNextBddNode(ptr, hashTable->bins[hashValue]);
      hashTable->bins[hashValue] = ptr;
      ptr = next;
    }
  }
  Cal_MemFree(oldBins);
}

/**Function********************************************************************
******************************************************************************/
void
CalUniqueTableForIdRehashNode(CalHashTable_t *hashTable, CalBddNode_t *bddNode,
                              CalBddNode_t *thenBddNode,
                              CalBddNode_t *elseBddNode)

{
  CalBddNode_t *nextBddNode;
  CalBddNode_t *ptr;
  int found;
  int hashValue;
  int oldHashValue;
  Cal_Bdd_t thenBdd;

  oldHashValue = CalDoHash2(thenBddNode, elseBddNode, hashTable);
  hashValue = CalDoHash2(CalBddNodeGetThenBddNode(bddNode),
                         CalBddNodeGetElseBddNode(bddNode),
                         hashTable);
  CalBddNodeGetThenBdd(bddNode, thenBdd);
  if (CalBddIsComplement(thenBdd)) {
    CalBddFatalMessage("Complement edge on then pointer");
  }
  if (oldHashValue == hashValue) {
    return;
  }

  found = 0;
  ptr = hashTable->bins[oldHashValue];
  if ((ptr != Cal_Nil(CalBddNode_t)) && (ptr == bddNode)) {
    hashTable->bins[oldHashValue] = CalBddNodeGetNextBddNode(bddNode);
    found = 1;
  } else {
    while (ptr != Cal_Nil(CalBddNode_t)) {
      nextBddNode = CalBddNodeGetNextBddNode(ptr);
      if (nextBddNode == bddNode) {
        CalBddNodePutNextBddNode(ptr, CalBddNodeGetNextBddNode(bddNode));
        found = 1;
        break;
      }
      ptr = nextBddNode;
    }
  }

  if (!found) {
    CalBddFatalMessage("Node not found in the unique table");
  } else {
    CalBddNodePutNextBddNode(bddNode, hashTable->bins[hashValue]);
    hashTable->bins[hashValue] = bddNode;
  }
}

/**Function********************************************************************
******************************************************************************/
unsigned long
CalBddUniqueTableNumLockedNodes(Cal_BddManager_t *bddManager,
                                CalHashTable_t *uniqueTableForId)
{
  CalBddNode_t *bddNode;
  long i;
  unsigned long numLockedNodes = 0;

  for(i=0; i<uniqueTableForId->numBins; i++){
    bddNode = uniqueTableForId->bins[i];
    while (bddNode){
      numLockedNodes += CalBddNodeIsRefCountMax(bddNode);
      bddNode = CalBddNodeGetNextBddNode(bddNode);
    }
  }
  return numLockedNodes;
}

/**Function********************************************************************
******************************************************************************/
void
CalPackNodes(Cal_BddManager_t *bddManager)
{
  int index, id;
  CalNodeManager_t *nodeManager;
  CalHashTable_t *uniqueTableForId;

  for (index = bddManager->numVars-1; index >= 0; index--){
    id = bddManager->indexToId[index];
    nodeManager = bddManager->nodeManagerArray[id];
    uniqueTableForId = bddManager->uniqueTable[id];
    CalBddPackNodesForSingleId(bddManager, id);
  }
}

/**Function********************************************************************
******************************************************************************/
void
CalBddPackNodesForSingleId(Cal_BddManager_t *bddManager,
                           Cal_BddId_t id)
{
  /* Need to copy the one for "AfterReorder" and suitably modify. */
}

/**Function********************************************************************
  Packs the nodes if the variables which has just been sifted.

  fixForwardedNodesFlag: Whether we need to fix the forwarded nodes of variables
  corresponding to bestIndex through bottomIndex. If this flag is set, then the
  forwarded nodes of these variables are traversed and updated after the nodes
  of the bestIndex have been copied. At the end the forwarded nodes are freed.
  If this flag is not set, it is assumed that the cleanup pass has already been
  performed.
******************************************************************************/
void
CalBddPackNodesAfterReorderForSingleId(Cal_BddManager_t *bddManager,
                                       int fixForwardedNodesFlag,
                                       int bestIndex,
                                       int bottomIndex)
{
  /* We need to pack the nodes for this id and fix the cofactors of
     the upper indices.
     */
  CalBddNode_t *node, *nextBddNode, *dupNode, **oldBins;
  CalBddNode_t *thenBddNode, *elseBddNode, *bddNode;
  Cal_Bdd_t thenBdd;
  CalAddress_t *page;
  int id = bddManager->indexToId[bestIndex];
  CalNodeManager_t *nodeManager = bddManager->nodeManagerArray[id];
  CalAddress_t **oldPageList = nodeManager->pageList;
  int oldNumPages = nodeManager->numPages;
  CalHashTable_t *uniqueTableForId = bddManager->uniqueTable[id];
  int numPagesRequired, newSizeIndex, index, i;
  long oldNumBins, hashValue;


#ifdef _CAL_VERBOSE
  fprintf(stdout,"Repacking id %3d\n", id);
#endif


  nodeManager->freeNodeList = Cal_Nil(CalBddNode_t);
  nodeManager->numPages = 0;
  numPagesRequired = uniqueTableForId->numEntries/NUM_NODES_PER_PAGE;
  nodeManager->maxNumPages =
      2*(numPagesRequired ? numPagesRequired : 1);

  nodeManager->pageList = Cal_MemAlloc(CalAddress_t *,
                                       nodeManager->maxNumPages);

  oldBins = uniqueTableForId->bins;
  oldNumBins = uniqueTableForId->numBins;
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

  memset((char *)uniqueTableForId->bins, 0,
        uniqueTableForId->numBins*sizeof(CalBddNode_t *));

  for (i = 0; i < oldNumBins; i++){
    node = oldBins[i];
    while (node){
      nextBddNode = CalBddNodeGetNextBddNode(node);
      CalNodeManagerCreateAndDupBddNode(nodeManager, node, dupNode);
      thenBddNode = CalBddNodeGetThenBddNode(dupNode);
      elseBddNode = CalBddNodeGetElseBddNode(dupNode);
      hashValue = CalDoHash2(thenBddNode, elseBddNode, uniqueTableForId);
      CalBddNodePutNextBddNode(dupNode, uniqueTableForId->bins[hashValue]);
      uniqueTableForId->bins[hashValue] = dupNode;
      CalBddNodePutThenBddNode(node, dupNode);
      CalBddNodePutThenBddId(node, id);
      CalBddNodePutElseBddNode(node, FORWARD_FLAG);
      node = nextBddNode;
      Cal_Assert(!(CalBddNodeIsRefCountZero(dupNode)));
    }
  }

  if (fixForwardedNodesFlag){
      CalBddNode_t *requestNodeList =
          bddManager->uniqueTable[id]->startNode.nextBddNode;
      for (bddNode = requestNodeList; bddNode; bddNode = nextBddNode){
        Cal_Assert(CalBddNodeIsForwarded(bddNode));
        nextBddNode = CalBddNodeGetNextBddNode(bddNode);
        CalBddNodeGetThenBdd(bddNode, thenBdd);
        if (CalBddGetBddId(thenBdd) == id){
          if (CalBddIsForwarded(thenBdd)) {
            CalBddForward(thenBdd);
            Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
            CalBddNodePutThenBdd(bddNode, thenBdd);
          }
        }
        Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
      }
      for (index = bestIndex+1; index <= bottomIndex; index++){
      int varId = bddManager->indexToId[index];
      requestNodeList =
          bddManager->uniqueTable[varId]->startNode.nextBddNode;
      for (bddNode = requestNodeList; bddNode; bddNode = nextBddNode){
        Cal_Assert(CalBddNodeIsForwarded(bddNode));
        nextBddNode = CalBddNodeGetNextBddNode(bddNode);
        CalBddNodeGetThenBdd(bddNode, thenBdd);
        if (CalBddIsForwarded(thenBdd)) {
          CalBddForward(thenBdd);
          Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
          CalBddNodePutThenBdd(bddNode, thenBdd);
        }
        Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
      }
    }
  }

/* Traverse the upper indices fixing the cofactors */
  for (index = bestIndex-1; index >= 0; index--){
    CalBddReorderFixCofactors(bddManager,
                           bddManager->indexToId[index]);
  }

  if (bddManager->pipelineState == CREATE){
    /* There are some results computed in pipeline */
    CalBddReorderFixProvisionalNodes(bddManager);
  }

  /* Fix the user BDDs */
  CalBddReorderFixUserBddPtrs(bddManager);

  CalBddIsForwardedTo(bddManager->varBdds[id]);

  /* Fix the association */
  CalReorderAssociationFix(bddManager);

  /* Free the old bins */
  Cal_MemFree(oldBins);

  uniqueTableForId->endNode = &(uniqueTableForId->startNode);
  uniqueTableForId->startNode.nextBddNode = NULL;
  if (fixForwardedNodesFlag){
    CalBddReorderReclaimForwardedNodes(bddManager, bestIndex+1,
                                    bottomIndex);
  }
  /* Free the old pages */
  for (i = 0; i < oldNumPages; i++){
    page = oldPageList[i];
    CalPageManagerFreePage(nodeManager->pageManager, page);
  }
  Cal_MemFree(oldPageList);
  Cal_Assert(CalCheckAllValidity(bddManager));
}

/**Function********************************************************************
******************************************************************************/
void
CalBddPackNodesForMultipleIds(Cal_BddManager_t *bddManager,
                              Cal_BddId_t beginId, int numLevels)
{
  /* We need to pack the nodes for this id and fix the cofactors of
     the upper indices.
     */
  int index = bddManager->idToIndex[beginId];
  int level, id;
  long i, j;
  CalBddNode_t *node, *nextBddNode, *dupNode, *thenBddNode;
  CalBddNode_t *elseBddNode, **oldBins;
  Cal_Bdd_t thenBdd, elseBdd;
  CalNodeManager_t *nodeManager;
  CalHashTable_t *uniqueTableForId;
  int someRepackingDone = 0;
  long oldNumBins, hashValue;
  int newSizeIndex;


  CalAddress_t *page, ***oldPageListArray, **oldPageList;
  int *oldNumPagesArray;
  int numPagesRequired;

  oldPageListArray = Cal_MemAlloc(CalAddress_t **, numLevels);

  oldNumPagesArray = Cal_MemAlloc(int, numLevels);

  for (level = numLevels-1; level >= 0; level--){
    id = bddManager->indexToId[index+level];
    oldNumPagesArray[level] = 0;
    oldPageListArray[level] = Cal_Nil(CalAddress_t *);
    if (CalBddIdNeedsRepacking(bddManager, id)){
      nodeManager = bddManager->nodeManagerArray[id];
      uniqueTableForId = bddManager->uniqueTable[id];
      oldPageListArray[level] = nodeManager->pageList;
      oldNumPagesArray[level] = nodeManager->numPages;
      nodeManager->freeNodeList = Cal_Nil(CalBddNode_t);
      nodeManager->numPages = 0;
      numPagesRequired = uniqueTableForId->numEntries/NUM_NODES_PER_PAGE;
      nodeManager->maxNumPages =
          2*(numPagesRequired ? numPagesRequired : 1);
      nodeManager->pageList = Cal_MemAlloc(CalAddress_t *,
                                           nodeManager->maxNumPages);
      oldBins = uniqueTableForId->bins;
      oldNumBins = uniqueTableForId->numBins;
      /* Create the new set of bins */
      newSizeIndex =
          CeilLog2(uniqueTableForId->numEntries /
                   HASH_TABLE_DEFAULT_MAX_DENSITY);
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
      memset((char *)uniqueTableForId->bins, 0,
            uniqueTableForId->numBins*sizeof(CalBddNode_t *));

      for (i = 0; i < oldNumBins; i++){
        node = oldBins[i];
        while (node){
          nextBddNode = CalBddNodeGetNextBddNode(node);
          CalBddNodeGetThenBdd(node, thenBdd);
          CalBddNodeGetElseBdd(node, elseBdd);
          if (CalBddIsForwarded(thenBdd)){
            CalBddForward(thenBdd);
            CalBddNodePutThenBdd(node, thenBdd);
          }
          if (CalBddIsForwarded(elseBdd)){
            CalBddForward(elseBdd);
            CalBddNodePutElseBdd(node, elseBdd);
          }
          CalNodeManagerCreateAndDupBddNode(nodeManager, node, dupNode);
          thenBddNode = CalBddNodeGetThenBddNode(dupNode);
          elseBddNode = CalBddNodeGetElseBddNode(dupNode);
          hashValue = CalDoHash2(thenBddNode, elseBddNode, uniqueTableForId);
          CalBddNodePutNextBddNode(dupNode, uniqueTableForId->bins[hashValue]);
          uniqueTableForId->bins[hashValue] = dupNode;
          CalBddNodePutThenBddNode(node, dupNode);
          CalBddNodePutThenBddId(node, id);
          CalBddNodePutElseBddNode(node, FORWARD_FLAG);
          node = nextBddNode;
          Cal_Assert(!(CalBddNodeIsRefCountZero(dupNode)));
        }
      }

#ifdef __FOO__
      /*fprintf(stdout,"Repacking id = %d, index = %d\n", id, index+level);*/
      /* First put all the nodes in that list */
      nodeList = Cal_Nil(CalBddNode_t);
      for (i = 0; i < uniqueTableForId->numBins; i++){
        node = uniqueTableForId->bins[i];
        while (node){
          nextBddNode = CalBddNodeGetNextBddNode(node);
          /* The "then" and "else" pointers could be forwarded */
          CalBddNodeGetThenBdd(node, thenBdd);
          CalBddNodeGetElseBdd(node, elseBdd);
          if (CalBddIsForwarded(thenBdd)){
            CalBddForward(thenBdd);
            CalBddNodePutThenBdd(node, thenBdd);
          }
          if (CalBddIsForwarded(elseBdd)){
            CalBddForward(elseBdd);
            CalBddNodePutElseBdd(node, elseBdd);
          }
          CalBddNodePutNextBddNode(node, nodeList);
          nodeList = node;
          node = nextBddNode;
        }
        uniqueTableForId->bins[i] = Cal_Nil(CalBddNode_t);
      }
      uniqueTableForId->numEntries = 0;

      for (node = nodeList; node; node = nextBddNode){
        nextBddNode = CalBddNodeGetNextBddNode(node);
        CalNodeManagerCreateAndDupBddNode(nodeManager, node, dupNode);
        /* Hash the dupNode */
        CalHashTableAddDirect(uniqueTableForId, dupNode);
        /* Make the original node a forwarding node */
        CalBddNodePutThenBddNode(node, dupNode);
        CalBddNodePutThenBddId(node, id);
        CalBddNodePutElseBddNode(node, FORWARD_FLAG);
      }
#endif
      someRepackingDone = 1;
    }
    else if (someRepackingDone){ /* Still need to fix the cofactors */
      CalBddReorderFixCofactors(bddManager, id);
    }
  }


  /* Traverse the upper indices fixing the cofactors */
  for (i = index-1; i >= 0; i--){
    CalBddReorderFixCofactors(bddManager,
                              bddManager->indexToId[i]);
  }

  /* Fix the user BDDs */
  CalBddReorderFixUserBddPtrs(bddManager);
  if (bddManager->pipelineState == CREATE){
    /* There are some results computed in pipeline */
    CalBddReorderFixProvisionalNodes(bddManager);
  }
  /* Fix Cache Tables */
  (void)CalCacheTableTwoRepackUpdate(bddManager->cacheTable);

  for (level = numLevels - 1 ; level >= 0; level--){
    id = bddManager->indexToId[index+level];
    /* Update varBdd field of bdd manager */
    CalBddIsForwardedTo(bddManager->varBdds[id]);
    /* Fix associations */
    CalVarAssociationRepackUpdate(bddManager, id);
    /* Free the old pages */
    nodeManager = bddManager->nodeManagerArray[id];
    oldPageList = oldPageListArray[level];
    for (j = 0; j < oldNumPagesArray[level]; j++){
      page = oldPageList[j];
      CalPageManagerFreePage(nodeManager->pageManager, page);
    }
    if ((unsigned long)oldPageList) Cal_MemFree(oldPageList);
  }
  Cal_MemFree(oldPageListArray);
  Cal_MemFree(oldNumPagesArray);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************
  Returns the smallest integer greater than or equal to log2 of a number (The
  assumption is that the number is >= 1)
******************************************************************************/
static int
CeilLog2(
  int  number)
{
  int num, count;
  for (num=number, count=0; num > 1; num >>= 1, count++);
  if ((1 << count) != number) count++;
  return count;
}
