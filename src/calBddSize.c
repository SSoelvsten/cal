/**CFile***********************************************************************

  FileName    [calBddSize.c]

  PackageName [cal]

  Synopsis    [BDD size and profile routines]
              

  Description [ ]

  SeeAlso     [optional]

  Author      [Jagesh Sanghavi (sanghavi@eecs.berkeley.edu)
               Rajeev Ranjan   (rajeev@eecs.berkeley.edu)
               Originally written by David Long.
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

  Revision    [$Id: calBddSize.c,v 1.1.1.3 1998/05/04 00:58:53 hsv Exp $]

******************************************************************************/
#include "calInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef int (*CountFn_t)(Cal_Bdd_t);

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

static void BddMarkBdd(Cal_Bdd_t f);
static int BddCountNoNodes(Cal_Bdd_t f);
static int BddCountNodes(Cal_Bdd_t f);
static long BddSizeStep(Cal_Bdd_t f, CountFn_t countFn);
static void BddProfileStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, long * levelCounts, CountFn_t countFn);
static void BddHighestRefStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, CalHashTable_t * h);
static void BddDominatedStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, long * funcCounts, CalHashTable_t * h);

/**AutomaticEnd***************************************************************/

static
int (*(countingFns[]))(Cal_Bdd_t) = 
{
  BddCountNoNodes,
  BddCountNodes,
};

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Returns the number of nodes in f when negout is nonzero. If
  negout is zero, we pretend that the BDDs don't have negative-output pointers.]

  Description [optional]

  SideEffects [None]

  SeeAlso     [optional]

******************************************************************************/
long
Cal_BddSize(Cal_BddManager bddManager, Cal_Bdd fUserBdd, int  negout)
{
  Cal_Bdd_t f, g;

  if(CalBddPreProcessing(bddManager, 1, fUserBdd)){
    f = CalBddGetInternalBdd(bddManager, fUserBdd);
    g =  CalBddOne(bddManager);
    CalBddPutMark(g, 0);
    BddMarkBdd(f);
    return BddSizeStep(f, countingFns[!negout]);
  }
  return (0l);
}


/**Function********************************************************************

  Synopsis    [The routine is like Cal_BddSize, but takes a null-terminated
               array of BDDs and accounts for sharing of nodes.]

  Description [optional]

  SideEffects [None]

  SeeAlso     [optional]

******************************************************************************/
long
Cal_BddSizeMultiple(Cal_BddManager bddManager, Cal_Bdd *fUserBddArray,
                    int  negout)
{
  long size;
  Cal_Bdd_t *f;
  Cal_Bdd_t g;
  Cal_Bdd_t *fArray;
  int i, j;
  Cal_Bdd userBdd;
  
  if (CalBddArrayPreProcessing(bddManager, fUserBddArray) == 0){
    return -1;
  }
  
  for(i = 0; (userBdd = fUserBddArray[i]); ++i);

  fArray = Cal_MemAlloc(Cal_Bdd_t, i+1);
  for (j=0; j < i; j++){
    fArray[j] = CalBddGetInternalBdd(bddManager,fUserBddArray[j]);
  }
  fArray[j] = bddManager->bddNull;
  
  g  =  CalBddOne(bddManager);
  CalBddPutMark(g, 0);
  for(f = fArray; !CalBddIsBddNull(bddManager, *f); ++f){
    BddMarkBdd(*f);
  }
  size  =  0l;
  for(f = fArray; !CalBddIsBddNull(bddManager, *f); ++f){
    size +=  BddSizeStep(*f, countingFns[!negout]);
  }
  Cal_MemFree(fArray);
  return size;
}

/**Function********************************************************************

  Synopsis    [Returns a "node profile" of f, i.e., the number of nodes at each
  level in f.]

  Description [negout is as in Cal_BddSize. levelCounts should be an array of
  size Cal_BddVars(bddManager)+1 to hold the profile.]

  SideEffects [None]

  SeeAlso     [optional]

******************************************************************************/
void
Cal_BddProfile(Cal_BddManager bddManager, Cal_Bdd fUserBdd,
               long * levelCounts, int  negout)
{
  Cal_BddIndex_t i;
  Cal_Bdd_t f, g;
  if(CalBddPreProcessing(bddManager, 1, fUserBdd)){
    f = CalBddGetInternalBdd(bddManager, fUserBdd);
    for(i = 0; i <=  bddManager->numVars; i++){
      levelCounts[i] = 0l;
    }
    g = CalBddOne(bddManager);
    CalBddPutMark(g, 0);
    BddMarkBdd(f);
    BddProfileStep(bddManager, f, levelCounts, countingFns[!negout]);
  }
}


/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [None]

  SeeAlso     [optional]

