/**CFile***********************************************************************

  FileName    [calDump.c]

  PackageName [cal]

  Synopsis    [BDD library dump/undump routines]
              

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

  Revision    [$Id: calDump.c,v 1.1.1.3 1998/05/04 00:58:56 hsv Exp $]

******************************************************************************/
#include "calInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define MAGIC_COOKIE 0x5e02f795l
#define CAL_BDD_IOERROR 100

#define TRUE_ENCODING 0xffffff00l
#define FALSE_ENCODING 0xffffff01l
#define POSVAR_ENCODING 0xffffff02l
#define NEGVAR_ENCODING 0xffffff03l
#define POSNODE_ENCODING 0xffffff04l
#define NEGNODE_ENCODING 0xffffff05l
#define NODELABEL_ENCODING 0xffffff06l
#define CONSTANT_ENCODING 0xffffff07l

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


static long indexMask[] = {0xffl, 0xffffl, 0xffffffl};

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void Write(Cal_BddManager_t * bddManager, unsigned long n, int bytes, FILE * fp);
static void BddDumpBddStep(Cal_BddManager_t * bddManager, Cal_Bdd_t f, FILE * fp, CalHashTable_t * h, Cal_BddIndex_t * normalizedIndexes, int indexSize, int nodeNumberSize);
static unsigned long Read(int * error, int bytes, FILE * fp);
static Cal_Bdd_t BddUndumpBddStep(Cal_BddManager_t * bddManager, Cal_Bdd_t * vars, FILE * fp, Cal_BddIndex_t numberVars, Cal_Bdd_t * shared, long numberShared, long * sharedSoFar, int indexSize, int nodeNumberSize, int * error);
static int BytesNeeded(long n);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Reads a BDD from a file]

  Description [Loads an encoded description of a BDD from the file given by
  fp. The argument vars should be a null terminated array of variables that will
  become the support of the BDD. As in Cal_BddDumpBdd, these need not be in
  the order of increasing index. If the same array of variables in used in 
  dumping and undumping, the BDD returned will be equal to the one that was 
  dumped. More generally, if array v1 is used when dumping, and the array v2
  is used when undumping, the BDD returned will be equal to the original BDD
  with the ith variable in v2 substituted for the ith variable in v1 for all i.
  Null BDD is returned in the operation fails for reason (node limit reached,
  I/O error, invalid file format, etc.). In this case, an error code is stored
  in error. the code will be one of the following. 
  CAL_BDD_UNDUMP_FORMAT Invalid file format
  CAL_BDD_UNDUMP_OVERFLOW Node limit exceeded
  CAL_BDD_UNDUMP_IOERROR File I/O error
  CAL_BDD_UNDUMP_EOF Unexpected EOF]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
Cal_Bdd
Cal_BddUndumpBdd(
  Cal_BddManager bddManager,
  Cal_Bdd * userVars,
  FILE * fp,
  int * error)
{
  long i,j;
  Cal_BddIndex_t numberVars;
  long numberShared;
  int indexSize;
  int nodeNumberSize;
  Cal_Bdd_t *shared;
  long sharedSoFar;
  Cal_Bdd_t v;
  Cal_Bdd_t result;
  Cal_Bdd_t *vars;

  *error = 0;
  for(i = 0; userVars[i]; ++i){
    if(Cal_BddType(bddManager, userVars[i]) !=  CAL_BDD_TYPE_POSVAR){
      CalBddWarningMessage("Cal_BddUndumpBdd: support is not all positive variables"); 
      return (Cal_Bdd) 0;
    }
  }
  vars = Cal_MemAlloc(Cal_Bdd_t, i);
  for (j=0; j < i; j++){
    vars[j] = CalBddGetInternalBdd(bddManager,userVars[j]);
  }

  if(Read(error, sizeof(long), fp) !=  MAGIC_COOKIE){
    if(!*error){
      *error = CAL_BDD_UNDUMP_FORMAT;
    }
    Cal_MemFree(vars);
    return (Cal_Bdd) 0;
  }
  numberVars = Read(error, sizeof(Cal_BddIndex_t), fp);
  if(*error){
    Cal_MemFree(vars);
    return (Cal_Bdd) 0;
  }
  if(numberVars !=  i){
    *error = CAL_BDD_UNDUMP_FORMAT;
    Cal_MemFree(vars);
    return (Cal_Bdd) 0;
  }
  numberShared = Read(error, sizeof(long), fp);
  if(*error){
    Cal_MemFree(vars);
    return (Cal_Bdd) 0;
  }
  indexSize = BytesNeeded(numberVars+1);
  nodeNumberSize = BytesNeeded(numberShared);
  if(numberShared < 0){
    *error = CAL_BDD_UNDUMP_FORMAT;
    Cal_MemFree(vars);
    return (Cal_Bdd) 0;
  }
  shared = Cal_MemAlloc(Cal_Bdd_t, numberShared);
  for(i = 0; i < numberShared; ++i){
    shared[i] = CalBddNull(bddManager);
  }
  sharedSoFar = 0;
  result = BddUndumpBddStep(bddManager, vars, fp, numberVars, shared,
      numberShared, &sharedSoFar, indexSize, nodeNumberSize, error);
  Cal_MemFree(vars);
  
  for(i = 0; i < numberShared; ++i){
    v = shared[i];
    if(!CalBddIsBddNull(bddManager, v)){
      CalBddFree(v);
    }
  }
  if(!*error && sharedSoFar !=  numberShared){
    *error = CAL_BDD_UNDUMP_FORMAT;
  }
  Cal_MemFree(shared);
  if(*error){
    if(!CalBddIsBddNull(bddManager, result)){
      CalBddFree(result);
    }
    return (Cal_Bdd) 0;
  }
  /*
   * Decrement the reference count of result by 1. Since it has
   * already been incremented in BddUndumpBddStep.
   */
  CalBddDcrRefCount(result);
  return CalBddGetExternalBdd(bddManager, result);
}


