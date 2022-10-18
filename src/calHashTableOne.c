/**CFile***********************************************************************

  FileName    [calHashTableOne.c]

  PackageName [cal]

  Synopsis    [Routines for managing hash table with Bdd is a key and
               int, long, or double as a value]

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

  Revision    [$Id: calHashTableOne.c,v 1.1.1.3 1998/05/04 00:58:57 hsv Exp $]

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
#  define HashTableOneDoHash(hashTable, keyBdd) \
     (((CalAddress_t)(CalBddGetBddNode(keyBdd)) / NODE_SIZE) & ((hashTable)->numBins - 1))
#else
#  define HashTableOneDoHash(hashTable, keyBdd) \
     (((CalAddress_t)(CalBddGetBddNode(keyBdd)) / NODE_SIZE) % hashTable->numBins)
#endif

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void HashTableOneRehash(CalHashTable_t * hashTable, int grow);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Initialize a hash table using default parameters.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
CalHashTable_t *
CalHashTableOneInit(Cal_BddManager_t * bddManager, int  itemSize)
{
  int i;
  CalHashTable_t *hashTable;

  hashTable = Cal_MemAlloc(CalHashTable_t, 1);
  if(hashTable == Cal_Nil(CalHashTable_t)){
    CalBddFatalMessage("out of memory");
  }
  hashTable->sizeIndex = HASH_TABLE_DEFAULT_SIZE_INDEX;
  hashTable->numBins = TABLE_SIZE(hashTable->sizeIndex);
  hashTable->maxCapacity = hashTable->numBins*HASH_TABLE_DEFAULT_MAX_DENSITY;
  hashTable->bins = Cal_MemAlloc(CalBddNode_t *, hashTable->numBins);
  if(hashTable->bins == Cal_Nil(CalBddNode_t *)){
    CalBddFatalMessage("out of memory");
  }
  for(i = 0; i < hashTable->numBins; i++){
    hashTable->bins[i] = Cal_Nil(CalBddNode_t);
  }
  hashTable->numEntries = 0;
  hashTable->bddId = (Cal_BddId_t)itemSize;
  if(itemSize > NODE_SIZE){
    CalBddFatalMessage("CalHashTableOneInit: itemSize exceeds NODE_SIZE");
  }
  hashTable->nodeManager = bddManager->nodeManagerArray[0];
  hashTable->requestNodeList = Cal_Nil(CalRequestNode_t);
  return hashTable;
}


/**Function********************************************************************

  Synopsis    [Free a hash table along with the associated storage.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalHashTableOneQuit(
  CalHashTable_t * hashTable)
{
  CalBddNode_t *ptr, *next, *node;
  int i;
  if(hashTable == Cal_Nil(CalHashTable_t))return;
  for(i = 0; i < hashTable->numBins; i++){
    ptr = hashTable->bins[i];
    while(ptr != Cal_Nil(CalBddNode_t)){
      next = CalBddNodeGetNextBddNode(ptr);
      node = CalBddNodeGetElseBddNode(ptr);
      CalNodeManagerFreeNode(hashTable->nodeManager, node);
      CalNodeManagerFreeNode(hashTable->nodeManager, ptr);
      ptr = next;
    }
  }
  Cal_MemFree(hashTable->bins);
  Cal_MemFree(hashTable);
}


/**Function********************************************************************

  Synopsis    [Directly insert a BDD node in the hash table.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalHashTableOneInsert(CalHashTable_t * hashTable, Cal_Bdd_t  keyBdd,
                      char * valuePtr)
{
  int hashValue;
  CalBddNode_t *bddNode, *dataPtr;

  hashValue = HashTableOneDoHash(hashTable, keyBdd);
  hashTable->numEntries++;
  if(hashTable->numEntries >= hashTable->maxCapacity){
    HashTableOneRehash(hashTable, 1);
    hashValue = HashTableOneDoHash(hashTable, keyBdd);
  }
  CalNodeManagerAllocNode(hashTable->nodeManager, dataPtr);
  memcpy(dataPtr, valuePtr, (size_t)hashTable->bddId);
  CalNodeManagerAllocNode(hashTable->nodeManager, bddNode);
  CalBddNodePutThenBdd(bddNode, keyBdd);
  CalBddNodePutElseBddNode(bddNode, dataPtr);
  CalBddNodePutNextBddNode(bddNode, hashTable->bins[hashValue]);
  hashTable->bins[hashValue] = bddNode;
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalHashTableOneLookup(CalHashTable_t * hashTable, Cal_Bdd_t  keyBdd,
                      char ** valuePtrPtr)
{
  CalBddNode_t *ptr;
  Cal_Bdd_t tmpBdd;
  int hashValue;
  
  hashValue = HashTableOneDoHash(hashTable, keyBdd);
  ptr = hashTable->bins[hashValue];
  while(ptr != Cal_Nil(CalBddNode_t)){
    CalBddNodeGetThenBdd(ptr, tmpBdd);
    if(CalBddIsEqual(keyBdd, tmpBdd)){
      if(valuePtrPtr){
        *valuePtrPtr = (char *)CalBddNodeGetElseBddNode(ptr);
      }
      return 1;
    }
    ptr = CalBddNodeGetNextBddNode(ptr);
  }
  if(valuePtrPtr){
    *valuePtrPtr = Cal_Nil(char);
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
static void
HashTableOneRehash(
  CalHashTable_t * hashTable,
  int  grow)
{
  CalBddNode_t *ptr, *next;
  CalBddNode_t **oldBins = hashTable->bins;
  int i, hashValue;
  int oldNumBins = hashTable->numBins;
  Cal_Bdd_t keyBdd;

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
      CalBddNodeGetThenBdd(ptr, keyBdd);
      hashValue = HashTableOneDoHash(hashTable, keyBdd);
      CalBddNodePutNextBddNode(ptr, hashTable->bins[hashValue]);
      hashTable->bins[hashValue] = ptr;
      ptr = next;
    }
  }
  Cal_MemFree(oldBins);
}










