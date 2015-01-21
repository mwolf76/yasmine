/**
 *  @file evaluator.cc
 *  @brief Expr evaluator
 *
 *  This module contains definitions and services that implement an
 *  optimized storage for expressions. Expressions are stored in a
 *  Directed Acyclic Graph (DAG) for data sharing.
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

#include <expr/expr.hh>

#include <symb/proxy.hh>

#include <witness/evaluator.hh>
#include <witness/witness_mgr.hh>

Evaluator::Evaluator(WitnessMgr& owner)
    : f_owner(owner)
{
    DRIVEL
        << "Created Evaluator @"
        << this
        << std::endl;
}

Evaluator::~Evaluator()
{
    DRIVEL
        << "Destroying Evaluator @"
        << this
        << std::endl;
}

void Evaluator::clear_internals()
{
    f_type_stack.clear();
    f_values_stack.clear();
    f_ctx_stack.clear();
    f_time_stack.clear();
    f_te2v_map.clear();
}

Expr_ptr Evaluator::process(Witness &witness, Expr_ptr ctx,
                           Expr_ptr body, step_t time)
{
    ExprMgr& em
        (ExprMgr::INSTANCE());

    clear_internals();

    // setting the environment
    f_witness = &witness;

    // walk body in given ctx
    f_ctx_stack.push_back(ctx);

    // toplevel (time is assumed at 0, arbitrarily nested next allowed)
    f_time_stack.push_back(time);

    /* Invoke walker on the body of the expr to be processed */
    (*this)(body);

    // sanity conditions
    assert(1 == f_values_stack.size());
    assert(1 == f_type_stack.size());
    assert(1 == f_ctx_stack.size());
    assert(1 == f_time_stack.size());

    // Exactly one expression
    value_t res_value
        (f_values_stack.back());

    POP_TYPE(res_type);
    if (res_type -> is_boolean())
        return res_value ? em.make_true() : em.make_false();

    else if (res_type -> is_enum()) {
        EnumType_ptr enum_type
            (res_type -> as_enum());

        const ExprSet& literals
            (enum_type -> literals());

        ExprSet::const_iterator i
            (literals.begin());

        while (0 < res_value) {
            -- res_value;
            ++ i;
        }

        return *i;
    }

    else if (res_type -> is_algebraic())
        return em.make_const(res_value);

    else assert(false);
}

/*  Evaluation engine is implemented using a simple expression walker
 *  pattern: (a) on preorder, return true if the node has not yet been
 *  visited; (b) always do in-order (for binary nodes); (c) perform
 *  proper compilation in post-order hooks. */

bool Evaluator::walk_next_preorder(const Expr_ptr expr)
{
    step_t curr_time = f_time_stack.back();
    f_time_stack.push_back(curr_time + 1);
    return true;
}
void Evaluator::walk_next_postorder(const Expr_ptr expr)
{
    assert (0 < f_time_stack.size());
    f_time_stack.pop_back(); // reset time stack
}

bool Evaluator::walk_neg_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Evaluator::walk_neg_postorder(const Expr_ptr expr)
{
    POP_VALUE(lhs);
    PUSH_VALUE(- lhs);
}

bool Evaluator::walk_not_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Evaluator::walk_not_postorder(const Expr_ptr expr)
{
    POP_VALUE(lhs);
    PUSH_VALUE(! lhs);
}

bool Evaluator::walk_bw_not_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Evaluator::walk_bw_not_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(lhs);
    PUSH_VALUE(~ lhs);
}

bool Evaluator::walk_add_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_add_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_add_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(lhs + rhs);
}

bool Evaluator::walk_sub_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_sub_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_sub_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(lhs - rhs);
}

bool Evaluator::walk_div_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_div_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_div_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(lhs / rhs);
}

bool Evaluator::walk_mul_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_mul_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_mul_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(lhs * rhs);
}

bool Evaluator::walk_mod_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_mod_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_mod_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(lhs % rhs);
}

bool Evaluator::walk_and_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_and_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_and_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(lhs && rhs);
}

bool Evaluator::walk_bw_and_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_bw_and_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_bw_and_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(lhs & rhs);
}

bool Evaluator::walk_or_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_or_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_or_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(lhs || rhs);
}

bool Evaluator::walk_xor_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_xor_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_xor_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE((lhs && !rhs) || (!lhs && rhs));
}


bool Evaluator::walk_bw_or_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_bw_or_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_bw_or_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(lhs | rhs);
}

bool Evaluator::walk_bw_xor_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_bw_xor_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_bw_xor_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(lhs ^ rhs);
}

bool Evaluator::walk_bw_xnor_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_bw_xnor_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_bw_xnor_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(( ! lhs | rhs ) & ( ! rhs | lhs ));
}

bool Evaluator::walk_implies_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_implies_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_implies_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(( ! lhs || rhs ));
}

bool Evaluator::walk_iff_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_iff_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_iff_postorder(const Expr_ptr expr)
{
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(( ! lhs || rhs ) && ( ! rhs || lhs ));
}

