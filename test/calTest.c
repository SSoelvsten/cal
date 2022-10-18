/**CFile***********************************************************************

  FileName    [calTest.c]

  PackageName [cal]

  Synopsis    [This file contains the test routines for the CAL package.]

  Description [optional]

  SeeAlso     [optional]

  Author      [Rajeev K. Ranjan (rajeev@eecs.berkeley.edu)
               Jagesh Sanghavi  (sanghavi@eecs.berkeley.edu)
               Modified and extended from the original version written
               by David Long. 
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

  Revision    [$Id: calTest.c,v 1.4 1998/09/17 08:51:30 ravi Exp $]

******************************************************************************/

#include <calInt.h>
#include "time.h"
#include <signal.h>

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define VARS 50
#define TT_BITS 32              /* Size of tt in bits */
#define MAX_TT_VARS 20
#define ITERATIONS 50         /* Number of trials to run */
#define BITS_PER_INT 32
#define LG_BITS_PER_INT 5

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef unsigned long TruthTable_t;       /* "Truth table" */


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
static Cal_BddManager bddManager;
static Cal_Bdd vars[VARS];
static TruthTable_t cofactorMasks[]=
{
  0xffff0000,
  0xff00ff00,
  0xf0f0f0f0,
  0xcccccccc,
  0xaaaaaaaa,
};
static int TT_VARS;
static CalAddress_t asDoubleSpace[2];
/*static char filename[]="/tmp/tmpXXXXXX";*/
static char filename[1024];


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define EncodingToBdd(table) (Decode(0, (table)))

