/**CFile***********************************************************************

  FileName    [calBddSubstitute.c]

  PackageName [cal]

  Synopsis    [Routine for simultaneous substitution of an array of
  variables with an array of functions.]

  Description [Routine for simultaneous substitution of an array of
  variables with an array of functions.]

  SeeAlso     [optional]

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

  Revision    [$Id: calBddSubstitute.c,v 1.1.1.4 1998/05/04 00:58:54 hsv Exp $]

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

static void CalHashTableSubstituteApply(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, int lastIndex, CalHashTable_t ** reqQueForSubstitute);
static void CalHashTableSubstituteReduce(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, CalHashTable_t ** reqQueForITE, CalHashTable_t * uniqueTableForId);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Substitute a set of variables by functions]

  Description [Returns a BDD for f using the substitution defined by current
  variable association. Each variable is replaced by its associated BDDs. The 
  substitution is effective simultaneously]

  SideEffects [None]

  SeeAlso     [Cal_BddCompose]

******************************************************************************/
Cal_Bdd
Cal_BddSubstitute(Cal_BddManager bddManager, Cal_Bdd fUserBdd)
{
  CalRequest_t result;
  int bddId, bddIndex, lastIndex;
  CalHashTable_t *hashTable;
  CalHashTable_t *uniqueTableForId;
  CalHashTable_t **reqQueForSubstitute = bddManager->reqQue[0];
  CalHashTable_t **reqQueForITE = bddManager->reqQue[1]; 
  Cal_Bdd_t f;
  
  if (CalBddPreProcessing(bddManager, 1, fUserBdd) == 0){
    return (Cal_Bdd) 0;
  }
  f = CalBddGetInternalBdd(bddManager, fUserBdd);  
  if(CalBddIsBddConst(f)){
    return CalBddGetExternalBdd(bddManager, f);
  }

  CalHashTableFindOrAdd(reqQueForSubstitute[CalBddGetBddId(f)], f, 
    bddManager->bddNull, &result);

  /* ReqQueApply */
  lastIndex = bddManager->currentAssociation->lastBddIndex;
  for(bddIndex = 0; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTable = reqQueForSubstitute[bddId];
    if(hashTable->numEntries){
      CalHashTableSubstituteApply(bddManager, hashTable, lastIndex, 
                                  reqQueForSubstitute);
    }
  }

  /* ReqQueReduce */
  for(bddIndex = bddManager->numVars - 1; bddIndex >= 0; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    hashTable = reqQueForSubstitute[bddId];
    if(hashTable->numEntries){
      CalHashTableSubstituteReduce(bddManager, hashTable,
                                   reqQueForITE, uniqueTableForId);
    }
  }

  CalRequestIsForwardedTo(result);

  /* ReqQueCleanUp */
  for(bddId = 1; bddId <= bddManager->numVars; bddId++){
    CalHashTableCleanUp(reqQueForSubstitute[bddId]);
  }

  return CalBddGetExternalBdd(bddManager, result);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalHashTableSubstituteApply(
  Cal_BddManager_t * bddManager,
  CalHashTable_t * hashTable,
  int  lastIndex,
  CalHashTable_t ** reqQueForSubstitute)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  CalRequestNode_t *requestNode;
  Cal_BddId_t bddId;
  /*Cal_BddIndex_t bddIndex;*/
  int bddIndex;
  Cal_Bdd_t f, calBdd;
  Cal_Bdd_t nullBdd = bddManager->bddNull;

  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      /* Process the requestNode */
      CalRequestNodeGetF(requestNode, f);
      /* Process Left Cofactor */
      CalBddGetThenBdd(f, calBdd);
      bddId = CalBddGetBddId(calBdd);
      bddIndex = bddManager->idToIndex[bddId];
      if(bddIndex <= lastIndex){
        CalHashTableFindOrAdd(reqQueForSubstitute[bddId], calBdd, nullBdd, 
            &calBdd);
      }
      CalBddIcrRefCount(calBdd);
      CalRequestNodePutThenRequest(requestNode, calBdd);
      /* Process Right Cofactor */
      CalBddGetElseBdd(f, calBdd);
      bddId = CalBddGetBddId(calBdd);
      bddIndex = bddManager->idToIndex[bddId];
      if(bddIndex <= lastIndex){
        CalHashTableFindOrAdd(reqQueForSubstitute[bddId], calBdd, nullBdd, 
            &calBdd);
      }
      CalBddIcrRefCount(calBdd);
      CalRequestNodePutElseRequest(requestNode, calBdd);
    }
  }
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalHashTableSubstituteReduce(
  Cal_BddManager_t * bddManager,
  CalHashTable_t * hashTable,
  CalHashTable_t ** reqQueForITE,
  CalHashTable_t * uniqueTableForId)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  Cal_BddId_t varBddId = hashTable->bddId;
  CalNodeManager_t *nodeManager = hashTable->nodeManager;
  /*CalRequestNode_t *requestNodeList = hashTable->requestNodeList;*/
  CalRequestNode_t *endNode = hashTable->endNode;
  CalRequestNode_t *requestNodeListForITE = Cal_Nil(CalRequestNode_t);
  CalRequestNode_t *requestNode, *next;
  CalBddNode_t *bddNode;
  Cal_Bdd_t varBdd;
  Cal_Bdd_t thenBdd, elseBdd, result;
  Cal_Bdd_t h;
  Cal_BddIndex_t varBddIndex;
  Cal_BddRefCount_t refCount;
  int bddId, bddIndex;
  CalHashTable_t *hashTableForITE;

  varBddIndex = bddManager->idToIndex[varBddId];
  varBdd = bddManager->varBdds[varBddId];
  h = bddManager->currentAssociation->varAssociation[varBddId];
  if(!CalBddIsBddNull(bddManager, h)){
    for(i = 0; i < numBins; i++){
      for(requestNode = bins[i], bins[i] = Cal_Nil(CalRequestNode_t);
          requestNode != Cal_Nil(CalRequestNode_t);
          requestNode = next){
        next = CalRequestNodeGetNextRequestNode(requestNode);
        /* Process the requestNode */
        CalRequestNodeGetThenRequest(requestNode, thenBdd);
        CalRequestNodeGetElseRequest(requestNode, elseBdd);
        CalRequestIsForwardedTo(thenBdd);
        CalRequestIsForwardedTo(elseBdd);
        if(CalBddIsEqual(thenBdd, elseBdd)){
          CalBddNodeGetRefCount(requestNode, refCount);
          CalBddAddRefCount(thenBdd, refCount - 2);
          CalRequestNodePutThenRequest(requestNode, thenBdd);
          CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
          endNode->nextBddNode = requestNode;
          endNode = requestNode;
        }
        else{
          CalBddDcrRefCount(thenBdd);
          CalBddDcrRefCount(elseBdd);
          result = CalOpITE(bddManager, h, thenBdd, elseBdd, reqQueForITE);
          CalRequestNodePutThenRequest(requestNode, result);
          CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
          CalRequestNodePutNextRequestNode(requestNode,
              requestNodeListForITE);
          requestNodeListForITE = requestNode;
        }
      }
    }
  }
  else{
    for(i = 0; i < numBins; i++){
      for(requestNode = bins[i], bins[i] = Cal_Nil(CalRequestNode_t);
          requestNode != Cal_Nil(CalRequestNode_t);
          requestNode = next){
        next = CalRequestNodeGetNextRequestNode(requestNode);
        /* Process the requestNode */
        CalRequestNodeGetThenRequest(requestNode, thenBdd);
        CalRequestNodeGetElseRequest(requestNode, elseBdd);
        CalRequestIsForwardedTo(thenBdd);
        CalRequestIsForwardedTo(elseBdd);
        if(CalBddIsEqual(thenBdd, elseBdd)){
          CalBddNodeGetRefCount(requestNode, refCount);
          CalBddAddRefCount(thenBdd, refCount - 2);
          CalRequestNodePutThenRequest(requestNode, thenBdd);
          CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
          endNode->nextBddNode = requestNode;
          endNode = requestNode;
        }
        else if(varBddIndex < CalBddGetBddIndex(bddManager, thenBdd) &&
            varBddIndex < CalBddGetBddIndex(bddManager, elseBdd)){
          if(CalUniqueTableForIdLookup(bddManager, uniqueTableForId,
              thenBdd, elseBdd, &result) == 1){
            CalBddDcrRefCount(thenBdd);
            CalBddDcrRefCount(elseBdd);
            CalBddNodeGetRefCount(requestNode, refCount);
            CalBddAddRefCount(result, refCount);
            CalRequestNodePutThenRequest(requestNode, result);
            CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
            endNode->nextBddNode = requestNode;
            endNode = requestNode;
          }
          else if(CalBddIsOutPos(thenBdd)){
            CalRequestNodePutThenRequest(requestNode, thenBdd);
            CalRequestNodePutElseRequest(requestNode, elseBdd);
            CalHashTableAddDirect(uniqueTableForId, requestNode);
            bddManager->numNodes++;
            bddManager->gcCheck--;
          }
          else{
            CalNodeManagerAllocNode(nodeManager, bddNode);
            CalBddNodePutThenBddId(bddNode, CalBddGetBddId(thenBdd));
            CalBddNodePutThenBddNode(bddNode,
                CalBddGetBddNodeNot(thenBdd));
            CalBddNodePutElseBddId(bddNode, CalBddGetBddId(elseBdd));
            CalBddNodePutElseBddNode(bddNode,
                CalBddGetBddNodeNot(elseBdd));
            CalBddNodeGetRefCount(requestNode, refCount);
            CalBddNodePutRefCount(bddNode, refCount);
            CalHashTableAddDirect(uniqueTableForId, bddNode);
            bddManager->numNodes++;
            bddManager->gcCheck--;
            CalRequestNodePutThenRequestId(requestNode, varBddId);
            CalRequestNodePutThenRequestNode(requestNode,
                CalBddNodeNot(bddNode));
            CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
            endNode->nextBddNode = requestNode;
            endNode = requestNode;
          }
        }
        else{
          CalBddDcrRefCount(thenBdd);
          CalBddDcrRefCount(elseBdd);
          result = CalOpITE(bddManager, varBdd, thenBdd, elseBdd, reqQueForITE);
          CalRequestNodePutThenRequest(requestNode, result);
          CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
          CalRequestNodePutNextRequestNode(requestNode,
              requestNodeListForITE);
          requestNodeListForITE = requestNode;
        }
      }
    }
  }

  /* ITE Apply */
  for(bddIndex = 0; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTableForITE = reqQueForITE[bddId];
    if(hashTableForITE->numEntries){
      CalHashTableITEApply(bddManager, hashTableForITE, reqQueForITE);
    }
  }
  /* ITE Reduce */
  for(bddIndex = bddManager->numVars - 1; bddIndex >= 0; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    hashTableForITE = reqQueForITE[bddId];
    if(hashTableForITE->numEntries){
      CalHashTableReduce(bddManager, hashTableForITE,
          bddManager->uniqueTable[bddId]);
    }
  }
    

  /*last = Cal_Nil(CalRequestNode_t);*/
  for(requestNode = requestNodeListForITE; 
      requestNode != Cal_Nil(CalRequestNode_t);
      /*last = requestNode, */
      requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
    CalRequestNodeGetThenRequest(requestNode, result);
    CalBddNodeGetRefCount(requestNode, refCount);
    CalRequestIsForwardedTo(result);
    CalBddAddRefCount(result, refCount);
    CalRequestNodePutThenRequest(requestNode, result);
  }

  /*CalBddNodePutNextBddNode(endNode, requestNodeListForITE);*/
  endNode->nextBddNode = requestNodeListForITE;
  hashTable->endNode = endNode;
  
  /* ITE Cleanup */
  for(bddId = 1; bddId <= bddManager->numVars; bddId++){
    CalHashTableCleanUp(reqQueForITE[bddId]);
  }
}













