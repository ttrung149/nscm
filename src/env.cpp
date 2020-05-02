 /*============================================================================
 *  nanoscheme
 *  Copyright (c) 2019-2020 - Trung Truong
 *
 *  File name: env.cpp
 *  Description: Implementation of `Env` class
 * 
 *==========================================================================*/
 #include "env.h"

 /* Constructors */
Env::Env(std::unordered_map<std::string, Expr*> &f)
    :frame(f), tail(nullptr) {};
Env::Env(std::unordered_map<std::string, Expr*> &f, Env *tl)
    :frame(f), tail(tl) {};
Env::Env(Env *tl) { frame = {}; tail = tl; };

/* Destructor */
Env::~Env() {};

/* Env state modifiers  */
Env *Env::get_tl() { 
    return tail; 
};

void Env::add_key_value_pair(std::string &k, Expr *v) {
    frame[k] = v;
};

Expr* Env::find_var(std::string name) {
    const auto itr = frame.find(name);
    if (itr != frame.end()) return itr->second;
    else if (itr == frame.end() && tail != nullptr) {
        return this->tail->find_var(name);
    }
    else throw "Unknown identifier: '" + name + "'";
};

Expr* Env::find_var_in_frame(std::string name) {
    const auto itr = frame.find(name);
    if (itr != frame.end()) return itr->second; else return nullptr;
}
