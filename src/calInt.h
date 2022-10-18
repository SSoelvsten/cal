/**CHeaderFile*****************************************************************

  FileName    [calInt.h]

  PackageName [cal]

  Synopsis    [The internal data structures, macros and function declarations]

  Description []

  SeeAlso     [cal.h]

  Author      [Rajeev K. Ranjan (rajeev@ic.eecs.berkeley.edu
               Jagesh Sanghavi  (sanghavi@ic.eecs.berkeley.edu)]

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

  Revision    [$Id: calInt.h,v 1.9 1998/09/15 19:02:53 ravi Exp $]

******************************************************************************/

#ifndef _CALINT
#define _CALINT

#include "cal.h"

/* Make sure variable argument lists work */
#include <stdarg.h>

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/* Begin Performance Related Constants */

/* Garbage collection and reordering related constants */
/* The following constants could significantly affect the
   performance of the package */ 


#define CAL_MIN_GC_LIMIT 10000 /* minimum number of nodes in the unique table
                                  before performing garbage collection. It can
                                  be overridden by user define node limit */




#define CAL_REPACK_AFTER_GC_THRESHOLD 0.75 /* If the number of nodes fall below
                                           this factor after garbage
                                           collection, repacking should be
                                           done */ 
/* A note about repacking after garbage collection: Since repacking
** moves the node pointers, it is important that "user" does not have
** access to internal node pointers during such times. If for some
** purposes the node handle is needed and also there is a possibility of
** garbage collection being invoked, this field (repackAfterGCThreshold) of
** the bdd manager should be set to 0.
*/

#define CAL_TABLE_REPACK_THRESHOLD 0.9 /* If the page utility of a unique
                                            table (for some id) goes below this, repacking
                                            would be done */ 

#define CAL_BDD_REORDER_THRESHOLD 10000 /* Don't perform reordering below these
                                           many nodes */

#define CAL_NUM_PAGES_THRESHOLD 3

#define CAL_NUM_FORWARDED_NODES_LIMIT 50000 /* maximum number of forwarded nodes 
                                               allowed during BF reordering */

#define CAL_GC_CHECK 100       /* garbage collection check performed after
                                  addition of every GC_CHECK number of nodes to
                                  the unique table */

/* End Performance Related Constants */

/* Memory Management related constants */
#define NODE_SIZE sizeof(CalBddNode_t)		/* sizeof(CalBddNode_t) */

#ifndef PAGE_SIZE
#  define PAGE_SIZE 4096	/* size of a virtual memory page */
#endif
#ifndef LG_PAGE_SIZE
#  define LG_PAGE_SIZE 12	/* log2 of the page size */
#endif

#define NUM_NODES_PER_PAGE (PAGE_SIZE/NODE_SIZE)

#define MAX_NUM_SEGMENTS 32 
#define NUM_PAGES_PER_SEGMENT 64 /* We start with grabbing 64 pages at a time */
#define MIN_NUM_PAGES_PER_SEGMENT 4
#define MAX_NUM_PAGES 10

#define MIN_REC_SIZE CAL_ALLOC_ALIGNMENT
#define MAX_REC_SIZE (sizeof(CalHashTable_t)) /* size of hash table */
#define NUM_REC_MGRS (((MAX_REC_SIZE-MIN_REC_SIZE)/CAL_ALLOC_ALIGNMENT)+1)




/* true / false */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif




/* Error Codes */
#define CAL_BDD_OK 0
#define CAL_BDD_OVERFLOWED 1

/* bdd variable id and index related constants */
#define CAL_BDD_NULL_ID ((unsigned short) ((1 << 8*sizeof(unsigned short)) - 1))
#define CAL_BDD_CONST_ID 0
#define CAL_MAX_VAR_ID ((unsigned short) (CAL_BDD_NULL_ID - 1))
#define CAL_BDD_NULL_INDEX (unsigned short) ((1 << 8*sizeof(unsigned short)) - 1)
#define CAL_BDD_CONST_INDEX CAL_BDD_NULL_INDEX
#define CAL_MAX_VAR_INDEX (CAL_BDD_NULL_INDEX - 1)
#define CAL_MAX_REF_COUNT (unsigned short)((1 << 8*sizeof(char)) - 1)
#define CAL_INFINITY (1 << 20)

/* Pipeline related constants */
#define MAX_INSERT_DEPTH 256
#define PIPELINE_EXECUTION_DEPTH 1
#define DEFAULT_DEPTH 4
#define DEFAULT_MAX_DEPTH 6


#define FORWARD_FLAG 0     /* Flag used to identify redundant nodes */


/* Hash table management related constants. */
#define HASH_TABLE_DEFAULT_MAX_DENSITY 5
#define HASH_TABLE_DEFAULT_SIZE_INDEX 8
#define HASH_TABLE_DEFAULT_NUM_BINS TABLE_SIZE(HASH_TABLE_DEFAULT_SIZE_INDEX)
#define HASH_TABLE_DEFAULT_MAX_CAPACITY HASH_TABLE_DEFAULT_NUM_BINS*HASH_TABLE_DEFAULT_MAX_DENSITY
extern unsigned long calPrimes[];

#define USE_POWER_OF_2
#ifdef  USE_POWER_OF_2
#define TABLE_SIZE(sizeIndex) (1<<sizeIndex)
#else
#define TABLE_SIZE(sizeIndex) (calPrimes[sizeIndex])
#endif


/* Codes to be used in cache table */
#define  CAL_OP_INVALID 0x0000
#define  CAL_OP_OR 0x1000
#define  CAL_OP_AND 0x2000
#define  CAL_OP_NAND 0x3000
#define  CAL_OP_QUANT 0x4000
#define  CAL_OP_REL_PROD 0x5000
#define  CAL_OP_COMPOSE 0x6000
#define  CAL_OP_SUBST 0x7000
#define  CAL_OP_VAR_SUBSTITUTE 0x8000

#define  CAL_LARGE_BDD (1<<19) /* For smaller BDDs, we would
                                  use depth-first exist and forall routines */


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct Cal_BddStruct Cal_Bdd_t;
typedef struct CalBddNodeStruct CalBddNode_t;
typedef unsigned short Cal_BddRefCount_t;
typedef struct CalPageManagerStruct CalPageManager_t;
typedef struct CalNodeManagerStruct CalNodeManager_t;
typedef struct CalListStruct CalList_t;
typedef struct CalHashTableStruct CalHashTable_t;
typedef struct CalHashTableStruct *CalReqQueForId_t;
typedef struct CalHashTableStruct CalReqQueForIdAtDepth_t;
typedef struct CalAssociationStruct CalAssociation_t;
typedef struct CalBddNodeStruct CalRequestNode_t;
typedef struct Cal_BddStruct CalRequest_t;
typedef struct CalCacheTableStruct CalCacheTable_t;
typedef int (*CalBddNodeToIndexFn_t)(CalBddNode_t*, Cal_BddId_t);
typedef unsigned long CalAddress_t;
typedef struct Cal_BlockStruct Cal_Block_t;

struct Cal_BddStruct {
  Cal_BddId_t bddId;      /* variable id */
  CalBddNode_t *bddNode;  /* pointer to the bdd node */
};

typedef int (*CalOpProc_t) (Cal_BddManager, Cal_Bdd_t, Cal_Bdd_t, Cal_Bdd_t *); 
typedef int (*CalOpProc1_t) (Cal_BddManager, Cal_Bdd_t, Cal_Bdd_t *); 


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

enum CalPipeStateEnum { READY, CREATE, UPDATE };
typedef enum CalPipeStateEnum CalPipeState_t;

struct CalNodeManagerStruct{
  CalPageManager_t *pageManager;
  CalBddNode_t *freeNodeList;
  int numPages;
  int maxNumPages;
  CalAddress_t **pageList;
};

struct CalPageManagerStruct {
  CalAddress_t *freePageList;
  CalAddress_t **pageSegmentArray; /* Array of pointers to segments */
  int *numPagesArray; /* Number of pages in each segment */
  int numSegments;
  int totalNumPages; /* Total number of pages = sum of elements of numPagesArray */
  int numPagesPerSegment;
  int maxNumSegments;
};

struct CalBddNodeStruct {
  CalBddNode_t *nextBddNode; /* Attn: CalPageManagerFreePage overwrites this field of the node. Hence need to be aware of the consequences if the order of the fields is changed */

  CalBddNode_t *thenBddNode;
  CalBddNode_t *elseBddNode;
  Cal_BddId_t thenBddId;
  Cal_BddId_t elseBddId;
};

struct CalHashTableStruct {
  int sizeIndex;
  long numBins;
  long maxCapacity;
  CalBddNode_t **bins;
  Cal_BddId_t bddId;
  CalNodeManager_t *nodeManager;
  CalBddNode_t *requestNodeList;
 /* The following two fields are added to improve the performance of hash table clean up.*/
  CalBddNode_t startNode;
  CalBddNode_t *endNode;
  long numEntries;
};

struct CalAssociationStruct {
  Cal_Bdd_t *varAssociation;
  int lastBddIndex;
  int id;
  int refCount;
  CalAssociation_t *next;
};

struct Cal_BlockStruct
{
  long numChildren;
  Cal_Block_t **children;
  int reorderable;
  long firstIndex;
  long lastIndex;
};

/* Cal_BddManager_t - manages the BDD nodes */
struct Cal_BddManagerStruct {

  int numVars; /*
                * Number of BDD variables present in the manager. This does
                * not include the constant. The maximum number of variables 
                * manager can have is CAL_MAX_VAR_ID (as opposed to
                CAL_MAX_VAR_ID+1, id "0" being used for constant).
                * CAL_MAX_VAR_ID = (((1 << 16) - 1) -1 )
                */
  int maxNumVars; /* Maximum number of variables which can be created without
                     reallocating memory */

  Cal_Bdd_t *varBdds; /* Array of Cal_Bdd_t's. Cal_Bdd_t[i] is the BDD
                         corresponding to variable with id "i". */
  
