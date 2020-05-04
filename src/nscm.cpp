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
#include <csignal>
#include "env.h"
#include "expr.h"
#include "parser.h"

void terminate(int signum) {
    exit(signum);
}

/*============================================================================
 *  REPL implementation
 *===========================================================================*/
void repl(std::istream &in) {
    std::unordered_map<std::string, Expr*> std_env_frame {};
    Env global_env(std_env_frame);

    while (true) {
        std::string expr_str;
        std::cout << "nscm> ";
        std::getline(in, expr_str);
        if (expr_str.size() < 1) break;
        if (expr_str == "exit") break;

        try {
            Expr *expr = build_AST(expr_str, &global_env);
            if (expr->get_expr_type() == ExpType::PRIM)
                expr->eval(NO_BINDING, &global_env).print_to_console();
            else
                expr->print_to_console();
            std::cout << "\n";
        }
        catch (const char* e) {
            std::cerr << "ERR: " << e << "\n";
        }
        catch (const std::string &e) {
            std::cerr << "ERR: " << e << "\n";
        }
        catch (...) {
            std::cerr << "Unexpected error\n";
        }
    }
}

/*============================================================================
 *  Main driver
 *===========================================================================*/
int main(int argc, char*argv[]) {
    (void) argc;
    (void) argv;
    signal(SIGINT, terminate);
    repl(std::cin);

    return 0;
}
