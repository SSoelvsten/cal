/**CFile***********************************************************************

  FileName    [cal.c]

  PackageName [cal]

  Synopsis    [Miscellaneous collection of exported BDD functions]

  Description []

  SeeAlso     []

  Author      [
               Rajeev K. Ranjan (rajeev@eecs.berkeley.edu) and
               Jagesh V. Sanghavi (sanghavi@eecs.berkeley.edu
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

  Revision    [$Id: cal.c,v 1.1.1.5 1998/05/04 00:58:48 hsv Exp $]

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

static Cal_Bdd_t BddIntersectsStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_Bdd_t g);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Returns 1 if argument BDDs are equal, 0 otherwise.]

  Description [Returns 1 if argument BDDs are equal, 0 otherwise.]

  SideEffects [None.]

  SeeAlso     []

******************************************************************************/
int
Cal_BddIsEqual(Cal_BddManager bddManager, Cal_Bdd userBdd1, Cal_Bdd userBdd2)
{
  return (userBdd1 == userBdd2);
}

/**Function********************************************************************

  Synopsis    [Returns 1 if the argument BDD is constant one, 0 otherwise.]

  Description [Returns 1 if the argument BDD is constant one, 0 otherwise.]

  SideEffects [None.]

  SeeAlso     [Cal_BddIsBddZero]

******************************************************************************/
int
Cal_BddIsBddOne(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  return (userBdd == bddManager->userOneBdd);
}

/**Function********************************************************************

  Synopsis    [Returns 1 if the argument BDD is constant zero, 0 otherwise.]

  Description [Returns 1 if the argument BDD is constant zero, 0 otherwise.]

  SideEffects [None.]

  SeeAlso     [Cal_BddIsBddOne]

******************************************************************************/
int
Cal_BddIsBddZero(
  Cal_BddManager bddManager,
  Cal_Bdd userBdd)
{
  return (userBdd == bddManager->userZeroBdd);
}

/**Function********************************************************************

  Synopsis    [Returns 1 if the argument BDD is NULL, 0 otherwise.]

  Description [Returns 1 if the argument BDD is NULL, 0 otherwise.]

  SideEffects [None.]

******************************************************************************/
int
Cal_BddIsBddNull(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  return (userBdd == 0);
}

/**Function********************************************************************

  Synopsis    [Returns 1 if the argument BDD is a constant, 0 otherwise.]

  Description [Returns 1 if the argument BDD is either constant one or
  constant zero, otherwise returns 0.]

  SideEffects [None.]

  SeeAlso     [Cal_BddIsBddOne, Cal_BddIsBddZero]

******************************************************************************/
int
Cal_BddIsBddConst(
  Cal_BddManager bddManager,
  Cal_Bdd userBdd)
{
  return ((userBdd == bddManager->userOneBdd) ||
          (userBdd == bddManager->userZeroBdd));
}

/**Function********************************************************************

  Synopsis    [Returns the duplicate BDD of the argument BDD.]

  Description [Returns the duplicate BDD of the argument BDD.]

  SideEffects [The reference count of the BDD is increased by 1.]

  SeeAlso     [Cal_BddNot]

******************************************************************************/
Cal_Bdd
Cal_BddIdentity(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  /* Interface BDD reference count */
  CalBddNode_t *bddNode = CAL_BDD_POINTER(userBdd);
  CalBddNodeIcrRefCount(bddNode);
  return userBdd;
}

/**Function********************************************************************

  Synopsis    [Returns the BDD for the constant one]

  Description [Returns the BDD for the constant one]

  SideEffects [None]

  SeeAlso     [Cal_BddZero]

******************************************************************************/
Cal_Bdd
Cal_BddOne(Cal_BddManager bddManager)
{
  return bddManager->userOneBdd;
}

/**Function********************************************************************

  Synopsis    [Returns the BDD for the constant zero]

  Description [Returns the BDD for the constant zero]

  SideEffects [None]

  SeeAlso     [Cal_BddOne]

******************************************************************************/
Cal_Bdd
Cal_BddZero(Cal_BddManager bddManager)
{
  return bddManager->userZeroBdd;
}

