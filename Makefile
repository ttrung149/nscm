#------------------------------------------------------------------------
#
#  tiny-lisp-interpreter
#  
#  Copyright (c) 2019 - Trung Truong
#  All rights reserved.

#  Permission is hereby granted, free of charge, to any person 
#  obtaining a copy of this software and associated documentation 
#  files (the "Software"), to deal in the Software without restriction,
#  including without limitation the rights to use, copy, modify, merge,
#  publish, distribute, sublicense, and/or sell copies of the Software,
#  and to permit persons to whom the Software is furnished to do so, 
#  subject to the following conditions:
#  
#  The above copyright notice and this permission notice shall be included 
#  in all copies or substantial portions of the Software.
#  
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
#  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
#  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY 
#  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
#  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
#  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
# 
#  File name: Makefile
#
#  Description: Create executables
#------------------------------------------------------------------------

HEADERS  = src/all.h

OBJECTS  = $(SOURCES:.c=.o)
RESULT   = tinylisp

#  Compiler Flags
CC 			= gcc 
CFLAGS      = -g -std=c99 -pedantic -Wall -Werror -Wextra \
              -Wno-overlength-strings -Wfatal-errors -pedantic
LDFLAGS     = -g
CPPFLAGS    = -I.
RM          = rm -f 

.SUFFIXES:
.SUFFIXES: .c .o

$(RESULT): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $(OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(RM) $(RESULT) tests/*.o src/*.o

# Unit tests
test-ast: src/ast.o tests/test-ast.o
	$(CC) $(CFLAGS) -o tests/test-ast src/ast.o tests/test-ast.o

test-ast.o: src/ast.c $(HEADERS) src/ast.h tests/test-ast.c
	$(CC) $(CFLAGS) -c src/ast.c tests/test-ast.c