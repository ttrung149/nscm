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
#include <cstring>
#include "env.h"
#include "expr.h"
#include "parser.h"

void terminate(int signum) {
    std::cout << "\nExiting..\n";
    exit(signum);
}

/*============================================================================
 *  REPL implementation
 *===========================================================================*/
/**
 * Read-eval-print loop. Prints the result of the expression to stdout.
 * @param in Input stream
 * @returns void
 */
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
        catch (const char* e)         { std::cerr << "ERR: " << e << "\n"; }
        catch (const std::string &e)  { std::cerr << "ERR: " << e << "\n"; }
        catch (...)                   { std::cerr << "Unexpected error\n"; }
    }
}

/**
 * Evaluate .scm files
 * @param num_files Number of input files
 * @param file_names Input files name. File must have .scm extension.
 * @returns void
 */
void eval_files(int num_files, char* file_names[]) {
    std::unordered_map<std::string, Expr*> std_env_frame {};
    Env global_env(std_env_frame);

    for (int i = 1; i <= num_files; i++) {
        if (strstr(file_names[i], ".scm") == NULL) {
            std::cerr << "ERR: File '" + std::string(file_names[i]) + 
                         "' does not have a `.scm` extension.\n";
            exit(EXIT_FAILURE);
        }

        std::ifstream f(file_names[i]);
        if (f.is_open()) {
            std::string expr((std::istreambuf_iterator<char>(f)),
                              std::istreambuf_iterator<char>());
            try {
                std::vector<std::string> vec = parse_expr("(" + expr + ")");
                for (auto &expr_str : vec) {
                    Expr *expr = build_AST(expr_str, &global_env);
                    if (expr->get_expr_type() == ExpType::PRIM)
                        expr->eval(NO_BINDING, &global_env).print_to_console();
                    else
                        expr->print_to_console();
                    std::cout << "\n";
                }
                f.close();
            }
            catch (const char* e)        { std::cerr << "ERR: " << e << "\n"; }
            catch (const std::string &e) { std::cerr << "ERR: " << e << "\n"; }
            catch (...)                  { std::cerr << "Unexpected error\n"; }
        }
        else {
            std::cerr << "ERR: Can't open '" + std::string(file_names[i]) +
                         "'\n";
            exit(EXIT_FAILURE);
        }
    }
}
/*============================================================================
 *  Main driver
 *===========================================================================*/
int main(int argc, char*argv[]) {
    signal(SIGINT, terminate);

    /* Start repl */
    if (argc == 1) repl(std::cin);
    
    /* Help menu */
    else if (argc == 2 && strcmp(argv[1], "--help") == 0) {
        std::cout << "\n*=================================================="
                  << "\n*  nanoscheme"
                  << "\n*  Copyright (c) 2019-2020 - Trung Truong"
                  << "\n*==================================================\n";
        std::cout << "\n> Run \"./nscm\" to start the read-eval-print loop"
                  << "\n> Run \"./nscm <file.scm> ..\" to eval .scm files"
                  << "\n> Type \"exit\" to break eval loop\n\n";
    }

    /* Eval from files */
    else eval_files(argc - 1, argv);
    return EXIT_SUCCESS;
}
