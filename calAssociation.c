/**CFile***********************************************************************

  FileName    [calAssociation.c]

  PackageName [cal]

  Synopsis    [Contains the routines related to the variable association.]

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

  Revision    [$Id: calAssociation.c,v 1.1.1.3 1998/05/04 00:58:50 hsv Exp $]

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

static int AssociationIsEqual(Cal_BddManager_t * bddManager, Cal_Bdd_t * p, Cal_Bdd_t * q);
static int CheckAssoc(Cal_BddManager_t *bddManager, Cal_Bdd *assocInfo, int pairs);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Creates or finds a variable association.]

  Description [Creates or finds a variable association. The association is
  specified by associationInfo, which is a an array of BDD with 
  Cal_BddNull(bddManager) as the end marker. If pairs is 0, the array is
  assumed to be an array of variables. In this case, each variable is paired
  with constant BDD one. Such an association may viewed as specifying a set
  of variables for use with routines such as Cal_BddExists. If pair is not 0,
  then the even numbered array elements should be variables and the odd numbered
  elements should be the BDDs which they are mapped to. In both cases, the 
  return value is an integer identifier for this association. If the given
  association is equivalent to one which already exists, the same identifier
  is used for both, and the reference count of the association is increased by
  one.]

  SideEffects [None]

  SeeAlso     [Cal_AssociationQuit]

******************************************************************************/
int
Cal_AssociationInit(Cal_BddManager bddManager,
                    Cal_Bdd *associationInfoUserBdds,
                    int  pairs)
{
  int i, numAssociations;
  CalAssociation_t *p, **q;
  Cal_Bdd_t f;
  Cal_Bdd_t *varAssociation;
  Cal_BddId_t j;
  long last;
  Cal_Bdd_t *associationInfo;
  
  if (!CheckAssoc(bddManager, associationInfoUserBdds, pairs)){
    return (-1);
  }


/* First count the number of elements */
  for (i=0; associationInfoUserBdds[i]; i++);
  if (pairs)  numAssociations = i/2;
  else numAssociations = i;
  associationInfo = Cal_MemAlloc(Cal_Bdd_t, i+1);
  for (j=0; j < i; j++){
    associationInfo[j] =
        CalBddGetInternalBdd(bddManager,associationInfoUserBdds[j]);
  }
  associationInfo[j] = bddManager->bddNull;

  
  varAssociation = Cal_MemAlloc(Cal_Bdd_t, bddManager->maxNumVars+1);
  for(i = 0; i <= bddManager->maxNumVars; i++){
    varAssociation[i] = bddManager->bddNull;
  }

  
  if(pairs){
    for(i = 0; i < numAssociations; i++){
      f = associationInfo[(i<<1)];
      varAssociation[CalBddGetBddId(f)] = associationInfo[(i<<1)+1];
    }
  }
  else{
    for(i = 0; i < numAssociations; i++){
      f=associationInfo[i];
      varAssociation[CalBddGetBddId(f)] = CalBddOne(bddManager);
    }
  }
  /* Check for existence. */
  for(p = bddManager->associationList; p; p = p->next){
    if(AssociationIsEqual(bddManager, p->varAssociation, varAssociation)){
	Cal_MemFree(varAssociation);
    Cal_MemFree(associationInfo);
	p->refCount++;
	return (p->id);
    }
  }
  /* Find the first unused id. */
  for(q = &bddManager->associationList, p = *q, i = 0;
      p && p->id == i; q = &p->next, p = *q, ++i){
  }
  /*p = Cal_MemAlloc(CalAssociation_t, 1);*/
  /*p = CAL_BDD_NEW_REC(bddManager, CalAssociation_t);*/
  p = Cal_MemAlloc(CalAssociation_t, 1);
  p->id = i;
  p->next = *q;
  *q = p;
  p->varAssociation = varAssociation;
  last = -1;
  if(pairs){
    for(i = 0; i < numAssociations; i++){
      f = associationInfo[(i<<1)];
      j = CalBddGetBddIndex(bddManager, f);
      if((long)j > last){
	  last = j;
      }
    }
  }
  else{
    for(i = 0; i < numAssociations; i++){
      f = associationInfo[i];
      j = CalBddGetBddIndex(bddManager, f);
      if((long)j > last){
	  last = j;
      }
    }
  }
  p->lastBddIndex = last;
  p->refCount = 1;
  /* Protect BDDs in the association. */
  if(pairs){
    for(i = 0; i < numAssociations; i++){
      f = associationInfo[(i<<1)+1];
      CalBddIcrRefCount(f);
    }
  }
  Cal_MemFree(associationInfo);
  return p->id;
}


