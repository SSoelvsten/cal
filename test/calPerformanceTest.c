/**CFile***********************************************************************

  FileName    [calPerformanceTest.c]

  PackageName [cal]

  Synopsis    [This file contains the performance test routines for
  the CAL package.] 

  Description [optional]

  SeeAlso     [optional]

  Author      [Rajeev K. Ranjan (rajeev@eecs.berkeley.edu)
               Jagesh Sanghavi  (sanghavi@eecs.berkeley.edu)
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

  Revision    [$Id: calPerformanceTest.c,v 1.7 1998/09/17 02:26:27 ravi Exp $]

******************************************************************************/

#include "calInt.h"

#include <unistd.h>

#include <sys/types.h>
#include "sys/resource.h" // <-- rusage
#include <sys/time.h>     // <-- timeval, timezone


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
static int ITERATION;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void CalPerformanceTestAnd(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions);
#ifdef COMPUTE_MEMORY_OVERHEAD
static void CalPerformanceMemoryOverhead(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions);
#endif
static void CalPerformaceTestSuperscalar(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions);
static void CalPerformanceTestNonSuperscalar(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions);
static void CalPerformanceTestMultiway(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions);
static void CalPerformanceTestOneway(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions);
static void CalPerformanceTestCompose(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions);
static void CalPerformanceTestQuantifyAllTogether(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions, int bfZeroBFPlusDFOne, int cacheExistsResultsFlag, int cacheOrResultsFlag);
static void CalQuantifySanityCheck(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions);
static void CalPerformanceTestRelProd(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions, int bfZeroBFPlusDFOne, int cacheRelProdResultsFlag, int cacheAndResultsFlag, int cacheOrResultsFlag);
static void CalPerformanceTestSubstitute(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions);
static void CalPerformanceTestSwapVars(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions);
static long elapsedTime();
static double cpuTime();
static long pageFaults();
static void GetRandomNumbers(int lowerBound, int upperBound, int count, int *resultVector);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Main routine for testing performances of various routines.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
Cal_PerformanceTest(Cal_BddManager bddManager,
                    Cal_Bdd *outputBddArray,
                    int numFunctions, int iteration, int seed,
                    int andPerformanceFlag,
                    int multiwayPerformanceFlag,
                    int onewayPerformanceFlag,
                    int quantifyPerformanceFlag, 
                    int composePerformanceFlag,
                    int relprodPerformanceFlag,
                    int swapPerformanceFlag,
                    int substitutePerformanceFlag,
                    int sanityCheckFlag,
                    int computeMemoryOverheadFlag,
                    int superscalarFlag) 
{
  
  CalUtilSRandom((long)seed);
  
  ITERATION = iteration;
  fprintf(stdout,"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
  fprintf(stdout, "Performing %d iterations for each function\n", iteration);
  Cal_BddSetGCMode(bddManager, 0);
#ifdef QUANTIFY
  quantify_start_recording_data();
#endif

#ifdef PURECOV
	purecov_clear_data();
#endif

  if (relprodPerformanceFlag){
    CalPerformanceTestRelProd(bddManager, outputBddArray, numFunctions, 1, 1,
                              1, 1);
    CalUtilSRandom((long)seed);

  }
  if (sanityCheckFlag == 1){
    CalQuantifySanityCheck(bddManager, outputBddArray,
                           numFunctions);
    CalUtilSRandom((long)seed);
  }
  if (quantifyPerformanceFlag){
    CalPerformanceTestQuantifyAllTogether(bddManager, outputBddArray,
                                          numFunctions, 1, 1, 1);
    CalUtilSRandom((long)seed);
	/*
    CalPerformanceTestNonSuperscalarQuant(bddManager, outputBddArray, numFunctions);
    CalUtilSRandom((long)seed);
	*/
  }

  if (multiwayPerformanceFlag){
    CalPerformanceTestMultiway(bddManager, outputBddArray, numFunctions); 
    CalUtilSRandom((long)seed);
  }
  if (onewayPerformanceFlag){
    CalPerformanceTestOneway(bddManager, outputBddArray, numFunctions);
    CalUtilSRandom((long)seed);
  }
  if (andPerformanceFlag){
    CalPerformanceTestAnd(bddManager, outputBddArray, numFunctions); 
    CalUtilSRandom((long)seed);
  }
  if (composePerformanceFlag){
    CalPerformanceTestCompose(bddManager, outputBddArray, numFunctions);
    CalUtilSRandom((long)seed);
  }
  if (swapPerformanceFlag){
    CalPerformanceTestSwapVars(bddManager, outputBddArray, numFunctions);
    CalUtilSRandom((long)seed);
  }
  if (substitutePerformanceFlag){
    CalPerformanceTestSubstitute(bddManager, outputBddArray, numFunctions);
    CalUtilSRandom((long)seed);
  }
#ifdef COMPUTE_MEMORY_OVERHEAD
  if (computeMemoryOverheadFlag){
    CalPerformaceMemoryOverhead(bddManager, outputBddArray, numFunctions);
    CalUtilSRandom((long)seed);
  }
#endif
  if (superscalarFlag){
    CalPerformaceTestSuperscalar(bddManager, outputBddArray, numFunctions);
    CalUtilSRandom((long)seed);
    CalPerformanceTestNonSuperscalar(bddManager, outputBddArray, numFunctions);
    CalUtilSRandom((long)seed);
  }
#ifdef QUANTIFY
  quantify_stop_recording_data();
#endif
#ifdef PURECOV
	purecov_save_data();
	purecov_disable_save();
#endif
  fprintf(stdout,"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
  Cal_BddSetGCMode(bddManager, 1);
  return 0;
}

int main(int argc, char ** argv)
{
  Cal_BddManager bddManager = Cal_BddManagerInit();
  int output_size = 42;
  Cal_Bdd outputBddArray[42];

  Cal_PerformanceTest(bddManager, outputBddArray, output_size,
                      /*iteration: */ 0, /*seed: */ 42,
                      1, // andPerformanceFlag,
                      1, // multiwayPerformanceFlag,
                      1, // onewayPerformanceFlag,
                      1, // quantifyPerformanceFlag,
                      1, // composePerformanceFlag,
                      1, // relprodPerformanceFlag,
                      1, // swapPerformanceFlag,
                      1, // substitutePerformanceFlag,
                      1, // sanityCheckFlag,
                      1, // computeMemoryOverheadFlag,
                      1 // superscalarFlag
                      );

  Cal_BddManagerGC(bddManager);
  Cal_BddStats(bddManager, stdout);
  /*CalUniqueTablePrint(bddManager);*/
  Cal_BddManagerQuit(bddManager);
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
int
CalIncreasingOrderCompare(const void *a, const void *b)
{
  return (*(int *)b-*(int *)a);
}

/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
CalDecreasingOrderCompare(const void *a, const void *b)
{
  return (*(int *)a-*(int *)b);
}
/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalPerformanceTestAnd(Cal_BddManager bddManager, Cal_Bdd
                      *outputBddArray, int numFunctions)
{
  int i;
  Cal_Bdd function1, function2;
  Cal_Bdd result;
  
  
  (void) elapsedTime();
  cpuTime();
  pageFaults();
  for (i=0; i<ITERATION; i++){
    function1 = outputBddArray[CalUtilRandom()%numFunctions];
    function2 = outputBddArray[CalUtilRandom()%numFunctions];
    result = Cal_BddAnd(bddManager, function1, function2);
    Cal_BddFree(bddManager, result);
  }
  fprintf(stdout, "%-20s%-10ld%-8.2f%-10ld\n", "AND", elapsedTime(),
          cpuTime(), pageFaults());
  Cal_BddManagerGC(bddManager);
}



#ifdef COMPUTE_MEMORY_OVERHEAD
/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalPerformanceMemoryOverhead(Cal_BddManager bddManager, Cal_Bdd
                            *outputBddArray, int numFunctions)
{
  int i, j, *varIdArray;
  Cal_Bdd function1, function2;
  Cal_Bdd result, *bddArray;
  double maxReduceToApplyRatio = 0;
  double maxReduceToUniqueTableRatio = 0;
  int num, power;

  if (numFunctions <= 1) return;

  for (i=0; i<ITERATION; i++){
    function1 = outputBddArray[CalUtilRandom()%numFunctions];
    function2 = outputBddArray[CalUtilRandom()%numFunctions];
    result = Cal_BddAnd(bddManager, function1, function2);
    Cal_BddFree(bddManager, result);
    if (maxReduceToApplyRatio < calAfterReduceToAfterApplyNodesRatio){
      maxReduceToApplyRatio = calAfterReduceToAfterApplyNodesRatio;
    }
    if (maxReduceToUniqueTableRatio < calAfterReduceToUniqueTableNodesRatio){
      maxReduceToUniqueTableRatio = calAfterReduceToUniqueTableNodesRatio;
    }
  }

  fprintf(stdout, "%-20s Max R/A: %-8.6f Max R/U: %-8.6f\n", "MEMORYOVERHEAD-AND",
          calAfterReduceToAfterApplyNodesRatio,
          calAfterReduceToUniqueTableNodesRatio);
  Cal_BddManagerGC(bddManager);

  for (power = 1; power <= 5; power++){
    num = (1<<power);
    if (num > numFunctions) return;
    varIdArray = Cal_MemAlloc(int, num);
    bddArray = Cal_MemAlloc(Cal_Bdd, num+1);
    bddArray[num] = Cal_BddGetNullBdd(bddManager);
    
    maxReduceToApplyRatio = 0;
    maxReduceToUniqueTableRatio = 0;
    
    for (i=0; i<ITERATION; i++){
      GetRandomNumbers(0, numFunctions-1, num, varIdArray);
      for (j=0; j<num; j++){
        bddArray[j] = outputBddArray[varIdArray[j]];
      }
      result = Cal_BddMultiwayAnd(bddManager, bddArray);
      Cal_BddFree(bddManager, result);
      if (maxReduceToApplyRatio < calAfterReduceToAfterApplyNodesRatio){
        maxReduceToApplyRatio = calAfterReduceToAfterApplyNodesRatio;
      }
      if (maxReduceToUniqueTableRatio <
          calAfterReduceToUniqueTableNodesRatio){ 
        maxReduceToUniqueTableRatio = calAfterReduceToUniqueTableNodesRatio;
      }
    }
    
    fprintf(stdout, "%-16s%4d Max R/A: %-8.6f Max R/U: %-8.6f\n",
            "MH-MULTIWAY-AND", num,
            calAfterReduceToAfterApplyNodesRatio, 
            calAfterReduceToUniqueTableNodesRatio);
    Cal_MemFree(varIdArray);
    Cal_MemFree(bddArray);
    Cal_BddManagerGC(bddManager);
  }
}
#endif

/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalPerformaceTestSuperscalar(Cal_BddManager bddManager, Cal_Bdd
                         *outputBddArray, int numFunctions)
{
  int i,j;
  Cal_Bdd *bddArray, *resultArray;
  int *varIdArray;
  int num = (((numFunctions%2) == 0)? numFunctions : (numFunctions-1));
  if (num == 0) return;
  if (num > 100) num = 100;
  varIdArray = Cal_MemAlloc(int, num);
  bddArray = Cal_MemAlloc(Cal_Bdd, num+1);
  bddArray[num] = Cal_BddNull(bddManager);
  
  
  (void) elapsedTime();
  cpuTime();
  pageFaults();
  for (i=0; i<ITERATION; i++){
    GetRandomNumbers(0, numFunctions-1, num, varIdArray);
    for (j=0; j<num; j++){
      bddArray[j] = outputBddArray[varIdArray[j]];
    }
    resultArray = Cal_BddPairwiseAnd(bddManager, bddArray);
    for (j=0; j<num/2; j++){
      Cal_BddFree(bddManager, resultArray[j]);
    }
    Cal_MemFree(resultArray);
  }
  fprintf(stdout, "%-20s%-10ld%-8.2f%-10ld\n", "SUPERSCALARAND", elapsedTime(),
          cpuTime(), pageFaults());
  Cal_MemFree(varIdArray);
  Cal_MemFree(bddArray);
  Cal_BddManagerGC(bddManager);
}

/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalPerformanceTestNonSuperscalar(Cal_BddManager bddManager, Cal_Bdd
                                 *outputBddArray, int numFunctions)
{
  int i, j;
  Cal_Bdd *bddArray, *resultArray;
  int *varIdArray;

  int num = (((numFunctions%2) == 0)? numFunctions : (numFunctions-1));

  if (num == 0) return;
  if (num > 100) num = 100;

  varIdArray = Cal_MemAlloc(int, num);
  bddArray = Cal_MemAlloc(Cal_Bdd, num+1);
  bddArray[num] = Cal_BddNull(bddManager);
  resultArray = Cal_MemAlloc(Cal_Bdd, num/2);
  
  (void) elapsedTime();
  cpuTime();
  pageFaults();
  for (i=0; i<ITERATION; i++){
    GetRandomNumbers(0, numFunctions-1, num, varIdArray);
    for (j=0; j<num/2; j++){
      resultArray[j] = Cal_BddAnd(bddManager,
                          outputBddArray[varIdArray[j<<1]],
                          outputBddArray[varIdArray[(j<<1)+1]]);  
    }
    for (j=0; j<num/2; j++){
      Cal_BddFree(bddManager, resultArray[j]);
    }
  }
  fprintf(stdout, "%-20s%-10ld%-8.2f%-10ld\n", "NONSUPERSCALARAND", elapsedTime(),
          cpuTime(), pageFaults());
  Cal_MemFree(resultArray);
  Cal_MemFree(bddArray);
  Cal_MemFree(varIdArray);
  Cal_BddManagerGC(bddManager);
}


/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalPerformanceTestMultiway(Cal_BddManager bddManager, Cal_Bdd
                           *outputBddArray, int numFunctions)
{
  int i,j;
  Cal_Bdd result, *bddArray;
  int *varIdArray;
  int power;
  int num;
  
  if (numFunctions <= 1) return;
  for (power = 1; power <= 5; power++){
    num = (1<<power);
    if (num > numFunctions) return;
    varIdArray = Cal_MemAlloc(int, num);
    bddArray = Cal_MemAlloc(Cal_Bdd, num+1);
    bddArray[num] = Cal_BddNull(bddManager);
    (void) elapsedTime();
    cpuTime();
    pageFaults();
    for (i=0; i<ITERATION; i++){
      GetRandomNumbers(0, numFunctions-1, num, varIdArray);
      for (j=0; j<num; j++){
        bddArray[j] = outputBddArray[varIdArray[j]];
      }
      result = Cal_BddMultiwayAnd(bddManager, bddArray);
      Cal_BddFree(bddManager, result);
    }
    fprintf(stdout, "%-20s%-4d%-10ld%-8.2f%-10ld\n", "MULTIWAYAND", num,
            elapsedTime(), cpuTime(), pageFaults());
    Cal_MemFree(varIdArray);
    Cal_MemFree(bddArray);
    Cal_BddManagerGC(bddManager);
  }
}

/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalPerformanceTestOneway(Cal_BddManager bddManager, Cal_Bdd
                         *outputBddArray, int numFunctions)
{
  int i, j;
  Cal_Bdd result, tempResult;
  int *varIdArray;
  int power, num;
  
  if (numFunctions <= 1) return;
  
  for (power = 1; power <= 5; power++){
    num = (1<<power);
    if (num > numFunctions) return;
    varIdArray = Cal_MemAlloc(int, num);
    (void) elapsedTime();
    cpuTime();
    pageFaults();
    for (i=0; i<ITERATION; i++){
      GetRandomNumbers(0, numFunctions-1, num, varIdArray);
      result = Cal_BddOne(bddManager);
      for (j=0; j<num; j++){
        tempResult = Cal_BddAnd(bddManager, result,
                                outputBddArray[varIdArray[j]]); 
        Cal_BddFree(bddManager, result);
        result = tempResult;
      }
      Cal_BddFree(bddManager, result);
    }
    fprintf(stdout, "%-20s%-4d%-10ld%-8.2f%-10ld\n", "ONEWAYAND", num,
            elapsedTime(), cpuTime(), pageFaults());
    Cal_MemFree(varIdArray);
    Cal_BddManagerGC(bddManager);
  }
}
/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalPerformanceTestCompose(Cal_BddManager bddManager, Cal_Bdd
                                   *outputBddArray, int numFunctions)
{
  int i;
  int numVars = Cal_BddVars(bddManager);
  Cal_Bdd function;
  Cal_Bdd variable;
  Cal_Bdd substituteFunction;
  Cal_Bdd result;
  
  
  (void) elapsedTime();
  cpuTime();
  pageFaults();
  for (i=0; i<ITERATION; i++){
    function = outputBddArray[CalUtilRandom()%numFunctions];
    variable = Cal_BddManagerGetVarWithId(bddManager,(Cal_BddId_t)CalUtilRandom()%numVars+1);
    substituteFunction = outputBddArray[CalUtilRandom()%numFunctions];
    result = Cal_BddCompose(bddManager, function, variable,
                                    substituteFunction);
    Cal_BddFree(bddManager, result);
  }
  fprintf(stdout, "%-20s%-10ld%-8.2f%-10ld\n", "COMPOSE", elapsedTime(),
          cpuTime(), pageFaults());
  Cal_BddManagerGC(bddManager);
}
/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalPerformanceTestQuantifyAllTogether(Cal_BddManager bddManager, Cal_Bdd
                                      *outputBddArray, int numFunctions,
                                      int bfZeroBFPlusDFOne, int
                                      cacheExistsResultsFlag, int
                                      cacheOrResultsFlag)
{
  int i, j;
  int numVars = Cal_BddVars(bddManager);
  int numQuantifyVars = numVars/2;
  int *varIdArray = Cal_MemAlloc(int, numQuantifyVars);
  Cal_Bdd *assoc = Cal_MemAlloc(Cal_Bdd, numQuantifyVars+1);
  Cal_Bdd function, result;
  int assocId;
  
  for (i=0; i <= numQuantifyVars; i++){
    assoc[i] = Cal_BddNull(bddManager);
  }
  
  (void) elapsedTime();
  cpuTime();
  pageFaults();
  for (i=0; i<ITERATION; i++){
    function = outputBddArray[CalUtilRandom()%numFunctions];
    GetRandomNumbers(1, numVars, numQuantifyVars, varIdArray);
    for (j=0; j<numQuantifyVars; j++){
      assoc[j] = Cal_BddManagerGetVarWithId(bddManager, varIdArray[j]);
    }
    assocId = Cal_AssociationInit(bddManager, assoc, 0);
    Cal_AssociationSetCurrent(bddManager, assocId);
    result = Cal_BddExists(bddManager, function);
    Cal_BddFree(bddManager, result); 
  }
  fprintf(stdout, "%-20s%-10ld%-8.2f%-10ld\n", "QUANTIFY", elapsedTime(),
          cpuTime(), pageFaults());
  Cal_MemFree(assoc);
  Cal_MemFree(varIdArray);
  Cal_BddManagerGC(bddManager);
}




/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalQuantifySanityCheck(Cal_BddManager bddManager, Cal_Bdd
                       *outputBddArray, int numFunctions) 
{
  int i, j;
  int numVars = Cal_BddVars(bddManager);
  int numQuantifyVars = numVars/2;
  int *varIdArray = Cal_MemAlloc(int, numQuantifyVars);
  Cal_Bdd *assoc = Cal_MemAlloc(Cal_Bdd, numQuantifyVars+1);
  Cal_Bdd function, oneAtATimeResult, allTogetherResult, tempResult, nonSuperscalarResult;
  
  
  for (i=0; i <= numQuantifyVars; i++){
    assoc[i] = Cal_BddNull(bddManager);
  }
  
  (void) elapsedTime();
  for (i=0; i<ITERATION; i++){
    function = outputBddArray[CalUtilRandom()%numFunctions];
    GetRandomNumbers(1, numVars, numQuantifyVars, varIdArray);
    for (j=0; j<numQuantifyVars; j++){
      assoc[j] = Cal_BddManagerGetVarWithId(bddManager, varIdArray[j]);
    }
    Cal_TempAssociationInit(bddManager, assoc, 0);
    Cal_AssociationSetCurrent(bddManager, -1);
    allTogetherResult = Cal_BddExists(bddManager, function);

    oneAtATimeResult = Cal_BddIdentity(bddManager, function);
    qsort((void *) varIdArray, (size_t)numQuantifyVars, (size_t)sizeof(int),
          CalDecreasingOrderCompare);
    for (j=0; j<numQuantifyVars; j++){
      assoc[0] =
          Cal_BddManagerGetVarWithId(bddManager,varIdArray[j]); 
      assoc[1] = Cal_BddNull(bddManager);
      Cal_TempAssociationAugment(bddManager, assoc, 0);
      tempResult = Cal_BddExists(bddManager, oneAtATimeResult);
      Cal_BddFree(bddManager, oneAtATimeResult);
      oneAtATimeResult = tempResult;
    }
    
    nonSuperscalarResult = Cal_BddExists(bddManager, function);
    
    assert(Cal_BddIsEqual(bddManager, allTogetherResult, oneAtATimeResult));
    assert(Cal_BddIsEqual(bddManager, allTogetherResult, nonSuperscalarResult));
    Cal_BddFree(bddManager, oneAtATimeResult); 
    Cal_BddFree(bddManager, allTogetherResult); 
    Cal_BddFree(bddManager, nonSuperscalarResult);
  }
  fprintf(stdout, "Quantify Sanity Check Passed\n");
  Cal_MemFree(assoc);
  Cal_MemFree(varIdArray);
  Cal_TempAssociationQuit(bddManager);
}

/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalPerformanceTestRelProd(Cal_BddManager bddManager, Cal_Bdd
                          *outputBddArray, int numFunctions, int
                          bfZeroBFPlusDFOne, int cacheRelProdResultsFlag, int 
                          cacheAndResultsFlag, int cacheOrResultsFlag)
{
  int i, j;
  int numVars = Cal_BddVars(bddManager);
  int numQuantifyVars = numVars/2;
  int *varIdArray = Cal_MemAlloc(int, numQuantifyVars);
  Cal_Bdd *assoc = Cal_MemAlloc(Cal_Bdd, numQuantifyVars+1);
  Cal_Bdd function1, function2, result;
  int assocId;
  
  for (i=0; i <= numQuantifyVars; i++){
    assoc[i] = Cal_BddNull(bddManager);
  }
  
  elapsedTime();
  cpuTime();
  pageFaults();
  for (i=0; i<ITERATION; i++){
    function1 = outputBddArray[CalUtilRandom()%numFunctions];
    function2 = outputBddArray[CalUtilRandom()%numFunctions]; 
   GetRandomNumbers(1, numVars, numQuantifyVars,varIdArray);
    for (j=0; j<numQuantifyVars; j++){
      assoc[j] = Cal_BddManagerGetVarWithId(bddManager, varIdArray[j]);
    }
    assocId = Cal_AssociationInit(bddManager, assoc, 0);
    Cal_AssociationSetCurrent(bddManager, assocId);
    result = Cal_BddRelProd(bddManager, function1, function2);
    Cal_BddFree(bddManager, result);
  }
  fprintf(stdout, "%-20s%-10ld%-8.2f%-10ld\n", "RELPROD", elapsedTime(),
          cpuTime(), pageFaults());
  Cal_MemFree(assoc);
  Cal_MemFree(varIdArray);
  Cal_BddManagerGC(bddManager);
}


/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalPerformanceTestSubstitute(Cal_BddManager bddManager, Cal_Bdd
                             *outputBddArray, int numFunctions)
{
  int i, j;
  int numVars = Cal_BddVars(bddManager);
  int numQuantifyVars = ((numVars/2 > numFunctions/2) ? numFunctions/2
                         : numVars/2);
  int *varIdArray = Cal_MemAlloc(int, numQuantifyVars);
  Cal_Bdd *assoc = Cal_MemAlloc(Cal_Bdd, 2*numQuantifyVars+1);
  Cal_Bdd function, result;
  
  for (i=0; i <= 2*numQuantifyVars; i++){
    assoc[i] = Cal_BddNull(bddManager);
  }
  (void) elapsedTime();
  cpuTime();
  pageFaults();
  for (i=0; i<ITERATION/5; i++){
    function = outputBddArray[CalUtilRandom()%numFunctions];
    GetRandomNumbers(1, numVars, numQuantifyVars,varIdArray);
    for (j=0; j<numQuantifyVars; j++){
      assoc[(j<<1)] = Cal_BddManagerGetVarWithId(bddManager,varIdArray[j]);
    }
    GetRandomNumbers(0, numFunctions-1, numQuantifyVars, varIdArray);
    for (j=0; j<numQuantifyVars; j++){
      assoc[(j<<1)+1] = outputBddArray[varIdArray[j]];
    }
    Cal_TempAssociationInit(bddManager, assoc, 1);
    Cal_AssociationSetCurrent(bddManager, -1);
    result = Cal_BddSubstitute(bddManager, function);
    Cal_BddFree(bddManager, result);
  }
  fprintf(stdout, "%-20s%-10ld%-8.2f%-10ld\n", "SUBSTITUTE", elapsedTime(),
          cpuTime(), pageFaults());
  Cal_MemFree(assoc);
  Cal_MemFree(varIdArray);
  Cal_TempAssociationQuit(bddManager);
  Cal_BddManagerGC(bddManager);
}

/**Function********************************************************************

  Synopsis    [Performance test routine for quantify (all variables at the same
  time).]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalPerformanceTestSwapVars(Cal_BddManager bddManager, Cal_Bdd
                           *outputBddArray, int numFunctions) 
{
  int i;
  int numVars = Cal_BddVars(bddManager);
  Cal_Bdd function, result;
  Cal_Bdd var1, var2;
  
  elapsedTime();
  cpuTime();
  pageFaults();
  for (i=0; i<ITERATION; i++){
    function = outputBddArray[CalUtilRandom()%numFunctions];
    var1 = Cal_BddManagerGetVarWithId(bddManager,(Cal_BddId_t)(CalUtilRandom()%numVars)+1);
    var2 = Cal_BddManagerGetVarWithId(bddManager,(Cal_BddId_t)(CalUtilRandom()%numVars)+1);
    result = Cal_BddSwapVars(bddManager, function, var1,var2);
    Cal_BddFree(bddManager, result);
  }
  fprintf(stdout, "%-20s%-10ld%-8.2f%-10ld\n", "SWAPVARS", elapsedTime(),
          cpuTime(), pageFaults());
  Cal_BddManagerGC(bddManager);
}
/**Function********************************************************************

  Synopsis    [Computes the time.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static long
elapsedTime()
{
  static long time_new, time_old;
  struct timeval t;
  struct timezone tz;
  static int flag = 0;
  
  gettimeofday(&t, &tz);
  if (flag == 0){
    time_old = time_new = t.tv_sec;
    flag = 1;
  }
  else {
    time_old = time_new;
    time_new =  t.tv_sec;
  }
  return time_new-time_old;
}

/**Function********************************************************************

  Synopsis    [Computes the number of page faults.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static double
cpuTime()
{
#if HAVE_SYS_RESOURCE_H
  static double timeNew, timeOld;
  struct rusage rusage;
  static int flag = 0;
  
  getrusage(RUSAGE_SELF, &rusage);
  if (flag == 0){
    timeOld = timeNew = rusage.ru_utime.tv_sec+
        ((double)rusage.ru_utime.tv_usec)/1000000;
    flag = 1;
  }
  else {
    timeOld = timeNew;
    timeNew = rusage.ru_utime.tv_sec+
        ((float)rusage.ru_utime.tv_usec)/1000000;
  }
  return timeNew - timeOld;
#else /* No sys/resource.h */
  return 0;
#endif
}

/**Function********************************************************************

  Synopsis    [Computes the number of page faults.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static long
pageFaults()
{
#if HAVE_SYS_RESOURCE_H
  static long faultNew, faultOld;
  struct rusage rusage;
  static int flag = 0;
  
  getrusage(RUSAGE_SELF, &rusage);
  if (flag == 0){
    faultOld = faultNew = rusage.ru_majflt;
    flag = 1;
  }
  else {
    faultOld = faultNew;
    faultNew = rusage.ru_majflt;
  }
  return faultNew - faultOld;
#else /* Don't have sys/resource.h */
  return 0;
#endif
}

/**Function********************************************************************

  Synopsis    [Generates "count" many random numbers ranging between
  "lowerBound" and "upperBound".]

  Description [The restriction is that count <= upperBound-lowerBound+1. The
  size of the resultVector should be >= count.]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
GetRandomNumbers(int lowerBound, int upperBound, int count, int *resultVector)
{
  int i,j, tempVector[2048], number;
  int range = (upperBound - lowerBound + 1);

  for (i=0; i<range; i++)  tempVector[i] = lowerBound+i;
  for (i=0; i<count; i++){
    number = (int)CalUtilRandom()% (range-i);
    resultVector[i] = tempVector[number];
    for (j=number; j < range-i; j++){
      tempVector[j] = tempVector[j+1];
    }
  }
  /*
  fprintf(stdout,"%d\t%d\t%d\n", lowerBound, upperBound, count);
  for (i=0; i<count; i++)  fprintf(stdout,"%d ", resultVector[i]);
  fprintf(stdout, "\n");
  */
}




