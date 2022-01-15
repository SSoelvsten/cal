/**CFile***********************************************************************

  FileName    [calUtil.c]

  PackageName [cal]

  Synopsis    [Utility functions for the Cal package.]

  Description [Utility functions used in the Cal package.]

  SeeAlso     [optional]

  Author      [Jagesh Sanghavi (sanghavi@eecs.berkeley.edu)
               Rajeev K. Ranjan   (rajeev@eecs.berkeley.edu)
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

  Revision    [$Id: calUtil.c,v 1.3 1998/09/15 19:02:54 ravi Exp $]

******************************************************************************/

#include "calInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/* Random generator constants. */
#define CAL_MODULUS1 2147483563
#define CAL_LEQA1 40014
#define CAL_LEQQ1 53668
#define CAL_LEQR1 12211
#define CAL_MODULUS2 2147483399
#define CAL_LEQA2 40692
#define CAL_LEQQ2 52774
#define CAL_LEQR2 3791
#define CAL_STAB_SIZE 64
#define CAL_STAB_DIV (1 + (CAL_MODULUS1 - 1) / CAL_STAB_SIZE)

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
static long utilRand = 0;
static long utilRand2;
static long shuffleSelect;
static long shuffleTable[CAL_STAB_SIZE];

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
void
Cal_ImageDump(Cal_BddManager_t *bddManager, FILE *fp)
{

  CalPageManager_t *pageManager;
  int i, j;
  char *segment, c;
  int count = NUM_PAGES_PER_SEGMENT * PAGE_SIZE;

  pageManager = bddManager->pageManager1;
  for(i = 0; i < pageManager->numSegments; i++){
    segment = (char *) pageManager->pageSegmentArray[i];
    for(j = 1; j <= count; j++){
      c = segment[j];
      fprintf(fp, "%c", j%64?c:'\n');
    }
  }
  pageManager = bddManager->pageManager2;
  for(i = 0; i < pageManager->numSegments; i++){
    segment = (char *) pageManager->pageSegmentArray[i];
    for(j = 1; j <= count; j++){
      c = segment[j];
      fprintf(fp, "%c", j%64?c:'\n');
    }
  }
}