  /* memory management */
  CalPageManager_t *pageManager1;  /* manages memory pages */
  CalPageManager_t *pageManager2;  /* manages memory pages */
  CalNodeManager_t **nodeManagerArray; /*
                                        * nodeManagerArray[i] is the node
                                        * manager for the variable with id = i.
                                        */     
  /* special nodes */
  Cal_Bdd_t bddOne; /* Constant: Id = 0; Index = CAL_MAX_INDEX */
  Cal_Bdd_t bddZero;
  Cal_Bdd_t bddNull;
  CalBddNode_t *userOneBdd;
  CalBddNode_t *userZeroBdd;

  Cal_BddId_t *indexToId; /*
                           * Table mapping index to id. If there are n
                           * variables, then this table has n entries from
                           * 0 to n-1 (indexToId[0] through indexToId[n-1]).
                           */
  Cal_BddIndex_t *idToIndex; /*
                              * Table mapping id to index:
                              * idToIndex[0] = CAL_MAX_INDEX
                              * If there are n variables in the manager, then
                              * corresponding to these variables this table
                              * has entries from 1 to n (idToIndex[1] through
                              * idToIndex[n]).
                              */
  
  CalHashTable_t **uniqueTable; /* uniqueTable[i] is the unique table for the
                                 * variable id " i". Unique table for an id is
                                 * a hash table of the nodes with that
                                 * variable id.
                                 */
  CalCacheTable_t *cacheTable; /* Computed table */

  /* Special functions */
  void (*TransformFn) (Cal_BddManager_t*, CalAddress_t, CalAddress_t,
       CalAddress_t*, CalAddress_t*, Cal_Pointer_t);
  Cal_Pointer_t transformEnv;

  /* logic operation management */
  CalHashTable_t ***reqQue;     /* reqQue[depth][id] is the hash table of
                                 * requests corresponding to variable id "id"
                                 * and the request depth "depth".
                                 */
                               
  /* Pipeline related information */
  int depth;
  int maxDepth;
  CalPipeState_t pipelineState;
  CalOpProc_t pipelineFn;
  int pipelineDepth;
  int currentPipelineDepth;
  CalRequestNode_t **requestNodeArray;/* Used for pipelined operations. */
  CalRequestNode_t *userProvisionalNodeList;   /* List of user BDD nodes
                                                  pointing to provisional
                                                  BDDs */
  CalRequestNode_t **requestNodeListArray;
  

  /* garbage collection related information */
  unsigned long numNodes;
  unsigned long numPeakNodes;
  unsigned long numNodesFreed;
  int gcCheck;
  unsigned long uniqueTableGCLimit;
  int  numGC;
  int gcMode;
  unsigned long nodeLimit;
  int overflow;
  float repackAfterGCThreshold;
  

  /* Association related stuff */
  CalAssociation_t *currentAssociation;
  CalAssociation_t *associationList;
  CalAssociation_t *tempAssociation;
  unsigned short tempOpCode; /* To store the id of temporary associations. */
  
  /* Variable reordering related stuff */
  long *interact; /* Interaction matrix */
  int dynamicReorderingEnableFlag;
  int reorderMethod;
  int reorderTechnique;
  long numForwardedNodes;
  int numReorderings;
  long maxNumVarsSiftedPerReordering;
  long numSwaps;
  long numTrivialSwaps;
  long maxNumSwapsPerReordering;
  double maxSiftingGrowth;
  long reorderingThreshold;
  long maxForwardedNodes;
  float tableRepackThreshold;
  Cal_Block superBlock; /* Variable blocks */


  void *hooks;
  int debugFlag;

  
};


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
#ifdef COMPUTE_MEMORY_OVERHEAD
long calNumEntriesAfterReduce, calNumEntriesAfterApply;
double calAfterReduceToAfterApplyNodesRatio, calAfterReduceToUniqueTableNodesRatio;
#endif

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/ 


#define CalNodeManagerAllocNode(nodeManager, node)                          \
{                                                                           \
  if((nodeManager)->freeNodeList != Cal_Nil(CalBddNode_t)){                 \
    node = nodeManager->freeNodeList;                                       \
    nodeManager->freeNodeList = ((CalBddNode_t *)(node))->nextBddNode;      \
    Cal_Assert(!((CalAddress_t)nodeManager->freeNodeList & 0xf));\
  }                                                                         \
  else{                                                                     \
    CalBddNode_t *_freeNodeList, *_nextNode, *_node;                        \
    _freeNodeList =                                                         \
        (CalBddNode_t *)CalPageManagerAllocPage(nodeManager->pageManager);  \
    for(_node = _freeNodeList + NUM_NODES_PER_PAGE - 1, _nextNode =0;       \
        _node != _freeNodeList; _nextNode = _node--){                       \
      _node->nextBddNode = _nextNode;                                       \
    }                                                                       \
    nodeManager->freeNodeList = _freeNodeList + 1;                          \
    node = _node;                                                           \
    if ((nodeManager)->numPages == (nodeManager)->maxNumPages){             \
      (nodeManager)->maxNumPages *= 2;                                      \
      (nodeManager)->pageList =                                            \
          Cal_MemRealloc(CalAddress_t *, (nodeManager)->pageList,          \
                         (nodeManager)->maxNumPages);                       \
    }                                                                       \
    (nodeManager)->pageList[(nodeManager)->numPages++] = (CalAddress_t *)_freeNodeList;    \
  }                                                                         \
  ((CalBddNode_t *)(node))->nextBddNode = 0;                                \
  ((CalBddNode_t *)(node))->thenBddId = 0;                                  \
  ((CalBddNode_t *)(node))->elseBddId = 0;                                  \
  ((CalBddNode_t *)(node))->thenBddNode = 0;                                \
  ((CalBddNode_t *)(node))->elseBddNode = 0;                                \
}

#define CalNodeManagerFreeNode(nodeManager, node) \
{ \
  (node)->nextBddNode = (nodeManager)->freeNodeList; \
  (nodeManager)->freeNodeList = node; \
}
#define CalNodeManagerInitBddNode(nodeManager, thenBdd, elseBdd, next, node) \
{ \
  if((nodeManager)->freeNodeList != Cal_Nil(CalBddNode_t)){ \
    node = nodeManager->freeNodeList; \
    nodeManager->freeNodeList = ((CalBddNode_t *)(node))->nextBddNode; \
    Cal_Assert(!((CalAddress_t)nodeManager->freeNodeList & 0xf));\
  } \
  else{ \
    CalBddNode_t *_freeNodeList, *_nextNode, *_node; \
    _freeNodeList = \
        (CalBddNode_t *)CalPageManagerAllocPage(nodeManager->pageManager); \
    for(_node = _freeNodeList + NUM_NODES_PER_PAGE - 1, _nextNode =0; \
        _node != _freeNodeList; _nextNode = _node--){ \
      _node->nextBddNode = _nextNode; \
    } \
    nodeManager->freeNodeList = _freeNodeList + 1; \
    node = _node; \
    if ((nodeManager)->numPages == (nodeManager)->maxNumPages){             \
      (nodeManager)->maxNumPages *= 2;                                      \
      (nodeManager)->pageList =                                            \
          Cal_MemRealloc(CalAddress_t *, (nodeManager)->pageList,          \
                         (nodeManager)->maxNumPages);                       \
    }                                                                       \
    (nodeManager)->pageList[(nodeManager)->numPages++] = (CalAddress_t *)_freeNodeList;    \
  } \
  ((CalBddNode_t *)(node))->nextBddNode = next; \
  ((CalBddNode_t *)(node))->thenBddId = CalBddGetBddId(thenBdd); \
  ((CalBddNode_t *)(node))->elseBddId = CalBddGetBddId(elseBdd); \
  ((CalBddNode_t *)(node))->thenBddNode = CalBddGetBddNode(thenBdd); \
  ((CalBddNode_t *)(node))->elseBddNode = CalBddGetBddNode(elseBdd); \
}

#define CalNodeManagerCreateAndDupBddNode(nodeManager, node, dupNode)\
{ \
  if((nodeManager)->freeNodeList != Cal_Nil(CalBddNode_t)){ \
    dupNode = nodeManager->freeNodeList; \
    nodeManager->freeNodeList = ((CalBddNode_t *)(dupNode))->nextBddNode; \
  } \
  else{ \
    CalBddNode_t *_freeNodeList, *_nextNode, *_node; \
    _freeNodeList = \
        (CalBddNode_t *)CalPageManagerAllocPage(nodeManager->pageManager); \
    for(_node = _freeNodeList + NUM_NODES_PER_PAGE - 1, _nextNode =0; \
        _node != _freeNodeList; _nextNode = _node--){ \
      _node->nextBddNode = _nextNode; \
    } \
    nodeManager->freeNodeList = _freeNodeList + 1; \
    dupNode = _node; \
    if ((nodeManager)->numPages == (nodeManager)->maxNumPages){             \
      (nodeManager)->maxNumPages *= 2;                                      \
      (nodeManager)->pageList =                                            \
          Cal_MemRealloc(CalAddress_t *, (nodeManager)->pageList,          \
                         (nodeManager)->maxNumPages);                       \
    }                                                                       \
    (nodeManager)->pageList[(nodeManager)->numPages++] = (CalAddress_t *)_freeNodeList;    \
  } \
  ((CalBddNode_t *)(dupNode))->nextBddNode = (node)->nextBddNode; \
  ((CalBddNode_t *)(dupNode))->thenBddId = (node)->thenBddId;\
  ((CalBddNode_t *)(dupNode))->elseBddId = (node)->elseBddId;\
  ((CalBddNode_t *)(dupNode))->thenBddNode = (node)->thenBddNode;\
  ((CalBddNode_t *)(dupNode))->elseBddNode = (node)->elseBddNode; \
}

/* Record manager size range stuff */

#define CAL_BDD_NEW_REC(bddManager, type) ((type *)Cal_MemNewRec((bddManager)->recordMgrArray[(CAL_ROUNDUP(sizeof(type))-MIN_REC_SIZE)/CAL_ALLOC_ALIGNMENT]))
#define CAL_BDD_FREE_REC(bddManager, rec, type) Cal_MemFreeRec((bddManager)->recordMgrArray[(CAL_ROUNDUP(sizeof(type))-MIN_REC_SIZE)/CAL_ALLOC_ALIGNMENT], (rec))

