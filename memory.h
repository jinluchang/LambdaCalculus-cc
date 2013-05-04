#ifndef MEMORY_H
#define MEMORY_H

#include "expr.h"
#include "eval.h"

void initializeHeap();
void initializeRoot();
void garbageCollection();

class GarbageCollectionRoot {
public :
    ExprC * retEvalulation;
    int spEvaluation;
    int spPadVariable;
    EvaluateFrame * stackEvaluation;
    PadVariableFrame * stackPadVariable;
};

#endif