/**Function********************************************************************

   Synopsis    [Returns the NULL BDD.]

   Description [Returns the NULL BDD.]

   SideEffects [None.]

******************************************************************************/
Cal_Bdd
Cal_BddNull(Cal_BddManager bddManager)
{
  return (Cal_Bdd) 0;
}

/**Function********************************************************************

  Synopsis    [Returns the complement of the argument BDD.]

  Description [Returns the complement of the argument BDD.]

  SideEffects [The reference count of the argument BDD is increased by 1.]

  SeeAlso     [Cal_BddIdentity]

******************************************************************************/
Cal_Bdd
Cal_BddNot(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  /* Interface BDD reference count */
  CalBddNode_t *bddNode = CAL_BDD_POINTER(userBdd);
  CalBddNodeIcrRefCount(bddNode);
  return CalBddNodeNot(userBdd);
}

/**Function********************************************************************

  Synopsis    [Returns the index of the top variable of the argument BDD.]

  Description [Returns the index of the top variable of the argument BDD.]

  SideEffects [None]

  SeeAlso     [Cal_BddGetIfId]

******************************************************************************/
Cal_BddId_t
Cal_BddGetIfIndex(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  Cal_Bdd_t F;
  if (CalBddPreProcessing(bddManager, 1, userBdd) == 1){
    F = CalBddGetInternalBdd(bddManager, userBdd);
    if (CalBddIsBddConst(F)){
      return -1;
    }
    return CalBddGetBddIndex(bddManager, F);
  }
  return -1;
}


/**Function********************************************************************

  Synopsis    [Returns the id of the top variable of the argument BDD.]

  Description [Returns the id of the top variable of the argument BDD.]

  SideEffects [None]

  SeeAlso     [Cal_BddGetIfIndex]

******************************************************************************/
Cal_BddId_t
Cal_BddGetIfId(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  Cal_Bdd_t F;
  if (CalBddPreProcessing(bddManager, 1, userBdd) == 1){
    F = CalBddGetInternalBdd(bddManager, userBdd);
    if (CalBddIsBddConst(F)){
      return 0;
    }
    return CalBddGetBddId(F);
  }
  return -1;
}

/**Function********************************************************************

  Synopsis    [Returns the BDD corresponding to the top variable of
  the argument BDD.]

  Description [Returns the BDD corresponding to the top variable of
  the argument BDD.]

  SideEffects [None.]

******************************************************************************/
Cal_Bdd
Cal_BddIf(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  Cal_Bdd_t F;
  if (CalBddPreProcessing(bddManager, 1, userBdd) == 0){
    return Cal_BddNull(bddManager);
  }
  F = CalBddGetInternalBdd(bddManager, userBdd);
  if (CalBddIsBddConst(F)){
    CalBddWarningMessage("Cal_BddIf: argument is constant");
  }
  return CalBddGetExternalBdd(bddManager, bddManager->varBdds[CalBddGetBddId(F)]);
}


/**Function********************************************************************

  Synopsis    [Returns the positive cofactor of the argument BDD with
  respect to the top variable of the BDD.]

  Description [Returns the positive cofactor of the argument BDD with
  respect to the top variable of the BDD.]

  SideEffects [The reference count of the returned BDD is increased by 1.]

  SeeAlso     [Cal_BddElse]

******************************************************************************/
Cal_Bdd
Cal_BddThen(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  Cal_Bdd_t thenBdd;
  Cal_Bdd_t F;
  if (CalBddPreProcessing(bddManager, 1, userBdd) == 0){
    return Cal_BddNull(bddManager);
  }
  F = CalBddGetInternalBdd(bddManager, userBdd);
  CalBddGetThenBdd(F, thenBdd);
  return CalBddGetExternalBdd(bddManager, thenBdd);
}

/**Function********************************************************************

  Synopsis    [Returns the negative cofactor of the argument BDD with
  respect to the top variable of the BDD.]

  Description [Returns the negative cofactor of the argument BDD with
  respect to the top variable of the BDD.]

  SideEffects [The reference count of the returned BDD is increased by 1.]

  SeeAlso     [Cal_BddThen]

******************************************************************************/
Cal_Bdd
Cal_BddElse(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  Cal_Bdd_t elseBdd;
  Cal_Bdd_t F;
  if (CalBddPreProcessing(bddManager, 1, userBdd) == 0){
    return Cal_BddNull(bddManager);
  }
  F = CalBddGetInternalBdd(bddManager, userBdd);
  CalBddGetElseBdd(F, elseBdd);
  return CalBddGetExternalBdd(bddManager, elseBdd);
}

