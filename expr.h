#ifndef EXPR_H
#define EXPR_H

#include <string>

enum Tag {
	TVar,
	TLam,
	TApp,
	TFreeVar,
	TClosure,
	TThunk,
};

class Expr;
class Env;

class Var {
public :
	std::string * name;
};

class Lam {
public :
	std::string * parm;
	Expr * body;
};

class Closure {
public :
	Lam def;
	Env * env;
};

class App {
public :
	Expr * fun;
	Expr * arg;
};

class Thunk {
public :
	Expr * expr;
	Env * env;
};

class Expr {
public :
	Tag tag;
	union {
		Var var;
		Lam lam;
		App app;
		Closure closure;
		Thunk thunk;
	};
};

class Env {
public :
	std::string * name;
	Expr * value;
	Env * next;
};

Expr * eval(Expr * expr, Env * env);
Expr * apply(Expr * fun, Expr * arg, Env * env);

std::string showExpr(Expr * expr);
std::string showAExpr(Expr * expr);

#endif
