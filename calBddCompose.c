/**CFile***********************************************************************

  FileName    [calBddCompose.c]

  PackageName [cal]

  Synopsis    [Routine for composing one BDD into another.]

  Description []

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

  Revision    [$Id: calBddCompose.c,v 1.1.1.3 1998/05/04 00:58:50 hsv Exp $]

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

/**Function********************************************************************

  Synopsis    [composition - substitute a BDD variable by a function]

  Description [Returns the BDD obtained by substituting a variable by a function]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddCompose(Cal_BddManager bddManager, Cal_Bdd  fUserBdd,
               Cal_Bdd  gUserBdd, Cal_Bdd hUserBdd)
{
  Cal_Bdd_t result;
  CalRequestNode_t *requestNode;
  Cal_Bdd_t F, G, H;
  
  if (CalBddPreProcessing(bddManager, 3, fUserBdd, gUserBdd, hUserBdd) == 0){
    result = bddManager->bddNull;
  }
  F = CalBddGetInternalBdd(bddManager, fUserBdd);
  G = CalBddGetInternalBdd(bddManager, gUserBdd);
  H = CalBddGetInternalBdd(bddManager, hUserBdd);

  if(CalBddIsBddConst(G)){
    CalBddNodeIcrRefCount(fUserBdd);
    return fUserBdd;
  }
  CalNodeManagerAllocNode(bddManager->nodeManagerArray[0], requestNode);
  CalRequestNodePutF(requestNode, F);
  CalRequestNodePutG(requestNode, H);
  CalRequestNodePutNextRequestNode(requestNode, 0);
  bddManager->requestNodeListArray[0] = requestNode;
  /*
  ** We can achieve a superscalar compose operation, provided G
  ** is the same for all the compose operation 
  */

  CalRequestNodeListCompose(bddManager, bddManager->requestNodeListArray[0],
      CalBddGetBddIndex(bddManager, G));

  CalRequestNodeGetThenRequest(requestNode, result);
  CalNodeManagerFreeNode(bddManager->nodeManagerArray[0], requestNode);
  bddManager->requestNodeListArray[0] = Cal_Nil(CalRequestNode_t);

  return CalBddGetExternalBdd(bddManager, result);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

  note        [THERE IS A POSSIBILITY OF HAVING A PIPELINED VERSION
               NEED TO THINK IT THROUGH]