bool Evaluator::walk_lshift_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_lshift_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_lshift_postorder(const Expr_ptr expr)
{
    /* drops rhs, which is fine */
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(( lhs << rhs ));
}

bool Evaluator::walk_rshift_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_rshift_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_rshift_postorder(const Expr_ptr expr)
{
    /* drop rhs, which is fine */
    DROP_TYPE();

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(( lhs >> rhs ));
}

bool Evaluator::walk_eq_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_eq_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_eq_postorder(const Expr_ptr expr)
{
    TypeMgr& tm
        (f_owner.tm());

    DROP_TYPE();
    DROP_TYPE();
    PUSH_TYPE(tm.find_boolean());

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(( lhs == rhs ));
}

bool Evaluator::walk_ne_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_ne_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_ne_postorder(const Expr_ptr expr)
{
    TypeMgr& tm
        (f_owner.tm());

    DROP_TYPE();
    DROP_TYPE();
    PUSH_TYPE(tm.find_boolean());

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(lhs != rhs);
}

bool Evaluator::walk_gt_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_gt_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_gt_postorder(const Expr_ptr expr)
{
    TypeMgr& tm
        (f_owner.tm());

    DROP_TYPE();
    DROP_TYPE();
    PUSH_TYPE(tm.find_boolean());

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(( lhs > rhs ));
}

bool Evaluator::walk_ge_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_ge_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_ge_postorder(const Expr_ptr expr)
{
    TypeMgr& tm
        (f_owner.tm());

    DROP_TYPE();
    DROP_TYPE();
    PUSH_TYPE(tm.find_boolean());

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(( lhs >= rhs ));
}

bool Evaluator::walk_lt_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_lt_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_lt_postorder(const Expr_ptr expr)
{
    TypeMgr& tm
        (f_owner.tm());

    DROP_TYPE();
    DROP_TYPE();
    PUSH_TYPE(tm.find_boolean());

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(( lhs < rhs ));
}

bool Evaluator::walk_le_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_le_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_le_postorder(const Expr_ptr expr)
{
    TypeMgr& tm
        (f_owner.tm());

    DROP_TYPE();
    DROP_TYPE();
    PUSH_TYPE(tm.find_boolean());

    POP_VALUE(rhs);
    POP_VALUE(lhs);
    PUSH_VALUE(( lhs <= rhs ));
}

bool Evaluator::walk_ite_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_ite_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_ite_postorder(const Expr_ptr expr)
{
   POP_TYPE(rhs_type);
   DROP_TYPE();
   DROP_TYPE();
   PUSH_TYPE(rhs_type);

   POP_VALUE(rhs);
   POP_VALUE(lhs);
   POP_VALUE(cnd);
   PUSH_VALUE(( cnd ? lhs : rhs ));
}

bool Evaluator::walk_cond_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_cond_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_cond_postorder(const Expr_ptr expr)
{ /* nop */ }

bool Evaluator::walk_dot_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_dot_inorder(const Expr_ptr expr)
{
    ExprMgr& em
        (f_owner.em());

    DROP_TYPE();

    TOP_CTX(parent_ctx);

    Expr_ptr ctx
        (em.make_dot( parent_ctx, expr -> lhs()));
    PUSH_CTX(ctx);

    return true;
}
void Evaluator::walk_dot_postorder(const Expr_ptr expr)
{ DROP_CTX(); }

bool Evaluator::walk_params_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_params_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_params_postorder(const Expr_ptr expr)
{ assert (false); /* not yet implemented */ }

bool Evaluator::walk_subscript_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
bool Evaluator::walk_subscript_inorder(const Expr_ptr expr)
{ return true; }

