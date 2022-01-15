/**CFile***********************************************************************

  FileName    [calQuant.c]

  PackageName [cal]

  Synopsis    [Routines for existential/universal quantification and
  relational product.]

  Description []

  SeeAlso     [None]

  Author      [Rajeev K. Ranjan (rajeev@eecs.berkeley.edu) and
               Jagesh V. Sanghavi (sanghavi@eecs.berkeley.edu)]

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

  Revision    [$Id: calQuant.c,v 1.1.1.4 1998/05/04 00:59:02 hsv Exp $]

******************************************************************************/

#include "calInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define DEFAULT_EXIST_HASH_TABLE_SIZE_INDEX 4
#define DEFAULT_EXIST_HASH_TABLE_SIZE 16

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

static Cal_Bdd_t BddExistsStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, unsigned short opCode, CalAssociation_t *association);
static Cal_Bdd_t BddRelProdStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_Bdd_t g, unsigned short opCode, CalAssociation_t *assoc);
static Cal_Bdd_t BddDFStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_Bdd_t g, CalOpProc_t calOpProc, unsigned short opCode);
static void HashTableApply(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, CalHashTable_t ** reqQueAtPipeDepth, CalOpProc_t calOpProc, unsigned long opCode);
static void HashTableReduce(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, CalHashTable_t * uniqueTableForId);
static void BddExistsApply(Cal_BddManager_t *bddManager, int quantifying, CalHashTable_t *existHashTable, CalHashTable_t **existHashTableArray, CalOpProc1_t calOpProc, unsigned short opCode, CalAssociation_t *assoc);
static void BddExistsBFAux(Cal_BddManager_t *bddManager, int minIndex, CalHashTable_t **existHashTableArray, CalHashTable_t **orHashTableArray, CalOpProc1_t calOpProc, unsigned short opCode, CalAssociation_t *assoc);
static void BddExistsReduce(Cal_BddManager_t *bddManager, CalHashTable_t *existHashTable, CalHashTable_t **existHashTableArray, CalHashTable_t **orHashTableArray, unsigned short opCode, CalAssociation_t *association);
static Cal_Bdd_t BddExistsBFPlusDF(Cal_BddManager_t *bddManager, Cal_Bdd_t f, unsigned short opCode, CalAssociation_t *association);
static void BddRelProdApply(Cal_BddManager_t *bddManager, int quantifying, CalHashTable_t *relProdHashTable, CalHashTable_t **relProdHashTableArray, CalHashTable_t **andHashTableArray, CalOpProc_t calOpProc, unsigned short opCode, CalAssociation_t *assoc);
static void BddRelProdReduce(Cal_BddManager_t *bddManager, CalHashTable_t *relProdHashTable, CalHashTable_t **relProdHashTableArray, CalHashTable_t **andHashTableArray, CalHashTable_t **orHashTableArray, unsigned short opCode, CalAssociation_t *assoc);
static void BddRelProdBFAux(Cal_BddManager_t *bddManager, int minIndex, CalHashTable_t **relProdHashTableArray, CalHashTable_t **andHashTableArray, CalHashTable_t **orHashTableArray, unsigned short opCode, CalAssociation_t *assoc);
static Cal_Bdd_t BddRelProdBFPlusDF(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_Bdd_t g, unsigned short opCode, CalAssociation_t *association);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Returns the result of existentially quantifying some
  variables from the given BDD.]

  Description [Returns the BDD for f with all the variables that are
  paired with something in the current variable association
  existentially quantified out.]

  SideEffects [None.]

  SeeAlso     [Cal_BddRelProd]

