/**
 *  @file inferrer.cc
 *  @brief Expr type inferrer
 *
 *  This module contains definitions and services that implement a
 *  type inference engine. The type inference engine is implemented
 *  using a simple walker pattern: (a) on preorder, return true if the
 *  node has not yet been visited; (b) always do in-order (for binary
 *  nodes); (c) perform proper type checking in post-order
 *  hooks. Implicit conversion rules are designed to follow as closely
 *  as possible section 6.3.1 of iso/iec 9899:1999 (aka C99)
 *  standard. Type rules are implemented in the result_type methods of
 *  the TypeMgr class.
 *
 *  Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **/

#include <common.hh>

#include <expr.hh>
#include <inferrer.hh>

#include <model_mgr.hh>
#include <type_exceptions.hh>

#include <proxy.hh>

// uncommment following line to enable post_node_hook debug (verbose!)
#define DEBUG_INFERRER

Inferrer::Inferrer(ModelMgr& owner)
    : f_map()
    , f_type_stack()
    , f_ctx_stack()
    , f_owner(owner)
{
    DEBUG << "Created Inferrer @" << this << endl;
}

Inferrer::~Inferrer()
{ DEBUG << "Destroying Inferrer @" << this << endl; }

/* this function is not memoized by design, for a memoized wrapper use type() */
Type_ptr Inferrer::process(Expr_ptr ctx, Expr_ptr body, expected_t expected)
{
    // remove previous results
    f_type_stack.clear();
    f_ctx_stack.clear();

    // walk body in given ctx
    f_ctx_stack.push_back(ctx);

    // invoke walker on the body of the expr to be processed
    (*this)(body);

    assert(1 == f_type_stack.size());

    /* possibly throws an exception */
    return check_expected_type(expected);
}

void Inferrer::pre_hook()
{}
void Inferrer::post_hook()
{}

void Inferrer::pre_node_hook(Expr_ptr expr)
{
}

void Inferrer::post_node_hook(Expr_ptr expr)
{
    memoize_result(expr);

#ifdef DEBUG_INFERRER
    activation_record curr = f_recursion_stack.top();
    DEBUG << "inferrer post-node hook, expr = " << curr.expr << endl;

    DEBUG << "Type Stack" << endl;
    for (TypeStack::reverse_iterator i = f_type_stack.rbegin();
         i != f_type_stack.rend(); ++ i) {
        DEBUG << *i << endl;
    }
    DEBUG << "--------------------" << endl;
#endif
}

Type_ptr Inferrer::check_expected_type(expected_t expected)
{
    TypeMgr& tm = f_owner.tm();

    POP_TYPE(type);
    assert (NULL != type);

    /* what could go wrong? */
    if ((tm.is_boolean(type) &&
         (0 == (expected & TP_BOOLEAN))) ||

        (tm.is_int_const(type) &&
         (0 == (expected & TP_INT_CONST))) ||

        (tm.is_signed_algebraic(type) &&
         (0 == (expected & TP_SIGNED_INT))) ||

        (tm.is_unsigned_algebraic(type) &&
         (0 == (expected & TP_UNSIGNED_INT))) ||

        (tm.is_enum(type) &&
         (0 == (expected & TP_ENUM))) ||

        (tm.is_array(type) &&
         (0 == (expected & TP_ARRAY)))) {

        throw BadType(type->repr(), expected);
    }

    return type;
}

bool Inferrer::walk_F_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Inferrer::walk_F_postorder(const Expr_ptr expr)
{ walk_unary_temporal_postorder(expr); }

bool Inferrer::walk_G_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Inferrer::walk_G_postorder(const Expr_ptr expr)
{ walk_unary_temporal_postorder(expr); }

bool Inferrer::walk_X_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Inferrer::walk_X_postorder(const Expr_ptr expr)
{ walk_unary_temporal_postorder(expr); }

bool Inferrer::walk_U_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_U_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_U_postorder(const Expr_ptr expr)
{ walk_binary_temporal_postorder(expr); }

bool Inferrer::walk_R_preorder(const Expr_ptr expr)
{ return cache_miss(expr);  }
bool Inferrer::walk_R_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_R_postorder(const Expr_ptr expr)
{ walk_binary_temporal_postorder(expr); }

bool Inferrer::walk_AF_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Inferrer::walk_AF_postorder(const Expr_ptr expr)
{ walk_unary_temporal_postorder(expr); }

