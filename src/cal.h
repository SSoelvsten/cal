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

////////////////////////////////////////////////////////////////////////////////
/// \defgroup module__c C API
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \addtogroup module__c
///
/// \{

////////////////////////////////////////////////////////////////////////////////
/// \name Constant declarations
///
/// \{

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

/// \}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \name Type declarations
///
/// \{

typedef struct Cal_BddManagerStruct *Cal_BddManager;
typedef struct Cal_BddManagerStruct Cal_BddManager_t;
typedef struct CalBddNodeStruct *Cal_Bdd;
typedef unsigned short int Cal_BddId_t;
typedef unsigned short int Cal_BddIndex_t;
typedef char * (*Cal_VarNamingFn_t)(Cal_BddManager, Cal_Bdd, Cal_Pointer_t);
typedef char * (*Cal_TerminalIdFn_t)(Cal_BddManager, Cal_Address_t, Cal_Address_t, Cal_Pointer_t);
typedef struct Cal_BlockStruct *Cal_Block;

/// \}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \name Structure declarations
///
/// \{

enum Cal_BddOpEnum {CAL_AND, CAL_OR, CAL_XOR};
typedef enum Cal_BddOpEnum Cal_BddOp_t;

/// \}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \name Macro declarations
///
/// \{
#ifndef EXTERN
#define EXTERN extern
#endif

////////////////////////////////////////////////////////////////////////////////
/// \brief Default BDD variable naming function
////////////////////////////////////////////////////////////////////////////////
#define Cal_BddNamingFnNone ((char *(*)(Cal_BddManager, Cal_Bdd, Cal_Pointer_t))0)

////////////////////////////////////////////////////////////////////////////////
/// \brief Default BDD terminal naming function
////////////////////////////////////////////////////////////////////////////////
#define Cal_BddTerminalIdFnNone ((char *(*)(Cal_BddManager, Cal_Address_t, Cal_Address_t, Cal_Pointer_t))0)

#define Cal_Assert(valid) assert(valid)

/// \}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \name CAL Manager
///
/// \{

////////////////////////////////////////////////////////////////////////////////
/// \brief Creates and initializes a new BDD manager.
///
/// \details Initializes and allocates fields of the BDD manager. Some of the
/// fields are initialized for maxNumVars+1 or numVars+1, whereas some of them
/// are initialized for maxNumVars or numVars. The first kind of fields are
/// associated with the id of a variable and the second ones are with the index
/// of the variable.
///
/// \see Cal_BddManagerQuit
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_BddManager Cal_BddManagerInit();

////////////////////////////////////////////////////////////////////////////////
/// \brief Frees the BDD manager and all the associated allocations.
///
/// \see Cal_BddManagerInit
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddManagerQuit(Cal_BddManager bddManager);

/*---------------------------------------------------------------------------*/
/* | Hooks and Parameters                                                    */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns the hooks field of the manager.
///
/// \see Cal_BddManagerSetHooks
////////////////////////////////////////////////////////////////////////////////
EXTERN void * Cal_BddManagerGetHooks(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Sets the hooks field of the manager.
///
/// \sideeffect Hooks field changes.
///
/// \see Cal_BddManagerGetHooks
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddManagerSetHooks(Cal_BddManager bddManager, void *hooks);

////////////////////////////////////////////////////////////////////////////////
/// \brief Sets appropriate fields of BDD Manager.
///
/// \details This function is used to set the parameters which are used to
/// control the reordering process. These parameters have different affect on
/// the computational and memory usage aspects of reordeing. For instance,
/// higher value of `maxForwardedNodes` will result in process consuming more
/// memory, and a lower value on the other hand would invoke the cleanup process
/// repeatedly resulting in increased computation.
///
/// \param reorderingThreshold The number of nodes below which reordering will NOT be
///                            invoked.
///
/// \param maxForwardedNodes The maximum number of forwarded nodes which are
///                          allowed (at that point the cleanup must be done)
///
/// \param repackAfterGCThreshold The fraction of the page utilized that garbage
///                               collection has to achieve.
///
/// \param tableRepackThreshold The fraction of the page utilized below which
///                             repacking has to be invoked.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddManagerSetParameters(Cal_BddManager bddManager,
                                        long reorderingThreshold,
                                        long maxForwardedNodes,
                                        double repackAfterGCThreshold,
                                        double tableRepackThreshold);

