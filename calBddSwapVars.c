/**CFile***********************************************************************

  FileName    [calBddSwapVars.c]

  PackageName [cal]

  Synopsis    [Routine for swapping two variables.]

  Description [Routine for swapping two variables.]

  SeeAlso     [None]

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

  Revision    [$Id: calBddSwapVars.c,v 1.1.1.3 1998/05/04 00:58:55 hsv Exp $]

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

static void CalHashTableSwapVarsApply(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, Cal_BddIndex_t gIndex, Cal_BddIndex_t hIndex, CalHashTable_t ** reqQueForSwapVars, CalHashTable_t ** reqQueForSwapVarsPlus, CalHashTable_t ** reqQueForSwapVarsMinus, CalHashTable_t ** reqQueForCompose, CalHashTable_t ** reqQueForITE);
static void CalHashTableSwapVarsPlusApply(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, Cal_BddIndex_t hIndex, CalHashTable_t ** reqQueForSwapVars, CalHashTable_t ** reqQueForSwapVarsPlus, CalHashTable_t ** reqQueForSwapVarsMinus, CalHashTable_t ** reqQueForCompose);
static void CalHashTableSwapVarsMinusApply(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, Cal_BddIndex_t hIndex, CalHashTable_t ** reqQueForSwapVars, CalHashTable_t ** reqQueForSwapVarsPlus, CalHashTable_t ** reqQueForSwapVarsMinus, CalHashTable_t ** reqQueForCompose);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Return a function obtained by swapping two variables]

  Description [Returns the BDD obtained by simultaneously substituting variable
  g by variable h and variable h and variable g in the BDD f]

  SideEffects [None]

  SeeAlso     [Cal_BddSubstitute]

******************************************************************************/
Cal_Bdd
Cal_BddSwapVars(Cal_BddManager  bddManager, Cal_Bdd  fUserBdd,
                Cal_Bdd gUserBdd,
                Cal_Bdd hUserBdd)
{
  Cal_Bdd_t f,g,h,tmpBdd;
  Cal_BddIndex_t gIndex, hIndex;
  CalRequest_t result;
  int bddId, bddIndex;
  CalHashTable_t *hashTable;
  CalHashTable_t *uniqueTableForId;
  CalHashTable_t **reqQueForSwapVars = bddManager->reqQue[0];
  CalHashTable_t **reqQueForSwapVarsPlus = bddManager->reqQue[1];
  CalHashTable_t **reqQueForSwapVarsMinus = bddManager->reqQue[2];
  CalHashTable_t **reqQueForCompose = bddManager->reqQue[3];
  CalHashTable_t **reqQueForITE = bddManager->reqQue[4]; 
  
  if (CalBddPreProcessing(bddManager, 3, fUserBdd, gUserBdd, hUserBdd) == 0){
	return (Cal_Bdd) 0;
  }
  f = CalBddGetInternalBdd(bddManager, fUserBdd);
  g = CalBddGetInternalBdd(bddManager, gUserBdd);
  h = CalBddGetInternalBdd(bddManager, hUserBdd);

  if(CalBddIsBddConst(g) || CalBddIsBddConst(h)){
    CalBddWarningMessage("Unacceptable arguments for Cal_BddSwapVars");
    return (Cal_Bdd) 0;
  }
  if(CalBddIsEqual(g, h)){
    /*
    CalBddIcrRefCount(f);
    */
    return CalBddGetExternalBdd(bddManager, f);
  }
  if(CalBddGetBddIndex(bddManager, g) > CalBddGetBddIndex(bddManager, h)){
    tmpBdd = g;
    g = h;
    h = tmpBdd;
  }

  gIndex = CalBddGetBddIndex(bddManager, g);
  hIndex = CalBddGetBddIndex(bddManager, h);

  CalBddGetMinId2(bddManager, f, g, bddId);
  CalHashTableFindOrAdd(reqQueForSwapVars[bddId], f, 
      bddManager->bddNull, &result);

  /* ReqQueApply */
  for(bddIndex = 0; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTable = reqQueForSwapVars[bddId];
    if(hashTable->numEntries){
      CalHashTableSwapVarsApply(bddManager, hashTable, gIndex, hIndex,
          reqQueForSwapVars, reqQueForSwapVarsPlus, reqQueForSwapVarsMinus,
          reqQueForCompose, reqQueForITE);
    }
    hashTable = reqQueForSwapVarsPlus[bddId];
    if(hashTable->numEntries){
      CalHashTableSwapVarsPlusApply(bddManager, hashTable, hIndex,
          reqQueForSwapVars, reqQueForSwapVarsPlus, reqQueForSwapVarsMinus,
          reqQueForCompose);
    }
    hashTable = reqQueForSwapVarsMinus[bddId];
    if(hashTable->numEntries){
      CalHashTableSwapVarsMinusApply(bddManager, hashTable, hIndex,
          reqQueForSwapVars, reqQueForSwapVarsPlus, reqQueForSwapVarsMinus,
          reqQueForCompose);
    }
    hashTable = reqQueForCompose[bddId];
    if(hashTable->numEntries){
      CalHashTableComposeApply(bddManager, hashTable, hIndex,
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
    hashTable = reqQueForSwapVars[bddId];
    if(hashTable->numEntries){
      CalHashTableReduce(bddManager, hashTable, uniqueTableForId);
    }
    hashTable = reqQueForSwapVarsPlus[bddId];
    if(hashTable->numEntries){
      CalHashTableReduce(bddManager, hashTable, uniqueTableForId);
    }
    hashTable = reqQueForSwapVarsMinus[bddId];
    if(hashTable->numEntries){
      CalHashTableReduce(bddManager, hashTable, uniqueTableForId);
    }
    hashTable = reqQueForCompose[bddId];
    if(hashTable->numEntries){
      CalHashTableReduce(bddManager, hashTable, uniqueTableForId);
    }
    hashTable = reqQueForITE[bddId];
    if(hashTable->numEntries){
      CalHashTableReduce(bddManager, hashTable, uniqueTableForId);
    }
  }

  CalRequestIsForwardedTo(result);

  /* ReqQueCleanUp */
  for(bddId = 1; bddId <= bddManager->numVars; bddId++){
    CalHashTableCleanUp(reqQueForSwapVars[bddId]);
    CalHashTableCleanUp(reqQueForSwapVarsPlus[bddId]);
    CalHashTableCleanUp(reqQueForSwapVarsMinus[bddId]);
    CalHashTableCleanUp(reqQueForCompose[bddId]);
    CalHashTableCleanUp(reqQueForITE[bddId]);
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
CalHashTableSwapVarsApply(
  Cal_BddManager_t * bddManager,
  CalHashTable_t * hashTable,
  Cal_BddIndex_t  gIndex,
  Cal_BddIndex_t  hIndex,
  CalHashTable_t ** reqQueForSwapVars,
  CalHashTable_t ** reqQueForSwapVarsPlus,
  CalHashTable_t ** reqQueForSwapVarsMinus,
  CalHashTable_t ** reqQueForCompose,
  CalHashTable_t ** reqQueForITE)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  CalRequestNode_t *requestNode;
  Cal_BddId_t bddId;
  Cal_BddIndex_t fIndex, bddIndex;
  Cal_Bdd_t f, calBdd;
  Cal_Bdd_t thenBdd, elseBdd;
  Cal_Bdd_t nullBdd = bddManager->bddNull;
  Cal_Bdd_t result;

  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      CalRequestNodeGetF(requestNode, f);
      fIndex = CalBddGetBddIndex(bddManager, f);
      if(fIndex < gIndex){
        /* left cofactor */
        CalBddGetThenBdd(f, calBdd);
        bddId = CalBddGetBddId(calBdd);
        bddIndex = bddManager->idToIndex[bddId];
        if(bddIndex <= hIndex){
          if(bddIndex > gIndex){
            bddId = bddManager->indexToId[gIndex];
          }
          CalHashTableFindOrAdd(reqQueForSwapVars[bddId],
              calBdd, nullBdd, &calBdd);
        }
        CalBddIcrRefCount(calBdd);
        CalRequestNodePutThenRequest(requestNode, calBdd);
        /* right cofactor */
        CalBddGetElseBdd(f, calBdd);
        bddId = CalBddGetBddId(calBdd);
        bddIndex = bddManager->idToIndex[bddId];
        if(bddIndex <= hIndex){
          if(bddIndex > gIndex){
            bddId = bddManager->indexToId[gIndex];
          }
          CalHashTableFindOrAdd(reqQueForSwapVars[bddId],
              calBdd, nullBdd, &calBdd);
        }
        CalBddIcrRefCount(calBdd);
        CalRequestNodePutElseRequest(requestNode, calBdd);
      }
      else if(fIndex == gIndex){
        /* SwapVarsPlus */
        CalBddGetThenBdd(f, thenBdd);
        CalBddGetElseBdd(f, elseBdd);
        if(CalBddGetBddIndex(bddManager, thenBdd) == hIndex){
          CalBddGetThenBdd(thenBdd, thenBdd);
        }
        if(CalBddGetBddIndex(bddManager, elseBdd) == hIndex){
          CalBddGetThenBdd(elseBdd, elseBdd);
        }
        if(CalBddIsEqual(thenBdd, elseBdd)){
          if(hIndex > CalBddGetBddIndex(bddManager, thenBdd)){
            CalHashTableFindOrAdd(reqQueForCompose[CalBddGetBddId(thenBdd)], 
                thenBdd, CalBddOne(bddManager), &result);
          }
          else{
            result = thenBdd;
          }
        }
        else if(hIndex < CalBddGetBddIndex(bddManager, thenBdd) &&
            hIndex < CalBddGetBddIndex(bddManager, elseBdd)){
          bddId = bddManager->indexToId[hIndex];
          if(!CalUniqueTableForIdFindOrAdd(bddManager, 
              bddManager->uniqueTable[bddId], thenBdd, elseBdd, &result)){
            CalBddIcrRefCount(thenBdd);
            CalBddIcrRefCount(elseBdd);
          }
        }
        else{
          CalBddGetMinId2(bddManager, thenBdd, elseBdd, bddId);
          CalHashTableFindOrAdd(reqQueForSwapVarsPlus[bddId],
              thenBdd, elseBdd, &result);
        }
        CalBddIcrRefCount(result);
        CalRequestNodePutThenRequest(requestNode, result);
        /* SwapVarsMinus */
        CalBddGetThenBdd(f, thenBdd);
        CalBddGetElseBdd(f, elseBdd);
        if(CalBddGetBddIndex(bddManager, thenBdd) == hIndex){
          CalBddGetElseBdd(thenBdd, thenBdd);
        }
        if(CalBddGetBddIndex(bddManager, elseBdd) == hIndex){
          CalBddGetElseBdd(elseBdd, elseBdd);
        }
        if(CalBddIsEqual(thenBdd, elseBdd)){
          if(hIndex > CalBddGetBddIndex(bddManager, thenBdd)){
            CalHashTableFindOrAdd(reqQueForCompose[CalBddGetBddId(thenBdd)], 
                thenBdd, CalBddZero(bddManager), &result);
          }
          else{
            result = thenBdd;
          }
        }
        else if(hIndex < CalBddGetBddIndex(bddManager, thenBdd) &&
            hIndex < CalBddGetBddIndex(bddManager, elseBdd)){
          bddId = bddManager->indexToId[hIndex];
          if(!CalUniqueTableForIdFindOrAdd(bddManager, 
              bddManager->uniqueTable[bddId], thenBdd, elseBdd, &result)){
            CalBddIcrRefCount(thenBdd);
            CalBddIcrRefCount(elseBdd);
          }
        }
        else{
          CalBddGetMinId2(bddManager, thenBdd, elseBdd, bddId);
          CalHashTableFindOrAdd(reqQueForSwapVarsMinus[bddId],
              thenBdd, elseBdd, &result);
        }
        CalBddIcrRefCount(result);
        CalRequestNodePutElseRequest(requestNode, result);
      }
      else{ /* fIndex > gIndex */
        CalComposeRequestCreate(bddManager,
            f, CalBddOne(bddManager), hIndex,
            reqQueForCompose, reqQueForITE, &result);
        CalBddIcrRefCount(result);
        CalRequestNodePutThenRequest(requestNode, result);
        CalComposeRequestCreate(bddManager,
            f, CalBddZero(bddManager), hIndex,
            reqQueForCompose, reqQueForITE, &result);
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
static void
CalHashTableSwapVarsPlusApply(
  Cal_BddManager_t * bddManager,
  CalHashTable_t * hashTable,
  Cal_BddIndex_t  hIndex,
  CalHashTable_t ** reqQueForSwapVars,
  CalHashTable_t ** reqQueForSwapVarsPlus,
  CalHashTable_t ** reqQueForSwapVarsMinus,
  CalHashTable_t ** reqQueForCompose)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  CalRequestNode_t *requestNode;
  Cal_BddId_t bddId;
  Cal_Bdd_t f1, f2, g1, g2;
  Cal_Bdd_t result;

  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      CalRequestNodeGetCofactors(bddManager, requestNode, f1, f2, g1, g2);
      /* left cofactor */ 
      if(CalBddGetBddIndex(bddManager, f1) == hIndex){
        CalBddGetThenBdd(f1, f1);
      }
      if(CalBddGetBddIndex(bddManager, g1) == hIndex){
        CalBddGetThenBdd(g1, g1);
      }
      if(CalBddIsEqual(f1, g1)){
        if(hIndex > CalBddGetBddIndex(bddManager, f1)){
          CalHashTableFindOrAdd(reqQueForCompose[CalBddGetBddId(f1)], 
              f1, CalBddOne(bddManager), &result);
        }
        else{
          result = f1;
        }
      }
      else if(hIndex < CalBddGetBddIndex(bddManager, f1) &&
          hIndex < CalBddGetBddIndex(bddManager, g1)){
        bddId = bddManager->indexToId[hIndex];
        if(!CalUniqueTableForIdFindOrAdd(bddManager, 
            bddManager->uniqueTable[bddId], f1, g1, &result)){
          CalBddIcrRefCount(f1);
          CalBddIcrRefCount(g1);
        }
      }
      else{
        CalBddGetMinId2(bddManager, f1, g1, bddId);
        CalHashTableFindOrAdd(reqQueForSwapVarsPlus[bddId], f1, g1, &result);
      }
      CalBddIcrRefCount(result);
      CalRequestNodePutThenRequest(requestNode, result);
      /* right cofactor */
      if(CalBddGetBddIndex(bddManager, f2) == hIndex){
        CalBddGetThenBdd(f2, f2);
      }
      if(CalBddGetBddIndex(bddManager, g2) == hIndex){
        CalBddGetThenBdd(g2, g2);
      }
      if(CalBddIsEqual(f2, g2)){
        if(hIndex > CalBddGetBddIndex(bddManager, f2)){
          CalHashTableFindOrAdd(reqQueForCompose[CalBddGetBddId(f2)], 
              f2, CalBddOne(bddManager), &result);
        }
        else{
          result = f2;
        }
      }
      else if(hIndex < CalBddGetBddIndex(bddManager, f2) &&
          hIndex < CalBddGetBddIndex(bddManager, g2)){
        bddId = bddManager->indexToId[hIndex];
        if(!CalUniqueTableForIdFindOrAdd(bddManager, 
            bddManager->uniqueTable[bddId], f2, g2, &result)){
          CalBddIcrRefCount(f2);
          CalBddIcrRefCount(g2);
        }
      }
      else{
        CalBddGetMinId2(bddManager, f2, g2, bddId);
        CalHashTableFindOrAdd(reqQueForSwapVarsPlus[bddId], f2, g2, &result);
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
CalHashTableSwapVarsMinusApply(
  Cal_BddManager_t * bddManager,
  CalHashTable_t * hashTable,
  Cal_BddIndex_t  hIndex,
  CalHashTable_t ** reqQueForSwapVars,
  CalHashTable_t ** reqQueForSwapVarsPlus,
  CalHashTable_t ** reqQueForSwapVarsMinus,
  CalHashTable_t ** reqQueForCompose)
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  CalRequestNode_t *requestNode;
  Cal_BddId_t bddId;
  Cal_Bdd_t f1, f2, g1, g2;
  Cal_Bdd_t result;

  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      CalRequestNodeGetCofactors(bddManager, requestNode, f1, f2, g1, g2);
      /* left cofactor */ 
      if(CalBddGetBddIndex(bddManager, f1) == hIndex){
        CalBddGetElseBdd(f1, f1);
      }
      if(CalBddGetBddIndex(bddManager, g1) == hIndex){
        CalBddGetElseBdd(g1, g1);
      }
      if(CalBddIsEqual(f1, g1)){
        if(hIndex > CalBddGetBddIndex(bddManager, f1)){
          CalHashTableFindOrAdd(reqQueForCompose[CalBddGetBddId(f1)], 
              f1, CalBddZero(bddManager), &result);
        }
        else{
          result = f1;
        }
      }
      else if(hIndex < CalBddGetBddIndex(bddManager, f1) &&
          hIndex < CalBddGetBddIndex(bddManager, g1)){
        bddId = bddManager->indexToId[hIndex];
        if(!CalUniqueTableForIdFindOrAdd(bddManager,
            bddManager->uniqueTable[bddId], f1, g1, &result)){
          CalBddIcrRefCount(f1);
          CalBddIcrRefCount(g1);
        }
      }
      else{
        CalBddGetMinId2(bddManager, f1, g1, bddId);
        CalHashTableFindOrAdd(reqQueForSwapVarsMinus[bddId], f1, g1, &result);
      }
      CalBddIcrRefCount(result);
      CalRequestNodePutThenRequest(requestNode, result);
      /* right cofactor */
      if(CalBddGetBddIndex(bddManager, f2) == hIndex){
        CalBddGetElseBdd(f2, f2);
      }
      if(CalBddGetBddIndex(bddManager, g2) == hIndex){
        CalBddGetElseBdd(g2, g2);
      }
      if(CalBddIsEqual(f2, g2)){
        if(hIndex > CalBddGetBddIndex(bddManager, f2)){
          CalHashTableFindOrAdd(reqQueForCompose[CalBddGetBddId(f2)], 
              f2, CalBddZero(bddManager), &result);
        }
        else{
          result = f2;
        }
      }
      else if(hIndex < CalBddGetBddIndex(bddManager, f2) &&
          hIndex < CalBddGetBddIndex(bddManager, g2)){
        bddId = bddManager->indexToId[hIndex];
        if(!CalUniqueTableForIdFindOrAdd(bddManager, 
            bddManager->uniqueTable[bddId], f2, g2, &result)){
          CalBddIcrRefCount(f2);
          CalBddIcrRefCount(g2);
        }
      }
      else{
        CalBddGetMinId2(bddManager, f2, g2, bddId);
        CalHashTableFindOrAdd(reqQueForSwapVarsMinus[bddId], f2, g2, &result);
      }
      CalBddIcrRefCount(result);
      CalRequestNodePutElseRequest(requestNode, result);
    }
  }
}