bool Inferrer::walk_AG_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Inferrer::walk_AG_postorder(const Expr_ptr expr)
{ walk_unary_temporal_postorder(expr); }

bool Inferrer::walk_AX_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Inferrer::walk_AX_postorder(const Expr_ptr expr)
{ walk_unary_temporal_postorder(expr); }

bool Inferrer::walk_AU_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_AU_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_AU_postorder(const Expr_ptr expr)
{ walk_binary_temporal_postorder(expr); }

bool Inferrer::walk_AR_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_AR_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_AR_postorder(const Expr_ptr expr)
{ walk_binary_temporal_postorder(expr); }

bool Inferrer::walk_EF_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Inferrer::walk_EF_postorder(const Expr_ptr expr)
{ walk_unary_temporal_postorder(expr); }

bool Inferrer::walk_EG_preorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_EG_postorder(const Expr_ptr expr)
{ walk_unary_temporal_postorder(expr); }

bool Inferrer::walk_EX_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Inferrer::walk_EX_postorder(const Expr_ptr expr)
{ walk_unary_temporal_postorder(expr); }

bool Inferrer::walk_EU_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_EU_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_EU_postorder(const Expr_ptr expr)
{ walk_binary_temporal_postorder(expr); }

bool Inferrer::walk_ER_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_ER_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_ER_postorder(const Expr_ptr expr)
{ walk_binary_temporal_postorder(expr); }

bool Inferrer::walk_next_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Inferrer::walk_next_postorder(const Expr_ptr expr)
{ walk_unary_fsm_postorder(expr); }

bool Inferrer::walk_neg_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Inferrer::walk_neg_postorder(const Expr_ptr expr)
{ walk_unary_arithmetical_postorder(expr); }

bool Inferrer::walk_not_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Inferrer::walk_not_postorder(const Expr_ptr expr)
{ walk_unary_logical_postorder(expr); }

bool Inferrer::walk_add_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_add_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_add_postorder(const Expr_ptr expr)
{ walk_binary_arithmetical_postorder(expr); }

bool Inferrer::walk_sub_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_sub_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_sub_postorder(const Expr_ptr expr)
{ walk_binary_arithmetical_postorder(expr); }

bool Inferrer::walk_div_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_div_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_div_postorder(const Expr_ptr expr)
{ walk_binary_arithmetical_postorder(expr); }

bool Inferrer::walk_mul_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_mul_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_mul_postorder(const Expr_ptr expr)
{ walk_binary_arithmetical_postorder(expr); }

bool Inferrer::walk_mod_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_mod_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_mod_postorder(const Expr_ptr expr)
{ walk_binary_arithmetical_postorder(expr); }

bool Inferrer::walk_and_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_and_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_and_postorder(const Expr_ptr expr)
{ walk_binary_logical_or_bitwise_postorder(expr); }

bool Inferrer::walk_or_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_or_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_or_postorder(const Expr_ptr expr)
{ walk_binary_logical_or_bitwise_postorder(expr); }

bool Inferrer::walk_xor_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_xor_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_xor_postorder(const Expr_ptr expr)
{ walk_binary_logical_or_bitwise_postorder(expr); }

bool Inferrer::walk_xnor_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_xnor_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_xnor_postorder(const Expr_ptr expr)
{ walk_binary_logical_or_bitwise_postorder(expr); }

bool Inferrer::walk_implies_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_implies_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_implies_postorder(const Expr_ptr expr)
{ walk_binary_logical_or_bitwise_postorder(expr); }

bool Inferrer::walk_iff_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_iff_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_iff_postorder(const Expr_ptr expr)
{ walk_binary_logical_or_bitwise_postorder(expr); }

bool Inferrer::walk_lshift_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_lshift_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_lshift_postorder(const Expr_ptr expr)
{ walk_binary_shift_postorder(expr); }

bool Inferrer::walk_rshift_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_rshift_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_rshift_postorder(const Expr_ptr expr)
{ walk_binary_shift_postorder(expr); }

bool Inferrer::walk_eq_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_eq_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_eq_postorder(const Expr_ptr expr)
{ walk_binary_boolean_or_relational_postorder(expr); }

bool Inferrer::walk_ne_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_ne_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_ne_postorder(const Expr_ptr expr)
{ walk_binary_boolean_or_relational_postorder(expr); }

bool Inferrer::walk_gt_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_gt_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_gt_postorder(const Expr_ptr expr)
{ walk_binary_relational_postorder(expr); }

