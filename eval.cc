/* vim: set expandtab : */

#include <iostream>
#include <sstream>
#include <string>

#include "global.h"
#include "expr.h"
#include "eval.h"
#include "queens.h"
#include "memory.h"

using namespace std;

ExprB * analysis(Expr * expr, EnvB * env) {
    ExprB * ans;
    Tag tag = expr->tag;
    if (TVar == tag) {
        ans = lookupEnvB(expr->var, env);
    } else if (TLam == tag) {
        ans = newLamB(analysis(expr->lam.body, addToEnvB(expr->lam.parm, env)));
    } else if (TApp == tag) {
        ExprB * fun = analysis(expr->app.fun, env);
        ExprB * arg = analysis(expr->app.arg, env);
        ans = newAppB(fun, arg);
    } else {
        cerr << "analysis : tag = " << tag << endl;
        ans = NULL;
    }
    return ans;
}

ExprC * eval(ExprB * expr, EnvC * env) {
    ExprC * ans;
    TagB tag = expr->tag;
    if (TVarB == tag) {
        ans = newVarC(expr->var.name);
    } else if (TBoundB == tag) {
        ans = lookupEnvC(expr->bound, env);
        if (TThunk == ans->tag) {
            ans->tag = TBlackHole;
            *ans = *eval(ans->thunk.expr, ans->thunk.env);
        }
    } else if (TLamB == tag) {
        ans = newClosure(expr->lam, env);
    } else if (TAppB == tag) {
        ans = apply(expr->app.fun, expr->app.arg, env);
    } else {
        cerr << "eval : tag = " << tag << endl;
        ans = NULL;
    }
    return ans;
}

ExprC * apply(ExprB * fun, ExprB * arg, EnvC * env) {
    ExprC * ans;
    ExprC * funC = eval(fun, env);
    if (TClosure == funC->tag) {
        ExprC * argC = newThunk(arg, env);
        ans = eval(funC->closure.def.body, addToEnvC(argC, funC->closure.env));
    } else {
        ExprC * argC = eval(arg, env);
        ans = newAppC(funC, argC);
    }
    return ans;
}

string showEntry(EvaluateEntryLabel entry) {
    if (LEval == entry) {
        return "LEval";
    } else if (LUpdateE == entry) {
        return "LUpdateE";
    } else if (LApplyE == entry) {
        return "LApplyE";
    } else if (LMkAppE == entry) {
        return "LMkAppE";
    } else {
        cerr << "showEntry : " << entry << endl;
        return "";
    }
}

ExprC * evaluate(ExprB * expr, EnvC * env) {
    EvaluateFrame * stack = Root.stackEvaluation;
    int sp = 0;
    ExprC * ret = NULL;
    stack[0].entry = LEval;
    stack[0].expr = expr;
    stack[0].env = env;
    stack[0].tmp = NULL;
    while (0 <= sp) {
        if (Hp > MAX_HEAP_SIZE / 5 || EnvHp > MAX_ENV_HEAP_SIZE / 2) {
            Root.spEvaluation = sp;
            Root.retEvalulation = ret;
            garbageCollection();
            ret = Root.retEvalulation;
        }
        EvaluateEntryLabel entry = stack[sp].entry;
        ExprB * expr = stack[sp].expr;
        EnvC * env = stack[sp].env;
        if (LEval == entry) {
            TagB tag = stack[sp].expr->tag;
            if (TVarB == tag) {
                ret = newVarC(expr->var.name);
                sp--;
            } else if (TBoundB == tag) {
                ret = lookupEnvC(expr->bound, env);
                if (TThunk == ret->tag) {
                    ret->tag=TBlackHole;
                    stack[sp].tmp = ret;
                    stack[sp].entry = LUpdateE;
                    stack[sp+1].entry = LEval;
                    stack[sp+1].expr = ret->thunk.expr;
                    stack[sp+1].env = ret->thunk.env;
                    stack[sp+1].tmp = NULL;
                    ret = NULL;
                    sp++;
                } else {
                    sp--;
                }
            } else if (TLamB == tag) {
                ret = newClosure(expr->lam, env);
                sp--;
            } else if (TAppB == tag) {
                stack[sp].entry = LApplyE;
                stack[sp+1].entry = LEval;
                stack[sp+1].expr = expr->app.fun;
                stack[sp+1].env = env;
                stack[sp+1].tmp = NULL;
                ret = NULL;
                sp++;
            } else {
                cerr << "evaluate : tag = " << tag << endl;
            }
        } else if (LUpdateE == entry) {
            *(stack[sp].tmp) = *ret;
            ret = stack[sp].tmp;
            sp--;
        } else if (LApplyE == entry) {
            ExprC * funC = ret;
            if (TClosure == funC->tag) {
                ExprC * argC = newThunk(expr->app.arg, env);
                stack[sp].entry = LEval;
                stack[sp].expr = funC->closure.def.body;
                stack[sp].env = addToEnvC(argC, funC->closure.env);
                stack[sp].tmp = NULL;
                ret = NULL;
            } else {
                stack[sp].tmp = funC;
                stack[sp].entry = LMkAppE;
                stack[sp+1].entry = LEval;
                stack[sp+1].expr = expr->app.arg;
                stack[sp+1].env = env;
                stack[sp+1].tmp = NULL;
                ret = NULL;
                sp++;
            }
        } else if (LMkAppE == entry) {
            ExprC * argC = ret;
            ExprC * funC = stack[sp].tmp;
            ret = newAppC(funC, argC);
            sp--;
        } else {
            cerr << "evaluate : entry = " << entry << endl;
        }
    }
    Root.spEvaluation = -1;
    Root.retEvalulation = NULL;
    return ret;
}

