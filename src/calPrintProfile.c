/**CFile***********************************************************************

  FileName    [calPrintProfile.c]

  PackageName [cal]

  Synopsis    [Routines for printing various profiles for a BDD.]

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

  Revision    [$Id: calPrintProfile.c,v 1.1.1.3 1998/05/04 00:59:01 hsv Exp $]

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

static char profileWidth[] = "XXXXXXXXX";

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void CalBddPrintProfileAux(Cal_BddManager_t * bddManager, long * levelCounts, Cal_VarNamingFn_t varNamingProc, char * env, int lineLength, FILE * fp);
static void chars(char c, int n, FILE * fp);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Displays the node profile for f on fp. lineLength specifies 
               the maximum line length.  varNamingFn is as in
               Cal_BddPrintBdd.]

  Description [optional]

  SideEffects [None]

  SeeAlso     [optional]

******************************************************************************/
void
Cal_BddPrintProfile(Cal_BddManager  bddManager,
                    Cal_Bdd  fUserBdd,
                    Cal_VarNamingFn_t varNamingProc,
                    char * env,
                    int  lineLength,
                    FILE * fp)
{
  long *levelCounts;

  if (CalBddPreProcessing(bddManager, 1, fUserBdd) == 0){
	return;
  }
  levelCounts = Cal_MemAlloc(long, bddManager->numVars+1);
  Cal_BddProfile(bddManager, fUserBdd, levelCounts, 1);
  CalBddPrintProfileAux(bddManager, levelCounts, varNamingProc, env,
                        lineLength, fp); 
  Cal_MemFree(levelCounts);
}

/**Function********************************************************************

  Synopsis    [Cal_BddPrintProfileMultiple is like Cal_BddPrintProfile except
               it displays the profile for a set of BDDs]

  Description [optional]

  SideEffects [None]

  SeeAlso     [optional]

******************************************************************************/
void
Cal_BddPrintProfileMultiple(
  Cal_BddManager bddManager,
  Cal_Bdd *userBdds,
  Cal_VarNamingFn_t varNamingProc,
  char * env,
  int  lineLength,
  FILE * fp)
{
  long *levelCounts;

  if (CalBddArrayPreProcessing(bddManager, userBdds) == 0){
	return;
  }
  levelCounts = Cal_MemAlloc(long, bddManager->numVars+1);
  Cal_BddProfileMultiple(bddManager, userBdds, levelCounts, 1);
  CalBddPrintProfileAux(bddManager, levelCounts, varNamingProc, env, lineLength, fp);
  Cal_MemFree(levelCounts);
}



/**Function********************************************************************

  Synopsis    [Cal_BddPrintFunctionProfile is like Cal_BddPrintProfile except
               it displays a function profile for f]

  Description [optional]

  SideEffects [None]

  SeeAlso     [optional]

******************************************************************************/
void
Cal_BddPrintFunctionProfile(Cal_BddManager bddManager,
                            Cal_Bdd  f,
                            Cal_VarNamingFn_t varNamingProc,
                            char * env,
                            int  lineLength,
                            FILE * fp)
{
  long *levelCounts;
  if (CalBddPreProcessing(bddManager, 1, f)){
	return;
  }
  levelCounts = Cal_MemAlloc(long, bddManager->numVars+1);
  Cal_BddFunctionProfile(bddManager, f, levelCounts);
  CalBddPrintProfileAux(bddManager, levelCounts, varNamingProc, env,
                        lineLength, fp); 
  Cal_MemFree(levelCounts);
}


/**Function********************************************************************

  Synopsis    [Cal_BddPrintFunctionProfileMultiple is like
               Cal_BddPrintFunctionProfile except for multiple BDDs]

  Description [optional]

  SideEffects [None]

  SeeAlso     [optional]

******************************************************************************/
void
Cal_BddPrintFunctionProfileMultiple(Cal_BddManager bddManager,
                                    Cal_Bdd *userBdds,
                                    Cal_VarNamingFn_t varNamingProc,
                                    char * env,
                                    int  lineLength,
                                    FILE * fp)
{
  long *levelCounts;
  if (CalBddArrayPreProcessing(bddManager, userBdds) == 0){
	return;
  }
  levelCounts = Cal_MemAlloc(long, bddManager->numVars+1);
  Cal_BddFunctionProfileMultiple(bddManager, userBdds, levelCounts);
  CalBddPrintProfileAux(bddManager, levelCounts, varNamingProc, env, lineLength, fp);
  Cal_MemFree(levelCounts);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Prints a profile to the file given by fp.  The varNamingProc
               is as in Cal_BddPrintBdd. lineLength gives the line width to scale
               the profile to.]

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
CalBddPrintProfileAux(
  Cal_BddManager_t * bddManager,
  long * levelCounts,
  Cal_VarNamingFn_t varNamingProc,
  char * env,
  int  lineLength,
  FILE * fp)
{
  long i, n;
  int l;
  char *name;
  int maxPrefixLen;
  int maxProfileWidth;
  int histogramColumn;
  int histogramWidth;
  int profileScale;
  long total;

  n = bddManager->numVars;
  /* max_... initialized with values for leaf nodes */
  maxPrefixLen = 5;
  maxProfileWidth = levelCounts[n];
  total = levelCounts[n];
  for(i = 0; i < n; i++){
    if(levelCounts[i]){
      sprintf(profileWidth, "%ld", levelCounts[i]);
      l = strlen(CalBddVarName(bddManager, 
          bddManager->varBdds[bddManager->indexToId[i]],
          varNamingProc, env)) + strlen((char *)profileWidth);
      if(l > maxPrefixLen){
        maxPrefixLen = l;
      }
      if(levelCounts[i] > maxProfileWidth){
        maxProfileWidth = levelCounts[i];
      }
      total += levelCounts[i];
    }
  }
  histogramColumn = maxPrefixLen+3;
  histogramWidth = lineLength-histogramColumn-1;
  if(histogramWidth < 20)
    histogramWidth = 20;		/* Random minimum width */
  if(histogramWidth >= maxProfileWidth){
    profileScale = 1;
  }
  else{
    profileScale = (maxProfileWidth+histogramWidth-1)/histogramWidth;
  }
  for(i = 0; i < n; ++i){
    if(levelCounts[i]){
      name = CalBddVarName(bddManager,
          bddManager->varBdds[bddManager->indexToId[i]],
          varNamingProc, env);
      fputs(name, fp);
      fputc(':', fp);
      sprintf(profileWidth, "%ld", levelCounts[i]);
      chars(' ', (int)(maxPrefixLen-strlen(name)-strlen(profileWidth)+1), fp);
      fputs(profileWidth, fp);
      fputc(' ', fp);
      chars('#', levelCounts[i]/profileScale, fp);
      fputc('\n', fp);
    }
  }
  fputs("leaf:", fp);
  sprintf(profileWidth, "%ld", levelCounts[n]);
  chars(' ', (int)(maxPrefixLen-4-strlen(profileWidth)+1), fp);
  fputs(profileWidth, fp);
  fputc(' ', fp);
  chars('#', levelCounts[n]/profileScale, fp);
  fputc('\n', fp);
  fprintf(fp, "Total: %ld\n", total);
}

/**Function********************************************************************

  Synopsis    []

  Description [optional]

  SideEffects [required]

  SeeAlso     [optional]

******************************************************************************/
static void
chars(
  char  c,
  int  n,
  FILE * fp)
{
  int i;

  for(i = 0; i < n; ++i){
    fputc(c, fp);
  }
}