/**Function********************************************************************

  Synopsis    [Frees the argument BDD.]

  Description [Frees the argument BDD. It is an error to free a BDD
  more than once.]

  SideEffects [The reference count of the argument BDD is decreased by 1.]

  SeeAlso     [Cal_BddUnFree]

******************************************************************************/
void
Cal_BddFree(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  /* Interface BDD reference count */
  CalBddNodeDcrRefCount(CAL_BDD_POINTER(userBdd));
}


/**Function********************************************************************

  Synopsis    [Unfrees the argument BDD.]

  Description [Unfrees the argument BDD. It is an error to pass a BDD
  with reference count of zero to be unfreed.]

  SideEffects [The reference count of the argument BDD is increased by 1.]

  SeeAlso     [Cal_BddFree]

******************************************************************************/
void
Cal_BddUnFree(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  /* Interface BDD reference count */
  CalBddNode_t *bddNode = CAL_BDD_POINTER(userBdd);
  CalBddNodeIcrRefCount(bddNode);
}


/**Function********************************************************************

  Synopsis    [Returns a BDD with positive from a given BDD with arbitrary phase]

  Description [Returns a BDD with positive from a given BDD with arbitrary phase]

  SideEffects [None.]

******************************************************************************/
Cal_Bdd
Cal_BddGetRegular(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  return CAL_BDD_POINTER(userBdd);
}

/**Function********************************************************************

  Synopsis    [Computes a BDD that implies conjunction of f and g.]

  Description [Computes a BDD that implies conjunction of f and g.]

  SideEffects [None]

  SeeAlso     [Cal_BddImplies]

******************************************************************************/
Cal_Bdd
Cal_BddIntersects(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd
                  gUserBdd) 
{
  Cal_Bdd_t result;
  Cal_Bdd_t f, g;
  if (CalBddPreProcessing(bddManager, 2, fUserBdd, gUserBdd) == 0){
    return Cal_BddNull(bddManager);
  }
  f = CalBddGetInternalBdd(bddManager, fUserBdd);
  g = CalBddGetInternalBdd(bddManager, gUserBdd);
  result = BddIntersectsStep(bddManager,f,g);
  return CalBddGetExternalBdd(bddManager, result);
}

/**Function********************************************************************

  Synopsis    [Computes a BDD that implies conjunction of f and Cal_BddNot(g)]

  Description [Computes a BDD that implies conjunction of f and Cal_BddNot(g)]

  SideEffects [none]

  SeeAlso     [Cal_BddIntersects]

******************************************************************************/
Cal_Bdd
Cal_BddImplies(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd)
{
  Cal_Bdd_t result;
  Cal_Bdd_t f, g;
  if (CalBddPreProcessing(bddManager, 2, fUserBdd, gUserBdd)){
    Cal_Bdd_t gNot;
    f = CalBddGetInternalBdd(bddManager, fUserBdd);
    g = CalBddGetInternalBdd(bddManager, gUserBdd);
    CalBddNot(g, gNot);
    result = BddIntersectsStep(bddManager,f, gNot);
  }
  else{
    return Cal_BddNull(bddManager);
  }
  return CalBddGetExternalBdd(bddManager, result);
}

/**Function********************************************************************

  Synopsis    [Returns the number of nodes in the Unique table]

  Description [Returns the number of nodes in the Unique table]

  SideEffects [None]

  SeeAlso     [Cal_BddManagerGetNumNodes]

******************************************************************************/
unsigned long
Cal_BddTotalSize(Cal_BddManager bddManager)
{
  return Cal_BddManagerGetNumNodes(bddManager);
}


