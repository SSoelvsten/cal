/**CFile***********************************************************************

  FileName    [calInteract.c]

  PackageName [cal]

  Synopsis    [Functions to manipulate the variable interaction matrix.]

  Description [
  The interaction matrix tells whether two variables are
  both in the support of some function of the DD. The main use of the
  interaction matrix is in the in-place swapping. Indeed, if two
  variables do not interact, there is no arc connecting the two layers;
  therefore, the swap can be performed in constant time, without
  scanning the subtables. Another use of the interaction matrix is in
  the computation of the lower bounds for sifting. Finally, the
  interaction matrix can be used to speed up aggregation checks in
  symmetric and group sifting.<p>
  The computation of the interaction matrix is done with a series of
  depth-first searches. The searches start from those nodes that have
  only external references. The matrix is stored as a packed array of bits;
  since it is symmetric, only the upper triangle is kept in memory.
  As a final remark, we note that there may be variables that do
  intercat, but that for a given variable order have no arc connecting
  their layers when they are adjacent.]

  SeeAlso     []

  Author      [Original author:Fabio Somenzi. Modified for CAL package
  by Rajeev K. Ranjan]

  Copyright [ This file was created at the University of Colorado at
  Boulder.  The University of Colorado at Boulder makes no warranty
  about the suitability of this software for any purpose.  It is
  presented on an AS IS basis.]

******************************************************************************/

#include "calInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#if SIZEOF_LONG == 8
#define BPL 64
#define LOGBPL 6
#else
#define BPL 32
#define LOGBPL 5
#endif

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
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

static void ddSuppInteract(Cal_BddManager_t *bddManager, Cal_Bdd_t f, int *support);
static void ddClearLocal(Cal_Bdd_t f);
static void ddUpdateInteract(Cal_BddManager_t *bddManager, int *support);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Set interaction matrix entries.]

  Description [Given a pair of variables 0 <= x < y < table->size,
  sets the corresponding bit of the interaction matrix to 1.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void
CalSetInteract(Cal_BddManager_t *bddManager, int x, int y)
{
    int posn, word, bit;

    Cal_Assert(x < y);
    Cal_Assert(y < bddManager->numVars);
    Cal_Assert(x >= 0);

    posn = ((((bddManager->numVars << 1) - x - 3) * x) >> 1) + y - 1;
    word = posn >> LOGBPL;
    bit = posn & (BPL-1);
    bddManager->interact[word] |= 1 << bit;

} /* end of CalSetInteract */


/**Function********************************************************************

  Synopsis    [Test interaction matrix entries.]

  Description [Given a pair of variables 0 <= x < y < bddManager->numVars,
  tests whether the corresponding bit of the interaction matrix is 1.
  Returns the value of the bit.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
CalTestInteract(Cal_BddManager_t *bddManager, int x, int y)
{
    int posn, word, bit, result;

    x -= 1; 
    y -= 1;
    
    if (x > y) {
	int tmp = x;
	x = y;
	y = tmp;
    }
    Cal_Assert(x < y);
    Cal_Assert(y < bddManager->numVars);
    Cal_Assert(x >= 0);

    posn = ((((bddManager->numVars << 1) - x - 3) * x) >> 1) + y - 1;
    word = posn >> LOGBPL;
    bit = posn & (BPL-1);
    result = (bddManager->interact[word] >> bit) & 1;
    return(result);

} /* end of CalTestInteract */


/**Function********************************************************************

  Synopsis    [Initializes the interaction matrix.]

  Description [Initializes the interaction matrix. The interaction
  matrix is implemented as a bit vector storing the upper triangle of
  the symmetric interaction matrix. The bit vector is kept in an array
  of long integers. The computation is based on a series of depth-first
  searches, one for each root of the DAG. A local flag (the mark bits)
  is used.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
CalInitInteract(Cal_BddManager_t *bddManager)
{
  int i,k;
  int words;
  long *interact;
  int *support;
  long numBins;
  CalBddNode_t **bins, *bddNode, *nextBddNode;
  
  int n = bddManager->numVars;
  
  words = ((n * (n-1)) >> (1 + LOGBPL)) + 1;
  bddManager->interact = interact = Cal_MemAlloc(long, words);
  if (interact == NULL) return(0);
  for (i = 0; i < words; i++) {
      interact[i] = 0;
  }
  
  support = Cal_MemAlloc(int, n);
  if (support == Cal_Nil(int)) {
    Cal_MemFree(interact);
    return(0);
  }
  bins = bddManager->uniqueTable[0]->bins;
  numBins = bddManager->uniqueTable[0]->numBins;
  for (i=0; i<numBins; i++){
    for (bddNode = bins[i]; bddNode; bddNode = nextBddNode) {
      Cal_Bdd_t internalBdd;
      nextBddNode = CalBddNodeGetNextBddNode(bddNode);
      CalBddNodeGetThenBdd(bddNode, internalBdd);
      for (k = 0; k < n; k++) {
        support[k] = 0;
      }
      ddSuppInteract(bddManager, internalBdd, support);
      ddClearLocal(internalBdd);
      ddUpdateInteract(bddManager, support);
    }
  }
  /* If there are some results pending in the pipeline, we need to
     take those into account as well */

  if (bddManager->pipelineState == CREATE){
    CalRequestNode_t **requestNodeListArray =
        bddManager->requestNodeListArray; 
    Cal_Bdd_t resultBdd;
    for (i=0;
         i<bddManager->pipelineDepth-bddManager->currentPipelineDepth;
         i++){
      for (bddNode = *requestNodeListArray; bddNode;
           bddNode = nextBddNode){ 
        nextBddNode = CalBddNodeGetNextBddNode(bddNode);
        Cal_Assert(CalBddNodeIsForwarded(bddNode));
        CalBddNodeGetThenBdd(bddNode, resultBdd);
        Cal_Assert(CalBddIsForwarded(resultBdd) == 0);
        for (k = 0; k < n; k++) {
          support[k] = 0;
        }
        ddSuppInteract(bddManager, resultBdd, support);
        ddClearLocal(resultBdd);
        ddUpdateInteract(bddManager, support);
      }
      requestNodeListArray++;
    }
  }
  
  
  Cal_MemFree(support);
  return(1);
  
} /* end of CalInitInteract */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Find the support of f.]

  Description [Performs a DFS from f. Uses the LSB of the then pointer
  as visited flag.]

  SideEffects [Accumulates in support the variables on which f depends.]

  SeeAlso     []

