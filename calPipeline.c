/**CFile***********************************************************************

  FileName    [calPipeline.c]

  PackageName [cal]

  Synopsis    [Routines for creating and managing the pipelined BDD
  operations.] 

  Description [Eventually we would like to have this feature
  transparent to the user.]

  SeeAlso     [optional]

  Author      [ Rajeev K. Ranjan   (rajeev@eecs.berkeley.edu)
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

  Revision    [$Id: calPipeline.c,v 1.1.1.3 1998/05/04 00:59:01 hsv Exp $]

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

  Synopsis    [Set depth of a BDD pipeline.]

  Description [The "depth" determines the amount of dependency we
  would allow in pipelined computation.]

  SideEffects [None.]

  SeeAlso     []

******************************************************************************/
void
Cal_PipelineSetDepth(Cal_BddManager bddManager, int depth)
{
  int i, j;
  if(depth > 6){
    CalBddWarningMessage("PipelineDepth can not exceed 6\n");
    CalBddWarningMessage("setting PipelineDepth to 6\n");
    depth = 6;
  }
  if(bddManager->maxDepth < depth){
    int oldMaxDepth = bddManager->maxDepth;
    bddManager->depth = bddManager->maxDepth = depth;
    bddManager->reqQue = Cal_MemRealloc(CalHashTable_t **, bddManager->reqQue,
                                 bddManager->maxDepth);
    for(i = oldMaxDepth; i < bddManager->maxDepth; i++){
      bddManager->reqQue[i] = Cal_MemAlloc(CalHashTable_t *, bddManager->maxNumVars+1);
      for(j = 0; j < bddManager->numVars+1; j++){
        bddManager->reqQue[i][j] =
            CalHashTableInit(bddManager, j);
      }
    }
  }
  else{
    bddManager->depth = depth;
  }  
}

/**Function********************************************************************

  Synopsis    [Initialize a BDD pipeline.]

  Description [All the operations for this pipeline must be of the
  same kind.]

  SideEffects [None.]

  SeeAlso     []

******************************************************************************/
int
Cal_PipelineInit(Cal_BddManager bddManager, Cal_BddOp_t bddOp)
{
  CalBddPostProcessing(bddManager);
  if(bddManager->pipelineState != READY){
    CalBddWarningMessage("Pipeline cannot be initialized");
    return 0;
  }
  else{
    bddManager->pipelineState = CREATE;
    switch(bddOp){
    case CAL_AND :
      bddManager->pipelineFn = CalOpAnd;
      break;
    case CAL_OR  :
      bddManager->pipelineFn = CalOpOr;
      break;
    case CAL_XOR :
      bddManager->pipelineFn = CalOpXor;
      break;
    default  :
      CalBddWarningMessage("Unknown Bdd Operation type");
      return 0;
    }
    return 1;
  }  
}

