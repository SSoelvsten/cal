/**CHeaderFile*****************************************************************

  FileName    [calMem.h]

  PackageName [cal]

  Synopsis    [Header file for memory management]

  Description [ ]

  SeeAlso     []

  Author      [Rajeev K. Ranjan (rajeev@eecs.berkeley.edu). Originally written by David Long. ]

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

  Revision    [$Id: calMem.h,v 1.2 1998/09/17 03:19:18 ravi Exp $]

******************************************************************************/

#ifndef _CAL_MEM
#define _CAL_MEM

#include <stdio.h>

/// \cond internal

////////////////////////////////////////////////////////////////////////////////
/// \addtogroup module__c
///
/// \{

////////////////////////////////////////////////////////////////////////////////
/// \name Memory Management
///
/// \{

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/* CAL_ALLOC_ALIGNMENT is the alignment for all storage returned by the */
/* storage allocation routines. */
/* was 16 for __osf__ systems, 8 otherwise */

#define CAL_ALLOC_ALIGNMENT sizeof(void *)

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct Cal_RecMgrStruct * Cal_RecMgr;
typedef void *Cal_Pointer_t;
typedef size_t Cal_Address_t;

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#ifndef EXTERN
#define EXTERN extern
#endif

#define Cal_Nil(obj) ((obj *)0)
#define Cal_MemAlloc(type, num) ((type *) malloc(sizeof(type) * (num)))
#define Cal_MemRealloc(type, obj, num)                              \
  (obj) ? ((type *) realloc((char *) obj, sizeof(type) * (num))) :  \
       ((type *) malloc(sizeof(type) * (num)))
#define Cal_MemFree(obj) ((obj) ? (free((char *) (obj)), (obj) = 0) : 0)
#define Cal_MemCopy(dest, src, size)  ((void *) memcpy((void *)dest, (const void *)src, (size_t)size));
#define Cal_MemZero(ptr, size) ((void)memset((void *)(ptr), 0, (Cal_Address_t)(size)))

/* Round a size up for alignment */

#define CAL_ROUNDUP(size) ((((size)+CAL_ALLOC_ALIGNMENT-1)/CAL_ALLOC_ALIGNMENT)*CAL_ALLOC_ALIGNMENT)

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

////////////////////////////////////////////////////////////////////////////////
/// \brief Prints an error message and exits.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_MemFatal(char *message);

////////////////////////////////////////////////////////////////////////////////
/// \brief Returns the memory allocated.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Address_t Cal_MemAllocation(void);

////////////////////////////////////////////////////////////////////////////////
/// \brief Allocates a new block of the specified size.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Pointer_t Cal_MemGetBlock(Cal_Address_t size);

////////////////////////////////////////////////////////////////////////////////
/// \brief Frees the block.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_MemFreeBlock(Cal_Pointer_t p);

////////////////////////////////////////////////////////////////////////////////
/// \brief Expands or contracts the block to a new size. We try to avoid
/// moving the block if possible.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Pointer_t Cal_MemResizeBlock(Cal_Pointer_t p, Cal_Address_t newSize);

////////////////////////////////////////////////////////////////////////////////
/// \brief Allocates a record from the specified record manager.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_Pointer_t Cal_MemNewRec(Cal_RecMgr mgr);

////////////////////////////////////////////////////////////////////////////////
/// \brief Frees a record managed by the indicated record manager.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_MemFreeRec(Cal_RecMgr mgr, Cal_Pointer_t rec);

////////////////////////////////////////////////////////////////////////////////
/// \brief Creates a new record manager with the given  record size.
////////////////////////////////////////////////////////////////////////////////
EXTERN Cal_RecMgr Cal_MemNewRecMgr(int size);

////////////////////////////////////////////////////////////////////////////////
/// \brief Frees all the storage associated with the specified record manager.
////////////////////////////////////////////////////////////////////////////////
EXTERN void Cal_MemFreeRecMgr(Cal_RecMgr mgr);

/// \endcond

#endif /* _CAL */