******************************************************************************/
static void
ddSuppInteract(Cal_BddManager_t *bddManager, Cal_Bdd_t f, int *support)
{
  Cal_Bdd_t thenBdd, elseBdd;

  if (CalBddIsBddConst(f) || CalBddIsMarked(f)){
    return;
  }
  support[f.bddId-1] = 1;
  CalBddGetThenBdd(f, thenBdd);
  CalBddGetElseBdd(f, elseBdd);
  ddSuppInteract(bddManager, thenBdd, support);
  ddSuppInteract(bddManager, elseBdd, support);
  CalBddMark(f);
  return;
} /* end of ddSuppInteract */


/**Function********************************************************************

  Synopsis    [Performs a DFS from f, clearing the LSB of the then pointers.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
ddClearLocal(Cal_Bdd_t f)
{
  Cal_Bdd_t thenBdd;
  Cal_Bdd_t elseBdd;
  CalBddGetElseBdd(f, elseBdd);  
  if (CalBddIsBddConst(f) || !CalBddIsMarked(f)){
    return;
  }
  /* clear visited flag */
  CalBddUnmark(f);
  CalBddGetThenBdd(f, thenBdd);
  CalBddGetElseBdd(f, elseBdd);
  ddClearLocal(thenBdd);
  ddClearLocal(elseBdd);
  return;
} /* end of ddClearLocal */


/**Function********************************************************************

  Synopsis [Marks as interacting all pairs of variables that appear in
  support.]

  Description [If support[i] == support[j] == 1, sets the (i,j) entry
  of the interaction matrix to 1.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
ddUpdateInteract(Cal_BddManager_t *bddManager, int *support)
{
  int i,j;
  int n = bddManager->numVars;
  
  for (i = 0; i < n-1; i++) {
	if (support[i] == 1) {
      for (j = i+1; j < n; j++) {
		if (support[j] == 1) {
          CalSetInteract(bddManager, i, j);
		}
      }
	}
  }
} /* end of ddUpdateInteract */


