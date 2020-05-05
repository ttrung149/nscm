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
 * @param bindings pointer to vector containing argument bindings
 * @param e pointer to env
 * @returns evaluated expression 
 */
Expr Expr::eval_sym(std::vector<Expr*> *bindings, Env *e) {
    if (type != ExpType::SYMBOL) throw "Eval failed: Not symbol type!";

    Expr *found_val = e->find_var(std::get<0>(sym));
    if (found_val != nullptr)
        return found_val->eval(bindings, e);
    else     
        throw "Unknown identifier: '" + std::get<0>(sym) + "'";
}

/**
 * Evaluate procedure expressions
 * @param bindings pointer to vector containing argument bindings
 * @param e pointer to env
 * @returns evaluated expression 
 */
Expr Expr::eval_proc(std::vector<Expr*> *bindings, Env *e) {
    if (type != ExpType::PROC) throw "Eval failed: Not procedure type!";
    
    Expr *params = std::get<0>(proc);
    Expr *body   = std::get<1>(proc);
    Env  *env    = std::get<2>(proc);

    if (bindings == nullptr || bindings->size() != params->list->size()) 
        throw "Non-matching number of args for procedure call";
    Env *new_env = new Env(env);

    /** 
     * For recursive function, function body is first initialized as 
     * an unbounded symbol. To evaluate recursive function, the function 
     * body must be evaluated, and then applied to the evaluated parameter
     * For example, consider `fact` function that recursively calls itself
     * to calculate the factorial:
     * 
     * ```(define fact (lambda (n) (if (< n 2) 1 (* n (fact (- n 1))))))```
     * ```(fact 10)```
     * 
     * When (fact (- n 1)) is parsed, `fact` is initialized as an unbounded 
     * symbol, `(- n 1)` is initalized as a prim. When (fact 10) is evaluated,
     * `fact` is evaluated to match symbol to defined procedure. Furthermore,
     * (- n 1) must also be evaluated to get the necessary binding for next
     * procedure call. Lastly, the value of `n` must be updated in the
     * environment to ensure the recursive function terminates.
     */
    if (body->get_expr_type() == ExpType::SYMBOL) {
        // symbol -> lambda -> procedure
        Expr caller = body->eval(bindings, e);
        if (caller.get_expr_type() != ExpType::PROC)
            throw "Eval failed: Not procedure type!";
        
        Expr *_params = std::get<0>(caller.proc);
        Expr *_body   = std::get<1>(caller.proc);
        std::vector<Expr*> eval_params_list = {};
        
        for (size_t i = 0; i < params->list->size(); i++) {
            Expr *_param = _params->list->at(i);
            if (_param->type != ExpType::STRING)
                throw "Non-string typed argument";
            
            Expr *eval_param 
                = new Expr(params->list->at(i)->eval(bindings, e));

            new_env->add_key_value_pair(_param->sval, eval_param);
            eval_params_list.push_back(eval_param);
        }
        return _body->eval(&eval_params_list, new_env);
    }

    /**
     * Non-recursive procedure call case
     * To evaluate non-recursive procedure call, evaluate params and add
     * a param to binding pair to the new environment frame. Evaluate 
     * function body in such new environment to obtain the procedure call
     * result.
     */
    for (size_t i = 0; i < bindings->size(); i++) {
        Expr *param = params->list->at(i);
        if (param->type != ExpType::STRING) throw "Non-string typed argument";
        Expr *value = new Expr(bindings->at(i)->eval(bindings, env));
        new_env->add_key_value_pair(param->sval, value);
    }
    return body->eval(bindings, new_env);
}

/**
 * Evaluate primitive expressions
 * @param bindings pointer to vector containing argument bindings
 * @param e pointer to env
 * @returns evaluated expression 
 */
