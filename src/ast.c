/*------------------------------------------------------------------------
 *
 *  tiny-lisp-interpreter
 *  
 *  Copyright (c) 2019 - Trung Truong
 *  All rights reserved.

 *  Permission is hereby granted, free of charge, to any person 
 *  obtaining a copy of this software and associated documentation 
 *  files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge,
 *  publish, distribute, sublicense, and/or sell copies of the Software,
 *  and to permit persons to whom the Software is furnished to do so, 
 *  subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included 
 *  in all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
 *  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 *  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
 *  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 *  File name: ast.c
 *
 *  Description: Definitions for function that creates Abstract Syntax Tree. 
 *  See individual description in each function for more details.
 *
 *------------------------------------------------------------------------*/

#include "all.h"

/*=========================================================================
 *  Functions returning expressions
 *========================================================================*/

Exp mkLit(Value literal) {
    Exp exp = malloc(sizeof(*exp));
    assert(exp);

    exp->type = LITERAL;
    exp->u.literal = literal;
    return exp;
}

Exp mkVar(Name var) {
    Exp exp = malloc(sizeof(*exp));
    assert(exp);

    exp->type = VAR;
    exp->u.var = var;
    return exp;
}

Exp mkSet(Name name, Exp exp) {
    Exp expression = malloc(sizeof(*expression));
    assert(expression);

    expression->type = SET;
    expression->u.set.name = name;
    expression->u.set.exp = exp;

    return expression;
}

Exp mkIfx(Exp cond, Exp truex, Exp falsex) {
    Exp expression = malloc(sizeof(*expression));
    assert(expression);

    expression->type = IFX;
    expression->u.ifx.cond = cond;
    expression->u.ifx.truex = truex;
    expression->u.ifx.falsex = falsex;

    return expression;
}

Exp mkWhileX(Exp cond, Exp body) {
    Exp expression = malloc(sizeof(*expression));
    assert(expression);

    expression->type = WHILEX;
    expression->u.whilex.cond = cond;
    expression->u.whilex.body = body;

    return expression;
}
