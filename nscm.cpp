/*============================================================================
 *  nanoscheme
 *  Copyright (c) 2019-2020 - Trung Truong
 *
 *  File name: nscm.cpp
 *
 *  Description: C++ implementation of nanoscheme language
 *  Usage: Run `./nscm --help` for more information.
 * 
 *==========================================================================*/
#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <inttypes.h>
#include <math.h>

/*============================================================================
 *  Enums and constants
 *===========================================================================*/
enum class ExpType { LIT, INT, FLOAT, STRING, LIST, SYMBOL, PROC, PRIM };
enum class PrimType { 
    IF, WHILE, DEFINE, SET,                         // Control flow, var assign
    ADD, SUB, MUL, DIV, MOD, GT, LT, GE, LE,        // Arithmetic operations
    IS_NUM, IS_SYMBOL, IS_PROC, IS_LIST,            // Type check
    LAMBDA,                                         // Lambda expression
    CAR, CDR, CONS                                  // List comprehension
};
enum class LitType { TRUE, FALSE, NIL };

/* Forward-declartion of `Expr` class */
class Expr;

/*============================================================================
 *  Environment class
 *===========================================================================*/
class Env {
private:
    std::unordered_map<std::string, Expr*> frame;
    Env *tail = nullptr;

public:
    Env(std::unordered_map<std::string, Expr*> &f) {
        for (const auto &entry : f) {
            frame[entry.first] = entry.second;
        }
    };
    Env(std::unordered_map<std::string, Expr*> &f, Env *tl) {
        for (const auto &entry : f) {
            frame[entry.first] = entry.second;
        }
        tail = tl;
    };
    ~Env() {};
    void add_key_value_pair(std::string k, Expr *v) {
        frame[k] = v;
    };
    Expr *find_var(std::string name) {
        const auto itr = frame.find(name);
        if (itr != frame.end()) return itr->second;
        else if (itr == frame.end() && tail != nullptr) {
            return this->tail->find_var(name);
        }
        else throw "Unknown identifier: '" + name + "'";
    };
};

/*============================================================================
 *  Expression class
 *===========================================================================*/
class Expr {
private:
    ExpType type;
    union {
        int64_t ival; double fval; std::string sval; LitType lit;
        std::tuple<std::string, Expr*> sym;
        std::tuple<PrimType, std::vector<Expr*> *> prim; 
        std::tuple<Expr*, Expr*, Env*> proc;
    };

public:
    /* Constructors */
    Expr(int64_t i)      { type = ExpType::INT; ival = i; };
    Expr(double f)       { type = ExpType::FLOAT; fval = f; };
    Expr(std::string s)  { type = ExpType::STRING; sval = s; };
    Expr(LitType l)      { type = ExpType::LIT; lit = l; };

    Expr(std::string sym_name, Expr* sym_val) {
        type = ExpType::SYMBOL;
        sym = std::make_tuple(sym_name, sym_val); 
    };
    Expr(PrimType t, std::vector<Expr*> *args) {
        type = ExpType::PRIM;
        prim = std::make_tuple(t, args);
    };
    Expr(Expr *params, Expr *body, Env *env) {
        type = ExpType::PROC;
        proc = std::make_tuple(params, body, env);
    };
    ~Expr()              {};

    /* Evaluators */
    Expr *eval_sym(Env *e);
    Expr *eval_proc(std::vector<Expr*> *bind, Env *e);
    Expr *eval_prim(Env *e);
    Expr *eval(std::vector<Expr*> *bind,Env *e);

    /* IO */
    void print_to_console(void);
    void free_expr(void);
};

/**
 * Evaluate symbol expressions
 * @param e pointer to env
 * @returns Pointer to evaluated expression 
 */
Expr* Expr::eval_sym(Env *e) {
    if (type != ExpType::SYMBOL) throw "Eval failed: Not symbol type!";
    return e->find_var(std::get<0>(sym));
}

/**
 * Evaluate procedure expressions
 * @param bind pointer to vector containing argument bindings
 * @param e pointer to env
 * @returns Pointer to evaluated expression 
 */
Expr* Expr::eval_proc(std::vector<Expr*> *bind, Env *e) {
    if (type != ExpType::PROC) throw "Eval failed: Not procedure type!";
    (void) bind;
    (void) e;
    return nullptr;
}

/**
 * Evaluate primitive expressions
 * @param e pointer to env
 * @returns Pointer to evaluated expression 
 */
