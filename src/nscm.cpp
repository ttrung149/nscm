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
#include "env.h"
#include "expr.h"
#include "parser.h"

/*============================================================================
 *  Main driver
 *===========================================================================*/
int main(int argc, char*argv[]) {
    (void) argc;
    (void) argv;
    std::unordered_map<std::string, Expr*> std_env_frame = {};
    Env global_env(std_env_frame);

    // ((lambda (x y) (/ x y)) 10 2) = 5
    Expr str_x("x");                Expr str_y("y");
    Expr sym_x("x", nullptr);       Expr sym_y("y", nullptr);

    std::vector<Expr*> args_vec = {&str_x, &str_y};
    Expr args_list(&args_vec);

    std::vector<Expr*> div_vec = {&sym_x, &sym_y};
    Expr div_x_y_op(PrimType::DIV, &div_vec);

    std::vector<Expr*> lambda_div_x_y_vec = {&args_list, &div_x_y_op};
    Expr function(PrimType::LAMBDA, &lambda_div_x_y_vec);

    Expr _x(int64_t(10)); Expr _y(3.0);
    std::vector<Expr*> bindings = {&_x, &_y};

    Expr add(PrimType::ADD, &bindings);

    try {
        function.eval(NO_BINDING, &global_env)
                .eval(&bindings, &global_env)
                .print_to_console();
        std::cout << "\n";
        add.eval(NO_BINDING, &global_env).print_to_console();
        std::cout << "\n";
    }
    catch (const char* e) {
        std::cerr << "ERR: " << e << "\n";
        exit(EXIT_FAILURE);
    }
    catch (const std::string &e) {
        std::cerr << "ERR: " << e << "\n";
        exit(EXIT_FAILURE);
    }

    try {
        build_AST("3124.3123", nullptr)
        ->eval(NO_BINDING, &global_env).print_to_console();
        std::cout << "\n";
    }
    catch (const char* e) {
        std::cerr << "ERR: " << e << "\n";
        exit(EXIT_FAILURE);
    }
    catch (const std::string &e) {
        std::cerr << "ERR: " << e << "\n";
        exit(EXIT_FAILURE);
    }

    return 0;
}
