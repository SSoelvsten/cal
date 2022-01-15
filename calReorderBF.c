/**CFile***********************************************************************

  FileName    [calReorderBF.c]

  PackageName [cal]

  Synopsis    [Routines for dynamic reordering of variables.]

  Description [This method dynamically reorders variables while
  preserving their locality. This entails both memory and
  computational overheads.  Conceptually and experimentally it has
  been observed that these overheads lead to poorer performance
  compared to the traditional reordering methods. For details, please
  refer to the work by Rajeev K. Ranjan et al - "Dynamic variable
  reordering in a breadth-first manipulation based package: Challenges
  and Solutions"- Proceedings of ICCD'97.]

  SeeAlso     [calReorderDF.c calReorderUtil.c]

  Author      [Rajeev K. Ranjan   (rajeev@ic.eecs.berkeley.edu)
               Wilsin Gosti (wilsin@ic.eecs.berkeley.edu)]
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

  Revision    [$Id: calReorderBF.c,v 1.2 1998/09/17 02:30:21 ravi Exp $]

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

static void BddReorderFixForwardingNodes(Cal_BddManager bddManager, Cal_BddId_t id);
static void BddReorderFixAndFreeForwardingNodes(Cal_BddManager bddManager, Cal_BddId_t id, int numLevels);
static void BddReorderSwapVarIndex(Cal_BddManager_t * bddManager, int varIndex, int forwardCheckFlag);
static int CofactorFixAndReclaimForwardedNodes(Cal_BddManager_t *bddManager, int cofactorCheckStartIndex, int cofactorCheckEndIndex, int reclaimStartIndex, int reclaimEndIndex);
static void BddReorderFreeNodes(Cal_BddManager_t * bddManager, int varId);
#ifdef _CAL_VERBOSE
static void PrintBddProfileAfterReorder(Cal_BddManager_t *bddManager);
#endif
static void BddReorderVarSift(Cal_BddManager bddManager, double maxSizeFactor);
static int BddReorderSiftToBestPos(Cal_BddManager_t * bddManager, int varStartIndex, double maxSizeFactor);
static void BddSiftPerfromPhaseIV(Cal_BddManager_t *bddManager, int varStartIndex, int bestIndex, int bottomMostSwapIndex);
static void BddReorderVarWindow(Cal_BddManager bddManager, char *levels);
static int BddReorderWindow2(Cal_BddManager bddManager, long index, int directionFlag);
static int BddReorderWindow3(Cal_BddManager bddManager, long index, int directionFlag);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
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
CalBddReorderAuxBF(Cal_BddManager_t * bddManager)
{
  Cal_Assert(CalCheckAllValidity(bddManager));
  CalInitInteract(bddManager);
#ifdef _CAL_QUANTIFY_
  quantify_start_recording_data();
#endif
  if (bddManager->reorderTechnique == CAL_REORDER_WINDOW){
    char *levels = Cal_MemAlloc(char, bddManager->numVars);
    BddReorderVarWindow(bddManager, levels);
    Cal_MemFree(levels);
  }
  else {
    BddReorderVarSift(bddManager, bddManager->maxSiftingGrowth);
  }
#ifdef _CAL_QUANTIFY_
  quantify_stop_recording_data();
#endif
  Cal_Assert(CalCheckAllValidity(bddManager));
  Cal_MemFree(bddManager->interact);
  bddManager->numReorderings++;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Fixes the forwarding nodes in a unique table.]

  Description        [As opposed to CalBddReorderFixCofactors, which fixes
  the cofactors of the non-forwarding nodes, this routine traverses
  the list of forwarding nodes and removes the intermediate level of
  forwarding. Number of levels should be 1 or 2.]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddReorderFixForwardingNodes(Cal_BddManager bddManager,
                             Cal_BddId_t id) 
{
  CalHashTable_t *uniqueTableForId =
      bddManager->uniqueTable[id];
  CalBddNode_t *bddNode, *nextBddNode;
  Cal_Bdd_t thenBdd;
  
  /* These are the forwarding nodes. */
  CalBddNode_t *requestNodeList =
      uniqueTableForId->startNode.nextBddNode;
  for (bddNode = requestNodeList; bddNode; bddNode = nextBddNode){
    nextBddNode = CalBddNodeGetNextBddNode(bddNode);
    CalBddNodeGetThenBdd(bddNode, thenBdd);
    if (CalBddIsForwarded(thenBdd)) {
      CalBddForward(thenBdd);
      CalBddNodePutThenBdd(bddNode, thenBdd);
    }
    else {
      /* there should not be anymore double forwarding */
      break;
    }
    Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
  }
  /* Adjust the list */
  uniqueTableForId->endNode->nextBddNode =
      uniqueTableForId->startNode.nextBddNode;
  uniqueTableForId->startNode.nextBddNode = bddNode;
}

/**Function********************************************************************

  Synopsis           [Traverses the forwarding node lists of index,
  index+1 .. up to index+level. Frees the intermediate forwarding nodes.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddReorderFixAndFreeForwardingNodes(Cal_BddManager bddManager,
                                    Cal_BddId_t id, int numLevels)
{
  CalHashTable_t *uniqueTableForId;
  Cal_BddIndex_t index = bddManager->idToIndex[id];
  CalBddNode_t *nextBddNode, *bddNode, *endNode;
  Cal_Bdd_t thenBdd;
  CalNodeManager_t *nodeManager;
  int i;
  
  /* Fixing */
  for (i=numLevels-1; i >= 0; i--){
    uniqueTableForId =
        bddManager->uniqueTable[bddManager->indexToId[index+i]];
    for (bddNode = uniqueTableForId->startNode.nextBddNode; bddNode;
         bddNode = nextBddNode){ 
      nextBddNode = CalBddNodeGetNextBddNode(bddNode);
      CalBddNodeGetThenBdd(bddNode, thenBdd);
      if (CalBddIsForwarded(thenBdd)){
        do{
          CalBddMark(thenBdd);
          CalBddForward(thenBdd);
        } while (CalBddIsForwarded(thenBdd));
        CalBddNodePutThenBdd(bddNode, thenBdd); 
      }
      Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
    }
  }
  /* Freeing */
  for (i=numLevels-1; i >= 0; i--){
    uniqueTableForId =
        bddManager->uniqueTable[bddManager->indexToId[index+i]];
    endNode = &(uniqueTableForId->startNode);
    for (bddNode = uniqueTableForId->startNode.nextBddNode; bddNode;
         bddNode = nextBddNode){ 
      nextBddNode = CalBddNodeGetNextBddNode(bddNode);
      CalBddNodeGetThenBdd(bddNode, thenBdd);
      if (CalBddIsMarked(thenBdd)){
        do{
          /* Free the node */
          nodeManager = bddManager->nodeManagerArray[CalBddGetBddId(thenBdd)];
          CalNodeManagerFreeNode(nodeManager, CalBddGetBddNode(thenBdd));
          bddManager->numForwardedNodes--;
        } while (CalBddIsMarked(thenBdd));
      }
      else{
        endNode->nextBddNode = bddNode;
        endNode = bddNode;
      }
    }
    uniqueTableForId->endNode = endNode;
  }
}

