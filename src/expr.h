/*============================================================================
 *  nanoscheme
 *  Copyright (c) 2019-2020 - Trung Truong
 *
 *  File name: expr.h
 *  Description: Header file for `Expr` class
 * 
 *==========================================================================*/
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <inttypes.h>
#include <math.h>

#include "env.h"
#ifndef EXPR_H_
#define EXPR_H_

/*============================================================================
 *  Enums and constants
 *===========================================================================*/
#define NO_BINDING nullptr

enum class ExpType { LIT, INT, FLOAT, STRING, LIST, SYMBOL, PROC, PRIM };
enum class PrimType { 
    IF, DEFINE, SET,                                // Control flow, var assign
    ADD, SUB, MUL, DIV, MOD, GT, LT, GE, LE,        // Arithmetic operations
    SIN, COS, TAN, SQRT, LOG, MAX, MIN, ABS,        // Math operations
    IS_NUM, IS_SYM, IS_PROC, IS_LIST, IS_STR,       // Type check
    IS_BOOL,
    LAMBDA,                                         // Lambda expression
    CAR, CDR, CONS, IS_NULL, MAP, FILTER, APPEND    // List operations
};
enum class LitType { TRUE, FALSE, NIL };

/*============================================================================
 *  Expression class
 *===========================================================================*/
class Expr {
private:
    ExpType type;
    union {
        int64_t ival; double fval; std::string sval = ""; LitType lit;
        std::vector<Expr*> *list;
        std::tuple<std::string, Expr*> sym;
        std::tuple<PrimType, std::vector<Expr*> *> prim;
        std::tuple<Expr*, Expr*, Env*> proc;
    };

    /* Specific type evaluators */
    Expr eval_sym(std::vector<Expr*> *bindings, Env *e);
    Expr eval_proc(std::vector<Expr*> *bindings, Env *e);
    Expr eval_prim(std::vector<Expr*> *bindings, Env *e);

public:
    /* Constructors */
    Expr(int64_t i);
    Expr(double f);
    Expr(std::string s);
    Expr(LitType l);
    Expr(std::vector<Expr*> *l);

    Expr(std::string sym_name, Expr *sym_val); 
    Expr(PrimType t, std::vector<Expr*> *args);
    Expr(Expr *params, Expr *body, Env *env);
    ~Expr();

    /* Copy constructor */
    Expr(const Expr &e);

    /* Getters */
    ExpType get_expr_type(void);
    PrimType get_prim_type(void);

    /* Generic evaluator dispatcher */
    Expr eval(std::vector<Expr*> *bindings, Env *e);

    /* IO */
    void print_to_console(void);
};

#endif
