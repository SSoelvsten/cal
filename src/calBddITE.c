/**CFile***********************************************************************

  FileName    [calBddITE.c]

  PackageName [cal]

  Synopsis    [Routine for computing ITE of 3 BDD operands.]

  Description []

  SeeAlso     [optional]

  Author      [Jagesh Sanghavi (sanghavi@eecs.berkeley.edu)
               Rajeev Ranjan   (rajeev@eecs.berkeley.edu)]

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

  Revision    [$Id: calBddITE.c,v 1.1.1.3 1998/05/04 00:58:51 hsv Exp $]

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

  Synopsis    [Returns the BDD for logical If-Then-Else

  Description [Returns the BDD for the logical operation IF f THEN g ELSE h
  - f g + f' h]

  SideEffects [None]

  SeeAlso     [Cal_BddAnd, Cal_BddNand, Cal_BddOr, Cal_BddNor, Cal_BddXor,
  Cal_BddXnor]

******************************************************************************/
Cal_Bdd
Cal_BddITE(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd,
           Cal_Bdd hUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd userResult;
  Cal_Bdd_t F, G, H;
  if (CalBddPreProcessing(bddManager, 3, fUserBdd, gUserBdd, hUserBdd)){
    F = CalBddGetInternalBdd(bddManager, fUserBdd);
    G = CalBddGetInternalBdd(bddManager, gUserBdd);
    H = CalBddGetInternalBdd(bddManager, hUserBdd);
    result = CalBddOpITEBF(bddManager, F, G, H);
  }
  else {
    return (Cal_Bdd) 0;
  }
  userResult =  CalBddGetExternalBdd(bddManager, result);
  if (CalBddPostProcessing(bddManager) == CAL_BDD_OVERFLOWED){
    Cal_BddFree(bddManager, userResult);
    Cal_BddManagerGC(bddManager);
    return (Cal_Bdd) 0;
  }
  return userResult;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Name        [CalRequestNodeListArrayOp]

  Synopsis    [required]

  Description [This routine is to be used for pipelined and
  superscalar ITE operations. Currently there is no user interface
  provided to this routine.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalRequestNodeListArrayITE(Cal_BddManager_t *bddManager,
                          CalRequestNode_t **requestNodeListArray)
{
  CalRequestNode_t *requestNode, *ptrIndirect;
  CalRequest_t F, G, H, result;
  int pipeDepth, bddId, bddIndex;
  CalHashTable_t **reqQueAtPipeDepth, *hashTable, *uniqueTableForId;
  CalHashTable_t ***reqQue = bddManager->reqQue;
  
  /* ReqQueInsertUsingReqListArray */
  for(pipeDepth = 0; pipeDepth < bddManager->depth; pipeDepth++){
    reqQueAtPipeDepth = reqQue[pipeDepth];
    for(requestNode = requestNodeListArray[pipeDepth];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      CalRequestNodeGetThenRequest(requestNode, F);
      CalRequestIsForwardedTo(F);
      ptrIndirect = CalRequestNodeGetElseRequestNode(requestNode);
      CalRequestNodeGetThenRequest(ptrIndirect, G);
      CalRequestIsForwardedTo(G);
      CalRequestNodeGetElseRequest(ptrIndirect, H);
      CalRequestIsForwardedTo(H);
      CalNodeManagerFreeNode(bddManager->nodeManagerArray[0], ptrIndirect);
      result = CalOpITE(bddManager, F, G, H, reqQueAtPipeDepth);
      CalRequestNodePutThenRequest(requestNode, result);
      CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
    }
  }

  /* ReqQueApply */
  for(bddIndex = 0; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    for(pipeDepth = 0; pipeDepth < bddManager->depth; pipeDepth++){
      reqQueAtPipeDepth = reqQue[pipeDepth];
      hashTable = reqQueAtPipeDepth[bddId];
      if(hashTable->numEntries){
        CalHashTableITEApply(bddManager, hashTable, reqQueAtPipeDepth);
      }
    }
  }

  /* ReqQueReduce */
  for(bddIndex = bddManager->numVars - 1; bddIndex >= 0; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    for(pipeDepth = 0; pipeDepth < bddManager->depth; pipeDepth++){
      hashTable = reqQue[pipeDepth][bddId];
      if(hashTable->numEntries){
        CalHashTableReduce(bddManager, hashTable, uniqueTableForId);
      }
    }
  }

  /* ReqListArrayReqForward */
  for(pipeDepth = 0; pipeDepth < bddManager->depth; pipeDepth++){
    for(requestNode = requestNodeListArray[pipeDepth];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      CalRequestNodeGetThenRequest(requestNode, result);
      CalRequestIsForwardedTo(result);
      CalRequestNodePutThenRequest(requestNode, result);
    }
  }

  /* ReqQueCleanUp */
  for(pipeDepth = 0; pipeDepth < bddManager->depth; pipeDepth++){
    reqQueAtPipeDepth = reqQue[pipeDepth];
    for(bddId = 1; bddId <= bddManager->numVars; bddId++){
      CalHashTableCleanUp(reqQueAtPipeDepth[bddId]);
    }
  }
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
Cal_Bdd_t
CalBddOpITEBF(
  Cal_BddManager_t *bddManager,
  Cal_Bdd_t  f,
  Cal_Bdd_t  g,
  Cal_Bdd_t  h)
{
  Cal_Bdd_t result;
  Cal_BddId_t bddId;
  /*Cal_BddIndex_t minIndex;*/
  int minIndex;
  int bddIndex;
  CalHashTable_t *hashTable;
  CalHashTable_t *uniqueTableForId;
  CalHashTable_t **reqQueForITE = bddManager->reqQue[0];
  
  result = CalOpITE(bddManager, f, g, h, reqQueForITE);
  
  CalBddGetMinIndex3(bddManager, f, g, h, minIndex);
  /* ReqQueApply */
  for(bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTable = reqQueForITE[bddId];
    if(hashTable->numEntries){
      CalHashTableITEApply(bddManager, hashTable, reqQueForITE);
    }
  }

  /* ReqQueReduce */
  for(bddIndex = bddManager->numVars - 1; bddIndex >= minIndex; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    hashTable = reqQueForITE[bddId];
    if(hashTable->numEntries){
      CalHashTableReduce(bddManager, hashTable, uniqueTableForId);
    }
  }

  CalRequestIsForwardedTo(result);

  /* Clean up */
  for(bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    CalHashTableCleanUp(reqQueForITE[bddId]);
  }

  return result;
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalHashTableITEApply(
  Cal_BddManager_t *bddManager,
  CalHashTable_t *hashTable,
  CalHashTable_t **reqQueAtPipeDepth)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  CalRequestNode_t *requestNode;
  Cal_Bdd_t fx, gx, fxbar, gxbar, hx, hxbar, result;
  CalNodeManager_t *nodeManager = hashTable->nodeManager;
  
  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      /* Process the requestNode */
      CalITERequestNodeGetCofactors(bddManager, requestNode,
          fx, fxbar, gx, gxbar, hx, hxbar);
      result = CalOpITE(bddManager, fx, gx, hx, reqQueAtPipeDepth);
      CalBddIcrRefCount(result);
      CalRequestNodePutThenRequest(requestNode, result);
      result = CalOpITE(bddManager, fxbar, gxbar, hxbar, reqQueAtPipeDepth);
      CalBddIcrRefCount(result);
      CalNodeManagerFreeNode(nodeManager,
          CalRequestNodeGetElseRequestNode(requestNode));
      CalRequestNodePutElseRequest(requestNode, result);
    }
  }
}

/**Function********************************************************************
 
   Synopsis    [Returns the BDD for logical If-Then-Else
 
   Description [Returns the BDD for the logical operation IF f THEN g ELSE h
   - f g + f' h]
 
   SideEffects [None]
 
   SeeAlso     [Cal_BddAnd, Cal_BddNand, Cal_BddOr, Cal_BddNor, Cal_BddXor,
   Cal_BddXnor]
 
******************************************************************************/
Cal_Bdd_t
CalBddITE(Cal_BddManager_t *bddManager, Cal_Bdd_t F, Cal_Bdd_t G,
          Cal_Bdd_t H)
{
  Cal_Bdd_t result;
  result = CalBddOpITEBF(bddManager, F, G, H);
  CalBddIcrRefCount(result);
  return result;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