******************************************************************************/
void
Cal_BddProfileMultiple(Cal_BddManager bddManager, Cal_Bdd *fUserBddArray,
                       long * levelCounts, int  negout)
{
  Cal_Bdd_t *f, *fArray;
  Cal_Bdd_t g;
  int i, j;

  Cal_Bdd userBdd;
  
  CalBddArrayPreProcessing(bddManager, fUserBddArray);

  for(i = 0; (userBdd = fUserBddArray[i]); ++i);

  fArray = Cal_MemAlloc(Cal_Bdd_t, i+1);
  for (j=0; j < i; j++){
    fArray[j] = CalBddGetInternalBdd(bddManager,fUserBddArray[j]);
  }
  fArray[j] = bddManager->bddNull;
    
  for(i = 0; i <=  bddManager->numVars; i++){
    levelCounts[i] = 0l;
  }
  g = CalBddOne(bddManager);
  CalBddPutMark(g, 0);
  for(f = fArray; !CalBddIsBddNull(bddManager, *f); ++f){
    BddMarkBdd(*f);
  }
  for(f = fArray; !CalBddIsBddNull(bddManager, *f); ++f){
    BddProfileStep(bddManager, *f, levelCounts, countingFns[!negout]);
  }
  Cal_MemFree(fArray);
}


/**Function********************************************************************

  Synopsis    [Returns a "function profile" for f.]

  Description [The nth entry of the function
  profile array is the number of subfunctions of f which may be obtained by 
  restricting the variables whose index is less than n.  An entry of zero 
  indicates that f is independent of the variable with the corresponding index.]

  SideEffects []

  SeeAlso     [optional]

******************************************************************************/
void
Cal_BddFunctionProfile(Cal_BddManager bddManager, Cal_Bdd fUserBdd,
                       long * funcCounts) 
{
  long i;
  Cal_BddIndex_t j;
  CalHashTable_t *h;
  Cal_Bdd_t f;
  
  /* The number of subfunctions obtainable by restricting the */
  /* variables of index < n is the number of subfunctions whose top */
  /* variable has index n plus the number of subfunctions obtainable */
  /* by restricting the variables of index < n+1 minus the number of */
  /* these latter subfunctions whose highest reference is by a node at */
  /* level n. */
  /* The strategy will be to start with the number of subfunctions */
  /* whose top variable has index n.  We compute the highest level at */
  /* which each subfunction is referenced.  Then we work bottom up; at */
  /* level n we add in the result from level n+1 and subtract the */
  /* number of subfunctions whose highest reference is at level n. */

  Cal_BddProfile(bddManager, fUserBdd, funcCounts, 0);
  if(CalBddPreProcessing(bddManager, 1, fUserBdd)){
    f = CalBddGetInternalBdd(bddManager, fUserBdd);
    /* Encode the profile.  The low bit of a count will be zero for */
    /* those levels where f actually has a node. */
    for(j = 0; j < bddManager->numVars; ++j){
      if(!funcCounts[j]){
	funcCounts[j] = 1;
      }
      else{
	funcCounts[j] <<= 1;
      }
    }
    h = CalHashTableOneInit(bddManager, sizeof(int));
    /* For each subfunction in f, compute the highest level where it is */
    /* referenced.  f itself is conceptually referenced at the highest */
    /* possible level, which we represent by -1. */
    i =  -1;
    CalHashTableOneInsert(h, f, (char *)&i);
    BddHighestRefStep(bddManager, f, h);
    /* Walk through these results.  For each subfunction, decrement the */
    /* count at the highest level where it is referenced. */
    BddDominatedStep(bddManager, f, funcCounts, h);
    CalHashTableOneQuit(h);
    /* Now add each level n+1 result to that of level n. */
    for(i = bddManager->numVars-1, j = i+1; i>=  0; --i){
      if(funcCounts[i] !=  1){
	funcCounts[i] = (funcCounts[i] >> 1) + funcCounts[j];
	j = i;
      }
      else{
	  funcCounts[i] = 0;
      }
    }
  }
}

