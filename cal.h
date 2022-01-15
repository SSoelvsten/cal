/**CHeaderFile*****************************************************************

  FileName    [cal.h]

  PackageName [cal]

  Synopsis    [Header CAL file for exported data structures and functions.]

  Description []

  SeeAlso     [optional]

  Author      [Rajeev K. Ranjan (rajeev@eecs.berkeley.edu)
               Jagesh V. Sanghavi (sanghavi@eecs.berkeley.edu)] 

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

  Revision    [$Id: cal.h,v 1.6 1998/09/15 19:02:51 ravi Exp $]

******************************************************************************/

#ifndef _CAL
#define _CAL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#if HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif
#if HAVE_SYS_RESOURCE_H
#  include <sys/resource.h>
#endif
#if HAVE_UNISTD_H
#  include <unistd.h>
#endif
#if HAVE_TIME_H
#  include <time.h>
#endif

#include <assert.h>
#include <math.h>

#include "calMem.h"
/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#ifndef EXTERN
#define EXTERN extern
#endif

#define CAL_BDD_TYPE_NONTERMINAL 0
#define CAL_BDD_TYPE_ZERO 1
#define CAL_BDD_TYPE_ONE 2
#define CAL_BDD_TYPE_POSVAR 3
#define CAL_BDD_TYPE_NEGVAR 4
#define CAL_BDD_TYPE_OVERFLOW 5
#define CAL_BDD_TYPE_CONSTANT 6

#define CAL_BDD_UNDUMP_FORMAT 1
#define CAL_BDD_UNDUMP_OVERFLOW 2
#define CAL_BDD_UNDUMP_IOERROR 3
#define CAL_BDD_UNDUMP_EOF 4