******************************************************************************/
void
CalRequestNodeListCompose(Cal_BddManager_t * bddManager,
                          CalRequestNode_t * requestNodeList,
                          Cal_BddIndex_t  composeIndex)
{
  CalRequestNode_t *requestNode;
  CalRequest_t F, H, result;
  int bddId, bddIndex;
  CalHashTable_t *hashTable, *uniqueTableForId;
  CalHashTable_t **reqQueForCompose = bddManager->reqQue[0];
  CalHashTable_t **reqQueForITE = bddManager->reqQue[1]; 
  
  /* ReqQueForComposeInsertUsingReqList */
  for(requestNode = requestNodeList;
      requestNode != Cal_Nil(CalRequestNode_t);
      requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
    CalRequestNodeGetF(requestNode, F);
    CalRequestNodeGetG(requestNode, H);
    CalComposeRequestCreate(bddManager, F, H, composeIndex, 
        reqQueForCompose, reqQueForITE, &result);
    CalRequestNodePutThenRequest(requestNode, result);
    CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
  }

  /* ReqQueApply */
  for(bddIndex = 0; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTable = reqQueForCompose[bddId];
    if(hashTable->numEntries){
      CalHashTableComposeApply(bddManager, hashTable, composeIndex, 
          reqQueForCompose, reqQueForITE);
    }
    hashTable = reqQueForITE[bddId];
    if(hashTable->numEntries){
      CalHashTableITEApply(bddManager, hashTable, reqQueForITE);
    }
  }

  /* ReqQueReduce */
  for(bddIndex = bddManager->numVars - 1; bddIndex >= 0; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    hashTable = reqQueForCompose[bddId];
    if(hashTable->numEntries){
      CalHashTableReduce(bddManager, hashTable, uniqueTableForId);
    }
    hashTable = reqQueForITE[bddId];
    if(hashTable->numEntries){
      CalHashTableReduce(bddManager, hashTable, uniqueTableForId);
    }
  }

  /* ReqListArrayReqForward */
  for(requestNode = requestNodeList; requestNode != Cal_Nil(CalRequestNode_t);
      requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
    CalRequestNodeGetThenRequest(requestNode, result);
    CalRequestIsForwardedTo(result);
    CalRequestNodePutThenRequest(requestNode, result);
  }

  /* ReqQueCleanUp */
  for(bddId = 1; bddId <= bddManager->numVars; bddId++){
    CalHashTableCleanUp(reqQueForCompose[bddId]);
    CalHashTableCleanUp(reqQueForITE[bddId]);
  }
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalHashTableComposeApply(Cal_BddManager_t *bddManager,
                         CalHashTable_t *hashTable,
                         Cal_BddIndex_t  gIndex,
                         CalHashTable_t **reqQueForCompose,
                         CalHashTable_t **reqQueForITE)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  CalRequestNode_t *requestNode;
  Cal_Bdd_t fx, fxbar;
  Cal_Bdd_t hx, hxbar;
  Cal_Bdd_t calBdd1, calBdd2, calBdd3;
  Cal_Bdd_t result;
  Cal_BddId_t bddId;
  Cal_BddIndex_t index;
  
  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){

      /* Process the requestNode */
      CalRequestNodeGetCofactors(bddManager, requestNode, fx, fxbar, hx, hxbar);

      /* Process left cofactor */
      index = CalBddGetBddIndex(bddManager, fx);
      if(index > gIndex){
        CalBddIcrRefCount(fx);
        CalRequestNodePutThenRequest(requestNode, fx);
      }
      else if(index < gIndex){
        CalBddGetMinId2(bddManager, fx, hx, bddId);
        CalHashTableFindOrAdd(reqQueForCompose[bddId], fx, hx, &result);
        CalBddIcrRefCount(result);
        CalRequestNodePutThenRequest(requestNode, result);
      }
      else{
        /* fxIndex == gIndex */
        /* RequestNodeThen = ITE(hx, fxThen, fxElse) */
        calBdd1 = hx;
        CalBddGetThenBdd(fx, calBdd2);
        CalBddGetElseBdd(fx, calBdd3);
	result = CalOpITE(bddManager, calBdd1, calBdd2, calBdd3, reqQueForITE);
        CalBddIcrRefCount(result);
        CalRequestNodePutThenRequest(requestNode, result);
      }

      /* Process right cofactor */
      index = CalBddGetBddIndex(bddManager, fxbar);
      if(index > gIndex){
        CalBddIcrRefCount(fxbar);
        CalRequestNodePutElseRequest(requestNode, fxbar);
      }
      else if(index < gIndex){
        CalBddGetMinId2(bddManager, fxbar, hxbar, bddId);
        CalHashTableFindOrAdd(reqQueForCompose[bddId], fxbar, hxbar, &result);
        CalBddIcrRefCount(result);
        CalRequestNodePutElseRequest(requestNode, result);
      }
      else{
        /* fxbarIndex == gIndex */
        /* RequestNodeElse = ITE(hxbar, fxbarThen, fxbarElse) */
        calBdd1 = hxbar;
        CalBddGetThenBdd(fxbar, calBdd2);
        CalBddGetElseBdd(fxbar, calBdd3);
        result = CalOpITE(bddManager, calBdd1, calBdd2, calBdd3, reqQueForITE);
        CalBddIcrRefCount(result);
        CalRequestNodePutElseRequest(requestNode, result);
      }
    }
  }
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalComposeRequestCreate(Cal_BddManager_t * bddManager,
                        Cal_Bdd_t  f,
                        Cal_Bdd_t  h,
                        Cal_BddIndex_t  composeIndex,
                        CalHashTable_t **reqQueForCompose,
                        CalHashTable_t **reqQueForITE,
                        Cal_Bdd_t *resultPtr)
{
  Cal_BddIndex_t index;
  Cal_BddId_t bddId;

  index = CalBddGetBddIndex(bddManager, f);
  if(index > composeIndex){
    *resultPtr = f;
  }
  else if(index < composeIndex){
    CalBddGetMinId2(bddManager, f, h, bddId);
    CalHashTableFindOrAdd(reqQueForCompose[bddId], f, h, resultPtr);
  }
  else{
    Cal_Bdd_t calBdd1, calBdd2, calBdd3;
    calBdd1 = h;
    CalBddGetThenBdd(f, calBdd2);
    CalBddGetElseBdd(f, calBdd3);
    *resultPtr = CalOpITE(bddManager, calBdd1, calBdd2, calBdd3, reqQueForITE);
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
