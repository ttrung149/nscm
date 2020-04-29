/*============================================================================
 *  nanoscheme
 *  Copyright (c) 2019-2020 - Trung Truong
 *
 *  File name: nscm.cpp
 *
 *  Description: C++ implementation of nanoscheme language
 *  Usage: Run `./nscm --help` for more information.
 * 
 *==========================================================================*/
#define DEBUG

#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <inttypes.h>
#include <math.h>

/*============================================================================
 *  Enums and constants
 *===========================================================================*/
enum class ExpType { LIT, INT, FLOAT, SYMBOL, PROC, PRIM };
enum class PrimType { 
    ADD, SUB, MUL, DIV, MOD, GT, LT, GE, LE,        // Arithmetic operations
    IF, WHILE, DEFINE, SET,                         // Control flow
    IS_NUM, IS_SYMBOL, IS_PROC                      // Type check
};
enum class LitType { TRUE, FALSE, NIL };

/* Forward-declartion of `Env` class */
class Env;

/*============================================================================
 *  Expression class
 *===========================================================================*/
class Expr {
private:
    ExpType type;
    union {
        int64_t ival;
        double fval;
        std::string symbol;
        LitType lit;
        std::tuple<PrimType, std::vector<Expr*> *, Env*> prim; 
        std::tuple<Expr*, Expr*, Env*> proc;
    };

public:
    Expr(int64_t i)      { type = ExpType::INT; ival = i; };
    Expr(double f)       { type = ExpType::FLOAT; fval = f; };
    Expr(std::string s)  { type = ExpType::SYMBOL; symbol = s; };
    Expr(LitType l)      { type = ExpType::LIT; lit = l; };

    Expr(PrimType t, std::vector<Expr*> *args, Env *env) {
        type = ExpType::PRIM;
        prim = std::make_tuple(t, args, env);
    };
    Expr(Expr *params, Expr *body, Env *env) {
        type = ExpType::PROC;
        proc = std::make_tuple(params, body, env);
    };
    ~Expr()              {};

    Expr *eval_prim(Env *env);
    Expr *eval(Env *env);
    void print_to_console(void);
};

/**
 * Evaluate primitive
 * @param e pointer to env
 * @returns Pointer to evaluated expression 
 */
Expr* Expr::eval_prim(Env *e) {
    if (type != ExpType::PRIM) throw "Eval failed: Not primitive type!";
    
    PrimType prim_type = std::get<0>(prim);
    std::vector<Expr*> args = *(std::get<1>(prim));

    switch (prim_type) {
        /*======================= Control flow ===========================*/
        /* If statement */
        case PrimType::IF: {
            if (args.size() != 3) throw "Invalid num args for 'if'";
            Expr *cond = args[0]->eval(e);
            if (cond->type == ExpType::LIT && cond->lit == LitType::TRUE) 
                return args[1]->eval(e);
            
            if (cond->type == ExpType::INT && cond->ival > 0)
                return args[1]->eval(e);

            if (cond->type == ExpType::FLOAT && cond->fval > 0.0)
                return args[1]->eval(e);

            return args[2]->eval(e);
        }
        /* While statement */
        case PrimType::WHILE: {
            if (args.size() != 2) throw "Invalid num args for 'while'";
            Expr *cond = args[0]->eval(e);
            if (cond->type != ExpType::LIT) throw "Condition not lit type!";
            while (cond->lit == LitType::TRUE) {
                args[1]->eval(e);
                cond = args[0]->eval(e);
            }
            return nullptr;
        }
        /*======================= Arith operations =======================*/
        /* Integer addition */
        case PrimType::ADD: {
            double s = 0.0;
            for (const auto &arg : args) {
                Expr *exp = arg->eval(e);
                if (exp->type == ExpType::INT)          s += exp->ival;
                else if (exp->type == ExpType::FLOAT)   s += exp->fval;
                else throw "Invalid args type for '+'";
            }
            if (std::floor(s) == s) return new Expr(int64_t(s));
            else return new Expr(s);
        }
        /* Integer subtraction */
        case PrimType::SUB: {
            if (args.size() != 2) throw "Invalid num args for '-'";
            Expr *e1 = args[0]->eval(e);
            Expr *e2 = args[1]->eval(e);

            if (e1->type == ExpType::INT && e2->type == ExpType::INT)
                return new Expr(int64_t(e1->ival - e2->ival)); 
            else if (e1->type == ExpType::FLOAT && e2->type == ExpType::INT)
                return new Expr(e1->fval - e2->ival);
            else if (e1->type == ExpType::INT && e2->type == ExpType::FLOAT)
                return new Expr(e1->ival - e2->fval);
            else if (e1->type == ExpType::FLOAT && e2->type == ExpType::FLOAT)
                return new Expr(e1->fval - e2->fval);
            else throw "Invalid args type for '-'";
        }
        /* Integer multiplication */
        case PrimType::MUL: {
            double p = 1.0;
            for (const auto &arg : args) {
                Expr *exp = arg->eval(e);
                if (exp->type == ExpType::INT)          p *= exp->ival;
                else if (exp->type == ExpType::FLOAT)   p *= exp->fval;
                else throw "Invalid args type for '*'";
            }
            if (std::floor(p) == p) return new Expr(int64_t(p));
            else return new Expr(p);
        }
        /* Integer division */
        case PrimType::DIV: {
            if (args.size() != 2) throw "Invalid num args for '/'";
            Expr *e1 = args[0]->eval(e);
            Expr *e2 = args[1]->eval(e);

            if ((e2->type == ExpType::INT && e2->ival == 0) ||
                (e2->type == ExpType::FLOAT && e2->fval == 0)) 
                throw "Division by zero";

            if (e1->type == ExpType::INT && e2->type == ExpType::INT)
                return new Expr(int64_t(e1->ival / e2->ival)); 
            else if (e1->type == ExpType::FLOAT && e2->type == ExpType::INT)
                return new Expr(e1->fval / e2->ival);
            else if (e1->type == ExpType::INT && e2->type == ExpType::FLOAT)
                return new Expr(e1->ival / e2->fval);
            else if (e1->type == ExpType::FLOAT && e2->type == ExpType::FLOAT)
                return new Expr(e1->fval / e2->fval);
            else throw "Invalid args type for '/'";
        }
        /* Integer modulo */
        case PrimType::MOD: {
            if (args.size() != 2) throw "Invalid num args for 'modulo'";
            Expr *e1 = args[0]->eval(e);
            Expr *e2 = args[1]->eval(e);
                    
            if ((e2->type == ExpType::INT && e2->ival == 0) ||
                (e2->type == ExpType::FLOAT && e2->fval == 0)) 
                throw "Division by zero";
            
            if (e1->type == ExpType::INT && e2->type == ExpType::INT)
                return new Expr(e1->ival % e2->ival);
            else throw "Invalid args type for 'modulo'";
        }
        /*======================= Invalid primative =======================*/
        default: throw "Invalid primitive'";
    }
};

