/* vim: set expandtab : */

#include <iostream>
#include <sstream>
#include <string>

#include "expr.h"
#include "queens.h"

using namespace std;

const int MAX_HEAP_SIZE = 30000000;
ExprC * Heap = NULL;
int Hp = 0;

const int MAX_ENV_HEAP_SIZE = 30000000;
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

string showExpr(Expr * expr) {
    string ans;
    Tag tag = expr->tag;
    if (TVar == tag) {
        ans = *(expr->var.name);
    } else if (TLam == tag) {
        ans += "(";
        ans += "\\";
        ans += *(expr->lam.parm);
        ans += " -> ";
        ans += showExpr(expr->lam.body);
        ans += ")";
    } else if (TApp == tag) {
        ans += showExpr(expr->app.fun);
        ans += " ";
        ans += showAExpr(expr->app.arg);
    } else {
        cerr << "showAExpr : tag = " << tag << endl;
    }
    return ans;
}

string showAExpr(Expr * expr) {
    string ans;
    Tag tag = expr->tag;
    if (TVar == tag) {
        ans = *(expr->var.name);
    } else if (TLam == tag) {
        ans += "(";
        ans += "\\";
        ans += *(expr->lam.parm);
        ans += " -> ";
        ans += showExpr(expr->lam.body);
        ans += ")";
    } else if (TApp == tag) {
        ans += "(";
        ans += showExpr(expr->app.fun);
        ans += " ";
        ans += showAExpr(expr->app.arg);
        ans += ")";
    } else {
        cerr << "showExpr : tag = " << tag << endl;
    }
    return ans;
}

int main() {
    Heap = new ExprC[MAX_HEAP_SIZE];
    EnvHeap = new EnvC[MAX_ENV_HEAP_SIZE];
    Expr * id = newLam("x", newVar("x"));
    Expr * expr = queens();
    ExprB * idB = analysis(id, emptyEnvB());
    ExprB * exprB = analysis(expr, emptyEnvB());
    cout << showExpr(id) << endl;
    cout << showExpr(expr) << endl;
    cout << showExpr(padVar(eval(idB, emptyEnvC()))) << endl;
    cout << showExpr(padVar(eval(exprB, emptyEnvC()))) << endl;
    cout << "Hp = " << Hp << " (MAX " << MAX_HEAP_SIZE << ")" << endl;
    cout << "Memory = " << Hp * sizeof(ExprC) / 1024 / 1024 << " MB" << endl;
    cout << "EnvHp = " << EnvHp << " (MAX " << MAX_ENV_HEAP_SIZE << ")" << endl;
    cout << "Memory = " << EnvHp * sizeof(EnvC) / 1024 / 1024 << " MB" << endl;
    return 0;
}