/*
** We would like to do repacking if :
** i) The id has more than minimum number of pages.
** ii) The ratio between the actual number of entries and the capacity is
**     less than a threshold.
*/
#define CalBddIdNeedsRepacking(bddManager, id)                              \
((bddManager->nodeManagerArray[id]->numPages > CAL_NUM_PAGES_THRESHOLD) && (bddManager->uniqueTable[id]->numEntries < bddManager->tableRepackThreshold *  \
  bddManager->nodeManagerArray[id]->numPages * NUM_NODES_PER_PAGE))


/*
 * Macros for managing Cal_Bdd_t and CalBddNode_t.
 * INTERNAL FUNCTIONS SHOULD NOT TOUCH THE INTERNAL FIELDS
 * FUNCTIONS IN calTerminal.c ARE EXCEPTION TO THIS GENERAL RULE
 *
 * {CalBdd} X {Get, Put} X {ThenBddId, ElseBddId, ThenBddNode, ElseBddNode,
 *     ThenBdd, ElseBdd, BddId, BddNode, NextBddNode}
 * {CalBdd} X {Get, Put, Icr, Dcr, Add} X {RefCount}
 * {CalBdd,CalBddNode} X {Get} X {BddIndex}
 * {CalBdd} X {Is} X {RefCountZero, OutPos, BddOne, BddZero, BddNull, BddConst} 
 * {CalBddManager} X {Get} X {BddZero, BddOne, BddNull}
 * {CalBddNode} X {Get, Put} X {ThenBddId, ElseBddId, ThenBddNode, ElseBddNode,
 *     ThenBdd, ElseBdd, NextBddNode}
 * {CalBddNode} X {Get, Put, Icr, Dcr, Add} X {RefCount}
 * {CalBddNode} X {Is} X {RefCountZero, OutPos}
 * {CalBddEqual, CalBddNodeEqual}
 *
 * {CalRequest} X {Get, Put} X {ThenRequestId, ElseRequestId, ThenRequestNode,
 *     ElseRequestNode, ThenRequest, ElseRequest, RequestId, RequestNode,
 *     F, G, Next}
 * {CalRequest} X {Is} X {Null}
 * {CalRequestNode} X {Get, Put} X {ThenRequestId, ElseRequestId,
 *     ThenRequestNode, ElseRequestNode, ThenRequest, ElseRequest, F, G, Next}
 */

#define CAL_BDD_POINTER(f) ((CalBddNode_t *)(((CalAddress_t)f) & ~0xf))
#define CAL_TAG0(pointer) ((int)((CalAddress_t)(pointer) & 0x1))
#define CalBddIsComplement(calBdd) CAL_TAG0((calBdd).bddNode)
#define CalBddUpdatePhase(calBdd, complement) \
    ((calBdd).bddNode = \
    (CalBddNode_t *)((CalAddress_t)((calBdd).bddNode) ^ complement))

#define CalBddZero(bddManager) ((bddManager)->bddZero)
#define CalBddOne(bddManager) ((bddManager)->bddOne)
#define CalBddNull(bddManager) ((bddManager)->bddNull)
#define CalBddIsBddConst(calBdd) ((calBdd).bddId == 0)
/* We are cheating here. Ideally we should compare both the id as well as the bdd node */
#define CalBddNodeIsBddNodeConst(bddNode) (((CalAddress_t)(bddNode)&~01) == ((CalAddress_t) bddManager->bddOne.bddNode & ~01))
#define CalBddIsEqual(calBdd1, calBdd2)\
    (((calBdd1).bddNode == (calBdd2).bddNode))
#define CalBddIsComplementEqual(calBdd1, calBdd2) \
    (((calBdd1).bddNode == \
    (CalBddNode_t *)(((CalAddress_t)(calBdd2).bddNode) ^ 0x1)))
#define CalBddSameOrNegation(calBdd1, calBdd2)	\
    (CAL_BDD_POINTER((calBdd1).bddNode) == CAL_BDD_POINTER((calBdd2).bddNode))

/* CAUTION: MACRO ASSUMES THAT THE INDEX CORRESPONDING TO varId IS LESS THAN OR
 * EQUAL TO THE INDEX OF calBdd */
#define CalBddGetCofactors(calBdd, varId, fx, fxbar) \
{ \
    if(varId == (calBdd).bddId){ \
      CalBddGetThenBdd(calBdd, fx); \
      CalBddGetElseBdd(calBdd, fxbar); \
    } \
    else{ \
      fx = calBdd; \
      fxbar = calBdd; \
    } \
}

#define CalBddGetThenBddId(calBdd) CAL_BDD_POINTER((calBdd).bddNode)->thenBddId
#define CalBddGetElseBddId(calBdd) CAL_BDD_POINTER((calBdd).bddNode)->elseBddId
#define CalBddGetThenBddIndex(bddManager, calBdd) \
    (bddManager->idToIndex[CAL_BDD_POINTER((calBdd).bddNode)->thenBddId])
#define CalBddGetElseBddIndex(bddManager, calBdd) \
    (bddManager->idToIndex[CAL_BDD_POINTER((calBdd).bddNode)->elseBddId])

#define CalBddGetThenBddNode(calBdd) \
    ((CalBddNode_t*) \
    (((CalAddress_t)(CAL_BDD_POINTER((calBdd).bddNode)->thenBddNode) \
    & ~0xe) ^ (CAL_TAG0((calBdd).bddNode))))

#define CalBddGetElseBddNode(calBdd) \
    ((CalBddNode_t*) \
    (((CalAddress_t)(CAL_BDD_POINTER((calBdd).bddNode)->elseBddNode) \
    & ~0xe) ^ (CAL_TAG0((calBdd).bddNode))))
 
#define CalBddGetThenBdd(calBdd, _thenBdd) \
{ \
  CalBddNode_t *_bddNode, *_bddNodeTagged; \
  _bddNodeTagged = (calBdd).bddNode; \
  _bddNode = CAL_BDD_POINTER(_bddNodeTagged); \
  (_thenBdd).bddId = _bddNode->thenBddId; \
  (_thenBdd).bddNode = (CalBddNode_t*) (((CalAddress_t) (_bddNode->thenBddNode) \
      & ~0xe)^(CAL_TAG0(_bddNodeTagged))); \
}

#define CalBddGetElseBdd(calBdd, _elseBdd) \
{ \
  CalBddNode_t *_bddNode, *_bddNodeTagged; \
  _bddNodeTagged = (calBdd).bddNode; \
  _bddNode = CAL_BDD_POINTER(_bddNodeTagged); \
  (_elseBdd).bddId = _bddNode->elseBddId; \
  (_elseBdd).bddNode = (CalBddNode_t*) (((CalAddress_t) (_bddNode->elseBddNode) \
        & ~0xe)^(CAL_TAG0(_bddNodeTagged)));\
}


#define CalBddGetBddId(calBdd) ((calBdd).bddId)
#define CalBddGetBddIndex(bddManager, calBdd) \
    (bddManager->idToIndex[(calBdd).bddId])
#define CalBddGetBddNode(calBdd) ((calBdd).bddNode)
#define CalBddGetBddNodeNot(calBdd) \
    ((CalBddNode_t*)(((CalAddress_t)((calBdd).bddNode))^0x1))

#define CalBddGetNextBddNode(calBdd) \
    ((CalBddNode_t *)(((CalAddress_t) \
    (CAL_BDD_POINTER((calBdd).bddNode)->nextBddNode)) & ~0xf))

#define CalBddPutThenBddId(calBdd, _thenBddId) \
    (CAL_BDD_POINTER((calBdd).bddNode)->thenBddId = _thenBddId)
#define CalBddPutElseBddId(calBdd, _elseBddId) \
    (CAL_BDD_POINTER((calBdd).bddNode)->elseBddId = _elseBddId)

#define CalBddPutThenBddNode(calBdd, _thenBddNode) \
{ \
  CalBddNode_t *_bddNode; \
  _bddNode = CAL_BDD_POINTER((calBdd).bddNode);	\
  _bddNode->thenBddNode = (CalBddNode_t*) \
      (((CalAddress_t)(_bddNode->thenBddNode) & 0xe)| \
      (((CalAddress_t) _thenBddNode) & ~0xe)); \
}

#define CalBddPutElseBddNode(calBdd, _elseBddNode) \
{ \
  CalBddNode_t *_bddNode; \
  _bddNode = CAL_BDD_POINTER((calBdd).bddNode);	\
  _bddNode->elseBddNode = (CalBddNode_t*) \
      (((CalAddress_t)(_bddNode->elseBddNode) & 0xe)| \
      (((CalAddress_t) _elseBddNode) & ~0xe));	\
}
 
#define CalBddPutThenBdd(calBdd, thenBdd) \
{ \
  CalBddNode_t *_bddNode; \
  _bddNode = CAL_BDD_POINTER((calBdd).bddNode);	\
  _bddNode->thenBddId = (thenBdd).bddId; \
  _bddNode->thenBddNode = (CalBddNode_t*) \
      (((CalAddress_t)(_bddNode->thenBddNode) & 0xe)| \
      (((CalAddress_t)(thenBdd).bddNode) & ~0xe)); \
}

#define CalBddPutElseBdd(calBdd, elseBdd) \
{ \
  CalBddNode_t *_bddNode; \
  _bddNode = CAL_BDD_POINTER((calBdd).bddNode);	\
  _bddNode->elseBddId = (elseBdd).bddId; \
  _bddNode->elseBddNode = (CalBddNode_t*) \
      (((CalAddress_t)(_bddNode->elseBddNode) & 0xe)| \
      (((CalAddress_t)(elseBdd).bddNode) & ~0xe)); \
}

#define CalBddPutBddId(calBdd, _bddId) ((calBdd).bddId = _bddId)
#define CalBddPutBddNode(calBdd, _bddNode) ((calBdd).bddNode = _bddNode)
#define CalBddPutNextBddNode(calBdd, _nextBddNode) \
{ \
  CalBddNode_t *_bddNode; \
  _bddNode = CAL_BDD_POINTER((calBdd).bddNode);	\
  _bddNode->nextBddNode = (CalBddNode_t*) \
      (((CalAddress_t)(_bddNode->nextBddNode) & 0xf)|  \
      (((CalAddress_t) _nextBddNode) & ~0xf));	 \
}