/**
 * Evaluate expression
 * @param env pointer to env
 * @returns Pointer to evaluated expression 
 */
Expr* Expr::eval(Env *env) {
    switch (type) {
        case ExpType::INT:     { return this; }
        case ExpType::FLOAT:   { return this; }
        case ExpType::LIT:     { return this; }
        case ExpType::PRIM:    { return eval_prim(env); }
        default:               { return nullptr; }
    }
};

/**
 * Print expression to stdout
 * @returns void
 */
void Expr::print_to_console(void) {
    switch (type) {
        case ExpType::INT:     { std::cout << ival << "\n"; break; }
        case ExpType::FLOAT:   { std::cout << fval << "\n"; break; }
        case ExpType::PROC:    { std::cout << "<procedure>\n"; break; }
        case ExpType::PRIM:    { std::cout << "<primitive>\n"; break; }
        case ExpType::LIT:     {
            switch (lit) {
                case LitType::TRUE:     std::cout<< "#t\n"; break;
                case LitType::FALSE:    std::cout<< "#f\n"; break;
                case LitType::NIL:      std::cout<< "()\n"; break;
                default: break;
            }
        }
        default:               break;
    }
}

/*============================================================================
 *  Environment class
 *===========================================================================*/
class Env {
private:
    std::unordered_map<std::string, Expr*> frame;
    // Env *outer = nullptr;

public:
    Env(Env *outer) {
        (void) outer;
    };
    ~Env() {};
    void add_key_value_pair(std::string k, Expr *v) {
        (void) k;
        (void) v;
    };
    bool find_var(std::string v) {
        (void) v;
        return false;
    };
};

/*============================================================================
 *  Main driver
 *===========================================================================*/
int main(int argc, char*argv[]) {
    (void) argc;
    (void) argv;

    // Expr* lit_true = new Expr(LitType::TRUE);
    Expr* x = new Expr(int64_t(10));
    Expr* y = new Expr(int64_t(2));
    Expr* z = new Expr(9.5);
    std::vector<Expr*> mul_args = {x, y};
    Expr *mul_x_y = new Expr(PrimType::MUL, &mul_args, nullptr);
    
    std::vector<Expr*> add_z_args = {mul_x_y, z};
    Expr *res = new Expr(PrimType::ADD, &add_z_args, nullptr);

    try {
        (res->eval(nullptr))->print_to_console();
    }
    catch (const char* e) {
        std::cerr << "ERR: " << e << "\n";
        exit(EXIT_FAILURE);
    }

    delete x;
    delete y;
    delete z;
    delete mul_x_y;
    delete res;

    return 0;
}
