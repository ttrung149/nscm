/*============================================================================
 *  nanoscheme
 *  Copyright (c) 2019-2020 - Trung Truong
 *
 *  File name: expr.cpp
 *  Description: Implementation of `Expr` class
 * 
 *==========================================================================*/
#include "expr.h"

/*============================================================================
 *  Constructors
 *===========================================================================*/
Expr::Expr(int64_t i)            : type(ExpType::INT),    ival(i) {}
Expr::Expr(double f)             : type(ExpType::FLOAT),  fval(f) {}
Expr::Expr(std::string s)        : type(ExpType::STRING), sval(s) {}
Expr::Expr(LitType l)            : type(ExpType::LIT),    lit(l)  {}
Expr::Expr(std::vector<Expr*> *l): type(ExpType::LIST),   list(l) {}  

Expr::Expr(std::string sym_name, Expr *sym_val)
    : type(ExpType::SYMBOL), sym(std::make_tuple(sym_name, sym_val)) {}
Expr::Expr(PrimType t, std::vector<Expr*> *args) 
    : type(ExpType::PRIM), prim(std::make_tuple(t, args)) {}
Expr::Expr(Expr *params, Expr *body, Env *env)
    : type(ExpType::PROC), proc(std::make_tuple(params, body, env)) {}
Expr::~Expr() {}

/* Copy constructor */
Expr::Expr(const Expr &e) {
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
}

/*============================================================================
 *  Getters
 *===========================================================================*/
ExpType Expr::get_expr_type(void) {
    return type;
}
PrimType Expr::get_prim_type(void) { 
    if (type != ExpType::PRIM) throw "Instance is not primitive type";
    else return std::get<0>(prim);
}

/*============================================================================
 *  Evaluators
 *===========================================================================*/
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

            while (args[0]->eval(NO_BINDING, e).lit == LitType::TRUE)
                args[1]->eval(NO_BINDING, e);
            
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
        case ExpType::INT:      return *this;
        case ExpType::FLOAT:    return *this;
        case ExpType::STRING:   return *this;
        case ExpType::LIST:     return *this;
        case ExpType::LIT:      return *this;
        case ExpType::PRIM:     return eval_prim(e);
        case ExpType::SYMBOL:   return eval_sym(e);
        case ExpType::PROC:     return eval_proc(bindings);
        default:                throw "Eval failed: Unknown token type";
    }
};

/*============================================================================
 *  IOs
 *===========================================================================*/
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
        default: break;
    }
}