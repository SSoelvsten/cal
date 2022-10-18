/**CFile***********************************************************************

  FileName    [calCacheTableTwo.c]

  PackageName [cal]

  Synopsis    [Functions to manage the Cache tables.]
  Description [ ]

  SeeAlso     [optional]

  Author      [ Rajeev K. Ranjan   (rajeev@eecs.berkeley.edu)
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

  Revision    [$Id: calCacheTableTwo.c,v 1.4 1998/09/15 19:02:52 ravi Exp $]

******************************************************************************/

#include "calInt.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/* cache table management related constants */
#define CACHE_TABLE_DEFAULT_SIZE_INDEX 16
#define CACHE_TABLE_DEFAULT_CACHE_RATIO 4

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct CacheEntryStruct CacheEntry_t;

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/
struct CalCacheTableStruct {
  long numBins;
  int sizeIndex;
  CacheEntry_t *bins;
  int cacheRatio;
  long numInsertions;
  long numEntries;
  long numHits;
  long numLookups;
  long numCollisions;
};

struct CacheEntryStruct {
  CalBddNode_t *operand1;
  CalBddNode_t *operand2;
  CalBddNode_t *resultBddNode;
  Cal_BddId_t resultBddId;
  unsigned short opCode;
};


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#ifdef USE_POWER_OF_2
#  define CacheTableTwoDoHash(table, operand1, operand2, opCode) \
   (((((((CalAddress_t)(operand1)) +	((CalAddress_t)(operand2))) / NODE_SIZE) << 2) + opCode + ((CalAddress_t)operand1 & 0x1)+ (((CalAddress_t)operand2 & 0x1)<<1)) &((table)->numBins - 1))
#else
#  define CacheTableTwoDoHash(table, operand1, operand2, opCode) \
   ((((((CalAddress_t)(operand1)) +	((CalAddress_t)(operand2))) / NODE_SIZE) + opCode + ((CalAddress_t)operand1 & 0x1)+ ((CalAddress_t)operand2 & 0x1)) %((table)->numBins))
/*(((((CalAddress_t)(operand1)) +	((CalAddress_t)(operand2))) + opCode) &((table)->numBins - 1))
((opCode + (((CalAddress_t)(operand1)) << 1) + (((CalAddress_t)(operand2)) <<2)) &((table)->numBins - 1))
((opCode + ((((CalAddress_t)(operand1)) + ((CalAddress_t)(operand2))) )+(((CalAddress_t)operand1 & 0x1) << (table->sizeIndex-1)) + (((CalAddress_t)operand2 & 0x1) << (table->sizeIndex-2))) &((table)->numBins - 1))
((opCode + (((CalAddress_t)(operand1)) << 1) + (((CalAddress_t)(operand2)) << 2)) &((table)->numBins - 1))
*/
#endif

#define CacheTableTwoCompareCacheEntry(entry, _operand1, _operand2, _opCode)  \
((((CalBddNode_t *)(((CalAddress_t)((entry)->operand1)) & ~0x2)) == (_operand1))\
 &&								\
 ((entry)->operand2 == (_operand2))\
 &&								\
 ((entry)->opCode == _opCode))

#define CacheResultNodeIsForwardedTo(resultBddNode, resultBddId) \
{ \
  CalBddNode_t *__resultBddNode;\
  __resultBddNode = CAL_BDD_POINTER(resultBddNode); \
  if(CalRequestNodeGetElseRequestNode(__resultBddNode) == FORWARD_FLAG){ \
    resultBddId = __resultBddNode->thenBddId; \
    resultBddNode = (CalBddNode_t*) \
                    (((CalAddress_t)(__resultBddNode->thenBddNode) & ~0xe)        \
                         ^(CAL_TAG0(resultBddNode))); \
  } \
}

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void CacheTableTwoRehash(CalCacheTable_t *cacheTable, int grow);
static void CacheTablePrint(CalCacheTable_t *cacheTable);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Initialize a Cache table using default parameters.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
CalCacheTable_t *
CalCacheTableTwoInit(Cal_BddManager_t *bddManager)
{
  CalCacheTable_t  *cacheTable;
  cacheTable = Cal_MemAlloc(CalCacheTable_t, 1);
  if (cacheTable == Cal_Nil(CalCacheTable_t)){
    CalBddFatalMessage("out of memory");
  }
  cacheTable->sizeIndex = CACHE_TABLE_DEFAULT_SIZE_INDEX;
  cacheTable->numBins = TABLE_SIZE(cacheTable->sizeIndex);
  cacheTable->cacheRatio = CACHE_TABLE_DEFAULT_CACHE_RATIO;
  cacheTable->bins = Cal_MemAlloc(CacheEntry_t, cacheTable->numBins);
  if(cacheTable->bins == Cal_Nil(CacheEntry_t)){
    CalBddFatalMessage("out of memory");
  }		
  memset((char *)cacheTable->bins, 0,
	 cacheTable->numBins*sizeof(CacheEntry_t));
  cacheTable->numInsertions = 0;
  cacheTable->numEntries = 0;
  cacheTable->numHits = 0;
  cacheTable->numLookups = 0;
  cacheTable->numCollisions = 0;
  return cacheTable;
}


