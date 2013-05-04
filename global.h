#ifndef GLOBAL_H
#define GLOBAL_H

#include "expr.h"
#include "memory.h"

const int MAX_HEAP_SIZE = 3000000;
extern ExprC * Heap;
extern int Hp;

const int MAX_ENV_HEAP_SIZE = 3000000;
extern EnvC * EnvHeap;
extern int EnvHp;

extern GarbageCollectionRoot Root;

#endif