#define CalBddGetRefCount(calBdd, refCount) \
{ \
  CalBddNode_t *_bddNode; \
  _bddNode = CAL_BDD_POINTER((calBdd).bddNode);	\
  refCount = ((CalAddress_t)(_bddNode->thenBddNode) & 0x2); \
  refCount <<= 3; \
  refCount |= ((CalAddress_t)(_bddNode->elseBddNode) & 0xe); \
  refCount <<= 3; \
  refCount |= ((CalAddress_t)(_bddNode->nextBddNode) & 0xf); \
}
                                        
#define CalBddPutRefCount(calBdd, count) \
{ \
  Cal_BddRefCount_t _nextTag, _thenTag, _elseTag; \
  CalBddNode_t *_bddNode; \
  _bddNode = CAL_BDD_POINTER((calBdd).bddNode);	\
  _nextTag = (count & 0xf); \
  _thenTag = ((count >> 6) & 0x2); \
  _elseTag = ((count >> 3) & 0xe); \
  _bddNode->nextBddNode = (CalBddNode_t*) \
      ((((CalAddress_t)(_bddNode->nextBddNode)) & ~0xf) | _nextTag); \
  _bddNode->thenBddNode = (CalBddNode_t*) \
      ((((CalAddress_t)(_bddNode->thenBddNode)) & ~0x2) | _thenTag); \
  _bddNode->elseBddNode = (CalBddNode_t*) \
      ((((CalAddress_t)(_bddNode->elseBddNode)) & ~0xe) | _elseTag); \
}

#define CalBddIcrRefCount(calBdd) \
{ CalBddNode_t *_bddNode; \
  _bddNode = CAL_BDD_POINTER((calBdd).bddNode);	\
  if(((CalAddress_t)(_bddNode->nextBddNode) & 0xf) != 0xf){ \
    _bddNode->nextBddNode = \
        (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) + 1); \
  } \
  else{ \
    if(((CalAddress_t)(_bddNode->elseBddNode) & 0xe) != 0xe){ \
      _bddNode->nextBddNode = \
          (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) & ~0xf); \
      _bddNode->elseBddNode = \
          (CalBddNode_t *)((CalAddress_t)(_bddNode->elseBddNode) + 0x2); \
    } \
    else{ \
      if(((CalAddress_t)(_bddNode->thenBddNode) & 0x2) == 0){ \
        _bddNode->nextBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) & ~0xf); \
        _bddNode->elseBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->elseBddNode) & ~0xe); \
        _bddNode->thenBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->thenBddNode) | 0x2); \
      } \
    } \
  } \
}

#define CalBddDcrRefCount(calBdd) \
{ CalBddNode_t *_bddNode; \
  _bddNode = CAL_BDD_POINTER((calBdd).bddNode);	\
  if(((CalAddress_t)(_bddNode->nextBddNode) & 0xf) == 0x0){ \
    if(((CalAddress_t)(_bddNode->elseBddNode) & 0xe) == 0x0){ \
      if(((CalAddress_t)(_bddNode->thenBddNode) & 0x2) == 0x0){ \
        CalBddWarningMessage("Trying to decrement reference count below zero"); \
      } \
      else{ \
        _bddNode->thenBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->thenBddNode) & ~0x2); \
        _bddNode->elseBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->elseBddNode) | 0xe); \
        _bddNode->nextBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) | 0xf); \
      } \
    } \
    else{ \
      _bddNode->elseBddNode = \
          (CalBddNode_t *)((CalAddress_t)(_bddNode->elseBddNode) - 0x2); \
      _bddNode->nextBddNode = \
          (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) | 0xf); \
    } \
  } \
  else if(((CalAddress_t)(_bddNode->nextBddNode) & 0xf) != 0xf \
      ||  ((CalAddress_t)(_bddNode->elseBddNode) & 0xe) != 0xe  \
      ||  ((CalAddress_t)(_bddNode->thenBddNode) & 0x2) != 0x2){ \
    _bddNode->nextBddNode = \
        (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) - 1); \
  } \
}

#define CalBddAddRefCount(calBdd, num) \
{ \
  Cal_BddRefCount_t _count; \
  CalBddGetRefCount(calBdd, _count); \
  if(_count < CAL_MAX_REF_COUNT){ \
    _count += num; \
    if(_count > CAL_MAX_REF_COUNT){ \
      _count = CAL_MAX_REF_COUNT; \
    } \
    CalBddPutRefCount(calBdd, _count); \
  } \
}

#define CalBddIsRefCountZero(calBdd) \
    (((((CalAddress_t)(CAL_BDD_POINTER((calBdd).bddNode)->thenBddNode)) & 0x2) \
    || (((CalAddress_t)(CAL_BDD_POINTER((calBdd).bddNode)->elseBddNode)) & 0xe)\
    || (((CalAddress_t)(CAL_BDD_POINTER((calBdd).bddNode)->nextBddNode)) & 0xf))\
    ? 0 : 1)

#define CalBddIsRefCountMax(calBdd) \
    ((((((CalAddress_t)(CAL_BDD_POINTER((calBdd).bddNode)->thenBddNode)) & 0x2) == 0x2) \
    && ((((CalAddress_t)(CAL_BDD_POINTER((calBdd).bddNode)->elseBddNode)) & 0xe) == 0xe)\
    && ((((CalAddress_t)(CAL_BDD_POINTER((calBdd).bddNode)->nextBddNode)) & 0xf) == 0xf))\
    ? 1 : 0)

#define CalBddFree(calBdd) CalBddDcrRefCount(calBdd)

#define CalBddIsOutPos(calBdd)  (!(((CalAddress_t)(calBdd).bddNode) & 0x1))

#define CalBddIsBddOne(manager, calBdd) CalBddIsEqual(calBdd, (manager)->bddOne)
#define CalBddIsBddZero(manager, calBdd) CalBddIsEqual(calBdd, (manager)->bddZero)
#define CalBddIsBddNull(manager, calBdd) CalBddIsEqual(calBdd,(manager)->bddNull)
#define CalBddManagerGetBddOne(manager) ((manager)->bddOne)
#define CalBddManagerGetBddZero(manager) ((manager)->bddZero)
#define CalBddManagerGetBddNull(manager) (manager)->bddNull



#define CalBddGetMinId2(bddManager, calBdd1, calBdd2, topId) \
{ \
  Cal_BddId_t _id1, _id2; \
  Cal_BddIndex_t _index1, _index2; \
  _id1 = CalBddGetBddId((calBdd1)); \
  _id2 = CalBddGetBddId((calBdd2)); \
  _index1 = (bddManager)->idToIndex[_id1]; \
  _index2 = (bddManager)->idToIndex[_id2]; \
  if (_index1 < _index2) topId = _id1; \
  else topId = _id2; \
}

#define CalBddGetMinId3(bddManager, calBdd1, calBdd2, calBdd3, topId) \
{ \
  Cal_BddId_t _id1, _id2, _id3; \
  Cal_BddIndex_t _index1, _index2, _index3; \
  _id1 = CalBddGetBddId((calBdd1)); \
  _id2 = CalBddGetBddId((calBdd2)); \
  _id3 = CalBddGetBddId((calBdd3)); \
  _index1 = (bddManager)->idToIndex[_id1]; \
  _index2 = (bddManager)->idToIndex[_id2]; \
  _index3 = (bddManager)->idToIndex[_id3]; \
  if(_index1 <= _index2){ \
    if(_index1 <= _index3){ \
      topId = _id1; \
    } \
    else{ \
      topId = _id3; \
    } \
  } \
  else{ \
    if(_index2 <= _index3){ \
      topId = _id2; \
    } \
    else{ \
      topId = _id3; \
    } \
  } \
}

#define CalBddGetMinIndex2(bddManager, calBdd1, calBdd2, topIndex) \
{ \
  Cal_BddIndex_t _index1, _index2; \
  _index1 = bddManager->idToIndex[CalBddGetBddId(calBdd1)]; \
  _index2 = bddManager->idToIndex[CalBddGetBddId(calBdd2)]; \
  if (_index1 < _index2) topIndex = _index1; \
  else topIndex = _index2; \
}

#define CalBddGetMinIndex3(bddManager, calBdd1, calBdd2, calBdd3, topIndex) \
{ \
  Cal_BddId_t _id1, _id2, _id3; \
  Cal_BddIndex_t _index1, _index2, _index3; \
  _id1 = CalBddGetBddId((calBdd1)); \
  _id2 = CalBddGetBddId((calBdd2)); \
  _id3 = CalBddGetBddId((calBdd3)); \
  _index1 = (bddManager)->idToIndex[_id1]; \
  _index2 = (bddManager)->idToIndex[_id2]; \
  _index3 = (bddManager)->idToIndex[_id3]; \
  if(_index1 <= _index2){ \
    if(_index1 <= _index3){ \
      topIndex = _index1; \
    } \
    else{ \
      topIndex = _index3; \
    } \
  } \
  else{ \
    if(_index2 <= _index3){ \
      topIndex = _index2; \
    } \
    else{ \
      topIndex = _index3; \
    } \
  } \
}

#define CalBddGetMinIdAndMinIndex(bddManager, calBdd1, calBdd2, topId, topIndex)\
{ \
  Cal_BddId_t _id1, _id2; \
  Cal_BddIndex_t _index1, _index2; \
  _id1 = CalBddGetBddId((calBdd1)); \
  _id2 = CalBddGetBddId((calBdd2)); \
  _index1 = (bddManager)->idToIndex[_id1]; \
  _index2 = (bddManager)->idToIndex[_id2]; \
  if (_index1 < _index2){ \
    topId = _id1; \
    topIndex = _index1; \
  } \
  else { \
    topId = _id2; \
    topIndex = _index2; \
  } \
}

#define CalBddNot(calBdd1, calBdd2) \
{ \
  (calBdd2).bddId = (calBdd1).bddId; \
  (calBdd2).bddNode = (CalBddNode_t *)((CalAddress_t)(calBdd1).bddNode ^ 0x1); \
}

