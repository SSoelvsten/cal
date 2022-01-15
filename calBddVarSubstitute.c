/**CFile***********************************************************************

  FileName    [calBddVarSubstitute.c]

  PackageName [cal]

  Synopsis    [Routine for simultaneous substitution of an array of
  variables with another array of variables.]

  Description []

  SeeAlso     [optional]

  Author      [Rajeev Ranjan   (rajeev@eecs.berkeley.edu)
               Jagesh Sanghavi (sanghavi@eecs.berkeley.edu)
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

  Revision    [$Id: calBddVarSubstitute.c,v 1.1.1.3 1998/05/04 00:58:55 hsv Exp $]

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

static void CalHashTableSubstituteApply(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, int lastIndex, CalHashTable_t ** reqQueForSubstitute, unsigned short opCode);
static void CalHashTableSubstituteReduce(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, CalHashTable_t ** reqQueForITE, CalHashTable_t * uniqueTableForId, unsigned short opCode);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Substitute a set of variables by set of another variables.]

  Description [Returns a BDD for f using the substitution defined by current
  variable association. It is assumed that each variable is replaced
  by another variable. For the substitution of a variable by a
  function, use Cal_BddSubstitute instead.]

  SideEffects [None]

  SeeAlso     [Cal_BddSubstitute]

******************************************************************************/
Cal_Bdd
Cal_BddVarSubstitute(Cal_BddManager bddManager, Cal_Bdd fUserBdd)
{
  CalRequest_t result;
  Cal_Bdd userResult;
  
  if (CalBddPreProcessing(bddManager, 1, fUserBdd)){
    Cal_Bdd_t f = CalBddGetInternalBdd(bddManager, fUserBdd);
    CalAssociation_t *assoc = bddManager->currentAssociation;
    unsigned short opCode;
    if (assoc->id == -1){
      opCode = bddManager->tempOpCode--;
    }
    else {
      opCode = CAL_OP_VAR_SUBSTITUTE + assoc->id;
    }
    result = CalBddVarSubstitute(bddManager, f, opCode, assoc);
    userResult =  CalBddGetExternalBdd(bddManager, result);
    if (CalBddPostProcessing(bddManager) == CAL_BDD_OVERFLOWED){
      Cal_BddFree(bddManager, userResult);
      Cal_BddManagerGC(bddManager);
      return (Cal_Bdd) 0;
    }
    return userResult;
  }
  return (Cal_Bdd) 0;
}
  

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Substitute a set of variables by functions]

  Description [Returns a BDD for f using the substitution defined by current
  variable association. Each variable is replaced by its associated BDDs. The 
  substitution is effective simultaneously]

  SideEffects [None]

  SeeAlso     [Cal_BddCompose]

