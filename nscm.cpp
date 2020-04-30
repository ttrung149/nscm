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
#define NO_BINDING nullptr

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
    Env *tail;

public:
    /* Constructors */
    Env(std::unordered_map<std::string, Expr*> &f)
        :frame(f), tail(nullptr) {};
    Env(std::unordered_map<std::string, Expr*> &f, Env *tl)
        :frame(f), tail(tl) {};
    Env(Env *tl) { frame = {}; tail = tl; };
    ~Env() {};

    /* Env state modifiers  */
    Env *get_tl() { return tail; };
    void add_key_value_pair(std::string &k, Expr *v) {
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
    Expr *find_var_in_frame(std::string name) {
        const auto itr = frame.find(name);
        if (itr != frame.end()) return itr->second; else return nullptr;
    }
};

/*============================================================================
 *  Expression class
 *===========================================================================*/
class Expr {
private:
    ExpType type;
    union {
        int64_t ival; double fval; std::string sval = ""; LitType lit;
        std::vector<Expr*> *list;
        std::tuple<std::string, Expr*> sym;
        std::tuple<PrimType, std::vector<Expr*> *> prim; 
        std::tuple<Expr*, Expr*, Env*> proc;
    };

public:
    /* Constructors */
    Expr(int64_t i)             : type(ExpType::INT),    ival(i) {};
    Expr(double f)              : type(ExpType::FLOAT),  fval(f) {};
    Expr(std::string s)         : type(ExpType::STRING), sval(s) {};
    Expr(LitType l)             : type(ExpType::LIT),    lit(l)  {};
    Expr(std::vector<Expr*> *l) : type(ExpType::LIST),   list(l) {};

    Expr(std::string sym_name, Expr *sym_val)
        : type(ExpType::SYMBOL), sym(std::make_tuple(sym_name, sym_val)) {}; 
    Expr(PrimType t, std::vector<Expr*> *args) 
        : type(ExpType::PRIM), prim(std::make_tuple(t, args)) {};
    Expr(Expr *params, Expr *body, Env *env)
        : type(ExpType::PROC), proc(std::make_tuple(params, body, env)) {};
    ~Expr() {};

    /* Copy constructor */
    Expr(const Expr &e) {
        type = e.type;
        switch (e.type) {
            case ExpType::INT:      { ival = e.ival; break; }
            case ExpType::FLOAT:    { fval = e.fval; break; }
            case ExpType::STRING:   { sval = e.sval; break; }
            case ExpType::LIT:      { lit  = e.lit;  break; }
            case ExpType::LIST:     { list = e.list; break; }
            case ExpType::SYMBOL:   { sym  = e.sym;  break; }
            case ExpType::PRIM:     { prim = e.prim; break; }
            case ExpType::PROC:     { proc = e.proc; break; }
            default:                                 break;
        }
    };

    /* Evaluators */
    Expr eval_sym(Env *e);
    Expr eval_proc(std::vector<Expr*> *bindings);
    Expr eval_prim(Env *e);
    Expr eval(std::vector<Expr*> *bindings, Env *e);

    /* IO */
    void print_to_console(void);
    void free_expr(void);
};

/**
 * Evaluate symbol expressions
 * @param e pointer to env
 * @returns evaluated expression 
 */
Expr Expr::eval_sym(Env *e) {
    if (type != ExpType::SYMBOL) throw "Eval failed: Not symbol type!";
    std::string name = std::get<0>(sym);
    return e->find_var(name)->eval(NO_BINDING, e);
}

/**
 * Evaluate procedure expressions
 * @param bindings pointer to vector containing argument bindings
 * @returns evaluated expression 
 */
Expr Expr::eval_proc(std::vector<Expr*> *bindings) {
    if (type != ExpType::PROC) throw "Eval failed: Not procedure type!";
    Expr *params = std::get<0>(proc);
    Expr *body   = std::get<1>(proc);
    Env  *env    = std::get<2>(proc);

    Env *new_env = new Env(env->get_tl());
    if (bindings->size() != params->list->size()) 
        throw "Number of args does not match!";
    
    for (size_t i = 0; i < bindings->size(); i++) {
        Expr *param = params->list->at(i);
        if (param->type != ExpType::STRING) throw "Non-string typed argument";
        Expr *value = new Expr(bindings->at(i)->eval(NO_BINDING, env));
        new_env->add_key_value_pair(param->sval, value);
    }
    return body->eval(NO_BINDING, new_env);
}

/**
 * Evaluate primitive expressions
 * @param e pointer to env
 * @returns evaluated expression 
 */