/**Function********************************************************************

  Synopsis    [Create a provisional BDD in the pipeline.]

  Description [The provisional BDD is automatically freed once the
  pipeline is quitted.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Cal_Bdd
Cal_PipelineCreateProvisionalBdd(Cal_BddManager bddManager, Cal_Bdd fUserBdd,
                                 Cal_Bdd gUserBdd)
{
  int insertDepth, operandDepth;
  CalRequestNode_t *requestNode;
  Cal_Bdd_t provisionalBdd, f, g;
  Cal_BddId_t bddId;
  Cal_Bdd userNode;
  
  insertDepth = 0;
  
  f = CalBddGetInternalBdd(bddManager, fUserBdd);
  g = CalBddGetInternalBdd(bddManager, gUserBdd);
  if(bddManager->pipelineState != CREATE){
    CalBddWarningMessage("Provisional Bdd not created: Pipeline is not initialized");
    return (Cal_Bdd) 0;
  }
  if(CalBddIsMarked(f)){
    CalBddGetDepth(f, operandDepth);
    if(insertDepth <= operandDepth){
      insertDepth = operandDepth + 1;
    }
  }
  if(CalBddIsMarked(g)){
    CalBddGetDepth(g, operandDepth);
    if(insertDepth <= operandDepth){
      insertDepth = operandDepth + 1;
    }
  }
  if (bddManager->pipelineDepth <= insertDepth){
    bddManager->pipelineDepth = insertDepth + 1;
  }
  if (insertDepth >= MAX_INSERT_DEPTH){
    CalBddWarningMessage("Provisional Bdd not created");
    CalBddWarningMessage("Maximum pipeline depth is reached");
    return (Cal_Bdd) 0;
  }

  CalNodeManagerAllocNode(bddManager->nodeManagerArray[0], requestNode);
  CalRequestNodePutF(requestNode, f);
  CalRequestNodePutG(requestNode, g);
  CalRequestNodeMark(requestNode);
  CalRequestNodePutDepth(requestNode, insertDepth);
  CalRequestNodePutNextRequestNode(requestNode,
      bddManager->requestNodeListArray[insertDepth]);
  bddManager->requestNodeListArray[insertDepth] = requestNode;

  CalBddGetMinId2(bddManager, f, g, bddId);
  CalBddPutBddId(provisionalBdd, bddId);
  CalBddPutBddNode(provisionalBdd, (CalBddNode_t *)requestNode);

  CalNodeManagerAllocNode(bddManager->nodeManagerArray[0], userNode);
  CalBddNodePutThenBdd(userNode, provisionalBdd);
  CalBddNodePutElseBdd(userNode, bddManager->bddOne);
  CalBddNodePutNextBddNode(userNode,
                           bddManager->userProvisionalNodeList);
  bddManager->userProvisionalNodeList = userNode;
  CalBddNodeIcrRefCount(userNode);
  return userNode;
}

/**Function********************************************************************

  Synopsis    [Executes a pipeline.]

  Description [All the results are computed. User should update the
  BDDs of interest. Eventually this feature would become transparent.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
Cal_PipelineExecute(Cal_BddManager bddManager)
{
  CalRequestNode_t **requestNodeListArray, *node, *nextNode;
  int  i;
  Cal_Bdd_t thenBdd;
  int automaticDepthControlFlag = 0;
  int pipelineDepth;
  
  if(bddManager->pipelineState != CREATE){
    CalBddWarningMessage("Pipeline cannot be executed");
    return 0;
  }

  /* Check if we need to control the depth value using some heuristic */
  if (bddManager->depth == 0) automaticDepthControlFlag = 1;
  
  requestNodeListArray = bddManager->requestNodeListArray;
  pipelineDepth = bddManager->pipelineDepth;
  while(pipelineDepth){
    if (automaticDepthControlFlag){
      if (bddManager->numNodes < 10000) bddManager->depth = 4;
      else if (bddManager->numNodes < 100000) bddManager->depth = 2;
      else bddManager->depth = 1;
    }
    if(bddManager->depth > pipelineDepth){
      bddManager->depth = pipelineDepth;
    }
    CalRequestNodeListArrayOp(bddManager, requestNodeListArray,
                              bddManager->pipelineFn);
    pipelineDepth -= bddManager->depth;

    /* Lock the results, in case garbage collection needs to be
       invoked */
    for (i=0; i<bddManager->depth; i++){
      for (node = requestNodeListArray[i]; node; node = nextNode){
        nextNode = CalBddNodeGetNextBddNode(node);
        CalBddNodeGetThenBdd(node, thenBdd);
        CalBddIcrRefCount(thenBdd);
      }
    }
    /* Save the current pipelineDepth */
    bddManager->currentPipelineDepth = pipelineDepth;
    if (CalBddPostProcessing(bddManager) == CAL_BDD_OVERFLOWED){
      /* Abort, may be we should clean up a little bit */
      fprintf(stderr,"Bdd Overflow: Aborting\n");
      return 0;
    }
    requestNodeListArray += bddManager->depth;
  }
  /* Need to decrement the reference counts */
  for (i=0; i<bddManager->pipelineDepth; i++){
    for (node=bddManager->requestNodeListArray[i]; node; node = nextNode){
      nextNode = CalBddNodeGetNextBddNode(node);
      CalBddNodeGetThenBdd(node, thenBdd);
      CalBddDcrRefCount(thenBdd);
    }
  }
  bddManager->pipelineState = UPDATE;
  return 1;
}
  
/**Function********************************************************************

  Synopsis    [Update a provisional Bdd obtained during pipelining.]

  Description [The provisional BDD is automatically freed after
  quitting pipeline.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Cal_Bdd
Cal_PipelineUpdateProvisionalBdd(Cal_BddManager bddManager,
                                 Cal_Bdd provisionalBdd) 
{
  Cal_Bdd_t calBdd = CalBddGetInternalBdd(bddManager, provisionalBdd);
  if(bddManager->pipelineState != UPDATE){
    CalBddWarningMessage("Provisional Bdd cannot be updated");
    return (Cal_Bdd) 0;
  }
  CalBddGetThenBdd(calBdd, calBdd);
  return CalBddGetExternalBdd(bddManager, calBdd);
}

/**Function********************************************************************

  Synopsis           [Returns 1, if the given user BDD contains
  provisional BDD node.]

  Description        [Returns 1, if the given user BDD contains
  provisional BDD node.]

  SideEffects        [None.]

  SeeAlso            []

******************************************************************************/
int
Cal_BddIsProvisional(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  Cal_Bdd_t internalBdd = CalBddGetInternalBdd(bddManager, userBdd);
  return CalBddIsMarked(internalBdd);
}