Expr Expr::eval_prim(std::vector<Expr*> *bindings, Env *e) {
    if (type != ExpType::PRIM) throw "Eval failed: Not primitive type!"; 
    PrimType prim_type = std::get<0>(prim);
    std::vector<Expr*> args = *std::get<1>(prim);

    switch (prim_type) {
        /*======================= Var assign =============================*/
        case PrimType::DEFINE: {
            if (args.size() != 2) throw "Invalid num args for 'define'";
            Expr name = args[0]->eval(bindings, e);

            // Bind variable name to an expression in environment
            if (name.type == ExpType::STRING) {
                e->add_key_value_pair(name.sval, args[1]);
                return Expr(LitType::NIL);
            }
            else throw "Non-string type variable name for 'define'";
        }
        case PrimType::SET: {
            if (args.size() != 2) throw "Invalid num args for 'set'";
            Expr name = args[0]->eval(bindings, e);
            
            if (name.type == ExpType::STRING) {
                if (!e->is_in_env(name.sval)) 
                    throw "Unbounded variable '" + name.sval + "'";
                
                // Re-bind variable name to a new expression in env
                e->add_key_value_pair(name.sval, args[1]);
                return Expr(LitType::NIL);
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
            Expr cond = args[0]->eval(bindings, e);
            if (cond.type == ExpType::LIT && cond.lit == LitType::TRUE) 
                return args[1]->eval(bindings, e);     
            if (cond.type == ExpType::INT && cond.ival > 0)
                return args[1]->eval(bindings, e);
            if (cond.type == ExpType::FLOAT && cond.fval > 0.0)
                return args[1]->eval(bindings, e);
            return args[2]->eval(bindings, e);
        }
        /*======================= Arith operations =======================*/
        /* Integer addition */
        case PrimType::ADD: {
            double s = 0.0;
            for (const auto &arg : args) {
                Expr exp = arg->eval(bindings, e);
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
            Expr e1 = args[0]->eval(bindings, e);
            Expr e2 = args[1]->eval(bindings, e);

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
                Expr exp = arg->eval(bindings, e);
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
            Expr e1 = args[0]->eval(bindings, e);
            Expr e2 = args[1]->eval(bindings, e);

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
            Expr e1 = args[0]->eval(bindings, e);
            Expr e2 = args[1]->eval(bindings, e);

            if ((e2.type == ExpType::INT && e2.ival == 0) ||
                (e2.type == ExpType::FLOAT && e2.fval == 0)) 
                throw "Division by zero";
            
            if (e1.type == ExpType::INT && e2.type == ExpType::INT)
                return Expr(e1.ival % e2.ival);
            else throw "Invalid args type for 'modulo'";
        }
        /*======================= Comparators =============================*/
        /* Greater than */
        case PrimType::GT: {
            if (args.size() != 2) throw "Invalid num args for '>'";
            Expr e1 = args[0]->eval(bindings, e);
            Expr e2 = args[1]->eval(bindings, e);

            if (e1.type == ExpType::INT && e2.type == ExpType::INT)
                if (e1.ival > e2.ival) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::INT)
                if (e1.fval > e2.ival) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::INT && e2.type == ExpType::FLOAT)
                if (e1.ival > e2.fval) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::FLOAT)
                if (e1.fval > e2.fval) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else throw "Invalid args type for '>'";
        }
        /* Less than */
        case PrimType::LT: {
            if (args.size() != 2) throw "Invalid num args for '<'";
            Expr e1 = args[0]->eval(bindings, e);
            Expr e2 = args[1]->eval(bindings, e);

            if (e1.type == ExpType::INT && e2.type == ExpType::INT)
                if (e1.ival < e2.ival) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::INT)
                if (e1.fval < e2.ival) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::INT && e2.type == ExpType::FLOAT)
                if (e1.ival < e2.fval) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::FLOAT)
                if (e1.fval < e2.fval) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else throw "Invalid args type for '<'";
        }
        /* Greater or equal than */
        case PrimType::GE: {
            if (args.size() != 2) throw "Invalid num args for '>='";
            Expr e1 = args[0]->eval(bindings, e);
            Expr e2 = args[1]->eval(bindings, e);

            if (e1.type == ExpType::INT && e2.type == ExpType::INT)
                if (e1.ival >= e2.ival) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::INT)
                if (e1.fval >= e2.ival) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::INT && e2.type == ExpType::FLOAT)
                if (e1.ival >= e2.fval) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::FLOAT)
                if (e1.fval >= e2.fval) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else throw "Invalid args type for '>='";
        }
        /* Less or equal than */
        case PrimType::LE: {
            if (args.size() != 2) throw "Invalid num args for '<='";
            Expr e1 = args[0]->eval(bindings, e);
            Expr e2 = args[1]->eval(bindings, e);

            if (e1.type == ExpType::INT && e2.type == ExpType::INT)
                if (e1.ival <= e2.ival) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::INT)
                if (e1.fval <= e2.ival) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::INT && e2.type == ExpType::FLOAT)
                if (e1.ival <= e2.fval) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else if (e1.type == ExpType::FLOAT && e2.type == ExpType::FLOAT)
                if (e1.fval <= e2.fval) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            else throw "Invalid args type for '<='";
        }
        /*======================= Type checking ===========================*/
        /* number? */
        case PrimType::IS_NUM: {
            if (args.size() != 1) throw "Invalid num args for 'number?'";
            Expr e1 = args[0]->eval(bindings, e);
            if (e1.type == ExpType::INT || e1.type == ExpType::FLOAT)
                return Expr(LitType::TRUE);
            else return Expr(LitType::FALSE);
        }
        /* symbol? */
        case PrimType::IS_SYM: {
            if (args.size() != 1) throw "Invalid num args for 'symbol?'";
            Expr e1 = args[0]->eval(bindings, e);
            if (e1.type == ExpType::SYMBOL)
                return Expr(LitType::TRUE);
            else return Expr(LitType::FALSE);
        }
        /* list? */
        case PrimType::IS_LIST: {
            if (args.size() != 1) throw "Invalid num args for 'list?'";
            Expr e1 = args[0]->eval(bindings, e);
            if (e1.type == ExpType::LIST)
                return Expr(LitType::TRUE);
            else return Expr(LitType::FALSE);
        }
        /* procedure? */
        case PrimType::IS_PROC: {
            if (args.size() != 1) throw "Invalid num args for 'procedure?'";
            Expr e1 = args[0]->eval(bindings, e);
            if (e1.type == ExpType::PROC)
                return Expr(LitType::TRUE);
            else return Expr(LitType::FALSE);
        }
        /* boolean? */
        case PrimType::IS_BOOL: {
            if (args.size() != 1) throw "Invalid num args for 'boolean?'";
            Expr e1 = args[0]->eval(bindings, e);
            if (e1.type == ExpType::LIT)
                return Expr(LitType::TRUE);
            else return Expr(LitType::FALSE);
        }
        /* string? */
        case PrimType::IS_STR: {
            if (args.size() != 1) throw "Invalid num args for 'string?'";
            Expr e1 = args[0]->eval(bindings, e);
            if (e1.type == ExpType::STRING)
                return Expr(LitType::TRUE);
            else return Expr(LitType::FALSE);
        }
        /*======================= List operations =========================*/
        /* car */
        case PrimType::CAR: {
            if (args.size() != 1) throw "Invalid num args for 'car'";
            Expr e1 = args[0]->eval(bindings, e);
            if (e1.type == ExpType::LIST) {
                std::vector<Expr*> l = *e1.list;
                if (l.size() == 0) return Expr(LitType::NIL);
                else return Expr(l[0]->eval(bindings, e)); 
            }
            else throw "Argument for 'car' is not list type"; ;
        }
        /* cdr */
        case PrimType::CDR: {
            if (args.size() != 1) throw "Invalid num args for 'cdr'";
            Expr e1 = args[0]->eval(bindings, e);
            if (e1.type == ExpType::LIST) {
                std::vector<Expr*> l = *e1.list;
                if (l.size() < 2) return Expr(LitType::NIL);
                else {
                    std::vector<Expr*> *res(new std::vector<Expr*>());
                    for (size_t i = 1; i < l.size(); i++)
                        res->push_back(l[i]);
                    return Expr(res);
                }
            }
            else throw "Argument for 'cdr' is not list type"; ;
        }
        /* cons */
        case PrimType::CONS: {
            if (args.size() != 2) throw "Invalid num args for 'cons'";
            Expr e1 = args[0]->eval(bindings, e);
            Expr e2 = args[1]->eval(bindings, e);
            if (e1.type != ExpType::LIST && e2.type == ExpType::LIST) {
                std::vector<Expr*> *l(new std::vector<Expr*>(*e2.list));
                Expr *new_val = new Expr(e1);
                l->insert(l->begin(), new_val);
                return Expr(l);
            }
            else throw "Invalid arguments type for 'cons'"; ;
        }
        /* append */
        case PrimType::APPEND: {
            if (args.size() != 2) throw "Invalid num args for 'append'";
            Expr e1 = args[0]->eval(bindings, e);
            Expr e2 = args[1]->eval(bindings, e);
            if (e1.type == ExpType::LIST && e2.type == ExpType::LIST) {
                std::vector<Expr*> *l(new std::vector<Expr*>(*e1.list));
                for (auto &elem : *e2.list) {
                    Expr *new_val(new Expr(*elem));
                    l->push_back(new_val);
                }
                return Expr(l);
            }
            else throw "Invalid arguments type for 'append'"; ;
        }
        /* map */
        case PrimType::MAP: {
            if (args.size() != 2) throw "Invalid num args for 'map'";
            Expr fun = args[0]->eval(bindings, e);
            Expr iter = args[1]->eval(bindings, e);
            if (fun.type == ExpType::PROC && iter.type == ExpType::LIST) {
                std::vector<Expr*> *l(new std::vector<Expr*>());
                for (auto &elem : *iter.list) {
                    std::vector<Expr*> args = {};
                    Expr *arg_binding(new Expr(*elem));
                    args.push_back(arg_binding);
                    
                    Expr *applied_elem(new Expr(fun.eval(&args, e)));
                    l->push_back(applied_elem);
                }
                return Expr(l);
            }
            else throw "Invalid arguments type for 'map'"; ;
        }
        /* filter */
        case PrimType::FILTER: {
            if (args.size() != 2) throw "Invalid num args for 'filter'";
            Expr fun = args[0]->eval(bindings, e);
            Expr iter = args[1]->eval(bindings, e);
            if (fun.type == ExpType::PROC && iter.type == ExpType::LIST) {
                std::vector<Expr*> *l(new std::vector<Expr*>());
                for (auto &elem : *iter.list) {
                    std::vector<Expr*> args { elem };
                    Expr applied_elem = Expr(fun.eval(&args, e));
                    if (applied_elem.get_expr_type() == ExpType::LIT) {
                        if (applied_elem.lit == LitType::TRUE)
                            l->push_back(new Expr(*elem));
                    }
                    else { throw "Decider function does not return lit type"; }
                }
                return Expr(l);
            }
            else throw "Invalid arguments type for 'filter'"; ;
        }
        /* null? */
        case PrimType::IS_NULL: {
            if (args.size() != 1) throw "Invalid num args for 'null?'";
            Expr e1 = args[0]->eval(bindings, e);
            if (e1.type == ExpType::LIST) {
                std::vector<Expr*> l = *e1.list;
                if (l.size() == 0) return Expr(LitType::TRUE);
                else return Expr(LitType::FALSE);
            }
            else throw "Invalid argument type for 'null?'"; ;
        }

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
        case ExpType::PRIM:     return eval_prim(bindings, e);
        case ExpType::SYMBOL:   return eval_sym(bindings, e);
        case ExpType::PROC:     return eval_proc(bindings, e);
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
        case ExpType::STRING:  { std::cout << sval;               break; }
        case ExpType::PROC:    { std::cout << "<procedure>";      break; }
        case ExpType::SYMBOL:  { 
            if (std::get<1>(sym))
                std::get<1>(sym)->print_to_console();
            else 
                std::cerr << "Unknown symbol '" << std::get<0>(sym) << "'";
            break;
        }
        case ExpType::PRIM: {
            switch (std::get<0>(prim)) {
                case PrimType::LAMBDA:  std::cout << "<closure>"; break;
                case PrimType::DEFINE:  break;
                case PrimType::SET:     break;
                default:                std::cout << "<primitive>"; break;
            }
            break;
        }
        case ExpType::LIT: {
            switch (lit) {
                case LitType::TRUE:     std::cout<< "#t"; break;
                case LitType::FALSE:    std::cout<< "#f"; break;
                case LitType::NIL:      std::cout<< "()"; break;
                default: break;
            }
            break;
        }
        case ExpType::LIST: {
            std::cout << "(";
            size_t i = 0;
            for (; i < list->size() - 1; i++) {
                list->at(i)->print_to_console(); std::cout << " ";
            }
            list->at(i)->print_to_console();     std::cout << ")";
            break;
        }
        default: break;
    }
}
