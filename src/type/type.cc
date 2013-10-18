/**
 *  @file type.cc
 *  @brief Type system classes
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

#include <type.hh>
#include <type_mgr.hh>

BooleanType::BooleanType(TypeMgr& owner)
    : MonolithicalType(owner)
{
    f_repr = f_owner.em().make_boolean_type();
}

unsigned BooleanType::size() const
{ return 1; }

bool BooleanType::is_abstract() const
{ return false; }

bool Type::is_scalar()
{ return NULL != dynamic_cast<ScalarType_ptr>( this ); }

bool Type::is_monolithical()
{ return NULL != dynamic_cast<MonolithicalType_ptr>( this ); }

bool Type::is_algebraic()
{ return NULL != dynamic_cast<AlgebraicType_ptr>( this ); }

bool Type::is_boolean()
{ return NULL != dynamic_cast<BooleanType_ptr>( this ); }

bool Type::is_intconst()
{ return NULL != dynamic_cast<IntConstType_ptr>( this ); }

bool Type::is_signed_algebraic()
{ return NULL != dynamic_cast<SignedAlgebraicType_ptr>( this ); }

bool Type::is_unsigned_algebraic()
{ return NULL != dynamic_cast<UnsignedAlgebraicType_ptr>( this ); }

bool Type::is_enum()
{ return NULL != dynamic_cast<EnumType_ptr>( this ); }

bool Type::is_array()
{ return NULL != dynamic_cast<ArrayType_ptr>( this ); }

EnumType::EnumType(TypeMgr& owner, ExprSet& literals)
    : MonolithicalType(owner)
    , f_literals(literals)
{
    f_repr = f_owner.em().make_set_type(f_literals);
    ExprSet::iterator i;

    for (i = literals.begin(); i != literals.end(); ++ i) {
        Expr_ptr expr = *i;
        assert(ExprMgr::INSTANCE().is_identifier(expr)); // debug only
    }
}

unsigned EnumType::size() const
{
    unsigned res = 0, pow = 1;

    while (pow < f_literals.size()) {
        ++ res;
        pow *= 2;
    }

    return res;
}
bool EnumType::is_abstract() const
{ return false; }

IntConstType::IntConstType(TypeMgr& owner)
    : AlgebraicType(owner)
{
    f_repr = f_owner.em().make_int_const_type();
}

unsigned IntConstType::size() const
{ return 1; }

bool IntConstType::is_abstract() const
{ return false; }

SignedAlgebraicType::SignedAlgebraicType(TypeMgr& owner)
    : AlgebraicType(owner)
    , f_width(0)
{
    f_repr = f_owner.em().make_abstract_signed_int_type();
}

SignedAlgebraicType::SignedAlgebraicType(TypeMgr& owner,
                                         unsigned width,
                                         ADD *dds)
    : AlgebraicType(owner)
    , f_width(width)
    , f_dds(dds)
{
    f_repr = f_owner.em().make_signed_int_type(width);
}

unsigned SignedAlgebraicType::size() const
{
    assert( 0 != f_width );
    return f_width;
}

bool SignedAlgebraicType::is_abstract() const
{ return 0 == f_width ; }

UnsignedAlgebraicType::UnsignedAlgebraicType(TypeMgr& owner)
    : AlgebraicType(owner)
    , f_width(0)
{
    f_repr = f_owner.em().make_abstract_unsigned_int_type();
}

UnsignedAlgebraicType::UnsignedAlgebraicType(TypeMgr& owner,
                                             unsigned width,
                                             ADD *dds)
    : AlgebraicType(owner)
    , f_width(width)
    , f_dds(dds)
{
    f_repr = f_owner.em().make_unsigned_int_type(width);
}

unsigned UnsignedAlgebraicType::size() const
{
    assert( 0 != f_width );
    return f_width;
}

bool UnsignedAlgebraicType::is_abstract() const
{ return 0 == f_width ; }

ArrayType::ArrayType(TypeMgr& owner, ScalarType_ptr of, unsigned nelems)
    : Type(owner)
    , f_of(of)
    , f_nelems(nelems)
{
    // 0 is reserved for abstract arrays
    assert (0 < nelems);

    // valid type
    assert( NULL != of);

    // scalar type, non-abstract only. Make sure we know how to calculate size()
    assert (f_of -> is_scalar() && ! f_of -> is_abstract());

    f_repr = f_owner.em().make_subscript( of->repr(),
                                          f_owner.em().make_iconst(nelems));
}

ArrayType::ArrayType(TypeMgr& owner, ScalarType_ptr of)
    : Type(owner)
    , f_of(of)
    , f_nelems(0)
{
    // valid type
    assert( NULL != of);

    // scalar type, non-abstract only. Consistency with comment above.
    assert (f_of -> is_scalar() && ! f_of -> is_abstract());

    f_repr = f_owner.em().make_abstract_array_type( of->repr());
}

unsigned ArrayType::size() const
{
    assert( 0 != f_nelems );
    return f_nelems * f_of -> size();
}

bool ArrayType::is_abstract() const
{ return 0 == f_nelems ; }

// ostream helper, uses FQExpr printer (see expr/expr.cc)
ostream& operator<<(ostream& os, Type_ptr type)
{ return os << type->repr(); }

Type::~Type()
{}