/**Function********************************************************************

  Synopsis    [Free a Cache table along with the associated storage.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalCacheTableTwoQuit(CalCacheTable_t *cacheTable)
{
  if(cacheTable == Cal_Nil(CalCacheTable_t))return 1;
  Cal_MemFree(cacheTable->bins);
  Cal_MemFree(cacheTable);
  return 0;
}


/**Function********************************************************************

  Synopsis    [Directly insert a BDD node in the Cache table.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalCacheTableTwoInsert(Cal_BddManager_t *bddManager, Cal_Bdd_t f,
                       Cal_Bdd_t g, Cal_Bdd_t result, unsigned long
                       opCode, int cacheLevel)
{
  int hashValue;
  CalCacheTable_t *cacheTable;
  CacheEntry_t *bin;
  CalBddNode_t *operand1Node, *operand2Node;

  cacheTable = bddManager->cacheTable;
  cacheTable->numInsertions++;
  hashValue = CacheTableTwoDoHash(cacheTable, CalBddGetBddNode(f),
                                  CalBddGetBddNode(g), opCode); 

  bin = cacheTable->bins + hashValue;
  if (bin->opCode != CAL_OP_INVALID){
    cacheTable->numCollisions++;
  }
  else{
    cacheTable->numEntries++;
  }
  
  bin->opCode = opCode;
  if ((CalAddress_t)CalBddGetBddNode(f) >
      (CalAddress_t)CalBddGetBddNode(g)){ 
    operand1Node = CalBddGetBddNode(g);
    operand2Node = CalBddGetBddNode(f);
  }
  else{
    operand1Node = CalBddGetBddNode(f);
    operand2Node = CalBddGetBddNode(g);
  }
  
  if (cacheLevel){
  /*
   * Mark this result as temporary node to be forwarded at the end of
   * operation. The reason we can use this tagging is because the
   * size of the structure is 16 bytes and we are requiring 8 or 16 byte
   * alignment (at least last 3 bits should be zero).
   */
    bin->operand1 = (CalBddNode_t *) (((CalAddress_t)operand1Node) | 0x2);
  }
  else {
    bin->operand1 = operand1Node;
  }
  bin->operand2 = operand2Node;
  bin->resultBddNode = CalBddGetBddNode(result);
  bin->resultBddId = CalBddGetBddId(result);
  return;
}

  
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalCacheTableTwoLookup(Cal_BddManager_t *bddManager, Cal_Bdd_t f,
                       Cal_Bdd_t g, unsigned long opCode, Cal_Bdd_t
                       *resultBddPtr)  
{
  int hashValue;
  CalCacheTable_t *cacheTable;
  CacheEntry_t *bin;
  CalBddNode_t *operand1Node, *operand2Node;
  
  cacheTable = bddManager->cacheTable;
  cacheTable->numLookups++;
  hashValue = CacheTableTwoDoHash(cacheTable, CalBddGetBddNode(f),
                                  CalBddGetBddNode(g), opCode); 

  bin = cacheTable->bins+hashValue;
  
  if ((CalAddress_t)CalBddGetBddNode(f) > (CalAddress_t)CalBddGetBddNode(g)){
    operand1Node = CalBddGetBddNode(g);
    operand2Node = CalBddGetBddNode(f);
  }
  else{
    operand1Node = CalBddGetBddNode(f);
    operand2Node = CalBddGetBddNode(g);
  }
  if (CacheTableTwoCompareCacheEntry(bin, operand1Node, operand2Node,
                                     opCode)){
    CalBddPutBddId((*resultBddPtr), bin->resultBddId);
    CalBddPutBddNode((*resultBddPtr), bin->resultBddNode);
    cacheTable->numHits++;
    return 1;
  }
  *resultBddPtr = bddManager->bddNull;
  return 0;
}

