#------------------------------------------------------------------------
#  nanoscheme
#  Copyright (c) 2019-2020 - Trung Truong
# 
#  File name: Makefile
#  Description: Create executables
#------------------------------------------------------------------------

# Compiler Flags
CC			= g++
CFLAGS      = -g -std=c++11 -pedantic -Wall -Werror -Wextra \
              -Wno-overlength-strings -Wfatal-errors -pedantic
LDFLAGS     = -g
CPPFLAGS    = -I.
RM          = rm -f

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean: 
	rm -rf src/*.o core* nscm

nscm: src/env.o src/expr.o src/parser.o src/nscm.o
	$(CC) $(CFLAGS) -o nscm src/env.o src/expr.o src/parser.o src/nscm.o