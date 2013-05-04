/* vim: set expandtab : */

#include <iostream>
#include <sstream>
#include <string>

#include "expr.h"
#include "queens.h"

using namespace std;

const int MAX_HEAP_SIZE = 3000000;
ExprC * Heap = NULL;
int Hp = 0;

const int MAX_ENV_HEAP_SIZE = 3000000;
EnvC * EnvHeap = NULL;
int EnvHp = 0;

Expr * newVar(string * name) {
    Expr * var = new Expr;
    var->tag = TVar;
    var->var.name = name;
    return var;
}

Expr * newVar(const char * name_cstr) {
    string * name = new string(name_cstr);
    return newVar(name);
}

Expr * newLam(string * parm, Expr * body) {
    Expr * lam = new Expr;
    lam->tag = TLam;
    lam->lam.parm = parm;
    lam->lam.body = body;
    return lam;
}

Expr * newLam(const char * parm_cstr, Expr * body) {
    string * parm = new string(parm_cstr);
    return newLam(parm, body);
}

Expr * newApp(Expr * fun, Expr * arg) {
    Expr * app = new Expr;
    app->tag = TApp;
    app->app.fun = fun;
    app->app.arg = arg;
    return app;
}

ExprB * newVarB(string * name) {
    ExprB * var = new ExprB;
    var->tag = TVarB;
    var->var.name = name;
    return var;
}

ExprB * newBoundB(int index) {
    ExprB * bound = new ExprB;
    bound->tag = TBoundB;
    bound->bound.index = index;
    return bound;
}

ExprB * newLamB(ExprB * body) {
    ExprB * lam = new ExprB;
    lam->tag = TLamB;
    lam->lam.body = body;
    return lam;
}

ExprB * newAppB(ExprB * fun, ExprB * arg) {
    ExprB * app = new ExprB;
    app->tag = TAppB;
    app->app.fun = fun;
    app->app.arg = arg;
    return app;
}

ExprC * newVarC(string * name) {
    ExprC * var = Heap + Hp;
    Hp++;
    var->tag = TVarC;
    var->var.name = name;
    return var;
}

ExprC * newAppC(ExprC * fun, ExprC * arg) {
    ExprC * app = Heap + Hp;
    Hp++;
    app->tag = TAppC;
    app->app.fun = fun;
    app->app.arg = arg;
    return app;
}

ExprC * newClosure(LamB def, EnvC * env) {
    ExprC * closure = Heap + Hp;
    Hp++;
    closure->tag = TClosure;
    closure->closure.def = def;
    closure->closure.env = env;
    return closure;
}

ExprC * newThunk(ExprB * expr, EnvC * env) {
    ExprC * thunk = Heap + Hp;
    Hp++;
    thunk->tag = TThunk;
    thunk->thunk.expr = expr;
    thunk->thunk.env = env;
    return thunk;
}

EnvB * emptyEnvB() {
    return NULL;
}

EnvB * addToEnvB(string * name, EnvB * oldEnv) {
    EnvB * env = new EnvB;
    env->name = name;
    env->next = oldEnv;
    return env;
}

ExprB * lookupEnvB(Var var, EnvB * env) {
    int index = 0;
    while (NULL != env) {
        if (0 == var.name->compare(*(env->name))) {
            return newBoundB(index);
        } else {
            index++;
            env = env->next;
        }
    }
    return newVarB(var.name);
}

EnvC * emptyEnvC() {
    return NULL;
}

EnvC * addToEnvC(ExprC * value, EnvC * oldEnv) {
    EnvC * env = EnvHeap + EnvHp;
    EnvHp++;
    env->value = value;
    env->next = oldEnv;
    return env;
}

ExprC * lookupEnvC(BoundB bound, EnvC * env) {
    while (0 != bound.index) {
        bound.index--;
        env = env->next;
    }
    return env->value;
}

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

string showInt(int n) {
    stringstream ss;
    ss << n;
    return ss.str();
}

string showExprB(ExprB * expr) {
    TagB tag = expr->tag;
    string ans;
    if (TVarB == tag) {
        ans += "TVarB ";
        ans += *expr->var.name;
    } else if (TBoundB == tag) {
        ans += "TBoundB ";
        ans += showInt(expr->bound.index);
    } else if (TLamB == tag) {
        ans += "TLamB ";
        ans += showExprB(expr->lam.body);
    } else if (TAppB == tag) {
        ans += "TAppB (";
        ans += showExprB(expr->app.fun);
        ans += ") (";
        ans += showExprB(expr->app.arg);
        ans += ")";
    } else {
        cerr << "showExprB : tag : " << tag << endl;
    }
    return ans;
}

string showExprC(ExprC * expr) {
    string ans;
    if (NULL == expr) {
        ans += "NULL";
    } else {
        TagC tag = expr->tag;
        if (TVarC == tag) {
            ans += "TVarC ";
            ans += *expr->var.name;
        } else if (TAppC == tag) {
            ans += "TAppC (";
            ans += showExprC(expr->app.fun);
            ans += ") (";
            ans += showExprC(expr->app.arg);
            ans += ")";
        } else if (TClosure == tag) {
            ans += "TClosure def : ";
            ans += showExprB(expr->closure.def.body);
        } else if (TThunk == tag) {
            ans += "TThunk expr : ";
            ans += showExprB(expr->thunk.expr);
        } else if (TBlackHole == tag) {
            ans += "TBlackHole";
        } else if (TIndC == tag) {
            ans += "TIndC : ";
            ans += showExprC(expr->ind);
        } else {
            cerr << "showExprC : tag : " << tag << endl;
        }
    }
    return ans;
}

