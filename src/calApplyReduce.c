/**CFile***********************************************************************

  FileName    [calApplyReduce.c]

  PackageName [cal]

  Synopsis    [Generic routines for processing temporary nodes during
  "apply" and "reduce" phases.]

  Description []

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

  Revision    [$Id: calApplyReduce.c,v 1.1.1.3 1998/05/04 00:58:49 hsv Exp $]

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

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalHashTableApply(Cal_BddManager_t * bddManager, CalHashTable_t *
                  hashTable, CalHashTable_t ** reqQueAtPipeDepth, CalOpProc_t
                  calOpProc) 
{
  int i, numBins;
  CalBddNode_t **bins = 0;
  CalRequestNode_t *requestNode;
  Cal_Bdd_t fx, gx, fxbar, gxbar, result;
  Cal_BddId_t bddId;

  numBins = hashTable->numBins;
  bins = hashTable->bins;
  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      /* Process the requestNode */
      CalRequestNodeGetCofactors(bddManager, requestNode, fx, fxbar, gx, gxbar);
      Cal_Assert(((CalAddress_t)(fx.bddNode)) & ~0xf);
      Cal_Assert(((CalAddress_t)(gx.bddNode)) & ~0xf);
      Cal_Assert(((CalAddress_t)(fxbar.bddNode)) & ~0xf);
      Cal_Assert(((CalAddress_t)(gxbar.bddNode)) & ~0xf);
      if((*calOpProc)(bddManager, fx, gx, &result) == 0){
        CalBddNormalize(fx, gx);
        CalBddGetMinId2(bddManager, fx, gx, bddId);
        CalHashTableFindOrAdd(reqQueAtPipeDepth[bddId], fx, gx, &result);
      }
      CalBddIcrRefCount(result);
      CalRequestNodePutThenRequest(requestNode, result);
      if((*calOpProc)(bddManager, fxbar, gxbar, &result) == 0){
        CalBddNormalize(fxbar, gxbar);
        CalBddGetMinId2(bddManager, fxbar, gxbar, bddId);
        CalHashTableFindOrAdd(reqQueAtPipeDepth[bddId], fxbar, gxbar, &result);
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
void
CalHashTableReduce(Cal_BddManager_t * bddManager,
                   CalHashTable_t * hashTable,
                   CalHashTable_t * uniqueTableForId)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  Cal_BddId_t currentBddId = uniqueTableForId->bddId;
  CalNodeManager_t *nodeManager = uniqueTableForId->nodeManager;
  CalRequestNode_t *requestNode, *next;
  CalBddNode_t *bddNode, *endNode;
  Cal_Bdd_t thenBdd, elseBdd, result;
  Cal_BddRefCount_t refCount;

  endNode = hashTable->endNode;
  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = next){
      next = CalRequestNodeGetNextRequestNode(requestNode);
      /* Process the requestNode */
      CalRequestNodeGetThenRequest(requestNode, thenBdd);
      CalRequestNodeGetElseRequest(requestNode, elseBdd);
      CalRequestIsForwardedTo(thenBdd);
      CalRequestIsForwardedTo(elseBdd);
      if(CalBddIsEqual(thenBdd, elseBdd)){
        CalRequestNodeGetRefCount(requestNode, refCount);
        CalRequestAddRefCount(thenBdd, refCount - 2);
        CalRequestNodePutThenRequest(requestNode, thenBdd);
        CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
        endNode->nextBddNode = requestNode;
        endNode = requestNode;
      }
      else
        if(CalUniqueTableForIdLookup(bddManager, uniqueTableForId,
          thenBdd, elseBdd, &result) == 1){
        CalBddDcrRefCount(thenBdd);
        CalBddDcrRefCount(elseBdd);
        CalRequestNodeGetRefCount(requestNode, refCount);
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
        CalRequestNodeGetRefCount(requestNode, refCount);
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
  memset((char *)bins, 0, hashTable->numBins * sizeof(CalBddNode_t *));
  hashTable->endNode = endNode;
}
/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


  