bool Inferrer::walk_ge_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_ge_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_ge_postorder(const Expr_ptr expr)
{ walk_binary_relational_postorder(expr); }

bool Inferrer::walk_lt_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_lt_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_lt_postorder(const Expr_ptr expr)
{ walk_binary_relational_postorder(expr); }

bool Inferrer::walk_le_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_le_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_le_postorder(const Expr_ptr expr)
{ walk_binary_relational_postorder(expr); }

bool Inferrer::walk_ite_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_ite_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_ite_postorder(const Expr_ptr expr)
{ walk_ternary_ite_postorder(expr); }

bool Inferrer::walk_cond_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_cond_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_cond_postorder(const Expr_ptr expr)
{ walk_ternary_cond_postorder(expr); }

bool Inferrer::walk_dot_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_dot_inorder(const Expr_ptr expr)
{
    Type_ptr tmp = f_type_stack.back();
    Expr_ptr ctx = tmp->repr();

    f_ctx_stack.push_back(ctx);
    return true;
}
void Inferrer::walk_dot_postorder(const Expr_ptr expr)
{
    assert(false); // no longer supported
#if 0
    TypeMgr& tm = f_owner.tm();
    Type_ptr rhs_type;

    { // RHS, no checks necessary/possible
        const Type_ptr top = f_type_stack.back(); f_type_stack.pop_back();
        rhs_type = top;
    }

    { // LHS, must be an instance (by assertion, otherwise leaf would have fail)
        const Type_ptr top = f_type_stack.back(); f_type_stack.pop_back();
        assert(tm.is_instance(top));
    }

    // propagate rhs type
    PUSH_TYPE(rhs_type);

    // restore previous ctx
    f_ctx_stack.pop_back();
#endif
}

bool Inferrer::walk_params_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_params_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_params_postorder(const Expr_ptr expr)
{ assert( false ); /* not yet implemented */ }

bool Inferrer::walk_subscript_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Inferrer::walk_subscript_inorder(const Expr_ptr expr)
{ return true; }
void Inferrer::walk_subscript_postorder(const Expr_ptr expr)
{
    TypeMgr& tm = f_owner.tm();

    /* return wrapped type */
    PUSH_TYPE(tm.as_array(check_array())->of());
}

void Inferrer::walk_leaf(const Expr_ptr expr)
{
    TypeMgr& tm = f_owner.tm();
    ExprMgr& em = f_owner.em();

    // cache miss took care of the stack already
    if (! cache_miss(expr)) return;

    // is an integer const ..
    if (em.is_int_numeric(expr)) {
        PUSH_TYPE(tm.find_int_const());
        return;
    }

    // .. or a symbol
    if (em.is_identifier(expr)) {
        ResolverProxy proxy;
        Expr_ptr ctx = f_ctx_stack.back();
        ISymbol_ptr symb = proxy.symbol(ctx, expr);

        if (symb->is_const()) {
            Type_ptr res = symb->as_const().type();
            PUSH_TYPE(res);
            return;
        }
        else if (symb->is_literal()) {
            Type_ptr res = symb->as_literal().type();
            PUSH_TYPE(res);
            return;
        }
        else if (symb->is_enum()) {
            Type_ptr res = symb->as_enum().type();
            PUSH_TYPE(res);
            return;
        }
        else if (symb->is_array()) {
            Type_ptr res = symb->as_array().type();
            PUSH_TYPE(res);
            return;
        }
        else if (symb->is_variable()) {
            Type_ptr res = symb->as_variable().type();
            PUSH_TYPE(res);
            return;
        }
        else if (symb->is_define()) {
            (*this)(symb->as_define().body());
            return;
        }
    }

    assert(false); // unexpected
}

// fun: temporal -> temporal
void Inferrer::walk_unary_temporal_postorder(const Expr_ptr expr)
{
    PUSH_TYPE(check_boolean());
}

// fun: temporal x temporal -> temporal
void Inferrer::walk_binary_temporal_postorder(const Expr_ptr expr)
{
    check_boolean();
    PUSH_TYPE(check_boolean());
}

// fun: T -> T
void Inferrer::walk_unary_fsm_postorder(const Expr_ptr expr)
{ /* no checks */ }

// fun: arithm -> arithm
void Inferrer::walk_unary_arithmetical_postorder(const Expr_ptr expr)
{
    PUSH_TYPE( check_arithmetical() );
}

