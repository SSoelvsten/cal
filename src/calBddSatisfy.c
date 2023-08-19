/**CFile***********************************************************************

  FileName    [calBddSatisfy.c]

  PackageName [cal]

  Synopsis    [Routines for BDD satisfying valuation.]


  Description [ ]

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

  Revision    [$Id: calBddSatisfy.c,v 1.1.1.3 1998/05/04 00:58:53 hsv Exp $]

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

static Cal_Bdd_t BddSatisfyStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f);
static Cal_Bdd_t BddSatisfySupportStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_BddId_t * support);
static int IndexCmp(const void * p1, const void * p2);
static double BddSatisfyingFractionStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, CalHashTable_t * hashTable);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************
******************************************************************************/
Cal_Bdd
Cal_BddSatisfy(Cal_BddManager bddManager, Cal_Bdd  fUserBdd)
{
  Cal_Bdd_t f;
  if(CalBddPreProcessing(bddManager, 1, fUserBdd)){
    f = CalBddGetInternalBdd(bddManager, fUserBdd);
    if(CalBddIsBddZero(bddManager, f)){
      CalBddWarningMessage("Cal_BddSatisfy: argument is false");
      return (fUserBdd);
    }
    f = BddSatisfyStep(bddManager, f);
    return CalBddGetExternalBdd(bddManager, f);
  }
  return Cal_BddNull(bddManager);
}


/**Function********************************************************************
******************************************************************************/
Cal_Bdd
Cal_BddSatisfySupport(Cal_BddManager bddManager, Cal_Bdd fUserBdd)
{
  Cal_BddId_t *support, *p;
  long i;
  Cal_Bdd_t result;
  Cal_Bdd_t f;

  if(CalBddPreProcessing(bddManager, 1, fUserBdd)){
    f = CalBddGetInternalBdd(bddManager, fUserBdd);
    if(CalBddIsBddZero(bddManager, f)){
      CalBddWarningMessage("Cal_BddSatisfySupport: argument is false");
      return (fUserBdd);
    }
    support = Cal_MemAlloc(Cal_BddId_t, bddManager->numVars+1);
    for(i = 1, p = support; i <= bddManager->numVars; i++){
      if(!CalBddIsBddNull(bddManager,
          bddManager->currentAssociation->varAssociation[i])){
        *p = bddManager->idToIndex[i];
        ++p;
      }
    }
    *p = 0;
    qsort(support, (unsigned)(p - support), sizeof(Cal_BddId_t), IndexCmp);
    while(p != support){
      --p;
      *p = bddManager->indexToId[*p];
    }
    result = BddSatisfySupportStep(bddManager, f, support);
    Cal_MemFree(support);
    return CalBddGetExternalBdd(bddManager, result);
  }
  return Cal_BddNull(bddManager);
}