******************************************************************************/
Cal_Bdd_t
CalBddVarSubstitute(Cal_BddManager bddManager, Cal_Bdd_t f, unsigned
                    short opCode, CalAssociation_t *assoc)
{
  CalRequest_t result;
  int bddId, bddIndex, lastIndex;
  CalHashTable_t *hashTable;
  CalHashTable_t *uniqueTableForId;
  CalHashTable_t **reqQueForSubstitute = bddManager->reqQue[0];
  CalHashTable_t **reqQueForITE = bddManager->reqQue[1]; 
  Cal_BddId_t fId = CalBddGetBddId(f);
  /*Cal_BddIndex_t fIndex = bddManager->idToIndex[fId];*/
  int fIndex = bddManager->idToIndex[fId];
  
  if (CalOpBddVarSubstitute(bddManager, f, &result)){
    return result;
  }

  if (CalCacheTableOneLookup(bddManager, f, opCode, &result)){
    return result;
  }
  CalHashTableFindOrAdd(reqQueForSubstitute[fId], f, 
    bddManager->bddNull, &result);

  /* ReqQueApply */
  lastIndex = assoc->lastBddIndex;
  for(bddIndex = fIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTable = reqQueForSubstitute[bddId];
    if(hashTable->numEntries){
      CalHashTableSubstituteApply(bddManager, hashTable, lastIndex, 
                                  reqQueForSubstitute, opCode);
    }
  }

  /* ReqQueReduce */
  for(bddIndex = bddManager->numVars - 1; bddIndex >= fIndex; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    hashTable = reqQueForSubstitute[bddId];
    if(hashTable->numEntries){
      CalHashTableSubstituteReduce(bddManager, hashTable,
                                   reqQueForITE, uniqueTableForId,
                                   opCode); 
    }
  }

  CalRequestIsForwardedTo(result);

  /* ReqQueCleanUp */
  for(bddId = 1; bddId <= bddManager->numVars; bddId++){
    CalHashTableCleanUp(reqQueForSubstitute[bddId]);
  }
  CalCacheTableTwoFixResultPointers(bddManager);
  CalCacheTableOneInsert(bddManager, f, result, opCode, 0);
  return result;
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalOpBddVarSubstitute(Cal_BddManager_t * bddManager, Cal_Bdd_t  f, Cal_Bdd_t *
            resultBddPtr) 
{
  if (bddManager->idToIndex[CalBddGetBddId(f)] >
      bddManager->currentAssociation->lastBddIndex){ 
    *resultBddPtr = f;
    return 1;
  }
  return 0;
}


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
  CalHashTable_t ** reqQueForSubstitute,
  unsigned short opCode)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  CalRequestNode_t *requestNode;
  Cal_BddId_t bddId;
  /*Cal_BddIndex_t bddIndex;*/
  int bddIndex;
  Cal_Bdd_t f, fx, fxBar, result;
  Cal_Bdd_t nullBdd = bddManager->bddNull;

  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      /* Process the requestNode */
      CalRequestNodeGetF(requestNode, f);
      /* Process Left Cofactor */
      CalBddGetThenBdd(f, fx);
      bddId = CalBddGetBddId(fx);
      bddIndex = bddManager->idToIndex[bddId];
      if(bddIndex <= lastIndex){
          if (CalCacheTableOneLookup(bddManager, fx, opCode, &result)){ 
            CalRequestIsForwardedTo(result);
          }
          else {
            CalHashTableFindOrAdd(reqQueForSubstitute[bddId], fx, nullBdd, 
                              &result);
            CalCacheTableOneInsert(bddManager, fx, result,
                                   opCode, 1);
          }
      }
      else{
        result = fx;
      }
      CalBddIcrRefCount(result);
      CalRequestNodePutThenRequest(requestNode, result);
      /* Process Right Cofactor */
      CalBddGetElseBdd(f, fxBar);
      bddId = CalBddGetBddId(fxBar);
      bddIndex = bddManager->idToIndex[bddId];
      if(bddIndex <= lastIndex){
          if (CalCacheTableOneLookup(bddManager, fxBar, opCode, &result)){ 
            CalRequestIsForwardedTo(result);
          }
          else {
            CalHashTableFindOrAdd(reqQueForSubstitute[bddId], fxBar, nullBdd, 
                              &result);
            CalCacheTableOneInsert(bddManager, fxBar, result,
                                   opCode, 1);
          }
      }
      else{
        result = fxBar;
      }
      CalBddIcrRefCount(result);
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
CalHashTableSubstituteReduce(
  Cal_BddManager_t * bddManager,
  CalHashTable_t * hashTable,
  CalHashTable_t ** reqQueForITE,
  CalHashTable_t * uniqueTableForId,
  unsigned short opCode)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  Cal_BddId_t varBddId = hashTable->bddId;
  CalNodeManager_t *nodeManager = hashTable->nodeManager;
  /*CalRequestNode_t *requestNodeList = hashTable->requestNodeList;*/
  CalRequestNode_t *endNode = hashTable->endNode;
  CalRequestNode_t *requestNodeListForITE = Cal_Nil(CalRequestNode_t);
  CalRequestNode_t *requestNode, *next, *last;
  CalBddNode_t *bddNode;
  Cal_Bdd_t varBdd;
  Cal_Bdd_t thenBdd, elseBdd, result;
  Cal_Bdd_t h;
  Cal_BddIndex_t resultIndex, varBddIndex;
  int minITEindex = bddManager->numVars;
  Cal_BddRefCount_t refCount;
  int bddId, bddIndex;
  CalHashTable_t *hashTableForITE;

  varBddIndex = bddManager->idToIndex[varBddId];
  varBdd = bddManager->varBdds[varBddId];
  h = bddManager->currentAssociation->varAssociation[varBddId];
  if(!CalBddIsBddNull(bddManager, h)){
    Cal_BddId_t hId = CalBddGetBddId(h);
    Cal_BddIndex_t hIndex = bddManager->idToIndex[hId];
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
          if(hIndex < CalBddGetBddIndex(bddManager, thenBdd) &&
             hIndex < CalBddGetBddIndex(bddManager, elseBdd)){
            if(CalUniqueTableForIdLookup(bddManager, bddManager->uniqueTable[hId],
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
              /* Get a node from the node manager of h */
              CalNodeManager_t *hNodeManager =
                  bddManager->nodeManagerArray[hId];
              CalNodeManagerInitBddNode(hNodeManager, thenBdd, elseBdd,
                                      Cal_Nil(CalBddNode_t), bddNode);
              CalBddNodeGetRefCount(requestNode, refCount);
              CalBddNodePutRefCount(bddNode, refCount);
              CalHashTableAddDirect(bddManager->uniqueTable[hId], bddNode);
              bddManager->numNodes++;
              bddManager->gcCheck--;
              CalRequestNodePutThenRequestId(requestNode, hId);
              CalRequestNodePutThenRequestNode(requestNode, bddNode); 
              CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
              endNode->nextBddNode = requestNode;
              endNode = requestNode;
            }
            else{
              /* Get a node from the node manager of h */
              CalNodeManager_t *hNodeManager =
                  bddManager->nodeManagerArray[hId];
              CalBddNot(thenBdd, thenBdd);
              CalBddNot(elseBdd, elseBdd);
              CalNodeManagerInitBddNode(hNodeManager, thenBdd, elseBdd,
                                      Cal_Nil(CalBddNode_t), bddNode);
              CalBddNodeGetRefCount(requestNode, refCount);
              CalBddNodePutRefCount(bddNode, refCount);
              CalHashTableAddDirect(bddManager->uniqueTable[hId], bddNode);
              bddManager->numNodes++;
              bddManager->gcCheck--;
              CalRequestNodePutThenRequestId(requestNode, hId);
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
            result = CalOpITE(bddManager, h, thenBdd, elseBdd, reqQueForITE);
            if ((resultIndex = bddManager->idToIndex[CalBddGetBddId(result)]) < minITEindex){
              minITEindex = resultIndex;
            }
            CalRequestNodePutThenRequest(requestNode, result);
            CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
            CalRequestNodePutNextRequestNode(requestNode,
                                             requestNodeListForITE);
            requestNodeListForITE = requestNode;
          }
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
        else{
          if(varBddIndex < CalBddGetBddIndex(bddManager, thenBdd) &&
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
              CalBddNot(thenBdd, thenBdd);
              CalBddNot(elseBdd, elseBdd);
              CalNodeManagerInitBddNode(nodeManager, thenBdd, elseBdd,
                                      Cal_Nil(CalBddNode_t), bddNode);
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
            if ((resultIndex = bddManager->idToIndex[CalBddGetBddId(result)]) < minITEindex){
              minITEindex = resultIndex;
            }
            CalRequestNodePutThenRequest(requestNode, result);
            CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
            CalRequestNodePutNextRequestNode(requestNode,
                                             requestNodeListForITE);
            requestNodeListForITE = requestNode;
          }
        }
      }
    }
  }

  /* ITE Apply */
  for(bddIndex = minITEindex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTableForITE = reqQueForITE[bddId];
    if(hashTableForITE->numEntries){
      CalHashTableITEApply(bddManager, hashTableForITE, reqQueForITE);
    }
  }
  /* ITE Reduce */
  for(bddIndex = bddManager->numVars - 1; bddIndex >= minITEindex; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    hashTableForITE = reqQueForITE[bddId];
    if(hashTableForITE->numEntries){
      CalHashTableReduce(bddManager, hashTableForITE,
          bddManager->uniqueTable[bddId]);
    }
  }
    

  last = Cal_Nil(CalRequestNode_t);
  for(requestNode = requestNodeListForITE; 
      requestNode != Cal_Nil(CalRequestNode_t);
      last = requestNode, 
      requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
    CalRequestNodeGetThenRequest(requestNode, result);
    CalBddNodeGetRefCount(requestNode, refCount);
    CalRequestIsForwardedTo(result);
    CalBddAddRefCount(result, refCount);
    CalRequestNodePutThenRequest(requestNode, result);
  }

  endNode->nextBddNode = requestNodeListForITE;
  hashTable->endNode = endNode;
  /* ITE Cleanup */
  for(bddId = 1; bddId <= bddManager->numVars; bddId++){
    CalHashTableCleanUp(reqQueForITE[bddId]);
  }
}

