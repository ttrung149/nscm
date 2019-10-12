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
 *  File name: all.h
 *
 *  Description: Struct definitions and constants for all source files. 
 *  See individual description for more details.
 *
 *------------------------------------------------------------------------*/

/* stdlib */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _ALL_HEADER
#define _ALL_HEADER

/*==========================================================================
 *  Enums Definitions
 *=========================================================================*/

/*
 *  From `Programming Languages: Build, Prove, and Compare`:
 * 
 *  Def* = VAL (Value)
 *       | EXP (Exp)
 *       | DEFINE
 */
typedef enum DefType {VAL, EXP, DEFINE} DefType;

/*
 *  From `Programming Languages: Build, Prove, and Compare`:
 * 
 *  Exp* = LITERAL (Value)
 *       | VAR (Name)
 *       | SET (Name, Exp)
 *       | IFX (Exp cond, Exp truex, Exp falsex)
 *       | WHILEX (Exp cond, Exp body)
 *       | BEGIN (list of Exp)
 *       | APPLY (Exp fun, list of Exp)
 *       | LETX
 *       | LAMBDAX (Lambda)
 */
typedef enum ExpType {
    LITERAL,
    VAR,
    SET,
    IFX,
    WHILEX,
    BEGIN,
    APPLY,
    LETX,
    LAMBDAX
} ExpType;

/*
 *  From `Programming Languages: Build, Prove, and Compare`:
 * 
 *  Val* = NIL
 *       | BOOLV (bool)
 *       | NUM (int32_t)
 *       | SYM (Name)
 *       | PAIR (Value *car, Value *cdr)
 *       | CLOSURE
 *       | PRIMITIVE (int tag, Primitive *function)
 */
typedef enum ValueType {
    NIL,
    BOOLV,
    NUM,
    SYM,
    PAIR,
    CLOSURE,
    PRIMITIVE
} ValueType;

/*==========================================================================
 *  Struct Definitions
 *=========================================================================*/

typedef struct Name *Name;
typedef struct Value *Value;
typedef struct Exp *Exp;
typedef struct Env *Env;

// Name - Containing a string that represents a variable name
struct Name {
    const char *s;
};

// Exp - An expression, containing data corresponding to each
// type specified in ExpType enum
struct Exp {
    ExpType type;
    union {
        Value literal;
        Name var;
        struct {
            Name name; 
            Exp exp;
        } set;
        struct {
            Exp cond; 
            Exp truex; 
            Exp falsex;
        } ifx;
        struct {
            Exp cond;
            Exp body;
        } whilex;
    } u;
};

// Value - A value, containing data corresponding to each
// type specified in ValueType enum
struct Value {
    ValueType type;
    union {
        bool boolv;
        int32_t num;
        Name sym;
        struct {
            Value *car;
            Value *cdr;
        } pair;
    } u;
};

// Environment - ρ (rho)
// Linked list representation of run-time environment
// Binds name to location (Name ==> Value*)
struct Env {
    Name name;
    Value *loc;
    Env *tail;
};

#endif
