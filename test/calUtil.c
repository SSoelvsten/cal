#include <calInt.h>

int main(int argc, char **argv)
{
  CalBddNode_t *bddNode, *thenBddNode, *elseBddNode;

  bddNode = Cal_MemAlloc(CalBddNode_t, 1);
  thenBddNode = Cal_MemAlloc(CalBddNode_t, 1);
  elseBddNode = Cal_MemAlloc(CalBddNode_t, 1);

  CalBddNodePutThenBddId(bddNode, 1);
  CalBddNodePutThenBddNode(bddNode, thenBddNode);
  CalBddNodePutElseBddId(bddNode, 2);
  CalBddNodePutElseBddNode(bddNode, elseBddNode);

  printf("then( 1 %x) else( 2 %x)\n", thenBddNode, elseBddNode);
  CalBddNodePrint(bddNode);

  return 0;
}