Expr* Expr::eval_prim(Env *e) {
    if (type != ExpType::PRIM) throw "Eval failed: Not primitive type!";
    
    PrimType prim_type = std::get<0>(prim);
    std::vector<Expr*> args = *(std::get<1>(prim));

    switch (prim_type) {
        /*======================= Var assign =============================*/
        case PrimType::DEFINE: {
            if (args.size() != 2) throw "Invalid num args for 'define'";
            Expr *name = args[0];
            Expr *value = args[1]->eval(nullptr, e);
            if (name->type == ExpType::STRING) {
                e->add_key_value_pair(name->sval, value);
                return new Expr(name->sval, value);
            }
            else throw "Non-string type variable name for 'define'";
        }
        case PrimType::SET: {
            if (args.size() != 2) throw "Invalid num args for 'set'";
            Expr *name = args[0];
            Expr *value = args[1]->eval(nullptr, e);
            if (name->type == ExpType::STRING) {
                e->find_var(name->sval);
                e->add_key_value_pair(name->sval, value);
                return new Expr(name->sval, value);
            }
            else throw "Non-string type variable name for 'set!'";
        }
        /*======================= Lambda exp =============================*/
        case PrimType::LAMBDA: {
            if (args.size() != 2) throw "Invalid num args for 'lambda'";
            return new Expr(args[0], args[1], e);
        }
        /*======================= Control flow ===========================*/
        /* If statement */
        case PrimType::IF: {
            if (args.size() != 3) throw "Invalid num args for 'if'";
            Expr *cond = args[0]->eval(nullptr, e);
            if (cond->type == ExpType::LIT && cond->lit == LitType::TRUE) 
                return args[1]->eval(nullptr, e);     
            if (cond->type == ExpType::INT && cond->ival > 0)
                return args[1]->eval(nullptr, e);
            if (cond->type == ExpType::FLOAT && cond->fval > 0.0)
                return args[1]->eval(nullptr, e);
            return args[2]->eval(nullptr, e);
        }
        /* While statement */
        case PrimType::WHILE: {
            if (args.size() != 2) throw "Invalid num args for 'while'";
            Expr *cond = args[0]->eval(nullptr, e);
            if (cond->type != ExpType::LIT) throw "Condition not lit type!";
            while (cond->lit == LitType::TRUE) {
                args[1]->eval(nullptr, e);
                cond = args[0]->eval(nullptr, e);
            }
            return nullptr;
        }
        /*======================= Arith operations =======================*/
        /* Integer addition */
        case PrimType::ADD: {
            double s = 0.0;
            for (const auto &arg : args) {
                Expr *exp = arg->eval(nullptr, e);
                if (exp->type == ExpType::INT)          s += exp->ival;
                else if (exp->type == ExpType::FLOAT)   s += exp->fval;
                else throw "Invalid args type for '+'";
            }
            if (std::floor(s) == s) return new Expr(int64_t(s));
            else return new Expr(s);
        }
        /* Integer subtraction */
        case PrimType::SUB: {
            if (args.size() != 2) throw "Invalid num args for '-'";
            Expr *e1 = args[0]->eval(nullptr, e);
            Expr *e2 = args[1]->eval(nullptr, e);

            if (e1->type == ExpType::INT && e2->type == ExpType::INT)
                return new Expr(int64_t(e1->ival - e2->ival)); 
            else if (e1->type == ExpType::FLOAT && e2->type == ExpType::INT)
                return new Expr(e1->fval - e2->ival);
            else if (e1->type == ExpType::INT && e2->type == ExpType::FLOAT)
                return new Expr(e1->ival - e2->fval);
            else if (e1->type == ExpType::FLOAT && e2->type == ExpType::FLOAT)
                return new Expr(e1->fval - e2->fval);
            else throw "Invalid args type for '-'";
        }
        /* Integer multiplication */
        case PrimType::MUL: {
            double p = 1.0;
            for (const auto &arg : args) {
                Expr *exp = arg->eval(nullptr, e);
                if (exp->type == ExpType::INT)          p *= exp->ival;
                else if (exp->type == ExpType::FLOAT)   p *= exp->fval;
                else throw "Invalid args type for '*'";
            }
            if (std::floor(p) == p) return new Expr(int64_t(p));
            else return new Expr(p);
        }
        /* Integer division */
        case PrimType::DIV: {
            if (args.size() != 2) throw "Invalid num args for '/'";
            Expr *e1 = args[0]->eval(nullptr, e);
            Expr *e2 = args[1]->eval(nullptr, e);

            if ((e2->type == ExpType::INT && e2->ival == 0) ||
                (e2->type == ExpType::FLOAT && e2->fval == 0)) 
                throw "Division by zero";

            if (e1->type == ExpType::INT && e2->type == ExpType::INT)
                return new Expr(int64_t(e1->ival / e2->ival)); 
            else if (e1->type == ExpType::FLOAT && e2->type == ExpType::INT)
                return new Expr(e1->fval / e2->ival);
            else if (e1->type == ExpType::INT && e2->type == ExpType::FLOAT)
                return new Expr(e1->ival / e2->fval);
            else if (e1->type == ExpType::FLOAT && e2->type == ExpType::FLOAT)
                return new Expr(e1->fval / e2->fval);
            else throw "Invalid args type for '/'";
        }
        /* Integer modulo */
        case PrimType::MOD: {
            if (args.size() != 2) throw "Invalid num args for 'modulo'";
            Expr *e1 = args[0]->eval(nullptr, e);
            Expr *e2 = args[1]->eval(nullptr, e);
                    
            if ((e2->type == ExpType::INT && e2->ival == 0) ||
                (e2->type == ExpType::FLOAT && e2->fval == 0)) 
                throw "Division by zero";
            
            if (e1->type == ExpType::INT && e2->type == ExpType::INT)
                return new Expr(e1->ival % e2->ival);
            else throw "Invalid args type for 'modulo'";
        }
        /*======================= Type checking ===========================*/

        /*======================= Invalid primative =======================*/
        default: throw "Invalid primitive'";
    }
};