/**Function********************************************************************

  Synopsis    [Deletes the variable association given by id]

  Description [Decrements the reference count of the variable association with
  identifier id, and frees it if the reference count becomes zero.]

  SideEffects [None]

  SeeAlso     [Cal_AssociationInit]

******************************************************************************/
void
Cal_AssociationQuit(Cal_BddManager bddManager, int  associationId)
{
  Cal_BddId_t i;
  Cal_Bdd_t f;
  CalAssociation_t *p, **q;

  if(bddManager->currentAssociation->id == associationId){
    bddManager->currentAssociation = bddManager->tempAssociation;
  }
  for(q = &bddManager->associationList, p = *q; p; q = &p->next, p = *q){
    if(p->id == associationId){
      p->refCount--;
      if(!p->refCount){
        /* Unprotect the BDDs in the association. */
        for(i = 1; i <= bddManager->numVars; i++){
          f = p->varAssociation[i];
          if(!CalBddIsBddNull(bddManager, f)){
            CalBddDcrRefCount(f);
          }
        }
        *q = p->next;
        Cal_MemFree(p->varAssociation);
        /*CAL_BDD_FREE_REC(bddManager, p, CalAssociation_t);*/
        Cal_MemFree(p);
        CalCacheTableTwoFlushAssociationId(bddManager, associationId);
      }
      return;
    }
  }
  CalBddWarningMessage("Cal_AssociationQuit: no association with specified ID");
}

/**Function********************************************************************

  Synopsis    [Sets the current variable association to the one given by id and
  returns the ID of the old association.]

  Description [Sets the current variable association to the one given by id and
  returns the ID of the old association.  An id of -1 indicates the temporary
  association]

  SideEffects [None]

******************************************************************************/
int
Cal_AssociationSetCurrent(Cal_BddManager bddManager, int  associationId)
{
  int oldAssociationId;
  CalAssociation_t *p;

  oldAssociationId = bddManager->currentAssociation->id;
  if(associationId != -1){
    for(p = bddManager->associationList; p; p = p->next){
      if(p->id == associationId){
        bddManager->currentAssociation = p;
        return (oldAssociationId);
      }
    }
    CalBddWarningMessage(
        "Cal_AssociationSetCurrent: no variable association with specified ID.\n May have been discarded during dynamic reordering.");
  }
  bddManager->currentAssociation = bddManager->tempAssociation;
  return oldAssociationId;
}


/**Function********************************************************************

  Synopsis    [Adds to the temporary variable association.]

  Description [Pairs is 0 if the information represents only a list of
  variables rather than a full association.]

  SideEffects [None]

******************************************************************************/
void
Cal_TempAssociationAugment(Cal_BddManager bddManager,
                           Cal_Bdd *associationInfoUserBdds,
                           int  pairs)
{
  int i, j, numAssociations;
  Cal_Bdd_t f;
  long last;
  Cal_Bdd_t *associationInfo;
  
  if (CheckAssoc(bddManager, associationInfoUserBdds, pairs) == 0) {
    return;
  }

  /*while (associationInfoUserBdds[i++]);*/
  for (i=0; associationInfoUserBdds[i]; i++);
  if (pairs) numAssociations = i/2;
  else numAssociations = i;
  associationInfo = Cal_MemAlloc(Cal_Bdd_t, i+1);
  for (j=0; j < i; j++){
    associationInfo[j] =
        CalBddGetInternalBdd(bddManager,associationInfoUserBdds[j]);
  }
  associationInfo[j] = bddManager->bddNull;
  
  last = bddManager->tempAssociation->lastBddIndex;
  if(pairs){
    for(i = 0; i < numAssociations; i++){
      f = associationInfo[(i<<1)];
      j = CalBddGetBddId(f);
      if(bddManager->idToIndex[j] > last){
        last = bddManager->idToIndex[j];
      }
      f = bddManager->tempAssociation->varAssociation[j];
      if(!CalBddIsBddNull(bddManager, f)){
        CalBddDcrRefCount(f);
      }
      f = associationInfo[(i<<1)+1];
      bddManager->tempAssociation->varAssociation[j] = f;
      /* Protect BDDs in the association. */
      CalBddIcrRefCount(f);
    }
  }
  else{
    for(i = 0; i < numAssociations; i++){
      f = associationInfo[i];
      j = CalBddGetBddId(f);
      if(bddManager->idToIndex[j] > last){
        last = bddManager->idToIndex[j];
      }
      f = bddManager->tempAssociation->varAssociation[j];
      if(!CalBddIsBddNull(bddManager, f)){
        CalBddDcrRefCount(f);
      }
      bddManager->tempAssociation->varAssociation[j] = CalBddOne(bddManager);
    } 
  }
  bddManager->tempAssociation->lastBddIndex = last;
  Cal_MemFree(associationInfo);
}



/**Function********************************************************************

  Synopsis    [Sets the temporary variable association.]
  
  Description [Pairs is 0 if the information represents only a list of
  variables rather than a full association.]

  SideEffects [None]

******************************************************************************/
void
Cal_TempAssociationInit(Cal_BddManager bddManager,
                        Cal_Bdd *associationInfoUserBdds,
                        int  pairs)
{
  long i;
  Cal_Bdd_t f;

  /* Clean up old temporary association. */
  for(i = 1; i <= bddManager->numVars; i++){
    f = bddManager->tempAssociation->varAssociation[i];
    if(!CalBddIsBddNull(bddManager, f)){
      CalBddDcrRefCount(f);
      bddManager->tempAssociation->varAssociation[i] = bddManager->bddNull;
    }
  }
  bddManager->tempAssociation->lastBddIndex = -1;
  Cal_TempAssociationAugment(bddManager, associationInfoUserBdds, pairs);
}

