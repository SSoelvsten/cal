#include <calInt.h>

int main(int argc, char **argv)
{
  Cal_Bdd_t f1, f2, f3, f4, f5, result;
  Cal_BddManager_t *bddManager = Cal_BddManagerInit();
  int i;
  CalCacheTable_t *cacheTable;

  for (i=0; i<5; i++){
    Cal_BddManagerCreateNewVarLast(bddManager);
  }

  CalCacheTablePrint(bddManager);

  f1 = bddManager->varBdds[1];
  f2 = bddManager->varBdds[2];
  f3 = bddManager->varBdds[3];
  f4 = bddManager->varBdds[4];
  f5 = bddManager->varBdds[5];

  CalCacheTableTwoInsert(bddManager, f1, f2, f3, CAL_OP_OR, 0);
  CalCacheTableTwoInsert(bddManager, f3, f2, f4, CAL_OP_AND,0);
  CalCacheTableTwoInsert(bddManager, f3, f4, f5, CAL_OP_REL_PROD,0);
  /*CacheTableTwoRehash(bddManager->cacheTableArray[2], 1);*/
  CalCacheTablePrint(bddManager);

  /* Look up */
  CalCacheTableTwoLookup(bddManager, f3, f2, CAL_OP_AND, &result);
  assert(CalBddIsEqual(result, f4));

  CalCacheTableTwoLookup(bddManager, f3, f2, CAL_OP_OR, &result);
  assert(CalBddIsEqual(result, bddManager->bddNull));

  CalCacheTableTwoLookup(bddManager, f3, f1, CAL_OP_OR, &result);
  assert(CalBddIsEqual(result, bddManager->bddNull));

  /* Another look up */
  CalCacheTableTwoLookup(bddManager, f4, f3, CAL_OP_REL_PROD, &result);
  assert(CalBddIsEqual(result, f5));

  /* It will bump off the entry (f2, f2, AND, f4)*/
  CalCacheTableTwoInsert(bddManager, f3, f2, f1, CAL_OP_AND,0);
  /* Do lookup and see if that's what happened */
  CalCacheTableTwoLookup(bddManager, f3, f2, CAL_OP_AND, &result);
  assert(CalBddIsEqual(result, f1));

  /*
   * Rehashing will visit (f2, f3, AND, f4) first and then (f2, f3,
   * AND, f1)
   * Hence the we should have (f2, f3, AND, f1) in the first slot
   */
  CacheTableTwoRehash(bddManager->cacheTable, 1);
  CalCacheTableTwoLookup(bddManager, f3, f2, CAL_OP_AND, &result);
  assert(CalBddIsEqual(result, f1));
  Cal_BddManagerQuit(bddManager);

  return 0;
}