******************************************************************************/
Cal_Bdd
Cal_BddExists(Cal_BddManager bddManager, Cal_Bdd fUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd userResult;

  if (CalBddPreProcessing(bddManager, 1, fUserBdd)){
    Cal_Bdd_t f = CalBddGetInternalBdd(bddManager, fUserBdd);
    CalAssociation_t *assoc = bddManager->currentAssociation;
    unsigned short opCode;
    
    if (assoc->id == -1){
      opCode = bddManager->tempOpCode--;
    }
    else {
      opCode = CAL_OP_QUANT + assoc->id;
    }
    if (bddManager->numNodes <= CAL_LARGE_BDD){
      /* If number of nodes is small, call depth first routine. */
      result = BddExistsStep(bddManager, f, opCode, assoc);
    }
    else {
      result = BddExistsBFPlusDF(bddManager, f, opCode, assoc);
    }
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


/**Function********************************************************************

  Synopsis    [Returns the result of taking the logical AND of the
  argument BDDs and existentially quantifying some variables from the
  product.] 

  Description [Returns the BDD for the logical AND of f and g with all
  the variables that are paired with something in the current variable
  association existentially quantified out.]

  SideEffects [None.]

******************************************************************************/
Cal_Bdd
Cal_BddRelProd(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd userResult;
  
  if (CalBddPreProcessing(bddManager, 2, fUserBdd, gUserBdd)){
    Cal_Bdd_t f = CalBddGetInternalBdd(bddManager, fUserBdd);
    Cal_Bdd_t g = CalBddGetInternalBdd(bddManager, gUserBdd);
    CalAssociation_t *assoc = bddManager->currentAssociation;
    unsigned short opCode;
    
    if (bddManager->currentAssociation->id == -1){
      opCode = bddManager->tempOpCode--;
      bddManager->tempOpCode--;
    }
    else {
      opCode = CAL_OP_REL_PROD + assoc->id;
    }
    if (bddManager->numNodes <= CAL_LARGE_BDD){
      /* If number of nodes is small, call depth first routine. */
      result = BddRelProdStep(bddManager, f, g, opCode, assoc);
    }
    else {
      result = BddRelProdBFPlusDF(bddManager, f, g, opCode, assoc);
    }
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

/**Function********************************************************************

  Synopsis    [Returns the result of universally quantifying some
  variables from the given BDD.]

  Description [Returns the BDD for f with all the variables that are
  paired with something in the current variable association
  universally quantified out.]

  SideEffects [None.]

******************************************************************************/
Cal_Bdd
Cal_BddForAll(Cal_BddManager bddManager, Cal_Bdd fUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd userResult;

  if (CalBddPreProcessing(bddManager, 1, fUserBdd)){
    Cal_Bdd_t f = CalBddGetInternalBdd(bddManager, fUserBdd);
    CalAssociation_t *assoc = bddManager->currentAssociation;
    unsigned short opCode;

    CalBddNot(f, f);
    if (assoc->id == -1){
      opCode = bddManager->tempOpCode--;
    }
    else {
      opCode = CAL_OP_QUANT + assoc->id;
    }
    if (bddManager->numNodes <= CAL_LARGE_BDD){
      /* If number of nodes is small, call depth first routine. */
      result = BddExistsStep(bddManager, f, opCode, assoc);
    }
    else {
      result = BddExistsBFPlusDF(bddManager, f, opCode, assoc);
    }
    CalBddNot(result, result);
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
  
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalOpExists(Cal_BddManager_t * bddManager, Cal_Bdd_t  f, Cal_Bdd_t *
            resultBddPtr) 
{
  if (((int)bddManager->idToIndex[CalBddGetBddId(f)]) >
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
int
CalOpRelProd(Cal_BddManager_t * bddManager, Cal_Bdd_t  f, Cal_Bdd_t  g,
             Cal_Bdd_t * resultBddPtr) 
{
  if (CalBddIsBddZero(bddManager, f) || CalBddIsBddZero(bddManager, g) ||
      CalBddIsComplementEqual(f, g)){
    *resultBddPtr = bddManager->bddZero;
    return 1;
  }
  else if (CalBddIsBddOne(bddManager, f) && CalBddIsBddOne(bddManager, g)){
    *resultBddPtr = bddManager->bddOne;
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
BddExistsStep(Cal_BddManager_t * bddManager, Cal_Bdd_t  f, unsigned
              short opCode, CalAssociation_t *association)
{
  Cal_Bdd_t temp1, temp2;
  Cal_Bdd_t f1, f2;
  Cal_Bdd_t result;
  Cal_BddId_t topId;
  int quantifying;
  
  if (((int)CalBddGetBddIndex(bddManager, f)) > association->lastBddIndex){
    return f;
  }
  if (CalCacheTableOneLookup(bddManager, f, opCode, &result)){
    return result;
  }

  topId = CalBddGetBddId(f);
  quantifying = (CalBddIsBddNull(bddManager,
                                 association->varAssociation[topId]) ? 0 : 1);
  CalBddGetCofactors(f, topId, f1, f2);
  temp1 = BddExistsStep(bddManager, f1, opCode, association);
  if (quantifying && CalBddIsEqual(temp1, bddManager->bddOne)){
    result=temp1;
  }
  else {
    temp2 = BddExistsStep(bddManager, f2, opCode, association);
    if (quantifying){
      CalBddNot(temp1, temp1);
      CalBddNot(temp2, temp2);
	  result = BddDFStep(bddManager, temp1, temp2, CalOpNand, CAL_OP_NAND);
	}
    else {
      Cal_BddId_t id = CalBddGetBddId(f);
      if (CalUniqueTableForIdFindOrAdd(bddManager, bddManager->uniqueTable[id],
                                       temp1, temp2, &result) == 0){
        CalBddIcrRefCount(temp1);
        CalBddIcrRefCount(temp2);
      }
    }
  } 
  CalCacheTableOneInsert(bddManager, f, result, opCode, 0);
  return (result);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static Cal_Bdd_t
BddRelProdStep(Cal_BddManager_t * bddManager, Cal_Bdd_t  f, Cal_Bdd_t
               g, unsigned short opCode, CalAssociation_t *assoc)
{
  Cal_BddId_t topId;
  Cal_Bdd_t f1, f2, g1, g2;
  Cal_Bdd_t temp1, temp2;
  Cal_Bdd_t  result;
  int quantifying;

  if (CalBddIsBddConst(f) || CalBddIsBddConst(g)){
    if (CalBddIsBddZero(bddManager, f) || CalBddIsBddZero(bddManager, g)){
      return bddManager->bddZero;
    }
    if (assoc->id != -1){
      opCode = CAL_OP_QUANT+assoc->id;
    }
    else{
      opCode--;
    }
    if (CalBddIsBddOne(bddManager, f)){
      return (BddExistsStep(bddManager, g, opCode, assoc));
    }
    return (BddExistsStep(bddManager, f, opCode, assoc));
  }
  if ((((int)CalBddGetBddIndex(bddManager, f)) > assoc->lastBddIndex) &&
      (((int)CalBddGetBddIndex(bddManager, g)) > assoc->lastBddIndex)){
    result = BddDFStep(bddManager, f, g, CalOpNand, CAL_OP_NAND);
    CalBddNot(result, result);
    return result;
  }
  if(CalOpRelProd(bddManager, f, g, &result) == 1){
    return result;
  }
  CalBddNormalize(f, g);
  if(CalCacheTableTwoLookup(bddManager, f, g, opCode, &result)){
    return result;
  }
  CalBddGetMinId2(bddManager, f, g, topId);
  
  quantifying = (CalBddIsBddNull(bddManager, assoc->varAssociation[topId]) ? 0
                 : 1);
  CalBddGetCofactors(f, topId, f1, f2);
  CalBddGetCofactors(g, topId, g1, g2);

  temp1 = BddRelProdStep(bddManager, f1, g1, opCode, assoc);
  if (quantifying && CalBddIsBddOne(bddManager, temp1)){
    result=temp1;
  }
  else {
    temp2 = BddRelProdStep(bddManager, f2, g2, opCode, assoc);
    if (quantifying) {
      CalBddNot(temp1, temp1);
      CalBddNot(temp2, temp2);
	  result = BddDFStep(bddManager, temp1, temp2, CalOpNand, CAL_OP_NAND);
	  /*result = BddDFStep(bddManager, temp1, temp2, CalOpOr, CAL_OP_OR);*/
	}
    else {
      if (CalUniqueTableForIdFindOrAdd(bddManager,
                                       bddManager->uniqueTable[topId],
                                       temp1, temp2, &result) == 0){
        CalBddIcrRefCount(temp1);
        CalBddIcrRefCount(temp2);
      }
    }
  }
  CalCacheTableTwoInsert(bddManager, f, g, result, opCode, 0);
  return (result);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static Cal_Bdd_t
BddDFStep(Cal_BddManager_t * bddManager, Cal_Bdd_t  f, Cal_Bdd_t g,
          CalOpProc_t calOpProc, unsigned short opCode)
{
  Cal_BddId_t topId;
  Cal_Bdd_t temp1, temp2, fx, fxbar, gx, gxbar;
  Cal_Bdd_t  result;

  if((*calOpProc)(bddManager, f, g, &result) == 1){
    return result;
  }
  CalBddNormalize(f, g);
  if(CalCacheTableTwoLookup(bddManager, f, g, opCode, &result)){
    return result;
  }
  CalBddGetMinId2(bddManager, f, g, topId);
  CalBddGetCofactors(f, topId, fx, fxbar);
  CalBddGetCofactors(g, topId, gx, gxbar);
  temp1 = BddDFStep(bddManager, fx, gx, calOpProc, opCode);
  temp2 = BddDFStep(bddManager, fxbar, gxbar, calOpProc, opCode);

  if (CalUniqueTableForIdFindOrAdd(bddManager,
                                   bddManager->uniqueTable[topId],
                                   temp1, temp2, &result) == 0){
    CalBddIcrRefCount(temp1);
    CalBddIcrRefCount(temp2);
  }
  CalCacheTableTwoInsert(bddManager, f, g, result, opCode, 0);
  return (result);
}
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
HashTableApply(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable,
               CalHashTable_t ** reqQueAtPipeDepth, CalOpProc_t calOpProc,
               unsigned long opCode)  
{
  int i, numBins = hashTable->numBins;
  CalBddNode_t **bins = hashTable->bins;
  CalRequestNode_t *requestNode;
  Cal_Bdd_t fx, gx, fxbar, gxbar, result;
  Cal_BddId_t bddId;
  
  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      CalRequestNodeGetCofactors(bddManager, requestNode, fx, fxbar, gx, gxbar);
      CalBddNormalize(fx, gx);
      if((*calOpProc)(bddManager, fx, gx, &result) == 0){
        if (CalCacheTableTwoLookup(bddManager, fx, gx, opCode, &result) == 0){
          CalBddGetMinId2(bddManager, fx, gx, bddId);
          CalHashTableFindOrAdd(reqQueAtPipeDepth[bddId], fx, gx, &result);
          CalCacheTableTwoInsert(bddManager, fx, gx, result, opCode, 1);
        }
        else {
          CalRequestIsForwardedTo(result);
        }
      }
      CalBddIcrRefCount(result);
      CalRequestNodePutThenRequest(requestNode, result);
      CalBddNormalize(fxbar, gxbar);
      if((*calOpProc)(bddManager, fxbar, gxbar, &result) == 0){
        if (CalCacheTableTwoLookup(bddManager, fxbar, gxbar, opCode, &result)
            == 0){ 
          CalBddGetMinId2(bddManager, fxbar, gxbar, bddId);
          CalHashTableFindOrAdd(reqQueAtPipeDepth[bddId], fxbar, gxbar,
                                &result); 
          CalCacheTableTwoInsert(bddManager, fxbar, gxbar, result,
                                 opCode, 1);
        }
        else {
          CalRequestIsForwardedTo(result);
        }
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
HashTableReduce(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable,
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

  /*requestNodeList = hashTable->requestNodeList;*/
  endNode = hashTable->endNode;
  hashTable->numEntries = 0;
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
        /*
        ** CalRequestNodePutNextRequestNode(requestNode, requestNodeList);
        ** requestNodeList = requestNode;
        */
        /*CalRequestNodePutNextRequestNode(endNode, requestNode);*/
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
        /*
        ** CalRequestNodePutNextRequestNode(requestNode, requestNodeList);
        ** requestNodeList = requestNode;
        */
        /*CalRequestNodePutNextRequestNode(endNode, requestNode);*/
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
        /*
        CalNodeManagerInitBddNode(nodeManager, thenBdd, elseBdd,
                               Cal_Nil(CalBddNode_t), bddNode); 
                               */
        CalBddNodeGetRefCount(requestNode, refCount);
        CalBddNodePutRefCount(bddNode, refCount);
        CalHashTableAddDirect(uniqueTableForId, bddNode);
        bddManager->numNodes++;
        bddManager->gcCheck--;
        CalRequestNodePutThenRequestId(requestNode, currentBddId);
        CalRequestNodePutThenRequestNode(requestNode, CalBddNodeNot(bddNode));
        CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
        /*
        ** CalRequestNodePutNextRequestNode(requestNode, requestNodeList);
        ** requestNodeList = requestNode;
        */
        /*CalRequestNodePutNextRequestNode(endNode, requestNode);*/
        endNode->nextBddNode = requestNode;
        endNode = requestNode;
      }
    }
  }
  /* hashTable->requestNodeList = requestNodeList; */
  hashTable->endNode = endNode;
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddExistsApply(Cal_BddManager_t *bddManager, int quantifying,
               CalHashTable_t *existHashTable, CalHashTable_t
               **existHashTableArray,  CalOpProc1_t calOpProc, 
               unsigned short opCode, CalAssociation_t *assoc)  
{
  int i, numBins = existHashTable->numBins;
  CalBddNode_t **bins = existHashTable->bins;
  CalRequestNode_t *requestNode;
  Cal_Bdd_t f, fx, fxbar, result, resultBar;
  int lastBddIndex = assoc->lastBddIndex; 
  
  if (quantifying){
    for(i = 0; i < numBins; i++){
      for(requestNode = bins[i];
          requestNode != Cal_Nil(CalRequestNode_t);
          requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
        CalRequestNodeGetF(requestNode, f);
        CalBddGetThenBdd(f, fx);
        CalBddGetElseBdd(f, fxbar);
      
        /*if(calOpProc(bddManager, fx, &result) == 0){*/
        if (((int)bddManager->idToIndex[CalBddGetBddId(fx)]) <= lastBddIndex){
          if (CalCacheTableOneLookup(bddManager, fx, opCode, &result)){ 
            CalRequestIsForwardedTo(result);
          }
          else {
            CalHashTableFindOrAdd(existHashTableArray[CalBddGetBddId(fx)], fx,
                                  bddManager->bddOne, &result);
            CalCacheTableOneInsert(bddManager, fx, result,
                                   opCode, 1);
          }
        }
        else {
          result = fx;
        }
        CalRequestNodePutThenRequest(requestNode, result);
        CalRequestNodePutElseRequest(requestNode, fxbar);
      }
    }
  }
  else {
    for(i = 0; i < numBins; i++){
      for(requestNode = bins[i];
          requestNode != Cal_Nil(CalRequestNode_t);
          requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
        CalRequestNodeGetF(requestNode, f);
        CalBddGetThenBdd(f, fx);
        CalBddGetElseBdd(f, fxbar);
      
        if (((int)bddManager->idToIndex[CalBddGetBddId(fx)]) <= lastBddIndex){
          if (CalCacheTableOneLookup(bddManager, fx, opCode, &result)){ 
            CalRequestIsForwardedTo(result);
          }
          else {
            CalHashTableFindOrAdd(existHashTableArray[CalBddGetBddId(fx)], fx,
                                  bddManager->bddOne, &result);
            CalCacheTableOneInsert(bddManager, fx, result,
                                   opCode, 1);
          }
        }
        else {
          result = fx;
        }
        CalRequestNodePutThenRequest(requestNode, result);
        CalBddIcrRefCount(result);
        /*if(calOpProc(bddManager, fxbar, &resultBar) == 0){*/
        if (((int)bddManager->idToIndex[CalBddGetBddId(fxbar)]) <= lastBddIndex){
          if (CalCacheTableOneLookup(bddManager, fxbar, opCode,
                                       &resultBar)){
            CalRequestIsForwardedTo(resultBar);
          }
          else {
            CalHashTableFindOrAdd(existHashTableArray[CalBddGetBddId(fxbar)], fxbar,
                                  bddManager->bddOne, &resultBar);
            CalCacheTableOneInsert(bddManager, fxbar, resultBar,
                                   opCode, 1); 
          }
        }
        else{
          resultBar = fxbar;
        }
        CalBddIcrRefCount(resultBar);
        CalRequestNodePutElseRequest(requestNode, resultBar);
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
BddExistsBFAux(Cal_BddManager_t *bddManager, int minIndex,
               CalHashTable_t **existHashTableArray, CalHashTable_t
               **orHashTableArray,  CalOpProc1_t calOpProc, unsigned
               short opCode, CalAssociation_t *assoc)   
{
  int index;
  Cal_BddId_t bddId;
  int quantifying;
  
  /* Apply phase */
  for (index = minIndex; index < bddManager->numVars; index++){
    bddId = bddManager->indexToId[index];
    if (existHashTableArray[bddId]->numEntries){
      quantifying = (CalBddIsBddNull(bddManager,
                                     assoc->varAssociation[bddId]) ? 0 : 1); 
      BddExistsApply(bddManager, quantifying,
                     existHashTableArray[bddId], existHashTableArray,
                     calOpProc, opCode, assoc);    
    }
  }
  
  /* Reduce phase */
  for (index = bddManager->numVars-1; index >= minIndex; index--){
    bddId = bddManager->indexToId[index];
    if (existHashTableArray[bddId]->numEntries){
      quantifying = (CalBddIsBddNull(bddManager,
                                     assoc->varAssociation[bddId]) ? 0 : 1); 
      if (quantifying){
        BddExistsReduce(bddManager, existHashTableArray[bddId],
                        existHashTableArray, orHashTableArray,
                        opCode, assoc);
      } 
      else {
        HashTableReduce(bddManager, existHashTableArray[bddId],
                        bddManager->uniqueTable[bddId]);
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
BddExistsReduce(Cal_BddManager_t *bddManager, CalHashTable_t
                *existHashTable, CalHashTable_t **existHashTableArray,
                CalHashTable_t **orHashTableArray, unsigned short
                opCode, CalAssociation_t *association) 
{
  int i, numBins = existHashTable->numBins;
  CalBddNode_t **bins = existHashTable->bins;
  CalRequestNode_t *requestNode, *next, *requestNodeListAux;
  CalBddNode_t *endNode;
  
  int bddIndex;
  /*Cal_BddIndex_t minIndex, elseIndex;*/
  int minIndex, elseIndex;
  Cal_BddId_t bddId, minId;
  Cal_Bdd_t thenBdd, elseBdd, result, orResult;
  Cal_BddRefCount_t refCount;
  int lastBddIndex = association->lastBddIndex; 
  

  /* For those nodes which get processed in the first pass */
  /* requestNodeList = existHashTable->requestNodeList; */
  endNode = existHashTable->endNode;

  /* For the other ones. This list is merged with the requestNodeList
   * after processing is complete.
   */
  requestNodeListAux = Cal_Nil(CalRequestNode_t);
  existHashTable->numEntries = 0;
  
  minIndex = bddManager->numVars;
  
  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i], bins[i] = Cal_Nil(CalRequestNode_t);
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = next){
      next = CalRequestNodeGetNextRequestNode(requestNode);
      /* Process the requestNode */
      CalRequestNodeGetThenRequest(requestNode, thenBdd);
      CalRequestNodeGetElseRequest(requestNode, elseBdd);
      CalRequestIsForwardedTo(thenBdd);
      CalRequestNodePutThenRequest(requestNode, thenBdd);
      if (CalBddIsBddOne(bddManager, thenBdd)){
        CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
        /*
        ** CalRequestNodePutNextRequestNode(requestNode, requestNodeList);
        ** requestNodeList = requestNode;
        */
        /*CalRequestNodePutNextRequestNode(endNode, requestNode);*/
        endNode->nextBddNode = requestNode;
        endNode = requestNode;
        continue;
      }
      
      CalRequestNodePutNextRequestNode(requestNode, requestNodeListAux);
      requestNodeListAux = requestNode;

      /*if(CalOpExists(bddManager, elseBdd, &result) == 0){*/
      if (((int)bddManager->idToIndex[CalBddGetBddId(elseBdd)]) <= lastBddIndex){
        if (CalCacheTableOneLookup(bddManager, elseBdd, opCode,
                                   &result)){  
          CalRequestIsForwardedTo(result);
        }
        else{
          CalHashTableFindOrAdd(existHashTableArray[CalBddGetBddId(elseBdd)], elseBdd,
                                bddManager->bddOne, &result);
          CalCacheTableOneInsert(bddManager, elseBdd, result,
                                 opCode, 1);
          if (minIndex > (elseIndex = CalBddGetBddIndex(bddManager,
                                                        elseBdd))){ 
            minIndex = elseIndex;
          }
        }
      }
      else{
        result = elseBdd;
      }
      CalRequestNodePutElseRequest(requestNode, result);
    }
  }
  
  if (!requestNodeListAux){
    /* requestNodeList = requestNodeList; */
    existHashTable->endNode = endNode;
    return;
  }
  
  BddExistsBFAux(bddManager, minIndex, existHashTableArray,
                 orHashTableArray,  CalOpExists, opCode, association); 
  minIndex = bddManager->numVars;
  for (requestNode = requestNodeListAux; requestNode; requestNode = next){
    Cal_Bdd_t thenResult, elseResult;
    Cal_BddIndex_t orResultIndex;
    
    next = CalRequestNodeGetNextRequestNode(requestNode);
    CalRequestNodeGetThenRequest(requestNode, thenResult);
    CalRequestNodeGetElseRequest(requestNode, elseResult);
    CalRequestIsForwardedTo(elseResult);
    if (CalOpOr(bddManager, thenResult, elseResult, &orResult) == 0){
      CalBddNormalize(thenResult, elseResult);
      CalBddNot(thenResult, thenResult);
      CalBddNot(elseResult, elseResult);
      if (CalCacheTableTwoLookup(bddManager, thenResult,elseResult,
                                 CAL_OP_NAND, &orResult)){
        CalRequestIsForwardedTo(orResult);
      }
      else {
        CalBddGetMinIdAndMinIndex(bddManager, thenResult, elseResult,
                                  minId, orResultIndex);
        CalHashTableFindOrAdd(orHashTableArray[minId], thenResult, elseResult,
                              &orResult);
        CalCacheTableTwoInsert(bddManager, thenResult, elseResult, orResult,
                               CAL_OP_NAND, 1);
        if (minIndex > orResultIndex) minIndex = orResultIndex;
      }
    }
    CalRequestNodePutThenRequest(requestNode, orResult);
  }
  

  /* Call "OR" apply and reduce */
  for (bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    if(orHashTableArray[bddId]->numEntries){
      HashTableApply(bddManager, orHashTableArray[bddId], orHashTableArray,
                     CalOpNand, CAL_OP_NAND);
    }
  }
  
  for(bddIndex = bddManager->numVars - 1; bddIndex >= minIndex; bddIndex--){
    CalHashTable_t *uniqueTableForId;
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    if(orHashTableArray[bddId]->numEntries){
      HashTableReduce(bddManager, orHashTableArray[bddId], uniqueTableForId);
    }
  }
  
  for (requestNode = requestNodeListAux; requestNode; requestNode = next){
    next = CalRequestNodeGetNextRequestNode(requestNode);
    CalRequestNodeGetThenRequest(requestNode, result);
    CalRequestIsForwardedTo(result);
    CalBddNodeGetRefCount(requestNode, refCount);
    CalBddAddRefCount(result, refCount);
    CalRequestNodePutThenRequest(requestNode, result);
    CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
    /*
    ** CalRequestNodePutNextRequestNode(requestNode, requestNodeList);
    ** requestNodeList = requestNode;
    */
    /*CalRequestNodePutNextRequestNode(endNode, requestNode);*/
    endNode->nextBddNode = requestNode;
    endNode = requestNode;
  }
  /*existHashTable->requestNodeList = requestNodeList;*/
  existHashTable->endNode = endNode;
  
}
  
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static Cal_Bdd_t
BddExistsBFPlusDF(Cal_BddManager_t *bddManager, Cal_Bdd_t f, unsigned
                  short opCode, CalAssociation_t *association)
{
  Cal_BddId_t fId = CalBddGetBddId(f);
  Cal_BddIndex_t bddIndex;
  Cal_BddId_t bddId;
  
  Cal_BddIndex_t fIndex = bddManager->idToIndex[fId];
  CalHashTable_t **orHashTableArray = bddManager->reqQue[4];
  CalHashTable_t **existHashTableArray = bddManager->reqQue[5];
  Cal_Bdd_t result;
  
  if (CalOpExists(bddManager, f, &result) == 1){
    return result;
  }

  if (CalCacheTableOneLookup(bddManager, f, opCode, &result)){
    return result;
  }
  
  /*
   * Change the size of the exist hash table to min. size 
   */
  for (bddIndex = fIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    existHashTableArray[bddId]->sizeIndex =
        DEFAULT_EXIST_HASH_TABLE_SIZE_INDEX;  
    existHashTableArray[bddId]->numBins = DEFAULT_EXIST_HASH_TABLE_SIZE;
    Cal_MemFree(existHashTableArray[bddId]->bins);
    existHashTableArray[bddId]->bins = Cal_MemAlloc(CalBddNode_t*,
                                             DEFAULT_EXIST_HASH_TABLE_SIZE);
    memset((char *)existHashTableArray[bddId]->bins, 0,
           existHashTableArray[bddId]->numBins*sizeof(CalBddNode_t*));
  }
  
  CalHashTableFindOrAdd(existHashTableArray[fId], f, bddManager->bddOne,
                        &result);  


  BddExistsBFAux(bddManager, fIndex, existHashTableArray, orHashTableArray,
                 CalOpExists, opCode, association);  

  CalRequestIsForwardedTo(result);
  
  CalCacheTableTwoFixResultPointers(bddManager);
  CalCacheTableOneInsert(bddManager, f, result, opCode, 0);
  for (bddIndex = fIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    CalHashTableCleanUp(existHashTableArray[bddId]);
    CalHashTableCleanUp(orHashTableArray[bddId]);
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
BddRelProdApply(Cal_BddManager_t *bddManager, int quantifying, CalHashTable_t
                *relProdHashTable, CalHashTable_t **relProdHashTableArray,
                CalHashTable_t **andHashTableArray, CalOpProc_t
                calOpProc, unsigned short opCode, CalAssociation_t *assoc)
{
  int i, numBins = relProdHashTable->numBins;
  CalBddNode_t **bins = relProdHashTable->bins;
  Cal_BddId_t minId;
  CalRequestNode_t *requestNode;
  Cal_Bdd_t fx, fxbar, gx, gxbar, result, resultBar;
  /*Cal_BddIndex_t minIndex;*/
  int minIndex;
  
  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i];
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = CalRequestNodeGetNextRequestNode(requestNode)){
      CalRequestNodeGetCofactors(bddManager, requestNode, fx, fxbar, gx, gxbar);
      CalBddNormalize(fx, gx);
      CalBddGetMinIdAndMinIndex(bddManager, fx, gx, minId, minIndex);
      if (minIndex > assoc->lastBddIndex){
        if (CalOpAnd(bddManager, fx, gx, &result) == 0){
          if (CalCacheTableTwoLookup(bddManager, fx, gx, CAL_OP_NAND,
                                     &result)){  
            CalRequestIsForwardedTo(result);
          }
          else{
            CalHashTableFindOrAdd(andHashTableArray[minId], fx, gx, &result);
            CalCacheTableTwoInsert(bddManager, fx, gx, result,
                                   CAL_OP_NAND, 1);
          }
          CalBddNot(result, result);
        }
      }
      else {
        if(calOpProc(bddManager, fx, gx, &result) == 0){
          if (CalCacheTableTwoLookup(bddManager, fx, gx, opCode,
                                     &result)){      
            CalRequestIsForwardedTo(result);
          }
          else {
            CalHashTableFindOrAdd(relProdHashTableArray[minId], fx, gx,
                                  &result);   
            CalCacheTableTwoInsert(bddManager, fx, gx, result, opCode, 1); 
          }
        }
      }
      CalRequestNodePutThenRequest(requestNode, result);
      if (quantifying){
        Cal_Bdd_t elseRequest;
        Cal_BddId_t elseRequestId;
        CalBddNode_t *elseRequestNode;
        
        CalBddGetMinId2(bddManager, fxbar, gxbar, elseRequestId);
        CalNodeManagerInitBddNode(bddManager->nodeManagerArray[elseRequestId],
                                  fxbar, gxbar, Cal_Nil(CalBddNode_t),
                                  elseRequestNode);
        /*
          CalNodeManagerAllocNode(bddManager->nodeManagerArray[elseRequestId],
          elseRequestNode);  
          CalRequestNodePutF(elseRequestNode, fxbar);
          CalRequestNodePutG(elseRequestNode, gxbar);
        */
        CalRequestPutRequestId(elseRequest, elseRequestId);
        CalRequestPutRequestNode(elseRequest, elseRequestNode);
        CalRequestNodePutElseRequest(requestNode, elseRequest);
      }
      else {
        CalBddIcrRefCount(result);
        CalBddNormalize(fxbar, gxbar);
        CalBddGetMinIdAndMinIndex(bddManager, fxbar, gxbar, minId, minIndex);
        if (minIndex > assoc->lastBddIndex){
          if (CalOpAnd(bddManager, fxbar, gxbar, &resultBar) == 0){
            if( CalCacheTableTwoLookup(bddManager, fxbar, gxbar,
                                       CAL_OP_NAND, &resultBar)){  
              CalRequestIsForwardedTo(resultBar);
            }
            else{
              CalHashTableFindOrAdd(andHashTableArray[minId], fxbar, gxbar,
                                    &resultBar); 
              CalCacheTableTwoInsert(bddManager, fxbar, gxbar, resultBar,
                                     CAL_OP_NAND, 1); 
            }
            CalBddNot(resultBar, resultBar);
          }
        }
        else {
          if(calOpProc(bddManager, fxbar, gxbar, &resultBar) == 0){
            if (CalCacheTableTwoLookup(bddManager, fxbar, gxbar, opCode,
                                       &resultBar)){   
              CalRequestIsForwardedTo(resultBar);
            }
            else { 
              CalHashTableFindOrAdd(relProdHashTableArray[minId],
                                    fxbar, gxbar, &resultBar);
              CalCacheTableTwoInsert(bddManager, fxbar, gxbar,
                                     resultBar, opCode, 1); 
            }
          }
        }
        CalBddIcrRefCount(resultBar);
        CalRequestNodePutElseRequest(requestNode, resultBar);
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
BddRelProdReduce(Cal_BddManager_t *bddManager, CalHashTable_t
                 *relProdHashTable, CalHashTable_t
                 **relProdHashTableArray, CalHashTable_t
                 **andHashTableArray, CalHashTable_t
                 **orHashTableArray, unsigned short opCode,
                 CalAssociation_t *assoc)  
{
  int i, numBins = relProdHashTable->numBins;
  CalBddNode_t **bins = relProdHashTable->bins;
  CalRequestNode_t *requestNode, *next, *requestNodeListAux;
  CalBddNode_t  *elseRequestNode;
  int bddIndex;
  /*Cal_BddIndex_t minIndex;*/
  int minIndex;
  Cal_BddId_t bddId, minId, elseRequestId;
  Cal_Bdd_t thenBdd, elseBdd, result, orResult;
  Cal_BddRefCount_t refCount;
  Cal_Bdd_t fxbar, gxbar;
  CalBddNode_t *endNode;
  

  /* For those nodes which get processed in the first pass */
  /*requestNodeList = relProdHashTable->requestNodeList;*/
  endNode = relProdHashTable->endNode;
  
  /* For the other ones. This list is merged with the requestNodeList
   * after processing is complete.
   */
  requestNodeListAux = Cal_Nil(CalRequestNode_t);
  
  minIndex = bddManager->numVars;
  
  for(i = 0; i < numBins; i++){
    for(requestNode = bins[i], bins[i] = Cal_Nil(CalRequestNode_t);
        requestNode != Cal_Nil(CalRequestNode_t);
        requestNode = next){
      next = CalRequestNodeGetNextRequestNode(requestNode);
      /* Process the requestNode */
      CalRequestNodeGetThenRequest(requestNode, thenBdd);
      CalRequestIsForwardedTo(thenBdd);
      /*CalRequestNodePutThenRequest(requestNode, thenBdd);*/
      CalRequestNodeGetElseRequest(requestNode, elseBdd);
      CalRequestIsForwardedTo(elseBdd);
      CalRequestGetF(elseBdd, fxbar);
      CalRequestGetG(elseBdd, gxbar);
      
      /* Free the else request node because it is not needed */
      elseRequestNode = CalRequestNodeGetElseRequestNode(requestNode);
      elseRequestId = CalRequestNodeGetElseRequestId(requestNode);
      CalNodeManagerFreeNode(bddManager->nodeManagerArray[elseRequestId],
                             elseRequestNode);
      if (CalBddIsBddOne(bddManager, thenBdd)){
        CalRequestNodePutThenRequest(requestNode, bddManager->bddOne);
        CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
        /*
        ** CalRequestNodePutNextRequestNode(requestNode, requestNodeList);
        ** requestNodeList = requestNode;
        */
        /*CalRequestNodePutNextRequestNode(endNode, requestNode);*/
        endNode->nextBddNode = requestNode;
        endNode = requestNode;
        continue;
      }
      
      CalRequestNodePutNextRequestNode(requestNode, requestNodeListAux);
      requestNodeListAux = requestNode;

      CalBddGetMinIdAndMinIndex(bddManager, fxbar, gxbar, bddId, bddIndex);
      CalBddNormalize(fxbar, gxbar);
      if (bddIndex > assoc->lastBddIndex){
        if (CalOpAnd(bddManager, fxbar, gxbar, &result) == 0){
          if (CalCacheTableTwoLookup(bddManager, fxbar, gxbar,
                                     CAL_OP_NAND, &result)){
            CalRequestIsForwardedTo(result);
          }
          else {
            CalHashTableFindOrAdd(andHashTableArray[bddId], fxbar,
                                  gxbar, &result);
            CalCacheTableTwoInsert(bddManager, fxbar, gxbar, result,
                                   CAL_OP_NAND, 1);
            if (minIndex > bddIndex) minIndex = bddIndex;
          }
          CalBddNot(result, result);
        }
      }
      else {
        if(CalOpRelProd(bddManager, fxbar, gxbar, &result) == 0){
          if (CalCacheTableTwoLookup(bddManager, fxbar, gxbar, opCode,
                                     &result)){  
            CalRequestIsForwardedTo(result);
          }
          else {
            CalHashTableFindOrAdd(relProdHashTableArray[bddId], fxbar, gxbar, 
                                  &result);
            CalCacheTableTwoInsert(bddManager, fxbar, gxbar, result,
                                   opCode, 1); 
            if (minIndex > bddIndex) minIndex = bddIndex;
          }
        }
      }
      CalRequestNodePutElseRequest(requestNode, result);
    }
  }

  if (!requestNodeListAux){
    /*relProdHashTable->requestNodeList = requestNodeList;*/
    relProdHashTable->endNode = endNode;
    return;
  }
  
  BddRelProdBFAux(bddManager, minIndex, relProdHashTableArray,
                  andHashTableArray, orHashTableArray, opCode, assoc);
  
  minIndex = bddManager->numVars;
  for (requestNode = requestNodeListAux; requestNode; requestNode = next){
    Cal_Bdd_t thenResult, elseResult;
    Cal_BddIndex_t orResultIndex;
    
    next = CalRequestNodeGetNextRequestNode(requestNode);
    CalRequestNodeGetThenRequest(requestNode, thenResult);
    CalRequestNodeGetElseRequest(requestNode, elseResult);
    CalRequestIsForwardedTo(elseResult);
    CalRequestIsForwardedTo(thenResult);
    CalBddNormalize(thenResult, elseResult);
    if (CalOpOr(bddManager, thenResult, elseResult, &orResult) == 0){
      CalBddNot(thenResult, thenResult);
      CalBddNot(elseResult, elseResult);
      if (CalCacheTableTwoLookup(bddManager, thenResult, elseResult,
                                 CAL_OP_NAND, &orResult)){ 
        CalRequestIsForwardedTo(orResult);
      }
      else {
        CalBddGetMinIdAndMinIndex(bddManager, thenResult, elseResult,
                                  minId, orResultIndex);
        CalHashTableFindOrAdd(orHashTableArray[minId], thenResult, elseResult,
                              &orResult);
        CalCacheTableTwoInsert(bddManager, thenResult, elseResult, orResult,
                                 CAL_OP_NAND, 1); 
        if (minIndex > orResultIndex) minIndex = orResultIndex;
      }
    }
    CalRequestNodePutThenRequest(requestNode, orResult);
  }

  /* Call "OR" apply and reduce */
  for (bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    if(orHashTableArray[bddId]->numEntries){
        HashTableApply(bddManager, orHashTableArray[bddId], orHashTableArray,
                       CalOpNand, CAL_OP_NAND); 
    }
  }
  
  for(bddIndex = bddManager->numVars - 1; bddIndex >= minIndex; bddIndex--){
    CalHashTable_t *uniqueTableForId;
    bddId = bddManager->indexToId[bddIndex];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    if(orHashTableArray[bddId]->numEntries){
      HashTableReduce(bddManager, orHashTableArray[bddId], uniqueTableForId);
    }
  }
  for (requestNode = requestNodeListAux; requestNode; requestNode = next){
    next = CalRequestNodeGetNextRequestNode(requestNode);
    CalRequestNodeGetThenRequest(requestNode, result);
    CalRequestIsForwardedTo(result);
    CalBddNodeGetRefCount(requestNode, refCount);
    CalBddAddRefCount(result, refCount);
    CalRequestNodePutThenRequest(requestNode, result);
    CalRequestNodePutElseRequestNode(requestNode, FORWARD_FLAG);
    /*
    ** CalRequestNodePutNextRequestNode(requestNode, requestNodeList);
    ** requestNodeList = requestNode;
    */
    /*CalRequestNodePutNextRequestNode(endNode, requestNode);*/
    endNode->nextBddNode = requestNode;
    endNode = requestNode;
  }

  /*relProdHashTable->requestNodeList = requestNodeList;*/
  relProdHashTable->endNode = endNode;
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddRelProdBFAux(Cal_BddManager_t *bddManager, int minIndex,
                CalHashTable_t **relProdHashTableArray, CalHashTable_t
                **andHashTableArray, CalHashTable_t
                **orHashTableArray, unsigned short opCode,
                CalAssociation_t *assoc)
{
  Cal_BddIndex_t bddIndex;
  int quantifying;
  int index;
  Cal_BddId_t bddId;
  CalHashTable_t *hashTable;
  
  for (bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    hashTable = andHashTableArray[bddId];
    if(hashTable->numEntries){
      HashTableApply(bddManager, hashTable, andHashTableArray, CalOpNand,
                     CAL_OP_NAND); 
    }
    hashTable = relProdHashTableArray[bddId];
    if(hashTable->numEntries){
      quantifying = (CalBddIsBddNull(bddManager,
                                     assoc->varAssociation[bddId]) ? 0 : 1); 
      BddRelProdApply(bddManager, quantifying, hashTable,
                      relProdHashTableArray, andHashTableArray,
                      CalOpRelProd, opCode, assoc); 
    }
  }

  /* Reduce phase */
  for (index = bddManager->numVars-1; index >= minIndex; index--){
    CalHashTable_t *uniqueTableForId;
    bddId = bddManager->indexToId[index];
    uniqueTableForId = bddManager->uniqueTable[bddId];
    hashTable = andHashTableArray[bddId];
    if(hashTable->numEntries){
      HashTableReduce(bddManager, hashTable, uniqueTableForId);
    }
    if (relProdHashTableArray[bddId]->numEntries){
      quantifying = (CalBddIsBddNull(bddManager,
                                     assoc->varAssociation[bddId]) ? 0 : 1); 
      if (quantifying){
        BddRelProdReduce(bddManager, relProdHashTableArray[bddId],
                         relProdHashTableArray, andHashTableArray,
                         orHashTableArray, opCode, assoc); 
      }
      else {
        HashTableReduce(bddManager, relProdHashTableArray[bddId],
                        bddManager->uniqueTable[bddId]);
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
static Cal_Bdd_t
BddRelProdBFPlusDF(Cal_BddManager_t * bddManager, Cal_Bdd_t  f,
                   Cal_Bdd_t  g, unsigned short opCode,
                   CalAssociation_t *association)
{
  Cal_Bdd_t result;
  /*Cal_BddIndex_t minIndex;*/
  int  minIndex;
  int bddIndex;
  CalHashTable_t **andHashTableArray = bddManager->reqQue[3];
  CalHashTable_t **relProdHashTableArray = bddManager->reqQue[4];
  CalHashTable_t **orHashTableArray = bddManager->reqQue[5];
  Cal_BddId_t bddId, minId;

  if(CalOpRelProd(bddManager, f, g, &result) == 1){
    return result;
  }
  CalBddNormalize(f, g);
  if(CalCacheTableTwoLookup(bddManager, f, g, opCode, &result)){
    return result;
  }

  CalBddGetMinIdAndMinIndex(bddManager, f, g, minId, minIndex);

  /*
   * Change the size of the exist hash table to min. size 
   */
  for (bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    relProdHashTableArray[bddId]->sizeIndex =
        DEFAULT_EXIST_HASH_TABLE_SIZE_INDEX;  
    relProdHashTableArray[bddId]->numBins = DEFAULT_EXIST_HASH_TABLE_SIZE;
    Cal_MemFree(relProdHashTableArray[bddId]->bins);
    relProdHashTableArray[bddId]->bins = Cal_MemAlloc(CalBddNode_t*,
                                             DEFAULT_EXIST_HASH_TABLE_SIZE);
    memset((char *)relProdHashTableArray[bddId]->bins, 0,
           relProdHashTableArray[bddId]->numBins*sizeof(CalBddNode_t*));
  }

  if (minIndex > association->lastBddIndex) {
    if (CalOpAnd(bddManager, f, g, &result) == 0){
      if (CalCacheTableTwoLookup(bddManager, f, g, CAL_OP_NAND, &result)
          == 0){
        CalHashTableFindOrAdd(andHashTableArray[minId], f, g, &result);
      }
      else{
        CalCacheTableTwoInsert(bddManager, f, g, result, CAL_OP_NAND,
                               1);
      }
      CalBddNot(result, result);
    }
  }
  else {
    CalHashTableFindOrAdd(relProdHashTableArray[minId], f, g, &result); 
  }

  BddRelProdBFAux(bddManager, minIndex, relProdHashTableArray,
                  andHashTableArray, orHashTableArray, opCode, association); 
  CalRequestIsForwardedTo(result);
  CalCacheTableTwoFixResultPointers(bddManager);
  CalCacheTableTwoInsert(bddManager, f, g, result, opCode, 0);
  for (bddIndex = minIndex; bddIndex < bddManager->numVars; bddIndex++){
    bddId = bddManager->indexToId[bddIndex];
    CalHashTableCleanUp(relProdHashTableArray[bddId]);
    CalHashTableCleanUp(andHashTableArray[bddId]);
    CalHashTableCleanUp(orHashTableArray[bddId]);
  }
  return result;
}