/**Function********************************************************************

  Synopsis    [Cleans up temporary association]

  Description [Cleans up temporary associationoptional]

  SideEffects [None]

******************************************************************************/
void
Cal_TempAssociationQuit(Cal_BddManager bddManager)
{
  int i;
  Cal_Bdd_t f;

  /* Clean up old temporary association. */
  for(i = 1; i <= bddManager->numVars; i++){
    f = bddManager->tempAssociation->varAssociation[i];
    if(!CalBddIsBddNull(bddManager, f)){
      CalBddDcrRefCount(f);
      bddManager->tempAssociation->varAssociation[i] = bddManager->bddNull;
    }
  }
  bddManager->tempAssociation->lastBddIndex = -1;
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Frees the variable associations]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalAssociationListFree(Cal_BddManager_t *  bddManager)
{
  CalAssociation_t *assoc, *nextAssoc;
  
  for(assoc = bddManager->associationList;
      assoc != Cal_Nil(CalAssociation_t); assoc = nextAssoc){
    nextAssoc = assoc->next;
    Cal_MemFree(assoc->varAssociation);
    /*CAL_BDD_FREE_REC(bddManager, assoc, CalAssociation_t);*/
    Cal_MemFree(assoc);
  }
}

/**Function********************************************************************

  Synopsis    [Need to be called after repacking.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalVarAssociationRepackUpdate(Cal_BddManager_t *  bddManager,
                              Cal_BddId_t id)
{
  CalAssociation_t *assoc, *nextAssoc;
  int i;
  
  for(assoc = bddManager->associationList;
      assoc != Cal_Nil(CalAssociation_t); assoc = nextAssoc){
    nextAssoc = assoc->next;
    for (i=1; i <= bddManager->numVars; i++){
      if (CalBddGetBddId(assoc->varAssociation[i]) == id){
        CalBddForward(assoc->varAssociation[i]);
      }
    }
  }
  /* fix temporary association */
  assoc = bddManager->tempAssociation;
  for (i=1; i <= bddManager->numVars; i++){
    if (CalBddGetBddId(assoc->varAssociation[i]) == id){
      CalBddForward(assoc->varAssociation[i]);
    }
  }
}

/**Function********************************************************************

  Synopsis    [Checks the validity of association.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
void
CalCheckAssociationValidity(Cal_BddManager_t *  bddManager)
{
  CalAssociation_t *assoc, *nextAssoc;
  int i;
  
  for(assoc = bddManager->associationList;
      assoc != Cal_Nil(CalAssociation_t); assoc = nextAssoc){
    nextAssoc = assoc->next;
    for (i=1; i <= bddManager->numVars; i++){
      Cal_Assert(CalBddIsForwarded(assoc->varAssociation[i]) == 0);
    }
  }
  /* temporary association */
  assoc = bddManager->tempAssociation;
  for (i=1; i <= bddManager->numVars; i++){
    Cal_Assert(CalBddIsForwarded(assoc->varAssociation[i]) == 0);
  }
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
void
CalReorderAssociationFix(Cal_BddManager_t *bddManager)
{
  CalAssociation_t *assoc, *nextAssoc;
  int i;
  
  for(assoc = bddManager->associationList;
      assoc != Cal_Nil(CalAssociation_t); assoc = nextAssoc){
    nextAssoc = assoc->next;
    for (i=1; i <= bddManager->numVars; i++){
        if (CalBddGetBddId(assoc->varAssociation[i]) != CAL_BDD_NULL_ID){
            CalBddIsForwardedTo(assoc->varAssociation[i]);
        }
    }
  }
  /* fix temporary association */
  assoc = bddManager->tempAssociation;
  for (i=1; i <= bddManager->numVars; i++){
      if (CalBddGetBddId(assoc->varAssociation[i]) != CAL_BDD_NULL_ID){
          CalBddIsForwardedTo(assoc->varAssociation[i]);
      }
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Checks for equality of two associations]

  Description [Checks for equality of two associations]

  SideEffects [None]

******************************************************************************/
static int
AssociationIsEqual(Cal_BddManager_t * bddManager,
                   Cal_Bdd_t * p,
                   Cal_Bdd_t * q)
{
  int i;
  for(i = 1; i <= bddManager->maxNumVars; i++){
    if(CalBddIsEqual(p[i], q[i]) == 0){
      return (0);
    }
  }
  return (1);
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int
CheckAssoc(Cal_BddManager_t *bddManager, Cal_Bdd *assocInfo, int pairs)
{
  CalBddArrayPreProcessing(bddManager, assocInfo);
  if (pairs){
    while (assocInfo[0] && assocInfo[1]){
      if (CalBddTypeAux(bddManager,
                        CalBddGetInternalBdd(bddManager, assocInfo[0])) !=
          CAL_BDD_TYPE_POSVAR){  
	    CalBddWarningMessage("CheckAssoc: first element in pair is not a positive variable"); 
	    return (0);
	  }
      assocInfo+=2;
    }
  }
  return (1);
}