/**Function********************************************************************

  Synopsis    [required]

  Description [Traversesoptional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddReorderSwapVarIndex(Cal_BddManager_t * bddManager, int varIndex,
                       int forwardCheckFlag)
{
  int thenVarIndex;
  int elseVarIndex;
  int varId;
  int nextVarId;
  int i;
  int numBins;
  int curSize1;
  int curSize2;
  int refCount;
  int f0Found;
  int f1Found;
  CalHashTable_t *uniqueTableForId;
  CalHashTable_t *nextUniqueTableForId;
  CalBddNode_t **bins;
  CalBddNode_t *bddNode, *startNode;
  CalBddNode_t *nextBddNode, *processingNodeList;
  CalBddNode_t *prevBddNode = Cal_Nil(CalBddNode_t);
  Cal_Bdd_t newF;
  Cal_Bdd_t f0;
  Cal_Bdd_t f1;
  Cal_Bdd_t newF0;
  Cal_Bdd_t newF1;
  Cal_Bdd_t f00;
  Cal_Bdd_t f01;
  Cal_Bdd_t f10;
  Cal_Bdd_t f11;
  CalAssociation_t *assoc;
  
  varId = bddManager->indexToId[varIndex];
  nextVarId = bddManager->indexToId[varIndex + 1];
  
  if (CalTestInteract(bddManager, varId, nextVarId)){
  bddManager->numSwaps++;
#ifdef _CAL_VERBOSE
  /*fprintf(stdout," %3d(%3d) going down,  %3d(%3d) going up\n",
          varId, varIndex, nextVarId, varIndex+1);*/