#define CAL_BDD_OUT_OF_ORDER(f, g) \
    ((CalAddress_t)CalBddGetBddNode(f) > (CalAddress_t)CalBddGetBddNode(g))

#define CAL_BDD_SWAP(f, g) \
{ \
  Cal_Bdd_t _tmp; \
  _tmp = f; \
  f = g; \
  g = _tmp; \
}


/* BddNode related Macros */
#define CalBddNodeGetThenBddId(_bddNode) ((_bddNode)->thenBddId)
#define CalBddNodeGetElseBddId(_bddNode) ((_bddNode)->elseBddId)
#define CalBddNodeGetThenBddIndex(bddManager, _bddNode) \
    bddManager->idToIndex[((_bddNode)->thenBddId)]
#define CalBddNodeGetElseBddIndex(bddManager, _bddNode) \
    bddManager->idToIndex[((_bddNode)->elseBddId)]
#define CalBddNodeGetThenBddNode(_bddNode) \
    ((CalBddNode_t *)((CalAddress_t)((_bddNode)->thenBddNode) & ~0xe))
#define CalBddNodeGetElseBddNode(_bddNode) \
    ((CalBddNode_t *)((CalAddress_t)((_bddNode)->elseBddNode) & ~0xe))
 
#define CalBddNodeGetThenBdd(_bddNode, _thenBdd) \
{ \
  (_thenBdd).bddId = (_bddNode)->thenBddId; \
  (_thenBdd).bddNode =  \
      (CalBddNode_t*) (((CalAddress_t) ((_bddNode)->thenBddNode) & ~0xe)); \
}

#define CalBddNodeGetElseBdd(_bddNode, _elseBdd) \
{ \
  (_elseBdd).bddId = (_bddNode)->elseBddId; \
  (_elseBdd).bddNode = \
      (CalBddNode_t*) (((CalAddress_t) ((_bddNode)->elseBddNode) & ~0xe)); \
}

#define CalBddNodeGetNextBddNode(_bddNode) \
    ((CalBddNode_t *)(((CalAddress_t) ((_bddNode)->nextBddNode)) & ~0xf))

#define CalBddNodePutThenBddId(_bddNode, _thenBddId) \
    ((_bddNode)->thenBddId = _thenBddId)

#define CalBddNodePutElseBddId(_bddNode, _elseBddId) \
    ((_bddNode)->elseBddId = _elseBddId)

#define CalBddNodePutThenBddNode(_bddNode, _thenBddNode) \
{ \
  (_bddNode)->thenBddNode = (CalBddNode_t*) \
      (((CalAddress_t)((_bddNode)->thenBddNode) & 0xe)| \
       (((CalAddress_t) _thenBddNode) & ~0xe));	\
}

#define CalBddNodePutElseBddNode(_bddNode, _elseBddNode) \
{ \
  (_bddNode)->elseBddNode = (CalBddNode_t*) \
      (((CalAddress_t)((_bddNode)->elseBddNode) & 0xe)| \
      (((CalAddress_t) _elseBddNode) & ~0xe));	\
}
 
#define CalBddNodePutThenBdd(_bddNode, _thenBdd) \
{ \
  (_bddNode)->thenBddId = (_thenBdd).bddId; \
  (_bddNode)->thenBddNode = (CalBddNode_t*) \
      (((CalAddress_t)((_bddNode)->thenBddNode) & 0xe)| \
       (((CalAddress_t)(_thenBdd).bddNode) & ~0xe)); \
}

#define CalBddNodePutElseBdd(_bddNode, _elseBdd) \
{ \
  (_bddNode)->elseBddId = (_elseBdd).bddId; \
  (_bddNode)->elseBddNode = (CalBddNode_t*) \
      (((CalAddress_t)((_bddNode)->elseBddNode) & 0xe)| \
       (((CalAddress_t) (_elseBdd).bddNode) & ~0xe)); \
}

#define CalBddNodePutNextBddNode(_bddNode, _nextBddNode) \
{ \
  (_bddNode)->nextBddNode = (CalBddNode_t*) \
      (((CalAddress_t)((_bddNode)->nextBddNode) & 0xf)|  \
       (((CalAddress_t) _nextBddNode) & ~0xf));	 \
}


#define CalBddNodeGetRefCount(_bddNode, refCount) \
{ \
  refCount = ((CalAddress_t)(_bddNode->thenBddNode) & 0x2); \
  refCount <<= 3; \
  refCount |= ((CalAddress_t)(_bddNode->elseBddNode) & 0xe); \
  refCount <<= 3; \
  refCount |= ((CalAddress_t)(_bddNode->nextBddNode) & 0xf); \
}
                                        
#define CalBddNodePutRefCount(_bddNode, count) \
{ \
  Cal_BddRefCount_t _nextTag, _thenTag, _elseTag; \
  _nextTag = (count & 0xf); \
  _thenTag = ((count >> 6) & 0x2); \
  _elseTag = ((count >> 3) & 0xe); \
  _bddNode->nextBddNode = (CalBddNode_t*) \
      ((((CalAddress_t)(_bddNode->nextBddNode)) & ~0xf) | _nextTag); \
  _bddNode->thenBddNode = (CalBddNode_t*) \
      ((((CalAddress_t)(_bddNode->thenBddNode)) & ~0x2) | _thenTag); \
  _bddNode->elseBddNode = (CalBddNode_t*) \
      ((((CalAddress_t)(_bddNode->elseBddNode)) & ~0xe) | _elseTag); \
}

#define CalBddNodeDcrRefCount(_bddNode) \
{ \
  if(((CalAddress_t)(_bddNode->nextBddNode) & 0xf) == 0x0){ \
    if(((CalAddress_t)(_bddNode->elseBddNode) & 0xe) == 0x0){ \
      if(((CalAddress_t)(_bddNode->thenBddNode) & 0x2) == 0x0){ \
        CalBddWarningMessage("Trying to decrement reference count below zero"); \
      } \
      else{ \
        _bddNode->thenBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->thenBddNode) & ~0x2); \
        _bddNode->elseBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->elseBddNode) | 0xe); \
        _bddNode->nextBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) | 0xf); \
      } \
    } \
    else{ \
      _bddNode->elseBddNode = \
          (CalBddNode_t *)((CalAddress_t)(_bddNode->elseBddNode) - 0x2); \
      _bddNode->nextBddNode = \
          (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) | 0xf); \
    } \
  } \
  else if(((CalAddress_t)(_bddNode->nextBddNode) & 0xf) != 0xf \
      ||  ((CalAddress_t)(_bddNode->elseBddNode) & 0xe) != 0xe  \
      ||  ((CalAddress_t)(_bddNode->thenBddNode) & 0x2) != 0x2){ \
    _bddNode->nextBddNode = \
        (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) - 1); \
  } \
}

#define CalBddNodeIcrRefCount(_bddNode) \
{ \
  if(((CalAddress_t)(_bddNode->nextBddNode) & 0xf) != 0xf){ \
    _bddNode->nextBddNode = \
        (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) + 1); \
  } \
  else{ \
    if(((CalAddress_t)(_bddNode->elseBddNode) & 0xe) != 0xe){ \
      _bddNode->nextBddNode = \
          (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) & ~0xf); \
      _bddNode->elseBddNode = \
          (CalBddNode_t *)((CalAddress_t)(_bddNode->elseBddNode) + 0x2); \
    } \
    else{ \
      if(((CalAddress_t)(_bddNode->thenBddNode) & 0x2) == 0){ \
        _bddNode->nextBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->nextBddNode) & ~0xf); \
        _bddNode->elseBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->elseBddNode) & ~0xe); \
        _bddNode->thenBddNode = \
            (CalBddNode_t *)((CalAddress_t)(_bddNode->thenBddNode) | 0x2); \
      } \
    } \
  } \
}

#define CalBddNodeAddRefCount(__bddNode, num)				\
{ \
  Cal_BddRefCount_t _count; \
  CalBddNodeGetRefCount(__bddNode, _count); \
  _count += num; \
  if(_count > CAL_MAX_REF_COUNT){ \
    _count = CAL_MAX_REF_COUNT; \
  } \
  CalBddNodePutRefCount(__bddNode, _count); \
}

#define CalBddNodeIsRefCountZero(_bddNode) \
    (((((CalAddress_t) ((_bddNode)->thenBddNode)) & 0x2) || \
    (((CalAddress_t) ((_bddNode)->elseBddNode)) & 0xe) || \
    (((CalAddress_t) ((_bddNode)->nextBddNode)) & 0xf)) \
    ? 0 : 1)

#define CalBddNodeIsRefCountMax(_bddNode) \
    ((((((CalAddress_t) ((_bddNode)->thenBddNode)) & 0x2) == 0x2)&& \
    ((((CalAddress_t) ((_bddNode)->elseBddNode)) & 0xe) == 0xe)&& \
    ((((CalAddress_t) ((_bddNode)->nextBddNode)) & 0xf) == 0xf)) \
    ? 1 : 0)

#define CalBddNodeIsOutPos(bddNode)  (!(((CalAddress_t)bddNode) & 0x1))
#define CalBddNodeRegular(bddNode) ((CalBddNode_t *)(((unsigned long)(bddNode)) & ~01))
#define CalBddRegular(calBdd1, calBdd2)                 \
{                                                       \
  calBdd2.bddId = calBdd1.bddId;                        \
  calBdd2.bddNode = CalBddNodeRegular(calBdd1.bddNode); \
}

#define CalBddNodeEqual(calBddNode1, calBddNode2)\
  ((CalAddress_t)calBddNode1 == (CalAddress_t)calBddNode2)

#define CalBddNodeNot(bddNode) ((CalBddNode_t*)(((CalAddress_t)(bddNode))^0x1))

/* Mark / Unmark */
#define CalBddIsMarked(calBdd) \
    CalBddNodeIsMarked(CAL_BDD_POINTER((calBdd).bddNode))

#define CalBddMark(calBdd) \
    CalBddNodeMark(CAL_BDD_POINTER((calBdd).bddNode))

