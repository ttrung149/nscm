 /*============================================================================
 *  nanoscheme
 *  Copyright (c) 2019-2020 - Trung Truong
 *
 *  File name: parser.h
 *  Description: Function signatures for parsing functions
 * 
 *==========================================================================*/
#include "env.h"
#include "expr.h"

std::vector<std::string> parse_expr(std::string expr);
Expr *build_AST(std::string expr, Env *env);