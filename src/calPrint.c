/**CFile***********************************************************************

  FileName    [calPrint.c]

  PackageName [cal]

  Synopsis    [Routine for printing a BDD.]

  Description []

  SeeAlso     [None]

  Author      [Rajeev K. Ranjan (rajeev@eecs.berkeley.edu) and
               Jagesh V. Sanghavi (sanghavi@eecs.berkeley.edu)
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

  Revision    [$Id: calPrint.c,v 1.2 1998/09/16 16:40:41 ravi Exp $]

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
static char defaultTerminalId[]="terminal XXXXXXXXXX XXXXXXXXXX";
static char defaultVarName[]="var.XXXXXXXXXX";


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void Chars(char c, int n, FILE *fp);
static void BddPrintTopVar(Cal_BddManager_t *bddManager, Cal_Bdd_t f, Cal_VarNamingFn_t VarNamingFn, Cal_Pointer_t env, FILE *fp);
static void BddPrintBddStep(Cal_BddManager_t *bddManager, Cal_Bdd_t f, Cal_VarNamingFn_t VarNamingFn, Cal_TerminalIdFn_t TerminalIdFn, Cal_Pointer_t env, FILE *fp, CalHashTable_t* hashTable, int indentation);
static char * BddTerminalId(Cal_BddManager_t *bddManager, Cal_Bdd_t f, Cal_TerminalIdFn_t TerminalIdFn, Cal_Pointer_t env);
static void BddTerminalValueAux(Cal_BddManager_t *bddManager, Cal_Bdd_t f, CalAddress_t *value1, CalAddress_t *value2);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Prints a BDD in the human readable form.]

  Description [Prints a human-readable representation of the BDD f to
  the file given by fp. The namingFn should be a pointer to a function
  taking a bddManager, a BDD and the pointer given by env. This
  function should return either a null pointer or a srting that is the
  name of the supplied variable. If it returns a null pointer, a
  default name is generated based on the index of the variable. It is
  also legal for naminFN to e null; in this case, default names are
  generated for all variables. The macro bddNamingFnNone is a null
  pointer of suitable type. terminalIdFn should be apointer to a
  function taking a bddManager and two longs. plus the pointer given
  by the env. It should return either a null pointer. If it returns a
  null pointer, or if terminalIdFn is null, then default names are
  generated for the terminals. The macro bddTerminalIdFnNone is a null
  pointer of suitable type.]

  SideEffects [None.]

******************************************************************************/
void
Cal_BddPrintBdd(Cal_BddManager bddManager,
                Cal_Bdd fUserBdd, Cal_VarNamingFn_t VarNamingFn,
                Cal_TerminalIdFn_t TerminalIdFn,
                Cal_Pointer_t env, FILE *fp)
{
  long next;
  CalHashTable_t *hashTable;

  Cal_Bdd_t f = CalBddGetInternalBdd(bddManager,fUserBdd);
  CalBddMarkSharedNodes(bddManager, f);
  hashTable = CalHashTableOneInit(bddManager, sizeof(long));
  next = 0;
  CalBddNumberSharedNodes(bddManager, f, hashTable, &next);
  BddPrintBddStep(bddManager, f, VarNamingFn, TerminalIdFn, env, fp,
                  hashTable, 0);
  CalHashTableOneQuit(hashTable);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
char *
CalBddVarName(Cal_BddManager_t *bddManager, Cal_Bdd_t v,
           Cal_VarNamingFn_t VarNamingFn,  Cal_Pointer_t env)
{
  char *name;
  if (VarNamingFn){
    Cal_Bdd userV = CalBddGetExternalBdd(bddManager, v);
    name = (*VarNamingFn)(bddManager, userV, env);
    Cal_BddFree(bddManager, userV);
  }
 else
   name=0;
  if (!name){
    sprintf(defaultVarName, "var.%d", CalBddGetBddIndex(bddManager, v));
    name = defaultVarName;
  }
  return (name);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalBddNumberSharedNodes(Cal_BddManager_t *bddManager, Cal_Bdd_t f,
                     CalHashTable_t *hashTable, long *next)
{
  Cal_Bdd_t thenBdd, elseBdd;
  int mark;
  
  if (CalBddIsBddConst(f) || ((1 << CalBddTypeAux(bddManager, f)) &
                           ((1 << CAL_BDD_TYPE_POSVAR) |
                            (1 << CAL_BDD_TYPE_NEGVAR))))
    return;
  mark = CalBddGetMark(f);
  if (mark == 0) return;
  if (mark  == 2) {
    CalHashTableOneInsert(hashTable, f, (char *)next);
    ++*next;
  }
  CalBddPutMark(f, 0);
  CalBddGetThenBdd(f, thenBdd);
  CalBddGetElseBdd(f, elseBdd);
  CalBddNumberSharedNodes(bddManager, thenBdd, hashTable, next);
  CalBddNumberSharedNodes(bddManager, elseBdd, hashTable, next);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalBddMarkSharedNodes(Cal_BddManager_t *bddManager, Cal_Bdd_t f)
{
  int mark;
  Cal_Bdd_t thenBdd, elseBdd;
  
  if (CalBddIsOutPos(f) == 0){
    CalBddNot(f,f);
  }
  if (CalBddIsBddConst(f) || CalBddTypeAux(bddManager, f) ==
      CAL_BDD_TYPE_POSVAR)
    return; 
  if ((mark = CalBddGetMark(f))){
    if (mark == 1){
      CalBddPutMark(f, 2);
      return;
    }
  }
  CalBddPutMark(f, 1);
  CalBddGetThenBdd(f, thenBdd);
  CalBddGetElseBdd(f, elseBdd);
  CalBddMarkSharedNodes(bddManager, thenBdd);
  CalBddMarkSharedNodes(bddManager, elseBdd);
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
Chars(char c, int n,FILE *fp)
{
  int i;
  for (i=0; i < n; ++i){
    fputc(c, fp);
  }
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddPrintTopVar(Cal_BddManager_t *bddManager, Cal_Bdd_t f,
               Cal_VarNamingFn_t VarNamingFn, Cal_Pointer_t env, FILE *fp)
{
  Cal_Bdd_t ifVar;
  ifVar = CalBddIf(bddManager, f);
  fputs(CalBddVarName(bddManager, ifVar, VarNamingFn, env), fp);  
  fputc('\n', fp);
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/static void
BddPrintBddStep(Cal_BddManager_t *bddManager,
                Cal_Bdd_t f, Cal_VarNamingFn_t VarNamingFn,
                Cal_TerminalIdFn_t TerminalIdFn,
                Cal_Pointer_t env, FILE *fp, CalHashTable_t* hashTable,
                int indentation)
{
  int negated;
  long *number;
  Cal_Bdd_t fNot, thenBdd, elseBdd;
  
  Chars(' ', indentation, fp);
  switch (CalBddTypeAux(bddManager, f)){
      case CAL_BDD_TYPE_ZERO:
      case CAL_BDD_TYPE_ONE:
        fputs(BddTerminalId(bddManager, f, TerminalIdFn, env), fp);
        fputc('\n', fp);
        break;
      case CAL_BDD_TYPE_NEGVAR:
        fputc('!', fp);
        /* fall through */
      case CAL_BDD_TYPE_POSVAR:
        BddPrintTopVar(bddManager, f, VarNamingFn, env, fp);
        break;
      case CAL_BDD_TYPE_NONTERMINAL:
        CalBddNot(f, fNot);
        if (CalHashTableOneLookup(hashTable, fNot, Cal_Nil(char *))){
          f = fNot;
          negated = 1;
        }
        else {
          negated=0;
        }
        CalHashTableOneLookup(hashTable, f, (char **)&number);
        if (number && *number < 0){
          if (negated)
            fputc('!', fp);
          fprintf(fp, "subformula %d\n", (int)-*number-1);
        }
      else {
        if (number){
	      fprintf(fp, "%d: ", (int) *number);
	      *number= -*number-1;
	    }
        fputs("if ", fp);
        BddPrintTopVar(bddManager, f, VarNamingFn, env, fp);
        CalBddGetThenBdd(f, thenBdd);
        BddPrintBddStep(bddManager, thenBdd, VarNamingFn,
                        TerminalIdFn, env, fp, hashTable, indentation+2);
        Chars(' ', indentation, fp);
        fputs("else if !", fp);
        BddPrintTopVar(bddManager, f, VarNamingFn, env, fp);
        CalBddGetElseBdd(f, elseBdd);
        BddPrintBddStep(bddManager, elseBdd, VarNamingFn,
                        TerminalIdFn, env, fp, hashTable, indentation+2); 
        Chars(' ', indentation, fp);
        fputs("endif ", fp);
        BddPrintTopVar(bddManager, f, VarNamingFn, env, fp);
      }
        break;
      default:
        CalBddFatalMessage("BddPrintBddStep: unknown type returned by Cal_BddType"); 
  }
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static char *
BddTerminalId(Cal_BddManager_t *bddManager, Cal_Bdd_t f,
              Cal_TerminalIdFn_t TerminalIdFn, Cal_Pointer_t env)
{
  char *id;
  CalAddress_t  v1, v2;
  BddTerminalValueAux(bddManager, f, &v1, &v2);
  if (TerminalIdFn) id = (*TerminalIdFn)(bddManager, v1, v2, env);
  else id=0;
  if (!id){
    if (CalBddIsBddOne(bddManager, f)) return ("1");
    if (CalBddIsBddZero(bddManager, f)) return ("0");
    sprintf(defaultTerminalId, "terminal %ld %ld", (long)v1, (long)v2);
    id = defaultTerminalId;
  }
  return (id);
}



/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddTerminalValueAux(Cal_BddManager_t *bddManager, Cal_Bdd_t f,
                    CalAddress_t *value1, CalAddress_t *value2)
{
  if (CalBddIsOutPos(f)){
    *value1 = (CalAddress_t)CalBddGetThenBddNode(f);
    *value2 = (CalAddress_t)CalBddGetElseBddNode(f);
  }
  else
    (*bddManager->TransformFn)(bddManager,
                               (CalAddress_t)CalBddGetThenBddNode(f),
                               (CalAddress_t)CalBddGetElseBddNode(f),
                                value1, value2, bddManager->transformEnv);  
}