Expr Expr::eval_prim(Env *e) {
    if (type != ExpType::PRIM) throw "Eval failed: Not primitive type!"; 
    PrimType prim_type = std::get<0>(prim);
    std::vector<Expr*> args = *std::get<1>(prim);

    switch (prim_type) {
        /*======================= Var assign =============================*/
        case PrimType::DEFINE: {
            if (args.size() != 2) throw "Invalid num args for 'define'";
            Expr name  = args[0]->eval(NO_BINDING, e);
            Expr value = *args[1];
            
            Expr *defined_value = new Expr(value);
            if (name.type == ExpType::STRING) {
                e->add_key_value_pair(name.sval, defined_value);
                return Expr(name.sval, defined_value);
            }
            else throw "Non-string type variable name for 'define'";
        }
        case PrimType::SET: {
            if (args.size() != 2) throw "Invalid num args for 'set'";
            Expr name  = args[0]->eval(NO_BINDING, e);
            Expr value = *args[1];
            
            if (name.type == ExpType::STRING) {
                Expr *var = e->find_var_in_frame(name.sval);
                if (var == nullptr) throw "Set failed: Unknown identifier";
                delete var;
                Expr *set_value = new Expr(value);
                return Expr(name.sval, set_value);
            }
            else throw "Non-string type variable name for 'set!'";
        }
        /*======================= Lambda exp =============================*/
        case PrimType::LAMBDA: {
            if (args.size() != 2) throw "Invalid num args for 'lambda'";
            if (args[0]->type != ExpType::LIST) throw "Non-list typed args";
            return Expr(args[0], args[1], e);
        }
        /*======================= Control flow ===========================*/
        /* If statement */
        case PrimType::IF: {
            if (args.size() != 3) throw "Invalid num args for 'if'";
            Expr cond = args[0]->eval(NO_BINDING, e);
            if (cond.type == ExpType::LIT && cond.lit == LitType::TRUE) 
                return args[1]->eval(NO_BINDING, e);     
            if (cond.type == ExpType::INT && cond.ival > 0)
                return args[1]->eval(NO_BINDING, e);
            if (cond.type == ExpType::FLOAT && cond.fval > 0.0)
                return args[1]->eval(NO_BINDING, e);
            return args[2]->eval(NO_BINDING, e);
        }
        /* While statement */
        case PrimType::WHILE: {
            if (args.size() != 2) throw "Invalid num args for 'while'";
            Expr cond = args[0]->eval(NO_BINDING, e);
            if (cond.type != ExpType::LIT) throw "Condition not lit type!";

            while (args[0]->eval(NO_BINDING, e).lit == LitType::TRUE) {
                args[1]->eval(NO_BINDING, e);
            }
            return nullptr;
        }
        /*======================= Arith operations =======================*/
        /* Integer addition */
        case PrimType::ADD: {
            double s = 0.0;
            for (const auto &arg : args) {
                Expr exp = arg->eval(NO_BINDING, e);
                if (exp.type == ExpType::INT)          s += exp.ival;
                else if (exp.type == ExpType::FLOAT)   s += exp.fval;
                else throw "Invalid args type for '+'";
            }
            if (std::floor(s) == s) return Expr(int64_t(s));
            else return Expr(s);
        }
        /* Integer subtraction */
        case PrimType::SUB: {
            if (args.size() != 2) throw "Invalid num args for '-'";
            Expr e1 = args[0]->eval(NO_BINDING, e);
            Expr e2 = args[1]->eval(NO_BINDING, e);

            if (e1.type == ExpType::INT && e2.type == ExpType::INT)
                return Expr(int64_t(e1.ival - e2.ival)); 
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::INT)
                return Expr(e1.fval - e2.ival);
            else if (e1.type == ExpType::INT && e2.type == ExpType::FLOAT)
                return Expr(e1.ival - e2.fval);
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::FLOAT)
                return Expr(e1.fval - e2.fval);
            else throw "Invalid args type for '-'";
        }
        /* Integer multiplication */
        case PrimType::MUL: {
            double p = 1.0;
            for (const auto &arg : args) {
                Expr exp = arg->eval(NO_BINDING, e);
                if (exp.type == ExpType::INT)          p *= exp.ival;
                else if (exp.type == ExpType::FLOAT)   p *= exp.fval;
                else throw "Invalid args type for '*'";
            }
            if (std::floor(p) == p) return Expr(int64_t(p));
            else return Expr(p);
        }
        /* Integer division */
        case PrimType::DIV: {
            if (args.size() != 2) throw "Invalid num args for '/'";
            Expr e1 = args[0]->eval(NO_BINDING, e);
            Expr e2 = args[1]->eval(NO_BINDING, e);

            if ((e2.type == ExpType::INT && e2.ival == 0) ||
                (e2.type == ExpType::FLOAT && e2.fval == 0)) 
                throw "Division by zero";

            if (e1.type == ExpType::INT && e2.type == ExpType::INT)
                return Expr(int64_t(e1.ival / e2.ival)); 
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::INT)
                return Expr(e1.fval / e2.ival);
            else if (e1.type == ExpType::INT && e2.type == ExpType::FLOAT)
                return Expr(e1.ival / e2.fval);
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::FLOAT)
                return Expr(e1.fval / e2.fval);
            else throw "Invalid args type for '/'";
        }
        /* Integer modulo */
        case PrimType::MOD: {
            if (args.size() != 2) throw "Invalid num args for 'modulo'";
            Expr e1 = args[0]->eval(NO_BINDING, e);
            Expr e2 = args[1]->eval(NO_BINDING, e);

            if ((e2.type == ExpType::INT && e2.ival == 0) ||
                (e2.type == ExpType::FLOAT && e2.fval == 0)) 
                throw "Division by zero";
            
            if (e1.type == ExpType::INT && e2.type == ExpType::INT)
                return Expr(e1.ival % e2.ival);
            else throw "Invalid args type for 'modulo'";
        }
        /*======================= Type checking ===========================*/

        /*======================= Invalid primative =======================*/
        default: throw "Invalid primitive";
    }
};

