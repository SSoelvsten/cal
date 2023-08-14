/**CFile***********************************************************************

  FileName    [calBddManager.c]

  PackageName [cal]

  Synopsis    [Routines for maintaing the manager and creating
  variables etc.]

  Description []

  SeeAlso     [optional]

  Author      [Rajeev K. Ranjan (rajeev@eecs.berkeley.edu)
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

  Revision    [$Id: calBddManager.c,v 1.7 1998/09/16 16:07:44 ravi Exp $]

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
unsigned long calPrimes[] = 
{
  1,
  2,
  3,
  7,
  13,
  23,
  59,
  113,
  241,
  503,
  1019,
  2039,
  4091,
  8179,
  11587,
  16369,
  23143,
  32749,
  46349,
  65521,
  92683,
  131063,
  185363,
  262139,
  330287,
  416147,
  524269,
  660557,
  832253,
  1048571,
  1321109,
  1664501,
  2097143,
  2642201,
  3328979,
  4194287,
  5284393,
  6657919,
  8388593,
  10568797,
  13315831,
  16777199,
  33554393,
  67108859,
  134217689,
  268435399,
  536870879,
  1073741789,
  2147483629
};

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define CalBddManagerGetNodeManager(bddManager, id) \
    bddManager->nodeManagerArray[id]


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void BddDefaultTransformFn(Cal_BddManager_t * bddManager, CalAddress_t value1, CalAddress_t value2, CalAddress_t * result1, CalAddress_t * result2, Cal_Pointer_t pointer);
#ifdef CALBDDMANAGER
static int CalBddManagerPrint(Cal_BddManager_t *bddManager);
#endif /* CALBDDMANAGER */

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Creates and initializes a new BDD manager.]

  Description [Initializes and allocates fields of the BDD manager. Some of the
  fields are initialized for maxNumVars+1 or numVars+1, whereas some of them are
  initialized for maxNumVars or numVars. The first kind of fields are associated
  with the id of a variable and the second ones are with the index of the
  variable.]

  SideEffects [None]

  SeeAlso     [Cal_BddManagerQuit]