/**Function********************************************************************

  Synopsis    [Prints miscellaneous BDD statistics]

  Description [Prints miscellaneous BDD statistics]

  SideEffects [None]

******************************************************************************/
void
Cal_BddStats(Cal_BddManager bddManager, FILE * fp)
{
  unsigned long cacheInsertions = 0;
  unsigned long cacheEntries = 0;
  unsigned long cacheSize = 0;
  unsigned long cacheHits = 0;
  unsigned long cacheLookups = 0;
  unsigned long cacheCollisions = 0;
  unsigned long numLockedNodes = 0;
  int i, id, depth;
  long numPages;
  unsigned long totalBytes;
  
  
  fprintf(fp, "**** CAL modifiable parameters ****\n");
  fprintf(fp, "Node limit: %ld\n", bddManager->nodeLimit);
  fprintf(fp, "Garbage collection enabled: %s\n",
          ((bddManager->gcMode) ? "yes" : "no"));
  fprintf(fp, "Maximum number of variables sifted per reordering: %ld\n", 
          bddManager->maxNumVarsSiftedPerReordering);
  fprintf(fp, "Maximum number of variable swaps per reordering: %ld\n",
          bddManager->maxNumSwapsPerReordering);
  fprintf(fp, "Maximum growth while sifting a variable: %2.2f\n",
          bddManager->maxSiftingGrowth);
  fprintf(fp, "Dynamic reordering of BDDs enabled: %s\n", 
          ((bddManager->reorderTechnique != CAL_REORDER_NONE) ? "yes" : "no"));
  fprintf(fp, "Repacking after GC Threshold: %f\n", 
          bddManager->repackAfterGCThreshold);
  fprintf(fp, "**** CAL statistics ****\n");
  fprintf(fp, "Total BDD Node Usage : %ld nodes, %ld Bytes\n",
          bddManager->numNodes, bddManager->numNodes*sizeof(CalBddNode_t));
  fprintf(fp, "Peak BDD Node Usage : %ld nodes, %ld Bytes\n",
          bddManager->numPeakNodes,
          bddManager->numPeakNodes*sizeof(CalBddNode_t)); 
  for (i=1; i<=bddManager->numVars; i++){
    numLockedNodes += CalBddUniqueTableNumLockedNodes(bddManager,
                                                      bddManager->uniqueTable[i]);
  }
  fprintf(fp, "Number of nodes locked: %ld\n", numLockedNodes);
  fprintf(fp, "Total Number of variables: %d\n", bddManager->numVars);
  numPages =
      bddManager->pageManager1->totalNumPages+
      bddManager->pageManager2->totalNumPages;
  fprintf(fp, "Total memory allocated for BDD nodes: %ld pages (%ld Bytes)\n",
          numPages, PAGE_SIZE*numPages);
  /* Calculate the memory consumed */
  totalBytes =
      /* Over all bdd manager */
      sizeof(Cal_BddManager_t)+
      bddManager->maxNumVars*(sizeof(Cal_Bdd_t)+sizeof(CalNodeManager_t *)+
                              sizeof(CalHashTable_t *) +
                              sizeof(CalHashTable_t *) + sizeof(CalRequestNode_t*)*2)+
      sizeof(CalPageManager_t)*2+
      /* Page manager */
      bddManager->pageManager1->maxNumSegments*(sizeof(CalAddress_t *)+sizeof(int))+
      bddManager->pageManager2->maxNumSegments*
      (sizeof(CalAddress_t *)+sizeof(int)); 

  for (id=0; id <= bddManager->numVars; id++){
    totalBytes += bddManager->nodeManagerArray[id]->maxNumPages*sizeof(int);;
  }
  /* IndexToId and IdToIndex */
  totalBytes += 2*bddManager->maxNumVars*(sizeof(Cal_BddIndex_t));
  for (id=0; id <= bddManager->numVars; id++){
    totalBytes += bddManager->uniqueTable[id]->numBins*sizeof(int);;
  }
  /* Cache Table */
  totalBytes += CalCacheTableMemoryConsumption(bddManager->cacheTable);
  
  /* Req que */
  totalBytes += bddManager->maxDepth*sizeof(CalHashTable_t **);
  for (depth = 0; depth < bddManager->depth; depth++){
    for (id=0; id <= bddManager->numVars; id++){
      if (bddManager->reqQue[depth][id]){
        totalBytes +=
            bddManager->reqQue[depth][id]->numBins*
            sizeof(CalBddNode_t*);
      }
    }
  }
  /* Association */
  totalBytes += sizeof(CalAssociation_t)*2;
  /* Block */
  totalBytes += CalBlockMemoryConsumption(bddManager->superBlock);

  fprintf(fp, "Total memory consumed: %ld Pages (%ld Bytes)\n",
          numPages+totalBytes/PAGE_SIZE,
          PAGE_SIZE*numPages+totalBytes);  

  CalBddManagerGetCacheTableData(bddManager, &cacheSize,
                                 &cacheEntries, &cacheInsertions, 
                                 &cacheLookups, &cacheHits, &cacheCollisions);
  fprintf(fp, "Cache Size: %ld\n", cacheSize);
  fprintf(fp, "Cache Entries: %ld\n", cacheEntries);
  fprintf(fp, "Cache Insertions: %ld\n", cacheInsertions);
  fprintf(fp, "Cache Collisions: %ld\n", cacheCollisions);
  fprintf(fp, "Cache Hits: %ld\n", cacheHits);
  if (cacheLookups){
    fprintf(fp, "Cache Lookup: %ld\n", cacheLookups);
    fprintf(fp, "Cache hit ratio: %-.2f\n", ((double)cacheHits)/cacheLookups);
  }
  fprintf(fp, "Number of nodes garbage collected: %ld\n",
          bddManager->numNodesFreed);
  fprintf(fp,"number of garbage collections: %d\n", bddManager->numGC);
  fprintf(fp,"number of dynamic reorderings: %d\n",
          bddManager->numReorderings); 
  fprintf(fp,"number of trivial swaps: %ld\n", bddManager->numTrivialSwaps); 
  fprintf(fp,"number of swaps in last reordering: %ld\n", bddManager->numSwaps); 
  fprintf(fp,"garbage collection limit: %ld\n", bddManager->uniqueTableGCLimit);
  fflush(fp);
}