/**Function********************************************************************

  Synopsis    [Write a BDD to a file]

  Description [Writes an encoded description of the BDD to the file given by fp.
  The argument vars should be a null-terminated array of variables that include
  the support of f .  These variables need not be in order of increasing index.
  The function returns a nonzero value if f was written to the file successfully.
  ]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
int
Cal_BddDumpBdd(
  Cal_BddManager bddManager,
  Cal_Bdd  fUserBdd,
  Cal_Bdd * userVars,
  FILE * fp)
{
  long i;
  Cal_BddIndex_t *normalizedIndexes;
  Cal_BddIndex_t vIndex;
  Cal_Bdd_t f;
  Cal_BddIndex_t numberVars;
  Cal_Bdd *support;
  int ok;
  CalHashTable_t *h;
  int indexSize;
  long next;
  int nodeNumberSize;

  if(CalBddPreProcessing(bddManager, 1, fUserBdd)){
    f = CalBddGetInternalBdd(bddManager, fUserBdd);
    for(i = 0; userVars[i]; ++i){
      if(Cal_BddType(bddManager, userVars[i]) !=  CAL_BDD_TYPE_POSVAR){
        CalBddWarningMessage("Cal_BddDumpBdd: support is not all positive variables");
        return (0);
      }
    }
    normalizedIndexes = Cal_MemAlloc(Cal_BddIndex_t, bddManager->numVars);
    for(i = 0; i < bddManager->numVars; ++i){
      normalizedIndexes[i] = CAL_BDD_CONST_INDEX;
    }
    for(i = 0; userVars[i]; ++i){
      vIndex = Cal_BddGetIfIndex(bddManager, userVars[i]);
      if(normalizedIndexes[vIndex] !=  CAL_BDD_CONST_INDEX){
        CalBddWarningMessage("Cal_BddDumpBdd: variables duplicated in support");
        Cal_MemFree(normalizedIndexes);
        return 0;
      }
      normalizedIndexes[vIndex] = i;
    }
    numberVars = i;
    support = Cal_MemAlloc(Cal_Bdd, bddManager->numVars+1);
    Cal_BddSupport(bddManager, fUserBdd, support);
    ok = 1;
    for(i = 0; ok && support[i]; ++i){
      if(normalizedIndexes[Cal_BddGetIfIndex(bddManager, support[i])] == CAL_BDD_CONST_INDEX){
        CalBddWarningMessage("Cal_BddDumpBdd: incomplete support specified");
        ok = 0;
      }
    }
    if(!ok){
      Cal_MemFree(normalizedIndexes);
      Cal_MemFree(support);
      return 0;
    }
    Cal_MemFree(support);
    /* Everything checked now; barring I/O errors, we should be able to */
    /* Write a valid output file. */
    f = CalBddGetInternalBdd(bddManager, fUserBdd);
    h = CalHashTableOneInit(bddManager, sizeof(long));
    indexSize = BytesNeeded(numberVars+1);
    CalBddMarkSharedNodes(bddManager, f);
    next = 0;
    CalBddNumberSharedNodes(bddManager, f, h, &next);
    nodeNumberSize = BytesNeeded(next);
    Write(bddManager, MAGIC_COOKIE, sizeof(long), fp);
    Write(bddManager, (unsigned long)numberVars, sizeof(Cal_BddIndex_t), fp);
    Write(bddManager, (unsigned long)next, sizeof(long), fp);
    BddDumpBddStep(bddManager, f, fp, h, normalizedIndexes, indexSize, nodeNumberSize);
    CalHashTableOneQuit(h);
    Cal_MemFree(normalizedIndexes);
    return (1);
  }
  return (0);
}

