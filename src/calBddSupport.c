/**CFile***********************************************************************

  FileName    [calBddSupport.c]

  PackageName [cal]

  Synopsis    [Routines related to the support of a BDD.]
              

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

  Revision    [$Id: calBddSupport.c,v 1.1.1.3 1998/05/04 00:58:54 hsv Exp $]

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

static Cal_Bdd_t * CalBddSupportStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_Bdd_t * support);
static void CalBddUnmarkNodes(Cal_BddManager_t * bddManager, Cal_Bdd_t f);
static int CalBddDependsOnStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, Cal_BddIndex_t varIndex, int mark);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Name        [Cal_BddSupport]

  Synopsis    [returns the support of f as a null-terminated array of variables]

  Description [optional]

  SideEffects [None]

  SeeAlso     [optional]

******************************************************************************/
void
Cal_BddSupport(Cal_BddManager bddManager, Cal_Bdd fUserBdd,
               Cal_Bdd *support)
{
  if(CalBddPreProcessing(bddManager, 1, fUserBdd)){
    Cal_Bdd_t f = CalBddGetInternalBdd(bddManager, fUserBdd);
    Cal_Bdd_t *internalSupport = Cal_MemAlloc(Cal_Bdd_t, bddManager->numVars+1);
    Cal_Bdd_t *end;
    int i = 0;
    end = CalBddSupportStep(bddManager, f, internalSupport);
    *end = CalBddNull(bddManager);
    CalBddUnmarkNodes(bddManager, f);
    while (CalBddIsBddNull(bddManager, internalSupport[i]) == 0){
      *support = CalBddGetExternalBdd(bddManager, internalSupport[i]);
      support++;
      i++;
    }
    Cal_MemFree(internalSupport);
  }
  *support = (Cal_Bdd) 0;
}

/**Function********************************************************************

  Name        [Cal_BddDependsOn]

  Synopsis    [Returns 1 if f depends on var and returns 0 otherwise.]

  Description [Returns 1 if f depends on var and returns 0 otherwise.]

  SideEffects [None]

******************************************************************************/
int
Cal_BddDependsOn(Cal_BddManager bddManager, Cal_Bdd  fUserBdd,
                 Cal_Bdd varUserBdd)
{
  Cal_BddIndex_t bddIndex;
  Cal_Bdd_t f, var;
  
  if(CalBddPreProcessing(bddManager, 2, fUserBdd, varUserBdd)){
    f = CalBddGetInternalBdd(bddManager, fUserBdd);
    var = CalBddGetInternalBdd(bddManager, varUserBdd);
    if(CalBddIsBddConst(var)){
      return 1;
    }
    bddIndex = CalBddGetBddIndex(bddManager, var);
    CalBddDependsOnStep(bddManager, f, bddIndex, 1);
    return CalBddDependsOnStep(bddManager, f, bddIndex, 0);
  }
  return (0);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Name        [CalBddSupportStep]

  Synopsis    [returns the support of f as a null-terminated array of variables]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static Cal_Bdd_t *
CalBddSupportStep(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  f,
  Cal_Bdd_t * support)
{
  Cal_Bdd_t tempBdd;

  if(CalBddIsMarked(f) || CalBddIsBddConst(f)){
    return support;
  }
  tempBdd = bddManager->varBdds[CalBddGetBddId(f)];
  if(!CalBddIsMarked(tempBdd)){
    CalBddMark(tempBdd);
    *support = tempBdd;
    ++support;
  }
  CalBddMark(f);
  CalBddGetThenBdd(f, tempBdd);
  support = CalBddSupportStep(bddManager, tempBdd, support);
  CalBddGetElseBdd(f, tempBdd);
  return CalBddSupportStep(bddManager, tempBdd, support);
}


/**Function********************************************************************

  Name        [CalBddUnmarkNodes]

  Synopsis    [recursively unmarks the nodes]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalBddUnmarkNodes(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  f)
{
  Cal_Bdd_t tempBdd;

  if(!CalBddIsMarked(f) || CalBddIsBddConst(f)){
    return;
  }
  CalBddUnmark(f);
  tempBdd = bddManager->varBdds[CalBddGetBddId(f)];
  CalBddUnmark(tempBdd);
  CalBddGetThenBdd(f, tempBdd);
  CalBddUnmarkNodes(bddManager, tempBdd);
  CalBddGetElseBdd(f, tempBdd);
  CalBddUnmarkNodes(bddManager, tempBdd);
}


/**Function********************************************************************


  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static int
CalBddDependsOnStep(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  f,
  Cal_BddIndex_t  varIndex,
  int  mark)
{
  Cal_BddIndex_t fIndex;
  Cal_Bdd_t tempBdd;

  fIndex=CalBddGetBddIndex(bddManager, f);
  if(fIndex > varIndex){
    return 0;
  }
  if(fIndex == varIndex){
    return 1;
  }
  if((mark && CalBddIsMarked(f)) || (!mark && !CalBddIsMarked(f))){
    return (0);
  }
  if(mark){
    CalBddMark(f);
  }
  else{
    CalBddUnmark(f);
  }
  CalBddGetThenBdd(f, tempBdd);
  if(CalBddDependsOnStep(bddManager, tempBdd, varIndex, mark)){
    return 1;
  }
  CalBddGetElseBdd(f, tempBdd);
  return CalBddDependsOnStep(bddManager, tempBdd, varIndex, mark);
}











