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
#include <inttypes.h>
#include <memory>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <functional>

/*============================================================================
 *  Enums and constatn
 *===========================================================================*/
enum class Type {
    INT, FLOAT, SYMBOL, PROC, PRIM
};

enum class PrimType {
    ADD, SUB, MUL, DIV
};

/* Forward-declartion of `Env` class */
class Env;

/*============================================================================
 *  Expression class
 *===========================================================================*/
class Expr {
private:
    Type type;
    union {
        int64_t ival;
        double fval;
        std::string symbol;
        std::tuple<PrimType, std::vector<Expr*> *, Env*> prim; 
        std::tuple<std::vector<Expr*> *, Expr*, Env*> proc;
    };
public:
    Expr(int64_t i)      { type = Type::INT; ival = i; };
    Expr(double f)       { type = Type::FLOAT; fval = f; };
    Expr(std::string s)  { type = Type::SYMBOL; symbol = s; };

    Expr(PrimType t, std::vector<Expr*> *args, Env *env) {
        type = Type::PRIM;
        prim = std::make_tuple(t, args, env);
    };
    Expr(std::vector<Expr*> *params, Expr *body, Env *env) {
        type = Type::PROC;
        proc = std::make_tuple(params, body, env);
    };
    ~Expr()              {};

    Expr *eval_prim(Env *env);
    Expr *eval(Env *env);
    void print_to_console();
};

/**
 * Evaluate primitive
 * @param e pointer to env
 * @returns Pointer to evaluated expression 
 */
Expr* Expr::eval_prim(Env *e) {
    if (type != Type::PRIM) throw "Eval failed: Not primitive type!";
    
    PrimType prim_type = std::get<0>(prim);
    std::vector<Expr*> args = *(std::get<1>(prim));

    switch (prim_type) {
        // Integer addition
        case PrimType::ADD: {
            int64_t s = 0;
            for (const auto &arg : args) {
                s += (arg->eval(e))->ival;
            }
            return new Expr(s);
        }
        // Integer subtraction
        case PrimType::SUB: {
            if (args.size() != 2) throw "Invalid num args for '-'";
            int64_t d = args[0]->eval(e)->ival - args[1]->eval(e)->ival;
            return new Expr(d);
        }
        // Integer multiplication
        case PrimType::MUL: {
            int64_t p = 1;
            for (const auto &arg : args) {
                p *= arg->eval(e)->ival;
            }
            return new Expr(p);
        }
        // Integer division
        case PrimType::DIV: {
            if (args.size() != 2) throw "Invalid num args for '/'";
            int64_t den = (args[1]->eval(e))->ival;
            if (den == 0) throw "Err: Division by zero";
            int64_t num = (args[0]->eval(e))->ival;
            return new Expr(num / den);
        }

        // Invalid primitive
        default: throw "Invalid primitive'";
    }
};

Expr* Expr::eval(Env *env) {
    switch (type) {
        case Type::INT:     { return this; }
        case Type::FLOAT:   { return this; }
        case Type::PRIM:    { return eval_prim(env); }
        default:            { return nullptr; }
    }
};

void Expr::print_to_console() {
    switch (type) {
        case Type::INT:     { std::cout << ival << "\n"; break; }
        case Type::FLOAT:   { std::cout << fval << "\n"; break; }
        case Type::PROC:    { std::cout << "<procedure>\n"; break; }  
        default:            break;
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

int main(int argc, char*argv[]) {
    (void) argc;
    (void) argv;

    Expr* a = new Expr(int64_t(2));
    Expr* b = new Expr(int64_t(5));
    Expr* c = new Expr(int64_t(10));
    Expr* d = new Expr(int64_t(10.5));
    std::vector<Expr*> x = {a, b};

    Expr *e = new Expr(PrimType::SUB, &x, nullptr);

    try {
        (e->eval(nullptr))->print_to_console();
    }
    catch (const char* e) {
        std::cerr << "ERR: " << e << "\n";
        exit(EXIT_FAILURE);
    }

    delete a;
    delete b;
    delete c;
    delete d;
    delete e;

    return 0;
}