/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
Write(
  Cal_BddManager_t * bddManager,
  unsigned long  n,
  int  bytes,
  FILE * fp)
{
  while(bytes){
    if(fputc((char)(n >> (8*(bytes-1)) & 0xff), fp) == EOF){
    }
    --bytes;
  }
}


/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
BddDumpBddStep(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t  f,
  FILE * fp,
  CalHashTable_t * h,
  Cal_BddIndex_t * normalizedIndexes,
  int  indexSize,
  int  nodeNumberSize)
{
  int negated;
  long *number;
  Cal_Bdd_t thenBdd, elseBdd;

  switch(CalBddTypeAux(bddManager, f)){
  case CAL_BDD_TYPE_ZERO:
    Write(bddManager, FALSE_ENCODING, indexSize+1, fp);
    break;
  case CAL_BDD_TYPE_ONE:
    Write(bddManager, TRUE_ENCODING, indexSize+1, fp);
    break;
  case CAL_BDD_TYPE_CONSTANT:
    Write(bddManager, CONSTANT_ENCODING, indexSize+1, fp);
#ifdef JAGESH
    Write(bddManager, (unsigned long)BDD_DATA(f)[0], sizeof(long), fp);
    Write(bddManager, (unsigned long)BDD_DATA(f)[1], sizeof(long), fp);
#endif
    break;
  case CAL_BDD_TYPE_POSVAR:
    Write(bddManager, POSVAR_ENCODING, indexSize+1, fp);
    Write(bddManager,
        (unsigned long)normalizedIndexes[CalBddGetBddIndex(bddManager, f)],
        indexSize, fp);
    break;
  case CAL_BDD_TYPE_NEGVAR:
    Write(bddManager, NEGVAR_ENCODING, indexSize+1, fp);
    Write(bddManager, 
        (unsigned long)normalizedIndexes[CalBddGetBddIndex(bddManager, f)],
        indexSize, fp);
    break;
  case CAL_BDD_TYPE_NONTERMINAL:
    CalBddNot(f, f);
    if(CalHashTableOneLookup(h, f, (char **)0)){
      negated  =  1;
    }
    else{
      CalBddNot(f, f);
      negated = 0;
    }
    CalHashTableOneLookup(h, f, (char **)&number);
    if(number && *number < 0){
	  if(negated)
	    Write(bddManager, NEGNODE_ENCODING, indexSize+1, fp);
	  else
	    Write(bddManager, POSNODE_ENCODING, indexSize+1, fp);
	  Write(bddManager, (unsigned long)(-*number-1), nodeNumberSize, fp);
    }
    else{
      if(number){
	      Write(bddManager, NODELABEL_ENCODING, indexSize+1, fp);
	      *number =  -*number-1;
      }
      Write(bddManager,
          (unsigned long)normalizedIndexes[CalBddGetBddIndex(bddManager, f)],
          indexSize, fp);
      CalBddGetThenBdd(f, thenBdd);
      BddDumpBddStep(bddManager, thenBdd, fp, h, normalizedIndexes,
          indexSize, nodeNumberSize);
      CalBddGetElseBdd(f, elseBdd);
      BddDumpBddStep(bddManager, elseBdd, fp, h, normalizedIndexes,
          indexSize, nodeNumberSize);
    }
    break;
  default:
    CalBddFatalMessage("BddDumpBddStep: unknown type returned by CalBddType");
  }
}



