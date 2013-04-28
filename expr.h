/* vim : set expandtab : */

#ifndef EXPR_H
#define EXPR_H

#include <string>

enum Tag {
    TVar,
    TLam,
    TApp
};

class Expr;

class Var {
public :
    std::string * name;
};

class Lam {
public :
    std::string * parm;
    Expr * body;
};

class App {
public :
    Expr * fun;
    Expr * arg;
};

class Expr {
public :
    Tag tag;
    union {
        Var var;
        Lam lam;
        App app;
    };
};

enum TagB {
    TVarB,
    TBoundB,
    TLamB,
    TAppB
};

class ExprB;

class VarB {
public :
    std::string * name;
};

class BoundB {
public :
    int index;
};

class LamB {
public :
    ExprB * body;
};

class AppB {
public :
    ExprB * fun;
    ExprB * arg;
};

class ExprB {
public :
    TagB tag;
    union {
        VarB var;
        BoundB bound;
        LamB lam;
        AppB app;
    };
};

class EnvB {
public :
    std::string * name;
    EnvB * next;
};

enum TagC {
    TVarC,
    TAppC,
    TClosure,
    TThunk
};

class ExprC;
class EnvC;

class VarC {
public :
    std::string * name;
};

class AppC {
public :
    ExprC * fun;
    ExprC * arg;
};

class Closure {
public :
    LamB def;
    EnvC * env;
};

class Thunk {
public :
    ExprB * expr;
    EnvC * env;
};

class ExprC {
public :
    TagC tag;
    union {
        VarC var;
        AppC app;
        Closure closure;
        Thunk thunk;
    };
};

class EnvC {
public :
    std::string * name;
    ExprC * value;
    EnvC * next;
};

ExprC * eval(ExprB * expr, EnvC * env);
ExprC * apply(ExprB * fun, ExprB * arg, EnvC * env);

std::string showExpr(Expr * expr);
std::string showAExpr(Expr * expr);

#endif
