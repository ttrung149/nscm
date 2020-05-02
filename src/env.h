/*============================================================================
 *  nanoscheme
 *  Copyright (c) 2019-2020 - Trung Truong
 *
 *  File name: env.h
 *  Description: Header file for `Env` class
 * 
 *==========================================================================*/
#include <string>
#include <unordered_map>
#ifndef ENV_H_
#define ENV_H_

/* Forward-declartion of `Expr` class */
class Expr;

/*============================================================================
 *  Environment class
 *===========================================================================*/
class Env {
private:
    std::unordered_map<std::string, Expr*> frame;
    Env *tail;

public:
    /* Constructors */
    Env(std::unordered_map<std::string, Expr*> &f);
    Env(std::unordered_map<std::string, Expr*> &f, Env *tl);
    Env(Env *tl);
    ~Env();

    /* Env state modifiers  */
    Env *get_tl();
    void add_key_value_pair(std::string &k, Expr *v);
    Expr *find_var(std::string name);
    Expr *find_var_in_frame(std::string name);
};

#endif