#define CalBddUnmark(calBdd) \
    CalBddNodeUnmark(CAL_BDD_POINTER((calBdd).bddNode))

#define CalBddGetMark(calBdd) \
    CalBddNodeGetMark(CAL_BDD_POINTER((calBdd).bddNode))

#define CalBddPutMark(calBdd, mark) \
    CalBddNodePutMark(CAL_BDD_POINTER((calBdd).bddNode), (mark))

#define CalBddNodeIsMarked(bddNode) \
  ((((CalAddress_t)((bddNode)->thenBddNode)) & 0x4) >> 2)

#define CalBddNodeMark(bddNode) \
  ((bddNode)->thenBddNode = \
     (CalBddNode_t *)(((CalAddress_t)(bddNode)->thenBddNode) | 0x4))

#define CalBddNodeUnmark(bddNode) \
  ((bddNode)->thenBddNode = \
     (CalBddNode_t *)(((CalAddress_t)(bddNode)->thenBddNode) & ~0x4))

#define CalBddNodeGetMark(bddNode) \
  ((((CalAddress_t)((bddNode)->thenBddNode)) & 0xc) >> 2)

#define CalBddNodePutMark(bddNode, mark) \
  ((bddNode)->thenBddNode = (CalBddNode_t *) \
      ((((CalAddress_t)(bddNode)->thenBddNode) & ~0xc) | ((mark) << 2)))


/* THIS SHOULD BE CHANGED TO MACROS WITH ARGUMENTS */
#define CalRequestGetThenRequestId  CalBddGetThenBddId
#define CalRequestGetElseRequestId CalBddGetElseBddId
#define CalRequestGetThenRequestNode   CalBddGetThenBddNode
#define CalRequestGetElseRequestNode  CalBddGetElseBddNode
#define CalRequestGetThenRequest  CalBddGetThenBdd
#define CalRequestGetElseRequest  CalBddGetElseBdd
#define CalRequestGetRequestId  CalBddGetBddId
#define CalRequestGetRequestNode CalBddGetBddNode
#define CalRequestGetF CalBddGetThenBdd
#define CalRequestGetG CalBddGetElseBdd
#define CalRequestGetNextNode CalBddGetNextBddNode

#define CalRequestPutThenRequestId  CalBddPutThenBddId
#define CalRequestPutElseRequestId CalBddPutElseBddId
#define CalRequestPutThenRequestNode   CalBddPutThenBddNode
#define CalRequestPutElseRequestNode  CalBddPutElseBddNode
#define CalRequestPutThenRequest  CalBddPutThenBdd
#define CalRequestPutElseRequest  CalBddPutElseBdd
#define CalRequestPutRequestId  CalBddPutBddId
#define CalRequestPutRequestNode CalBddPutBddNode
#define CalRequestPutF CalBddPutThenBdd
#define CalRequestPutG CalBddPutElseBdd
#define CalRequestPutNextNode CalBddPutNextBddNode

/* Macros related to the CalRequestNode */

#define CalRequestNodeGetThenRequestId  CalBddNodeGetThenBddId
#define CalRequestNodeGetElseRequestId CalBddNodeGetElseBddId
#define CalRequestNodeGetThenRequestNode   CalBddNodeGetThenBddNode
#define CalRequestNodeGetElseRequestNode  CalBddNodeGetElseBddNode
#define CalRequestNodeGetThenRequest  CalBddNodeGetThenBdd
#define CalRequestNodeGetElseRequest  CalBddNodeGetElseBdd
#define CalRequestNodeGetF CalBddNodeGetThenBdd
#define CalRequestNodeGetG CalBddNodeGetElseBdd
#define CalRequestNodeGetNextRequestNode CalBddNodeGetNextBddNode

#define CalRequestNodePutThenRequestId  CalBddNodePutThenBddId
#define CalRequestNodePutElseRequestId CalBddNodePutElseBddId
#define CalRequestNodePutThenRequestNode   CalBddNodePutThenBddNode
#define CalRequestNodePutElseRequestNode  CalBddNodePutElseBddNode
#define CalRequestNodePutThenRequest  CalBddNodePutThenBdd
#define CalRequestNodePutElseRequest  CalBddNodePutElseBdd
#define CalRequestNodePutF CalBddNodePutThenBdd
#define CalRequestNodePutG CalBddNodePutElseBdd
#define CalRequestNodePutNextRequestNode CalBddNodePutNextBddNode
#define CalRequestIsNull(calRequest) \
    ((CalRequestGetRequestId(calRequest) == 0) \
    && (CalRequestGetRequestNode(calRequest) == 0))
                                      
#define CalRequestIsMarked CalBddIsMarked
#define CalRequestMark CalBddMark
#define CalRequestUnmark CalBddUnmark
#define CalRequestGetMark CalBddGetMark
#define CalRequestPutMark CalBddPutMark
#define CalRequestNodeIsMarked CalBddNodeIsMarked
#define CalRequestNodeMark CalBddNodeMark
#define CalRequestNodeUnmark CalBddNodeUnmark
#define CalRequestNodeGetMark CalBddNodeGetMark
#define CalRequestNodePutMark CalBddNodePutMark

#define CalRequestNodeGetCofactors(bddManager,requestNode,fx,fxbar,gx,gxbar) \
{ \
  Cal_Bdd_t __f, __g; \
  Cal_BddIndex_t __index1, __index2; \
  CalRequestNodeGetF(requestNode, __f); \
  CalRequestNodeGetG(requestNode, __g); \
  __index1 = (bddManager)->idToIndex[CalBddGetBddId(__f)]; \
  __index2 = (bddManager)->idToIndex[CalBddGetBddId(__g)]; \
  if(__index1 == __index2){ \
    CalBddGetThenBdd(__f, fx); \
    CalBddGetElseBdd(__f, fxbar); \
    CalBddGetThenBdd(__g, gx); \
    CalBddGetElseBdd(__g, gxbar); \
  } \
  else if(__index1 < __index2){ \
    CalBddGetThenBdd(__f, fx); \
    CalBddGetElseBdd(__f, fxbar); \
    gx = gxbar = __g; \
  } \
  else{ \
    fx = fxbar = __f; \
    CalBddGetThenBdd(__g, gx); \
    CalBddGetElseBdd(__g, gxbar); \
  } \
}

#define CalBddPairGetCofactors(bddManager,f,g,fx,fxbar,gx,gxbar) \
{ \
  Cal_BddIndex_t __index1, __index2; \
  __index1 = (bddManager)->idToIndex[CalBddGetBddId(f)]; \
  __index2 = (bddManager)->idToIndex[CalBddGetBddId(g)]; \
  if(__index1 == __index2){ \
    CalBddGetThenBdd(f, fx); \
    CalBddGetElseBdd(f, fxbar); \
    CalBddGetThenBdd(g, gx); \
    CalBddGetElseBdd(g, gxbar); \
  } \
  else if(__index1 < __index2){ \
    CalBddGetThenBdd(f, fx); \
    CalBddGetElseBdd(f, fxbar); \
    gx = gxbar = g; \
  } \
  else{ \
    fx = fxbar = f; \
    CalBddGetThenBdd(g, gx); \
    CalBddGetElseBdd(g, gxbar); \
  } \
}

#define CalBddIsForwarded(bdd) \
  (CAL_BDD_POINTER(CalBddGetElseBddNode(bdd)) == FORWARD_FLAG)

#define CalBddNodeIsForwarded(bddNode) \
  (((CalAddress_t)(CAL_BDD_POINTER(CalBddNodeGetElseBddNode(bddNode)))) == FORWARD_FLAG)

#define CalBddForward(bdd) \
{ \
  CalBddNode_t *_bddNode, *_bddNodeTagged; \
  _bddNodeTagged = CalBddGetBddNode(bdd); \
  _bddNode = CAL_BDD_POINTER(_bddNodeTagged); \
  (bdd).bddId = _bddNode->thenBddId; \
  (bdd).bddNode = (CalBddNode_t*) \
                  (((CalAddress_t)(_bddNode->thenBddNode) & ~0xe) \
                   ^(CAL_TAG0(_bddNodeTagged))); \
}

#define CalBddNodeForward(_bddNodeTagged) \
{ \
  CalBddNode_t *_bddNode; \
  _bddNode = CAL_BDD_POINTER(_bddNodeTagged); \
  _bddNodeTagged = (CalBddNode_t*) \
                  (((CalAddress_t)(_bddNode->thenBddNode) & ~0xe) \
                   ^(CAL_TAG0(_bddNodeTagged))); \
}

#define CalBddNodeIsForwardedTo(_bddNodeTagged) \
{ \
  CalBddNode_t *__bddNode;\
  __bddNode = CAL_BDD_POINTER(_bddNodeTagged); \
  if(CalBddNodeGetElseBddNode(__bddNode) == FORWARD_FLAG){ \
    _bddNodeTagged = (CalBddNode_t*) \
                     (((CalAddress_t)(__bddNode->thenBddNode) & ~0xe)        \
                      ^(CAL_TAG0(_bddNodeTagged))); \
  } \
}

#define CalRequestIsForwardedTo(request) \
{ \
  CalBddNode_t *__bddNode, *__bddNodeTagged; \
  __bddNodeTagged = (request).bddNode; \
  __bddNode = CAL_BDD_POINTER(__bddNodeTagged); \
  if(CalRequestNodeGetElseRequestNode(__bddNode) == FORWARD_FLAG){ \
    (request).bddId = __bddNode->thenBddId; \
    (request).bddNode = (CalBddNode_t*) \
                        (((CalAddress_t)(__bddNode->thenBddNode) & ~0xe)        \
                         ^(CAL_TAG0(__bddNodeTagged))); \
  } \
}

#define CalBddIsForwardedTo CalRequestIsForwardedTo

#define CalBddNormalize(fBdd, gBdd) \
{ \
  Cal_Bdd_t _tmpBdd; \
  if((unsigned long)CAL_BDD_POINTER(CalBddGetBddNode(gBdd)) < \
      (unsigned long)CAL_BDD_POINTER(CalBddGetBddNode(fBdd))){ \
    _tmpBdd = fBdd; \
    fBdd = gBdd; \
    gBdd = _tmpBdd; \
  } \
}

