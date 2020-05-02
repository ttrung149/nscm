 /*============================================================================
 *  nanoscheme
 *  Copyright (c) 2019-2020 - Trung Truong
 *
 *  File name: parser.cpp
 *  Description: Implementation of parsing functions
 * 
 *==========================================================================*/
#include "expr.h"

/* Parsing table */
const std::unordered_map<std::string, PrimType> token_table {
    { "+"       , PrimType::ADD    },  { "-"         , PrimType::SUB     },
    { "*"       , PrimType::MUL    },
    { "/"       , PrimType::DIV    },  { ">"         , PrimType::GT      },
    { "<"       , PrimType::LT     },  { "mod"       , PrimType::MOD     },
    { ">="      , PrimType::GE     },  { "<="        , PrimType::LE      },
    { "car"     , PrimType::CAR    },  { "cdr"       , PrimType::CDR     }, 
    { "CONS"    , PrimType::CONS   },  { "lambda"    , PrimType::LAMBDA  },
    { "define"  , PrimType::DEFINE },  { "set"       , PrimType::SET     },
    { "number?" , PrimType::IS_NUM },  { "procedure?", PrimType::IS_PROC },
    { "symbol?" , PrimType::IS_SYM },  { "list?"     , PrimType::IS_LIST },
    { "if"      , PrimType::IF     },  { "while"     , PrimType::WHILE   }
};

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

/**
 * Helper function - generate number/string/literal/symbol expression 
 * @param expr Input expression string
 * @returns Pointer to allocated number/string/literal/symbol expression
 */
static Expr *make_const(std::string expr) {
    /* string expression */
    if (expr.size() > 1 && expr[0] == '"' && expr[expr.size()-1] == '"')
        return new Expr(expr);
    
    auto tokens = parse_expr("(" + expr + ")");
    if (tokens.size() != 1) throw "Invalid syntax at \n>>> " + expr;
    int64_t parsed_int;
    double parsed_float;

    /* numbers and literals */
    if (is_float(expr, parsed_float))       return new Expr(parsed_float);
    else if (is_int(expr, parsed_int))      return new Expr(parsed_int);
    else if (expr == "#t")                  return new Expr(LitType::TRUE);
    else if (expr == "#f")                  return new Expr(LitType::FALSE);
    else if (expr == "nil")                 return new Expr(LitType::NIL);
    
    /* symbol expression */
    else                                    return new Expr(expr, nullptr);
}

static Expr *make_list(ExpType type, std::string expr, Env *env) {
    (void) expr;
    (void) env;
    (void) type;
    return nullptr;
}

/**
 * Helper function - generate primitive 'define' or 'set' expression 
 * @param type Either PrimType::DEFINE or PrimType::SET
 * @param tokens Vector containing parsed string tokens for a 'define' or
 * 'set' expression
 * @param env Pointer to env
 * @returns Pointer to allocated expression for var assignment primitive
 */
static Expr *make_var_assignment(PrimType type, 
                        std::vector<std::string> &tokens, Env *env) {
    std::vector<Expr*> *args_list(new std::vector<Expr*>());
    
    if (tokens.size() != 3 && type == PrimType::DEFINE) {
        throw "Missing arguments for 'define'";
    }
    if (tokens.size() != 3 && type == PrimType::SET) {
        throw "Missing arguments for 'set!'";
    }
    Expr *sym_name = new Expr(tokens[1]);
    Expr *sym_val  = build_AST(tokens[2], env);

    args_list->push_back(sym_name);
    args_list->push_back(sym_val);
    return new Expr(type, args_list);
}

/**
 * Helper function - generate primitive 'lambda' expression 
 * @param tokens Vector containing parsed string tokens for a 'lambda' 
 * expression
 * @param env Pointer to env
 * @returns Pointer to allocated expression for lambda primitive
 */
static Expr *make_lambda(std::vector<std::string> &tokens, Env *env) {
    std::vector<Expr*> *args_list(new std::vector<Expr*>());

    if (tokens.size() != 3) throw "Missing arguments for 'lambda'";
    std::string _params = tokens[1];
    std::string _body   = tokens[2];

    if (_params[0] == '(' && _params[_params.size()-1] == ')') {
        throw "Missing brackets for closure argument";
    }
    if (_body[0] == '(' && _body[_body.size()-1] == ')') {
        throw "Missing brackets for closure body";
    }
    Expr *params = make_list(ExpType::STRING, _params, env);
    Expr *body   = build_AST(_body, env);

    args_list->push_back(params);
    args_list->push_back(body);
    return new Expr(PrimType::LAMBDA, args_list);
}

/**
 * Helper function - generic dispatcher to generate primitive expression
 * @param tokens Vector containing parsed string tokens for input 
 * expression
 * @param env Pointer to env
 * @returns Pointer to allocated primitive expression
 */
static Expr *make_prim(std::vector<std::string> &tokens, Env *env) {
    const auto prim_type = token_table.find(tokens[0]);

    if (prim_type == token_table.end())
        throw "Undefined primitive type: '" + prim_type->first + "'";

    /* var define and assignment */
    if (prim_type->first == "define" || prim_type->first == "set")
        return make_var_assignment(prim_type->second, tokens, env);
    
    /* lambda function */
    else if (prim_type->first == "lambda")
        return make_lambda(tokens, env);

    /* other primitives */
    else {
        std::vector<Expr*> *args_list(new std::vector<Expr*>());
        for (size_t i = 1; i < tokens.size(); i++) {
            args_list->push_back(build_AST(tokens[i], env));
        }
        return new Expr(prim_type->second, args_list);
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

    /* number/string/literal/symbol expression */
    if (expr[0] != '(' && expr[expr.size()-1] != ')') 
        return make_const(expr);
    
    /* list expression */
    // if (expr[0] != '\'')

    auto tokens = parse_expr(expr);
    if (tokens.size() == 0) throw "Can't parse expression of length zero";

    /* primitive expression */
    if (token_table.find(tokens[0]) != token_table.end())
        return make_prim(tokens, env);

    /* procedure call expression */

    return nullptr;
}