void Evaluator::walk_subscript_postorder(const Expr_ptr expr)
{
#if 0
    unsigned base = Cudd_V(f_enc.base().getNode());

    const Type_ptr rhs_type = f_type_stack.back(); f_type_stack.pop_back();

    /* algebrize selection expr (rhs), using machine width */
    assert (rhs_type->is_algebraic());
    // algebrize_unary_subscript();
    assert(false); // tODO

    ADD selector[2]; // TODO
    for (unsigned i = 0; i < 2; ++ i) {
        selector[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    const Type_ptr lhs_type = f_type_stack.back(); f_type_stack.pop_back();
    unsigned size = lhs_type -> size();

    const Type_ptr scalar_type = lhs_type -> as_array() ->of();
    unsigned width = scalar_type->size();

    /* fetch DDs from the stack */
    ADD dds[width * size];
    for (unsigned i = 0; i < width * size; ++ i) {
        dds[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD res; /* one digit at a time */
    for (unsigned i = 0; i < width; ++ i) {
        unsigned ndx = width - i - 1;
        res = f_enc.zero(); // TODO: FAILURE here

        for (unsigned j = 0; j < size; ++ j) {

            /* selected index */
            unsigned selection = size - j -1;

            ADD cond = f_enc.one();
            value_t value = ndx;

            /* encode the case selection as a conjunction of
               Equals ADD digit-by-digit (inlined) */
            for (unsigned k = 0; k < width; ++ k) {

                ADD digit = f_enc.constant(value % base);
                f_add_stack.push_back(digit);
                value /= base;

                /* case selection */
                cond *= selector[ndx].Equals(digit);
            }
            assert (value == 0); // not overflowing

            /* chaining */
            res = cond.Ite( dds[width * selection + i], res);
        }

        f_add_stack.push_back(res);
    }
#endif
}

bool Evaluator::walk_set_preorder(const Expr_ptr expr)
{ return cache_miss(expr); }
void Evaluator::walk_set_postorder(const Expr_ptr expr)
{
    assert(false); // TODO
}

bool Evaluator::walk_comma_preorder(const Expr_ptr expr)
{  return cache_miss(expr); }
bool Evaluator::walk_comma_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_comma_postorder(const Expr_ptr expr)
{ assert (false); /* TODO support inlined non-determinism */ }

bool Evaluator::walk_type_preorder(const Expr_ptr expr)
{  return cache_miss(expr); }
bool Evaluator::walk_type_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_type_postorder(const Expr_ptr expr)
{ assert (false); /* TODO support inlined non-determinism */ }

bool Evaluator::walk_cast_preorder(const Expr_ptr expr)
{  return cache_miss(expr); }
bool Evaluator::walk_cast_inorder(const Expr_ptr expr)
{ return true; }
void Evaluator::walk_cast_postorder(const Expr_ptr expr)
{ assert (false); /* TODO support inlined non-determinism */ }


void Evaluator::walk_leaf(const Expr_ptr expr)
{
    ExprMgr& em
        (f_owner.em());

    TypeMgr& tm
        (f_owner.tm());

    /* cached? */
    if (! cache_miss(expr))
        return;

    TOP_CTX(ctx);
    TOP_TIME(time);

    // 0. explicit boolean consts
    if (em.is_bool_const(expr)) {

        PUSH_TYPE(tm.find_boolean());

        if (em.is_false(expr))
            PUSH_VALUE(0);
        else if (em.is_true(expr))
            PUSH_VALUE(1);
        else assert(false);

        return;
    }

    // 1. explicit constants are integer consts (e.g. 42) ..
    else if (em.is_int_numeric(expr)) {
        unsigned ww
            (OptsMgr::INSTANCE().word_width());
        PUSH_TYPE (tm.find_unsigned(ww));
        PUSH_VALUE(expr -> value());

        return;
    }

    ResolverProxy resolver;

    Expr_ptr full
        (em.make_dot( ctx, expr));

    Symbol_ptr symb
        (resolver.symbol(full));

    // 3. enum literals
    if (symb->is_literal()) {

        Literal& lit
            (symb->as_literal());

        Type_ptr type
            (lit.type());
        PUSH_TYPE(type);

        value_t value
            (lit.value());
        PUSH_VALUE(value);
        return;
    }

    else if (symb->is_variable()) {

        // push into type stack
        Type_ptr type
            (symb->as_variable().type());

        PUSH_TYPE(type);
        if (type->is_instance())
            return;

        if (f_witness -> has_value( full, time)) {
            Expr_ptr expr
                (f_witness -> value( full, time));

            // promote FALSE -> 0, TRUE -> 1
            if (em.is_false(expr))
                f_values_stack.push_back(0);

            else if (em.is_true(expr))
                f_values_stack.push_back(1);

            else
                f_values_stack.push_back(expr -> value());

            return;
        }
    }

    else if (symb->is_parameter()) {

        ModelMgr& mm
            (ModelMgr::INSTANCE());

        /* parameters must be resolved against the Param map
           maintained by the ModelMgr */
        Expr_ptr rewrite
            (mm.rewrite_parameter(full));

#if 0
        DRIVEL
            << "Rewritten `"
            << full << "` to "
            << rewrite
            << std::endl;
#endif

        Expr_ptr rewritten_ctx
            (rewrite -> lhs());
        PUSH_CTX(rewritten_ctx);

        Expr_ptr rewritten_expr
            (rewrite -> rhs());
        (*this) (rewritten_expr);

        DROP_CTX();
        return;
    }

    else if (symb -> is_define()) {
        Expr_ptr body
            (symb -> as_define().body());

#if 0
        DRIVEL
            << "Inlining `"
            << expr
            << "` := "
            << body
            << std::endl;
#endif

        (*this) (body);
        return;
    }

    assert( false ); /* unexpected */
}

bool Evaluator::cache_miss(const Expr_ptr expr)
{
    ExprMgr& em
        (ExprMgr::INSTANCE());

    assert(0 < f_ctx_stack.size());
    Expr_ptr ctx
        (f_ctx_stack.back());

    assert(0 < f_time_stack.size());
    step_t step
        (f_time_stack.back());

    TimedExpr key
        ( em.make_dot( ctx , expr), step);

    TimedExprValueMap::iterator eye
        (f_te2v_map.find(key));

    if (eye != f_te2v_map.end()) {
        value_t res
            ((*eye).second);

        PUSH_VALUE(res);
        return false;
    }

    return true;
}
