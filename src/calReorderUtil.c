/**CFile***********************************************************************

  FileName    [calReorderUtil.c]

  PackageName [cal]

  Synopsis    [Some utility routines used by both breadth-first and
  depth-first reordering techniques.]

  Description []

  SeeAlso     [optional]

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

  Revision    [$Id: calReorderUtil.c,v 1.2 1998/09/17 02:37:15 ravi Exp $]

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


/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalBddReorderFixUserBddPtrs(Cal_BddManager bddManager)
{
  CalHashTable_t *userBddUniqueTable = bddManager->uniqueTable[0];
  int i;
  int numBins;
  int rehashFlag;
  CalBddNode_t **bins;
  CalBddNode_t *bddNode;
  CalBddNode_t *nextBddNode;
  CalBddNode_t *thenBddNode;
  CalBddNode_t *elseBddNode;
  Cal_Bdd_t internalBdd;

  numBins = userBddUniqueTable->numBins;
  bins = userBddUniqueTable->bins;

  for(i = 0; i < numBins; i++) {
    for(bddNode = bins[i];
        bddNode != Cal_Nil(CalBddNode_t);
        bddNode = nextBddNode) {
      /*
       * Process one bddNode at a time
       */
      nextBddNode = CalBddNodeGetNextBddNode(bddNode);
      rehashFlag = 0;

      thenBddNode = CalBddNodeGetThenBddNode(bddNode);
      elseBddNode = CalBddNodeGetElseBddNode(bddNode);
      CalBddNodeGetThenBdd(bddNode, internalBdd);
      if (CalBddIsForwarded(internalBdd)) {
        CalBddForward(internalBdd);
        CalBddNodePutThenBdd(bddNode, internalBdd);
        rehashFlag = 1;
      }
      Cal_Assert(CalBddIsForwarded(internalBdd) == 0);
      /*Cal_Assert(!CalBddIsRefCountZero(internalBdd));*/
      /*
       * Rehash if necessary
       */
      if (rehashFlag) {
        CalUniqueTableForIdRehashNode(userBddUniqueTable, bddNode,
                                      thenBddNode, elseBddNode);
      }
    }
  }
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
int
CalCheckAllValidity(Cal_BddManager bddManager)
{
  int id;
  for(id = 0; id <= bddManager->numVars; id++){
    CalCheckValidityOfNodesForId(bddManager, id);
  }
  CalCheckAssociationValidity(bddManager);
  if (bddManager->pipelineState == CREATE){
    CalCheckPipelineValidity(bddManager);
  }
  CalCheckRefCountValidity(bddManager);
  CalCheckCacheTableValidity(bddManager);
  return 1;
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
int
CalCheckValidityOfNodesForId(Cal_BddManager bddManager, int id)
{
  int i, numBins;
  CalHashTable_t *uniqueTableForId;
  CalBddNode_t *bddNode, *nextBddNode;
  Cal_Bdd_t thenBdd, elseBdd;
  
  uniqueTableForId = bddManager->uniqueTable[id];
  Cal_Assert(uniqueTableForId->startNode.nextBddNode == NULL);
  numBins = uniqueTableForId->numBins;
  for (i = 0; i < numBins; i++){
    for (bddNode = uniqueTableForId->bins[i]; bddNode;
         bddNode = nextBddNode){
      nextBddNode = CalBddNodeGetNextBddNode(bddNode);
      CalCheckValidityOfANode(bddManager, bddNode, id);
      CalBddNodeGetThenBdd(bddNode, thenBdd);
      CalBddNodeGetElseBdd(bddNode, elseBdd);
      Cal_Assert(CalDoHash2(CalBddGetBddNode(thenBdd),
                            CalBddGetBddNode(elseBdd), 
                            uniqueTableForId) == i);
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
int
CalCheckValidityOfNodesForWindow(Cal_BddManager bddManager,
                                 Cal_BddIndex_t index, int numLevels)
{
  int i, numBins, j;
  CalHashTable_t *uniqueTableForId;
  CalBddNode_t *bddNode, *nextBddNode;
  Cal_Bdd_t thenBdd, elseBdd;

  for (i = 0; i < numLevels; i++){
    uniqueTableForId =
        bddManager->uniqueTable[bddManager->indexToId[index+i]]; 
    numBins = uniqueTableForId->numBins;
    for (j = 0; j < numBins; j++){
      for (bddNode = uniqueTableForId->bins[j]; bddNode;
           bddNode = nextBddNode){
        nextBddNode = CalBddNodeGetNextBddNode(bddNode);
        Cal_Assert(CalBddNodeIsForwarded(bddNode) == 0);
        CalBddNodeGetThenBdd(bddNode, thenBdd);
        CalBddNodeGetElseBdd(bddNode, elseBdd);
        Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
        Cal_Assert(CalBddIsForwarded(elseBdd) == 0);
        Cal_Assert(CalDoHash2(CalBddGetBddNode(thenBdd),
                              CalBddGetBddNode(elseBdd), 
                             uniqueTableForId) == j);
      }
    }
    for (bddNode = uniqueTableForId->startNode.nextBddNode; bddNode;
         bddNode = nextBddNode){ 
      nextBddNode = CalBddNodeGetNextBddNode(bddNode);
      CalBddNodeGetThenBdd(bddNode, thenBdd);
      Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
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
int
CalCheckValidityOfANode(Cal_BddManager_t *bddManager, CalBddNode_t
                     *bddNode, int id) 
{
  Cal_Bdd_t thenBdd, elseBdd, thenBdd_, elseBdd_;
  if (id != 0){
    /* id = 0 corresponds to the constants and the user BDDs */
    Cal_Assert(bddManager->idToIndex[id] <
               bddManager->idToIndex[bddNode->thenBddId]);   
    Cal_Assert(bddManager->idToIndex[id] < 
               bddManager->idToIndex[bddNode->elseBddId]);
  }
  Cal_Assert(!CalBddNodeIsForwarded(bddNode));
  Cal_Assert(!CalBddNodeIsRefCountZero(bddNode));
  CalBddNodeGetThenBdd(bddNode, thenBdd);
  CalBddNodeGetElseBdd(bddNode, elseBdd);
  Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
  Cal_Assert(CalBddIsForwarded(elseBdd) == 0);
  Cal_Assert(!CalBddIsRefCountZero(thenBdd));
  Cal_Assert(!CalBddIsRefCountZero(elseBdd));
  /* Make sure that the then and else bdd nodes are part of the
     respective unique tables */
  if (bddNode->thenBddId != 0){
    CalBddGetThenBdd(thenBdd, thenBdd_);
    CalBddGetElseBdd(thenBdd, elseBdd_);
    Cal_Assert(
      CalUniqueTableForIdLookup(bddManager,
                                bddManager->uniqueTable[bddNode->thenBddId],  
                                thenBdd_, elseBdd_, &bdd));
  }
  if (bddNode->elseBddId != 0){
    CalBddGetThenBdd(elseBdd, thenBdd_);
    CalBddGetElseBdd(elseBdd, elseBdd_);
    Cal_Assert(
      CalUniqueTableForIdLookup(bddManager,
                                bddManager->uniqueTable[bddNode->elseBddId],
                                thenBdd_, elseBdd_, &bdd));
  }
  return 1;
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalCheckRefCountValidity(Cal_BddManager_t *bddManager)
{
  int i, numBins, index;
  CalHashTable_t *uniqueTableForId;
  CalBddNode_t *bddNode, *nextBddNode;
  Cal_Bdd_t thenBdd, elseBdd, internalBdd;
  CalAssociation_t *assoc, *nextAssoc;
  
  /* First traverse the user BDDs */
  uniqueTableForId = bddManager->uniqueTable[0];
  numBins = uniqueTableForId->numBins;
  for (i = 0; i < numBins; i++){
    for (bddNode = uniqueTableForId->bins[i]; bddNode;
         bddNode = nextBddNode){
      nextBddNode = CalBddNodeGetNextBddNode(bddNode);
      CalBddNodeGetThenBdd(bddNode, internalBdd);
      CalBddDcrRefCount(internalBdd);
    }
  }
      /* Traverse the associations */
  
  for(assoc = bddManager->associationList;
      assoc != Cal_Nil(CalAssociation_t); assoc = nextAssoc){
    nextAssoc = assoc->next;
    for (i=1; i <= bddManager->numVars; i++){
        if (CalBddGetBddId(assoc->varAssociation[i]) != CAL_BDD_NULL_ID){
            CalBddDcrRefCount(assoc->varAssociation[i]);
        }
    }
  }
  /* temporary association */
  assoc = bddManager->tempAssociation;
  for (i=1; i <= bddManager->numVars; i++){
      if (CalBddGetBddId(assoc->varAssociation[i]) != CAL_BDD_NULL_ID){
          CalBddDcrRefCount(assoc->varAssociation[i]);
      }
  }

  
  /* Now traverse all the nodes in order */
  for (index = 0; index < bddManager->numVars; index++){
    uniqueTableForId = bddManager->uniqueTable[bddManager->indexToId[index]];
    numBins = uniqueTableForId->numBins;
    for (i = 0; i < numBins; i++){
      for (bddNode = uniqueTableForId->bins[i]; bddNode;
           bddNode = nextBddNode){
        nextBddNode = CalBddNodeGetNextBddNode(bddNode);
        CalBddNodeGetThenBdd(bddNode, thenBdd);
        CalBddNodeGetElseBdd(bddNode, elseBdd);
        CalBddDcrRefCount(thenBdd);
        CalBddDcrRefCount(elseBdd);
      }
    }
  }

  /* All the reference count must be zero  or max */
  for (index = 0; index < bddManager->numVars; index++){
    uniqueTableForId = bddManager->uniqueTable[bddManager->indexToId[index]];
    numBins = uniqueTableForId->numBins;
    for (i = 0; i < numBins; i++){
      for (bddNode = uniqueTableForId->bins[i]; bddNode;
           bddNode = nextBddNode){
        nextBddNode = CalBddNodeGetNextBddNode(bddNode);
        /* If the pipeline execution is going on, the following
           assertion will not hold */
        if (bddManager->pipelineState != CREATE){
          Cal_Assert(CalBddNodeIsRefCountZero(bddNode) ||
                     CalBddNodeIsRefCountMax(bddNode));
        }
      }
    }
  }

  /* Put back the ref count */
  /* traverse all the nodes in order */
  for (index = 0; index < bddManager->numVars; index++){
    uniqueTableForId = bddManager->uniqueTable[bddManager->indexToId[index]];
    numBins = uniqueTableForId->numBins;
    for (i = 0; i < numBins; i++){
      for (bddNode = uniqueTableForId->bins[i]; bddNode;
           bddNode = nextBddNode){
        nextBddNode = CalBddNodeGetNextBddNode(bddNode);
        CalBddNodeGetThenBdd(bddNode, thenBdd);
        CalBddNodeGetElseBdd(bddNode, elseBdd);
        CalBddIcrRefCount(thenBdd);
        CalBddIcrRefCount(elseBdd);
      }
    }
  }
      /* Traverse the associations */
  for(assoc = bddManager->associationList;
      assoc != Cal_Nil(CalAssociation_t); assoc = nextAssoc){
    nextAssoc = assoc->next;
    for (i=1; i <= bddManager->numVars; i++){
        if (CalBddGetBddId(assoc->varAssociation[i]) != CAL_BDD_NULL_ID){
            CalBddIcrRefCount(assoc->varAssociation[i]);
        }
    }
  }
  /* temporary association */
  assoc = bddManager->tempAssociation;
  for (i=1; i <= bddManager->numVars; i++){
      if (CalBddGetBddId(assoc->varAssociation[i]) != CAL_BDD_NULL_ID){
          CalBddIcrRefCount(assoc->varAssociation[i]);
      }
  }

  /* Traverse the user BDDs */
  uniqueTableForId = bddManager->uniqueTable[0];
  numBins = uniqueTableForId->numBins;
  for (i = 0; i < numBins; i++){
    for (bddNode = uniqueTableForId->bins[i]; bddNode;
         bddNode = nextBddNode){
      nextBddNode = CalBddNodeGetNextBddNode(bddNode);
      CalBddNodeGetThenBdd(bddNode, internalBdd);
      CalBddIcrRefCount(internalBdd);
    }
  }
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
int
CalCheckAssoc(Cal_BddManager_t *bddManager)
{
  CalAssociation_t *assoc, *nextAssoc;
  int i;
  int expectedLastBddIndex, bddIndex;

  for(assoc = bddManager->associationList;
      assoc != Cal_Nil(CalAssociation_t); assoc = nextAssoc){
    nextAssoc = assoc->next;
    expectedLastBddIndex = -1;
    for (i=1; i <= bddManager->numVars; i++){
      if (CalBddIsBddNull(bddManager, assoc->varAssociation[i]) == 0){
        bddIndex = bddManager->idToIndex[i];
        if (expectedLastBddIndex < bddIndex){
          expectedLastBddIndex = bddIndex;
        }
      }
    }
    Cal_Assert(expectedLastBddIndex == assoc->lastBddIndex);
  }
  /* fix temporary association */
  assoc = bddManager->tempAssociation;
  expectedLastBddIndex = -1;
  for (i=1; i <= bddManager->numVars; i++){
    if (CalBddIsBddNull(bddManager, assoc->varAssociation[i]) == 0){
      bddIndex = bddManager->idToIndex[i];
      if (expectedLastBddIndex < bddIndex){
        expectedLastBddIndex = bddIndex;
      }
    }
  }
  Cal_Assert(expectedLastBddIndex == assoc->lastBddIndex);
  return 1;
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalFixupAssoc(Cal_BddManager_t *bddManager, long id1, long id2,
           CalAssociation_t *assoc)
{
  if (assoc->lastBddIndex == -1) return;
  /* Variable with id1 is moving down a spot. */
  if ((CalBddIsBddNull(bddManager, assoc->varAssociation[id1]) == 0)
      && (assoc->lastBddIndex == bddManager->idToIndex[id1])){
    assoc->lastBddIndex++;
  }
  else if ((CalBddIsBddNull(bddManager, assoc->varAssociation[id1])) &&
           (CalBddIsBddNull(bddManager, assoc->varAssociation[id2]) ==
            0) && 
  (assoc->lastBddIndex == bddManager->idToIndex[id2])){
    assoc->lastBddIndex--;
  }
  Cal_Assert((assoc->lastBddIndex >= 0) && (assoc->lastBddIndex <=
                                           CAL_BDD_CONST_INDEX));
   
}
/**Function********************************************************************

  Synopsis           [Fixes the cofactors of the nodes belonging to
  the given index.]

  Description        [This routine traverses the unique table and for
  each node, looks at the then and else cofactors. If needed fixes the
  cofactors.]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalBddReorderFixCofactors(Cal_BddManager bddManager, Cal_BddId_t id)
{
  CalHashTable_t *uniqueTableForId =
      bddManager->uniqueTable[id];
  CalBddNode_t *bddNode, *nextBddNode, **bins, *thenBddNode, *elseBddNode;
  Cal_Bdd_t f0, f1;
  long numBins;
  int i, rehashFlag;
  
  numBins = uniqueTableForId->numBins;
  bins = uniqueTableForId->bins;

  for(i = 0; i < numBins; i++) {
    for(bddNode = bins[i];
        bddNode != Cal_Nil(CalBddNode_t);
        bddNode = nextBddNode) {
      nextBddNode = CalBddNodeGetNextBddNode(bddNode);
      /*
       * Process one bddNode at a time
       */
      /*
      ** Because we have kept all the forwarding nodes in the list,
      ** this should not be a forwarding node.
      */
      Cal_Assert(CalBddNodeIsForwarded(bddNode) == 0);
      Cal_Assert(CalBddNodeIsRefCountZero(bddNode) == 0);
      thenBddNode = CalBddNodeGetThenBddNode(bddNode);
      elseBddNode = CalBddNodeGetElseBddNode(bddNode);
      rehashFlag = 0;
      CalBddNodeGetThenBdd(bddNode, f1);
      CalBddNodeGetElseBdd(bddNode, f0);
      if (CalBddIsForwarded(f1)) {
        CalBddForward(f1);
        CalBddNodePutThenBdd(bddNode, f1);
        rehashFlag = 1;
      }
      Cal_Assert(CalBddIsForwarded(f1) == 0);
      if (CalBddIsForwarded(f0)) {
        CalBddForward(f0);
        CalBddNodePutElseBdd(bddNode, f0); 
        rehashFlag = 1;
      }
      Cal_Assert(CalBddIsForwarded(f0) == 0);
      /* Rehash if necessary */
      if (rehashFlag) {
        CalUniqueTableForIdRehashNode(uniqueTableForId, bddNode, thenBddNode,
                                      elseBddNode);
      }
    }
  }
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalBddReorderReclaimForwardedNodes(Cal_BddManager bddManager, int
                                startIndex, int endIndex)
{
  Cal_BddIndex_t index;
  Cal_BddId_t id;
  CalHashTable_t *uniqueTableForId;
  CalNodeManager_t *nodeManager;
  
  for(index = startIndex; index <= endIndex; index++){
    id = bddManager->indexToId[index];
    uniqueTableForId = bddManager->uniqueTable[id];
    nodeManager = uniqueTableForId->nodeManager;
    uniqueTableForId->endNode->nextBddNode = nodeManager->freeNodeList;
    nodeManager->freeNodeList = uniqueTableForId->startNode.nextBddNode;
    uniqueTableForId->endNode = &(uniqueTableForId->startNode);
    uniqueTableForId->startNode.nextBddNode = NULL;
  }
  bddManager->numForwardedNodes = 0;
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                          */
/*---------------------------------------------------------------------------*/