/**Function********************************************************************

  Synopsis    [Specify dynamic reordering technique and method.]

  Description [Selects the technique andmethod for dynamic reordering.]

  SideEffects [None]

  SeeAlso     [Cal_BddReorder]

******************************************************************************/
void
Cal_BddDynamicReordering(Cal_BddManager bddManager, int technique, int method)
{
  bddManager->reorderTechnique = technique;
  bddManager->reorderMethod = method;
}

/**Function********************************************************************

  Synopsis    [Invoke the current dynamic reodering method.]

  Description [Invoke the current dynamic reodering method.]

  SideEffects [Index of a variable may change due to reodering]

  SeeAlso     [Cal_BddDynamicReordering]

******************************************************************************/
void
Cal_BddReorder(Cal_BddManager bddManager)
{
  if (bddManager->reorderTechnique == CAL_REORDER_NONE){
    return;
  }
  CalCacheTableTwoFlush(bddManager->cacheTable);
  if (bddManager->reorderMethod == CAL_REORDER_METHOD_DF){
    CalBddReorderAuxDF(bddManager);
  }
  else if (bddManager->reorderMethod == CAL_REORDER_METHOD_BF){ 
    Cal_BddManagerGC(bddManager);
    CalBddReorderAuxBF(bddManager);
  }
}


/**Function********************************************************************

  Synopsis    [Returns type of a BDD ( 0, 1, +var, -var, ovrflow, nonterminal)]

  Description [Returns BDD_TYPE_ZERO if f is false, BDD_TYPE_ONE 
  if f is true, BDD_TYPE_POSVAR is f is an unnegated variable,
  BDD_TYPE_NEGVAR if f is a negated variable, BDD_TYPE_OVERFLOW if f
  is null, and BDD_TYPE_NONTERMINAL otherwise.]

  SideEffects [None]

******************************************************************************/
int
Cal_BddType(Cal_BddManager bddManager, Cal_Bdd fUserBdd)
{
  Cal_Bdd_t f;
  if (CalBddPreProcessing(bddManager, 1, fUserBdd)){
    f = CalBddGetInternalBdd(bddManager, fUserBdd);
    return (CalBddTypeAux(bddManager, f));
  }
  return (CAL_BDD_TYPE_OVERFLOW);
}
/**Function********************************************************************

  Synopsis    [Returns the number of BDD variables]

  Description [Returns the number of BDD variables]

  SideEffects [None]

******************************************************************************/
long
Cal_BddVars(Cal_BddManager bddManager)
{
  return (bddManager->numVars);
}


