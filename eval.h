/* vim: set expandtab : */

#ifndef EVAL_H
#define EVAL_H

#include "expr.h"

ExprB * analysis(Expr * expr, EnvB * env);

ExprC * eval(ExprB * expr, EnvC * env);
ExprC * apply(ExprB * fun, ExprB * arg, EnvC * env);

Expr * padVar(ExprC * expr);

enum EvaluateEntryLabel {
    LEval,
    LUpdateE,
    LApplyE,
    LMkAppE,
};

class EvaluateFrame {
public :
    EvaluateEntryLabel entry;
    ExprB * expr;
    EnvC * env;
    ExprC * tmp;
};

ExprC * evaluate(ExprB * expr, EnvC * env);

enum PadVariableEntryLabel {
    LPad,
    LMkLamP,
    LApplyP,
    LMkAppP,
};

class PadVariableFrame {
public :
    PadVariableEntryLabel entry;
    ExprC * expr;
    Expr * fun;
    ExprC * tmp;
};

Expr * padVariable(ExprC * expr);

#endif
