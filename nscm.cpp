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

/* Enums and constants */
enum class Type {
    INT, FLOAT, SYMBOL, PROCEDURE, PRIMITIVE
};

/* Forward-declared expression class */
class Expr;

/* Environment class */
class Env {
private:
    std::unordered_map<std::string, Expr*> frame;
    Env *outer = nullptr;

public:
    Env();
    ~Env();
    void add_key_value_pair(std::string k, Expr *v) {
        (void) k;
        (void) v;
    };

    bool find_var(std::string v) {
        (void) v;
        return false;
    };
};

class Expr {
private:
    Type type;
    union {
        int ival;
        double fval;
        std::string symbol;
        std::tuple<std::vector<Expr*>, Expr*, Env*> proc;
        std::tuple<std::vector<Expr*>, Env*> primitive;
    };
public:
    Expr(int64_t i)      { type = Type::INT; ival = i; };
    Expr(double f)       { type = Type::FLOAT; fval = f; };
    Expr(std::string s)  { type = Type::SYMBOL; symbol = s; };

    Expr(std::vector<Expr*> args, Env *env) {
        type = Type::PRIMITIVE;
        primitive = {args, env};
    };
    Expr(std::vector<Expr*> params, Expr *body, Env *env) {
        type = Type::PROCEDURE;
        proc = {params, body, env};
    };
    ~Expr()              {};

    Expr *eval() {
        switch (type) {
            case Type::INT:
                return this;
                break;

            case Type::FLOAT:
                return this;
                break;

            default:
                return nullptr;
                break;
        }
    };

    void print_to_console() {
        switch (type) {
            case Type::INT:
                std::cout << ival << "\n";
                break;

            case Type::FLOAT:
                std::cout << fval << "\n";
                break;

            default:
                break;
        }
    }
};

int main(int argc, char*argv[]) {
    (void) argc;
    (void) argv;

    std::unique_ptr<Expr> e(new Expr(int64_t(3)));
    (e->eval())->print_to_console();

    return 0;
}