/**Function********************************************************************

  Synopsis    [Returns a "function profile" for fArray.]

  Description [optional]

  SideEffects [None]

  SeeAlso     [optional]

******************************************************************************/
void
Cal_BddFunctionProfileMultiple(Cal_BddManager bddManager, Cal_Bdd
                               *fUserBddArray, long * funcCounts)
{
  long i;
  Cal_BddIndex_t j;
  Cal_Bdd_t *f, *fArray;
  CalHashTable_t *h;

  Cal_Bdd userBdd;
  
  CalBddArrayPreProcessing(bddManager, fUserBddArray);

  for(i = 0; (userBdd = fUserBddArray[i]); ++i);

  fArray = Cal_MemAlloc(Cal_Bdd_t, i+1);
  for (j=0; j < i; j++){
    fArray[j] = CalBddGetInternalBdd(bddManager,fUserBddArray[j]);
  }
  fArray[j] = bddManager->bddNull;

  /* See cmu_bdd_function_profile for the strategy involved here. */
  Cal_BddProfileMultiple(bddManager, fUserBddArray, funcCounts, 0);
  for(j = 0; j < bddManager->numVars; ++j){
    if(!funcCounts[j]){
      funcCounts[j] = 1;
    }
    else{
      funcCounts[j] <<= 1;
    }
  }
  h = CalHashTableOneInit(bddManager, sizeof(int));
  for(f = fArray; !CalBddIsBddNull(bddManager, *f); ++f){
    BddHighestRefStep(bddManager, *f, h);
  }
  i = -1;
  for(f = fArray; !CalBddIsBddNull(bddManager, *f); ++f){
    CalHashTableOneInsert(h, *f, (char *)&i);
  }
  for(f = fArray; !CalBddIsBddNull(bddManager, *f); ++f){
    BddDominatedStep(bddManager, *f, funcCounts, h);
  }
  CalHashTableOneQuit(h);
  for(i = bddManager->numVars-1, j = i+1; i >=  0; --i){
    if(funcCounts[i] !=  1){
      funcCounts[i] = (funcCounts[i] >> 1) + funcCounts[j];
      j = i;
    }
    else{
      funcCounts[i] = 0;
    }
  }
  Cal_MemFree(fArray);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddMarkBdd(Cal_Bdd_t  f)
{
  int currMarking, thisMarking;
  Cal_Bdd_t thenBdd, elseBdd;

  currMarking = CalBddGetMark(f);
  thisMarking = (1 << CalBddIsComplement(f));
  if(currMarking & thisMarking){
    return;
  }
  CalBddPutMark(f, currMarking | thisMarking);
  if(CalBddIsBddConst(f)){
    return;
  }
  CalBddGetThenBdd(f, thenBdd);
  BddMarkBdd(thenBdd);
  CalBddGetElseBdd(f, elseBdd);
  BddMarkBdd(elseBdd);
}


/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static int
BddCountNoNodes(
  Cal_Bdd_t  f)
{
  return (CalBddGetMark(f) > 0);
}


/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static int
BddCountNodes(
  Cal_Bdd_t  f)
{
  int mark;

  mark = CalBddGetMark(f);
  return (((mark & 0x1) !=  0) + ((mark & 0x2) !=  0));
}



/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static long
BddSizeStep(
  Cal_Bdd_t  f,
  CountFn_t countFn)
{
  long result;
  Cal_Bdd_t thenBdd, elseBdd;

  if(!CalBddGetMark(f)){
    return (0l);
  }
  result = (*countFn)(f);
  if(!CalBddIsBddConst(f)){
    CalBddGetThenBdd(f, thenBdd);
    CalBddGetElseBdd(f, elseBdd);
    result +=
        BddSizeStep(thenBdd, countFn) +
        BddSizeStep(elseBdd, countFn);
  }
  CalBddPutMark(f, 0);
  return result;
}



/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddProfileStep(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  f,
  long * levelCounts,
  CountFn_t countFn)
{
  Cal_Bdd_t thenBdd, elseBdd;
  if(!CalBddGetMark(f)){
    return;
  }
  if(CalBddIsBddConst(f)){
    levelCounts[bddManager->numVars] += (*countFn)(f);
  }
  else{
    levelCounts[CalBddGetBddIndex(bddManager, f)] += (*countFn)(f);
    CalBddGetThenBdd(f, thenBdd);
    BddProfileStep(bddManager, thenBdd, levelCounts, countFn);
    CalBddGetElseBdd(f, elseBdd);
    BddProfileStep(bddManager, elseBdd, levelCounts, countFn);
  }
  CalBddPutMark(f, 0);
}



/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddHighestRefStep(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  f,
  CalHashTable_t * h)
{
  int fIndex;
  Cal_Bdd_t keyBdd;
  int *dataPtr;

  if(CalBddIsBddConst(f)){
    return;
  }
  fIndex = CalBddGetBddIndex(bddManager, f);
  CalBddGetThenBdd(f, keyBdd);
  if(CalHashTableOneLookup(h, keyBdd, (char **)&dataPtr)){
    if(*dataPtr > fIndex){
      *dataPtr = fIndex;
    }
  }
  else{
    CalHashTableOneInsert(h, keyBdd, (char *)&fIndex);
    BddHighestRefStep(bddManager, keyBdd, h);
  }
  CalBddGetElseBdd(f, keyBdd);
  if(CalHashTableOneLookup(h, keyBdd, (char **)&dataPtr)){
    if(*dataPtr > fIndex){
      *dataPtr = fIndex;
    }
  }
  else{
    CalHashTableOneInsert(h, keyBdd, (char *)&fIndex);
    BddHighestRefStep(bddManager, keyBdd, h);
  }
}

/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddDominatedStep(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  f,
  long * funcCounts,
  CalHashTable_t * h)
{
  Cal_Bdd_t thenBdd, elseBdd;
  int *dataPtr;

  CalHashTableOneLookup(h, f, (char **)&dataPtr);
  if(*dataPtr >=  0)
    funcCounts[*dataPtr] -= 2;
  if(*dataPtr > -2){
    *dataPtr = -2;
    if(!CalBddIsBddConst(f)){
      CalBddGetThenBdd(f, thenBdd);
      BddDominatedStep(bddManager, thenBdd, funcCounts, h);
      CalBddGetElseBdd(f, elseBdd);
      BddDominatedStep(bddManager, elseBdd, funcCounts, h);
    }
  }
}