******************************************************************************/
Cal_BddManager
Cal_BddManagerInit( )
{
  Cal_BddManager_t *bddManager;
  int i;
  CalBddNode_t *bddNode;
  Cal_Bdd_t resultBdd;
  
    
  bddManager = Cal_MemAlloc(Cal_BddManager_t, 1);

  bddManager->numVars = 0;

  bddManager->maxNumVars = 30;
  
  bddManager->varBdds = Cal_MemAlloc(Cal_Bdd_t, bddManager->maxNumVars+1);
  
  bddManager->pageManager1 = CalPageManagerInit(4);
  bddManager->pageManager2 = CalPageManagerInit(NUM_PAGES_PER_SEGMENT);

  bddManager->nodeManagerArray = Cal_MemAlloc(CalNodeManager_t*, bddManager->maxNumVars+1);

  bddManager->nodeManagerArray[0] = CalNodeManagerInit(bddManager->pageManager1);
  bddManager->uniqueTable = Cal_MemAlloc(CalHashTable_t *,
                                         bddManager->maxNumVars+1);
  bddManager->uniqueTable[0] = CalHashTableInit(bddManager, 0);
  
  /* Constant One */
  CalBddPutBddId(bddManager->bddOne, CAL_BDD_CONST_ID);
  CalNodeManagerAllocNode(bddManager->nodeManagerArray[0], bddNode);
  CalBddPutBddNode(bddManager->bddOne, bddNode);
  /* ~0x0 put so that the node is not mistaken for forwarded node */
  CalBddPutThenBddNode(bddManager->bddOne, (CalBddNode_t *)~0x0);
  CalBddPutElseBddNode(bddManager->bddOne, (CalBddNode_t *)~0x0);
  CalBddPutRefCount(bddManager->bddOne, CAL_MAX_REF_COUNT);

  /* Constant Zero */
  CalBddNot(bddManager->bddOne, bddManager->bddZero);

  /* Create a user BDD */
  CalHashTableAddDirectAux(bddManager->uniqueTable[0], bddManager->bddOne,
                           bddManager->bddOne, &resultBdd);
  CalBddPutRefCount(resultBdd, CAL_MAX_REF_COUNT);
  bddManager->userOneBdd =  CalBddGetBddNode(resultBdd);
  bddManager->userZeroBdd = CalBddNodeNot(bddManager->userOneBdd);
  
  /* Null BDD */
  CalBddPutBddId(bddManager->bddNull, CAL_BDD_NULL_ID);
  CalNodeManagerAllocNode(bddManager->nodeManagerArray[0], bddNode);
  CalBddPutBddNode(bddManager->bddNull, bddNode);
  /* ~0x10 put so that the node is not mistaken for forwarded node or
     the constant nodes. */
  CalBddPutThenBddNode(bddManager->bddNull, (CalBddNode_t *)~0x10);
  CalBddPutElseBddNode(bddManager->bddNull, (CalBddNode_t *)~0x10);
  CalBddPutRefCount(bddManager->bddNull, CAL_MAX_REF_COUNT);
  /* Put in the unique table, so that it gets locked */
  /*CalHashTableAddDirect(bddManager->uniqueTable[0], bddNode);*/

  bddManager->indexToId = Cal_MemAlloc(Cal_BddId_t, bddManager->maxNumVars);
  bddManager->idToIndex = Cal_MemAlloc(Cal_BddIndex_t, bddManager->maxNumVars+1);
  bddManager->idToIndex[CAL_BDD_CONST_ID] = CAL_BDD_CONST_INDEX;

  bddManager->depth = DEFAULT_DEPTH;
  bddManager->maxDepth = DEFAULT_MAX_DEPTH;
  bddManager->pipelineState = READY;
  bddManager->pipelineDepth = PIPELINE_EXECUTION_DEPTH;
  bddManager->currentPipelineDepth = 0;
  bddManager->pipelineFn = CalOpAnd;


  bddManager->reqQue = Cal_MemAlloc(CalHashTable_t **, bddManager->maxDepth);
  bddManager->cacheTable = CalCacheTableTwoInit(bddManager);
  
  for (i=0; i < bddManager->maxDepth; i++){
    bddManager->reqQue[i] = Cal_MemAlloc(CalHashTable_t *,
                                         bddManager->maxNumVars+1);
    bddManager->reqQue[i][0] = CalHashTableInit(bddManager, 0);
  }

  bddManager->requestNodeListArray = Cal_MemAlloc(CalRequestNode_t*,
                                                  MAX_INSERT_DEPTH);
  for(i = 0; i < MAX_INSERT_DEPTH; i++){
    bddManager->requestNodeListArray[i] = Cal_Nil(CalRequestNode_t);
  }
  bddManager->userProvisionalNodeList = Cal_Nil(CalRequestNode_t);

  /* Garbage collection related information */
  bddManager->numNodes = bddManager->numPeakNodes = 1;
  bddManager->numNodesFreed = 0;
  bddManager->gcCheck = CAL_GC_CHECK;
  bddManager->uniqueTableGCLimit =  CAL_MIN_GC_LIMIT;
  bddManager->numGC = 0;
  bddManager->gcMode = 1;
  bddManager->nodeLimit = 0;
  bddManager->overflow = 0;
  bddManager->repackAfterGCThreshold = CAL_REPACK_AFTER_GC_THRESHOLD;
  

  /* Special functions */
  bddManager->TransformFn = BddDefaultTransformFn;
  bddManager->transformEnv = 0;


  /* Association related information */
  bddManager->associationList = Cal_Nil(CalAssociation_t);
  /*bddManager->tempAssociation = CAL_BDD_NEW_REC(bddManager, CalAssociation_t);*/
  bddManager->tempAssociation = Cal_MemAlloc(CalAssociation_t, 1);
  bddManager->tempAssociation->id = -1;
  bddManager->tempAssociation->lastBddIndex = -1;
  bddManager->tempAssociation->varAssociation =
      Cal_MemAlloc(Cal_Bdd_t, bddManager->maxNumVars+1);
  for(i = 0; i < bddManager->maxNumVars+1; i++){
     bddManager->tempAssociation->varAssociation[i] = bddManager->bddNull;
  }
  bddManager->tempOpCode = -1;
  bddManager->currentAssociation = bddManager->tempAssociation;

  /* BDD reordering related information */
  bddManager->dynamicReorderingEnableFlag = 1;
  bddManager->reorderMethod = CAL_REORDER_METHOD_BF;
  bddManager->reorderTechnique = CAL_REORDER_NONE;
  bddManager->numForwardedNodes = 0;
  bddManager->numReorderings = 0;
  bddManager->maxNumVarsSiftedPerReordering = 1000;
  bddManager->maxNumSwapsPerReordering = 2000000;
  bddManager->numSwaps = 0;
  bddManager->numTrivialSwaps = 0;
  bddManager->maxSiftingGrowth = 2.0;
  bddManager->reorderingThreshold = CAL_BDD_REORDER_THRESHOLD;
  bddManager->maxForwardedNodes = CAL_NUM_FORWARDED_NODES_LIMIT;
  bddManager->tableRepackThreshold = CAL_TABLE_REPACK_THRESHOLD;
  

  /*bddManager->superBlock = CAL_BDD_NEW_REC(bddManager, Cal_Block_t);*/
  bddManager->superBlock = Cal_MemAlloc(Cal_Block_t, 1);
  bddManager->superBlock->numChildren=0;
  bddManager->superBlock->children=0;
  bddManager->superBlock->reorderable=1;
  bddManager->superBlock->firstIndex= -1;
  bddManager->superBlock->lastIndex=0;
  
  bddManager->hooks = Cal_Nil(void);
  
  return bddManager;
}

