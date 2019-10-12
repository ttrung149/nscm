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
 *  File name: test-ast.c
 *
 *  Description: Test functions that create Abstract Syntax Tree. 
 *
 *------------------------------------------------------------------------*/

#include "../src/all.h"
#include "../src/ast.h"

int main() {
    // EXP: Test for mkLit
    Value numVal = mkNum(1000);
    Exp litExp = mkLit(numVal);

    assert(litExp->type == LITERAL);
    assert(litExp->u.literal.type == NUM);
    assert(litExp->u.literal.u.num == 1000);

    free(litExp);
    printf("%s\n", "Test for mkLit passed!");

    // EXP: Test for mkVar

    // EXP: Test for mkSet

    // EXP: Test for mkIfx

    // EXP: Test for mkWhileX
    return 0;
}
