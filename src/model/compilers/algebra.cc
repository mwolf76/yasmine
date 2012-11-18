/**
 *  @file algebra.cc
 *  @brief Boolean compiler - algebraic manipulations
 *
 *  This module contains definitions and services that implement the
 *  booolean expressions compilation into a form which is suitable for
 *  the SAT analysis. Current implementation uses ADDs to perform
 *  expression manipulation and booleanization. Expressions are
 *  assumed to be type-safe, only boolean expressions on arithmetic
 *  predicates are supported. The final result of expression
 *  compilation must be a 0-1 ADD which is suitable for CNF clauses
 *  injection directly into the SAT solver. The compilation engine is
 *  implemented using a simple walker pattern: (a) on preorder, return
 *  true if the node has not yet been visited; (b) always do in-order
 *  (for binary nodes); (c) perform proper compilation in post-order
 *  hooks.
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
#include <compiler.hh>

/* Important Remark: operand arguments (which are DD vectors) are
   fetched from the internal DD stack in a little-endian fashion, that
   is MSB first. On the other hand, to ensure proper behavior the
   *result* of the operation has to be pushed in reverse order. */

void Compiler::algebraic_neg(const Expr_ptr expr)
{
    assert( is_unary_algebraic(expr) );
    ExprMgr& em = f_owner.em();
    TypeMgr& tm = f_owner.tm();

    const Type_ptr type = f_type_stack.back(); f_type_stack.pop_back();
    unsigned width = tm.as_algebraic(type)->width();

    /* create temp complemented ADDs */
    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back().BWCmpl(); f_add_stack.pop_back();
    }

    /* rewrite ( -x ) as ( !x + 1 ) */
    FQExpr temp = make_temporary_encoding(lhs, width);
    (*this)(em.make_add(temp.expr(), em.make_one()));
}

void Compiler::algebraic_not(const Expr_ptr expr)
{
    assert( is_unary_algebraic(expr) );
    TypeMgr& tm = f_owner.tm();

    const Type_ptr type = f_type_stack.back(); // just inspect
    unsigned width = tm.as_algebraic(type)->width();

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* perform bw arithmetic, nothing fancy  here :-) */
    for (unsigned i = 0; i < width; ++ i) {
        /* ! x[i] */
        unsigned ndx = width - i - 1;
        f_add_stack.push_back(lhs[ndx].BWCmpl());
    }
}

void Compiler::algebraic_plus(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest, takes care of type stack

    ADD rhs[width];
    for (unsigned i = 0; i < width ; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width ; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* perform arithmetic sum using positional algorithm */
    ADD carry = f_enc.zero();
    for (unsigned i = 0; i < width; ++ i) {

        /* x[i] + y[i] + c */
        unsigned ndx = width - i - 1;
        ADD tmp = lhs[ndx].Plus(rhs[ndx]).Plus(carry);
        carry = f_enc.base().LEQ(tmp); /* c >= 0x10 */

        /* x[i] = (x[i] + y[i] + c) % base */ // TODO: detect overflow on MSB
        f_add_stack.push_back(tmp.Modulus(f_enc.base()));
    }
}

void Compiler::algebraic_sub(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );

    /* rewrite requires discarding operands */
    algebraic_discard_op();
    algebraic_discard_op();

    ExprMgr& em = f_owner.em();
    (*this)(em.make_add(expr->lhs(), em.make_neg(expr->rhs())));
}

void Compiler::algebraic_mul(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned pos, width = algebrize_ops_binary(); // largest, takes care of type stack

    ADD rhs[width];
    for (int i = width -1; (0 <= i) ; -- i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (int i = width -1; (0 <= i) ; -- i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD res[width];
    for (int i = width -1; 0 <= i; -- i) {
        res[i] = f_enc.zero();
    }

    ADD tmp[width];
    for (int i = width -1; 0 <= i; -- i) {
        tmp[i] = f_enc.zero();
    }

    ADD carry = f_enc.zero();

    for (int i = width -1; 0 <= i; -- i) {
        for (int j = width -1; 0 <= j; -- j) {

            // ignore what happend out of result boundaries
            if (0 <= (pos = width - i - j)) {

                /* build mul table for digit product */
                ADD product = lhs[i].Times(rhs[j]).Plus(carry);

                tmp[pos] = product.Modulus(f_enc.base());
                carry = product.Divide(f_enc.base());
            }
        }

        // update result
        for (int j = width -1; i <= j; -- j) {
            res[j] += tmp[j];
        }

        // return i-th digit of result
        f_add_stack.push_back(res[i]);
    }
}

void Compiler::algebraic_div(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    assert( false ); // TODO
}

void Compiler::algebraic_mod(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    assert( false ); // TODO
}

void Compiler::algebraic_and(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* perform bw arithmetic, nothing fancy  here :-) */
    for (unsigned i = 0; i < width; ++ i) {

        /* x[i] &  y[i] */
        unsigned ndx = width - i - 1;
        f_add_stack.push_back(lhs[ndx].BWTimes(rhs[ndx]));
    }
}

void Compiler::algebraic_or(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* perform bw arithmetic, nothing fancy  here :-) */
    for (unsigned i = 0; i < width; ++ i) {

        /* x[i] | y[i] */
        unsigned ndx = width - i - 1;
        f_add_stack.push_back(lhs[ndx].BWOr(rhs[ndx]));
    }
}

