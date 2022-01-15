/**CFile***********************************************************************

  FileName    [calBddReorderTest.c]

  PackageName [cal]

  Synopsis    [A test routine for checking the functionality of
  dynamic reordering.] 

  Description []

  SeeAlso     [optional]

  Author      [Wilsin Gosti    (wilsin@eecs.berkeley.edu)
               Rajeev Ranjan   (rajeev@eecs.berkeley.edu)
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

  Revision    [$Id: calBddReorderTest.c,v 1.1.1.4 1998/05/04 00:58:52 hsv Exp $]

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
static CalAddress_t asDoubleSpace[2];

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define CalBddReorderBddIsForwarded(bdd) \
  (CAL_BDD_POINTER(CalBddGetElseBddNode(bdd)) == FORWARD_FLAG)

#define CalBddReorderBddNodeIsForwarded(bddNode) \
  (CAL_BDD_POINTER(CalBddNodeGetElseBddNode(bddNode)) == FORWARD_FLAG)

#define CalBddReorderForward(bdd) \
{ \
  CalBddNode_t *_bddNode, *_bddNodeTagged; \
  _bddNodeTagged = CalBddGetBddNode(bdd); \
  _bddNode = CAL_BDD_POINTER(_bddNodeTagged); \
  (bdd).bddId = _bddNode->thenBddId; \
  (bdd).bddNode = (CalBddNode_t*) \
                  (((CalAddress_t)(_bddNode->thenBddNode) & ~0xe) \
                   ^(CAL_TAG0(_bddNodeTagged))); \
}

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static double cpuTime();
static long elapsedTime();

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
main(int argc, char **argv)
{
  Cal_Bdd expected;
  Cal_Bdd a[100];
  Cal_Bdd temp1;
  Cal_Bdd temp2;
  Cal_Bdd b, c, d, e, f, g, result;
  Cal_BddManager_t *bddManager;
  CalBddNode_t *bddNode;
  int i;
  int numVars;
  int siftFlag = 0;
  
  if (argc == 1) {
    numVars = 5;
  } else if (argc >= 2) {
    numVars = atoi(argv[1]);
  }
  if (argc == 3) {
    siftFlag = 1;
  }

  bddManager = Cal_BddManagerInit();

  for (i = 0; i < 2 * numVars; i++) {
    a[i] = Cal_BddManagerCreateNewVarLast(bddManager);
  }

  result = Cal_BddZero(bddManager);
  for (i = 0; i < numVars; i++) {
    temp1 = Cal_BddAnd(bddManager, a[i], a[numVars + i]);
    temp2 = Cal_BddOr(bddManager, result, temp1);
    Cal_BddFree(bddManager, temp1);
    Cal_BddFree(bddManager, result);
    result = temp2;
  }
  Cal_BddManagerGC(bddManager);
  Cal_BddStats(bddManager, stdout);
  cpuTime();
  elapsedTime();
  printf("%%%%%%%%%%%% Reordering %%%%%%%%%%%%%%%%%%%\n");
  if (siftFlag){
    printf("Using Sift Technique\n");
    Cal_BddDynamicReordering(bddManager, CAL_REORDER_SIFT);
  }
  else{
    printf("Using Window Technique\n");
    Cal_BddDynamicReordering(bddManager, CAL_REORDER_WINDOW);
  }
  Cal_BddReorder(bddManager);
  printf("CPU time: %-8.2f\t Elapsed Time = %-10ld\n", cpuTime(), elapsedTime());
  printf("%%%%%%%%%%%% %%%%%%%%%%%%%%%%%%%\n");
  Cal_BddManagerGC(bddManager);
  Cal_BddStats(bddManager, stdout);
  /*Cal_BddFunctionPrint(bddManager, result, "Result");*/
  temp1 = Cal_BddZero(bddManager);
  for (i = 0; i < numVars; i++) {
    temp2 = Cal_BddAnd(bddManager, a[i], a[numVars + i]);
    expected = Cal_BddOr(bddManager, temp1, temp2);
    Cal_BddFree(bddManager, temp1);
    Cal_BddFree(bddManager, temp2);
    temp1 = expected;
  }

  if (!Cal_BddIsEqual(bddManager, result, expected)) {
    printf("ERROR: BDDs are not equal\n");
    Cal_BddFunctionPrint(bddManager, result, "Result");
    Cal_BddFunctionPrint(bddManager, expected, "Expected");
  }
  printf("\n%%%%%%BDDs are equal\n");
  Cal_BddFree(bddManager, result);
  Cal_BddFree(bddManager, expected);
  Cal_BddManagerGC(bddManager);
  Cal_BddStats(bddManager, stdout);
  Cal_BddManagerQuit(bddManager);
}


/**Function********************************************************************

  Synopsis    [required]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static double
cpuTime()
{
  static double timeNew, timeOld;
  struct rusage rusage;
  static flag = 0;

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
  static flag = 0;
  
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
