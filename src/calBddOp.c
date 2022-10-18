/**CFile***********************************************************************

  FileName    [calBddOp.c]

  PackageName [cal]

  Synopsis    [Routines for performing simple boolean operations on a
  pair of BDDs or on an array of pair of BDDs or on an array of BDDs.]

  Description [The "cal" specific routines are "Cal_BddPairwiseAnd/Or",
  "Cal_BddMultiwayAnd/Or".] 

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

  Revision    [$Id: calBddOp.c,v 1.1.1.3 1998/05/04 00:58:52 hsv Exp $]

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

static Cal_Bdd_t * BddArrayOpBF(Cal_BddManager_t * bddManager, Cal_Bdd_t* bddArray, int numFunction, CalOpProc_t calOpProc);
static Cal_Bdd_t BddMultiwayOp(Cal_BddManager_t * bddManager, Cal_Bdd_t * calBddArray, int numBdds, CalOpProc_t op);
static void BddArrayToRequestNodeListArray(Cal_BddManager_t * bddManager, Cal_Bdd_t * calBddArray, int numBdds);
static int CeilLog2(int number);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Returns the BDD for logical AND of argument BDDs]

  Description [Returns the BDD for logical AND of f and g]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddAnd(
  Cal_BddManager bddManager,
  Cal_Bdd  fUserBdd,
  Cal_Bdd  gUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd userResult;
  Cal_Bdd_t F, G;

  if (CalBddPreProcessing(bddManager, 2, fUserBdd, gUserBdd)){
    F = CalBddGetInternalBdd(bddManager, fUserBdd);
    G = CalBddGetInternalBdd(bddManager, gUserBdd);
    result = CalBddOpBF(bddManager, CalOpAnd, F, G);
  }
  else {
    return (Cal_Bdd)0;
  }
  userResult =  CalBddGetExternalBdd(bddManager, result);
  if (CalBddPostProcessing(bddManager) == CAL_BDD_OVERFLOWED){
    Cal_BddFree(bddManager, userResult);
    Cal_BddManagerGC(bddManager);
    return (Cal_Bdd) 0;
  }
  return userResult;
}

/**Function********************************************************************

  Synopsis    [Returns the BDD for logical NAND of argument BDDs]

  Description [Returns the BDD for logical NAND of f and g]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddNand(
  Cal_BddManager bddManager,
  Cal_Bdd fUserBdd,
  Cal_Bdd gUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd_t F, G;
  Cal_Bdd userResult;
  if (CalBddPreProcessing(bddManager, 2, fUserBdd, gUserBdd)){
    F = CalBddGetInternalBdd(bddManager, fUserBdd); 
    G = CalBddGetInternalBdd(bddManager, gUserBdd); 
    result = CalBddOpBF(bddManager, CalOpNand, F, G);
  }
  else{
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

/**Function********************************************************************

  Synopsis    [Returns the BDD for logical OR of argument BDDs]

  Description [Returns the BDD for logical OR of f and g]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddOr(Cal_BddManager bddManager,
          Cal_Bdd fUserBdd,
          Cal_Bdd gUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd_t F, G;
  Cal_Bdd userResult;
  if (CalBddPreProcessing(bddManager, 2, fUserBdd, gUserBdd)){
    F = CalBddGetInternalBdd(bddManager, fUserBdd); 
    G = CalBddGetInternalBdd(bddManager, gUserBdd); 
    result = CalBddOpBF(bddManager, CalOpOr, F, G);
  }
  else{
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

/**Function********************************************************************

  Synopsis    [Returns the BDD for logical NOR of argument BDDs]

  Description [Returns the BDD for logical NOR of f and g]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddNor(Cal_BddManager bddManager,
           Cal_Bdd fUserBdd,
           Cal_Bdd gUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd userResult;
  Cal_Bdd_t F, G;
  if (CalBddPreProcessing(bddManager, 2, fUserBdd, gUserBdd)){
    F = CalBddGetInternalBdd(bddManager, fUserBdd); 
    G = CalBddGetInternalBdd(bddManager, gUserBdd); 
    result = CalBddOpBF(bddManager, CalOpOr, F, G);
    CalBddNot(result, result);
  }
  else{
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

/**Function********************************************************************

  Synopsis    [Returns the BDD for logical exclusive OR of argument BDDs]

  Description [Returns the BDD for logical exclusive OR of f and g]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddXor(Cal_BddManager bddManager,
           Cal_Bdd fUserBdd,
           Cal_Bdd gUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd userResult;
  Cal_Bdd_t F, G;
  if (CalBddPreProcessing(bddManager, 2, fUserBdd, gUserBdd)){
    F = CalBddGetInternalBdd(bddManager, fUserBdd); 
    G = CalBddGetInternalBdd(bddManager, gUserBdd); 
    result = CalBddOpBF(bddManager, CalOpXor, F, G);
  }
  else{
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

/**Function********************************************************************

  Synopsis    [Returns the BDD for logical exclusive NOR of argument BDDs]

  Description [Returns the BDD for logical exclusive NOR of f and g]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddXnor(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd userResult;
  Cal_Bdd_t F, G;
  if (CalBddPreProcessing(bddManager, 2, fUserBdd, gUserBdd)){
    F = CalBddGetInternalBdd(bddManager, fUserBdd); 
    G = CalBddGetInternalBdd(bddManager, gUserBdd); 
    result = CalBddOpBF(bddManager, CalOpXor, F, G);
    CalBddNot(result, result);
  }
  else{
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

/**Function********************************************************************

  Synopsis    [Returns an array of BDDs obtained by logical AND of BDD pairs
  specified by an BDD array in which a BDD at an even location is paired with
  a BDD at an odd location of the array]

  Description [Returns an array of BDDs obtained by logical AND of BDD pairs
  specified by an BDD array in which a BDD at an even location is paired with
  a BDD at an odd location of the array]

  SideEffects [None]

  SeeAlso     [Cal_BddPairwiseOr]

******************************************************************************/
Cal_Bdd *
Cal_BddPairwiseAnd(Cal_BddManager bddManager, Cal_Bdd *userBddArray)
{
  int i, num;
  Cal_Bdd_t *bddArray;
  Cal_Bdd_t *resultArray;
  Cal_Bdd userBdd;
  Cal_Bdd *userResultArray;
 
  for (num = 0; (userBdd = userBddArray[num]); num++){
    if (CalBddPreProcessing(bddManager, 1, userBdd) == 0){
      return Cal_Nil(Cal_Bdd);
    }
  }
  if ((num == 0) || (num%2 != 0)){
    fprintf(stdout,"Odd number of arguments passed to array AND\n");
    return Cal_Nil(Cal_Bdd);
  }
  bddArray = Cal_MemAlloc(Cal_Bdd_t, num);
  for (i = 0; i < num; i++){
    bddArray[i] = CalBddGetInternalBdd(bddManager, userBddArray[i]);
  }
  resultArray =  BddArrayOpBF(bddManager, bddArray, num, CalOpAnd);
  userResultArray = Cal_MemAlloc(Cal_Bdd, num/2);
  for (i=0; i<num/2; i++){
    userResultArray[i] = CalBddGetExternalBdd(bddManager, resultArray[i]);
  }
  Cal_MemFree(bddArray);
  Cal_MemFree(resultArray);
  if (CalBddPostProcessing(bddManager)  == CAL_BDD_OVERFLOWED){
    for (i=0; i<num/2; i++){
      Cal_BddFree(bddManager, userResultArray[i]);
      userResultArray[i] = (Cal_Bdd) 0;
    }
    Cal_BddManagerGC(bddManager);
    return userResultArray;
  }
  return userResultArray;
}