/**Function********************************************************************

  Synopsis    [Sets the node limit to new_limit and returns the old limit.]

  Description [Sets the node limit to new_limit and returns the old limit.]

  SideEffects [Threshold for garbage collection may change]

  SeeAlso     [Cal_BddManagerGC]

******************************************************************************/
long
Cal_BddNodeLimit(
  Cal_BddManager bddManager,
  long  newLimit)
{
  long oldLimit;

  oldLimit = bddManager->nodeLimit;
  if (newLimit < 0){
    newLimit=0;
  }
  bddManager->nodeLimit = newLimit;
  if (newLimit && (bddManager->uniqueTableGCLimit > newLimit)){
    bddManager->uniqueTableGCLimit = newLimit;
  }
  return (oldLimit);
}

/**Function********************************************************************

  Synopsis    [Returns 1 if the node limit has been exceeded, 0 otherwise. The
  overflow flag is cleared.]

  Description [Returns 1 if the node limit has been exceeded, 0 otherwise. The
  overflow flag is cleared.]

  SideEffects [None]

  SeeAlso     [Cal_BddNodeLimit]

******************************************************************************/
int
Cal_BddOverflow(Cal_BddManager bddManager)
{
  int result;
  result = bddManager->overflow;
  bddManager->overflow = 0;
  return (result);
}

/**Function********************************************************************

  Synopsis           [Returns 1 if the argument BDD is a cube, 0 otherwise]

  Description        [Returns 1 if the argument BDD is a cube, 0 otherwise]

  SideEffects        [None]

******************************************************************************/
int
Cal_BddIsCube(
  Cal_BddManager bddManager,
  Cal_Bdd fUserBdd)
{
  Cal_Bdd_t f0, f1;
  Cal_Bdd_t f;
  f = CalBddGetInternalBdd(bddManager, fUserBdd);
  if (CalBddIsBddConst(f)){
    if (CalBddIsBddZero(bddManager, f)){
      CalBddFatalMessage("Cal_BddIsCube called with 0");
    }
    else return 1;
  }

  CalBddGetThenBdd(f, f1);
  CalBddGetElseBdd(f, f0);
  /*
   * Exactly one branch of f must point to ZERO to be a cube.
   */
  if (CalBddIsBddZero(bddManager, f1)){
	return (CalBddIsCubeStep(bddManager, f0));
  } else if (CalBddIsBddZero(bddManager, f0)){
	return (CalBddIsCubeStep(bddManager, f1));
  } else { /* not a cube, because neither branch is zero */
	return 0;
  }
}

/**Function********************************************************************

  Synopsis           [Returns the hooks field of the manager.]

  Description        [Returns the hooks field of the manager.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void *
Cal_BddManagerGetHooks(Cal_BddManager bddManager)
{
  return bddManager->hooks;
}

/**Function********************************************************************

  Synopsis           [Sets the hooks field of the manager.]

  Description        [Sets the hooks field of the manager.]

  SideEffects        [Hooks field changes. ]

  SeeAlso            []

******************************************************************************/
void 
Cal_BddManagerSetHooks(Cal_BddManager bddManager, void *hooks)
{
  bddManager->hooks = hooks;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Returns the BDD corresponding to the top variable of
  the argument BDD.]

  Description [Returns the BDD corresponding to the top variable of
  the argument BDD.]

  SideEffects [None.]

******************************************************************************/
Cal_Bdd_t
CalBddIf(Cal_BddManager bddManager, Cal_Bdd_t F)
{
  if (CalBddIsBddConst(F)){
    CalBddWarningMessage("CalBddIf: argument is constant");
  }
  return bddManager->varBdds[CalBddGetBddId(F)];
}

/**Function********************************************************************

  Synopsis           [Returns 1 if the argument BDD is a cube, 0 otherwise]

  Description        [Returns 1 if the argument BDD is a cube, 0 otherwise]

  SideEffects        [None]

******************************************************************************/
int
CalBddIsCubeStep(Cal_BddManager bddManager, Cal_Bdd_t f)
{
  Cal_Bdd_t f0, f1;
  if (CalBddIsBddConst(f)){
    if (CalBddIsBddZero(bddManager, f)){
      CalBddFatalMessage("Cal_BddIsCube called with 0");
    }
    else return 1;
  }

  CalBddGetThenBdd(f, f1);
  CalBddGetElseBdd(f, f0);
  /*
   * Exactly one branch of f must point to ZERO to be a cube.
   */
  if (CalBddIsBddZero(bddManager, f1)){
	return (CalBddIsCubeStep(bddManager, f0));
  } else if (CalBddIsBddZero(bddManager, f0)){
	return (CalBddIsCubeStep(bddManager, f1));
  } else { /* not a cube, because neither branch is zero */
	return 0;
  }
}