/**Function********************************************************************

  Synopsis    [Frees the BDD manager and all the associated allocations]

  Description [Frees the BDD manager and all the associated allocations]

  SideEffects [None]

  SeeAlso     [Cal_BddManagerInit]

******************************************************************************/
int
Cal_BddManagerQuit(Cal_BddManager bddManager)
{
  int i, j;

  if(bddManager == Cal_Nil(Cal_BddManager_t)) return 1;

  for (i=0; i < bddManager->maxDepth; i++){
    for (j=0; j <= bddManager->numVars; j++){
      CalHashTableQuit(bddManager, bddManager->reqQue[i][j]);
    }
    Cal_MemFree(bddManager->reqQue[i]);
  }
  
  for (i=0; i <= bddManager->numVars; i++){
    CalHashTableQuit(bddManager, bddManager->uniqueTable[i]);
    CalNodeManagerQuit(bddManager->nodeManagerArray[i]);
  }

  CalCacheTableTwoQuit(bddManager->cacheTable);
  Cal_MemFree(bddManager->tempAssociation->varAssociation);
  /*CAL_BDD_FREE_REC(bddManager, bddManager->tempAssociation, CalAssociation_t);*/
  Cal_MemFree(bddManager->tempAssociation);
  /*CAL_BDD_FREE_REC(bddManager, bddManager->superBlock, Cal_Block_t);*/
  CalFreeBlockRecursively(bddManager->superBlock);
  CalAssociationListFree(bddManager);
  Cal_MemFree(bddManager->varBdds);
  Cal_MemFree(bddManager->indexToId);
  Cal_MemFree(bddManager->idToIndex);
  Cal_MemFree(bddManager->uniqueTable);
  Cal_MemFree(bddManager->reqQue);
  Cal_MemFree(bddManager->requestNodeListArray);
  Cal_MemFree(bddManager->nodeManagerArray);
  CalPageManagerQuit(bddManager->pageManager1);
  CalPageManagerQuit(bddManager->pageManager2);
  Cal_MemFree(bddManager);

  return 0;
}

/**Function********************************************************************

  Synopsis    [Sets appropriate fields of BDD Manager.]

  Description [This function is used to set the parameters which are
  used to control the reordering process. "reorderingThreshold"
  determines the number of nodes below which reordering will NOT be
  invoked, "maxForwardedNodes" determines the maximum number of
  forwarded nodes which are allowed (at that point the cleanup must be
  done), and "repackingThreshold" determines the fraction of the page
  utilized below which repacking has to be invoked. These parameters
  have different affect on the computational and memory usage aspects
  of reordeing. For instance, higher value of "maxForwardedNodes" will
  result in process consuming more memory, and a lower value on the
  other hand would invoke the cleanup process repeatedly resulting in
  increased computation.]
  Sets appropriate fields of BDD Manager]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void
Cal_BddManagerSetParameters(Cal_BddManager bddManager,
                            long reorderingThreshold,
                            long maxForwardedNodes,
                            double repackAfterGCThreshold,
                            double tableRepackThreshold)
{
  if (reorderingThreshold >= 0){
    bddManager->reorderingThreshold = reorderingThreshold;
  }
  if (maxForwardedNodes >= 0){
    bddManager->maxForwardedNodes = maxForwardedNodes;
  }
  if (repackAfterGCThreshold >= 0.0){
    bddManager->repackAfterGCThreshold = repackAfterGCThreshold;
  }
  if (tableRepackThreshold >= 0.0){
    bddManager->tableRepackThreshold = tableRepackThreshold;
  }
}


/**Function********************************************************************

  Synopsis    [Returns the number of BDD nodes]

  Description [Returns the number of BDD nodes]

  SideEffects [None]

  SeeAlso     [Cal_BddTotalSize]

******************************************************************************/
unsigned long
Cal_BddManagerGetNumNodes(Cal_BddManager bddManager)
{
  return  bddManager->numNodes;
}



