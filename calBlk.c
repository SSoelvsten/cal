/**CFile***********************************************************************

  FileName    [calBlk.c]

  PackageName [cal]

  Synopsis    [Routines for manipulating blocks of variables.]

  Description [Routines for manipulating blocks of variables.]

  Author      [Rajeev K. Ranjan (rajeev@eecs.berkeley.edu). Modelled on the BDD package
  developed by David Long.]
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

  Revision    [$Id: calBlk.c,v 1.1.1.2 1998/05/04 00:59:05 hsv Exp $]

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

static void AddBlock(Cal_Block b1, Cal_Block b2);

/**AutomaticEnd***************************************************************/

/* BDD variable block routines */
/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Creates and returns a variable block used for
  controlling dynamic reordering.]

  Description        [The block is specified by passing the first
  variable and the length of the block. The "length" number of
  consecutive variables starting from "variable" are put in the
  block.]   

  SideEffects        [A new block is created.]

  SeeAlso            []

******************************************************************************/
Cal_Block
Cal_BddNewVarBlock(Cal_BddManager bddManager, Cal_Bdd variable, long length)
{
  Cal_Block b;
  Cal_Bdd_t calBdd = CalBddGetInternalBdd(bddManager, variable);
  
  if (CalBddTypeAux(bddManager, calBdd) != CAL_BDD_TYPE_POSVAR) {
    CalBddWarningMessage("Cal_BddNewVarBlock: second argument is not a positive variable\n"); 
    if (CalBddIsBddConst(calBdd)){
      return (Cal_Block) 0;
	}
  }

  /*b = CAL_BDD_NEW_REC(bddManager, Cal_Block_t);*/
  b = Cal_MemAlloc(Cal_Block_t, 1);
  b->reorderable = 0;
  b->firstIndex = bddManager->idToIndex[calBdd.bddId];
  if (length <= 0) {
    CalBddWarningMessage("Cal_BddNewVarBlock: invalid final argument");
    length = 1;
  }
  b->lastIndex = b->firstIndex + length - 1;
  if (b->lastIndex >= bddManager->numVars) {
    CalBddWarningMessage("Cal_BddNewVarBlock: range covers non-existent variables"); 
    b->lastIndex = bddManager->numVars - 1;
  }
  AddBlock(bddManager->superBlock, b);
  return (b);
}
/**Function********************************************************************

  Synopsis           [Sets the reoderability of a particular block.]

  Description        [If a block is reorderable, the child blocks are
  recursively involved in swapping.]

  SideEffects        [None.]

  SeeAlso            []

******************************************************************************/
void
Cal_BddVarBlockReorderable(Cal_BddManager bddManager, Cal_Block block,
                           int reorderable)
{
  block->reorderable = reorderable;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
long
CalBddFindBlock(Cal_Block block, long index)
{
  long i, j, k;

  i = 0;
  j = block->numChildren-1;
  while (i <= j) {
    k = (i+j)/2;
    if (block->children[k]->firstIndex <= index &&
        block->children[k]->lastIndex >= index){
      return (k);
    }
    if (block->children[k]->firstIndex > index){
      j = k-1;
    }
    else {
      i = k+1;
    }
  }
  return i;
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalBddBlockDelta(Cal_Block b, long delta)
{
  long i;
  b->firstIndex += delta;
  b->lastIndex += delta;
  for (i=0; i < b->numChildren; ++i)
    CalBddBlockDelta(b->children[i], delta);
}


/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
Cal_Block
CalBddShiftBlock(Cal_BddManager_t *bddManager, Cal_Block b, long index)
{
  long i, j;
  Cal_Block p;

  if (b->firstIndex >= index) {
    CalBddBlockDelta(b, 1l);
    return (b);
  }
  if (b->lastIndex < index) return (b);
  b->lastIndex++;
  i = CalBddFindBlock(b, index);
  if (i == b->numChildren || b->children[i]->firstIndex == index) {
    b->children = (Cal_Block *)
        Cal_MemRealloc(Cal_Block, b->children, b->numChildren+1);
    for (j = b->numChildren-1; j >= i; --j){
      b->children[j+1] = CalBddShiftBlock(bddManager, b->children[j], index);
    }
    b->numChildren++;
    /*p = CAL_BDD_NEW_REC(bddManager, Cal_Block_t);*/
    p = Cal_MemAlloc(Cal_Block_t, 1);
    p->reorderable = 0;
    p->firstIndex = index;
    p->lastIndex = index;
    p->numChildren = 0;
    p->children = 0;
    b->children[i] = p;
  }
  else{
    while (i < b->numChildren) {
      CalBddShiftBlock(bddManager, b->children[i], index);
      ++i;
    }
  }
  return (b);
}
/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
unsigned long
CalBlockMemoryConsumption(Cal_Block block)
{
  unsigned long totalBytes = 0;
  int i;
  
  totalBytes += sizeof(Cal_Block);
  for (i=0; i<block->numChildren; i++){
    totalBytes += CalBlockMemoryConsumption(block->children[i]);
  }
  return totalBytes;
}
/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalFreeBlockRecursively(Cal_Block block)
{
  int i;
  
  for (i=0; i<block->numChildren; i++){
    CalFreeBlockRecursively(block->children[i]);
  }
  Cal_MemFree(block->children);
  Cal_MemFree(block);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static void
AddBlock(Cal_Block b1, Cal_Block b2)
{
  long i, j, k;
  Cal_Block start, end;

  if (b1->numChildren){
    i = CalBddFindBlock(b1, b2->firstIndex);
    start = b1->children[i];
    j = CalBddFindBlock(b1, b2->lastIndex);
    end = b1->children[j];
    if (i == j) {
      AddBlock(start, b2);
    }
    else {
      if (start->firstIndex != b2->firstIndex ||
          end->lastIndex != b2->lastIndex){
        CalBddFatalMessage("AddBlock: illegal block overlap");
      }
      b2->numChildren = j-i+1;
	  b2->children = Cal_MemAlloc(Cal_Block, b2->numChildren); 
	  for (k=0; k < b2->numChildren; ++k){
	    b2->children[k] = b1->children[i+k];
      }
	  b1->children[i] = b2;
	  ++i;
	  for (k=j+1; k < b1->numChildren; ++k, ++i){
	    b1->children[i] = b1->children[k];
      }
	  b1->numChildren -= (b2->numChildren-1);
	  b1->children = (Cal_Block *)
          Cal_MemRealloc(Cal_Block, b1->children, b1->numChildren);
	}
  }
  else {
      /* b1 and b2 are blocks representing just single variables. */
      b1->numChildren = 1;
      b1->children = Cal_MemAlloc(Cal_Block, b1->numChildren); 
      b1->children[0] = b2;
      b2->numChildren = 0;
      b2->children = 0;
  }
}


