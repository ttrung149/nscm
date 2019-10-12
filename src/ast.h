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
 *  File name: ast.h
 *
 *  Description: Prototypes for function that creates Abstract Syntax Tree. 
 *  See individual description in each function for more details.
 *
 *------------------------------------------------------------------------*/

#include "all.h"

#ifndef _AST_H
#define _ASH_H

/*=========================================================================
 *  Prototypes for functions returning expressions
 *  ----------------------------------------------
 *  mkLit(Value)          => Returns LITERAL expression from Value argument
 *  mkVar(Name)           => Returns VAR expression from Name argument
 *  mkSet(Name, Exp)      => Returns SET expression from Name and Exp
 *  mkIfx(Exp, Exp, Exp)  => Returns IFX expression
 *  mkWhileX(Exp, Exp);   => Returns WHILEX expression
 *========================================================================*/

Exp mkLit(Value literal);
Exp mkVar(Name var);
Exp mkSet(Name name, Exp exp);
Exp mkIfx(Exp cond, Exp truex, Exp falsex);
Exp mkWhileX(Exp cond, Exp body);

/*=========================================================================
 *  Prototypes for functions returning values
 *  -----------------------------------------
 *  mkNil()               => Returns Nil Value
 *  mkBoolv(bool)         => Returns Bool Value from bool argument
 *  mkNum(int32_t)        => Returns Num Value from uint32_t argument
 *  mkSym(Name)           => Returns Symbold Value from Name argument
 *  mkPair(Value, Value)  => Returns Pair Value from 2 Values argument
 *========================================================================*/

Value mkNil();
Value mkBoolv(bool boolv);
Value mkNum(int32_t num);
Value mkSym(Name sym);
Value mkPair(Value *car, Value *cdr);

#endif