/**Function********************************************************************

  Synopsis    [Creates and returns a new variable at the start of the variable
  order.]

  Description [Creates and returns a new variable at the start of the
  variable order.]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddManagerCreateNewVarFirst(Cal_BddManager bddManager)
{
  return CalBddGetExternalBdd(bddManager, CalBddManagerCreateNewVar(bddManager,
                                                        (Cal_BddIndex_t)0));
}

/**Function********************************************************************

  Synopsis    [Creates and returns a new variable at the end of the variable
  order.]

  Description [Creates and returns a new variable at the end of the variable
  order.]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddManagerCreateNewVarLast(Cal_BddManager bddManager)
{
  return CalBddGetExternalBdd(bddManager,
                              CalBddManagerCreateNewVar(bddManager,
                                                        (Cal_BddIndex_t)
                                                        bddManager->numVars));
}



/**Function********************************************************************

  Synopsis    [Creates and returns a new variable before the specified one in
  the variable order.]

  Description [Creates and returns a new variable before the specified one in
  the variable order.]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddManagerCreateNewVarBefore(Cal_BddManager bddManager,
                                 Cal_Bdd userBdd)
{
  Cal_Bdd_t calBdd = CalBddGetInternalBdd(bddManager, userBdd);
  if (CalBddIsBddConst(calBdd)){
    return Cal_BddManagerCreateNewVarLast(bddManager);
  }
  else{
    return CalBddGetExternalBdd(bddManager,
                                CalBddManagerCreateNewVar(bddManager,
                                                          CalBddGetBddIndex(bddManager, 
                                                  calBdd)));
  }
}

/**Function********************************************************************

  Synopsis    [Creates and returns a new variable after the specified one in
  the variable  order.]

  Description [Creates and returns a new variable after the specified one in
  the variable  order.]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddManagerCreateNewVarAfter(Cal_BddManager bddManager,
                                Cal_Bdd userBdd)
{
  Cal_Bdd_t calBdd = CalBddGetInternalBdd(bddManager, userBdd);
  if (CalBddIsBddConst(calBdd)){
    return Cal_BddManagerCreateNewVarLast(bddManager);
  }
  else{
    return CalBddGetExternalBdd(bddManager,
                                CalBddManagerCreateNewVar(bddManager,
                                                          CalBddGetBddIndex(bddManager, calBdd)+1));
  }
}


/**Function********************************************************************

  Synopsis    [Returns the variable with the specified index, null if no
  such variable exists]

  Description [Returns the variable with the specified index, null if no
  such variable exists]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddManagerGetVarWithIndex(Cal_BddManager bddManager, Cal_BddIndex_t  index)
{
  if (index >= bddManager->numVars){
    CalBddWarningMessage("Index out of range");
    return (Cal_Bdd) 0;
  }
  return CalBddGetExternalBdd(bddManager,
                              bddManager->varBdds[bddManager->indexToId[index]]); 
}

/**Function********************************************************************

  Synopsis    [Returns the variable with the specified id, null if no
  such variable exists]

  Description [Returns the variable with the specified id, null if no
  such variable exists]

  SideEffects [None]

  SeeAlso     [optional]

******************************************************************************/
Cal_Bdd
Cal_BddManagerGetVarWithId(Cal_BddManager bddManager,  Cal_BddId_t  id)
{
  if (id <= 0 || id > bddManager->numVars){
    CalBddWarningMessage("Id out of range");
    return (Cal_Bdd) 0;
  }
  return CalBddGetExternalBdd(bddManager, bddManager->varBdds[id]);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [This function creates and returns a new variable with given
  index value.]

  Description [Right now this function does not handle the case when the
  package is working in multiprocessor mode. We need to put in the necessary
  code later.]

  SideEffects [If the number of variables in the manager exceeds that of value
  of numMaxVars, then we need to reallocate various fields of the manager. Also
  depending upon the value of "index", idToIndex and indexToId tables would
  change.]

******************************************************************************/
Cal_Bdd_t
CalBddManagerCreateNewVar(Cal_BddManager_t * bddManager, Cal_BddIndex_t  index)
{
  Cal_Bdd_t calBdd;
  Cal_BddId_t varId;
  int totalNumVars, maxNumVars, i;
  CalAssociation_t *association;
  
  if (bddManager->numVars == CAL_MAX_VAR_ID){
    CalBddFatalMessage("Cannot create any new variable, no more Id left."); 
  }

  /*
   * Get the total number of variables. If the index is more than the total
   * number of variables, then report error.
   */
  totalNumVars = bddManager->numVars;
  
  if (index > totalNumVars){
    CalBddFatalMessage("The variable index out of range");
  }
  

  /*
   * Create a new variable in the manager which contains this index.
   * This might lead to change in the id->index, and index->id
   * for other managers.
   */

  /*
   * If the number of variables is equal to the maximum number of variables
   * then reallocate memory.
   */
  if (bddManager->numVars == bddManager->maxNumVars){
    int oldMaxNumVars;
    CalAssociation_t *p;
    
    oldMaxNumVars = bddManager->maxNumVars;
    if ((maxNumVars = 2*oldMaxNumVars) > CAL_MAX_VAR_ID){
      maxNumVars = CAL_MAX_VAR_ID;
    }
    bddManager->maxNumVars = maxNumVars;
    bddManager->varBdds = Cal_MemRealloc(Cal_Bdd_t,
                                         bddManager->varBdds, maxNumVars+1); 
    
    bddManager->nodeManagerArray = Cal_MemRealloc(CalNodeManager_t *,
                                                  bddManager->nodeManagerArray, 
                                                  maxNumVars+1);

    bddManager->idToIndex = Cal_MemRealloc(Cal_BddIndex_t, bddManager->idToIndex,
                                        maxNumVars+1);

    bddManager->indexToId = Cal_MemRealloc(Cal_BddIndex_t, bddManager->indexToId,
                                        maxNumVars);

    bddManager->uniqueTable = Cal_MemRealloc(CalHashTable_t *,
                                          bddManager->uniqueTable, maxNumVars+1);
    
    for (i=0; i<bddManager->maxDepth; i++){
      bddManager->reqQue[i] = Cal_MemRealloc(CalHashTable_t *, bddManager->reqQue[i],
                                          maxNumVars+1);
    }
    bddManager->tempAssociation->varAssociation = 
        Cal_MemRealloc(Cal_Bdd_t, bddManager->tempAssociation->varAssociation,
        maxNumVars+1);
    /* CHECK LOOP INDICES */
    for(i = oldMaxNumVars+1; i < maxNumVars+1; i++){
      bddManager->tempAssociation->varAssociation[i] = bddManager->bddNull;
    }
    for(p = bddManager->associationList; p; p = p->next){
      p->varAssociation = 
          Cal_MemRealloc(Cal_Bdd_t, p->varAssociation, maxNumVars+1);
      /* CHECK LOOP INDICES */
      for(i = oldMaxNumVars+1; i < maxNumVars+1; i++){
        p->varAssociation[i] = bddManager->bddNull;
      }
    }
  }

  /* If the variable has been created in the middle, shift the indices. */
  if (index != bddManager->numVars){
    for (i=0; i<bddManager->numVars; i++){
      if (bddManager->idToIndex[i+1] >= index){
        bddManager->idToIndex[i+1]++;
      }
    }
  }

  /* Fix indexToId table */
  for (i=bddManager->numVars; i>index; i--){
    bddManager->indexToId[i] = bddManager->indexToId[i-1];
  }

  for(association = bddManager->associationList; association;
                                              association =
                                                  association->next){
    if (association->lastBddIndex >= index){
      association->lastBddIndex++;
    }
  }
  if (bddManager->tempAssociation->lastBddIndex >= index){
    bddManager->tempAssociation->lastBddIndex++;
  }
  
  bddManager->numVars++;
  varId = bddManager->numVars;

  bddManager->idToIndex[varId] = index;
  bddManager->indexToId[index] = varId;
  
  bddManager->nodeManagerArray[varId] =
      CalNodeManagerInit(bddManager->pageManager2); 
  bddManager->uniqueTable[varId] =
      CalHashTableInit(bddManager, varId);
    
  /* insert node in the uniqueTableForId */
  CalHashTableAddDirectAux(bddManager->uniqueTable[varId],
                           bddManager->bddOne, bddManager->bddZero, &calBdd);
  CalBddPutRefCount(calBdd, CAL_MAX_REF_COUNT);
  bddManager->varBdds[varId] = calBdd;

  bddManager->numNodes++;
  
#ifdef __OLD__
  /* initialize req_que_for_id */
  bddManager->reqQue[varId] = Cal_MemAlloc(CalHashTable_t*, bddManager->maxDepth);
  for (i=0; i<manager->maxDepth; i++){
    bddManager->reqQue[varId][i] = CalHashTableInit(bddManager, varId);
  }
#endif
  
  /* initialize req_que_for_id */
  for (i=0; i<bddManager->maxDepth; i++){
    bddManager->reqQue[i][varId] =
        CalHashTableInit(bddManager, varId);
  }
  CalBddShiftBlock(bddManager, bddManager->superBlock, (long)index);
  return calBdd;
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
static void
BddDefaultTransformFn(
  Cal_BddManager_t * bddManager,
  CalAddress_t  value1,
  CalAddress_t  value2,
  CalAddress_t * result1,
  CalAddress_t * result2,
  Cal_Pointer_t  pointer)
{
  if (!value2)
    /* Will be a carry when taking 2's complement of value2.  Thus, */
    /* take 2's complement of high part. */
    value1= -(long)value1;
  else
    {
      value2= -(long)value2;
      value1= ~value1;
    }
  *result1=value1;
  *result2=value2;
}

#ifdef CALBDDMANAGER

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static int
CalBddManagerPrint(Cal_BddManager_t *bddManager)
{
  int i;
  CalHashTable_t *uniqueTableForId;
  printf("#####################   BDD MANAGER   #####################\n");
  for(i = 1; i < bddManager->numVars; i++){
    uniqueTableForId = bddManager->uniqueTable[i];
    CalHashTablePrint(uniqueTableForId);
  }
  fflush(stdout);
  return 0;
}


main(argc, argv)
int argc;
char **argv;
{
	Cal_Bdd_t n;
	Cal_BddManager_t *manager;

	manager = CalBddManagerInit(argc, argv);
	n = CalBddManagerCreateVariable(bddManager);
	CalBddFunctionPrint(n);
	n = CalBddManagerGetVariable(bddManager, 0);
	CalBddFunctionPrint(n);
}
#endif /* CALBDDMANAGER */

#ifdef __GC__
main(argc, argv)
int argc;
char **argv;
{
  Cal_BddManager_t *manager;
  Cal_Bdd_t vars[256];
  Cal_Bdd_t function, tempFunction;
  int i;
  int numVars;
  
  if (argc >= 2)
        numVars = atoi(argv[1]);
  else
    numVars = 3;
  
  manager = Cal_BddManagerInit();
  
  for (i = 0; i < numVars; i++){
    vars[i] = Cal_BddManagerCreateNewVarLast(bddManager);
  }
  
  function = vars[0];
  for (i = 1; i < numVars - 1; i++){
    tempFunction = Cal_BddITE(bddManager, vars[i], vars[i+1], function);
    Cal_BddFree(function);
    function = tempFunction;
            /*fprintf(stdout, "i = %d\n", i);
              unique_table_write(stdout, CalBddManager->unique_table);
              */
  }
  fprintf(stdout, "\n******************Before Free****************\n");
  for (i = 1; i <= numVars; i++){
    CalHashTablePrint(bddManager->uniqueTable[i]);
  }
  Cal_BddFree(function);
  fprintf(stdout, "\n****************After Free****************\n");
  for (i = 1; i <= numVars; i++){
    CalHashTablePrint(bddManager->uniqueTable[i]);
  }
  Cal_BddManagerGC(bddManager);
  fprintf(stdout, "\n****************After GC****************\n");
  for (i = 1; i <= numVars; i++){
    CalHashTablePrint(bddManager->uniqueTable[i]);
  }
}
#endif
