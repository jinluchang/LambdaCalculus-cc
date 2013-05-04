/* vim: set expandtab : */

#include <iostream>

#include "expr.h"
#include "global.h"

using namespace std;

EnvC * moveEnvC(EnvC * env) {
    if (NULL == env) {
        return NULL;
    } else if (NULL == env->value) {
        return env->next;
    } else {
//        cerr << "moveEnvC" << endl;
        EnvHeap[EnvHp] = *env;
        env->value = NULL;
        env->next = EnvHeap + EnvHp;
        EnvHp++;
        return env->next;
    }
}

ExprC * moveExprC(ExprC * expr) {
    if (NULL == expr) {
        return NULL;
    } else if (TIndC == expr->tag) {
        return expr->ind;
    } else {
//        cerr << "moveExprC" << endl;
        Heap[Hp] = *expr;
        expr->tag = TIndC;
        expr->ind = Heap + Hp;
        Hp++;
        return expr->ind;
    }
}

void updateExprC(ExprC * expr) {
//    cerr << "updateExprC" << endl;
    TagC tag = expr->tag;
    if (TAppC == tag) {
        expr->app.fun = moveExprC(expr->app.fun);
        expr->app.arg = moveExprC(expr->app.arg);
    } else if (TClosure == tag) {
        expr->closure.env = moveEnvC(expr->closure.env);
    } else if (TThunk == tag) {
        expr->thunk.env = moveEnvC(expr->thunk.env);
    } else if (TBlackHole == tag) {
//        expr->thunk.env = moveEnvC(expr->thunk.env);
    } else if (TIndC == tag) {
        cerr << "updateExprC : tag :" << tag << endl;
    }
}

void updateEnvC(EnvC * env) {
//    cerr << "updateEnvC" << endl;
    env->value = moveExprC(env->value);
    if (NULL != env->next) {
        env->next = moveEnvC(env->next);
    }
}

void initializeHeap() {
    static ExprC * oldHeap = NULL;
    static EnvC * oldEnvHeap = NULL;
    if (NULL == oldHeap) {
        Heap = new ExprC[MAX_HEAP_SIZE];
        oldHeap = new ExprC[MAX_HEAP_SIZE];
    } else {
        ExprC * tmp;
        tmp = oldHeap;
        oldHeap = Heap;
        Heap = tmp;
    }
    if (NULL == oldEnvHeap) {
        EnvHeap = new EnvC[MAX_ENV_HEAP_SIZE];
        oldEnvHeap = new EnvC[MAX_ENV_HEAP_SIZE];
    } else {
        EnvC * tmp;
        tmp = oldEnvHeap;
        oldEnvHeap = EnvHeap;
        EnvHeap = tmp;
    }
    Hp = 0;
    EnvHp = 0;
}

void initializeRoot() {
    Root.retEvalulation = NULL;
    const int MAX_EVALUATION_STACK_SIZE = 10000;
    Root.stackEvaluation = new EvaluateFrame[MAX_EVALUATION_STACK_SIZE];
    Root.spEvaluation = -1;
    const int MAX_PADVARIABLE_STACK_SIZE = 10000;
    Root.stackPadVariable = new PadVariableFrame[MAX_PADVARIABLE_STACK_SIZE];
    Root.spPadVariable = -1;
}

void garbageCollection() {
//    printState(pans, sp, stack);
//    cerr << "garbageCollection start" << endl;
    initializeHeap();
    Root.retEvalulation = moveExprC(Root.retEvalulation);
    for (int i = Root.spEvaluation; i >= 0; i--) {
        Root.stackEvaluation[i].env = moveEnvC(Root.stackEvaluation[i].env);
        Root.stackEvaluation[i].tmp = moveExprC(Root.stackEvaluation[i].tmp);
    }
    for (int i = Root.spPadVariable; i >= 0; i--) {
        Root.stackPadVariable[i].expr = moveExprC(Root.stackPadVariable[i].expr);
        Root.stackPadVariable[i].tmp = moveExprC(Root.stackPadVariable[i].tmp);
    }
    int hp = 0;
    int envHp = 0;
    while (hp < Hp || envHp < EnvHp) {
        while (hp < Hp) {
            updateExprC(Heap+hp);
            hp++;
        }
        while (envHp < EnvHp) {
            updateEnvC(EnvHeap+envHp);
            envHp++;
        }
    }
//    cerr << "garbageCollection end" << endl;
//    printState(pans, sp, stack);
}