/**Function********************************************************************

  Synopsis    [Returns the BDD type by recursively traversing the argument BDD]

  Description [Returns the BDD type by recursively traversing the argument BDD]

  SideEffects [None]

******************************************************************************/
int
CalBddTypeAux(Cal_BddManager_t * bddManager, Cal_Bdd_t f)
{
  Cal_Bdd_t thenBdd, elseBdd;
  
  if (CalBddIsBddConst(f)){
    if (CalBddIsBddZero(bddManager, f)) return (CAL_BDD_TYPE_ZERO);
    if (CalBddIsBddOne(bddManager, f)) return (CAL_BDD_TYPE_ONE);
  }
  CalBddGetThenBdd(f, thenBdd);
  CalBddGetElseBdd(f, elseBdd);
  if (CalBddIsBddOne(bddManager, thenBdd) &&
      CalBddIsBddZero(bddManager, elseBdd))
    return CAL_BDD_TYPE_POSVAR;
  if (CalBddIsBddZero(bddManager, thenBdd) &&
      CalBddIsBddOne(bddManager, elseBdd))
    return (CAL_BDD_TYPE_NEGVAR);
  return (CAL_BDD_TYPE_NONTERMINAL);
}

/**Function********************************************************************

  Synopsis    [Returns the duplicate BDD of the argument BDD.]

  Description [Returns the duplicate BDD of the argument BDD.]

  SideEffects [The reference count of the BDD is increased by 1.]

  SeeAlso     [Cal_BddNot]

******************************************************************************/
Cal_Bdd_t
CalBddIdentity(Cal_BddManager_t *bddManager, Cal_Bdd_t calBdd)
{
  CalBddIcrRefCount(calBdd);
  return calBdd;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Recursive routine to returns a BDD that implies conjunction of
  argument BDDs]

  Description [Recursive routine to returns a BDD that implies conjunction of
  argument BDDs]

  SideEffects [None]

******************************************************************************/
static Cal_Bdd_t
BddIntersectsStep(Cal_BddManager_t * bddManager, Cal_Bdd_t  f, Cal_Bdd_t  g)
{
  Cal_Bdd_t f1, f2, g1, g2, result, temp;
  Cal_BddId_t topId;
  
  
  if (CalBddIsBddConst(f)){
    if (CalBddIsBddZero(bddManager, f)){
      return f;
    }
    else {
      return g;
    }
  }
  if (CalBddIsBddConst(g)){
    if (CalBddIsBddZero(bddManager, g)){
      return g;
    }
    else {
      return f;
    }
  }
  if (CalBddSameOrNegation(f, g)){
    if (CalBddIsEqual(f, g)){
      return f;
    }
    else
      return bddManager->bddZero;
  }
  if (CAL_BDD_OUT_OF_ORDER(f, g)) CAL_BDD_SWAP(f, g);
  CalBddGetMinId2(bddManager, f, g, topId);
  CalBddGetCofactors(f, topId, f1, f2);
  CalBddGetCofactors(g, topId, g1, g2);
  temp = BddIntersectsStep(bddManager, f1, g1);
  if (CalBddIsBddZero(bddManager, temp)){
    temp = BddIntersectsStep(bddManager, f2, g2);
    if (CalBddIsBddZero(bddManager, temp)){
      return bddManager->bddZero;
    }
    else{
      if(CalUniqueTableForIdFindOrAdd(bddManager,
                                      bddManager->uniqueTable[topId],  
                                      bddManager->bddZero, temp,
                                      &result) == 0){
        CalBddIcrRefCount(temp);
      }
    }
  }
  else {
    if(CalUniqueTableForIdFindOrAdd(bddManager, bddManager->uniqueTable[topId],
                          temp, bddManager->bddZero,&result) == 0){
      CalBddIcrRefCount(temp);
    }
  }
  return result;
}






