/**
 * Evaluate generic expression. Delegate evaluation to data-type specific 
 * eval function.
 * @param bindings pointer to vector containing argument bindings
 * @param e pointer to env
 * @returns evaluated expression 
 */
Expr Expr::eval(std::vector<Expr*> *bindings, Env *e) {
    switch (type) {
        case ExpType::INT:      return Expr(this->ival);
        case ExpType::FLOAT:    return Expr(this->fval);
        case ExpType::STRING:   return Expr(this->sval);
        case ExpType::LIST:     return Expr(this->list);
        case ExpType::LIT:      return Expr(this->lit) ;
        case ExpType::PRIM:     return eval_prim(e);
        case ExpType::SYMBOL:   return eval_sym(e);
        case ExpType::PROC:     return eval_proc(bindings);
        default:                throw "Eval failed: Unknown token type";
    }
};

/**
 * Print expression to stdout
 * @returns void
 */
void Expr::print_to_console(void) {
    switch (type) {
        case ExpType::INT:     { std::cout << ival;               break; }
        case ExpType::FLOAT:   { std::cout << fval;               break; }
        case ExpType::PROC:    { std::cout << "<procedure>";      break; }
        case ExpType::PRIM:    { std::cout << "<primitive>";      break; }
        case ExpType::STRING:  { std::cout << "'" << sval << "'"; break; }
        case ExpType::SYMBOL:  { std::get<1>(sym)->print_to_console(); break; }
        case ExpType::LIT:     {
            switch (lit) {
                case LitType::TRUE:     std::cout<< "#t"; break;
                case LitType::FALSE:    std::cout<< "#f"; break;
                case LitType::NIL:      std::cout<< "()"; break;
                default: break;
            }
        }
        case ExpType::LIST:     {
            std::cout << "( ";
            for (const auto &elem : *list) {
                elem->print_to_console(); std::cout << " ";
            }
            std::cout << ")";
            break;
        }
        default:               break;
    }
}

/*============================================================================
 *  Parser implementation
 *===========================================================================*/

/*============================================================================
 *  Main driver
 *===========================================================================*/
int main(int argc, char*argv[]) {
    (void) argc;
    (void) argv;
    std::unordered_map<std::string, Expr*> std_env_frame = {};
    Env global_env(std_env_frame);

    // ((lambda (x y) (/ x y)) 10 2) = 5
    Expr str_x("x");                Expr str_y("y");
    Expr sym_x("x", nullptr);       Expr sym_y("y", nullptr);

    std::vector<Expr*> args_vec = {&str_x, &str_y};
    Expr args_list(&args_vec);

    std::vector<Expr*> div = {&sym_x, &sym_y};
    Expr div_x_y_op(PrimType::DIV, &div);

    std::vector<Expr*> lambda_div_x_y = {&args_list, &div_x_y_op};
    Expr function(PrimType::LAMBDA, &lambda_div_x_y);

    Expr _x(int64_t(10)); Expr _y(3.0);
    std::vector<Expr*> bindings = {&_x, &_y};

    try {
        function.eval(NO_BINDING, &global_env)
                .eval(&bindings, &global_env)
                .print_to_console();
        std::cout << "\n";
    }
    catch (const char* e) {
        std::cerr << "ERR: " << e << "\n";
        exit(EXIT_FAILURE);
    }
    catch (const std::string &e) {
        std::cerr << "ERR: " << e << "\n";
        exit(EXIT_FAILURE);
    }

    return 0;
}
