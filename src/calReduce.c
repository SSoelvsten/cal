/**CFile***********************************************************************

  FileName    [calReduce.c]

  PackageName [cal]

  Synopsis    [Routines for optimizing a BDD with respect to a don't
  care set (cofactor and restrict).]

  Description []

  SeeAlso     [None]

  Author      [Rajeev K. Ranjan (rajeev@eecs.berkeley.edu) and
               Jagesh V. Sanghavi (sanghavi@eecs.berkeley.edu]

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

  Revision    [$Id: calReduce.c,v 1.1.1.4 1998/05/04 00:59:02 hsv Exp $]

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

static Cal_Bdd_t BddReduceBF(Cal_BddManager_t * bddManager, CalOpProc_t calOpProc, Cal_Bdd_t f, Cal_Bdd_t c);
static Cal_Bdd_t BddCofactorBF(Cal_BddManager_t * bddManager, CalOpProc_t calOpProc, Cal_Bdd_t f, Cal_Bdd_t c);
static void HashTableReduceApply(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, CalHashTable_t ** reduceHashTableArray, CalHashTable_t ** orHashTableArray, CalOpProc_t calOpProc);
static void HashTableCofactorApply(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, CalHashTable_t ** cofactorHashTableArray, CalOpProc_t calOpProc);
static void HashTableCofactorReduce(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, CalHashTable_t * uniqueTableForId);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Returns the generalized cofactor of BDD f with respect
  to BDD c.]

  Description [Returns the generalized cofactor of BDD f with respect
  to BDD c. The constrain operator given by Coudert et al (ICCAD90) is
  used to find the generalized cofactor.]

  SideEffects [None.]

  SeeAlso     [Cal_BddReduce]

******************************************************************************/
Cal_Bdd
Cal_BddCofactor(Cal_BddManager  bddManager, Cal_Bdd fUserBdd,
                Cal_Bdd cUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd userResult;
  if (CalBddPreProcessing(bddManager, 2, fUserBdd, cUserBdd)){
    Cal_Bdd_t f = CalBddGetInternalBdd(bddManager, fUserBdd);
    Cal_Bdd_t c = CalBddGetInternalBdd(bddManager, cUserBdd);
    result = BddCofactorBF(bddManager, CalOpCofactor, f, c);
    userResult =  CalBddGetExternalBdd(bddManager, result);
    if (CalBddPostProcessing(bddManager) == CAL_BDD_OVERFLOWED){
      Cal_BddFree(bddManager, userResult);
      Cal_BddManagerGC(bddManager);
      return Cal_BddNull(bddManager);
    }
    return userResult;
  }
  return Cal_BddNull(bddManager);
}


/**Function********************************************************************

  Synopsis    [Returns a BDD which agrees with f for all valuations
  which satisfy c.]

  Description [Returns a BDD which agrees with f for all valuations
  which satisfy c. The result is usually smaller in terms of number of
  BDD nodes than f. This operation is typically used in state space
  searches to simplify the representation for the set of states wich
  will be expanded at each step.]

  SideEffects [None]

  SeeAlso     [Cal_BddCofactor]

******************************************************************************/
Cal_Bdd
Cal_BddReduce(Cal_BddManager  bddManager, Cal_Bdd fUserBdd,
              Cal_Bdd cUserBdd)
{
  if (CalBddPreProcessing(bddManager, 2, fUserBdd, cUserBdd)){
    Cal_Bdd_t f = CalBddGetInternalBdd(bddManager, fUserBdd);
    Cal_Bdd_t c = CalBddGetInternalBdd(bddManager, cUserBdd);
    Cal_Bdd_t result = BddReduceBF(bddManager, CalOpCofactor, f, c);
    Cal_Bdd userResult =  CalBddGetExternalBdd(bddManager, result);
    if (CalBddPostProcessing(bddManager) == CAL_BDD_OVERFLOWED){
      Cal_BddFree(bddManager, userResult);
      Cal_BddManagerGC(bddManager);
      return Cal_BddNull(bddManager);
    }
	if (Cal_BddSize(bddManager, userResult, 1) <
        Cal_BddSize(bddManager, fUserBdd, 1)){
      return userResult;
    }
    else{
      Cal_BddFree(bddManager, userResult);
      return Cal_BddIdentity(bddManager, fUserBdd);
    }
  }
  return Cal_BddNull(bddManager);
}