/**Function********************************************************************

  Synopsis    [Free a Cache table along with the associated storage.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalCacheTableTwoFlush(CalCacheTable_t *cacheTable)
{
  memset((char *)cacheTable->bins, 0,
	 cacheTable->numBins*sizeof(CacheEntry_t));
  cacheTable->numEntries = 0;
}

/**Function********************************************************************

  Synopsis    [Free a Cache table along with the associated storage.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalCacheTableTwoFlushAll(CalCacheTable_t *cacheTable)
{
  CalCacheTableTwoFlush(cacheTable);
  cacheTable->numInsertions = 0;
  cacheTable->numCollisions = 0;
  cacheTable->numLookups = 0;
  cacheTable->numHits = 0;
  return 0;
}
/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalCacheTableTwoGCFlush(CalCacheTable_t *cacheTable)
{
  int i;
  CacheEntry_t *bin = cacheTable->bins;
  int numBins = cacheTable->numBins;
  if (cacheTable->numEntries == 0) return;
  for (i=0; i<numBins; bin++,i++){
    if (bin->opCode != CAL_OP_INVALID){
      if (CalBddNodeIsMarked((CAL_BDD_POINTER(bin->operand1))) ||
          CalBddNodeIsMarked((CAL_BDD_POINTER(bin->operand2))) ||
          CalBddNodeIsMarked((CAL_BDD_POINTER(bin->resultBddNode)))){
        /* This entry needs to be freed */
        cacheTable->numEntries--;
        memset((char *)bin, 0, sizeof(CacheEntry_t));
      }
    }
  }
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalCacheTableTwoRepackUpdate(CalCacheTable_t *cacheTable)
{
  int i;
  CacheEntry_t *bin = cacheTable->bins;
  int numBins = cacheTable->numBins;
  
  for (i=0; i<numBins; bin++,i++){
    if (bin->opCode != CAL_OP_INVALID){
      if (CalBddNodeIsForwarded(CAL_BDD_POINTER(bin->operand1))){
        CalBddNodeForward(bin->operand1);
      }
      if (CalBddNodeIsForwarded(CAL_BDD_POINTER(bin->operand2))){
        CalBddNodeForward(bin->operand2);
      }
      if (CalBddNodeIsForwarded(CAL_BDD_POINTER(bin->resultBddNode))){
        CalBddNodeForward(bin->resultBddNode);
      }
    }
  }
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalCheckCacheTableValidity(Cal_BddManager bddManager)
{
  CalCacheTable_t *cacheTable = bddManager->cacheTable;
  int i;
  CacheEntry_t *bin = cacheTable->bins;
  int numBins = cacheTable->numBins;
  
  for (i=0; i<numBins; bin++,i++){
    if (bin->opCode != CAL_OP_INVALID){
      Cal_Assert(CalBddNodeIsForwarded(CAL_BDD_POINTER(bin->operand1))
                 == 0);
      Cal_Assert(CalBddNodeIsForwarded(CAL_BDD_POINTER(bin->operand2))
                 == 0);
      Cal_Assert(CalBddNodeIsForwarded(CAL_BDD_POINTER(bin->resultBddNode))
                 == 0);
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
CalCacheTableTwoFixResultPointers(Cal_BddManager_t *bddManager)
{
  CalCacheTable_t *cacheTable = bddManager->cacheTable;
  int i;
  CacheEntry_t *bin = cacheTable->bins;
  int numBins = cacheTable->numBins;
  
  for (i=0; i<numBins; bin++,i++){
    if ((CalAddress_t)bin->operand1 & 0x2){ /* If the result node is temporary
                                   node */
      CacheResultNodeIsForwardedTo(bin->resultBddNode, bin->resultBddId);
      bin->operand1 = (CalBddNode_t *)((CalAddress_t)bin->operand1 &
                                       ~0x2); /* It is no longer temporary */
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
CalCacheTablePrint(Cal_BddManager_t *bddManager)
{
  CacheTablePrint(bddManager->cacheTable);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalBddManagerGetCacheTableData(Cal_BddManager_t *bddManager,
                               unsigned long *cacheSize,
                               unsigned long *cacheEntries,
                               unsigned long *cacheInsertions,
                               unsigned long *cacheLookups,
                               unsigned long *cacheHits,
                               unsigned long *cacheCollisions)
{
  CalCacheTable_t *cacheTable = bddManager->cacheTable;
  *cacheSize += cacheTable->numBins;
  *cacheEntries += cacheTable->numEntries;
  *cacheInsertions += cacheTable->numInsertions;
  *cacheLookups += cacheTable->numLookups;
  *cacheHits += cacheTable->numHits;
  *cacheCollisions += cacheTable->numCollisions;
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalCacheTableRehash(Cal_BddManager_t *bddManager)
{
  CalCacheTable_t *cacheTable = bddManager->cacheTable;
  if((3*cacheTable->numBins < cacheTable->cacheRatio*cacheTable->numEntries) &&
     (32*cacheTable->numBins <
      8*(bddManager->numNodes))){
    CacheTableTwoRehash(cacheTable, 1);
  }
}
/**Function********************************************************************

  Synopsis           [Flushes the entries from the cache which
                      correspond to the given associationId.]

  Description        []

  SideEffects        [Cache entries are affected.]

  SeeAlso            []

******************************************************************************/
void
CalCacheTableTwoFlushAssociationId(Cal_BddManager_t *bddManager, int
                                   associationId)
{
  CalCacheTable_t *cacheTable =   bddManager->cacheTable;
  int i;
  CacheEntry_t *bin;
  
  for (i=0; i < cacheTable->numBins; i++){
    bin = cacheTable->bins+i;
    if ((bin->opCode == (CAL_OP_QUANT+associationId)) ||
        (bin->opCode == (CAL_OP_REL_PROD+associationId)) ||
        (bin->opCode == (CAL_OP_VAR_SUBSTITUTE+associationId))){
      /* This entry needs to be freed */
      cacheTable->numEntries--;
      memset((char *)bin, 0, sizeof(CacheEntry_t));
    }
  }
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
unsigned long
CalCacheTableMemoryConsumption(CalCacheTable_t *cacheTable)
{
  return (unsigned long) (sizeof(cacheTable)+cacheTable->numBins*sizeof(CacheEntry_t));
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
CacheTableTwoRehash(CalCacheTable_t *cacheTable,int grow)
{
  CacheEntry_t *oldBins = cacheTable->bins;
  int i, hashValue;
  int oldNumBins = cacheTable->numBins;
  CacheEntry_t *bin, *newBin;
  
  
  if(grow){
    cacheTable->sizeIndex++;
  }
  else{
    if (cacheTable->sizeIndex <= CACHE_TABLE_DEFAULT_SIZE_INDEX){/* No need to Rehash */
      return;
    }
    cacheTable->sizeIndex--;
  }

  cacheTable->numBins = TABLE_SIZE(cacheTable->sizeIndex);
  cacheTable->bins = Cal_MemAlloc(CacheEntry_t, cacheTable->numBins);
  if(cacheTable->bins == Cal_Nil(CacheEntry_t)){
    CalBddFatalMessage("out of memory");
  }
  
  memset((char *)cacheTable->bins, 0, 
	 cacheTable->numBins*sizeof(CacheEntry_t));

  for(i = 0; i < oldNumBins; i++){
      bin  = oldBins+i;
      if (bin->opCode == CAL_OP_INVALID) continue;
      hashValue = CacheTableTwoDoHash(cacheTable,
                                      bin->operand1,
                                      bin->operand2,
                                      bin->opCode);
      newBin = cacheTable->bins+hashValue;
      if (newBin->opCode != CAL_OP_INVALID){
        cacheTable->numEntries--;
      }
      newBin->opCode = bin->opCode;
      newBin->operand1 = bin->operand1;
      newBin->operand2 = bin->operand2;
      newBin->resultBddId = bin->resultBddId;
      newBin->resultBddNode = bin->resultBddNode;
  }
  Cal_MemFree(oldBins);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CacheTablePrint(CalCacheTable_t *cacheTable)
{
  int i;
  unsigned long opCode;
  CacheEntry_t *bin;
  
  printf("cacheTable entries(%ld) bins(%ld)\n",
         cacheTable->numEntries, cacheTable->numBins);
  for(i = 0; i < cacheTable->numBins; i++){
    bin = cacheTable->bins+i;
    opCode = bin->opCode;
    if (opCode != CAL_OP_INVALID){
      fprintf(stdout,"Op = %s O1 = %lx, O2 = %lx RId = %d, RNode = %lx\n",
              ((opCode == CAL_OP_OR) ? "OR" : ((opCode == CAL_OP_AND) ? "AND" :
                                               ((opCode ==
                                                            CAL_OP_QUANT) ?
                                                           "QUANT" :
                                                           ((opCode ==
                                                             CAL_OP_REL_PROD)   
                                                            ?
                                                            "RELPROD"
                                                            :
                                                            "Nothing")))), 
              (CalAddress_t)bin->operand1,
              (CalAddress_t)bin->operand2, bin->resultBddId, 
              (CalAddress_t)bin->resultBddNode);
    }
  }
}


#ifdef CACHE_TABLE_TWO_TEST
main(int argc, char **argv)
{
  Cal_Bdd_t f1, f2, f3, f4, f5, result;
  Cal_BddManager_t *bddManager = Cal_BddManagerInit();
  int i;
  CalCacheTable_t *cacheTable;
  
  for (i=0; i<5; i++){
    Cal_BddManagerCreateNewVarLast(bddManager);
  }

  CalCacheTablePrint(bddManager);
  
  f1 = bddManager->varBdds[1];
  f2 = bddManager->varBdds[2];
  f3 = bddManager->varBdds[3];
  f4 = bddManager->varBdds[4];
  f5 = bddManager->varBdds[5];
  
  CalCacheTableTwoInsert(bddManager, f1, f2, f3, CAL_OP_OR, 0);
  CalCacheTableTwoInsert(bddManager, f3, f2, f4, CAL_OP_AND,0);
  CalCacheTableTwoInsert(bddManager, f3, f4, f5, CAL_OP_REL_PROD,0);
  /*CacheTableTwoRehash(bddManager->cacheTableArray[2], 1);*/
  CalCacheTablePrint(bddManager);
  
  /* Look up */
  CalCacheTableTwoLookup(bddManager, f3, f2, CAL_OP_AND, &result);
  assert(CalBddIsEqual(result, f4));

  CalCacheTableTwoLookup(bddManager, f3, f2, CAL_OP_OR, &result);
  assert(CalBddIsEqual(result, bddManager->bddNull));
  
  CalCacheTableTwoLookup(bddManager, f3, f1, CAL_OP_OR, &result);
  assert(CalBddIsEqual(result, bddManager->bddNull));

  /* Another look up */
  CalCacheTableTwoLookup(bddManager, f4, f3, CAL_OP_REL_PROD, &result);
  assert(CalBddIsEqual(result, f5));

  /* It will bump off the entry (f2, f2, AND, f4)*/
  CalCacheTableTwoInsert(bddManager, f3, f2, f1, CAL_OP_AND,0);
  /* Do lookup and see if that's what happened */
  CalCacheTableTwoLookup(bddManager, f3, f2, CAL_OP_AND, &result);
  assert(CalBddIsEqual(result, f1));

  /*
   * Rehashing will visit (f2, f3, AND, f4) first and then (f2, f3,
   * AND, f1)
   * Hence the we should have (f2, f3, AND, f1) in the first slot
   */
  CacheTableTwoRehash(bddManager->cacheTable, 1);
  CalCacheTableTwoLookup(bddManager, f3, f2, CAL_OP_AND, &result);
  assert(CalBddIsEqual(result, f1));
  Cal_BddManagerQuit(bddManager);
  
}
#endif
