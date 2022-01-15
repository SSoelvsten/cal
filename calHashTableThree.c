/**CFile***********************************************************************

  FileName    [calHashTableThree.c]

  PackageName [cal]

  Synopsis    [Functions to manage the hash tables that are a part of
                  ITE operation]

  Description [CalHashTableThreeFindOrAdd]

  SeeAlso     [optional]

  Author      [Jagesh Sanghavi (sanghavi@eecs.berkeley.edu)
                Rajeev Ranjan   (rajeev@eecs.berkeley.edu)]

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

  Revision    [$Id: calHashTableThree.c,v 1.1.1.3 1998/05/04 00:58:58 hsv Exp $]

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
#ifdef USE_POWER_OF_2
#define CalDoHash3(fBddNode, gBddNode, hBddNode,table) \
((((CalAddress_t)fBddNode + \
   (CalAddress_t)gBddNode + \
   (CalAddress_t) hBddNode) \
  / NODE_SIZE) & ((table)->numBins-1))
#else
#define CalDoHash3(fBddNode, gBddNode, hBddNode,table) \
  ((((CalAddress_t)fBddNode + \
  (CalAddress_t)gBddNode + \
  (CalAddress_t) hBddNode) \
  / NODE_SIZE)% table->numBins)
#endif
/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void CalHashTableThreeRehash(CalHashTable_t *hashTable, int grow);

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
int
CalHashTableThreeFindOrAdd(CalHashTable_t * hashTable,
                           Cal_Bdd_t  f,
                           Cal_Bdd_t  g,
                           Cal_Bdd_t  h,
                           Cal_Bdd_t * bddPtr)
{
  CalBddNode_t *ptr, *ptrIndirect;
  Cal_Bdd_t tmpBdd;
  int hashValue;
  
  hashValue = CalDoHash3(CalBddGetBddNode(f), 
      CalBddGetBddNode(g), CalBddGetBddNode(h), hashTable);
  ptr = hashTable->bins[hashValue];
  while(ptr != Cal_Nil(CalBddNode_t)){
    CalBddNodeGetThenBdd(ptr, tmpBdd);
    if(CalBddIsEqual(f, tmpBdd)){
      ptrIndirect = CalBddNodeGetElseBddNode(ptr);
      CalBddNodeGetThenBdd(ptrIndirect, tmpBdd);
      if(CalBddIsEqual(g, tmpBdd)){
        CalBddNodeGetElseBdd(ptrIndirect, tmpBdd);
        if(CalBddIsEqual(h, tmpBdd)){
          CalBddPutBddId(*bddPtr, hashTable->bddId);
          CalBddPutBddNode(*bddPtr, ptr);
          return 1;
        }
      }
    }
    ptr = CalBddNodeGetNextBddNode(ptr);
  }
  hashTable->numEntries++;
  if(hashTable->numEntries > hashTable->maxCapacity){
    CalHashTableThreeRehash(hashTable,1);
    hashValue = CalDoHash3(CalBddGetBddNode(f),
        CalBddGetBddNode(g), CalBddGetBddNode(h), hashTable);
  }
  CalNodeManagerAllocNode(hashTable->nodeManager, ptr);
  CalNodeManagerAllocNode(hashTable->nodeManager, ptrIndirect);
  CalBddNodePutThenBdd(ptr, f);
  CalBddNodePutThenBdd(ptrIndirect, g);
  CalBddNodePutElseBdd(ptrIndirect, h);
  CalBddNodePutElseBddNode(ptr, ptrIndirect);
  CalBddNodePutNextBddNode(ptr, hashTable->bins[hashValue]);
  hashTable->bins[hashValue] = ptr;
  CalBddPutBddId(*bddPtr, hashTable->bddId);
  CalBddPutBddNode(*bddPtr, ptr);
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
static void
CalHashTableThreeRehash(CalHashTable_t *hashTable, int grow)
{
  CalBddNode_t *ptr, *ptrIndirect, *next;
  CalBddNode_t **oldBins = hashTable->bins;
  int i, hashValue;
  int oldNumBins = hashTable->numBins;

  if(grow){
    hashTable->sizeIndex++;
  }
  else{
    if (hashTable->sizeIndex <= HASH_TABLE_DEFAULT_SIZE_INDEX){/* No need to rehash */
      return;
    }
    hashTable->sizeIndex--;
  }

  hashTable->numBins = TABLE_SIZE(hashTable->sizeIndex);
  hashTable->numBins = hashTable->numBins;
  hashTable->maxCapacity = hashTable->numBins * HASH_TABLE_DEFAULT_MAX_DENSITY;
  hashTable->bins = Cal_MemAlloc(CalBddNode_t *, hashTable->numBins);
  if(hashTable->bins == Cal_Nil(CalBddNode_t *)){
    CalBddFatalMessage("out of memory");
  }
  for(i = 0; i < hashTable->numBins; i++){
    hashTable->bins[i] = Cal_Nil(CalBddNode_t);
  }

  for(i = 0; i < oldNumBins; i++){
    ptr = oldBins[i];
    while(ptr != Cal_Nil(CalBddNode_t)){
      next = CalBddNodeGetNextBddNode(ptr);
      ptrIndirect = CalBddNodeGetElseBddNode(ptr);
      hashValue = CalDoHash3(CalBddNodeGetThenBddNode(ptr),
          CalBddNodeGetThenBddNode(ptrIndirect),
          CalBddNodeGetElseBddNode(ptrIndirect), hashTable);
      CalBddNodePutNextBddNode(ptr, hashTable->bins[hashValue]);
      hashTable->bins[hashValue] = ptr;
      ptr = next;
    }
  }
  Cal_MemFree(oldBins);
}