void Compiler::algebraic_xor(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* perform bw arithmetic, nothing fancy  here :-) */
    for (unsigned i = 0; i < width; ++ i) {

        /* x[i] ^ y[i] */
        unsigned ndx = width - i - 1;
        f_add_stack.push_back(lhs[ndx].BWXor(rhs[ndx]));
    }
}

void Compiler::algebraic_xnor(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* perform bw arithmetic, nothing fancy  here :-) */
    for (unsigned i = 0; i < width; ++ i) {

        /* !(x[i] ^  y[i]) */
        unsigned ndx = width - i - 1;
        f_add_stack.push_back(lhs[ndx].BWXnor(rhs[ndx]));
    }
}

void Compiler::algebraic_implies(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* perform bw arithmetic, nothing fancy  here :-) */
    for (unsigned i = 0; i < width; ++ i) {

        /* x[i] ->  y[i] */
        unsigned ndx = width - i - 1;
        f_add_stack.push_back(lhs[ndx].BWCmpl().BWOr(rhs[ndx]));
    }
}

void Compiler::algebraic_lshift(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (int i = width -1; (0 <= i) ; -- i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (int i = width -1; (0 <= i) ; -- i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* perform bw arithmetic, nothing fancy  here :-) */
    for (int i = width -1; (0 <= i); -- i) {

        /* x[i] &  y[i] */
        ADD tmp = lhs[i].BWCmpl().BWXor(rhs[i]);
        f_add_stack.push_back(tmp);
    }
}

void Compiler::algebraic_rshift(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    assert( 0 ); // TODO: yet to be implemented...
}

void Compiler::algebraic_equals(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* perform bw arithmetic, similar to xnor, only conjuct res */
    ADD tmp = f_enc.one();
    for (unsigned i = 0; i < width; ++ i) {

        /* x[i] == y[i] */
        unsigned ndx = width - i - 1;
        tmp *= lhs[ndx].Equals(rhs[ndx]);
    }

    /* just one result */
    f_add_stack.push_back(tmp);
}

void Compiler::algebraic_not_equals(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* perform bw arithmetic, similar to xnor, only conjuct res and negate */
    ADD tmp = f_enc.one();
    for (unsigned i = 0; i < width; ++ i) {
        /* x[i] == y[i] */
        unsigned ndx = width - i - 1;
        tmp *= lhs[ndx].Equals(rhs[ndx]);
    }

    /* just one result */
    f_add_stack.push_back(tmp.Cmpl());
}

void Compiler::algebraic_gt(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* MSB predicate first, if false and prefix matches, inspect next
       digit. */
    ADD tmp = f_enc.zero();
    for (unsigned i = 0; i < width; ++ i) {

        ADD pfx = f_enc.one();
        for (unsigned j = 0; j < i; j ++ ) {
            pfx *= rhs[j].Equals(lhs[j]);
        }

        /* pfx & ( y[i] < x[i] ) */
        tmp += pfx.Times(rhs[i].LT(lhs[i]));
    }

    /* just one result */
    f_add_stack.push_back(tmp);
}

void Compiler::algebraic_ge(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* MSB predicate first, if false and prefix matches, inspect next
       digit. */
    ADD tmp = f_enc.zero();
    for (unsigned i = 0; i < width; ++ i) {

        ADD pfx = f_enc.one();
        for (unsigned j = 0; j < i; j ++ ) {
            pfx *= rhs[j].Equals(lhs[j]);
        }

        /* pfx & ( y[i] <= x[i] ) */
        tmp += pfx.Times(rhs[i].LEQ(lhs[i]));
    }

    /* just one result */
    f_add_stack.push_back(tmp);
}

void Compiler::algebraic_lt(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* MSB predicate first, if false and prefix matches, inspect next
       digit. */
    ADD tmp = f_enc.zero();
    for (unsigned i = 0; i < width; ++ i) {

        ADD pfx = f_enc.one();
        for (unsigned j = 0; j < i; j ++ ) {
            pfx *= rhs[j].Equals(lhs[j]);
        }

        /* pfx & ( x[i] < y[i] ) */
        tmp += pfx.Times(lhs[i].LT(rhs[i]));
    }

    /* just one result */
    f_add_stack.push_back(tmp);
}

void Compiler::algebraic_le(const Expr_ptr expr)
{
    assert( is_binary_algebraic(expr) );
    unsigned width = algebrize_ops_binary(); // largest

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    /* MSB predicate first, if false and prefix matches, inspect next
       digit. */
    ADD tmp = f_enc.zero();
    for (unsigned i = 0; i < width; ++ i) {

        ADD pfx = f_enc.one();
        for (unsigned j = 0; j < i; j ++ ) {
            pfx *= rhs[j].Equals(lhs[j]);
        }

        /* pfx & ( x[i] <= y[i] ) */
        tmp += pfx.Times(lhs[i].LEQ(rhs[i]));
    }

    /* just one result */
    f_add_stack.push_back(tmp);
}

void Compiler::algebraic_ite(const Expr_ptr expr)
{
    assert( is_ite_algebraic(expr) );
    unsigned width = algebrize_ops_binary( true ); // largest, kill extra type

    ADD rhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        rhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD lhs[width];
    for (unsigned i = 0; i < width; ++ i) {
        lhs[i] = f_add_stack.back(); f_add_stack.pop_back();
    }

    ADD cond = f_add_stack.back(); f_add_stack.pop_back();

    for (unsigned i = 0; i < width; ++ i) {

        /* (cond) ? x[i] : y[i] */
        unsigned ndx = width - i - 1;
        f_add_stack.push_back(cond.Ite(lhs[ndx], rhs[ndx]));
    }
}