/**Function********************************************************************

  Synopsis    [Resets the pipeline freeing all resources.]

  Description [The user must make sure to update all provisional BDDs
  of interest before calling this routine.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void
Cal_PipelineQuit(Cal_BddManager bddManager)
{
  CalRequestNode_t *requestNode, *next;
  int i;

  bddManager->pipelineState = READY;
  for(i = 0; i < bddManager->pipelineDepth; i++){
    for(requestNode = bddManager->requestNodeListArray[i], 
        bddManager->requestNodeListArray[i] = Cal_Nil(CalRequestNode_t);
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = next){
      next = CalRequestNodeGetNextRequestNode(requestNode);
      CalNodeManagerFreeNode(bddManager->nodeManagerArray[0], requestNode);
    }
    bddManager->requestNodeListArray[i] = Cal_Nil(CalRequestNode_t);
  }
  bddManager->pipelineDepth = 0;
  for (requestNode = bddManager->userProvisionalNodeList; requestNode;
       requestNode = next){
    next = CalRequestNodeGetNextRequestNode(requestNode);
    CalNodeManagerFreeNode(bddManager->nodeManagerArray[0],
                           requestNode);
  }
  bddManager->userProvisionalNodeList = Cal_Nil(CalRequestNode_t);
}

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
CalBddReorderFixProvisionalNodes(Cal_BddManager_t *bddManager)
{
  CalRequestNode_t **requestNodeListArray =
      bddManager->requestNodeListArray;
  CalRequestNode_t *node, *nextNode;
  int i;
  Cal_Bdd_t thenBdd, elseBdd;
  
  for (i=0;
       i<bddManager->pipelineDepth-bddManager->currentPipelineDepth;
       i++){ 
    for (node = *requestNodeListArray; node; node = nextNode){
      nextNode = CalBddNodeGetNextBddNode(node);
      Cal_Assert(CalBddNodeIsForwarded(node));
      CalBddNodeGetThenBdd(node, thenBdd);
      if (CalBddIsForwarded(thenBdd)) {
        CalBddForward(thenBdd);
      }
      CalBddNodePutThenBdd(node, thenBdd);
      Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
    }
    requestNodeListArray++;
  }
  for (; i<bddManager->pipelineDepth; i++){
    for (node = *requestNodeListArray; node; node = nextNode){
      nextNode = CalBddNodeGetNextBddNode(node);
      Cal_Assert(CalBddNodeIsForwarded(node) == 0);
      CalBddNodeGetThenBdd(node, thenBdd);
      if (CalBddIsForwarded(thenBdd)) {
        CalBddForward(thenBdd);
      }
      CalBddNodePutThenBdd(node, thenBdd);
      Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
      CalBddNodeGetElseBdd(node, elseBdd);
      if (CalBddIsForwarded(elseBdd)) {
        CalBddForward(elseBdd);
      }
      CalBddNodePutElseBdd(node, elseBdd);
      Cal_Assert(CalBddIsForwarded(elseBdd) == 0);
    }
    requestNodeListArray++;
  }
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalCheckPipelineValidity(Cal_BddManager_t *bddManager)
{
  CalRequestNode_t **requestNodeListArray =
      bddManager->requestNodeListArray;
  CalRequestNode_t *node, *nextNode;
  int i;
  Cal_Bdd_t thenBdd, elseBdd;
  
  for (i=0;
       i<bddManager->pipelineDepth-bddManager->currentPipelineDepth;
       i++){ 
    for (node = *requestNodeListArray; node; node = nextNode){
      nextNode = CalBddNodeGetNextBddNode(node);
      Cal_Assert(CalBddNodeIsForwarded(node));
      CalBddNodeGetThenBdd(node, thenBdd);
      Cal_Assert(CalBddIsForwarded(thenBdd) == 0);
    }
    requestNodeListArray++;
  }
  for (; i<bddManager->pipelineDepth; i++){
    for (node = *requestNodeListArray; node; node = nextNode){
      nextNode = CalBddNodeGetNextBddNode(node);
      Cal_Assert(CalBddNodeIsForwarded(node) == 0); 
      CalBddNodeGetThenBdd(node, thenBdd);
      /*Cal_Assert(CalBddIsForwarded(thenBdd) == 0);*/
      /* This is possible since the actual BDD of thenBdd could have been
         computed and it is forwarded, however this node is not yet
         updated with the result */
      CalBddNodeGetElseBdd(node, elseBdd);
      /*Cal_Assert(CalBddIsForwarded(elseBdd) == 0);*/
    }
    requestNodeListArray++;
  }
}
/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