/**Function********************************************************************

  Synopsis    [Returns an array of BDDs obtained by logical OR of BDD pairs
  specified by an BDD array in which a BDD at an even location is paired with
  a BDD at an odd location of the array]

  Description [Returns an array of BDDs obtained by logical OR of BDD pairs
  specified by an BDD array in which a BDD at an even location is paired with
  a BDD at an odd location of the array]

  SideEffects [None]

  SeeAlso     [Cal_BddPairwiseAnd]

******************************************************************************/
Cal_Bdd *
Cal_BddPairwiseOr(Cal_BddManager bddManager, Cal_Bdd *userBddArray)
{
  int i, num=0;
  Cal_Bdd_t *bddArray;
  Cal_Bdd_t *resultArray;
  Cal_Bdd userBdd;
  Cal_Bdd *userResultArray;
 
  for (num = 0; (userBdd = userBddArray[num]); num++){
    if (CalBddPreProcessing(bddManager, 1, userBdd) == 0){
      return Cal_Nil(Cal_Bdd);
    }
  }
  if ((num == 0) || (num%2 != 0)){
    fprintf(stdout,"Odd number of arguments passed to array OR\n");
    return Cal_Nil(Cal_Bdd);
  }
  bddArray = Cal_MemAlloc(Cal_Bdd_t, num);
  for (i = 0; i < num; i++){
    bddArray[i] = CalBddGetInternalBdd(bddManager, userBddArray[i]);
  }
  resultArray =  BddArrayOpBF(bddManager, bddArray, num, CalOpOr);
  userResultArray = Cal_MemAlloc(Cal_Bdd, num/2);
  for (i=0; i<num/2; i++){
    userResultArray[i] = CalBddGetExternalBdd(bddManager, resultArray[i]);
  }
  Cal_MemFree(bddArray);
  Cal_MemFree(resultArray);
  return userResultArray;
}

