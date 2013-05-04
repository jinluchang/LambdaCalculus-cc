#include "expr.h"
#include "memory.h"

ExprC * Heap = NULL;
int Hp = 0;

EnvC * EnvHeap = NULL;
int EnvHp = 0;

GarbageCollectionRoot Root;