/*---------------------------------------------------------------------------*/
/* | Statistics and Information                                              */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns the number of BDD nodes.
///
/// \see Cal_BddTotalSize
////////////////////////////////////////////////////////////////////////////////
EXTERN unsigned long Cal_BddManagerGetNumNodes(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns the number of nodes in the Unique table.
///
/// \see Cal_BddManagerGetNumNodes
////////////////////////////////////////////////////////////////////////////////
EXTERN unsigned long Cal_BddTotalSize(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns the number of BDD variables.
////////////////////////////////////////////////////////////////////////////////
EXTERN long Cal_BddVars(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns 1 if the node limit has been exceeded, 0 otherwise.
///
/// \sideeffect The overflow flag is cleared.
///
/// \see Cal_BddNodeLimit
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddOverflow(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Prints miscellaneous BDD statistics.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddStats(Cal_BddManager bddManager, FILE * fp);

/*---------------------------------------------------------------------------*/
/* | Memory and Garbage Collection                                           */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Sets the node limit to new_limit and returns the old limit.
///
/// \sideeffect Threshold for garbage collection may change.
///
/// \see Cal_BddManagerGC
////////////////////////////////////////////////////////////////////////////////
EXTERN long Cal_BddNodeLimit(Cal_BddManager bddManager, long newLimit);

////////////////////////////////////////////////////////////////////////////////
/// \brief Sets the garbage collection mode.
///
/// \param gcMode Set to 0 means the garbage collection should be turned off,
/// while 1 means garbage collection should be on.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddSetGCMode(Cal_BddManager bddManager, int gcMode);

////////////////////////////////////////////////////////////////////////////////
/// \brief Sets the limit of the garbage collection.
///
/// \details It tries to set the limit at twice the number of nodes in the
/// manager at the current point. However, the limit is not allowed to fall
/// below the `MIN_GC_LIMIT` or to exceed the value of node limit (if one
/// exists).
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddManagerSetGCLimit(Cal_BddManager manager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Invokes the garbage collection at the manager level.
///
/// \details For each variable in the increasing id free nodes with reference
/// count equal to zero freeing a node results in decrementing reference count
/// of then and else nodes by one.
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddManagerGC(Cal_BddManager bddManager);

/*---------------------------------------------------------------------------*/
/* | Variable Reordering                                                     */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Specify dynamic reordering technique and method.
///
/// \see Cal_BddReorder
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddDynamicReordering(Cal_BddManager bddManager,
                                     int technique,
                                     int method);

////////////////////////////////////////////////////////////////////////////////
/// \brief Invoke the current dynamic reodering method.
///
/// \sideeffect Indexes of a variable may change due to reodering.
///
/// \see Cal_BddDynamicReordering
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddReorder(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Creates and returns a variable block used for controlling dynamic
/// reordering.
///
/// \details The block is specified by passing the first variable and the length
/// of the block. The "length" number of consecutive variables starting from
/// "variable" are put in the block.
///
/// \sideeffect A new block is created.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Block Cal_BddNewVarBlock(Cal_BddManager bddManager,
                                    Cal_Bdd variable,
                                    long length);

////////////////////////////////////////////////////////////////////////////////
/// \brief Sets the reoderability of a particular block.
///
/// \details If a block is reorderable, the child blocks are recursively
/// involved in swapping.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddVarBlockReorderable(Cal_BddManager bddManager,
                                       Cal_Block block,
                                       int reorderable);

/*---------------------------------------------------------------------------*/
/* | Association List                                                        */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Creates or finds a variable association.
///
///
/// \param associationInfoUserBdds An array of BDD with Cal_BddNull(bddManager)
///                                as the end marker.
///
/// \param pairs If 0, the array assumed to be an array of variables. Otherwise,
///              it is interpreted as consecutive
///              pairs of variables.
///
/// \details If `pairs` is non-zero, then `associationInfoUserBdds` must be of
///          even length, the even numbered array elements should be variables,
///          and the odd numbered elements should be the BDDs which they are
///          mapped to.
///
/// \returns An integer identifier for this association. If the given
///          association is equivalent to one which already exists, the same
///          identifier is used for both, and the reference count of the
///          association is increased by one.
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_AssociationInit(Cal_BddManager bddManager,
                               Cal_Bdd *associationInfoUserBdds,
                               int pairs);

////////////////////////////////////////////////////////////////////////////////
/// \brief Deletes the variable association given by id.
///
/// \details Decrements the reference count of the variable association with
///          identifier id, and frees it if the reference count becomes zero.
///
/// \see Cal_AssociationInit
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_AssociationQuit(Cal_BddManager bddManager, int associationId);

////////////////////////////////////////////////////////////////////////////////
/// \brief Sets the current variable association to the one given.
///
/// \param associationId The id of the association to currently use. Set it to
/// -1 if the temporary association should be used.
///
/// \returns ID of the prior association (if any). A return value of -1
/// indicates the temporary association.
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_AssociationSetCurrent(Cal_BddManager bddManager,
                                     int associationId);

////////////////////////////////////////////////////////////////////////////////
/// \brief Sets the temporary variable association.
///
/// \param pairs Set this to 0 if the information represents only a list of
/// variables rather than a full association.
///
/// \see Cal_AssociationInit
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_TempAssociationInit(Cal_BddManager bddManager,
                                    Cal_Bdd *associationInfoUserBdds,
                                    int pairs);

////////////////////////////////////////////////////////////////////////////////
/// \brief Adds to the temporary variable association.
///
/// \param pairs Similar to `Cal_TempAssociationInit`.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_TempAssociationAugment(Cal_BddManager bddManager,
                                       Cal_Bdd *associationInfoUserBdds,
                                       int pairs);

////////////////////////////////////////////////////////////////////////////////
/// \brief Cleans up temporary association.
///
/// \see Cal_TempAssociationInit
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_TempAssociationQuit(Cal_BddManager bddManager);

/*---------------------------------------------------------------------------*/
/* | Save / Load of BDDs                                                     */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Write a BDD to a file.
///
/// \userVars A null-terminated array of variables that include the support of
/// `f`. These variables need not be in order of increasing index.
///
/// \param fp File to write to
///
/// \returns A nonzero value if f was written to the file successfully.
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddDumpBdd(Cal_BddManager bddManager,
                          Cal_Bdd fUserBdd,
                          Cal_Bdd * userVars,
                          FILE * fp);

////////////////////////////////////////////////////////////////////////////////
/// \brief Reads a BDD from a file.
///
/// \details If the same `userVars` is used in dumping and undumping, the BDD
/// returned will be equal to the one that was dumped. More generally, if array
/// v1 is used when dumping, and the array v2 is used when undumping, the BDD
/// returned will be equal to the original BDD with the ith variable in v2
/// substituted for the ith variable in v1 for all i.
///
/// \params userVars A null terminated array of variables that will become the
/// support of the BDD. As in `Cal_BddDumpBdd`, these need not be in the order
/// of increasing index.
///
/// \params fp File to load from.
///
/// \returns The desired BDD or Null if the operation fails for some reason
/// (node limit reached, I/O error, invalid file format, etc.). If it failed, an
/// error code is stored in error. the code will be one of the following.
/// `CAL_BDD_UNDUMP_FORMAT` Invalid file format `CAL_BDD_UNDUMP_OVERFLOW` Node
/// limit exceeded `CAL_BDD_UNDUMP_IOERROR` File I/O error `CAL_BDD_UNDUMP_EOF`
/// Unexpected EOF
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddUndumpBdd(Cal_BddManager bddManager,
                                Cal_Bdd * userVars,
                                FILE * fp,
                                int * error);

/// \}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \name BDD
///
/// \{

////////////////////////////////////////////////////////////////////////////////
/// \brief Frees the argument BDD.
///
/// \details It is an error to free a BDD more than once.
///
/// \sideeffect The reference count of the argument BDD is decreased by 1.
///
/// \see Cal_BddUnFree
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddFree(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Unfrees the argument BDD.
///
/// \details It is an error to pass a BDD with reference count of zero to be
/// unfreed.
///
/// \sideeffect The reference count of the argument BDD is increased by 1.
///
/// \see Cal_BddFree
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddUnFree(Cal_BddManager bddManager, Cal_Bdd userBdd);

/*---------------------------------------------------------------------------*/
/* | Basic Constructors                                                      */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns the NULL BDD.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddNull(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief The BDD for the constant one.
///
/// \see Cal_BddZero
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddOne(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief The BDD for the constant zero.
///
/// \see Cal_BddOne
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddZero(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Create and obtain a new variable at the start of the variable order.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddManagerCreateNewVarFirst(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Create and obtain a new variable at the end of the variable order.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddManagerCreateNewVarLast(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Create and obtain a new variable before the specified one in the
/// variable order.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddManagerCreateNewVarBefore(Cal_BddManager bddManager,
                                                Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Create and obtain a new variable after the specified one in the
/// variable order.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddManagerCreateNewVarAfter(Cal_BddManager bddManager,
                                               Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief The variable with the specified index, null if no such variable
/// exists.
///
/// \see Cal_BddManagerGetVarWithId
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddManagerGetVarWithIndex(Cal_BddManager bddManager,
                                             Cal_BddIndex_t index);

////////////////////////////////////////////////////////////////////////////////
/// \brief The variable with the specified id, null if no such variable exists
///
/// \see Cal_BddManagerGetVarWithIndex
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddManagerGetVarWithId(Cal_BddManager bddManager,
                                          Cal_BddId_t id);

/*---------------------------------------------------------------------------*/
/* | Predicates                                                              */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief 1 if the argument BDD is NULL, 0 otherwise.
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddIsBddNull(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns 1 if the argument BDD is constant one, 0 otherwise.
///
/// \see Cal_BddIsBddZero, Cal_BddIsBddConst
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddIsBddOne(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns 1 if the argument BDD is constant zero, 0 otherwise.
///
/// \see Cal_BddIsBddOne, Cal_BddIsBddConst
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddIsBddZero(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns 1 if the argument BDD is either constant one or constant
///        zero, otherwise returns 0.
///
/// \see Cal_BddIsBddOne, Cal_BddIsBddZero
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddIsBddConst(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns 1 if the argument BDD is a cube, 0 otherwise.
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddIsCube(Cal_BddManager bddManager, Cal_Bdd fUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns 1 if argument BDDs are equal, 0 otherwise.
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddIsEqual(Cal_BddManager bddManager,
                          Cal_Bdd userBdd1,
                          Cal_Bdd userBdd2);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns 1 if f depends on var and returns 0 otherwise.
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddDependsOn(Cal_BddManager bddManager,
                            Cal_Bdd fUserBdd,
                            Cal_Bdd varUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief BDD size of `fUserBdd`.
///
/// \param negout If non-zero then the number of nodes in `fUserBdd` is counted.
///               If zero, the counting pretends the BDDs do not have
///               negative-output pointers.
////////////////////////////////////////////////////////////////////////////////
EXTERN long Cal_BddSize(Cal_BddManager bddManager,
                        Cal_Bdd fUserBdd,
                        int negout);

////////////////////////////////////////////////////////////////////////////////
/// \brief Similar to `Cal_BddSize` for a null-terminated array of BDDs and
/// accounting for sharing of nodes.
////////////////////////////////////////////////////////////////////////////////
EXTERN long Cal_BddSizeMultiple(Cal_BddManager bddManager,
                                Cal_Bdd *fUserBddArray,
                                int negout);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns the support of f as a null-terminated array of variables.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddSupport(Cal_BddManager bddManager,
                           Cal_Bdd fUserBdd,
                           Cal_Bdd *support);

////////////////////////////////////////////////////////////////////////////////
/// \brief The number of nodes at each level in f.
///
/// \param levelCounts An array of size `Cal_BddVars(bddManager)+1` to hold the
///        profile.
///
/// \param negout Similar as in `Cal_BddSize`.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddProfile(Cal_BddManager bddManager,
                           Cal_Bdd fUserBdd,
                           long * levelCounts,
                           int negout);

////////////////////////////////////////////////////////////////////////////////
/// \brief Similar to `Cal_BddProfile` for a null-terminated array of BDDs and
/// accounting for sharing of nodes.
///
/// \see Cal_BddProfile
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddProfileMultiple(Cal_BddManager bddManager,
                                   Cal_Bdd *fUserBddArray,
                                   long * levelCounts,
                                   int negout);

////////////////////////////////////////////////////////////////////////////////
/// \brief The number of subfunctions of `f` which may be obtained by
/// restricting variables with an index lower than *n*.
///
/// \details The nth entry of the function profile array is the number of
/// subfunctions of f which may be obtained by restricting the variables whose
/// index is less than n. An entry of zero indicates that f is independent of
/// the variable with the corresponding index.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddFunctionProfile(Cal_BddManager bddManager,
                                   Cal_Bdd fUserBdd,
                                   long * funcCounts);

////////////////////////////////////////////////////////////////////////////////
/// \brief Similar to `Cal_BddFunctionProfile` for a null-terminated array of
/// BDDs and accounting for sharing of nodes.
///
/// \see Cal_BddFunctionProfile
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddFunctionProfileMultiple(Cal_BddManager bddManager, Cal_Bdd *fUserBddArray, long * funcCounts);

////////////////////////////////////////////////////////////////////////////////
/// \brief Prints a BDD in the human readable form.
///
/// \param VarNamingFn A pointer to a function taking a bddManager, a BDD and
///                    the pointer given by env. This function should return
///                    either a null pointer or a string that is the name of the
///                    supplied variable. If it returns a null pointer, a
///                    default name is generated based on the index of the
///                    variable. It is also legal for naminFN to be null; in
///                    this case, default names are generated for all variables.
///                    The macro bddNamingFnNone is a null pointer of suitable
///                    type.
/// \param TerminalIdFn A pointer to a function taking a bddManager, two longs,
///                     and the pointer given by the env. It should return
///                     either a null pointer. If it returns a null pointer, or
///                     if `terminalIdFn` is null, then default names are
///                     generated for the terminals. The macro
///                     bddTerminalIdFnNone is a null pointer of suitable type.
///
/// \param fp File to print the output to.
///
/// \see Cal_BddNamingFnNone, Cal_BddTerminalIdFnNone
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddPrintBdd(Cal_BddManager bddManager,
                            Cal_Bdd fUserBdd,
                            Cal_VarNamingFn_t VarNamingFn,
                            Cal_TerminalIdFn_t TerminalIdFn,
                            Cal_Pointer_t env,
                            FILE *fp);

////////////////////////////////////////////////////////////////////////////////
/// \brief Displays the node profile for `f` on `fp`.
///
/// \param lineLength The maximum line length.
///
/// \param varNamingFn similar to `Cal_BddPrintBdd`.
///
/// \see Cal_BddNamingFnNone
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddPrintProfile(Cal_BddManager bddManager,
                                Cal_Bdd fUserBdd,
                                Cal_VarNamingFn_t varNamingProc,
                                char * env,
                                int lineLength,
                                FILE * fp);

////////////////////////////////////////////////////////////////////////////////
/// \brief Similar to `Cal_BddPrintProfile` but displays the profile for a set
/// of BDDs.
///
/// \see Cal_BddPrintProfile, Cal_BddNamingFnNone
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddPrintProfileMultiple(Cal_BddManager bddManager,
                                        Cal_Bdd *userBdds,
                                        Cal_VarNamingFn_t varNamingProc,
                                        char * env,
                                        int lineLength,
                                        FILE * fp);

////////////////////////////////////////////////////////////////////////////////
/// \brief Similar to `Cal_BddPrintProfile` but displays a function profile for `f`.
///
/// \see Cal_BddPrintProfile, Cal_BddNamingFnNone
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddPrintFunctionProfile(Cal_BddManager bddManager,
                                        Cal_Bdd f,
                                        Cal_VarNamingFn_t varNamingProc,
                                        char * env,
                                        int lineLength,
                                        FILE * fp);

////////////////////////////////////////////////////////////////////////////////
/// \brief Similar to `Cal_BddPrintFunctionProfile` but displays a function
/// profile for a set of BDDs.
///
/// \see Cal_BddPrintFunctionProfile, Cal_BddNamingFnNone
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddPrintFunctionProfileMultiple(Cal_BddManager bddManager,
                                                Cal_Bdd *userBdds,
                                                Cal_VarNamingFn_t varNamingProc,
                                                char * env,
                                                int lineLength,
                                                FILE * fp);

////////////////////////////////////////////////////////////////////////////////
/// \brief Prints the function implemented by the argument BDD.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_BddFunctionPrint(Cal_BddManager bddManager,
                                 Cal_Bdd userBdd,
                                 char *name);

////////////////////////////////////////////////////////////////////////////////
/// \brief Duplicate of the argument BDD.
///
/// \sideeffect The reference count of the BDD is increased by 1.
///
/// \see Cal_BddNot
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddIdentity(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns a BDD with positive from a given BDD with arbitrary phase.
///
/// \warn This does **not** increase the reference count of the BDD.
///
/// \see Cal_BddNot
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddGetRegular(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Complement of the argument BDD.
///
/// \sideeffect The reference count of the argument BDD is increased by 1.
///
/// \see Cal_BddIdentity
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddNot(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Substitute a BDD variable by a function.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddCompose(Cal_BddManager bddManager,
                              Cal_Bdd fUserBdd,
                              Cal_Bdd gUserBdd,
                              Cal_Bdd hUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Computes a BDD that implies conjunction of `f` and `g`.
///
/// \sideffect None
///
/// \see Cal_BddImplies
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddIntersects(Cal_BddManager bddManager,
                                 Cal_Bdd fUserBdd,
                                 Cal_Bdd gUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Computes a BDD that implies conjunction of `f` and `Cal_BddNot(g)`.
///
/// \sideffect None
///
/// \see Cal_BddIntersects
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddImplies(Cal_BddManager bddManager,
                              Cal_Bdd fUserBdd,
                              Cal_Bdd gUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical If-Then-Else.
///
/// \details Returns the BDD for the logical operation `f ? g : h`, i.e.
/// `f&g | ~f&h`.
///
/// \see Cal_BddAnd, Cal_BddNand, Cal_BddOr, Cal_BddNor, Cal_BddXor, Cal_BddXnor
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddITE(Cal_BddManager bddManager,
                          Cal_Bdd fUserBdd,
                          Cal_Bdd gUserBdd,
                          Cal_Bdd hUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical AND of argument BDDs.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddAnd(Cal_BddManager bddManager,
                          Cal_Bdd fUserBdd,
                          Cal_Bdd gUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical AND of BDDs in argument null-terminated array of BDDs.
///
/// \see Cal_BddAnd
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddMultiwayAnd(Cal_BddManager bddManager,
                                  Cal_Bdd *userBddArray);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical NAND of argument BDDs.
///
/// \see Cal_BddAnd, Cal_BddNot
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddNand(Cal_BddManager bddManager,
                           Cal_Bdd fUserBdd,
                           Cal_Bdd gUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical Or of argument BDDs.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddOr(Cal_BddManager bddManager,
                         Cal_Bdd fUserBdd,
                         Cal_Bdd gUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical OR of BDDs in argument null-terminated array of BDDs.
///
/// \see Cal_BddOr
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddMultiwayOr(Cal_BddManager bddManager,
                                 Cal_Bdd *userBddArray);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical NOR of argument BDDs.
///
/// \see Cal_BddOr, Cal_BddNot
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddNor(Cal_BddManager bddManager,
                          Cal_Bdd fUserBdd,
                          Cal_Bdd gUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical XOR of argument BDDs.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddXor(Cal_BddManager bddManager,
                          Cal_Bdd fUserBdd,
                          Cal_Bdd gUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical XOR of BDDs in argument null-terminated array of BDDs.
///
/// \see Cal_BddXor
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddMultiwayXor(Cal_BddManager bddManager,
                                  Cal_Bdd *userBddArray);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical XNOR of argument BDDs.
///
/// \see Cal_BddXor, Cal_BddNot
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddXnor(Cal_BddManager bddManager,
                           Cal_Bdd fUserBdd,
                           Cal_Bdd gUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical AND of BDD pairs in argument null-termiinated array of BDDs.
///
/// \details Returns an array of BDDs obtained by logical AND of BDD pairs
/// specified by an BDD array in which a BDD at an even location is paired with
/// a BDD at an odd location of the array
///
/// \see Cal_BddPairwiseOr, Cal_BddPairwiseXor
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd * Cal_BddPairwiseAnd(Cal_BddManager bddManager,
                                    Cal_Bdd *userBddArray);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical OR of BDD pairs in argument null-termiinated array of BDDs.
///
/// \details Returns an array of BDDs obtained by logical OR of BDD pairs
/// specified by an BDD array in which a BDD at an even location is paired with
/// a BDD at an odd location of the array
///
/// \see Cal_BddPairwiseAnd, Cal_BddPairwiseXor
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd * Cal_BddPairwiseOr(Cal_BddManager bddManager,
                                   Cal_Bdd *userBddArray);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical XOR of BDD pairs in argument null-termiinated array of BDDs.
///
/// \details Returns an array of BDDs obtained by logical XOR of BDD pairs
/// specified by an BDD array in which a BDD at an even location is paired with
/// a BDD at an odd location of the array
///
/// \see Cal_BddPairwiseAnd, Cal_BddPairwiseOr
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd * Cal_BddPairwiseXor(Cal_BddManager bddManager,
                                    Cal_Bdd *userBddArray);

////////////////////////////////////////////////////////////////////////////////
/// \brief Substitute a set of variables by functions.
///
/// \details Returns a BDD for f using the substitution defined by current
/// variable association. Each variable is replaced by its associated BDDs. The
/// substitution is effective simultaneously.
///
/// \see Cal_BddCompose, Cal_BddVarSubstitute
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddSubstitute(Cal_BddManager bddManager, Cal_Bdd fUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Substitute a set of variables by set of another variables.
///
/// \details Returns a BDD for f using the substitution defined by current
/// variable association. It is assumed that each variable is replaced by
/// another variable.
///
/// \remark For the substitution of a variable by a function, use
/// `Cal::Substitute()` instead.
///
/// \see Cal_BddSubstitute
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddVarSubstitute(Cal_BddManager bddManager,
                                    Cal_Bdd fUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief The function obtained by swapping two variables.
///
/// \details Returns the BDD obtained by simultaneously substituting variable
/// g by variable h and variable h and variable g in the BDD f.
///
/// \see Cal_BddSubstitute
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddSwapVars(Cal_BddManager bddManager,
                               Cal_Bdd fUserBdd,
                               Cal_Bdd gUserBdd,
                               Cal_Bdd hUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Existentially quantification of some variables from the given BDD.
///
/// \details Returns the BDD for f with all the variables that are
/// paired with something in the current variable association
/// existentially quantified out.
///
/// \see Cal_AssociationInit, Cal_AssociationSetCurrent, Cal_TempAssociationInit
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddExists(Cal_BddManager bddManager, Cal_Bdd fUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Universal quantification of some variables from the given BDD.
///
/// \details Returns the BDD for f with all the variables that are
/// paired with something in the current variable association
/// universally quantified out.
///
/// \see Cal_AssociationInit, Cal_AssociationSetCurrent, Cal_TempAssociationInit
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddForAll(Cal_BddManager bddManager, Cal_Bdd fUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Logical AND of the argument BDDs and existentially quantifying some
/// variables from the product.
///
/// \see Cal_BddAnd, Cal_BddExists, Cal_AssociationInit,
///      Cal_AssociationSetCurrent, Cal_TempAssociationInit
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddRelProd(Cal_BddManager bddManager,
                              Cal_Bdd fUserBdd,
                              Cal_Bdd gUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Generalized cofactor of `f` with respect to `c`.
///
/// \details Returns the generalized cofactor of BDD f with respect to BDD c.
/// The constrain operator given by Coudert et al (ICCAD90) is used to find the
/// generalized cofactor.
///
/// \see Cal_BddReduce
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddCofactor(Cal_BddManager bddManager,
                               Cal_Bdd fUserBdd,
                               Cal_Bdd cUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief A function that agrees with `f` for all valuations which satisfy `c`.
///
/// \details Returns a BDD which agrees with `f` for all valuations which
/// satisfy `c`. The result is usually smaller in terms of number of BDD nodes
/// than f. This operation is typically used in state space searches to simplify
/// the representation for the set of states wich will be expanded at each step.
///
/// \see Cal_BddCofactor
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddReduce(Cal_BddManager bddManager,
                             Cal_Bdd fUserBdd,
                             Cal_Bdd cUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief A function that contains `fMin` and is contained in `fMax`.
///
/// \details Returns a minimal BDD f which is contains fMin and is contained in
/// `fMax` (`fMin <= f <= fMax`). This operation is typically used in state
/// space searches to simplify the representation for the set of states wich
/// will be expanded at each step (`Rk Rk-1' <= f <= Rk`).
///
/// \see Cal_BddReduce
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddBetween(Cal_BddManager bddManager,
                              Cal_Bdd fMinUserBdd,
                              Cal_Bdd fMaxUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief A satisfying assignment of `f`.
///
/// \details Returns a BDD which implies `f`, true for some valuation on which f
/// is true, and which has at most one node at each level.
///
/// \see Cal_BddSatisfySupport
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddSatisfy(Cal_BddManager bddManager, Cal_Bdd fUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns a special cube contained in `f`.
///
/// \details The returned BDD which implies f, is true for some valuation on
/// which f is true, which has at most one node at each level, and which has
/// exactly one node corresponding to each variable which is associated with
/// something in the current variable association.
///
/// \see Cal_BddSatisfySupport
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddSatisfySupport(Cal_BddManager bddManager,
                                     Cal_Bdd fUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Fraction of valuations which make `f` true.
///
/// \remark This fraction is independent of whatever set of variables `f` is
/// supposed to be a function of.
////////////////////////////////////////////////////////////////////////////////
EXTERN double Cal_BddSatisfyingFraction(Cal_BddManager bddManager,
                                        Cal_Bdd fUserBdd);

/*---------------------------------------------------------------------------*/
/* | Node Information and Traversal                                          */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief The type of the given BDD (0, 1, +var, -var, overflow, nonterminal).
///
/// \details Returns `BDD_TYPE_ZERO` if f is false, `BDD_TYPE_ONE` if f is true,
/// `BDD_TYPE_POSVAR` is f is an unnegated variable, `BDD_TYPE_NEGVAR` if f is a
/// negated variable, `BDD_TYPE_OVERFLOW` if f is null, and
/// `BDD_TYPE_NONTERMINAL` otherwise.
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddType(Cal_BddManager bddManager, Cal_Bdd fUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns the index of the top variable of the argument BDD.
///
/// \see Cal_BddGetIfId
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_BddId_t Cal_BddGetIfIndex(Cal_BddManager bddManager,
                                     Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns the id of the top variable of the argument BDD.
///
/// \see Cal_BddGetIfIndex
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_BddId_t Cal_BddGetIfId(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief A BDD corresponding to the top variable of the given BDD.
///
/// \see Cal_BddIfId, Cal_BddIfIndex
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddIf(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief The positive cofactor of the argument BDD with respect to its top
/// variable.
///
/// \see Cal_BddElse
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddThen(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief The negative cofactor of the argument BDD with respect to its top
/// variable.
///
/// \see Cal_BddThen
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_BddElse(Cal_BddManager bddManager, Cal_Bdd userBdd);

/// \}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \name Pipelined BDD Manipulation
///
/// \{

////////////////////////////////////////////////////////////////////////////////
/// \brief Initialize a BDD pipeline.
///
/// \remark All the operations for this pipeline must be of the same kind.
///
/// \see Cal_PipelineQuit
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_PipelineInit(Cal_BddManager bddManager, Cal_BddOp_t bddOp);

////////////////////////////////////////////////////////////////////////////////
/// \brief Resets the pipeline freeing all resources.
///
/// \remark Make sure to update all provisional BDDs of interest before calling
/// this routine.
///
/// \see Cal_BddIsProvisional, Cal_PipelineUpdateProvisionalBdd
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_PipelineQuit(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Set depth of a BDD pipeline.
///
/// \details The "depth" determines the amount of dependency we / would allow in
/// pipelined computation.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_PipelineSetDepth(Cal_BddManager bddManager, int depth);

////////////////////////////////////////////////////////////////////////////////
/// \brief Executes a pipeline.
///
/// \details All the results are computed. User should update the BDDs of
///          interest.
///
/// \todo This feature should eventually become transparent.
///
/// \returns 1 if the pipeline was succesfull, 0 otherwise.
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_PipelineExecute(Cal_BddManager bddManager);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns 1, if the given user BDD is a provisional BDD node.
///
/// \details A provisional BDD is automatically freed once the pipeline is
///          quitted.
////////////////////////////////////////////////////////////////////////////////
EXTERN int Cal_BddIsProvisional(Cal_BddManager bddManager, Cal_Bdd userBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Create a provisional BDD in the pipeline.
///
/// \details The provisional BDD is automatically freed once the pipeline is
///          quitted.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_PipelineCreateProvisionalBdd(Cal_BddManager bddManager,
                                                Cal_Bdd fUserBdd,
                                                Cal_Bdd gUserBdd);

////////////////////////////////////////////////////////////////////////////////
/// \brief Update a provisional BDD obtained during pipelining.
///
/// \details The provisional BDD is automatically freed after quitting pipeline.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Bdd Cal_PipelineUpdateProvisionalBdd(Cal_BddManager bddManager,
                                                Cal_Bdd provisionalBdd);

/// \}
////////////////////////////////////////////////////////////////////////////////

/// \}
////////////////////////////////////////////////////////////////////////////////

#endif /* _CAL */