/**Function********************************************************************
******************************************************************************/
double
Cal_BddSatisfyingFraction(Cal_BddManager bddManager, Cal_Bdd fUserBdd)
{
  double fraction;
  CalHashTable_t *hashTable;
  Cal_Bdd_t f;
  if(CalBddPreProcessing(bddManager, 1, fUserBdd)){
    f = CalBddGetInternalBdd(bddManager, fUserBdd);
    hashTable = CalHashTableOneInit(bddManager, sizeof(double));
    fraction = BddSatisfyingFractionStep(bddManager, f, hashTable);
    CalHashTableOneQuit(hashTable);
    return fraction;
  }
  return 0.0;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************
  Returns a BDD which implies f, is true for some valuation on which f is true,
  and which has at most one node at each level
******************************************************************************/
static Cal_Bdd_t
BddSatisfyStep(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  f)
{
  Cal_Bdd_t tempBdd;
  Cal_Bdd_t result;

  if(CalBddIsBddConst(f)){
    return (f);
  }
  CalBddGetThenBdd(f, tempBdd);
  if(CalBddIsBddZero(bddManager, tempBdd)){
    CalBddGetElseBdd(f, tempBdd);
    tempBdd = BddSatisfyStep(bddManager, tempBdd);
    if(!CalUniqueTableForIdFindOrAdd(bddManager,
        bddManager->uniqueTable[CalBddGetBddId(f)],
        CalBddZero(bddManager), tempBdd, &result)){
      CalBddIcrRefCount(tempBdd);
    }
  }
  else{
    tempBdd = BddSatisfyStep(bddManager, tempBdd);
    if(!CalUniqueTableForIdFindOrAdd(bddManager,
        bddManager->uniqueTable[CalBddGetBddId(f)],
        tempBdd, CalBddZero(bddManager), &result)){
      CalBddIcrRefCount(tempBdd);
    }
  }
  return (result);
}


/**Function********************************************************************
******************************************************************************/
static Cal_Bdd_t
BddSatisfySupportStep(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  f,
  Cal_BddId_t * support)
{
  Cal_Bdd_t tempBdd;
  Cal_Bdd_t result;

  if(!*support){
    return BddSatisfyStep(bddManager, f);
  }
  if(CalBddGetBddIndex(bddManager, f) <= bddManager->idToIndex[*support]){
    if(CalBddGetBddId(f) == *support){
  ++support;
    }
    CalBddGetThenBdd(f, tempBdd);
    if(CalBddIsBddZero(bddManager, tempBdd)){
      CalBddGetElseBdd(f, tempBdd);
      tempBdd = BddSatisfySupportStep(bddManager, tempBdd, support);
      if(!CalUniqueTableForIdFindOrAdd(bddManager,
          bddManager->uniqueTable[CalBddGetBddId(f)],
          CalBddZero(bddManager), tempBdd, &result)){
        CalBddIcrRefCount(tempBdd);
      }
    }
    else{
      tempBdd = BddSatisfySupportStep(bddManager, tempBdd, support);
      if(!CalUniqueTableForIdFindOrAdd(bddManager,
          bddManager->uniqueTable[CalBddGetBddId(f)],
          tempBdd, CalBddZero(bddManager), &result)){
        CalBddIcrRefCount(tempBdd);
      }
    }
  }
  else{
    tempBdd = BddSatisfySupportStep(bddManager, f, support+1);
    if(!CalUniqueTableForIdFindOrAdd(bddManager,
        bddManager->uniqueTable[*support],
        CalBddZero(bddManager), tempBdd, &result)){
      CalBddIcrRefCount(tempBdd);
    }
  }
  return (result);
}


/**Function********************************************************************
******************************************************************************/
static int
IndexCmp(const void * p1, const void * p2)
{
  Cal_BddIndex_t i1, i2;

  i1 = *(Cal_BddId_t *)p1;
  i2 = *(Cal_BddId_t *)p2;
  if(i1 < i2){
    return (-1);
  }
  if(i1 > i2){
    return (1);
  }
  return (0);
}

/**Function********************************************************************
******************************************************************************/
static double
BddSatisfyingFractionStep(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  f,
  CalHashTable_t * hashTable)
{
  double *resultPtr, result;
  Cal_Bdd_t thenBdd, elseBdd;

  if(CalBddIsBddConst(f)){
    if(CalBddIsBddZero(bddManager, f)){
      return 0.0;
    }
    return 1.0;
  }
  if(CalHashTableOneLookup(hashTable, f, (char **)&resultPtr)){
    return (*resultPtr);
  }
  CalBddGetThenBdd(f, thenBdd);
  CalBddGetElseBdd(f, elseBdd);
  result =
      0.5 * BddSatisfyingFractionStep(bddManager, thenBdd, hashTable) +
      0.5 * BddSatisfyingFractionStep(bddManager, elseBdd, hashTable);
  CalHashTableOneInsert(hashTable, f, (char *)&result);
  return (result);
}