Expr * padVar(ExprC * expr) {
    Expr * ans;
    TagC tag = expr->tag;
    if (TVarC == tag) {
        ans = newVar(expr->var.name);
    } else if (TAppC == tag) {
        ans = newApp(padVar(expr->app.fun), padVar(expr->app.arg));
    } else if (TClosure == tag) {
        ExprC * fvar = freshVarC();
        EnvC * env = addToEnvC(fvar, expr->closure.env);
        ans = newLam(fvar->var.name, padVar(eval(expr->closure.def.body, env)));
    } else {
        cerr << "padVar : tag = " << tag << endl;
        ans = NULL;
    }
    return ans;
}

Expr * padVariable(ExprC * expr) {
    PadVariableFrame * stack = Root.stackPadVariable;
    int sp = 0;
    Expr * ret = NULL;
    stack[0].entry = LPad;
    stack[0].expr = expr;
    stack[0].fun = NULL;
    stack[0].tmp = NULL;
    while (0 <= sp) {
        PadVariableEntryLabel entry = stack[sp].entry;
        ExprC * expr = stack[sp].expr;
        if (LPad == entry) {
            TagC tag = expr->tag;
            if (TVarC == tag) {
                ret = newVar(expr->var.name);
                sp--;
            } else if (TAppC == tag) {
                stack[sp].entry = LApplyP;
                stack[sp+1].entry = LPad;
                stack[sp+1].expr = expr->app.fun;
                stack[sp+1].fun = NULL;
                stack[sp+1].tmp = NULL;
                sp++;
            } else if (TClosure == tag) {
                ExprC * fvar = freshVarC();
                EnvC * env = addToEnvC(fvar, expr->closure.env);
                stack[sp].tmp = fvar;
                stack[sp].entry = LMkLamP;
                stack[sp+1].entry = LPad;
                Root.spPadVariable = sp;
                stack[sp+1].expr = evaluate(expr->closure.def.body, env);
                stack[sp+1].fun = NULL;
                stack[sp+1].tmp = NULL;
                sp++;
            } else {
                cerr << "padVar : tag = " << tag << endl;
            }
        } else if (LMkLamP == entry) {
            Expr * body = ret;
            ExprC * fvar = stack[sp].tmp;
            ret = newLam(fvar->var.name, body);
            sp--;
        } else if (LApplyP == entry) {
            stack[sp].fun = ret;
            stack[sp].entry = LMkAppP;
            stack[sp+1].entry = LPad;
            stack[sp+1].expr = expr->app.arg;
            stack[sp+1].fun = NULL;
            stack[sp+1].tmp = NULL;
            sp++;
        } else if (LMkAppP == entry) {
            Expr * arg = ret;
            Expr * fun = stack[sp].fun;
            ret = newApp(fun, arg);
            sp--;
        } else {
            cerr << "padVariable : entry = " << entry << endl;
        }
    }
    return ret;
}