/**Function********************************************************************

  Synopsis    [Returns a minimal BDD whose function contains fMin and is
  contained in fMax.]

  Description [Returns a minimal BDD f which is contains fMin and is
  contained in fMax ( fMin <= f <= fMax).
  This operation is typically used in state space searches to simplify
  the representation for the set of states wich will be expanded at
  each step (Rk Rk-1' <= f <= Rk).] 

  SideEffects [None]

  SeeAlso     [Cal_BddReduce]

******************************************************************************/
Cal_Bdd
Cal_BddBetween(Cal_BddManager  bddManager, Cal_Bdd fMinUserBdd,
               Cal_Bdd fMaxUserBdd)
{
  if (CalBddPreProcessing(bddManager, 2, fMinUserBdd, fMaxUserBdd)){
    Cal_Bdd_t fMin = CalBddGetInternalBdd(bddManager, fMinUserBdd);
    Cal_Bdd_t fMax = CalBddGetInternalBdd(bddManager, fMaxUserBdd);
    Cal_Bdd_t fMaxNot, careSet, result;
    Cal_Bdd resultUserBdd;
    long fMinSize, fMaxSize, resultSize;

    CalBddNot(fMax, fMaxNot);
    careSet = CalBddOpBF(bddManager, CalOpOr, fMin, fMaxNot);
    result = BddReduceBF(bddManager, CalOpCofactor, fMin, careSet);
    resultUserBdd =  CalBddGetExternalBdd(bddManager, result);
    if (CalBddPostProcessing(bddManager) == CAL_BDD_OVERFLOWED){
      Cal_BddFree(bddManager, resultUserBdd);
      Cal_BddManagerGC(bddManager);
      return Cal_BddNull(bddManager);
    }
    fMinSize = Cal_BddSize(bddManager, fMinUserBdd, 1);
    fMaxSize = Cal_BddSize(bddManager, fMaxUserBdd, 1);
    resultSize = Cal_BddSize(bddManager, resultUserBdd, 1);
    if (resultSize < fMinSize){
      if (resultSize < fMaxSize){
        return resultUserBdd;
      }
      else {
        Cal_BddFree(bddManager, resultUserBdd);
        return Cal_BddIdentity(bddManager, fMaxUserBdd);
      }
    }
    Cal_BddFree(bddManager, resultUserBdd);
    if (fMinSize < fMaxSize){
      return Cal_BddIdentity(bddManager, fMinUserBdd);
    }
    else{
      return Cal_BddIdentity(bddManager, fMaxUserBdd);
    }
  }
  return Cal_BddNull(bddManager);
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
int
CalOpCofactor(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  f,
  Cal_Bdd_t  c,
  Cal_Bdd_t * resultBddPtr)
{
  if (CalBddIsBddConst(c)){
    if (CalBddIsBddZero(bddManager, c)){
      *resultBddPtr = bddManager->bddNull;
    }
    else {
      *resultBddPtr = f;
    }
    return 1;
  }
  if (CalBddIsBddConst(f)){
    *resultBddPtr = f;
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static Cal_Bdd_t
BddReduceBF(
  Cal_BddManager_t * bddManager,
  CalOpProc_t calOpProc,
  Cal_Bdd_t  f,
  Cal_Bdd_t  c)
{
  Cal_Bdd_t result;
  CalHashTable_t  *hashTable;
  CalHashTable_t **orHashTableArray = bddManager->reqQue[0];
  CalHashTable_t **reduceHashTableArray = bddManager->reqQue[1];
  CalHashTable_t *uniqueTableForId;
  
  /*Cal_BddIndex_t minIndex;*/
  int minIndex;
  int bddIndex;
  Cal_BddId_t bddId, minId;
  
  
  if ((*calOpProc)(bddManager, f, c, &result) == 1){
    return result;
  }

  CalBddGetMinIdAndMinIndex(bddManager, f, c, minId, minIndex);
  CalHashTableFindOrAdd(reduceHashTableArray[minId], f, c, &result); 
  for (bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTable = reduceHashTableArray[bddId];
    if(hashTable->numEntries){
      HashTableReduceApply(bddManager, hashTable, reduceHashTableArray,
                           orHashTableArray, CalOpCofactor);
    }
  }
  for(bddIndex = bddManager->numVars - 1; bddIndex >= minIndex; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    hashTable = reduceHashTableArray[bddId];
    if(hashTable->numEntries){
        HashTableCofactorReduce(bddManager, hashTable, uniqueTableForId);
    }
  }
  CalRequestIsForwardedTo(result);
  for (bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    CalHashTableCleanUp(reduceHashTableArray[bddId]);
  }
  return result;
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static Cal_Bdd_t
BddCofactorBF(Cal_BddManager_t * bddManager,
              CalOpProc_t calOpProc,
              Cal_Bdd_t  f,
              Cal_Bdd_t  c)
{
  Cal_Bdd_t result;
  CalHashTable_t  *hashTable;
  CalHashTable_t **cofactorHashTableArray = bddManager->reqQue[0];
  CalHashTable_t *uniqueTableForId;
  
/*Cal_BddIndex_t minIndex;*/
  int minIndex;
  int bddIndex;
  Cal_BddId_t bddId, minId;
  
  if (CalBddIsBddZero(bddManager, c)){
    CalBddWarningMessage("Bdd Cofactor Called with zero care set");
    return bddManager->bddOne;
  }
  
  if (calOpProc(bddManager, f, c, &result) == 1){
    return result;
  }

  CalBddGetMinIdAndMinIndex(bddManager, f, c, minId, minIndex);
  CalHashTableFindOrAdd(cofactorHashTableArray[minId], f, c, &result); 
  for (bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTable = cofactorHashTableArray[bddId];
    if(hashTable->numEntries){
      HashTableCofactorApply(bddManager, hashTable, cofactorHashTableArray,
                             calOpProc); 
    }
  }
  for(bddIndex = bddManager->numVars - 1; bddIndex >= minIndex; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    hashTable = cofactorHashTableArray[bddId];
    if(hashTable->numEntries){
        HashTableCofactorReduce(bddManager, hashTable, uniqueTableForId);
    }
  }
  CalRequestIsForwardedTo(result);
  for (bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    CalHashTableCleanUp(cofactorHashTableArray[bddId]);
  }
  return result;
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
HashTableReduceApply(Cal_BddManager_t * bddManager,
                     CalHashTable_t * hashTable,
                     CalHashTable_t ** reduceHashTableArray,
                     CalHashTable_t ** orHashTableArray,
                     CalOpProc_t calOpProc)
{
  int i, numBins = hashTable->numBins;
  CalRequestNode_t *requestNode, *last, *nextRequestNode, *requestNodeList;
  Cal_Bdd_t f, c, fx, cx, fxbar, cxbar, result, orResult;
  Cal_BddId_t bddId, minId;
  /*Cal_BddIndex_t minIndex;*/
  int minIndex;
  int bddIndex;
  CalHashTable_t *orHashTable;
  
  requestNodeList = Cal_Nil(CalRequestNode_t);
  for(i = 0; i < numBins; i++){
    last = Cal_Nil(CalRequestNode_t);
    for (requestNode =  hashTable->bins[i]; requestNode !=
                                                Cal_Nil(CalRequestNode_t);
         requestNode = nextRequestNode){
      nextRequestNode = CalRequestNodeGetNextRequestNode(requestNode);  
      CalRequestNodeGetF(requestNode, f);
      CalRequestNodeGetG(requestNode, c);
      CalBddGetMinId2(bddManager, f, c, minId);
      CalBddGetCofactors(c, minId, cx, cxbar);
      if (CalBddGetBddId(f) != minId){
        if (CalOpOr(bddManager, cx, cxbar, &orResult) == 0){
          CalBddNormalize(cx, cxbar);
          CalBddGetMinId2(bddManager, cx, cxbar, minId);
          CalHashTableFindOrAdd(orHashTableArray[minId], cx, cxbar, &orResult);
        }
        CalRequestNodePutElseRequest(requestNode, orResult);
        if (last == Cal_Nil(CalRequestNode_t)){
          hashTable->bins[i] = nextRequestNode;
        }
        else {
          CalRequestNodePutNextRequestNode(last, nextRequestNode);
        }
        CalRequestNodePutNextRequestNode(requestNode, requestNodeList);
        requestNodeList = requestNode;
      }
      else{
        last = requestNode;
        CalBddGetCofactors(f, minId, fx, fxbar);
        if((*calOpProc)(bddManager, fx, cx, &result) == 0){
          CalBddGetMinId2(bddManager, fx, cx, bddId);
          CalHashTableFindOrAdd(reduceHashTableArray[bddId], fx, cx, &result);
        }
        if (CalBddIsBddNull(bddManager, result) == 0){
          CalBddIcrRefCount(result);
        }
        CalRequestNodePutThenRequest(requestNode, result);
        if((*calOpProc)(bddManager, fxbar, cxbar, &result) == 0){
          CalBddGetMinId2(bddManager, fxbar, cxbar, bddId);
          CalHashTableFindOrAdd(reduceHashTableArray[bddId], fxbar, cxbar,
                                &result);
        }
        if (CalBddIsBddNull(bddManager, result) == 0){
          CalBddIcrRefCount(result);
        }
        CalRequestNodePutElseRequest(requestNode, result);
      }
    }
  }
  minIndex = bddManager->idToIndex[hashTable->bddId];
  for (bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    orHashTable = orHashTableArray[bddId];
    if(orHashTable->numEntries){
      CalHashTableApply(bddManager, orHashTable, orHashTableArray, CalOpOr);
    }
  }
  
  for(bddIndex = bddManager->numVars - 1; bddIndex >= minIndex; bddIndex--){
    CalHashTable_t *uniqueTableForId;
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    orHashTable = orHashTableArray[bddId];
    if(orHashTable->numEntries){
      CalHashTableReduce(bddManager, orHashTable, uniqueTableForId);
    }
  }
  for (requestNode = requestNodeList; requestNode; requestNode =
                                                       nextRequestNode){
    nextRequestNode = CalRequestNodeGetNextRequestNode(requestNode);
    CalRequestNodeGetElseRequest(requestNode, orResult);
    CalRequestIsForwardedTo(orResult);
    CalRequestNodeGetThenRequest(requestNode, f);
    CalBddGetMinId2(bddManager, f, orResult, minId);
    CalHashTableFindOrAdd(reduceHashTableArray[minId], f, orResult,
                          &result);
    CalRequestNodePutThenRequest(requestNode, result);
    CalRequestNodePutElseRequest(requestNode, result);
    CalBddAddRefCount(result, 2);
    CalHashTableAddDirect(hashTable, requestNode);
  }
  
  /* Clean up the orHashTableArray */
  for(bddIndex = bddManager->numVars - 1; bddIndex >= minIndex; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    CalHashTableCleanUp(orHashTableArray[bddId]);
  }
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
HashTableCofactorApply(Cal_BddManager_t * bddManager,
                       CalHashTable_t * hashTable,
                       CalHashTable_t ** cofactorHashTableArray,
                       CalOpProc_t calOpProc)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  CalRequestNode_t *requestNode;
  Cal_Bdd_t f, c, fx, cx, fxbar, cxbar, result;
  Cal_BddId_t bddId, minId;

  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      CalRequestNodeGetF(requestNode, f);
      CalRequestNodeGetG(requestNode, c);
      CalBddGetMinId2(bddManager, f, c, minId);
      CalBddGetCofactors(f, minId, fx, fxbar);
      CalBddGetCofactors(c, minId, cx, cxbar);
      if((*calOpProc)(bddManager, fx, cx, &result) == 0){
        CalBddGetMinId2(bddManager, fx, cx, bddId);
        CalHashTableFindOrAdd(cofactorHashTableArray[bddId], fx, cx, &result);
      }
      if (CalBddIsBddNull(bddManager, result) == 0){
        CalBddIcrRefCount(result);
      }
      CalRequestNodePutThenRequest(requestNode, result);
      if((*calOpProc)(bddManager, fxbar, cxbar, &result) == 0){
        CalBddGetMinId2(bddManager, fxbar, cxbar, bddId);
        CalHashTableFindOrAdd(cofactorHashTableArray[bddId], fxbar, cxbar,
                              &result);
      }
      if (CalBddIsBddNull(bddManager, result) == 0){
          CalBddIcrRefCount(result);
      }
      CalRequestNodePutElseRequest(requestNode, result);
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
HashTableCofactorReduce(Cal_BddManager_t * bddManager,
                        CalHashTable_t * hashTable,
                        CalHashTable_t * uniqueTableForId)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  Cal_BddId_t currentBddId = uniqueTableForId->bddId;
  CalNodeManager_t *nodeManager = uniqueTableForId->nodeManager;
  CalRequestNode_t  *requestNode, *next, *endNode;
  CalBddNode_t *bddNode;
  Cal_Bdd_t thenBdd, elseBdd, result;
  Cal_BddRefCount_t refCount;

  endNode = hashTable->endNode;
  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i], bins[i] = Cal_Nil(CalRequestNode_t);
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = next){
      next = CalRequestNodeGetNextRequestNode(requestNode);
      /* Process the requestNode */
      CalRequestNodeGetThenRequest(requestNode, thenBdd);
      CalRequestNodeGetElseRequest(requestNode, elseBdd);
      if (CalBddIsBddNull(bddManager, thenBdd)){
        CalRequestIsForwardedTo(elseBdd);
        CalBddNodeGetRefCount(requestNode, refCount);
        CalBddAddRefCount(elseBdd, refCount - 1);
        CalRequestNodePutThenRequest(requestNode, elseBdd);
        CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
        endNode->nextBddNode = requestNode;
        endNode = requestNode;
        continue;
      }
      else if (CalBddIsBddNull(bddManager, elseBdd)){
        CalRequestIsForwardedTo(thenBdd);
        CalBddNodeGetRefCount(requestNode, refCount);
        CalBddAddRefCount(thenBdd, refCount - 1);
        CalRequestNodePutThenRequest(requestNode, thenBdd);
        CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
        endNode->nextBddNode = requestNode;
        endNode = requestNode;
        continue;
      }
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
      else if(CalUniqueTableForIdLookup(bddManager, uniqueTableForId,
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
        CalBddNodePutThenBddNode(bddNode, CalBddGetBddNodeNot(thenBdd));
        CalBddNodePutElseBddId(bddNode, CalBddGetBddId(elseBdd));
        CalBddNodePutElseBddNode(bddNode, CalBddGetBddNodeNot(elseBdd));
        CalBddNodeGetRefCount(requestNode, refCount);
        CalBddNodePutRefCount(bddNode, refCount);
        CalHashTableAddDirect(uniqueTableForId, bddNode);
        bddManager->numNodes++;
        bddManager->gcCheck--;
        CalRequestNodePutThenRequestId(requestNode, currentBddId);
        CalRequestNodePutThenRequestNode(requestNode, CalBddNodeNot(bddNode));
        CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
        endNode->nextBddNode = requestNode;
        endNode = requestNode;
      }
    }
  }
  hashTable->endNode = endNode;
}
  





