#define CAL_REORDER_NONE 0
#define CAL_REORDER_SIFT 1
#define CAL_REORDER_WINDOW 2
#define CAL_REORDER_METHOD_BF 0
#define CAL_REORDER_METHOD_DF 1

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct Cal_BddManagerStruct *Cal_BddManager;
typedef struct Cal_BddManagerStruct Cal_BddManager_t;
typedef struct CalBddNodeStruct *Cal_Bdd;
typedef unsigned short int Cal_BddId_t;
typedef unsigned short int Cal_BddIndex_t;
typedef char * (*Cal_VarNamingFn_t)(Cal_BddManager, Cal_Bdd, Cal_Pointer_t); 
typedef char * (*Cal_TerminalIdFn_t)(Cal_BddManager, Cal_Address_t, Cal_Address_t, Cal_Pointer_t);      
typedef struct Cal_BlockStruct *Cal_Block;

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/
enum Cal_BddOpEnum {CAL_AND, CAL_OR, CAL_XOR};
typedef enum Cal_BddOpEnum Cal_BddOp_t;


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define Cal_BddNamingFnNone ((char *(*)(Cal_BddManager, Cal_Bdd, Cal_Pointer_t))0)
#define Cal_BddTerminalIdFnNone ((char *(*)(Cal_BddManager, Cal_Address_t, Cal_Address_t, Cal_Pointer_t))0)
#ifdef _CAL_DEBUG_
#define Cal_Assert(valid) assert(valid)
#else
#define Cal_Assert(ignore) ((void)0)
#endif



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN int Cal_BddIsEqual(Cal_BddManager bddManager, Cal_Bdd userBdd1, Cal_Bdd userBdd2);
EXTERN int Cal_BddIsBddOne(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN int Cal_BddIsBddZero(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN int Cal_BddIsBddNull(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN int Cal_BddIsBddConst(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_Bdd Cal_BddIdentity(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_Bdd Cal_BddOne(Cal_BddManager bddManager);
EXTERN Cal_Bdd Cal_BddZero(Cal_BddManager bddManager);
EXTERN Cal_Bdd Cal_BddNot(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_BddId_t Cal_BddGetIfIndex(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_BddId_t Cal_BddGetIfId(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_Bdd Cal_BddIf(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_Bdd Cal_BddThen(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_Bdd Cal_BddElse(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN void Cal_BddFree(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN void Cal_BddUnFree(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_Bdd Cal_BddGetRegular(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_Bdd Cal_BddIntersects(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd);
EXTERN Cal_Bdd Cal_BddImplies(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd);
EXTERN unsigned long Cal_BddTotalSize(Cal_BddManager bddManager);
EXTERN void Cal_BddStats(Cal_BddManager bddManager, FILE * fp);
EXTERN void Cal_BddDynamicReordering(Cal_BddManager bddManager, int technique);
EXTERN void Cal_BddReorder(Cal_BddManager bddManager);
EXTERN int Cal_BddType(Cal_BddManager bddManager, Cal_Bdd fUserBdd);
EXTERN long Cal_BddVars(Cal_BddManager bddManager);
EXTERN long Cal_BddNodeLimit(Cal_BddManager bddManager, long newLimit);
EXTERN int Cal_BddOverflow(Cal_BddManager bddManager);
EXTERN int Cal_BddIsCube(Cal_BddManager bddManager, Cal_Bdd fUserBdd);
EXTERN void * Cal_BddManagerGetHooks(Cal_BddManager bddManager);
EXTERN void Cal_BddManagerSetHooks(Cal_BddManager bddManager, void *hooks);
EXTERN int Cal_AssociationInit(Cal_BddManager bddManager, Cal_Bdd *associationInfoUserBdds, int pairs);
EXTERN void Cal_AssociationQuit(Cal_BddManager bddManager, int associationId);
EXTERN int Cal_AssociationSetCurrent(Cal_BddManager bddManager, int associationId);
EXTERN void Cal_TempAssociationAugment(Cal_BddManager bddManager, Cal_Bdd *associationInfoUserBdds, int pairs);
EXTERN void Cal_TempAssociationInit(Cal_BddManager bddManager, Cal_Bdd *associationInfoUserBdds, int pairs);
EXTERN void Cal_TempAssociationQuit(Cal_BddManager bddManager);
EXTERN Cal_Bdd Cal_BddCompose(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd, Cal_Bdd hUserBdd);
EXTERN Cal_Bdd Cal_BddITE(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd, Cal_Bdd hUserBdd);
EXTERN Cal_BddManager Cal_BddManagerInit();
EXTERN int Cal_BddManagerQuit(Cal_BddManager bddManager);
EXTERN void Cal_BddManagerSetParameters(Cal_BddManager bddManager, long reorderingThreshold, long maxForwardedNodes, double repackAfterGCThreshold, double tableRepackThreshold);
EXTERN unsigned long Cal_BddManagerGetNumNodes(Cal_BddManager bddManager);
EXTERN Cal_Bdd Cal_BddManagerCreateNewVarFirst(Cal_BddManager bddManager);
EXTERN Cal_Bdd Cal_BddManagerCreateNewVarLast(Cal_BddManager bddManager);
EXTERN Cal_Bdd Cal_BddManagerCreateNewVarBefore(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_Bdd Cal_BddManagerCreateNewVarAfter(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN Cal_Bdd Cal_BddManagerGetVarWithIndex(Cal_BddManager bddManager, Cal_BddIndex_t index);
EXTERN Cal_Bdd Cal_BddManagerGetVarWithId(Cal_BddManager bddManager, Cal_BddId_t id);
EXTERN Cal_Bdd Cal_BddAnd(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd);
EXTERN Cal_Bdd Cal_BddNand(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd);
EXTERN Cal_Bdd Cal_BddOr(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd);
EXTERN Cal_Bdd Cal_BddNor(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd);
EXTERN Cal_Bdd Cal_BddXor(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd);
EXTERN Cal_Bdd Cal_BddXnor(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd);
EXTERN Cal_Bdd * Cal_BddPairwiseAnd(Cal_BddManager bddManager, Cal_Bdd *userBddArray);
EXTERN Cal_Bdd * Cal_BddPairwiseOr(Cal_BddManager bddManager, Cal_Bdd *userBddArray);
EXTERN Cal_Bdd * Cal_BddPairwiseXor(Cal_BddManager bddManager, Cal_Bdd *userBddArray);
EXTERN Cal_Bdd Cal_BddMultiwayAnd(Cal_BddManager bddManager, Cal_Bdd *userBddArray);
EXTERN Cal_Bdd Cal_BddMultiwayOr(Cal_BddManager bddManager, Cal_Bdd *userBddArray);
EXTERN Cal_Bdd Cal_BddMultiwayXor(Cal_BddManager bddManager, Cal_Bdd *userBddArray);
EXTERN Cal_Bdd Cal_BddSatisfy(Cal_BddManager bddManager, Cal_Bdd fUserBdd);
EXTERN Cal_Bdd Cal_BddSatisfySupport(Cal_BddManager bddManager, Cal_Bdd fUserBdd);
EXTERN double Cal_BddSatisfyingFraction(Cal_BddManager bddManager, Cal_Bdd fUserBdd);
EXTERN long Cal_BddSize(Cal_BddManager bddManager, Cal_Bdd fUserBdd, int negout);
EXTERN long Cal_BddSizeMultiple(Cal_BddManager bddManager, Cal_Bdd *fUserBddArray, int negout);
EXTERN void Cal_BddProfile(Cal_BddManager bddManager, Cal_Bdd fUserBdd, long * levelCounts, int negout);
EXTERN void Cal_BddProfileMultiple(Cal_BddManager bddManager, Cal_Bdd *fUserBddArray, long * levelCounts, int negout);
EXTERN void Cal_BddFunctionProfile(Cal_BddManager bddManager, Cal_Bdd fUserBdd, long * funcCounts);
EXTERN void Cal_BddFunctionProfileMultiple(Cal_BddManager bddManager, Cal_Bdd *fUserBddArray, long * funcCounts);
EXTERN Cal_Bdd Cal_BddSubstitute(Cal_BddManager bddManager, Cal_Bdd fUserBdd);
EXTERN void Cal_BddSupport(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd *support);
EXTERN int Cal_BddDependsOn(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd varUserBdd);
EXTERN Cal_Bdd Cal_BddSwapVars(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd, Cal_Bdd hUserBdd);
EXTERN Cal_Bdd Cal_BddVarSubstitute(Cal_BddManager bddManager, Cal_Bdd fUserBdd);
EXTERN Cal_Block Cal_BddNewVarBlock(Cal_BddManager bddManager, Cal_Bdd variable, long length);
EXTERN void Cal_BddVarBlockReorderable(Cal_BddManager bddManager, Cal_Block block, int reorderable);
EXTERN Cal_Bdd Cal_BddUndumpBdd(Cal_BddManager bddManager, Cal_Bdd * userVars, FILE * fp, int * error);
EXTERN int Cal_BddDumpBdd(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd * userVars, FILE * fp);
EXTERN void Cal_BddSetGCMode(Cal_BddManager bddManager, int gcMode);
EXTERN int Cal_BddManagerGC(Cal_BddManager bddManager);
EXTERN void Cal_BddManagerSetGCLimit(Cal_BddManager manager);
EXTERN void Cal_MemFatal(char *message);
EXTERN Cal_Address_t Cal_MemAllocation(void);
EXTERN Cal_Pointer_t Cal_MemGetBlock(Cal_Address_t size);
EXTERN void Cal_MemFreeBlock(Cal_Pointer_t p);
EXTERN Cal_Pointer_t Cal_MemResizeBlock(Cal_Pointer_t p, Cal_Address_t newSize);
EXTERN Cal_Pointer_t Cal_MemNewRec(Cal_RecMgr mgr);
EXTERN void Cal_MemFreeRec(Cal_RecMgr mgr, Cal_Pointer_t rec);
EXTERN Cal_RecMgr Cal_MemNewRecMgr(int size);
EXTERN void Cal_MemFreeRecMgr(Cal_RecMgr mgr);
EXTERN int Cal_PerformanceTest(Cal_BddManager bddManager, Cal_Bdd *outputBddArray, int numFunctions, int iteration, int seed, int andPerformanceFlag, int multiwayPerformanceFlag, int onewayPerformanceFlag, int quantifyPerformanceFlag, int composePerformanceFlag, int relprodPerformanceFlag, int swapPerformanceFlag, int substitutePerformanceFlag, int sanityCheckFlag, int computeMemoryOverheadFlag, int superscalarFlag);
EXTERN void Cal_PipelineSetDepth(Cal_BddManager bddManager, int depth);
EXTERN int Cal_PipelineInit(Cal_BddManager bddManager, Cal_BddOp_t bddOp);
EXTERN Cal_Bdd Cal_PipelineCreateProvisionalBdd(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd);
EXTERN int Cal_PipelineExecute(Cal_BddManager bddManager);
EXTERN Cal_Bdd Cal_PipelineUpdateProvisionalBdd(Cal_BddManager bddManager, Cal_Bdd provisionalBdd);
EXTERN int Cal_BddIsProvisional(Cal_BddManager bddManager, Cal_Bdd userBdd);
EXTERN void Cal_PipelineQuit(Cal_BddManager bddManager);
EXTERN void Cal_BddPrintBdd(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_VarNamingFn_t VarNamingFn, Cal_TerminalIdFn_t TerminalIdFn, Cal_Pointer_t env, FILE *fp);
EXTERN void Cal_BddPrintProfile(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_VarNamingFn_t varNamingProc, char * env, int lineLength, FILE * fp);
EXTERN void Cal_BddPrintProfileMultiple(Cal_BddManager bddManager, Cal_Bdd *userBdds, Cal_VarNamingFn_t varNamingProc, char * env, int lineLength, FILE * fp);
EXTERN void Cal_BddPrintFunctionProfile(Cal_BddManager bddManager, Cal_Bdd f, Cal_VarNamingFn_t varNamingProc, char * env, int lineLength, FILE * fp);
EXTERN void Cal_BddPrintFunctionProfileMultiple(Cal_BddManager bddManager, Cal_Bdd *userBdds, Cal_VarNamingFn_t varNamingProc, char * env, int lineLength, FILE * fp);
EXTERN Cal_Bdd Cal_BddExists(Cal_BddManager bddManager, Cal_Bdd fUserBdd);
EXTERN Cal_Bdd Cal_BddRelProd(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd gUserBdd);
EXTERN Cal_Bdd Cal_BddForAll(Cal_BddManager bddManager, Cal_Bdd fUserBdd);
EXTERN Cal_Bdd Cal_BddCofactor(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd cUserBdd);
EXTERN Cal_Bdd Cal_BddReduce(Cal_BddManager bddManager, Cal_Bdd fUserBdd, Cal_Bdd cUserBdd);
EXTERN Cal_Bdd Cal_BddBetween(Cal_BddManager bddManager, Cal_Bdd fMinUserBdd, Cal_Bdd fMaxUserBdd);
EXTERN void Cal_BddFunctionPrint(Cal_BddManager bddManager, Cal_Bdd userBdd, char *name);

/**AutomaticEnd***************************************************************/

#endif /* _CAL */
