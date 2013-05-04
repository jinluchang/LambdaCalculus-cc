/* vim: set expandtab : */

#include <iostream>
#include <sstream>
#include <string>

#include "expr.h"
#include "queens.h"
#include "eval.h"
#include "global.h"
#include "memory.h"

using namespace std;

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

ExprC * freshVarC() {
    static int k = 0;
    ostringstream out;
    out << "$var" << k;
    string * name = new string(out.str());
    k++;
    return newVarC(name);
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
