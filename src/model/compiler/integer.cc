/**
 *  @file integer.cc
 *  @brief Boolean compiler - integer consts manipulations
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

void Compiler::integer_neg(const Expr_ptr expr)
{
    POP_ADD(lhs);
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.Negate());
}

void Compiler::integer_not(const Expr_ptr expr)
{
    POP_ADD(lhs);
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.BWCmpl());
}

void Compiler::integer_plus(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.Plus(rhs));

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_sub(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.Minus(rhs));

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_mul(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.Times(rhs));

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_div(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.Divide(rhs));

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_mod(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.Modulus(rhs));

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_and(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.BWTimes(rhs)); /* bitwise integer arithmetic */

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_or(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.BWOr(rhs)); /* bitwise integer arithmetic */

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_xor(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.BWXor(rhs)); /* bitwise integer arithmetic */

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_xnor(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.BWXnor(rhs)); /* bitwise integer arithmetic */

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_implies(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.BWCmpl().BWOr(rhs)); /* bitwise integer arithmetic */

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_lshift(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.LShift(rhs)); /* bitwise integer arithmetic */

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_rshift(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.RShift(rhs)); /* bitwise integer arithmetic */

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_equals(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.Equals(rhs));

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_not_equals(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.Equals(rhs).Cmpl());

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_gt(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(rhs.LT(lhs)); // simulate GT op

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_ge(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(rhs.LEQ(lhs)); // simulate GEQ op

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_lt(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.LT(rhs));

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_le(const Expr_ptr expr)
{
    POP_ADD(rhs);
    POP_ADD(lhs);
    assert (f_enc.is_constant(rhs));
    assert (f_enc.is_constant(lhs));

    PUSH_ADD(lhs.LEQ(rhs));

    f_type_stack.pop_back(); // consume one, leave the other
}

void Compiler::integer_subscript(const Expr_ptr expr)
{
    f_type_stack.pop_back(); // consume index
    Type_ptr type = f_type_stack.back(); f_type_stack.pop_back(); // consume array

    assert( type -> is_array());
    ArrayType_ptr atype = type -> as_array();
    unsigned elem_size  = atype -> of() -> width();
    unsigned elem_count = atype -> nelems();

    POP_ADD(index);
    assert(f_enc.is_constant(index));
    unsigned ndx = f_enc.const_value(index);

    POP_ALGEBRAIC(vec, elem_size * elem_count);

    if (ndx < elem_count) {
        for (unsigned i = 0; i < elem_size; ++ i) {
            PUSH_ADD( vec[ ndx * elem_size + i ]);
        }
    }
    else {
        /* out of boundaries */
        for (unsigned i = 0; i < elem_size; ++ i) {
            PUSH_ADD( f_enc.error());
        }
    }

    f_type_stack.push_back( atype -> of());
}
