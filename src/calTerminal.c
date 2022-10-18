/**CFile***********************************************************************

  FileName    [calTerminal.c]

  PackageName [cal]

  Synopsis    [Contains the terminal function for various BDD operations.]

  Description []

  SeeAlso     []

  Author      [Rajeev K. Ranjan (rajeev@eecs.berkeley.edu) and
               Jagesh V. Sanghavi (sanghavi@eecs.berkeley.edu]

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

  Revision    [$Id: calTerminal.c,v 1.1.1.2 1997/02/12 21:11:30 hsv Exp $]

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
CalOpAnd(Cal_BddManager_t * bddManager,
         Cal_Bdd_t  F,
         Cal_Bdd_t  G,
         Cal_Bdd_t * resultBddPtr)
{
  if(CalBddIsBddConst(F)){
    if(CalBddIsBddOne(bddManager, F)){
      *resultBddPtr = G;
    }
    else{
      *resultBddPtr = F;
    }
    return 1;
  }
  else if(CalBddIsBddConst(G)){
    if(CalBddIsBddOne(bddManager, G)){
      *resultBddPtr = F;
    }
    else{
      *resultBddPtr = G;
    }
    return 1;
  }
  else{
    CalBddNode_t *bddNodeF, *bddNodeG;
    bddNodeF = CalBddGetBddNode(F);
    bddNodeG = CalBddGetBddNode(G);
    if((CAL_BDD_POINTER(bddNodeF) == CAL_BDD_POINTER(bddNodeG))){
      if((CalAddress_t)bddNodeF ^ (CalAddress_t)bddNodeG){
        *resultBddPtr = CalBddZero(bddManager);
      }
      else{
        *resultBddPtr = F;
      }
      return 1;
    }
    else{
      return 0;
    }
  }
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalOpNand(Cal_BddManager_t * bddManager,
          Cal_Bdd_t  F,
          Cal_Bdd_t  G,
          Cal_Bdd_t * resultBddPtr)
{
  if(CalBddIsBddConst(F)){
    if(CalBddIsBddOne(bddManager, F)){
      CalBddNot(G, *resultBddPtr);
    }
    else{
      CalBddNot(F, *resultBddPtr);
    }
    return 1;
  }
  else if(CalBddIsBddConst(G)){
    if(CalBddIsBddOne(bddManager, G)){
      CalBddNot(F, *resultBddPtr);
    }
    else{
      CalBddNot(G, *resultBddPtr);
    }
    return 1;
  }
  else{
    CalBddNode_t *bddNodeF, *bddNodeG;
    bddNodeF = CalBddGetBddNode(F);
    bddNodeG = CalBddGetBddNode(G);
    if((CAL_BDD_POINTER(bddNodeF) == CAL_BDD_POINTER(bddNodeG))){
      if((CalAddress_t)bddNodeF ^ (CalAddress_t)bddNodeG){
        *resultBddPtr = CalBddOne(bddManager);
      }
      else{
        CalBddNot(F, *resultBddPtr);
      }
      return 1;
    }
    else{
      return 0;
    }
  }
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalOpOr(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  F,
  Cal_Bdd_t  G,
  Cal_Bdd_t * resultBddPtr)
{
  if(CalBddIsBddConst(F)){
    if(CalBddIsBddOne(bddManager, F)){
      *resultBddPtr = F;
    }
    else{
      *resultBddPtr = G;
    }
    return 1;
  }
  else if(CalBddIsBddConst(G)){
    if(CalBddIsBddOne(bddManager, G)){
      *resultBddPtr = G;
    }
    else{
      *resultBddPtr = F;
    }
    return 1;
  }
  else{
    CalBddNode_t *bddNodeF, *bddNodeG;
    bddNodeF = CalBddGetBddNode(F);
    bddNodeG = CalBddGetBddNode(G);
    if((CAL_BDD_POINTER(bddNodeF) == CAL_BDD_POINTER(bddNodeG))){
      if((CalAddress_t)bddNodeF ^ (CalAddress_t)bddNodeG){
        *resultBddPtr = CalBddOne(bddManager);
      }
      else{
        *resultBddPtr = F;
      }
      return 1;
    }
    else{
      return 0;
    }
  }
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalOpXor(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  F,
  Cal_Bdd_t  G,
  Cal_Bdd_t * resultBddPtr)
{
  if(CalBddIsBddConst(F)){
    if(CalBddIsBddOne(bddManager, F)){
      CalBddNot(G, *resultBddPtr);
    }
    else{
      *resultBddPtr = G;
    }
    return 1;
  }
  else if(CalBddIsBddConst(G)){
    if(CalBddIsBddOne(bddManager, G)){
      CalBddNot(F, *resultBddPtr);
    }
    else{
      *resultBddPtr = F;
    }
    return 1;
  }
  else{
    CalBddNode_t *bddNodeF, *bddNodeG;
    bddNodeF = CalBddGetBddNode(F);
    bddNodeG = CalBddGetBddNode(G);
    if((CAL_BDD_POINTER(bddNodeF) == CAL_BDD_POINTER(bddNodeG))){
      if((CalAddress_t)bddNodeF ^ (CalAddress_t)bddNodeG){
        *resultBddPtr = CalBddOne(bddManager);
      }
      else{
        *resultBddPtr = CalBddZero(bddManager);
      }
      return 1;
    }
    else{
      return 0;
    }
  }
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
Cal_Bdd_t
CalOpITE(
  Cal_BddManager_t *bddManager,
  Cal_Bdd_t f,
  Cal_Bdd_t g,
  Cal_Bdd_t h,
  CalHashTable_t **reqQueForITE)
{
  CalBddNode_t *bddNode1, *bddNode2;
  int complementFlag = 0;

  /*
   * First phase: Make substitutions 
   * ITE(F,F,H) = ITE(F,1,H)
   * ITE(F,F',H) = ITE(F,0,H)
   * ITE(F,G,F) = ITE(F,G,0)
   * ITE(F,G,F') = ITE(F,G,1)
   */
  bddNode1 = CalBddGetBddNode(f);
  bddNode2 = CalBddGetBddNode(g);
  if((CAL_BDD_POINTER(bddNode1) == CAL_BDD_POINTER(bddNode2))){
    if((CalAddress_t)bddNode1 ^ (CalAddress_t)bddNode2){
      g = CalBddZero(bddManager);
    }
    else{
      g = CalBddOne(bddManager);
    }
  }
  bddNode2 = CalBddGetBddNode(h);
  if((CAL_BDD_POINTER(bddNode1) == CAL_BDD_POINTER(bddNode2))){
    if((CalAddress_t)bddNode1 ^ (CalAddress_t)bddNode2){
      h = CalBddOne(bddManager);
    }
    else{
      h = CalBddZero(bddManager);
    }
  }

  /*
   * Second phase: Fix the complement pointers.
   * There are 3 possible cases:
   * F +ve G -ve: ITE(F ,G',H ) = ITE(F ,G ,H')'
   * F -ve H +ve: ITE(F',G ,H ) = ITE(F ,H ,G)
   * F -ve H -ve: ITE(F',G ,H') = ITE(F ,H ,G')'
   */
  if(CalBddIsOutPos(f)){
    if(!CalBddIsOutPos(g)){
      CalBddNot(g, g);
      CalBddNot(h, h);
      complementFlag = 1;
    }
  }
  else{
    Cal_Bdd_t tmpBdd;
    CalBddNot(f, f);
    if(CalBddIsOutPos(h)){
      tmpBdd = g;
      g = h;
      h = tmpBdd;
    }
    else{
      tmpBdd = g;
      CalBddNot(h, g);
      CalBddNot(tmpBdd, h);
      complementFlag = 1;
    }
  }

  /*
   * Third phase: Check for the terminal cases; create new request if needed
   * ite(1,G,H) = G
   * ite(0,G,H) = H (impossible by construction in second phase)
   * ite(F,G,G) = G
   * ite(F,1,0) = F
   * ite(F,0,1) = F'(impossible by construction in second phase)
   */
  if(CalBddIsBddConst(f) || CalBddIsEqual(g, h)){
    CalBddUpdatePhase(g, complementFlag);
    return g;
  }
  else if(CalBddIsBddConst(g) && CalBddIsBddConst(h)){
    CalBddUpdatePhase(f, complementFlag);
    return f;
  }
  else{
    Cal_BddId_t bddId;
    Cal_Bdd_t result;
    CalBddGetMinId3(bddManager, f, g, h, bddId);
    CalHashTableThreeFindOrAdd(reqQueForITE[bddId], f, g, h, &result);
    CalBddUpdatePhase(result, complementFlag);
    return result;
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/