/**Function********************************************************************

  Synopsis    [Prints the function implemented by the argument BDD]

  Description [Prints the function implemented by the argument BDD]

  SideEffects [None]

******************************************************************************/
void
Cal_BddFunctionPrint(Cal_BddManager bddManager, Cal_Bdd  userBdd,
                     char *name)
{
  Cal_Bdd_t calBdd;
  calBdd = CalBddGetInternalBdd(bddManager, userBdd);
  CalBddFunctionPrint(bddManager, calBdd, name);
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
void
CalUniqueTablePrint(Cal_BddManager_t *bddManager)
{
  int i;
  for(i = 0; i <= bddManager->numVars; i++){
    CalHashTablePrint(bddManager->uniqueTable[i]);
  }
}

/**Function********************************************************************

  Synopsis    [Prints the function implemented by the argument BDD]

  Description [Prints the function implemented by the argument BDD]

  SideEffects [None]

******************************************************************************/
void
CalBddFunctionPrint(Cal_BddManager_t * bddManager,
                    Cal_Bdd_t  calBdd,
                    char * name)
{
  Cal_Bdd_t T,E;
  Cal_BddId_t id;
  char c;
  static int level;

  if(level == 0)printf("%s = ",name);
  level++;
  printf("( ");
  if(CalBddIsBddZero(bddManager, calBdd)){
    printf("0 ");
  }
  else if(CalBddIsBddOne(bddManager, calBdd)){
    printf("1 ");
  }
  else{
    id = CalBddGetBddId(calBdd);
    c = (char)((int)'a' + id - 1);
    printf("%c ", c);
    CalBddGetCofactors(calBdd, id, T, E);
    CalBddFunctionPrint(bddManager, T, " ");
    printf("+ %c' ", c);
    CalBddFunctionPrint(bddManager, E, " ");
  }
  level--;
  printf(") ");
  if(level == 0)printf("\n");
}

/**Function********************************************************************
  
  Synopsis    [required]
  
  Description [optional]
  
  SideEffects [required]
  
  SeeAlso     [optional]
  
******************************************************************************/
#if HAVE_STDARG_H
int
CalBddPreProcessing(Cal_BddManager_t *bddManager, int count, ...)
{
  int allValid;
  va_list ap;
  Cal_Bdd fUserBdd;
  Cal_Bdd_t f;
  
  va_start(ap, count);
#else
#  if HAVE_VARARGS_H
int
CalBddPreProcessing(va_alist)
va_dcl
{
  int allValid;
  va_list ap;
  Cal_Bdd *fUserBdd;
  int count;
  Cal_BddManager_t *bddManager;
  Cal_Bdd_t f;
  
  va_start(ap);
  bddManager = va_arg(ap, Cal_BddManager_t *);
  count = va_arg(ap, int);
#  endif
#endif  

  allValid=1;
  while (count){
    fUserBdd = va_arg(ap, Cal_Bdd);
	if (fUserBdd == 0){
	  allValid=0;
    }
	else {
      f = CalBddGetInternalBdd(bddManager, fUserBdd);
      if (CalBddIsRefCountZero(f)){
        CalBddFatalMessage("Bdd With Zero Reference Count Used.");
      }
    }
    --count;
  }
  if (allValid) {
    CalBddPostProcessing(bddManager);
  }
  va_end(ap);
  return (allValid);
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalBddPostProcessing(Cal_BddManager_t *bddManager)
{
  if (bddManager->gcCheck > 0) return CAL_BDD_OK;
  bddManager->gcCheck = CAL_GC_CHECK;
  if(bddManager->numNodes > bddManager->uniqueTableGCLimit){
    long origNodes = bddManager->numNodes;
    Cal_BddManagerGC(bddManager);
    if ((bddManager->numNodes > bddManager->reorderingThreshold) &&
        (3*bddManager->numNodes > 2* bddManager->uniqueTableGCLimit) &&
        (bddManager->dynamicReorderingEnableFlag) &&
        (bddManager->reorderTechnique != CAL_REORDER_NONE)){
      CalCacheTableTwoFlush(bddManager->cacheTable);
      if (bddManager->reorderMethod == CAL_REORDER_METHOD_BF){
        CalBddReorderAuxBF(bddManager);
      }
      else{
        CalBddReorderAuxDF(bddManager);
      }
    }
    else {
      /* Check if we should repack */
      Cal_Assert(CalCheckAllValidity(bddManager));
      if (bddManager->numNodes <
          bddManager->repackAfterGCThreshold*origNodes){
        CalRepackNodesAfterGC(bddManager);
      }
      Cal_Assert(CalCheckAllValidity(bddManager));
    }
    Cal_BddManagerSetGCLimit(bddManager);
    if (bddManager->nodeLimit && (bddManager->numNodes >
                                  bddManager->nodeLimit)){ 
      CalBddWarningMessage("Overflow: Node Limit Exceeded");
      bddManager->overflow = 1;
      return CAL_BDD_OVERFLOWED;
    }
    /*
     * Check to see if the cache table needs to be rehashed.
     */
    CalCacheTableRehash(bddManager);
  }
  return CAL_BDD_OK;
}

/**Function********************************************************************
  
  Synopsis    [required]
  
  Description [optional]
  
  SideEffects [required]
  
  SeeAlso     [optional]
  
******************************************************************************/
int
CalBddArrayPreProcessing(Cal_BddManager_t *bddManager, Cal_Bdd *userBddArray) 
{
  int i = 0;
  Cal_Bdd userBdd;
  while ((userBdd = userBddArray[i++])){
    if (CalBddPreProcessing(bddManager, 1, userBdd) == 0){
      return 0;
    }
  }
  return 1;
}
    
                       
/**Function********************************************************************

  Name        [CalBddFatalMessage]

  Synopsis    [Prints fatal message and exits.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
Cal_Bdd_t
CalBddGetInternalBdd(Cal_BddManager bddManager, Cal_Bdd userBdd)
{
  Cal_Bdd_t resultBdd;
  if (CalBddNodeIsOutPos(userBdd)){
    CalBddNodeGetThenBdd(userBdd, resultBdd);
  }
  else {
    Cal_Bdd userBddNot = CalBddNodeNot(userBdd);
    Cal_Bdd_t internalBdd;
	CalBddNodeGetThenBdd(userBddNot,internalBdd);
    CalBddNot(internalBdd, resultBdd);
  }
  return resultBdd;
}

/**Function********************************************************************

  Name        [CalBddFatalMessage]

  Synopsis    [Prints fatal message and exits.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
Cal_Bdd
CalBddGetExternalBdd(Cal_BddManager_t *bddManager, Cal_Bdd_t internalBdd)
{
  CalHashTable_t *hashTableForUserBdd = bddManager->uniqueTable[0];
  Cal_Bdd_t resultBdd;
  int found;
  
  if(CalBddIsOutPos(internalBdd)){
    found = CalHashTableFindOrAdd(hashTableForUserBdd, internalBdd,
                          bddManager->bddOne, &resultBdd);
  }
  else {
    Cal_Bdd_t internalBddNot;
    CalBddNot(internalBdd, internalBddNot);
    found = CalHashTableFindOrAdd(hashTableForUserBdd, internalBddNot,
                          bddManager->bddOne, &resultBdd);
    CalBddNot(resultBdd, resultBdd);
  }
  if (found == 0){
    CalBddIcrRefCount(internalBdd);
  }
  CalBddIcrRefCount(resultBdd);
  return CalBddGetBddNode(resultBdd);
}


/**Function********************************************************************

  Name        [CalBddFatalMessage]

  Synopsis    [Prints fatal message and exits.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalBddFatalMessage(char *string)
{
  (void) fprintf(stderr,"Fatal: %s\n", string);
  exit(-1);
}
/**Function********************************************************************

  Name        [CalBddWarningMessage]

  Synopsis    [Prints warning message.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalBddWarningMessage(char *string)
{
  (void) fprintf(stderr,"Warning: %s\n", string);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalBddNodePrint(CalBddNode_t *bddNode)
{
  int refCount;
  CalBddNodeGetRefCount(bddNode, refCount);
  printf("Node (%lx) thenBdd(%2d %lx)  elseBdd(%2d %lx) ref_count (%d) next (%lx)\n",
         (CalAddress_t)bddNode,
         CalBddNodeGetThenBddId(bddNode),
         (CalAddress_t) CalBddNodeGetThenBddNode(bddNode), 
         CalBddNodeGetElseBddId(bddNode),
         (CalAddress_t) CalBddNodeGetElseBddNode(bddNode),
         refCount, (CalAddress_t)bddNode->nextBddNode);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/

void
CalBddPrint(Cal_Bdd_t calBdd)
{
  printf("Id(%2d) node(%lx) ",
      CalBddGetBddId(calBdd), (CalAddress_t) CalBddGetBddNode(calBdd));
  printf("thenBdd(%2d %lx)  elseBdd(%2d %lx)\n",
         CalBddGetThenBddId(calBdd),
         (CalAddress_t) CalBddGetThenBddNode(calBdd), 
         CalBddGetElseBddId(calBdd),
         (CalAddress_t) CalBddGetElseBddNode(calBdd));
}

#ifdef TEST_CALBDDNODE
main(int argc, char **argv)
{
  CalBddNode_t *bddNode, *thenBddNode, *elseBddNode;

  bddNode = Cal_MemAlloc(CalBddNode_t, 1);
  thenBddNode = Cal_MemAlloc(CalBddNode_t, 1);
  elseBddNode = Cal_MemAlloc(CalBddNode_t, 1);

  CalBddNodePutThenBddId(bddNode, 1);
  CalBddNodePutThenBddNode(bddNode, thenBddNode);
  CalBddNodePutElseBddId(bddNode, 2);
  CalBddNodePutElseBddNode(bddNode, elseBddNode);

  printf("then( 1 %x) else( 2 %x)\n", thenBddNode, elseBddNode);
  CalBddNodePrint(bddNode);
}
#endif


/**Function********************************************************************

  Synopsis    [Prints a hash table.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalHashTablePrint(CalHashTable_t *hashTable)
{
  int i;
  CalBddNode_t *ptr;
  Cal_Bdd_t calBdd, T, E;
  int refCount, firstFlag;

  printf("HashTable bddId(%d) entries(%ld) bins(%ld) capacity(%ld)\n",
         hashTable->bddId, hashTable->numEntries, hashTable->numBins,
         hashTable->maxCapacity);
  for(i = 0; i < hashTable->numBins; i++){
    ptr = hashTable->bins[i];
    firstFlag = 1;
    while(ptr != Cal_Nil(CalBddNode_t)){
      CalBddPutBddNode(calBdd, ptr);
      CalBddNodeGetThenBdd(ptr, T);
      CalBddNodeGetElseBdd(ptr, E);
      if (firstFlag){
        printf("\tbin = (%d) ", i);
        firstFlag = 0;
      }
      printf("\t\tbddNode(%lx) ", (CalAddress_t)ptr);
      printf("thenId(%d) ", CalBddGetBddId(T)); 
      printf("thenBddNode(%lx) ", (CalAddress_t) CalBddGetBddNode(T));
      printf("elseId(%d) ", CalBddGetBddId(E)); 
      printf("elseBddNode(%lx) ", (unsigned long)CalBddGetBddNode(E));
      CalBddGetRefCount(calBdd, refCount);
      printf("refCount(%d)\n", refCount);
      ptr = CalBddNodeGetNextBddNode(ptr);
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
CalHashTableOnePrint(CalHashTable_t *hashTable, int flag)
{
  int i;
  CalBddNode_t *ptr, *node;
  Cal_Bdd_t keyBdd;

  printf("*************************************************\n");
  for(i = 0; i < hashTable->numBins; i++){
    ptr = hashTable->bins[i];
    while(ptr != Cal_Nil(CalBddNode_t)){
      CalBddNodeGetThenBdd(ptr, keyBdd);
      node = CalBddNodeGetElseBddNode(ptr);
      if(flag == 1){
        printf("Key(%d %lx) Value(%f)\n", 
            CalBddGetBddId(keyBdd), (CalAddress_t)CalBddGetBddNode(keyBdd), *(double *)node); 
      }
      else{
        printf("Key(%d %lx) Value(%d)\n",
            CalBddGetBddId(keyBdd), (CalAddress_t)CalBddGetBddNode(keyBdd),  *(int *)node); 
      }
      ptr = CalBddNodeGetNextBddNode(ptr);
    }
  }
}

/**Function********************************************************************

  Synopsis    [Initializer for the portable random number generator.]

  Description [Initializer for the portable number generator based on
  ran2 in "Numerical Recipes in C." The input is the seed for the
  generator. If it is negative, its absolute value is taken as seed.
  If it is 0, then 1 is taken as seed. The initialized sets up the two
  recurrences used to generate a long-period stream, and sets up the
  shuffle table.]

  SideEffects [None]

  SeeAlso     [CalUtilRandom]

******************************************************************************/
void
CalUtilSRandom(long seed)
{
    int i;

    if (seed < 0)       utilRand = -seed;
    else if (seed == 0) utilRand = 1;
    else                utilRand = seed;
    utilRand2 = utilRand;
    /* Load the shuffle table (after 11 warm-ups). */
    for (i = 0; i < CAL_STAB_SIZE + 11; i++) {
	long int w;
	w = utilRand / CAL_LEQQ1;
	utilRand = CAL_LEQA1 * (utilRand - w * CAL_LEQQ1) - w * CAL_LEQR1;
	utilRand += (utilRand < 0) * CAL_MODULUS1;
	shuffleTable[i % CAL_STAB_SIZE] = utilRand;
    }
    shuffleSelect = shuffleTable[1 % CAL_STAB_SIZE];
} /* end of CalUtilSRandom */

/**Function********************************************************************

  Synopsis    [Portable random number generator.]

  Description [Portable number generator based on ran2 from "Numerical
  Recipes in C." It is a long period (> 2 * 10^18) random number generator
  of L'Ecuyer with Bays-Durham shuffle. Returns a long integer uniformly
  distributed between 0 and 2147483561 (inclusive of the endpoint values).
  The random generator can be explicitly initialized by calling
  CalUtilSRandom. If no explicit initialization is performed, then the
  seed 1 is assumed.]

  SideEffects []

  SeeAlso     [CalUtilSRandom]

******************************************************************************/
long
CalUtilRandom()
{
    int i;	/* index in the shuffle table */
    long int w; /* work variable */

    /* utilRand == 0 if the geneartor has not been initialized yet. */
    if (utilRand == 0) CalUtilSRandom((long)1);

    /* Compute utilRand = (utilRand * CAL_LEQA1) % CAL_MODULUS1 avoiding
    ** overflows by Schrage's method.
    */
    w          = utilRand / CAL_LEQQ1;
    utilRand   = CAL_LEQA1 * (utilRand - w * CAL_LEQQ1) - w * CAL_LEQR1;
    utilRand  += (utilRand < 0) * CAL_MODULUS1;

    /* Compute utilRand2 = (utilRand2 * CAL_LEQA2) % CAL_MODULUS2 avoiding
    ** overflows by Schrage's method.
    */
    w          = utilRand2 / CAL_LEQQ2;
    utilRand2  = CAL_LEQA2 * (utilRand2 - w * CAL_LEQQ2) - w * CAL_LEQR2;
    utilRand2 += (utilRand2 < 0) * CAL_MODULUS2;

    /* utilRand is shuffled with the Bays-Durham algorithm.
    ** shuffleSelect and utilRand2 are combined to generate the output.
    */

    /* Pick one element from the shuffle table; "i" will be in the range
    ** from 0 to CAL_STAB_SIZE-1.
    */
    i = shuffleSelect / CAL_STAB_DIV;
    /* Mix the element of the shuffle table with the current iterate of
    ** the second sub-generator, and replace the chosen element of the
    ** shuffle table with the current iterate of the first sub-generator.
    */
    shuffleSelect   = shuffleTable[i] - utilRand2;
    shuffleTable[i] = utilRand;
    shuffleSelect  += (shuffleSelect < 1) * (CAL_MODULUS1 - 1);
    /* Since shuffleSelect != 0, and we want to be able to return 0,
    ** here we subtract 1 before returning.
    */
    return(shuffleSelect - 1);

} /* end of CalUtilRandom */