#endif
  uniqueTableForId = bddManager->uniqueTable[varId];
  nextUniqueTableForId = bddManager->uniqueTable[nextVarId];
  curSize1 = bddManager->uniqueTable[varId]->numEntries;
  curSize2 = bddManager->uniqueTable[nextVarId]->numEntries;
  
  /*uniqueTableForId->requestNodeList = Cal_Nil(CalBddNode_t);*/
  processingNodeList = Cal_Nil(CalBddNode_t);
  numBins = uniqueTableForId->numBins;
  bins = uniqueTableForId->bins;
  if (forwardCheckFlag){
    for(i = 0; i < numBins; ++i) {
      prevBddNode = Cal_Nil(CalBddNode_t);
      for(bddNode = bins[i];
          bddNode != Cal_Nil(CalBddNode_t);
          bddNode = nextBddNode) {
        /*
         * Process one bddNode at a time
         */
        nextBddNode = CalBddNodeGetNextBddNode(bddNode);
        /* The node should not be forwarded */
        Cal_Assert(CalBddNodeIsForwarded(bddNode) == 0);
        /*
        ** We don't know for sure if the reference count of the node
        ** could be 0. Let us use the assertion at this point.
        */
        Cal_Assert(CalBddNodeIsRefCountZero(bddNode) == 0);
        /*
        ** If ever the above assetion fails, or if we can convince
        ** ourselves that the reference count could be zero, we need
        ** to uncomment the following code.
        */
        /*
        if (CalBddNodeIsRefCountZero(bddNode)){
          thenBddNode = CAL_BDD_POINTER(CalBddNodeGetThenBddNode(bddNode));
          elseBddNode = CAL_BDD_POINTER(CalBddNodeGetElseBddNode(bddNode));
          CalBddNodeDcrRefCount(thenBddNode);
          CalBddNodeDcrRefCount(elseBddNode);
          if (prevBddNode){
            CalBddNodePutNextBddNode(prevBddNode, nextBddNode);
          }
          else{
            bins[i] = nextBddNode;
          }
          uniqueTableForId->numEntries--;
          bddManager->numNodes--;
          bddManager->numNodesFreed++;
          CalNodeManagerFreeNode(uniqueTableForId->nodeManager, bddNode);
          continue;
        }
        */
        CalBddNodeGetElseBdd(bddNode, f0);
        CalBddNodeGetThenBdd(bddNode, f1);
        
        if (CalBddIsForwarded(f1)) {
          CalBddForward(f1);
          CalBddNodePutThenBdd(bddNode, f1);
        }
        Cal_Assert(CalBddIsForwarded(f1) == 0);
        
        if (CalBddIsForwarded(f0)) {
          CalBddForward(f0);
          CalBddNodePutElseBdd(bddNode, f0); 
        }
        Cal_Assert(CalBddIsForwarded(f0) == 0);
        /*
        ** Get the index of f0 and f1 and create newF0 and newF1 if necessary
        */
        elseVarIndex = CalBddNodeGetElseBddIndex(bddManager, bddNode);
        thenVarIndex = CalBddNodeGetThenBddIndex(bddManager, bddNode);
        
        if ((elseVarIndex > (varIndex + 1))
            && (thenVarIndex > (varIndex + 1))) { 
          prevBddNode = bddNode;
          Cal_Assert(CalDoHash2(CalBddGetBddNode(f1),
                                CalBddGetBddNode(f0), 
                                uniqueTableForId) == i);
          continue;
        }
  
        /* This node is going to be forwared */
        CalBddNodePutNextBddNode(bddNode, processingNodeList);
        processingNodeList = bddNode;
        
        /* Update the unique table appropriately */
        if (prevBddNode){
          CalBddNodePutNextBddNode(prevBddNode, nextBddNode);
        }
        else{
          bins[i] = nextBddNode;
        } 
        uniqueTableForId->numEntries--;
        bddManager->numNodes--;
      }
    }
  }
  else{
    for(i = 0; i < numBins; i++) {
      prevBddNode = Cal_Nil(CalBddNode_t);
      for(bddNode = bins[i];
          bddNode != Cal_Nil(CalBddNode_t);
          bddNode = nextBddNode) {
        /*
         * Process one bddNode at a time
         */
        nextBddNode = CalBddNodeGetNextBddNode(bddNode);
        /* The node should not be forwarded */
        Cal_Assert(CalBddNodeIsForwarded(bddNode) == 0);

        /*
        ** We don't know for sure if the reference count of the node
        ** could be 0. Let us use the assertion at this point.
        */
        Cal_Assert(CalBddNodeIsRefCountZero(bddNode) == 0);
        /*
        ** If ever the above assetion fails, or if we can convince
        ** ourselves that the reference count could be zero, we need
        ** to uncomment the following code.
        */
        /*
        if (CalBddNodeIsRefCountZero(bddNode)){
          thenBddNode = CAL_BDD_POINTER(CalBddNodeGetThenBddNode(bddNode));
          elseBddNode = CAL_BDD_POINTER(CalBddNodeGetElseBddNode(bddNode));
          CalBddNodeDcrRefCount(thenBddNode);
          CalBddNodeDcrRefCount(elseBddNode);
          if (prevBddNode){
            CalBddNodePutNextBddNode(prevBddNode, nextBddNode);
          }
          else{
            bins[i] = nextBddNode;
          }
          uniqueTableForId->numEntries--;
          bddManager->numNodes--;
          bddManager->numNodesFreed++;
          CalNodeManagerFreeNode(uniqueTableForId->nodeManager, bddNode);
          continue;
        }
        */
        CalBddNodeGetThenBdd(bddNode, f1);
        Cal_Assert(CalBddIsForwarded(f1) == 0);
        CalBddNodeGetElseBdd(bddNode, f0);
        Cal_Assert(CalBddIsForwarded(f0) == 0);
        /*
        ** Get the index of f0 and f1 and create newF0 and newF1 if necessary
        */

        elseVarIndex = CalBddNodeGetElseBddIndex(bddManager, bddNode);
        thenVarIndex = CalBddNodeGetThenBddIndex(bddManager, bddNode);
        
        if ((elseVarIndex > (varIndex + 1))
            && (thenVarIndex > (varIndex + 1))) { 
          prevBddNode = bddNode;
          continue;
        }
  
        /* This node is going to be forwared */
        CalBddNodePutNextBddNode(bddNode, processingNodeList);
        processingNodeList = bddNode;
        
        /* Update the unique table appropriately */
        if (prevBddNode){
          CalBddNodePutNextBddNode(prevBddNode, nextBddNode);
        }
        else{
          bins[i] = nextBddNode;
        } 
        uniqueTableForId->numEntries--;
        bddManager->numNodes--;
      }
    }
  }
  bddNode = processingNodeList;
  /*endNode = uniqueTableForId->endNode;*/
  startNode = uniqueTableForId->startNode.nextBddNode;
  while (bddNode != Cal_Nil(CalBddNode_t)) {
    nextBddNode = CalBddNodeGetNextBddNode(bddNode);

    /*
     * Get the index of f0 and f1 and create newF0 and newF1 if necessary
     */

    CalBddNodeGetElseBdd(bddNode, f0);
    CalBddNodeGetThenBdd(bddNode, f1);
    elseVarIndex = CalBddNodeGetElseBddIndex(bddManager, bddNode);
    thenVarIndex = CalBddNodeGetThenBddIndex(bddManager, bddNode);

    if (elseVarIndex > (varIndex + 1)) {
      f00 = f0;
      f01 = f0;
      CalBddGetElseBdd(f1, f10);
      CalBddGetThenBdd(f1, f11);
    } else if (thenVarIndex > (varIndex + 1)) {
      f10 = f1;
      f11 = f1;
      CalBddGetElseBdd(f0, f00);
      CalBddGetThenBdd(f0, f01);
    }else{
      CalBddGetElseBdd(f1, f10);
      CalBddGetThenBdd(f1, f11);
      CalBddGetElseBdd(f0, f00);
      CalBddGetThenBdd(f0, f01);
    }
    Cal_Assert(CalBddIsForwarded(f10) == 0);
    Cal_Assert(CalBddIsForwarded(f11) == 0);
    Cal_Assert(CalBddIsForwarded(f00) == 0);
    Cal_Assert(CalBddIsForwarded(f01) == 0);

    if (CalBddIsEqual(f10,f00)) {
      newF0 = f00;
      f0Found = 1;
    }
    else {
      f0Found = CalUniqueTableForIdFindOrAdd(bddManager, uniqueTableForId, f10,
                                             f00, &newF0);
    }
    CalBddIcrRefCount(newF0);
    if (CalBddIsEqual(f11, f01)) {
      newF1 = f01;
      f1Found = 1;
    }
    else {
      f1Found = CalUniqueTableForIdFindOrAdd(bddManager, uniqueTableForId, f11,
                                             f01, &newF1);
    }
    CalBddIcrRefCount(newF1);

    if (!f0Found){
      CalBddIcrRefCount(f10);
      CalBddIcrRefCount(f00);
    }

    if (!f1Found){
      CalBddIcrRefCount(f11);
      CalBddIcrRefCount(f01);
    }

    CalBddDcrRefCount(f0);
    CalBddDcrRefCount(f1);
    /*
     * Create the new node for f. It cannot exist before, since at
     * least one of newF0 and newF1 must be dependent on currentIndex.
     * Otherwise, f00 == f10 and f01 == f11 (redundant nodes).
     */
    CalHashTableAddDirectAux(nextUniqueTableForId, newF1, newF0, &newF);
    bddManager->numNodes++;
    CalBddNodePutThenBdd(bddNode, newF);
    CalBddNodePutElseBddNode(bddNode, FORWARD_FLAG);
    bddManager->numForwardedNodes++;
    CalBddNodeGetRefCount(bddNode, refCount);
    CalBddAddRefCount(newF, refCount);
    Cal_Assert(!CalBddIsRefCountZero(newF));
    /* Put it in the forwarded list of the unique table */
    /*
    endNode->nextBddNode = bddNode;
    endNode = bddNode;
    */
    bddNode->nextBddNode = startNode;
    startNode = bddNode;
    
    bddNode = nextBddNode;
  }
  /*uniqueTableForId->endNode = endNode;*/
  uniqueTableForId->startNode.nextBddNode = startNode;

  BddReorderFreeNodes(bddManager, nextVarId);
  
  }
  else{
    bddManager->numTrivialSwaps++;
  }
  
  CalFixupAssoc(bddManager, varId, nextVarId, bddManager->tempAssociation);
  for(assoc = bddManager->associationList; assoc; assoc = assoc->next){
    CalFixupAssoc(bddManager, varId, nextVarId, assoc);
  }

  bddManager->idToIndex[varId] = varIndex + 1;
  bddManager->idToIndex[nextVarId] = varIndex;
  bddManager->indexToId[varIndex] = nextVarId;
  bddManager->indexToId[varIndex + 1] = varId;

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
static int
CofactorFixAndReclaimForwardedNodes(Cal_BddManager_t *bddManager, int
                                       cofactorCheckStartIndex, int
                                       cofactorCheckEndIndex, int
                                       reclaimStartIndex, int reclaimEndIndex)
{
  int index, varId;
  /* Clean up : Need to fix the cofactors of userBDDs and the
     indices above the varStartIndex only. */
  if (bddManager->pipelineState == CREATE){
    /* There are some results computed in pipeline */
    CalBddReorderFixProvisionalNodes(bddManager);
  }
  CalBddReorderFixUserBddPtrs(bddManager);
  CalReorderAssociationFix(bddManager);
  for (index = cofactorCheckStartIndex;
       index <= cofactorCheckEndIndex; index++){ 
    varId = bddManager->indexToId[index];
    CalBddReorderFixCofactors(bddManager, varId);
  }
  CalBddReorderReclaimForwardedNodes(bddManager, reclaimStartIndex,
                                     reclaimEndIndex);
  Cal_Assert(CalCheckAllValidity(bddManager));
  return 0;
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddReorderFreeNodes(Cal_BddManager_t * bddManager, int varId)
{
  CalBddNode_t *prevNode, *bddNode, *nextBddNode;
  CalBddNode_t *elseBddNode;
  CalBddNode_t *thenBddNode;
  CalHashTable_t *uniqueTableForId;
  CalBddNode_t **bins;
  int numBins;
  int i;
  long oldNumEntries, numNodesFreed;

  uniqueTableForId = bddManager->uniqueTable[varId];
  bins = uniqueTableForId->bins;
  numBins = uniqueTableForId->numBins;
  oldNumEntries = uniqueTableForId->numEntries;

  if (bddManager->numPeakNodes < (bddManager->numNodes +
                                  bddManager->numForwardedNodes)){
    bddManager->numPeakNodes = bddManager->numNodes +
        bddManager->numForwardedNodes ;
  }
  
  for(i = 0; i < numBins; i++){
    prevNode = NULL;
    bddNode = bins[i];
    while(bddNode != Cal_Nil(CalBddNode_t)){
      nextBddNode = CalBddNodeGetNextBddNode(bddNode);
      Cal_Assert(CalBddNodeIsForwarded(bddNode) == 0);
      if(CalBddNodeIsRefCountZero(bddNode)){
        thenBddNode = CAL_BDD_POINTER(CalBddNodeGetThenBddNode(bddNode));
        elseBddNode = CAL_BDD_POINTER(CalBddNodeGetElseBddNode(bddNode));
        Cal_Assert(CalBddNodeIsForwarded(thenBddNode) == 0);
        Cal_Assert(CalBddNodeIsForwarded(elseBddNode) == 0);
        CalBddNodeDcrRefCount(thenBddNode);
        CalBddNodeDcrRefCount(elseBddNode);
        if (prevNode == NULL) {
          bins[i] = nextBddNode;
        } else {
          CalBddNodePutNextBddNode(prevNode, nextBddNode);
        }
        CalNodeManagerFreeNode(uniqueTableForId->nodeManager, bddNode);
        uniqueTableForId->numEntries--;
      } else {
        prevNode = bddNode;
      }
      bddNode = nextBddNode;
    }
  }
  numNodesFreed = oldNumEntries - uniqueTableForId->numEntries;
  bddManager->numNodes -= numNodesFreed;
  bddManager->numNodesFreed += numNodesFreed;
}

#ifdef _CAL_VERBOSE

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
PrintBddProfileAfterReorder(Cal_BddManager_t *bddManager)
{
    int i, index, numBins, j;
    CalHashTable_t *uniqueTableForId;
    CalBddNode_t *bddNode, *nextBddNode;
    char *levels = Cal_MemAlloc(char, bddManager->numVars+1);
    CalBddNode_t *requestNodeList;
    Cal_Bdd_t thenBdd, elseBdd;
    
  /* Now traverse all the nodes in order */
    for (index = 0; index < bddManager->numVars; index++){
      fprintf(stdout,"**** %3d ****\n", bddManager->indexToId[index]);  
      uniqueTableForId = bddManager->uniqueTable[bddManager->indexToId[index]];
      numBins = uniqueTableForId->numBins;
      for (i=1; i<=bddManager->numVars; i++) {
          levels[i] = 0;
      }
      j = 0;
      for (i = 0; i < numBins; i++){
          for (bddNode = uniqueTableForId->bins[i]; bddNode;
               bddNode = nextBddNode){
              nextBddNode = CalBddNodeGetNextBddNode(bddNode);
              CalBddNodeGetThenBdd(bddNode, thenBdd);
              CalBddNodeGetElseBdd(bddNode, elseBdd);
              if (CalBddIsForwarded(thenBdd) ||
                  CalBddIsForwarded(elseBdd)){                       
                j++;
              }
              if (CalBddIsForwarded(thenBdd)) {                      
                  levels[CalBddGetThenBddId(thenBdd)]++;
              }
              if (CalBddIsForwarded(elseBdd)) {                      
                  levels[CalBddGetThenBddId(elseBdd)]++;
              }
          }
      }
      fprintf(stdout,"\tCofactors (%3d): ", j);
      for (i=1; i<=bddManager->numVars; i++){
          if (levels[i]) {
              fprintf(stdout,"%3d->%3d ", i, levels[i]);
          }
      }
      fprintf(stdout,"\n");
      for (i=1; i<=bddManager->numVars; i++) {
          levels[i] = 0;
      }
      j = 0;
      requestNodeList = uniqueTableForId->startNode.nextBddNode;
      for (bddNode = requestNodeList; bddNode; bddNode = nextBddNode){
          Cal_Assert(CalBddNodeIsForwarded(bddNode));
          nextBddNode = CalBddNodeGetNextBddNode(bddNode);
          CalBddNodeGetThenBdd(bddNode, thenBdd);
          Cal_Assert(!CalBddIsForwarded(thenBdd));
          levels[CalBddGetBddId(thenBdd)]++;
          j++;
      }
      fprintf(stdout,"\tForwarded nodes (%3d): ", j);
      for (i=1; i<=bddManager->numVars; i++){
          if (levels[i]) {
              fprintf(stdout,"%3d->%3d ", i, levels[i]);
          }
      }
      fprintf(stdout,"\n");
    }
    Cal_MemFree(levels);
}
#endif

/**Function********************************************************************

  Synopsis    [Reorder variables using "sift" algorithm.]

  Description [Reorder variables using "sift" algorithm.]

  SideEffects [None]

******************************************************************************/
static void
BddReorderVarSift(Cal_BddManager bddManager, double maxSizeFactor)
{
  int i,j;
  int mostNodesId = -1;
  long mostNodes, varNodes;
  int *idArray;
  long numVarsShifted = 0;
  bddManager->numSwaps = 0;
  
  idArray = Cal_MemAlloc(int, bddManager->numVars);
  for (i = 0; i < bddManager->numVars; i++) {
    idArray[i] = bddManager->indexToId[i];
  }

  while (i &&
         (numVarsShifted <=
          bddManager->maxNumVarsSiftedPerReordering) &&
         (bddManager->numSwaps <=
          bddManager->maxNumSwapsPerReordering)){ 
    i--;
    numVarsShifted++;
/*
 * Find var with the most number of nodes and do sifting on it.
 * idArray is used to make sure that a var is not sifted more than
 * once.
 */
    mostNodes = 0;
    for (j = 0; j <= i; j++){
      varNodes = bddManager->uniqueTable[idArray[j]]->numEntries;
      if (varNodes > mostNodes) {
        mostNodes = varNodes;
        mostNodesId = j;
      }
    }
 
    if (mostNodes <= 1) { /* I can put a different stopping criterion */
      /*
       * Most number of nodes among the vars not sifted yet is 0. Stop.
       */
      break;
    }

    BddReorderSiftToBestPos(bddManager,
                            bddManager->idToIndex[idArray[mostNodesId]],
                            maxSizeFactor); 
    Cal_Assert(CalCheckAllValidity(bddManager));
    idArray[mostNodesId] = idArray[i];
  }

  Cal_MemFree(idArray);
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static int
BddReorderSiftToBestPos(Cal_BddManager_t * bddManager, int
                        varStartIndex, double maxSizeFactor)
{
  long curSize;
  long bestSize;
  int bestIndex;
  int varCurIndex;
  int varId, i;
  int lastIndex = bddManager->numVars - 1;
  int numVars = bddManager->numVars;
  long startSize = bddManager->numNodes;
  long maxSize =  startSize * maxSizeFactor;
  int origId = bddManager->indexToId[varStartIndex];

  int topMostSwapIndex = 0; /* the variable has been swapped upto this
                               index */
  int bottomMostSwapIndex = lastIndex; /* the variable has been
                                          swapped upto this index */

  int swapFlag = 0; /* If a swap has taken place after last cleaning
                       up */
                       
  
  curSize = bestSize = bddManager->numNodes;
  bestIndex = varStartIndex;

#ifdef _CAL_VERBOSE
  for (i=0; i<bddManager->numVars; i++){
    fprintf(stdout, "%3d ", bddManager->indexToId[i]);
  }
  fprintf(stdout, "%8d\n", bddManager->numNodes);
#endif
  
  /*
  ** If varStartIndex > numVars/2, do: Down, Up, Down.
  ** If varStartIndex < numVars/2, do: Up, Down, Up
  ** Followed by a cleanup phase in either case.
  */
  
  if (varStartIndex >= (numVars >> 1)){
    /* Phase I: Downward swap, no forwarding check. */
    varCurIndex = varStartIndex;
    while (varCurIndex < lastIndex) {
      BddReorderSwapVarIndex(bddManager, varCurIndex, 0);
      swapFlag = 1;
      if (bddManager->numForwardedNodes > bddManager->maxForwardedNodes){
        CofactorFixAndReclaimForwardedNodes(bddManager, 0,
                                               varCurIndex-1,
                                               varCurIndex+1,
                                               varCurIndex+1); 
        swapFlag = 0;
      }
      varCurIndex++;
      curSize = bddManager->numNodes;
      /*if (curSize > maxSize){*/
      if (curSize >= (bestSize << 1)){
        bottomMostSwapIndex = varCurIndex;
        break;
      }
      if (curSize < bestSize) {
        bestSize = curSize;
        bestIndex = varCurIndex;
      }
    }
    
    /* Phase II : Two parts */
    /*
    ** Part One: Upward swap until varStartIndex. Fix cofactors and
    ** fix double pointers. 
    */
    
    while (varCurIndex > varStartIndex) {
      varCurIndex--;
      BddReorderSwapVarIndex(bddManager, varCurIndex, 1);
      swapFlag = 1;
      varId = bddManager->indexToId[varCurIndex];
      BddReorderFixForwardingNodes(bddManager, varId);
      if (bddManager->numForwardedNodes > bddManager->maxForwardedNodes){
        CofactorFixAndReclaimForwardedNodes(bddManager, 0,
                                               varCurIndex-1,
                                               varCurIndex,
                                               bottomMostSwapIndex); 
        swapFlag = 0;
      }
    }
    curSize = startSize;
    
    /*
    ** Part two: Upward swap all the way to the top. Fix cofactors.
    */
    while (varCurIndex > 0) {
      varCurIndex--;
      BddReorderSwapVarIndex(bddManager, varCurIndex, 1);
      swapFlag = 1;
      curSize = bddManager->numNodes;
      if (bddManager->numForwardedNodes > bddManager->maxForwardedNodes){
        CofactorFixAndReclaimForwardedNodes(bddManager, 0,
                                               varCurIndex-1,
                                               varCurIndex,
                                               bottomMostSwapIndex); 
        swapFlag = 0;
      }
      if (curSize > maxSize){
        topMostSwapIndex = varCurIndex;
        break;
      }
      if (curSize <= bestSize) {
        bestSize = curSize;
        bestIndex = varCurIndex;
      }
    }

    if (swapFlag){
      /* Fix user BDD pointers and reclaim forwarding nodes */
      if (bddManager->pipelineState == CREATE){
        /* There are some results computed in pipeline */
        CalBddReorderFixProvisionalNodes(bddManager);
      }
      CalBddReorderFixUserBddPtrs(bddManager);
      CalReorderAssociationFix(bddManager);
      
      /* The upward swapping might have stopped short */
      for (i = 0; i < topMostSwapIndex; i++){
        varId = bddManager->indexToId[i];
        CalBddReorderFixCofactors(bddManager, varId);
      }
      
      CalBddReorderReclaimForwardedNodes(bddManager, topMostSwapIndex,
                                         bottomMostSwapIndex);
      swapFlag = 0;
    }
    
    Cal_Assert(CalCheckAllValidity(bddManager));
    
    /* Phase III : Swap to the min position */

    while (varCurIndex < bestIndex) {
      BddReorderSwapVarIndex(bddManager, varCurIndex, 0); 
      swapFlag = 1;
      if (bddManager->numForwardedNodes > bddManager->maxForwardedNodes){
        CofactorFixAndReclaimForwardedNodes(bddManager, 0,
                                               varCurIndex-1,
                                               varCurIndex+1,
                                               varCurIndex+1); 
        swapFlag = 0;
      }
      varCurIndex++;
    }
  }
  else{
    /* Phase I: Upward swap, fix cofactors. */
    varCurIndex = varStartIndex;
    while (varCurIndex > 0) {
      varCurIndex--;
      BddReorderSwapVarIndex(bddManager, varCurIndex, 1);
      swapFlag = 1;
      curSize = bddManager->numNodes;
      if (bddManager->numForwardedNodes > bddManager->maxForwardedNodes){
        CofactorFixAndReclaimForwardedNodes(bddManager, 0,
                                               varCurIndex-1,
                                               varCurIndex+1,
                                               varStartIndex); 
        swapFlag = 0;
      }
      if (curSize > maxSize){
        topMostSwapIndex = varCurIndex;
        break;
      }
      if (curSize < bestSize) {
        bestSize = curSize;
        bestIndex = varCurIndex;
      }
    }
    
    if (swapFlag){
      /* Fix user BDD pointers and reclaim forwarding nodes */
      if (bddManager->pipelineState == CREATE){
        /* There are some results computed in pipeline */
        CalBddReorderFixProvisionalNodes(bddManager);
      }
      CalBddReorderFixUserBddPtrs(bddManager);
      CalReorderAssociationFix(bddManager);
      /* The upward swapping might have stopped short */
      for (i = 0; i < topMostSwapIndex; i++){
        varId = bddManager->indexToId[i];
        CalBddReorderFixCofactors(bddManager, varId);
      }
      CalBddReorderReclaimForwardedNodes(bddManager, topMostSwapIndex,
                                         varStartIndex);
      swapFlag = 0;
    }
    
    Cal_Assert(CalCheckAllValidity(bddManager));

    /* Phase II : Move all the way down : two parts */

    /* Swap it to the original position, no cofactor fixing, fix
       double pointers of the variable moving up.*/
    while (varCurIndex < varStartIndex){
      BddReorderSwapVarIndex(bddManager, varCurIndex, 0);
      swapFlag = 1;
      if (bddManager->numForwardedNodes > bddManager->maxForwardedNodes){
        CofactorFixAndReclaimForwardedNodes(bddManager, 0,
                                               varCurIndex-1,
                                               varCurIndex+1,
                                               varCurIndex+1); 
        swapFlag = 0;
      }
      varCurIndex++;
    }
    /* Swap to the bottom */
    while (varCurIndex < lastIndex){
      BddReorderSwapVarIndex(bddManager, varCurIndex, 0);
      swapFlag = 1;
      if (bddManager->numForwardedNodes > bddManager->maxForwardedNodes){
        CofactorFixAndReclaimForwardedNodes(bddManager, 0,
                                               varCurIndex-1,
                                               varCurIndex+1,
                                               varCurIndex+1); 
        swapFlag = 0;
      }
      varCurIndex++;
      curSize = bddManager->numNodes;
      /* if (curSize > maxSize){ */
      if (curSize >= (bestSize << 1)){
        bottomMostSwapIndex = varCurIndex;
        break;
      }
      if (curSize <= bestSize) {
        bestSize = curSize;
        bestIndex = varCurIndex;
      }
    }

    /* Phase III : Move up to the best position */
    while (varCurIndex > bestIndex){
      varCurIndex--;
      BddReorderSwapVarIndex(bddManager, varCurIndex, 1);
      swapFlag = 1;
      varId = bddManager->indexToId[varCurIndex];
      BddReorderFixForwardingNodes(bddManager, varId);
      if (bddManager->numForwardedNodes > bddManager->maxForwardedNodes){
        CofactorFixAndReclaimForwardedNodes(bddManager, 0, varCurIndex-1,
                                               varCurIndex,
                                               bottomMostSwapIndex); 
        swapFlag = 0;
      }
    }
  } /* End of else clause (varStartIndex < numVars/2) */

#ifdef _CAL_VERBOSE
  PrintBddProfileAfterReorder(bddManager);
#endif
  
  if (CalBddIdNeedsRepacking(bddManager, origId)){
    if (swapFlag){
      if (varStartIndex >= (numVars >> 1)){
        CalBddPackNodesAfterReorderForSingleId(bddManager, 0,
                                               bestIndex, bestIndex); 
      }
      /*
      else if (bestIndex >= (numVars >> 1)){
        int i;
        int nodeCounter = 0;
        for (i=bestIndex; i<numVars; i++){
          nodeCounter +=
              bddManager->uniqueTable[bddManager->indexToId[i]]->numEntries;
        }
        if ((bddManager->numNodes - nodeCounter) >
            bddManager->numForwardedNodes){
            BddPackNodesAfterReorderForSingleId(bddManager, 1, bestIndex,
                                                 bottomMostSwapIndex);
        }
        else {
          BddSiftPerfromPhaseIV(bddManager, varStartIndex, bestIndex,
                                bottomMostSwapIndex);
          BddPackNodesAfterReorderForSingleId(bddManager, 0,
                                                 bestIndex, bestIndex); 
        }
      }
      */
      else {
        /* Clean up - phase IV */
        BddSiftPerfromPhaseIV(bddManager, varStartIndex, bestIndex,
                              bottomMostSwapIndex);
        CalBddPackNodesAfterReorderForSingleId(bddManager, 0,
                                               bestIndex, bestIndex); 
      }
    }
    else {
      CalBddPackNodesAfterReorderForSingleId(bddManager, 0, bestIndex,
                                             bestIndex); 
    }
  }
  else if (swapFlag) {
    /* clean up - phase IV */
    BddSiftPerfromPhaseIV(bddManager, varStartIndex, bestIndex,
                          bottomMostSwapIndex);
  }
  Cal_Assert(CalCheckAllValidity(bddManager));
  
#ifdef _CAL_VERBOSE
  printf("ID = %3d SI = %3d EI = %3d Nodes = %7d\n", origId,
         varStartIndex, bestIndex, bddManager->numNodes);
#endif
  return bestIndex;
}

  
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddSiftPerfromPhaseIV(Cal_BddManager_t *bddManager, int varStartIndex,
                      int bestIndex, int bottomMostSwapIndex)
{
  int varCurIndex, varId;
  

/* We need to perform phase IV */
  varCurIndex = bestIndex-1;
  while (varCurIndex >= 0) {
    varId = bddManager->indexToId[varCurIndex];
    CalBddReorderFixCofactors(bddManager, varId);
    varCurIndex--;
  }
  if (bddManager->pipelineState == CREATE){
    /* There are some results computed in pipeline */
    CalBddReorderFixProvisionalNodes(bddManager);
  }
  CalBddReorderFixUserBddPtrs(bddManager);
  CalReorderAssociationFix(bddManager);
  if (varStartIndex >= (bddManager->numVars >> 1)){
    CalBddReorderReclaimForwardedNodes(bddManager, bestIndex, bestIndex);
  }
  else {
    CalBddReorderReclaimForwardedNodes(bddManager, bestIndex,
                                       bottomMostSwapIndex); 
  }
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
BddReorderVarWindow(Cal_BddManager bddManager, char *levels)
{
  long i;
  int moved;
  int anySwapped;
  int even;
  int lastIndex = bddManager->numVars-1;
  
#ifdef _CAL_VERBOSE
  for (i=0; i<bddManager->numVars; i++){
    fprintf(stdout, "%3d ", bddManager->indexToId[i]);
  }
  fprintf(stdout, "%8d\n", bddManager->numNodes);
#endif
  for (i=0; i < bddManager->numVars-1; i++) levels[i]=1;
  even = 1;
  do {
    anySwapped=0;
    if (even){
      /*fprintf(stdout, "Downward Swap\n");*/
      for (i=0; i < bddManager->numVars-1; i++){
        if (levels[i]) {
          if (i < bddManager->numVars-2) {
            moved = BddReorderWindow3(bddManager, i, 0);
            if (bddManager->numForwardedNodes >
                bddManager->maxForwardedNodes){   
              CofactorFixAndReclaimForwardedNodes(bddManager, 0,
                                                     i-1, 0, i+2);
              CalBddPackNodesForMultipleIds(bddManager,
                                         bddManager->indexToId[i], 3);
            }
          }
          else {
            moved = BddReorderWindow2(bddManager, i, 0);
          }
          if (moved){
            if (i > 0) {
              levels[i-1]=1;
              if (i > 1) levels[i-2]=1;
            }
            levels[i]=1;
            levels[i+1]=1;
            if (i < bddManager->numVars-2) {
              levels[i+2]=1;
              if (i < bddManager->numVars-3) {
                levels[i+3]=1;
                if (i < bddManager->numVars-4) levels[i+4]=1;
              }
            }
            anySwapped=1;
          }
          else {
            levels[i]=0;
          }
        }
      }
      /* new code added */
      for (i = bddManager->numVars-1; i >= 0; i--){
        CalBddReorderFixCofactors(bddManager, bddManager->indexToId[i]);
      }
      CalBddReorderFixUserBddPtrs(bddManager);
      if (bddManager->pipelineState == CREATE){
        /* There are some results computed in pipeline */
        CalBddReorderFixProvisionalNodes(bddManager);
      }
      CalReorderAssociationFix(bddManager);
      CalBddReorderReclaimForwardedNodes(bddManager, 0, lastIndex);
      /*even = 0;*/
    }
    else{
      /*fprintf(stdout, "Upward Swap\n");*/
      for (i=bddManager->numVars-1; i > 0; i--){
          /*
           * Fix the then and else cofactors. We need to fix it, even
           * if this level is not supposed to be moved.
           */
        if (i > 1) {
          CalBddReorderFixCofactors(bddManager,
                                 bddManager->indexToId[i-2]); 
        }
        else {
          CalBddReorderFixCofactors(bddManager,
                                 bddManager->indexToId[i-1]); 
        }
        if (levels[i]) {
          if (i > 1) {
            moved = BddReorderWindow3(bddManager, i-2, 1);
            if (bddManager->numForwardedNodes >
                bddManager->maxForwardedNodes){ 
              CofactorFixAndReclaimForwardedNodes(bddManager, 0,
                                                  i-3, 0,
                                                  lastIndex); 
              CalBddPackNodesForMultipleIds(bddManager,
                                            bddManager->indexToId[i-2], 3);
            }
          }
          else {
            moved = BddReorderWindow2(bddManager, i-1, 1);
          }
          if (moved){
            if (i < bddManager->numVars-1) {
              levels[i+1]=1;
              if (i < bddManager->numVars-2) {
                levels[i+2]=1;
                if (i < bddManager->numVars-3) {
                  levels[i+3]=1;
                  if (i < bddManager->numVars-4) {
                    levels[i+4]=1;
                  }
                }
              }
            }
            levels[i]=1;
            levels[i-1]=1;
            if (i > 1) {
              levels[i-2]=1;
            }
            anySwapped=1;
          }
          else {
            levels[i]=0;
          }
        }
      }
      even = 1;
      CalBddReorderFixUserBddPtrs(bddManager);
      if (bddManager->pipelineState == CREATE){
        /* There are some results computed in pipeline */
        CalBddReorderFixProvisionalNodes(bddManager);
      }
      CalReorderAssociationFix(bddManager);
      CalBddReorderReclaimForwardedNodes(bddManager, 0, lastIndex);
    }
  }
  while (anySwapped);
  if (!even){ /* Need to do pointer fixing */
    for (i = bddManager->numVars-1; i >= 0; i--){
      CalBddReorderFixCofactors(bddManager, bddManager->indexToId[i]);
    }
    CalBddReorderFixUserBddPtrs(bddManager);
    if (bddManager->pipelineState == CREATE){
      /* There are some results computed in pipeline */
      CalBddReorderFixProvisionalNodes(bddManager);
    }
    CalReorderAssociationFix(bddManager);
    CalBddReorderReclaimForwardedNodes(bddManager, 0, lastIndex);
  }
  Cal_Assert(CalCheckAllValidity(bddManager));
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
BddReorderWindow2(Cal_BddManager bddManager, long index, int directionFlag)
{
  long curSize, startSize;

  startSize = bddManager->numNodes;
  BddReorderSwapVarIndex(bddManager, index, 0);
  curSize = bddManager->numNodes;
  if (curSize > startSize){
    BddReorderSwapVarIndex(bddManager, index, 0);
  }
  if (directionFlag){/* Upward window swap */
    BddReorderFixAndFreeForwardingNodes(bddManager,
                                        bddManager->indexToId[index],
                                        bddManager->numVars-index); 
  }
  else{
    BddReorderFixAndFreeForwardingNodes(bddManager,
                                        bddManager->indexToId[index], 2);
  }
  Cal_Assert(CalCheckValidityOfNodesForWindow(bddManager, index, 2));
  return (curSize < startSize);
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
BddReorderWindow3(Cal_BddManager bddManager, long index, int directionFlag)
{
  int best;
  long curSize, bestSize;
  long origSize = bddManager->numNodes;
  
  /* 1 2 3 */
  best = 0;
  bestSize = bddManager->numNodes;
  BddReorderSwapVarIndex(bddManager, index, 0); 
  /* 2 1 3 */
  curSize = bddManager->numNodes;
  if (curSize < bestSize){
    best = 1;
    bestSize = curSize;
  }
  BddReorderSwapVarIndex(bddManager, index+1, 0);
  /* 2 3 1 */
  curSize = bddManager->numNodes;
  if (curSize < bestSize){
    best = 2;
    bestSize = curSize;
  }
  BddReorderSwapVarIndex(bddManager, index, 1);
  /* 3 2 1 */
  curSize = bddManager->numNodes;
  if (curSize <= bestSize){
    best = 3;
    bestSize = curSize;
  }
  BddReorderSwapVarIndex(bddManager, index+1, 0);
  /* 3 1 2 */
  curSize = bddManager->numNodes;
  if (curSize <= bestSize){
    best = 4;
    bestSize = curSize;
  }
  BddReorderSwapVarIndex(bddManager, index, 1);
  /* 1 3 2 */
  curSize = bddManager->numNodes;
  if (curSize <= bestSize){
    best = 5;
    bestSize = curSize;
  }
  switch (best) {
    case 0:
      BddReorderSwapVarIndex(bddManager, index+1, 0);
      break;
    case 1:
      BddReorderSwapVarIndex(bddManager, index+1, 0);
      BddReorderSwapVarIndex(bddManager, index, 1);
      break;
    case 2:
      BddReorderSwapVarIndex(bddManager, index, 0);
      BddReorderSwapVarIndex(bddManager, index+1, 0);
      BddReorderSwapVarIndex(bddManager, index, 1);
      break;
    case 3:
      BddReorderSwapVarIndex(bddManager, index, 0);
      BddReorderSwapVarIndex(bddManager, index+1, 0);
      break;
    case 4:
      BddReorderSwapVarIndex(bddManager, index, 0);
      break;
    case 5:
      break;
  }
  if ((best == 0) || (best == 3)){
    CalBddReorderFixCofactors(bddManager, bddManager->indexToId[index]);
  }
  if (directionFlag){/* Upward window swap */
    BddReorderFixAndFreeForwardingNodes(bddManager,
                                        bddManager->indexToId[index],
                                        bddManager->numVars-index); 
  }
  else{
    BddReorderFixAndFreeForwardingNodes(bddManager,
                                        bddManager->indexToId[index], 3);
  }
  Cal_Assert(CalCheckValidityOfNodesForWindow(bddManager, index, 3));
  return ((best > 0) && (origSize > bestSize));
}