/* Depth aliased as RefCount */
#define CalBddGetDepth CalBddGetRefCount
#define CalBddPutDepth CalBddPutRefCount
#define CalRequestNodeGetDepth CalBddNodeGetRefCount
#define CalRequestNodeGetRefCount CalBddNodeGetRefCount
#define CalRequestNodeAddRefCount CalBddNodeAddRefCount
#define CalRequestAddRefCount CalBddAddRefCount
#define CalRequestNodePutDepth CalBddNodePutRefCount

#define CalITERequestNodeGetCofactors(bddManager, requestNode, fx, fxbar, gx, gxbar, hx, hxbar) \
{ \
  Cal_Bdd_t __f, __g, __h; \
  Cal_BddIndex_t __index1, __index2, __index3; \
  CalBddNode_t *__ptrIndirect; \
  CalRequestNodeGetThenRequest(requestNode, __f); \
  __ptrIndirect = CalRequestNodeGetElseRequestNode(requestNode); \
  CalRequestNodeGetThenRequest(__ptrIndirect, __g); \
  CalRequestNodeGetElseRequest(__ptrIndirect, __h); \
  __index1 = (bddManager)->idToIndex[CalBddGetBddId(__f)]; \
  __index2 = (bddManager)->idToIndex[CalBddGetBddId(__g)]; \
  __index3 = (bddManager)->idToIndex[CalBddGetBddId(__h)]; \
  if(__index1 == __index2){ \
    if(__index3 == __index1){ \
      CalBddGetThenBdd(__f, fx); \
      CalBddGetElseBdd(__f, fxbar); \
      CalBddGetThenBdd(__g, gx); \
      CalBddGetElseBdd(__g, gxbar); \
      CalBddGetThenBdd(__h, hx); \
      CalBddGetElseBdd(__h, hxbar); \
    } \
    else if(__index3 < __index1){ \
      fx = fxbar = __f; \
      gx = gxbar = __g; \
      CalBddGetThenBdd(__h, hx); \
      CalBddGetElseBdd(__h, hxbar); \
    } \
    else{ \
      CalBddGetThenBdd(__f, fx); \
      CalBddGetElseBdd(__f, fxbar); \
      CalBddGetThenBdd(__g, gx); \
      CalBddGetElseBdd(__g, gxbar); \
      hx = hxbar = __h; \
    } \
  } \
  else if(__index1 < __index2){ \
    if(__index3 == __index1){ \
      CalBddGetThenBdd(__f, fx); \
      CalBddGetElseBdd(__f, fxbar); \
      gx = gxbar = __g; \
      CalBddGetThenBdd(__h, hx); \
      CalBddGetElseBdd(__h, hxbar); \
    } \
    else if(__index3 < __index1){ \
      fx = fxbar = __f; \
      gx = gxbar = __g; \
      CalBddGetThenBdd(__h, hx); \
      CalBddGetElseBdd(__h, hxbar); \
    } \
    else{ \
      CalBddGetThenBdd(__f, fx); \
      CalBddGetElseBdd(__f, fxbar); \
      gx = gxbar = __g; \
      hx = hxbar = __h; \
    } \
  } \
  else{ \
    if(__index3 == __index2){ \
      fx = fxbar = __f; \
      CalBddGetThenBdd(__g, gx); \
      CalBddGetElseBdd(__g, gxbar); \
      CalBddGetThenBdd(__h, hx); \
      CalBddGetElseBdd(__h, hxbar); \
    } \
    else if(__index3 < __index2){ \
      fx = fxbar = __f; \
      gx = gxbar = __g; \
      CalBddGetThenBdd(__h, hx); \
      CalBddGetElseBdd(__h, hxbar); \
    } \
    else{ \
      fx = fxbar = __f; \
      CalBddGetThenBdd(__g, gx); \
      CalBddGetElseBdd(__g, gxbar); \
      hx = hxbar = __h; \
    } \
  } \
}


#define CalCacheTableOneInsert(bddManager, f, result, opCode, cacheLevel) CalCacheTableTwoInsert(bddManager, f, (bddManager)->bddOne, result, opCode, cacheLevel)

#define CalCacheTableOneLookup(bddManager, f, opCode, resultPtr) CalCacheTableTwoLookup(bddManager, f, (bddManager)->bddOne, opCode, resultPtr)

#ifdef USE_POWER_OF_2
#define CalDoHash2(thenBddNode, elseBddNode, table) \
   (((((CalAddress_t)thenBddNode) + ((CalAddress_t)elseBddNode)) / NODE_SIZE) & ((table)->numBins - 1))
#else
#define CalDoHash2(thenBddNode, elseBddNode, table) \
                              (((((CalAddress_t)thenBddNode) + \
                                 ((CalAddress_t)elseBddNode)) / NODE_SIZE)% \
                               (table)->numBins)
#endif