// fun: logical -> logical
void Inferrer::walk_unary_logical_postorder(const Expr_ptr expr)
{
    PUSH_TYPE( check_boolean() );
}

// fun: arithm, arithm -> arithm
void Inferrer::walk_binary_arithmetical_postorder(const Expr_ptr expr)
{
    TypeMgr& tm = f_owner.tm();
    PUSH_TYPE( tm.result_type( expr,
                               check_arithmetical(),
                               check_arithmetical()));
}

// fun: logical x logical -> logical
void Inferrer::walk_binary_logical_postorder(const Expr_ptr expr)
{
    check_boolean();
    PUSH_TYPE( check_boolean() );
}

void Inferrer::walk_binary_logical_or_bitwise_postorder(const Expr_ptr expr)
{
    TypeMgr& tm = f_owner.tm();
    PUSH_TYPE( tm.result_type( expr,
                               check_boolean_or_integer(),
                               check_boolean_or_integer()) );
}


/* specialized for shift ops (use rhs) */
void Inferrer::walk_binary_shift_postorder(const Expr_ptr expr)
{
    Type_ptr rhs = check_arithmetical();
    check_arithmetical(); // discard lhs
    PUSH_TYPE( rhs );
}

// fun: arithmetical x arithmetical -> boolean
void Inferrer::walk_binary_relational_postorder(const Expr_ptr expr)
{
    TypeMgr& tm = f_owner.tm();
    PUSH_TYPE( tm.result_type( expr,
                               check_arithmetical(),
                               check_arithmetical()));
}

// fun: A/B x A/B -> B
void Inferrer::walk_binary_boolean_or_relational_postorder(const Expr_ptr expr)
{
    TypeMgr& tm = f_owner.tm();
    PUSH_TYPE( tm.result_type( expr,
                               check_arithmetical(),
                               check_arithmetical()));
}


// fun:  boolean x T -> T
void Inferrer::walk_ternary_cond_postorder(const Expr_ptr expr)
{
    Type_ptr rhs = f_type_stack.back(); f_type_stack.pop_back();
    check_boolean(); // condition

    PUSH_TYPE( rhs );
}

// fun: (boolean ? T) x T -> T
void Inferrer::walk_ternary_ite_postorder(const Expr_ptr expr)
{
    TypeMgr& tm = f_owner.tm();
    POP_TYPE(rhs);
    POP_TYPE(lhs);
    PUSH_TYPE( tm.result_type( expr, tm.find_boolean(), lhs, rhs));
}

void Inferrer::memoize_result (Expr_ptr expr)
{
    Type_ptr type = f_type_stack.back();
    f_map[ FQExpr(f_ctx_stack.back(),
                  find_canonical_expr(expr)) ] = type;
}

Expr_ptr Inferrer::find_canonical_expr(Expr_ptr expr)
{
    ExprMgr& em = f_owner.em();

    /* time is not relevant here */
    while (em.is_next(expr)) {
        expr = expr->lhs();
    }

    if (em.is_binary_relational(expr)) {
        return em.make_lt(expr->lhs(), expr->rhs());
    }
    else if (em.is_binary_arithmetical(expr)) {
        return em.make_add(expr->lhs(), expr->rhs());
    }
    else if (em.is_binary_logical(expr)) {
        return em.make_and(expr->lhs(), expr->rhs());
    }

    /* no rewrites */
    else if (em.is_unary_arithmetical(expr) ||
             em.is_unary_logical(expr) ||
             em.is_ite(expr) ||
             em.is_cond(expr) ||
             em.is_identifier(expr) ||
             em.is_int_numeric(expr) ||
             em.is_fxd_numeric(expr)) {
        return expr;
    }

    DEBUG << expr << endl;
    assert ( false ); // unreachable
}

Type_ptr Inferrer::type(FQExpr& fqexpr)
{
    /* to avoid a number of cache misses due to compiler rewrites,
       we squeeze types in equivalence classes: Relationals -> lhs
       '<' rhs, Algebraic -> lhs '+' rhs */
    FQExpr key( fqexpr.ctx(),
                find_canonical_expr( fqexpr.expr()));

    TypeReg::const_iterator eye = f_map.find(key);
    Type_ptr res = NULL;

    // cache miss, fallback to walker
    if (eye == f_map.end()) {
        res = process(fqexpr.ctx(), fqexpr.expr());
    }
    else {
        res = (*eye).second;
    }

    assert(NULL != res);
    return res;
}