/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static unsigned long
Read(
  int * error,
  int  bytes,
  FILE * fp)
{
  int c;
  long result;

  result = 0;
  if(*error){
    return (result);
  }
  while(bytes){
    c = fgetc(fp);
    if(c == EOF){
      if(ferror(fp)){
        *error = CAL_BDD_UNDUMP_IOERROR;
      }
      else{
        *error = CAL_BDD_UNDUMP_EOF;
      }
      return (0l);
    }
    result = (result << 8)+c;
    --bytes;
  } 
  return (result);
}




/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static Cal_Bdd_t
BddUndumpBddStep(
  Cal_BddManager_t * bddManager,
  Cal_Bdd_t * vars,
  FILE * fp,
  Cal_BddIndex_t  numberVars,
  Cal_Bdd_t * shared,
  long  numberShared,
  long * sharedSoFar,
  int  indexSize,
  int  nodeNumberSize,
  int * error)
{
  long nodeNumber;
  long encoding;
  Cal_BddIndex_t i;
  CalAddress_t value1, value2;
  Cal_Bdd_t v;
  Cal_Bdd_t temp1, temp2;
  Cal_Bdd_t result;

  i = Read(error, indexSize, fp);
  if(*error){
    return CalBddNull(bddManager);
  }
  if(i == indexMask[indexSize-1]){
    encoding = 0xffffff00l+Read(error, 1, fp);
    if(*error){
      return CalBddNull(bddManager);
    }
    switch(encoding){
    case TRUE_ENCODING:
      return (CalBddOne(bddManager));
    case FALSE_ENCODING:
      return (CalBddZero(bddManager));
    case CONSTANT_ENCODING:
      value1 = Read(error, sizeof(long), fp);
      value2 = Read(error, sizeof(long), fp);
      if(*error){
        return CalBddNull(bddManager);
      }
      *error = CAL_BDD_UNDUMP_OVERFLOW;
      return CalBddNull(bddManager);
    case POSVAR_ENCODING:
    case NEGVAR_ENCODING:
      i = Read(error, indexSize, fp);
      if(!*error && i >=  numberVars){
        *error = CAL_BDD_UNDUMP_FORMAT;
      }
      if(*error){
        return CalBddNull(bddManager);
      }
      v = vars[i];
      if(encoding == POSVAR_ENCODING){
        return (v);
      }
      else{
        CalBddNot(v, v);
        return (v);
      }
    case POSNODE_ENCODING:
    case NEGNODE_ENCODING:
      nodeNumber = Read(error, nodeNumberSize, fp);
      if(!*error && (nodeNumber >=  numberShared ||
          CalBddIsBddNull(bddManager, shared[nodeNumber]))){
        *error = CAL_BDD_UNDUMP_FORMAT;
      }
      if(*error){
        return CalBddNull(bddManager);
      }
      v = shared[nodeNumber];
      v = CalBddIdentity(bddManager, v);
      if(encoding == POSNODE_ENCODING){
        return (v);
      }
      else{
        CalBddNot(v, v);
        return (v);
      }
    case NODELABEL_ENCODING:
      nodeNumber =  *sharedSoFar;
      ++*sharedSoFar;
      v = BddUndumpBddStep(bddManager, vars, fp, numberVars, shared,
          numberShared, sharedSoFar, indexSize, nodeNumberSize, error);
      shared[nodeNumber] = v;
      v = CalBddIdentity(bddManager, v);
      return (v);
    default:
      *error = CAL_BDD_UNDUMP_FORMAT;
      return CalBddNull(bddManager);
    }
  }
  if(i >= numberVars){
    *error = CAL_BDD_UNDUMP_FORMAT;
    return CalBddNull(bddManager);
  }
  temp1 = BddUndumpBddStep(bddManager, vars, fp, numberVars, shared,
       numberShared, sharedSoFar, indexSize, nodeNumberSize, error);
  temp2 = BddUndumpBddStep(bddManager, vars, fp, numberVars, shared,
       numberShared, sharedSoFar, indexSize, nodeNumberSize, error);
  if(*error){
      CalBddFree(temp1);
      return CalBddNull(bddManager);
  }
  result = CalBddITE(bddManager, vars[i], temp1, temp2);
  CalBddFree(temp1);
  CalBddFree(temp2);
  if(CalBddIsBddNull(bddManager, result)){
     *error = CAL_BDD_UNDUMP_OVERFLOW;
  }
  return (result);
}



/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static int
BytesNeeded(
  long  n)
{
  if(n <= 0x100l){
    return (1);
  }
  if(n <= 0x10000l){
    return (2);
  }
  if(n <= 0x1000000l){
    return (3);
  }
  return (4);
}












