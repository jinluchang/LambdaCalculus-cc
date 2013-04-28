/* vim: set expandtab : */

#include <iostream>
#include <sstream>
#include <string>

#include "expr.h"

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

Expr * newFreeVar(string * name) {
    Expr * var = new Expr;
    var->tag = TFreeVar;
    var->var.name = name;
    return var;
}

Expr * newFreeVar(const char * name_cstr) {
    string * name = new string(name_cstr);
    return newFreeVar(name);
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

Expr * newClosure(Lam def, Env * env) {
    Expr * closure = new Expr;
    closure->tag = TClosure;
    closure->closure.def = def;
    closure->closure.env = env;
    return closure;
}

Expr * newThunk(Expr * expr, Env * env) {
    Expr * thunk = new Expr;
    thunk->tag = TThunk;
    thunk->thunk.expr = expr;
    thunk->thunk.env = env;
    return thunk;
}

Env * emptyEnv() {
    return NULL;
}

Env * addToEnv(string * name, Expr * value, Env * oldEnv) {
    Env * env = new Env;
    env->name = name;
    env->value = value;
    env->next = oldEnv;
    return env;
}

Expr * lookupEnv(string * name, Env * env) {
    while (NULL != env) {
        if (0 == name->compare(*(env->name))) {
            return env->value;
        } else {
            env = env->next;
        }
    }
    return NULL;
}

Expr * eval(Expr * expr, Env * env) {
    Expr * ans;
    Tag tag = expr->tag;
    if (TVar == tag) {
        ans = lookupEnv(expr->var.name, env);
        if (NULL == ans) {
            ans = newFreeVar(expr->var.name);
        } else if (TThunk == ans->tag) {
            *ans = *eval(ans->thunk.expr, ans->thunk.env);
        }
    } else if (TLam == tag) {
        ans = newClosure(expr->lam, env);
    } else if (TApp == tag) {
        ans = apply(expr->app.fun, expr->app.arg, env);
    } else {
        ans = NULL;
        cerr << "eval : tag = " << tag << endl;
    }
    return ans;
}

Expr * apply(Expr * fun, Expr * arg, Env * env) {
    Expr * ans;
    fun = eval(fun, env);
    if (TClosure == fun->tag) {
        arg = newThunk(arg, env);
        ans = eval(fun->closure.def.body, addToEnv(fun->closure.def.parm, arg, fun->closure.env));
    } else {
        arg = eval(arg, env);
        ans = newApp(fun, arg);
    }
    return ans;
}

Expr * freshVar() {
    static int k = 0;
    ostringstream out;
    out << "$var" << k;
    string * name = new string(out.str());
    k++;
    return newFreeVar(name);
}

Expr * padVar(Expr * expr) {
    Expr * ans;
    Tag tag = expr->tag;
    if (TFreeVar == tag) {
        ans = newVar(expr->var.name);
    } else if (TApp == tag) {
        ans = newApp(padVar(expr->app.fun), padVar(expr->app.arg));
    } else if (TClosure == tag) {
        Expr * fvar = freshVar();
        Env * env = addToEnv(expr->closure.def.parm, fvar, expr->closure.env);
        ans = newLam(fvar->var.name, padVar(eval(expr->closure.def.body, env)));
    } else {
        ans = NULL;
        cerr << "padVar : tag = " << tag << endl;
    }
    return ans;
}

string showExpr(Expr * expr) {
    string ans;
    Tag tag = expr->tag;
    if (TVar == tag || TFreeVar == tag) {
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
    if (TVar == tag || TFreeVar == tag) {
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
    Expr * id = newLam("x", newVar("x"));
    Expr * expr =

        newApp(newApp(newLam("f",
        newLam("b",
        newApp(newVar("b"),
        newVar("f")))),
        newLam("f",
        newLam("b",
        newApp(newVar("b"),
        newVar("f"))))),
        newLam("define",
        newApp(newApp(newVar("define"),
        newLam("x",
        newVar("x"))),
        newLam("id",
        newApp(newApp(newVar("define"),
        newLam("c",
        newLam("x",
        newVar("c")))),
        newLam("const",
        newApp(newApp(newVar("define"),
        newLam("f",
        newApp(newLam("x",
        newApp(newVar("f"),
        newApp(newVar("x"),
        newVar("x")))),
        newLam("x",
        newApp(newVar("f"),
        newApp(newVar("x"),
        newVar("x"))))))),
        newLam("fix",
        newApp(newApp(newVar("define"),
        newLam("x",
        newLam("y",
        newVar("x")))),
        newLam("true",
        newApp(newApp(newVar("define"),
        newLam("x",
        newLam("y",
        newVar("y")))),
        newLam("false",
        newApp(newApp(newVar("define"),
        newLam("b",
        newLam("x",
        newLam("y",
        newApp(newApp(newVar("b"),
        newVar("y")),
        newVar("x")))))),
        newLam("not",
        newApp(newApp(newVar("define"),
        newLam("a",
        newLam("b",
        newApp(newApp(newVar("a"),
        newVar("b")),
        newVar("false"))))),
        newLam("and",
        newApp(newApp(newVar("define"),
        newLam("a",
        newLam("b",
        newApp(newApp(newVar("a"),
        newVar("true")),
        newVar("b"))))),
        newLam("or",
        newApp(newApp(newVar("define"),
        newLam("x",
        newLam("y",
        newLam("f",
        newApp(newApp(newVar("f"),
        newVar("x")),
        newVar("y")))))),
        newLam("cons",
        newApp(newApp(newVar("define"),
        newLam("p",
        newApp(newVar("p"),
        newVar("true")))),
        newLam("car",
        newApp(newApp(newVar("define"),
        newLam("p",
        newApp(newVar("p"),
        newVar("false")))),
        newLam("cdr",
        newApp(newApp(newVar("define"),
        newVar("false")),
        newLam("nil",
        newApp(newApp(newVar("define"),
        newLam("p",
        newApp(newApp(newVar("p"),
        newLam("_",
        newLam("_",
        newLam("_",
        newVar("false"))))),
        newVar("true")))),
        newLam("isNil",
        newApp(newApp(newVar("define"),
        newLam("p",
        newApp(newVar("not"),
        newApp(newVar("isNil"),
        newVar("p"))))),
        newLam("isPair",
        newApp(newApp(newVar("define"),
        newApp(newVar("fix"),
        newLam("map",
        newLam("f",
        newLam("xs",
        newApp(newApp(newApp(newVar("isNil"),
        newVar("xs")),
        newVar("nil")),
        newApp(newApp(newVar("cons"),
        newApp(newVar("f"),
        newApp(newVar("car"),
        newVar("xs")))),
        newApp(newApp(newVar("map"),
        newVar("f")),
        newApp(newVar("cdr"),
        newVar("xs")))))))))),
        newLam("map",
        newApp(newApp(newVar("define"),
        newApp(newVar("fix"),
        newLam("filter",
        newLam("f",
        newLam("xs",
        newApp(newApp(newApp(newVar("isNil"),
        newVar("xs")),
        newVar("nil")),
        newApp(newApp(newApp(newVar("f"),
        newApp(newVar("car"),
        newVar("xs"))),
        newApp(newApp(newVar("cons"),
        newApp(newVar("car"),
        newVar("xs"))),
        newApp(newApp(newVar("filter"),
        newVar("f")),
        newApp(newVar("cdr"),
        newVar("xs"))))),
        newApp(newApp(newVar("filter"),
        newVar("f")),
        newApp(newVar("cdr"),
        newVar("xs")))))))))),
        newLam("filter",
        newApp(newApp(newVar("define"),
        newApp(newVar("fix"),
        newLam("foldr",
        newLam("step",
        newLam("init",
        newLam("xs",
        newApp(newApp(newApp(newVar("isNil"),
        newVar("xs")),
        newVar("init")),
        newApp(newApp(newVar("step"),
        newApp(newVar("car"),
        newVar("xs"))),
        newApp(newApp(newApp(newVar("foldr"),
        newVar("step")),
        newVar("init")),
        newApp(newVar("cdr"),
        newVar("xs"))))))))))),
        newLam("foldr",
        newApp(newApp(newVar("define"),
        newLam("f",
        newLam("x",
        newVar("x")))),
        newLam("0",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("f",
        newLam("x",
        newApp(newVar("f"),
        newApp(newApp(newVar("n"),
        newVar("f")),
        newVar("x"))))))),
        newLam("succ",
        newApp(newApp(newVar("define"),
        newApp(newVar("succ"),
        newVar("0"))),
        newLam("1",
        newApp(newApp(newVar("define"),
        newApp(newVar("succ"),
        newVar("1"))),
        newLam("2",
        newApp(newApp(newVar("define"),
        newApp(newVar("succ"),
        newVar("2"))),
        newLam("3",
        newApp(newApp(newVar("define"),
        newApp(newVar("succ"),
        newVar("3"))),
        newLam("4",
        newApp(newApp(newVar("define"),
        newApp(newVar("succ"),
        newVar("4"))),
        newLam("5",
        newApp(newApp(newVar("define"),
        newApp(newVar("succ"),
        newVar("5"))),
        newLam("6",
        newApp(newApp(newVar("define"),
        newApp(newVar("succ"),
        newVar("6"))),
        newLam("7",
        newApp(newApp(newVar("define"),
        newApp(newVar("succ"),
        newVar("7"))),
        newLam("8",
        newApp(newApp(newVar("define"),
        newApp(newVar("succ"),
        newVar("8"))),
        newLam("9",
        newApp(newApp(newVar("define"),
        newApp(newVar("succ"),
        newVar("9"))),
        newLam("10",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("f",
        newLam("x",
        newApp(newApp(newApp(newVar("n"),
        newLam("h",
        newLam("g",
        newApp(newVar("g"),
        newApp(newVar("h"),
        newVar("f")))))),
        newLam("g",
        newVar("x"))),
        newVar("id")))))),
        newLam("prec",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("m",
        newApp(newApp(newVar("n"),
        newVar("succ")),
        newVar("m"))))),
        newLam("+",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("m",
        newApp(newApp(newVar("m"),
        newVar("prec")),
        newVar("n"))))),
        newLam("-",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("m",
        newLam("f",
        newApp(newVar("m"),
        newApp(newVar("n"),
        newVar("f"))))))),
        newLam("*",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("m",
        newApp(newApp(newVar("m"),
        newApp(newVar("*"),
        newVar("n"))),
        newVar("1"))))),
        newLam("exponential",
        newApp(newApp(newVar("define"),
        newLam("n",
        newApp(newApp(newVar("n"),
        newApp(newVar("const"),
        newVar("true"))),
        newVar("false")))),
        newLam("isPositive",
        newApp(newApp(newVar("define"),
        newLam("n",
        newApp(newApp(newVar("n"),
        newApp(newVar("const"),
        newVar("false"))),
        newVar("true")))),
        newLam("isZero",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("m",
        newApp(newVar("isPositive"),
        newApp(newApp(newVar("-"),
        newVar("n")),
        newVar("m")))))),
        newLam(">",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("m",
        newApp(newVar("isPositive"),
        newApp(newApp(newVar("-"),
        newVar("m")),
        newVar("n")))))),
        newLam("<",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("m",
        newApp(newVar("isZero"),
        newApp(newApp(newVar("-"),
        newVar("m")),
        newVar("n")))))),
        newLam(">=",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("m",
        newApp(newVar("isZero"),
        newApp(newApp(newVar("-"),
        newVar("n")),
        newVar("m")))))),
        newLam("<=",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("m",
        newApp(newApp(newVar("and"),
        newApp(newApp(newVar("<="),
        newVar("n")),
        newVar("m"))),
        newApp(newApp(newVar(">="),
        newVar("n")),
        newVar("m")))))),
        newLam("=",
        newApp(newApp(newVar("define"),
        newLam("n",
        newLam("m",
        newApp(newApp(newVar("or"),
        newApp(newApp(newVar("<"),
        newVar("n")),
        newVar("m"))),
        newApp(newApp(newVar(">"),
        newVar("n")),
        newVar("m")))))),
        newLam("/=",
        newApp(newApp(newVar("define"),
        newApp(newVar("fix"),
        newLam("fac",
        newLam("n",
        newApp(newApp(newApp(newVar("isZero"),
        newVar("n")),
        newVar("1")),
        newApp(newApp(newVar("*"),
        newVar("n")),
        newApp(newVar("fac"),
        newApp(newVar("prec"),
        newVar("n"))))))))),
        newLam("factorial",
        newApp(newApp(newVar("define"),
        newApp(newVar("fix"),
        newLam("len",
        newLam("xs",
        newApp(newApp(newApp(newVar("isNil"),
        newVar("xs")),
        newVar("0")),
        newApp(newVar("succ"),
        newApp(newVar("len"),
        newApp(newVar("cdr"),
        newVar("xs"))))))))),
        newLam("length",
        newApp(newApp(newVar("define"),
        newLam("size",
        newApp(newApp(newVar("fix"),
        newLam("gen",
        newLam("n",
        newApp(newApp(newApp(newApp(newVar(">"),
        newVar("n")),
        newVar("size")),
        newVar("nil")),
        newApp(newApp(newVar("cons"),
        newVar("n")),
        newApp(newVar("gen"),
        newApp(newVar("succ"),
        newVar("n")))))))),
        newVar("1")))),
        newLam("genList",
        newApp(newApp(newVar("define"),
        newLam("ks",
        newLam("k",
        newApp(newApp(newApp(newVar("fix"),
        newLam("isOk",
        newLam("n",
        newLam("ks",
        newApp(newApp(newApp(newVar("isNil"),
        newVar("ks")),
        newVar("true")),
        newApp(newApp(newVar("define"),
        newApp(newVar("car"),
        newVar("ks"))),
        newLam("head",
        newApp(newApp(newVar("and"),
        newApp(newApp(newVar("/="),
        newVar("k")),
        newVar("head"))),
        newApp(newApp(newVar("and"),
        newApp(newApp(newVar("/="),
        newVar("head")),
        newApp(newApp(newVar("+"),
        newVar("n")),
        newVar("k")))),
        newApp(newApp(newVar("and"),
        newApp(newApp(newVar("/="),
        newVar("k")),
        newApp(newApp(newVar("+"),
        newVar("n")),
        newVar("head")))),
        newApp(newApp(newVar("isOk"),
        newApp(newVar("succ"),
        newVar("n"))),
        newApp(newVar("cdr"),
        newVar("ks"))))))))))))),
        newVar("1")),
        newVar("ks"))))),
        newLam("isValidNQ",
        newApp(newApp(newVar("define"),
        newLam("size",
        newApp(newApp(newApp(newApp(newVar("fix"),
        newLam("search",
        newLam("n",
        newLam("ks",
        newApp(newApp(newApp(newVar("isZero"),
        newVar("n")),
        newApp(newVar("cons"),
        newVar("ks"))),
        newApp(newApp(newApp(newVar("foldr"),
        newLam("f",
        newLam("g",
        newLam("x",
        newApp(newVar("f"),
        newApp(newVar("g"),
        newVar("x"))))))),
        newVar("id")),
        newApp(newApp(newVar("map"),
        newApp(newVar("search"),
        newApp(newVar("prec"),
        newVar("n")))),
        newApp(newApp(newVar("map"),
        newLam("k",
        newApp(newApp(newVar("cons"),
        newVar("k")),
        newVar("ks")))),
        newApp(newApp(newVar("filter"),
        newApp(newVar("isValidNQ"),
        newVar("ks"))),
        newApp(newVar("genList"),
        newVar("size"))))))))))),
        newVar("size")),
        newVar("nil")),
        newVar("nil")))),
        newLam("searchNQ",
        newApp(newVar("length"),
        newApp(newVar("searchNQ"),
        newVar("8")))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))

        ;
    cout << "hello world" << endl;
    cout << showExpr(id) << endl;
    cout << showExpr(expr) << endl;
    cout << showExpr(padVar(eval(id, emptyEnv()))) << endl;
    cout << showExpr(padVar(eval(expr, emptyEnv()))) << endl;
    return 0;
}