/**Function********************************************************************

  Synopsis    [Returns an array of BDDs obtained by logical XOR of BDD pairs
  specified by an BDD array in which a BDD at an even location is paired with
  a BDD at an odd location of the array]

  Description [Returns an array of BDDs obtained by logical XOR of BDD pairs
  specified by an BDD array in which a BDD at an even location is paired with
  a BDD at an odd location of the array]

  SideEffects [None]

  SeeAlso     [Cal_BddPairwiseAnd]

******************************************************************************/
Cal_Bdd *
Cal_BddPairwiseXor(Cal_BddManager bddManager, Cal_Bdd *userBddArray)
{
  int i, num=0;
  Cal_Bdd_t *bddArray;
  Cal_Bdd_t *resultArray;
  Cal_Bdd userBdd;
  Cal_Bdd *userResultArray;
 
  for (num = 0; (userBdd = userBddArray[num]); num++){
    if (CalBddPreProcessing(bddManager, 1, userBdd) == 0){
      return Cal_Nil(Cal_Bdd);
    }
  }
  if ((num == 0) || (num%2 != 0)){
    fprintf(stdout,"Odd number of arguments passed to array OR\n");
    return Cal_Nil(Cal_Bdd);
  }
  bddArray = Cal_MemAlloc(Cal_Bdd_t, num);
  for (i = 0; i < num; i++){
    bddArray[i] = CalBddGetInternalBdd(bddManager, userBddArray[i]);
  }
  resultArray =  BddArrayOpBF(bddManager, bddArray, num, CalOpXor);
  userResultArray = Cal_MemAlloc(Cal_Bdd, num/2);
  for (i=0; i<num/2; i++){
    userResultArray[i] = CalBddGetExternalBdd(bddManager, resultArray[i]);
  }
  Cal_MemFree(bddArray);
  Cal_MemFree(resultArray);
  return userResultArray;
}



