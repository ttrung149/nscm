 /*============================================================================
 *  nanoscheme
 *  Copyright (c) 2019-2020 - Trung Truong
 *
 *  File name: parser.cpp
 *  Description: Implementation of parsing functions
 * 
 *==========================================================================*/
#include "expr.h"

/**
 * Check if string is int. If string is int, parse the string
 * @param expr String of expression
 * @param parsed Reference to parsed int
 * @returns If string is int type, return true and pass parsed string by 
 * reference. Else, return false.
 */
static inline bool is_int(std::string expr, int64_t &parsed) {
    try { parsed = std::stol(expr, nullptr); return true; }
    catch(...) { return false; }
}

/**
 * Check if string is float. If string is float, parse the string
 * @param expr String of expression
 * @param parsed Reference to parsed float
 * @returns If string is float type, return true and pass parsed string by 
 * reference. Else, return false.
 */
static inline bool is_float(std::string expr, double &parsed) {
    try { parsed = std::stod(expr, nullptr); return true; }
    catch(...) { return false; }
}

/**
 * Find the closest word in the string before encountering a space
 * or a new line character
 * @param expr String of expression
 * @param parse Reference to parsed string
 * @returns The next position on the expression string from where the parsed
 * string ends
 */
static size_t read_til_space(std::string expr, std::string &parsed) {
    size_t idx = 0;
    std::string curr_str = "";

    while (idx < expr.size()) {
        if (expr[idx] == ' ' || expr[idx] == '\n' || expr[idx] == ')') 
            break;
        else curr_str += expr[idx];
        idx++;
    }
    parsed = curr_str;
    return idx;
}

/**
 * Find the closest expression in the string before encountering a matching
 * bracket. For every '(', there must be a matching ')' in argument string,
 * else an Exception is thrown.
 * @param expr String of expression
 * @param parse Reference to parsed string
 * @returns The next position on the expression string from where the parsed
 * string ends
 */
static size_t read_til_end_bracket(std::string expr, std::string &parsed) {
    size_t idx = expr.find('(');
    if (idx == std::string::npos) throw "Missing '('";
    std::string curr_str = "(";
    int bracket_stack = 1;
    idx++;

    while (bracket_stack != 0 && idx < expr.size()) {
        if (expr[idx] == '(')      { bracket_stack++; curr_str += expr[idx]; }
        else if (expr[idx] == ')') { bracket_stack--; curr_str += expr[idx]; }
        else                       { curr_str += expr[idx]; }
        idx++;
    }
    if (bracket_stack != 0) throw "Unmatching brackets \n>>> '" + expr + "'";
    parsed = curr_str;
    return idx;
}

/**
 * Parses expression from a given input string
 * @param expr String of expression - format: `(<op> <args1> <args2> ..)`
 * @returns A vector containing all parsed expression string from input.
 * Exception is thrown if input string cannot be parsed (syntax error)
 */
std::vector<std::string> parse_expr(std::string expr) {
    size_t expr_len = expr.size();
    if (expr_len == 0) 
        throw "Unable to parse empty string";
    if (expr[0] != '(' || expr[expr_len-1] != ')')
        throw "Unmatching brackets \n>>> '" + expr + "'";

    size_t idx = 1;
    std::vector<std::string> res;

    while (idx < expr_len - 1) {
        if (expr[idx] == '(') {
            std::string parsed = "";
            int cursor = idx;
            idx += read_til_end_bracket(expr.substr(cursor), parsed);
            res.push_back(parsed);
        }
        else if (expr[idx] == '\'' && expr[idx+1] == '(') {
            std::string parsed = "";
            idx++;
            int cursor = idx;
            idx += read_til_end_bracket(expr.substr(cursor), parsed);
            res.push_back("\'" + parsed);
        }
        else if (expr[idx] == ' ' || expr[idx] == '\n') {
            idx++;
        }
        else {
            std::string parsed = "";
            int cursor = idx;
            idx += read_til_space(expr.substr(cursor), parsed);
            res.push_back(parsed);
        }
    }
    return res;
}

/*============================================================================
 *  Abstract Syntax Tree (AST) implementation
 *===========================================================================*/
/* Forward declare build_AST funtion */
Expr *build_AST(std::string expr, Env *env);

static Expr *make_const(std::string expr) {
    auto tokens = parse_expr("(" + expr + ")");
    if (tokens.size() != 1) throw "Invalid syntax at \n>>> " + expr;

    int64_t parsed_int;
    double parsed_float;

    if (expr.size() > 1 && expr[0] == '"' && expr[expr.size()-1] == '"') {
        return new Expr(expr);
    }
    else if (is_float(expr, parsed_float)) {
        return new Expr(parsed_float);
    }
    else if (is_int(expr, parsed_int)) {
        return new Expr(parsed_int);
    }
    else {
        return new Expr(expr, nullptr);
    }
}

/**
 * Recursively generates AST based on input string
 * @param expr String of expressions
 * @param env Pointer to Env
 * @returns Pointer to root AST node
 */
Expr *build_AST(std::string expr, Env *env) {
    (void) env;
    (void) expr;

    if (expr[0] != '(' && expr[expr.size()-1] != ')') {
        return make_const(expr);
    }
    if (expr[0] != '\'') {
    }

    auto tokens = parse_expr(expr);
    if (tokens.size() == 0) throw "Can't parse expression of length zero";

    return nullptr;
}