void printState(ExprC ** pans, int sp, EvaluateFrame * stack) {
    cerr << "Hp = " << Hp << endl;
    cerr << "EnvHp = " << EnvHp << endl;
    cerr << "printState : --------------------------------------------- " << endl;
    cerr << "printState : ans : " << showExprC(*pans) << endl;
    cerr << "printState : sp : " << sp << endl;
    for (int i = sp; i >= 0 ; i--) {
        cerr << "printState : stack[" << i << "].entry  : " << showEntry(stack[i].entry) << endl;
        cerr << "printState : stack[" << i << "].expr   : " << showExprB(stack[i].expr) << endl;
        EnvC * env = stack[i].env;
        int envLevel = 0;
        while (NULL != env) {
            cerr << "printState : stack[" << i<< "].env[" << envLevel << "] : " << showExprC(env->value) << endl;
            env = env->next;
        }
        cerr << "printState : stack[" << i << "].tmp    : " << showExprC(stack[i].tmp) << endl;
    }
    cerr << "printState : --------------------------------------------- " << endl;
}

class garbageCollectionRoot {
public :
    ExprC * retEvalulation;
    int spEvaluation;
    int spPadVariable;
    EvaluateFrame * stackEvaluation;
    PadVariableFrame * stackPadVariable;
} Root;

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
//            cout << "Hp = " << Hp << " (MAX " << MAX_HEAP_SIZE << ")" << endl;
//            cout << "Memory = " << Hp * sizeof(ExprC) / 1024 / 1024 << " MB" << endl;
//            cout << "EnvHp = " << EnvHp << " (MAX " << MAX_ENV_HEAP_SIZE << ")" << endl;
//            cout << "Memory = " << EnvHp * sizeof(EnvC) / 1024 / 1024 << " MB" << endl;
            Root.spEvaluation = sp;
            Root.retEvalulation = ret;
            garbageCollection();
            ret = Root.retEvalulation;
//            cout << "Hp = " << Hp << " (MAX " << MAX_HEAP_SIZE << ")" << endl;
//            cout << "Memory = " << Hp * sizeof(ExprC) / 1024 / 1024 << " MB" << endl;
//            cout << "EnvHp = " << EnvHp << " (MAX " << MAX_ENV_HEAP_SIZE << ")" << endl;
//            cout << "Memory = " << EnvHp * sizeof(EnvC) / 1024 / 1024 << " MB" << endl;
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

ExprC * freshVar() {
    static int k = 0;
    ostringstream out;
    out << "$var" << k;
    string * name = new string(out.str());
    k++;
    return newVarC(name);
}

Expr * padVar(ExprC * expr) {
    Expr * ans;
    TagC tag = expr->tag;
    if (TVarC == tag) {
        ans = newVar(expr->var.name);
    } else if (TAppC == tag) {
        ans = newApp(padVar(expr->app.fun), padVar(expr->app.arg));
    } else if (TClosure == tag) {
        ExprC * fvar = freshVar();
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
                ExprC * fvar = freshVar();
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

string showParenthese(string s) {
    string ans;
    ans += "(";
    ans += s;
    ans += ")";
    return ans;
}

string showExprPrec(int prec, Expr * expr) {
    string ans;
    Tag tag = expr->tag;
    if (TVar == tag) {
        ans = *(expr->var.name);
    } else if (TLam == tag) {
        ans += "\\";
        ans += *(expr->lam.parm);
        ans += " -> ";
        ans += showExprPrec(0, expr->lam.body);
        if (0 < prec) {
            ans = showParenthese(ans);
        }
    } else if (TApp == tag) {
        ans += showExprPrec(1, expr->app.fun);
        ans += " ";
        ans += showExprPrec(2, expr->app.arg);
        if (1 < prec) {
            ans = showParenthese(ans);
        }
    } else {
        cerr << "showAExpr : tag = " << tag << endl;
    }
    return ans;
}

string showExpr(Expr * expr) {
    return showExprPrec(0, expr);
}

int main() {
    initializeHeap();
    initializeRoot();
    Expr * varx = newVar("x");
    Expr * varf = newVar("f");
    Expr * id = newLam("x", varx);
    Expr * test = newApp(newLam("x", varx), varf);
    Expr * expr = queens();
    ExprB * idB = analysis(id, emptyEnvB());
    ExprB * testB = analysis(test, emptyEnvB());
    ExprB * exprB = analysis(expr, emptyEnvB());
    cout << showExpr(id) << endl;
    cout << showExpr(test) << endl;
    cout << showExpr(expr) << endl;
    cout << showExpr(padVariable(evaluate(idB, emptyEnvC()))) << endl;
    cout << showExpr(padVariable(evaluate(testB, emptyEnvC()))) << endl;
    cout << showExpr(padVariable(evaluate(exprB, emptyEnvC()))) << endl;
    cout << "Hp = " << Hp << " (MAX " << MAX_HEAP_SIZE << ")" << endl;
    cout << "Memory = " << Hp * sizeof(ExprC) / 1024 / 1024 << " MB" << endl;
    cout << "EnvHp = " << EnvHp << " (MAX " << MAX_ENV_HEAP_SIZE << ")" << endl;
    cout << "Memory = " << EnvHp * sizeof(EnvC) / 1024 / 1024 << " MB" << endl;
    return 0;
}