/**Function********************************************************************

  Synopsis    [Returns the BDD for logical AND of argument BDDs]

  Description [Returns the BDD for logical AND of set of BDDs in the bddArray]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddMultiwayAnd(Cal_BddManager bddManager, Cal_Bdd *userBddArray)
{
  int i, numBdds = 0;
  Cal_Bdd_t result;
  Cal_Bdd_t *calBddArray;
  Cal_Bdd userBdd;

  for (numBdds  = 0; (userBdd = userBddArray[numBdds]); numBdds++){
    if (CalBddPreProcessing(bddManager, 1, userBdd) == 0){
      return (Cal_Bdd) 0;
    }
  }
  
  if (numBdds == 0){
    CalBddWarningMessage("Multiway AND called with 0 length array");
    return (Cal_Bdd) 0;
  }
  else if (numBdds == 1){
    return Cal_BddIdentity(bddManager, userBddArray[0]);
  }
  else{
    calBddArray = Cal_MemAlloc(Cal_Bdd_t, numBdds+1);
    for (i = 0; i < numBdds; i++){
      calBddArray[i] = CalBddGetInternalBdd(bddManager, userBddArray[i]);
    }
    result = BddMultiwayOp(bddManager, calBddArray, numBdds, CalOpAnd);
    Cal_MemFree(calBddArray);
  }
  return CalBddGetExternalBdd(bddManager, result);
}
 
/**Function********************************************************************

  Synopsis    [Returns the BDD for logical OR of argument BDDs]

  Description [Returns the BDD for logical OR of set of BDDs in the bddArray]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddMultiwayOr(Cal_BddManager bddManager, Cal_Bdd *userBddArray)
{
  int i, numBdds = 0;
  Cal_Bdd_t result;
  Cal_Bdd_t *calBddArray;
  Cal_Bdd userBdd;

  for (numBdds = 0; (userBdd = userBddArray[numBdds]); numBdds++){
    if (CalBddPreProcessing(bddManager, 1, userBdd) == 0){
      return (Cal_Bdd) 0;
    }
  }
  
  if (numBdds == 0){
    CalBddWarningMessage("Multiway OR called with 0 length array");
    return (Cal_Bdd) 0;
  }
  else if (numBdds == 1){
    return Cal_BddIdentity(bddManager, userBddArray[0]);
  }
  else{
    calBddArray = Cal_MemAlloc(Cal_Bdd_t, numBdds+1);
    for (i = 0; i < numBdds; i++){
      calBddArray[i] = CalBddGetInternalBdd(bddManager, userBddArray[i]);
    }
    result = BddMultiwayOp(bddManager, calBddArray, numBdds, CalOpOr);
    Cal_MemFree(calBddArray);
  }
  return CalBddGetExternalBdd(bddManager, result);
}
 
/**Function********************************************************************

  Synopsis    [Returns the BDD for logical XOR of argument BDDs]

  Description [Returns the BDD for logical XOR of set of BDDs in the bddArray]

  SideEffects [None]

******************************************************************************/
Cal_Bdd
Cal_BddMultiwayXor(Cal_BddManager bddManager, Cal_Bdd *userBddArray)
{
  int i, numBdds = 0;
  Cal_Bdd_t result;
  Cal_Bdd_t *calBddArray;
  Cal_Bdd userBdd;

  for (numBdds = 0; (userBdd = userBddArray[numBdds]); numBdds++){
    if (CalBddPreProcessing(bddManager, 1, userBdd) == 0){
      return (Cal_Bdd) 0;
    }
  }
  
  if (numBdds == 0){
    CalBddWarningMessage("Multiway OR called with 0 length array");
    return (Cal_Bdd) 0;
  }
  else if (numBdds == 1){
    return Cal_BddIdentity(bddManager, userBddArray[0]);
  }
  else{
    calBddArray = Cal_MemAlloc(Cal_Bdd_t, numBdds+1);
    for (i = 0; i < numBdds; i++){
      calBddArray[i] = CalBddGetInternalBdd(bddManager, userBddArray[i]);
    }
    result = BddMultiwayOp(bddManager, calBddArray, numBdds, CalOpXor);
    Cal_MemFree(calBddArray);
  }
  return CalBddGetExternalBdd(bddManager, result);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Computes result BDDs for an array of lists, each entry of which 
  is pair of pointers, each of which points to a operand BDD or an entry in
  another list with a smaller array index]

  Description [Computes result BDDs for an array of lists, each entry of which
  is pair of pointers, each of which points to a operand BDD or an entry in
  another list with a smaller array index]

  SideEffects [ThenBDD pointer of an entry is over written by the result BDD
  and ElseBDD pointer is marked using FORWARD_FLAG]

******************************************************************************/
void
CalRequestNodeListArrayOp(Cal_BddManager_t * bddManager,
                          CalRequestNode_t ** requestNodeListArray,
                          CalOpProc_t calOpProc)
{
  CalRequestNode_t *requestNode;
  CalRequest_t F, G, result;
  int pipeDepth, bddId, bddIndex;
  CalHashTable_t **reqQueAtPipeDepth, *hashTable, *uniqueTableForId;
  CalHashTable_t ***reqQue = bddManager->reqQue;
  
  /* ReqQueInsertUsingReqListArray */
  for(pipeDepth = 0; pipeDepth < bddManager->depth; pipeDepth++){
    reqQueAtPipeDepth = reqQue[pipeDepth];
    for(requestNode = requestNodeListArray[pipeDepth];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      CalRequestNodeGetF(requestNode, F);
      CalRequestIsForwardedTo(F);
      CalRequestNodeGetG(requestNode, G);
      CalRequestIsForwardedTo(G);
      if((*calOpProc)(bddManager, F, G, &result) == 0){
        CalBddNormalize(F, G);
        CalBddGetMinId2(bddManager, F, G, bddId);
        CalHashTableFindOrAdd(reqQueAtPipeDepth[bddId], F, G, &result);
      }
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
        CalHashTableApply(bddManager, hashTable, reqQueAtPipeDepth, calOpProc);
      }
    }
  }