static void Error(char *op, Cal_BddManager bddManager, Cal_Bdd result, Cal_Bdd expected, ...);

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static double asDouble(CalAddress_t v1, CalAddress_t v2);
static void asAddress(double n, CalAddress_t * r1, CalAddress_t * r2);
static char * terminalIdFn(Cal_BddManager bddManager, CalAddress_t v1, CalAddress_t v2, Cal_Pointer_t pointer);
static void PrintBdd(Cal_BddManager bddManager, Cal_Bdd f);
static void Error(char *op, Cal_BddManager bddManager, Cal_Bdd result, Cal_Bdd expected, ...);
static TruthTable_t Cofactor(TruthTable_t table, int var, int value);
static Cal_Bdd Decode(int var, TruthTable_t table);
static void TestAnd(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2);
static void TestNand(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2);
static void TestOr(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2);
static void TestITE(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2, Cal_Bdd f3, TruthTable_t table3);
static void TestXor(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2);
static void TestIdNot(Cal_BddManager bddManager, Cal_Bdd f, TruthTable_t table);
static void TestCompose(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2);
static void TestSubstitute(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2, Cal_Bdd f3, TruthTable_t table3);
static void TestVarSubstitute(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2, Cal_Bdd f3, TruthTable_t table3);
static void TestSwapVars(Cal_BddManager bddManager, Cal_Bdd f, TruthTable_t table);
static void TestMultiwayAnd(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2, Cal_Bdd f3, TruthTable_t table3);
static void TestMultiwayOr(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2, Cal_Bdd f3, TruthTable_t table3);
static void TestMultiwayLarge(Cal_BddManager bddManager, int numBdds);
static void TestArrayOp(Cal_BddManager bddManager, int numBdds);
static void TestInterImpl(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2);
static void TestQnt(Cal_BddManager bddManager, Cal_Bdd f, TruthTable_t table, int bfZeroBFPlusDFOne, int cacheExistsResultsFlag, int cacheOrResultsFlag);
static void TestAssoc(Cal_BddManager bddManager, Cal_Bdd f, TruthTable_t table);
static void TestRelProd(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2, int bfZeroBFPlusDFOne, int cacheRelProdResultsFlag, int cacheAndResultsFlag, int cacheOrResultsFlag);
static void TestReduce(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2);
static void TestGenCof(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2);
static void TestSize(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2);
static void TestSatisfy(Cal_BddManager bddManager, Cal_Bdd f, TruthTable_t table);
static void TestPipeline(Cal_BddManager bddManager, Cal_Bdd f1, TruthTable_t table1, Cal_Bdd f2, TruthTable_t table2, Cal_Bdd f3, TruthTable_t table3);
static void TestDump(Cal_BddManager bddManager, Cal_Bdd f);
static void TestReorderBlock(Cal_BddManager bddManager, TruthTable_t table, Cal_Bdd f);
static void TestReorder(Cal_BddManager bddManager, TruthTable_t table, Cal_Bdd f);
static void handler(int ignored);
static void RandomTests(int numVars, int iterations);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
main(int  argc, char ** argv)
{
  int numVars, iterations;
  if(argc < 2){
    iterations = ITERATIONS;
  }
  else{
    iterations = atoi(argv[1]);
  }
  if(argc < 3){
    TT_VARS = 5;
  }
  else {
    TT_VARS = atoi(argv[2]);
  }
  
  CalUtilSRandom((long)1);
  numVars = TT_VARS;
  RandomTests(numVars, iterations);
  return 0;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static double
asDouble(
  CalAddress_t  v1,
  CalAddress_t  v2)
{
  asDoubleSpace[0] = v1;
  asDoubleSpace[1] = v2;
  return (*(double *)asDoubleSpace);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
asAddress(
  double  n,
  CalAddress_t * r1,
  CalAddress_t * r2)
{
  (*(double *)asDoubleSpace)=n;
  *r1 = asDoubleSpace[0];
  *r2 = asDoubleSpace[1];
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static char *
terminalIdFn(
  Cal_BddManager bddManager,
  CalAddress_t  v1,
  CalAddress_t  v2,
  Cal_Pointer_t  pointer)
{
  static char result[100];
  sprintf(result, "%g", asDouble(v1, v2));
  return (result);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
PrintBdd(
  Cal_BddManager bddManager,
  Cal_Bdd  f)
{
  Cal_BddPrintBdd(bddManager, f, Cal_BddNamingFnNone, 
		  (Cal_TerminalIdFn_t) terminalIdFn,
                  (Cal_Pointer_t)0, stderr); 
}
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
Error(char *op, Cal_BddManager bddManager, Cal_Bdd result,
      Cal_Bdd expected, ...)
{
  int i;
  va_list ap;
  Cal_Bdd userBdd;
  Cal_Bdd_t f;

  va_start(ap, expected);

  fprintf(stderr, "\nError: operation %s:\n", op);
  i=0;
  while (1) {
    if (userBdd = va_arg(ap, Cal_Bdd)){
	  ++i;
	  fprintf(stderr, "Argument %d:\n", i);
	  Cal_BddFunctionPrint(bddManager, userBdd, "a");
	}
    else
      break;
  }
  fprintf(stderr, "Expected result:\n");
  Cal_BddFunctionPrint(bddManager, expected, "a");
  fprintf(stderr, "Result:\n");
  Cal_BddFunctionPrint(bddManager, result, "a");
  va_end(ap);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static TruthTable_t
Cofactor(TruthTable_t  table, int  var, int  value)
{
  int shift;
  
  shift = 1 << (TT_VARS-var-1);
  if(value) {
    table &= cofactorMasks[var];
    table |= table >> shift;
  }
  else {
    table &= ~cofactorMasks[var];
    table |= table << shift;
  }
  return (table);
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static Cal_Bdd
Decode(int  var, TruthTable_t  table)
{
  Cal_Bdd temp1, temp2;
  Cal_Bdd result;
  Cal_Bdd left, right;
  Cal_Bdd varBdd;

  if(var == TT_VARS){
    if(table & 0x1){
      result = Cal_BddOne(bddManager);
    }
    else{
      result = Cal_BddZero(bddManager);
    }
  }
  else{
    temp1 = Decode(var+1, table >> (1 << (TT_VARS-var-1)));
    temp2 = Decode(var+1, table);
    left = Cal_BddAnd(bddManager, vars[var], temp1);
    varBdd = Cal_BddNot(bddManager, vars[var]);
    right = Cal_BddAnd(bddManager, varBdd, temp2);
    result = Cal_BddOr(bddManager, left, right);
    /*
    result = Cal_BddITE(bddManager, vars[var], temp1, temp2);
    */
    Cal_BddFree(bddManager, left);
    Cal_BddFree(bddManager, right);
    Cal_BddFree(bddManager, temp1);
    Cal_BddFree(bddManager, temp2);
    Cal_BddFree(bddManager, varBdd);
  }
  return (result);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestAnd(Cal_BddManager bddManager, Cal_Bdd  f1, TruthTable_t  table1,
        Cal_Bdd  f2, TruthTable_t  table2)
{
  Cal_Bdd result;
  TruthTable_t resulttable;
  Cal_Bdd expected;

  result = Cal_BddAnd(bddManager, f1, f2);
  resulttable = table1 & table2;
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result, expected)){
    Error("AND", bddManager, result, expected, f1, f2, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestNand(Cal_BddManager bddManager, Cal_Bdd  f1, TruthTable_t  table1,
        Cal_Bdd  f2, TruthTable_t  table2)
{
  Cal_Bdd result;
  TruthTable_t resulttable;
  Cal_Bdd expected;

  result = Cal_BddNand(bddManager, f1, f2);
  resulttable = ~(table1 & table2);
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result, expected)){
    Error("NAND", bddManager, result, expected, f1, f2, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestOr(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2)
{
  Cal_Bdd result;
  TruthTable_t resulttable;
  Cal_Bdd expected;

  result = Cal_BddOr(bddManager, f1, f2);
  resulttable = table1 | table2;
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result, expected)){
    Error("OR", bddManager,result, expected, f1, f2, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestITE(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2,
  Cal_Bdd  f3,
  TruthTable_t  table3)
{
  Cal_Bdd result;
  TruthTable_t resultTable;
  Cal_Bdd expected;

  result = Cal_BddITE(bddManager, f1, f2, f3);
  resultTable = (table1 & table2) | (~table1 & table3);
  expected = EncodingToBdd(resultTable);
  if(Cal_BddIsEqual(bddManager, result, expected) == 0){
    Error("ITE", bddManager, result, expected, f1, f2, f3,
          (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestXor(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2)
{
  Cal_Bdd result;
  TruthTable_t resulttable;
  Cal_Bdd expected;

  result = Cal_BddXor(bddManager, f1, f2);
  resulttable = table1 ^ table2;
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result, expected)){
    Error("XOR", bddManager, result, expected, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestIdNot(
  Cal_BddManager bddManager,
  Cal_Bdd  f,
  TruthTable_t  table)
{
  Cal_Bdd result;
  TruthTable_t resulttable;
  Cal_Bdd expected;

  result = Cal_BddNot(bddManager, f);
  resulttable = ~table;
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result, expected)){
    Error("Not", bddManager, result, expected, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
  result = Cal_BddIdentity(bddManager, f);
  resulttable = table;
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result, expected)){
    Error("Identity", bddManager, result, expected, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
  
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestCompose(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2)
{
  int var;
  Cal_Bdd result, expected;
  TruthTable_t resulttable;
  

  var = (int)(((long)CalUtilRandom())%TT_VARS);

  result = Cal_BddCompose(bddManager, vars[var], vars[var], Cal_BddOne(bddManager));
  if(!Cal_BddIsEqual(bddManager, result, Cal_BddOne(bddManager))){
    Cal_BddFunctionPrint(bddManager, result, "Compose"); 
  }

  result = Cal_BddCompose(bddManager, vars[var], vars[var], Cal_BddZero(bddManager));
  if(!Cal_BddIsEqual(bddManager, result, Cal_BddZero(bddManager))){
    Cal_BddFunctionPrint(bddManager, result, "Compose"); 
  }

  result = Cal_BddCompose(bddManager, f1, vars[var], Cal_BddOne(bddManager));
  resulttable = Cofactor(table1, var, 1);
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result,expected)){
    Error("Restrict 1", bddManager, result, expected, f1, vars[var],
          (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);

  result = Cal_BddCompose(bddManager, f1, vars[var], Cal_BddZero(bddManager));
  resulttable = Cofactor(table1, var, 0);
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result, expected)){
    Error("Restrict 0", bddManager, result, expected, f1, vars[var],
          (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);

  result = Cal_BddCompose(bddManager, f1, vars[var], f2);
  resulttable = (table2 & Cofactor(table1, var, 1)) |
      (~table2 & Cofactor(table1, var, 0));
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result, expected)){
    Error("Compose", bddManager, result, expected, f1, vars[var],
          f2, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestSubstitute(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2,
  Cal_Bdd  f3,
  TruthTable_t  table3)
{
  int var1, var2;
  Cal_Bdd associationInfo[6];
  Cal_Bdd result;
  TruthTable_t resulttable;
  TruthTable_t temp1, temp2, temp3, temp4;
  Cal_Bdd expected;
  int assocId;
  
  var1 = (int)(((long)CalUtilRandom())%TT_VARS);
  do{
    var2 = (int)(((long)CalUtilRandom())%TT_VARS);
  }while (var1 == var2);

  associationInfo[0] = vars[var1];
  associationInfo[1] = f2;
  associationInfo[2] = vars[var2];
  associationInfo[3] = f3;
  associationInfo[4] = (Cal_Bdd) 0;
  associationInfo[5] = (Cal_Bdd) 0;

  assocId = Cal_AssociationInit(bddManager, associationInfo, 1);
  Cal_AssociationSetCurrent(bddManager, assocId);

  result = Cal_BddSubstitute(bddManager, f1);
  temp1 = Cofactor(Cofactor(table1, var1, 1), var2, 1);
  temp2 = Cofactor(Cofactor(table1, var1, 1), var2, 0);
  temp3 = Cofactor(Cofactor(table1, var1, 0), var2, 1);
  temp4 = Cofactor(Cofactor(table1, var1, 0), var2, 0);
  resulttable = table2 & table3 & temp1;
  resulttable |= table2 & ~table3 & temp2;
  resulttable |= ~table2 & table3 & temp3;
  resulttable |= ~table2 & ~table3 & temp4;
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result, expected)){
    Error("substitute", bddManager, result, expected,
        f1, vars[var1], f2, vars[var2], f3, (Cal_Bdd) 0);
  }
  /*Cal_AssociationQuit(bddManager, assocId);*/
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestVarSubstitute(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2,
  Cal_Bdd  f3,
  TruthTable_t  table3)
{
  int var1, var2, var3, var4;
  Cal_Bdd associationInfo[6];
  Cal_Bdd result1, result2;
  TruthTable_t resulttable;
  TruthTable_t temp1, temp2, temp3, temp4;
  Cal_Bdd expected;
  int assocId;
  
  var1 = (int)(((long)CalUtilRandom())%TT_VARS);
  do{
    var3 = (int)(((long)CalUtilRandom())%TT_VARS);
  }while (var1 == var3);

  var2 = (int)(((long)CalUtilRandom())%TT_VARS);
  do{
    var4 = (int)(((long)CalUtilRandom())%TT_VARS);
  }while (var2 == var4);

  /*
  f1 = vars[0];
  table1 = cofactorMasks[0];
  */
  associationInfo[0] = vars[var1];
  associationInfo[1] = vars[var3];
  associationInfo[2] = vars[var2];
  associationInfo[3] = vars[var4];
  associationInfo[4] = (Cal_Bdd) 0;
  associationInfo[5] = (Cal_Bdd) 0;

  assocId = Cal_AssociationInit(bddManager, associationInfo, 1);
  Cal_AssociationSetCurrent(bddManager, assocId);

  result1 = Cal_BddVarSubstitute(bddManager, f1);
  result2 = Cal_BddSubstitute(bddManager, f1);
  temp1 = Cofactor(Cofactor(table1, var2, 1), var1, 1);
  temp2 = Cofactor(Cofactor(table1, var2, 1), var1, 0);
  temp3 = Cofactor(Cofactor(table1, var2, 0), var1, 1);
  temp4 = Cofactor(Cofactor(table1, var2, 0), var1, 0);
  resulttable = cofactorMasks[var3] & cofactorMasks[var4] & temp1;
  resulttable |= ~cofactorMasks[var3] & cofactorMasks[var4] & temp2;
  resulttable |= cofactorMasks[var3] & ~cofactorMasks[var4] & temp3;
  resulttable |= ~cofactorMasks[var3] & ~cofactorMasks[var4] & temp4;
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result1, result2)){
    Error("var substitute and substitute differ", bddManager, result1, result2,
        f1, vars[var1], vars[var3], vars[var2], vars[var4],
          (Cal_Bdd) 0); 
  }
  if(!Cal_BddIsEqual(bddManager, result1, expected)){
    Error("var substitute", bddManager, result1, expected,
        f1, vars[var1], vars[var3], vars[var2], vars[var4],
          (Cal_Bdd) 0); 
  }
  /*Cal_AssociationQuit(bddManager, assocId);*/
  Cal_BddFree(bddManager, result1);
  Cal_BddFree(bddManager, result2);
  Cal_BddFree(bddManager, expected);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestSwapVars(
  Cal_BddManager bddManager,
  Cal_Bdd  f,
  TruthTable_t  table)
{
  int var1, var2;
  Cal_Bdd result;
  TruthTable_t resulttable;
  TruthTable_t temp1, temp2, temp3, temp4;
  Cal_Bdd expected;

  var1 = (int)(((long)CalUtilRandom())%TT_VARS);
  var2 = (int)(((long)CalUtilRandom())%TT_VARS);
  result = Cal_BddSwapVars(bddManager, f, vars[var1], vars[var2]);
  temp1 = Cofactor(Cofactor(table, var1, 1), var2, 1);
  temp2 = Cofactor(Cofactor(table, var1, 1), var2, 0);
  temp3 = Cofactor(Cofactor(table, var1, 0), var2, 1);
  temp4 = Cofactor(Cofactor(table, var1, 0), var2, 0);
  resulttable = cofactorMasks[var2] & cofactorMasks[var1] & temp1;
  resulttable |= cofactorMasks[var2] & ~cofactorMasks[var1] & temp2;
  resulttable |= ~cofactorMasks[var2] & cofactorMasks[var1] & temp3;
  resulttable |= ~cofactorMasks[var2] & ~cofactorMasks[var1] & temp4;
  expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result, expected)){
    Error("swap variables", bddManager, result, expected, 
        f, vars[var1], vars[var2], (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestMultiwayAnd(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2,
  Cal_Bdd  f3,
  TruthTable_t  table3)
{
	Cal_Bdd result;
	TruthTable_t resulttable;
	Cal_Bdd expected;
	Cal_Bdd *calBddArray;

    calBddArray = Cal_MemAlloc(Cal_Bdd, 4);
    calBddArray[0] = f1;
    calBddArray[1] = f2;
    calBddArray[2] = f3;
    calBddArray[3] = (Cal_Bdd) 0;
	result = Cal_BddMultiwayAnd(bddManager, calBddArray);
	resulttable = table1 & table2 & table3;
	expected = EncodingToBdd(resulttable);
    if(!Cal_BddIsEqual(bddManager, result, expected)){
      Error("Multiway And", bddManager, result, expected,
            (Cal_Bdd) 0);
    }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
  Cal_MemFree(calBddArray);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestMultiwayOr(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2,
  Cal_Bdd  f3,
  TruthTable_t  table3)
{
  Cal_Bdd result;
  TruthTable_t resulttable;
  Cal_Bdd expected;
  Cal_Bdd *calBddArray;
  
  calBddArray = Cal_MemAlloc(Cal_Bdd, 4);
  calBddArray[0] = f1;
  calBddArray[1] = f2;
  calBddArray[2] = f3;
  calBddArray[3] = (Cal_Bdd) 0;
  result = Cal_BddMultiwayOr(bddManager, calBddArray);
  resulttable = table1 | table2 | table3;
	expected = EncodingToBdd(resulttable);
  if(!Cal_BddIsEqual(bddManager, result, expected)){
    Error("Multiway Or", bddManager, result, expected, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
  Cal_MemFree(calBddArray);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestMultiwayLarge(
  Cal_BddManager bddManager,
  int  numBdds)
{
  TruthTable_t table, andResulttable, orResulttable;
  Cal_Bdd f, andResult, orResult, andExpected, orExpected;
  int i;
  Cal_Bdd *calBddArray;
  
  andResulttable = ~0x0;
  orResulttable = 0x0;
  calBddArray = Cal_MemAlloc(Cal_Bdd, numBdds+1);
  for (i=0; i<numBdds; i++){
    table = (TruthTable_t)CalUtilRandom();
    f = EncodingToBdd(table);
    calBddArray[i] = f;
    andResulttable &= table;
    orResulttable |= table;
  }
  calBddArray[numBdds] = (Cal_Bdd) 0;
  andResult = Cal_BddMultiwayAnd(bddManager, calBddArray);
  orResult = Cal_BddMultiwayOr(bddManager, calBddArray);
  andExpected = EncodingToBdd(andResulttable);
  orExpected = EncodingToBdd(orResulttable);
  if(!Cal_BddIsEqual(bddManager, andResult, andExpected)){
    Error("Multiway And", bddManager, andResult, andExpected,
          (Cal_Bdd) 0);
  }
  if(!Cal_BddIsEqual(bddManager, orResult, orExpected)){
    Error("Multiway Or", bddManager, andResult, andExpected,
          (Cal_Bdd) 0);
  }
  for (i=0; i<numBdds; i++) Cal_BddFree(bddManager, calBddArray[i]);
  Cal_MemFree(calBddArray);
  Cal_BddFree(bddManager, andResult);
  Cal_BddFree(bddManager, andExpected);
  Cal_BddFree(bddManager, orResult);
  Cal_BddFree(bddManager, orExpected);
}
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestArrayOp(Cal_BddManager bddManager,  int  numBdds)
{
  TruthTable_t fTable, gTable;
  Cal_Bdd f, g, *andExpectedArray, *orExpectedArray, *calBddArray;
  Cal_Bdd *andResultArray, *orResultArray;
  int i;
  
  calBddArray = Cal_MemAlloc(Cal_Bdd, 2*numBdds+1);
  andExpectedArray = Cal_MemAlloc(Cal_Bdd, numBdds);
  orExpectedArray = Cal_MemAlloc(Cal_Bdd, numBdds);
  calBddArray[numBdds<<1] = (Cal_Bdd) 0;

  for (i=0; i<numBdds; i++){
    fTable = (TruthTable_t)CalUtilRandom();
    gTable = (TruthTable_t)CalUtilRandom();
    f = EncodingToBdd(fTable);
    g = EncodingToBdd(gTable);
    calBddArray[i<<1] = f;
    calBddArray[(i<<1)+1] = g;
    andExpectedArray[i] = EncodingToBdd(fTable & gTable);
    orExpectedArray[i] = EncodingToBdd(fTable | gTable);
  }
  
  andResultArray = Cal_BddPairwiseAnd(bddManager, calBddArray);
  orResultArray = Cal_BddPairwiseOr(bddManager, calBddArray);

  for (i=0; i<numBdds; i++){
    if(!Cal_BddIsEqual(bddManager, andResultArray[i], andExpectedArray[i])){
      Error("Array OR", bddManager, andResultArray[i], andExpectedArray[i],
            (Cal_Bdd) 0);
      break;
    }
  }

  for (i=0; i<numBdds; i++){
    if(!Cal_BddIsEqual(bddManager, orResultArray[i], orExpectedArray[i])){
      Error("Array OR", bddManager, orResultArray[i], orExpectedArray[i],
            (Cal_Bdd) 0);
      break;
    }
  }
  for (i=0; i<numBdds; i++){
    Cal_BddFree(bddManager, calBddArray[i<<1]);
    Cal_BddFree(bddManager, calBddArray[(i<<1)+1]);
    Cal_BddFree(bddManager, andExpectedArray[i]);
    Cal_BddFree(bddManager, orExpectedArray[i]);
    Cal_BddFree(bddManager, andResultArray[i]);
    Cal_BddFree(bddManager, orResultArray[i]);
  }
  Cal_MemFree(calBddArray);
  Cal_MemFree(andExpectedArray);
  Cal_MemFree(orExpectedArray);
  Cal_MemFree(andResultArray);
  Cal_MemFree(orResultArray);
}
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestInterImpl(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2)
{
  Cal_Bdd result;
  TruthTable_t resulttable;
  Cal_Bdd expected;
  Cal_Bdd impliesResult;

  result = Cal_BddIntersects(bddManager, f1, f2);
  resulttable = table1 & table2;
  expected = EncodingToBdd(resulttable);
  impliesResult = Cal_BddImplies(bddManager, result, expected);
  if(Cal_BddIsBddZero(bddManager, impliesResult) == 0){
    Error("intersection test", bddManager, result, expected, f1, f2,
          (Cal_Bdd) 0); 
  }
  Cal_BddFree(bddManager, impliesResult);
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestQnt(Cal_BddManager bddManager, Cal_Bdd  f, TruthTable_t  table, int
        bfZeroBFPlusDFOne, int cacheExistsResultsFlag, int cacheOrResultsFlag)
{
  int var1, var2;
  Cal_Bdd assoc[3];
  Cal_Bdd temp, result, expected;
  TruthTable_t  resultTable;
  int associationId;
  
  var1= (int)(((long)CalUtilRandom())%TT_VARS);
  do
    var2= (int)(((long)CalUtilRandom())%TT_VARS);
  while (var1 == var2);
  assoc[0] = vars[var1];
  assoc[1] = vars[var2];
  assoc[2] = (Cal_Bdd) 0;
  associationId = Cal_AssociationInit(bddManager, assoc, 0);
  Cal_AssociationSetCurrent(bddManager, associationId);
  result = Cal_BddExists(bddManager, f);
  resultTable = Cofactor(table, var1, 1) | Cofactor(table, var1, 0);
  resultTable = Cofactor(resultTable, var2, 1) | Cofactor(resultTable,
                                                          var2, 0); 
  expected = EncodingToBdd(resultTable);
  if(Cal_BddIsEqual(bddManager, result, expected) == 0){
    Error("quantification", bddManager, result, expected, f, vars[var1],
          vars[var2], (Cal_Bdd) 0);
  }
  /*Cal_AssociationQuit(bddManager, associationId);*/
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestAssoc(Cal_BddManager bddManager, Cal_Bdd  f, TruthTable_t  table)
{
  int var1, var2;
  Cal_Bdd assoc[3];
  Cal_Bdd temp, result, expected;
  TruthTable_t  resultTable;
  int associationId;
  
  assoc[0] = (Cal_Bdd) 0;
  associationId = Cal_AssociationInit(bddManager, assoc, 0);
  Cal_AssociationSetCurrent(bddManager, associationId);
  result = Cal_BddExists(bddManager, f);
  expected = Cal_BddIdentity(bddManager, f);
  if(Cal_BddIsEqual(bddManager, result, expected) == 0){
    Error("quantification", bddManager, result, expected, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestRelProd(Cal_BddManager bddManager, Cal_Bdd  f1, TruthTable_t
            table1, Cal_Bdd  f2, TruthTable_t  table2, int bfZeroBFPlusDFOne,
            int cacheRelProdResultsFlag, int cacheAndResultsFlag, int
            cacheOrResultsFlag) 
{
  int var1, var2;
  Cal_Bdd assoc[3];
  Cal_Bdd result;
  TruthTable_t resultTable;
  Cal_Bdd expected;
  int assocId;
  
  var1=(int)(((long)CalUtilRandom())%TT_VARS);
  do
    var2=(int)(((long)CalUtilRandom())%TT_VARS);
  while (var1 == var2);
  assoc[0] = vars[var1];
  assoc[1] = vars[var2];
  assoc[2] = (Cal_Bdd) 0;
  assocId = Cal_AssociationInit(bddManager, assoc, 0);
  Cal_AssociationSetCurrent(bddManager, assocId);
  result = Cal_BddRelProd(bddManager, f1, f2);
  table1 &= table2;
  resultTable = Cofactor(table1, var1, 1) | Cofactor(table1, var1, 0);
  resultTable = Cofactor(resultTable, var2, 1) | Cofactor(resultTable, var2, 0);
  expected = EncodingToBdd(resultTable);
  if(Cal_BddIsEqual(bddManager, result, expected) == 0){
    Error("relational product", bddManager, result, expected, f1, f2,
          vars[var1], vars[var2], (Cal_Bdd) 0);
  }
  /*Cal_AssociationQuit(bddManager, assocId);*/
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestReduce(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2)
{
  Cal_Bdd result;
  Cal_Bdd temp1, temp2, temp3;

  result = Cal_BddReduce(bddManager, f1, f2);
  temp1 = Cal_BddXnor(bddManager, result, f1);
  temp2 = Cal_BddNot(bddManager, f2);
  temp3 = Cal_BddOr(bddManager, temp1, temp2);
  if(Cal_BddIsBddOne(bddManager, temp3) == 0){
    Error("d.c. comparison of reduce", bddManager, temp3,
          Cal_BddOne(bddManager), f1, f2, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, temp1);
  Cal_BddFree(bddManager, temp2);
  Cal_BddFree(bddManager, temp3);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestGenCof(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2)
{
  int var1, var2;
  Cal_Bdd result, temp1, temp2, temp3, expected;
  TruthTable_t resultTable;

  result = Cal_BddCofactor(bddManager, f1, f2);
  temp1 = Cal_BddXnor(bddManager, result, f1);
  temp2 = Cal_BddNot(bddManager, f2);
  temp3 = Cal_BddOr(bddManager, temp1, temp2);
  if (Cal_BddIsBddOne(bddManager, temp3) == 0){
    Error("d.c. comparison of generalized cofactor", bddManager,
          temp3, Cal_BddOne(bddManager), f1, f2, (Cal_Bdd) 0);
  }

  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, temp1);
  Cal_BddFree(bddManager, temp2);
  Cal_BddFree(bddManager, temp3);
  var1=(int)(((long)CalUtilRandom())%TT_VARS);
  do
    var2=(int)(((long)CalUtilRandom())%TT_VARS);
  while (var1 == var2);
  temp1 = Cal_BddNot(bddManager, vars[var2]);
  temp2 = Cal_BddAnd(bddManager, vars[var1], temp1);
  Cal_BddFree(bddManager, temp1);
  result = Cal_BddCofactor(bddManager, f1, temp2);
  resultTable = Cofactor(Cofactor(table1, var1, 1), var2, 0);
  expected = EncodingToBdd(resultTable);
  if (Cal_BddIsEqual(bddManager, result, expected) == 0){
    Error("generalized cofactor", bddManager, result, expected, f1,
          temp2, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
  Cal_BddFree(bddManager, temp2);
}
/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestSize(
  Cal_BddManager bddManager,
  Cal_Bdd  f1,
  TruthTable_t  table1,
  Cal_Bdd  f2,
  TruthTable_t  table2)
{
  int i;
  long size;
  long profile[MAX_TT_VARS+1];
  Cal_Bdd fs[3];

  size = Cal_BddSize(bddManager, f1, 1);
  Cal_BddProfile(bddManager, f1, profile, 1);
  for(i = 0; i < TT_VARS+1; i++){
    size -= profile[i];
  }
  if(size){
    fprintf(stderr, "\nError: size count vs. profile sum:\n");
    fprintf(stderr, "Argument:\n");
    Cal_BddFunctionPrint(bddManager, f1, "f1");
  }

  size = Cal_BddSize(bddManager, f1, 0);
  Cal_BddProfile(bddManager, f1, profile, 0);
  for(i = 0; i < TT_VARS+1; i++){
    size -= profile[i];
  }
  if(size){
    fprintf(stderr, "\nError: no negout size count vs. profile sum:\n");
    fprintf(stderr, "Argument:\n");
    Cal_BddFunctionPrint(bddManager, f1, "f1");
  }


  fs[0] = f1;
  fs[1] = f2;
  fs[2] = (Cal_Bdd) 0;

  size = Cal_BddSizeMultiple(bddManager, fs, 1);
  Cal_BddProfileMultiple(bddManager, fs, profile, 1);
  for(i = 0; i < TT_VARS+1; i++){
    size -= profile[i];
  }
  if(size){
    fprintf(stderr,"\nError: multiple size count vs. multiple profile sum:\n");
    fprintf(stderr, "Argument 1:\n");
    Cal_BddFunctionPrint(bddManager, f1, "f1");
    fprintf(stderr, "Argument 2:\n");
    Cal_BddFunctionPrint(bddManager, f2, "f2");
  }

  size = Cal_BddSizeMultiple(bddManager, fs, 0);
  Cal_BddProfileMultiple(bddManager, fs, profile, 0);
  for(i = 0; i < TT_VARS+1; i++){
    size -= profile[i];
  }
  if(size){
    fprintf(stderr,"\nError: multiple no negout size count vs. multiple profile sum:\n");
    fprintf(stderr, "Argument 1:\n");
    Cal_BddFunctionPrint(bddManager, f1, "f1");
    fprintf(stderr, "Argument 2:\n");
    Cal_BddFunctionPrint(bddManager, f2, "f2");
  }
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestSatisfy(
  Cal_BddManager bddManager,
  Cal_Bdd  f,
  TruthTable_t  table)
{
  int var1, var2;
  Cal_Bdd assoc[MAX_TT_VARS+1];
  Cal_Bdd result;
  Cal_Bdd temp1, temp2, temp3;
  int assocId;
  
  if(Cal_BddIsBddZero(bddManager, f)){
    return;
  }
  result = Cal_BddSatisfy(bddManager, f);
  temp1 = Cal_BddNot(bddManager, f);
  temp2 = Cal_BddIntersects(bddManager, temp1, result);
  if(!Cal_BddIsBddZero(bddManager, temp2)){
    Error("intersection of satisfy result with negated argument",
        bddManager, temp2, Cal_BddZero(bddManager), f, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, temp1);
  Cal_BddFree(bddManager, temp2);

  var1 = (int)(((long)CalUtilRandom())%TT_VARS);
  do{
    var2 = (int)(((long)CalUtilRandom())%TT_VARS);
  }while (var1 == var2);
  assoc[0] = vars[var1];
  assoc[1] = vars[var2];
  assoc[2] = (Cal_Bdd) 0;
  assocId = Cal_AssociationInit(bddManager, assoc, 0);
  Cal_AssociationSetCurrent(bddManager, assocId);
  temp1 = Cal_BddSatisfySupport(bddManager, result);
  temp2 = Cal_BddNot(bddManager, result);
  temp3 = Cal_BddIntersects(bddManager, temp2, temp1);
  if(!Cal_BddIsBddZero(bddManager, temp3)){
    Error("intersection of satisfy support result with negated argument",
        bddManager, temp3, Cal_BddZero(bddManager), result,
        (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, temp1);
  Cal_BddFree(bddManager, temp2);
  Cal_BddFree(bddManager, temp3);
  temp1 = Cal_BddCompose(bddManager, f, vars[var1], Cal_BddZero(bddManager));
  temp2 = Cal_BddCompose(bddManager, f, vars[var1], Cal_BddOne(bddManager));
  if(Cal_BddSatisfyingFraction(bddManager, temp1) + 
      Cal_BddSatisfyingFraction(bddManager, temp2) !=
      2.0 * Cal_BddSatisfyingFraction(bddManager, f)){
    fprintf(stderr, "\nError: operation satisfying fraction:\n");
    fprintf(stderr, "Argument:\n");
    Cal_BddFunctionPrint(bddManager, f, "f");
    fprintf(stderr, "Cofactor on:\n");
    Cal_BddFunctionPrint(bddManager, vars[var1], "var");
  }
  /*Cal_AssociationQuit(bddManager, assocId);*/
  Cal_BddFree(bddManager, temp1);
  Cal_BddFree(bddManager, temp2);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestPipeline(Cal_BddManager bddManager,
             Cal_Bdd  f1,
             TruthTable_t  table1,
             Cal_Bdd  f2,
             TruthTable_t  table2,
             Cal_Bdd  f3,
             TruthTable_t  table3)
{
  Cal_Bdd temp1, temp2, temp3, temp4, temp5, result, expected;
  TruthTable_t table;

  Cal_PipelineInit(bddManager, CAL_AND);
  Cal_PipelineSetDepth(bddManager, 0);
  temp1 = Cal_PipelineCreateProvisionalBdd(bddManager, f1, f2);
  temp2 = Cal_PipelineCreateProvisionalBdd(bddManager, f1, f3);
  temp3 = Cal_PipelineCreateProvisionalBdd(bddManager, f1, temp1);
  temp4 = Cal_PipelineCreateProvisionalBdd(bddManager, f2, temp2);
  temp5 = Cal_PipelineCreateProvisionalBdd(bddManager, temp3, temp4);
  result = Cal_PipelineCreateProvisionalBdd(bddManager, temp4, temp5);
  Cal_PipelineExecute(bddManager);
  result = Cal_PipelineUpdateProvisionalBdd(bddManager, result);
  Cal_PipelineQuit(bddManager);

  table = table1 & table2 & table3;
  expected = EncodingToBdd(table);

  if (Cal_BddIsEqual(bddManager, result, expected) == 0){
    Error("pipeline", bddManager, result, expected, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestDump(Cal_BddManager bddManager, Cal_Bdd f)
{
  FILE *fp;
  int i, j;
  Cal_Bdd dumpVars[MAX_TT_VARS];
  Cal_Bdd temp, result;
  int err;

  tmpnam(filename);
  if (!(fp=fopen(filename, "w+")))
    {
      fprintf(stderr, "could not open temporary file %s\n", filename);
      exit(1);
    }
  unlink(filename);
  for (i=0; i < TT_VARS; ++i)
    dumpVars[i]=vars[i];
  dumpVars[i]= (Cal_Bdd) 0;
  for (i=0; i < TT_VARS-1; ++i)
    {
      j=i+(int)(((long)CalUtilRandom())%(TT_VARS-i));
      temp=dumpVars[i];
      dumpVars[i]=dumpVars[j];
      dumpVars[j]=temp;
    }
  if (!Cal_BddDumpBdd(bddManager, f, dumpVars, fp))
    {
      fprintf(stderr, "Error: dump failure:\n");
      fprintf(stderr, "Argument:\n");
      PrintBdd(bddManager, f);
      fclose(fp);
      return;
    }
  rewind(fp);
  if (!(result=Cal_BddUndumpBdd(bddManager, dumpVars, fp, &err)) || err)
    {
      fprintf(stderr, "Error: undump failure: code %d:\n", err);
      fprintf(stderr, "Argument:\n");
      PrintBdd(bddManager, f);
      fclose(fp);
      return;
    }
  fclose(fp);
  if (result != f)
    Error("dump/undump", bddManager, result, f, f, (Cal_Bdd) 0);
  Cal_BddFree(bddManager, result);
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestReorderBlock(Cal_BddManager bddManager, TruthTable_t table, Cal_Bdd f)
{
  Cal_Bdd newFunction;
  Cal_Block block1, block2, block3, block4;
  int i;
  
  /*if (CalUtilRandom()&0x1){*/
  if (1){
    fprintf(stdout, "Using Window\n");
    Cal_BddDynamicReordering(bddManager, CAL_REORDER_WINDOW);
  }
  else{
    fprintf(stdout, "Using Sift\n");
    Cal_BddDynamicReordering(bddManager, CAL_REORDER_SIFT);
  }
  /* Create some blocks */
  block1 = Cal_BddNewVarBlock(bddManager,
                              vars[bddManager->indexToId[0]-1],
                              4); 
  block2 = Cal_BddNewVarBlock(bddManager,
                              vars[bddManager->indexToId[4]-1],
                              4);
  block3 = Cal_BddNewVarBlock(bddManager,
                              vars[bddManager->indexToId[8]-1],
                              4);
  Cal_BddVarBlockReorderable(bddManager, block2, 1);
  Cal_BddReorder(bddManager);
  newFunction = EncodingToBdd(table);
  if (Cal_BddIsEqual(bddManager, f, newFunction) == 0){
    Error("Reordering (window)", bddManager, newFunction, f, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, newFunction);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
TestReorder(Cal_BddManager bddManager, TruthTable_t table, Cal_Bdd f)
{
  Cal_Bdd newFunction;
  
  if (CalUtilRandom()&0x1){
    fprintf(stdout, "Using Window\n");
    Cal_BddDynamicReordering(bddManager, CAL_REORDER_WINDOW);
  }
  else{
    fprintf(stdout, "Using Sift\n");
    Cal_BddDynamicReordering(bddManager, CAL_REORDER_SIFT);
  }
  if (CalUtilRandom()&0x1){
    bddManager->reorderMethod = CAL_REORDER_METHOD_BF;
  }
  else{
    bddManager->reorderMethod = CAL_REORDER_METHOD_DF;
  }
  Cal_BddReorder(bddManager);
  newFunction = EncodingToBdd(table);
  if (Cal_BddIsEqual(bddManager, f, newFunction) == 0){
    Error("Reordering (window)", bddManager, newFunction, f, (Cal_Bdd) 0);
  }
  Cal_BddFree(bddManager, newFunction);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
handler(int ignored)
{
  printf("arthimetic exception ############\n");
}




/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
RandomTests(int numVars, int  iterations)
{
  int i, j, seed;
  TruthTable_t table1, table2, table3, table;
  Cal_Bdd f1, f2, f3, f4, result, f, g;
  Cal_Bdd *outputBddArray;
  int associationId;
  CalAssociation_t *assoc, *nextAssoc;
  Cal_Block block1, block2;
  
  signal(SIGFPE, handler);
  
  printf("Random operation tests...\n");
  bddManager  = Cal_BddManagerInit();
  seed = 1;
  /*
  (void) time((time_t *)&seed);
  */
  CalUtilSRandom((long)seed);

  for(i = 0; i < numVars; ++i){
    vars[i] = Cal_BddManagerCreateNewVarLast(bddManager);
  }
  
  /*
  f1 = Cal_BddAnd(bddManager, vars[1], vars[2]);
  f2 = Cal_BddAnd(bddManager, vars[1], vars[0]);
  f3 = Cal_BddOr(bddManager, f1, f2);
  Cal_BddFree(bddManager, f1);
  Cal_BddFree(bddManager, f2);
  Cal_BddDynamicReordering(bddManager, Cal_BddReorderSift);
  fprintf(stdout,"Original function:\n");
  Cal_BddFunctionPrint(bddManager, f3, "a");
  Cal_BddReorderNew(bddManager);
  f1 = Cal_BddAnd(bddManager, vars[1], vars[2]);
  f2 = Cal_BddAnd(bddManager, vars[1], vars[0]);
  f4 = Cal_BddOr(bddManager, f1, f2);
  Cal_BddFree(bddManager, f1);
  Cal_BddFree(bddManager, f2);
  fprintf(stdout,"New function:\n");
  Cal_BddFunctionPrint(bddManager, f4, "a");
  Cal_Assert(Cal_BddIsEqual(bddManager, f3, f4));
  Cal_BddFree(bddManager, f3);
  Cal_BddFree(bddManager, f4);
  */

/*
  block1 = Cal_BddNewVarBlock(bddManager,
                              vars[0], 2);
  block2 = Cal_BddNewVarBlock(bddManager, vars[3], 2);
  Cal_BddVarBlockReorderable(bddManager, block2, 1);
  */
  for (i = 0; i < iterations; i++){
    Cal_Bdd result;
    printf("Iteration %3d\n", i);
    table1 = (TruthTable_t)CalUtilRandom();
    table2 = (TruthTable_t)CalUtilRandom();
    table3 = (TruthTable_t)CalUtilRandom();
    f1 = EncodingToBdd(table1);
    f2 = EncodingToBdd(table2);
    f3 = EncodingToBdd(table3);

    /* The following tests will fail if you do not use 5 variables */
    if (numVars == 5){
      TestGenCof(bddManager, f1, table1, f2, table2);
      TestSubstitute(bddManager, f1, table1, f2, table2, f3, table3);
      TestSwapVars(bddManager, f1, table1);
      TestCompose(bddManager, f1, table1, f2, table2);
      TestRelProd(bddManager, f1, table1, f2, table2, 0, 0, 0, 0);
      TestQnt(bddManager, f1, table1, 1, 1, 1);
      TestVarSubstitute(bddManager, f1, table1, f2, table2, f3,
                        table3);
    }
    /* The following can be tested for larger number of variables */
    TestAnd(bddManager,f1, table1, f2, table2);
    TestIdNot(bddManager, f1, table1);
    TestITE(bddManager, f1, table1, f2, table2, f3, table3);
    TestNand(bddManager,f1, table1, f2, table2);
    TestOr(bddManager, f1, table1, f2, table2);
    TestXor(bddManager,f1, table1, f2, table2);
    TestMultiwayOr(bddManager, f1, table1, f2, table2, f3, table3);
    TestMultiwayAnd(bddManager, f1, table1, f2, table2, f3, table3);
    TestArrayOp(bddManager, 10);
    TestInterImpl(bddManager, f1, table1, f2, table2);
    TestReduce(bddManager, f1, table1, f2, table2);
    TestSize(bddManager, f1, table1, f2, table2);
    TestSatisfy(bddManager, f1, table1);
    TestAssoc(bddManager, f1, table1);
    TestDump(bddManager, f1); 
    TestPipeline(bddManager, f1, table1, f2, table2, f3, table3);
    TestReorder(bddManager, table1, f1);
    Cal_BddFree(bddManager, f1);
    Cal_BddFree(bddManager, f2);
    Cal_BddFree(bddManager, f3);
  	if (i && (i % 10 == 0)) {
      Cal_BddManagerGC(bddManager);
      (void)CalPackNodes(bddManager);
    }
  }
  for(i = 0; i < numVars; ++i){
    Cal_BddFree(bddManager, vars[i]);
  }
  Cal_BddStats(bddManager, stdout);
  for(assoc = bddManager->associationList;
      assoc != Cal_Nil(CalAssociation_t); assoc = nextAssoc){
    nextAssoc = assoc->next;
    for (i=1; i <= bddManager->numVars; i++){
      if (CalBddIsBddNull(bddManager, assoc->varAssociation[i]) == 0){
        CalBddDcrRefCount(assoc->varAssociation[i]);
        assoc->varAssociation[i] = bddManager->bddNull;
        assoc->lastBddIndex = -1;
      }
    }
  }
  /* fix temporary association */
  assoc = bddManager->tempAssociation;
  for (i=1; i <= bddManager->numVars; i++){
    if (CalBddIsBddNull(bddManager, assoc->varAssociation[i]) == 0){
      CalBddDcrRefCount(assoc->varAssociation[i]);
      assoc->varAssociation[i] = bddManager->bddNull;
      assoc->lastBddIndex = -1;
    }
  }

  Cal_BddManagerGC(bddManager);
  Cal_BddStats(bddManager, stdout);
  /*CalUniqueTablePrint(bddManager);*/
  Cal_BddManagerQuit(bddManager);
}