/**
 * Evaluate expression generic function. Delegate expression evaluation to 
 * class function handling each data type.
 * @param bind pointer to vector containing argument bindings
 * @param e pointer to env
 * @returns Pointer to evaluated expression 
 */
Expr* Expr::eval(std::vector<Expr*> *bind, Env *e) {
    switch (type) {
        case ExpType::INT:      return this;
        case ExpType::FLOAT:    return this;
        case ExpType::STRING:   return this;
        case ExpType::LIT:      return this;
        case ExpType::PRIM:     return eval_prim(e);
        case ExpType::SYMBOL:   return eval_sym(e);
        case ExpType::PROC:     return eval_proc(bind, e);
        default:                return nullptr;
    }
};

/**
 * Print expression to stdout
 * @returns void
 */
void Expr::print_to_console(void) {
    switch (type) {
        case ExpType::INT:     { std::cout << ival << "\n";         break; }
        case ExpType::FLOAT:   { std::cout << fval << "\n";         break; }
        case ExpType::PROC:    { std::cout << "<procedure>\n";      break; }
        case ExpType::STRING:  { std::cout << "'" << sval << "'\n"; break; }
        case ExpType::PRIM:
        case ExpType::SYMBOL:  { std::get<1>(sym)->print_to_console(); break; }
        case ExpType::LIT:     {
            switch (lit) {
                case LitType::TRUE:     std::cout<< "#t\n"; break;
                case LitType::FALSE:    std::cout<< "#f\n"; break;
                case LitType::NIL:      std::cout<< "()\n"; break;
                default: break;
            }
        }
        default:               break;
    }
}

/* TODO: free expression when deleted */
void Expr::free_expr(void) {

}

/*============================================================================
 *  Parser implementation
 *===========================================================================*/
Expr *tokenize(std::string) {
    return nullptr;
}

/*============================================================================
 *  Main driver
 *===========================================================================*/
int main(int argc, char*argv[]) {
    (void) argc;
    (void) argv;
    std::unordered_map<std::string, Expr*> std_env_frame = {};
    Env global_env = Env(std_env_frame);

    Expr* str = new Expr("hello world");
    Expr* x = new Expr(int64_t(10));
    Expr* y = new Expr(int64_t(2));
    Expr* z = new Expr(9.5);
    std::vector<Expr*> mul_args = {x, y};
    Expr *mul_x_y = new Expr(PrimType::MUL, &mul_args);
    
    std::vector<Expr*> add_z_args = {mul_x_y, z};
    Expr *res = new Expr(PrimType::ADD, &add_z_args);

    std::vector<Expr*> define_var = {str, res};
    Expr *def = new Expr(PrimType::DEFINE, &define_var);

    std::vector<Expr*> set_var = {str, x};
    Expr *set = new Expr(PrimType::SET, &set_var);

    try {
        (res->eval(nullptr, &global_env))->print_to_console();
        (str->eval(nullptr, &global_env))->print_to_console();
        (def->eval(nullptr, &global_env))->print_to_console();
        (set->eval(nullptr, &global_env))->print_to_console();
    }
    catch (const char* e) {
        std::cerr << "ERR: " << e << "\n";
        exit(EXIT_FAILURE);
    }
    catch (const std::string &e) {
        std::cerr << "ERR: " << e << "\n";
        exit(EXIT_FAILURE);
    }

    delete x;
    delete y;
    delete z;
    delete mul_x_y;
    delete res;

    return 0;
}