#ifdef COMPUTE_MEMORY_OVERHEAD
  {
    calNumEntriesAfterApply = 0;
    for(bddIndex = 0; bddIndex < bddManager->numVars; bddIndex++){
      bddId = bddManager->indexToId[bddIndex];
      for(pipeDepth = 0; pipeDepth < bddManager->depth; pipeDepth++){
        reqQueAtPipeDepth = reqQue[pipeDepth];
        hashTable = reqQueAtPipeDepth[bddId];
        calNumEntriesAfterApply += hashTable->numEntries;
      }
    }
  }
#endif

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

#ifdef COMPUTE_MEMORY_OVERHEAD
  {
    CalRequestNode_t *requestNode;
    calNumEntriesAfterReduce = 0;
    for(bddIndex = 0; bddIndex < bddManager->numVars; bddIndex++){
      CalRequestNode_t *next;
      Cal_BddId_t bddId;
      bddId = bddManager->indexToId[bddIndex];
      for(pipeDepth = 0; pipeDepth < bddManager->depth; pipeDepth++){
        hashTable = reqQue[pipeDepth][bddId];
        for (requestNode = hashTable->requestNodeList;
             requestNode != Cal_Nil(CalRequestNode_t); requestNode = next){
          next = CalRequestNodeGetNextRequestNode(requestNode);
          calNumEntriesAfterReduce++;
        }
      }
    }
    calAfterReduceToAfterApplyNodesRatio =
        ((double)calNumEntriesAfterReduce)/((double)calNumEntriesAfterApply); 
    calAfterReduceToUniqueTableNodesRatio = 
        ((double)calNumEntriesAfterReduce)/((double)bddManager->numNodes);
  }