EXTERN int CalBddPreProcessing(Cal_BddManager_t *bddManager, int count, ...);

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN Cal_Bdd_t CalBddIf(Cal_BddManager bddManager, Cal_Bdd_t F);
EXTERN int CalBddIsCubeStep(Cal_BddManager bddManager, Cal_Bdd_t f);
EXTERN int CalBddTypeAux(Cal_BddManager_t * bddManager, Cal_Bdd_t f);
EXTERN Cal_Bdd_t CalBddIdentity(Cal_BddManager_t *bddManager, Cal_Bdd_t calBdd);
EXTERN void CalHashTableApply(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, CalHashTable_t ** reqQueAtPipeDepth, CalOpProc_t calOpProc);
EXTERN void CalHashTableReduce(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, CalHashTable_t * uniqueTableForId);
EXTERN void CalAssociationListFree(Cal_BddManager_t * bddManager);
EXTERN void CalVarAssociationRepackUpdate(Cal_BddManager_t * bddManager, Cal_BddId_t id);
EXTERN void CalCheckAssociationValidity(Cal_BddManager_t * bddManager);
EXTERN void CalReorderAssociationFix(Cal_BddManager_t *bddManager);
EXTERN void CalRequestNodeListCompose(Cal_BddManager_t * bddManager, CalRequestNode_t * requestNodeList, Cal_BddIndex_t composeIndex);
EXTERN void CalHashTableComposeApply(Cal_BddManager_t *bddManager, CalHashTable_t *hashTable, Cal_BddIndex_t gIndex, CalHashTable_t **reqQueForCompose, CalHashTable_t **reqQueForITE);
EXTERN void CalComposeRequestCreate(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_Bdd_t h, Cal_BddIndex_t composeIndex, CalHashTable_t **reqQueForCompose, CalHashTable_t **reqQueForITE, Cal_Bdd_t *resultPtr);
EXTERN void CalRequestNodeListArrayITE(Cal_BddManager_t *bddManager, CalRequestNode_t **requestNodeListArray);
EXTERN Cal_Bdd_t CalBddOpITEBF(Cal_BddManager_t *bddManager, Cal_Bdd_t f, Cal_Bdd_t g, Cal_Bdd_t h);
EXTERN void CalHashTableITEApply(Cal_BddManager_t *bddManager, CalHashTable_t *hashTable, CalHashTable_t **reqQueAtPipeDepth);
EXTERN Cal_Bdd_t CalBddITE(Cal_BddManager_t *bddManager, Cal_Bdd_t F, Cal_Bdd_t G, Cal_Bdd_t H);
EXTERN Cal_Bdd_t CalBddManagerCreateNewVar(Cal_BddManager_t * bddManager, Cal_BddIndex_t index);
EXTERN void CalRequestNodeListArrayOp(Cal_BddManager_t * bddManager, CalRequestNode_t ** requestNodeListArray, CalOpProc_t calOpProc);
EXTERN Cal_Bdd_t CalBddOpBF(Cal_BddManager_t * bddManager, CalOpProc_t calOpProc, Cal_Bdd_t F, Cal_Bdd_t G);
EXTERN Cal_Bdd_t CalBddVarSubstitute(Cal_BddManager bddManager, Cal_Bdd_t f, unsigned short opCode, CalAssociation_t *assoc);
EXTERN int CalOpBddVarSubstitute(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_Bdd_t * resultBddPtr);
EXTERN long CalBddFindBlock(Cal_Block block, long index);
EXTERN void CalBddBlockDelta(Cal_Block b, long delta);
EXTERN Cal_Block CalBddShiftBlock(Cal_BddManager_t *bddManager, Cal_Block b, long index);
EXTERN unsigned long CalBlockMemoryConsumption(Cal_Block block);
EXTERN void CalFreeBlockRecursively(Cal_Block block);
EXTERN CalCacheTable_t * CalCacheTableTwoInit(Cal_BddManager_t *bddManager);
EXTERN int CalCacheTableTwoQuit(CalCacheTable_t *cacheTable);
EXTERN void CalCacheTableTwoInsert(Cal_BddManager_t *bddManager, Cal_Bdd_t f, Cal_Bdd_t g, Cal_Bdd_t result, unsigned long opCode, int cacheLevel);
EXTERN int CalCacheTableTwoLookup(Cal_BddManager_t *bddManager, Cal_Bdd_t f, Cal_Bdd_t g, unsigned long opCode, Cal_Bdd_t *resultBddPtr);
EXTERN void CalCacheTableTwoFlush(CalCacheTable_t *cacheTable);
EXTERN int CalCacheTableTwoFlushAll(CalCacheTable_t *cacheTable);
EXTERN void CalCacheTableTwoGCFlush(CalCacheTable_t *cacheTable);
EXTERN void CalCacheTableTwoRepackUpdate(CalCacheTable_t *cacheTable);
EXTERN void CalCheckCacheTableValidity(Cal_BddManager bddManager);
EXTERN void CalCacheTableTwoFixResultPointers(Cal_BddManager_t *bddManager);
EXTERN void CalCacheTablePrint(Cal_BddManager_t *bddManager);
EXTERN void CalBddManagerGetCacheTableData(Cal_BddManager_t *bddManager, unsigned long *cacheSize, unsigned long *cacheEntries, unsigned long *cacheInsertions, unsigned long *cacheLookups, unsigned long *cacheHits, unsigned long *cacheCollisions);
EXTERN void CalCacheTableRehash(Cal_BddManager_t *bddManager);
EXTERN void CalCacheTableTwoFlushAssociationId(Cal_BddManager_t *bddManager, int associationId);
EXTERN unsigned long CalCacheTableMemoryConsumption(CalCacheTable_t *cacheTable);
EXTERN void CalBddManagerGCCheck(Cal_BddManager_t * bddManager);
EXTERN int CalHashTableGC(Cal_BddManager_t *bddManager, CalHashTable_t *hashTable);
EXTERN void CalRepackNodesAfterGC(Cal_BddManager_t *bddManager);
EXTERN CalHashTable_t * CalHashTableInit(Cal_BddManager_t *bddManager, Cal_BddId_t bddId);
EXTERN int CalHashTableQuit(Cal_BddManager_t *bddManager, CalHashTable_t * hashTable);
EXTERN void CalHashTableAddDirect(CalHashTable_t * hashTable, CalBddNode_t * bddNode);
EXTERN int CalHashTableFindOrAdd(CalHashTable_t * hashTable, Cal_Bdd_t thenBdd, Cal_Bdd_t elseBdd, Cal_Bdd_t * bddPtr);
EXTERN int CalHashTableAddDirectAux(CalHashTable_t * hashTable, Cal_Bdd_t thenBdd, Cal_Bdd_t elseBdd, Cal_Bdd_t * bddPtr);
EXTERN void CalHashTableCleanUp(CalHashTable_t * hashTable);
EXTERN int CalHashTableLookup(CalHashTable_t * hashTable, Cal_Bdd_t thenBdd, Cal_Bdd_t elseBdd, Cal_Bdd_t * bddPtr);
EXTERN void CalHashTableDelete(CalHashTable_t * hashTable, CalBddNode_t * bddNode);
EXTERN int CalUniqueTableForIdLookup(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, Cal_Bdd_t thenBdd, Cal_Bdd_t elseBdd, Cal_Bdd_t * bddPtr);
EXTERN int CalUniqueTableForIdFindOrAdd(Cal_BddManager_t * bddManager, CalHashTable_t * hashTable, Cal_Bdd_t thenBdd, Cal_Bdd_t elseBdd, Cal_Bdd_t * bddPtr);
EXTERN void CalHashTableRehash(CalHashTable_t *hashTable, int grow);
EXTERN void CalUniqueTableForIdRehashNode(CalHashTable_t *hashTable, CalBddNode_t *bddNode, CalBddNode_t *thenBddNode, CalBddNode_t *elseBddNode);
EXTERN unsigned long CalBddUniqueTableNumLockedNodes(Cal_BddManager_t *bddManager, CalHashTable_t *uniqueTableForId);
EXTERN void CalPackNodes(Cal_BddManager_t *bddManager);
EXTERN void CalBddPackNodesForSingleId(Cal_BddManager_t *bddManager, Cal_BddId_t id);
EXTERN void CalBddPackNodesAfterReorderForSingleId(Cal_BddManager_t *bddManager, int fixForwardedNodesFlag, int bestIndex, int bottomIndex);
EXTERN void CalBddPackNodesForMultipleIds(Cal_BddManager_t *bddManager, Cal_BddId_t beginId, int numLevels);
EXTERN CalHashTable_t * CalHashTableOneInit(Cal_BddManager_t * bddManager, int itemSize);
EXTERN void CalHashTableOneQuit(CalHashTable_t * hashTable);
EXTERN void CalHashTableOneInsert(CalHashTable_t * hashTable, Cal_Bdd_t keyBdd, char * valuePtr);
EXTERN int CalHashTableOneLookup(CalHashTable_t * hashTable, Cal_Bdd_t keyBdd, char ** valuePtrPtr);
EXTERN int CalHashTableThreeFindOrAdd(CalHashTable_t * hashTable, Cal_Bdd_t f, Cal_Bdd_t g, Cal_Bdd_t h, Cal_Bdd_t * bddPtr);
EXTERN void CalSetInteract(Cal_BddManager_t *bddManager, int x, int y);
EXTERN int CalTestInteract(Cal_BddManager_t *bddManager, int x, int y);
EXTERN int CalInitInteract(Cal_BddManager_t *bddManager);
EXTERN CalPageManager_t * CalPageManagerInit(int numPagesPerSegment);
EXTERN int CalPageManagerQuit(CalPageManager_t * pageManager);
EXTERN void CalPageManagerPrint(CalPageManager_t * pageManager);
EXTERN CalNodeManager_t * CalNodeManagerInit(CalPageManager_t * pageManager);
EXTERN int CalNodeManagerQuit(CalNodeManager_t * nodeManager);
EXTERN void CalNodeManagerPrint(CalNodeManager_t * nodeManager);
EXTERN CalAddress_t * CalPageManagerAllocPage(CalPageManager_t * pageManager);
EXTERN void CalPageManagerFreePage(CalPageManager_t * pageManager, CalAddress_t * page);
EXTERN int CalIncreasingOrderCompare(const void *a, const void *b);
EXTERN int CalDecreasingOrderCompare(const void *a, const void *b);
EXTERN void CalBddReorderFixProvisionalNodes(Cal_BddManager_t *bddManager);
EXTERN void CalCheckPipelineValidity(Cal_BddManager_t *bddManager);
EXTERN char * CalBddVarName(Cal_BddManager_t *bddManager, Cal_Bdd_t v, Cal_VarNamingFn_t VarNamingFn, Cal_Pointer_t env);
EXTERN void CalBddNumberSharedNodes(Cal_BddManager_t *bddManager, Cal_Bdd_t f, CalHashTable_t *hashTable, long *next);
EXTERN void CalBddMarkSharedNodes(Cal_BddManager_t *bddManager, Cal_Bdd_t f);
EXTERN int CalOpExists(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_Bdd_t * resultBddPtr);
EXTERN int CalOpRelProd(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_Bdd_t g, Cal_Bdd_t * resultBddPtr);
EXTERN int CalOpCofactor(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_Bdd_t c, Cal_Bdd_t * resultBddPtr);
EXTERN void CalBddReorderAuxBF(Cal_BddManager_t * bddManager);
EXTERN void CalBddReorderFixCofactors(Cal_BddManager bddManager, Cal_BddId_t id);
EXTERN void CalFixupAssoc(Cal_BddManager_t *bddManager, long id1, long id2, CalAssociation_t *assoc);
EXTERN void CalBddReorderReclaimForwardedNodes(Cal_BddManager bddManager, int startIndex, int endIndex);
EXTERN void CalBddReorderBlockSift(Cal_BddManager_t *bddManager, double maxSizeFactor);
EXTERN void CalBddReorderBlockWindow(Cal_BddManager bddManager, Cal_Block block, char *levels);
EXTERN void CalBddReorderAuxDF(Cal_BddManager_t *bddManager);
EXTERN void CalAlignCollisionChains(Cal_BddManager_t *bddManager);
EXTERN void CalBddReorderFixUserBddPtrs(Cal_BddManager bddManager);
EXTERN int CalCheckAllValidity(Cal_BddManager bddManager);
EXTERN int CalCheckValidityOfNodesForId(Cal_BddManager bddManager, int id);
EXTERN int CalCheckValidityOfNodesForWindow(Cal_BddManager bddManager, Cal_BddIndex_t index, int numLevels);
EXTERN int CalCheckValidityOfANode(Cal_BddManager_t *bddManager, CalBddNode_t *bddNode, int id);
EXTERN void CalCheckRefCountValidity(Cal_BddManager_t *bddManager);
EXTERN int CalCheckAssoc(Cal_BddManager_t *bddManager);
EXTERN void CalBddReorderVarSift(Cal_BddManager bddManager, double maxSizeFactor);
EXTERN void CalBddReorderVarWindow(Cal_BddManager bddManager, char *levels);
EXTERN int CalOpAnd(Cal_BddManager_t * bddManager, Cal_Bdd_t F, Cal_Bdd_t G, Cal_Bdd_t * resultBddPtr);
EXTERN int CalOpNand(Cal_BddManager_t * bddManager, Cal_Bdd_t F, Cal_Bdd_t G, Cal_Bdd_t * resultBddPtr);
EXTERN int CalOpOr(Cal_BddManager_t * bddManager, Cal_Bdd_t F, Cal_Bdd_t G, Cal_Bdd_t * resultBddPtr);
EXTERN int CalOpXor(Cal_BddManager_t * bddManager, Cal_Bdd_t F, Cal_Bdd_t G, Cal_Bdd_t * resultBddPtr);
EXTERN Cal_Bdd_t CalOpITE(Cal_BddManager_t *bddManager, Cal_Bdd_t f, Cal_Bdd_t g, Cal_Bdd_t h, CalHashTable_t **reqQueForITE);
EXTERN int main(int argc, char ** argv);
EXTERN void CalUniqueTablePrint(Cal_BddManager_t *bddManager);
EXTERN void CalBddFunctionPrint(Cal_BddManager_t * bddManager, Cal_Bdd_t calBdd, char * name);
EXTERN int CalBddPreProcessing(Cal_BddManager_t *bddManager, int count, ...);
EXTERN int CalBddPostProcessing(Cal_BddManager_t *bddManager);
EXTERN int CalBddArrayPreProcessing(Cal_BddManager_t *bddManager, Cal_Bdd *userBddArray);
EXTERN Cal_Bdd_t CalBddGetInternalBdd(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_Bdd CalBddGetExternalBdd(Cal_BddManager_t *bddManager, Cal_Bdd_t internalBdd);
EXTERN void CalBddFatalMessage(char *string);
EXTERN void CalBddWarningMessage(char *string);
EXTERN void CalBddNodePrint(CalBddNode_t *bddNode);
EXTERN void CalBddPrint(Cal_Bdd_t calBdd);
EXTERN void CalHashTablePrint(CalHashTable_t *hashTable);
EXTERN void CalHashTableOnePrint(CalHashTable_t *hashTable, int flag);
EXTERN void CalUtilSRandom(long seed);
EXTERN long CalUtilRandom();

/**AutomaticEnd***************************************************************/

#endif /* _INT */