#endif

  /* ReqListArrayReqForward */
  for(pipeDepth = 0; pipeDepth < bddManager->depth; pipeDepth++){
    for(requestNode = requestNodeListArray[pipeDepth];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      CalRequestNodeGetThenRequest(requestNode, result);
      CalRequestIsForwardedTo(result);
      Cal_Assert(CalBddIsForwarded(result) == 0);
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

  Synopsis    [Internal routine to compute a logical operation on a pair of BDDs]

  Description [Internal routine to compute a logical operation on a pair of BDDs]

  SideEffects [None]

******************************************************************************/
Cal_Bdd_t
CalBddOpBF(
  Cal_BddManager_t * bddManager,
  CalOpProc_t calOpProc,
  Cal_Bdd_t  F,
  Cal_Bdd_t  G)
{
  Cal_Bdd_t result;
  Cal_BddId_t minId, bddId;
  /*Cal_BddIndex_t minIndex; Commented out because of the problem on alpha*/ 
  int minIndex;
  int bddIndex;
  CalHashTable_t *hashTable, **hashTableArray, *uniqueTableForId;
  
  if (calOpProc(bddManager, F, G, &result)){
    return result;
  }
  CalBddGetMinIdAndMinIndex(bddManager, F, G, minId, minIndex);
  hashTableArray = bddManager->reqQue[0];
  CalHashTableFindOrAdd(hashTableArray[minId], F, G, &result);
  
  /* ReqQueApply */
  for(bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTable = hashTableArray[bddId];
    if(hashTable->numEntries){
      CalHashTableApply(bddManager, hashTable, hashTableArray, calOpProc);
    }
  }
#ifdef COMPUTE_MEMORY_OVERHEAD
  {
    calNumEntriesAfterApply = 0;
    for(bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
      bddId = bddManager->indexToId[bddIndex];
      hashTable = hashTableArray[bddId];
      calNumEntriesAfterApply += hashTable->numEntries;
    }
  }
#endif
  /* ReqQueReduce */
  for(bddIndex = bddManager->numVars - 1; bddIndex >= minIndex; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    hashTable = hashTableArray[bddId];
    if(hashTable->numEntries){
      CalHashTableReduce(bddManager, hashTable, uniqueTableForId);
    }
  }
  CalRequestIsForwardedTo(result);

#ifdef COMPUTE_MEMORY_OVERHEAD
  {
    CalRequestNode_t *requestNode;
    calNumEntriesAfterReduce = 0;
    for(bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
      CalRequestNode_t *next;
      Cal_BddId_t bddId;
      
      bddId = bddManager->indexToId[bddIndex];
      hashTable = hashTableArray[bddId];
      for (requestNode = hashTable->requestNodeList; requestNode !=
                                                         Cal_Nil(CalRequestNode_t);
                                                     requestNode = next){
        next = CalRequestNodeGetNextRequestNode(requestNode);
        calNumEntriesAfterReduce++;
      }
    }
    calAfterReduceToAfterApplyNodesRatio =
        ((double)calNumEntriesAfterReduce)/((double)calNumEntriesAfterApply); 
    calAfterReduceToUniqueTableNodesRatio = 
        ((double)calNumEntriesAfterReduce)/((double)bddManager->numNodes);
  }
#endif

  /* Clean up */
  for(bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    CalHashTableCleanUp(hashTableArray[bddId]);
  }
  return result;
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Internal common routine for Cal_BddPairwiseAnd and Cal_BddPairwiseOr]

  SideEffects [None]

******************************************************************************/
static Cal_Bdd_t *
BddArrayOpBF(Cal_BddManager_t * bddManager, Cal_Bdd_t*  bddArray,
                int numFunction, CalOpProc_t calOpProc)
{
  Cal_BddId_t minId, bddId;
  /*Cal_BddIndex_t minIndex = 0;*/
  int minIndex = 0;
  int bddIndex;
  CalHashTable_t *hashTable, **hashTableArray, *uniqueTableForId;
  Cal_Bdd_t F, G, result;
  int numPairs = numFunction/2;
  Cal_Bdd_t *resultArray = Cal_MemAlloc(Cal_Bdd_t, numPairs);
  int i;
  
  hashTableArray = bddManager->reqQue[0];
  for (i=0; i<numPairs; i++){
    F = bddArray[i<<1];
    G = bddArray[(i<<1)+1];
    if ((*calOpProc)(bddManager, F, G, &result) == 0){
      CalBddGetMinIdAndMinIndex(bddManager, F, G, minId, minIndex);
      CalHashTableFindOrAdd(hashTableArray[minId], F, G, &result);
    }
    resultArray[i] = result;
  }
  
  
  /* ReqQueApply */
  for(bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTable = hashTableArray[bddId];
    if(hashTable->numEntries){
      CalHashTableApply(bddManager, hashTable, hashTableArray, calOpProc);
    }
  }

  /* ReqQueReduce */
  for(bddIndex = bddManager->numVars - 1; bddIndex >= minIndex; bddIndex--){
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    hashTable = hashTableArray[bddId];
    if(hashTable->numEntries){
      CalHashTableReduce(bddManager, hashTable, uniqueTableForId);
    }
  }
  for (i=0; i<numPairs; i++){
    CalRequestIsForwardedTo(resultArray[i]);
  }
  /* Clean up */
  for(bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    CalHashTableCleanUp(hashTableArray[bddId]);
  }
  return resultArray;
}

/**Function********************************************************************

  Synopsis    [Internal routine for multiway operations]

  Description [Internal routine for multiway operations]

  SideEffects [None]

******************************************************************************/
static Cal_Bdd_t
BddMultiwayOp(Cal_BddManager_t * bddManager, Cal_Bdd_t * calBddArray,
              int  numBdds, CalOpProc_t op)
{
  int pipeDepth;
  CalRequestNode_t *requestNode;
  Cal_Bdd_t result;
  
  BddArrayToRequestNodeListArray(bddManager, calBddArray, numBdds);
  CalRequestNodeListArrayOp(bddManager, bddManager->requestNodeListArray, op);
  for(pipeDepth = 0; pipeDepth < bddManager->depth-1; pipeDepth++){
    CalRequestNode_t *next;
    for(requestNode = bddManager->requestNodeListArray[pipeDepth];
        requestNode != Cal_Nil(CalRequestNode_t); requestNode = next){
      next = CalRequestNodeGetNextRequestNode(requestNode);
      CalNodeManagerFreeNode(bddManager->nodeManagerArray[0],
                             requestNode);
    }
    bddManager->requestNodeListArray[pipeDepth] =
        Cal_Nil(CalRequestNode_t);
  }
  requestNode = bddManager->requestNodeListArray[bddManager->depth-1];
  bddManager->requestNodeListArray[bddManager->depth-1] =
      Cal_Nil(CalRequestNode_t); 
  CalRequestNodeGetThenRequest(requestNode, result); 
  CalNodeManagerFreeNode(bddManager->nodeManagerArray[0],
                         requestNode);
  return result;
}


/**Function********************************************************************

  Synopsis    [Converts an array of BDDs to a list of requests representing BDD
  pairs]

  Description [Converts an array of BDDs to a list of requests representing BDD]

  SideEffects [None]

******************************************************************************/
static void
BddArrayToRequestNodeListArray(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t * calBddArray,
  int  numBdds)
{
  int i;
  Cal_Bdd_t lastBdd;
  CalRequestNode_t *even, *odd, *requestNode, *requestNodeList, *head;
  
  bddManager->depth = CeilLog2(numBdds);
  if (bddManager->depth > 10){
    CalBddFatalMessage("Don't be stooopid\n, Use lesser depth\n");
  }
    
  if (bddManager->depth > bddManager->maxDepth){
    /* Need to reallocate the memory for reqQue and
       requestNodeListArray */
    int oldMaxDepth = bddManager->maxDepth;
    bddManager->maxDepth = bddManager->depth;
    
    for (i=0; i<bddManager->maxDepth; i++){
      bddManager->requestNodeListArray[i] = Cal_Nil(CalRequestNode_t);
    }

    bddManager->reqQue = Cal_MemRealloc(CalHashTable_t **, bddManager->reqQue,
                                 bddManager->maxDepth);
    for (i=oldMaxDepth; i<bddManager->maxDepth; i++){
      int j;
      bddManager->reqQue[i] = Cal_MemAlloc(CalHashTable_t *, bddManager->maxNumVars+1);
      for (j=0; j<bddManager->numVars+1; j++){
        bddManager->reqQue[i][j] = CalHashTableInit(bddManager, j);
      }
    }
  }
  lastBdd = bddManager->bddNull;
  if (numBdds%2 != 0){/* Odd number of Bdd's */
    lastBdd = calBddArray[numBdds-1];
  }
  requestNodeList = bddManager->requestNodeListArray[0];
  for (i=0; i<numBdds/2; i++){
    CalNodeManagerAllocNode(bddManager->nodeManagerArray[0], requestNode);
    CalRequestNodePutF(requestNode, calBddArray[2*i]);
    CalRequestNodePutG(requestNode, calBddArray[2*i+1]);
    CalRequestNodePutNextRequestNode(requestNode, requestNodeList);
    requestNodeList = requestNode;
  }
  bddManager->requestNodeListArray[0] = requestNodeList;

  for (i=1; i<bddManager->depth; i++){
    requestNodeList = bddManager->requestNodeListArray[i];
    head = bddManager->requestNodeListArray[i-1];
    while ((odd = head) && (even = head->nextBddNode)){
      CalNodeManagerAllocNode(bddManager->nodeManagerArray[0], requestNode);
      /*
       * We don't have to worry about the Id's attached with
       * the requestNode or with the then and else part of it.
       */
      CalRequestNodePutThenRequestNode(requestNode, odd);
      CalRequestNodePutElseRequestNode(requestNode, even);
      CalRequestNodePutNextRequestNode(requestNode, requestNodeList);
      requestNodeList = requestNode;
      head = CalRequestNodeGetNextRequestNode(even);
    }
    if (odd != Cal_Nil(CalRequestNode_t)){/* There is an  odd node at this
                                      level */
      if (CalBddIsBddNull(bddManager,lastBdd)){ /* There are no odd nodes
                                                 * from previous levels, so
                                                 * make this an odd node.
                                                 */
        CalBddPutBddNode(lastBdd, odd);
      }
      else{ /* There exists an odd node, so make a pair now. */
        CalNodeManagerAllocNode(bddManager->nodeManagerArray[0], requestNode);
        CalRequestNodePutThenRequestNode(requestNode, odd);
        CalRequestNodePutElseRequest(requestNode, lastBdd);
        CalRequestNodePutNextRequestNode(requestNode, requestNodeList);
        lastBdd = bddManager->bddNull;
        requestNodeList = requestNode;
      }
    }
    bddManager->requestNodeListArray[i] = requestNodeList;
  }
}



/**Function********************************************************************

  Synopsis    [Returns the smallest integer greater than or equal to log2 of a
  number]

  Description [Returns the smallest integer greater than or equal to log2 of a
  number (The assumption is that the number is >= 1)]

  SideEffects [None]

******************************************************************************/
static int
CeilLog2(
  int  number)
{
  int num, count;
  for (num=number, count=0; num > 1; num >>= 1, count++);
  if ((1 << count) != number) count++;
  return count;
}


  



