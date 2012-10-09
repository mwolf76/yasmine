/**
 *  @file type_mgr.cc
 *  @brief Type system classes (TypeMgr)
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

// static initialization
TypeMgr_ptr TypeMgr::f_instance = NULL;

TypeMgr::TypeMgr()
    : f_register()
    , f_em(ExprMgr::INSTANCE())
{
    // register predefined types
    register_type( f_em.make_boolean_type(),
                   new BooleanType(*this));

    register_type( f_em.make_integer_type(),
                   new IntegerType(*this));
}

const Type_ptr TypeMgr::find_unsigned(unsigned bits)
{
    Expr_ptr descr(f_em.make_unsigned_type(bits));
    Type_ptr res = lookup_type(descr);
    if (NULL != res) return res;

    // new type, needs to be registered before returning
    res = new AlgebraicType(*this, bits, false);
    register_type(descr, res);
    return res;
}

const Type_ptr TypeMgr::find_signed(unsigned bits)
{
    Expr_ptr descr(f_em.make_signed_type(bits));
    Type_ptr res = lookup_type(descr);
    if (NULL != res) return res;

    // new type, needs to be registered before returning
    res = new AlgebraicType(*this, bits, true);  // signed
    register_type(descr, res);
    return res;
}

const Type_ptr TypeMgr::find_range(const Expr_ptr min, const Expr_ptr max)
{
    // normalize range
    assert (f_em.is_numeric(min));
    value_t min_ = min->value();

    assert (f_em.is_numeric(max));
    value_t max_ = max->value();

    // inverted range? ok, not polite but supported :-)
    if (max_ < min_) { value_t tmp = max_; max_ = min_; min_ = tmp; }

    const Expr_ptr min_expr = f_em.make_iconst(min_);
    const Expr_ptr max_expr = f_em.make_iconst(max_);

    Expr_ptr descr(f_em.make_range_type(min_expr, max_expr));
    Type_ptr res = lookup_type(descr);
    if (NULL != res) return res;

    // new type, needs to be registered before returning
    res = new IntRangeType(*this, min_, max_);
    register_type(descr, res);
    return res;
}

const Type_ptr TypeMgr::find_enum(ExprSet& lits)
{
    /*
       IMPORTANT: lits ordering has to be canonical for enum types to
       work as expected! Otherwise same set of lits with different
       ordering could be mistakingly seen as a different type.
    */

    Expr_ptr descr(f_em.make_enum_type(lits));
    Type_ptr res = lookup_type(descr);
    if (NULL != res) return res;

    // new type, needs to be registered before returning
    res = new EnumType(*this, lits);
    register_type(descr, res);
    return res;
}

const Type_ptr TypeMgr::find_instance(Expr_ptr identifier)
{
    Expr_ptr descr(f_em.make_params(identifier, NULL));
    Type_ptr res = lookup_type(descr);
    if (NULL != res) return res;

    // new type, needs to be registered before returning
    res = new Instance(*this, identifier);
    register_type(descr, res);
    return res;
}

bool TypeMgr::is_boolean(const Type_ptr tp) const
{
    return NULL != dynamic_cast <const BooleanType_ptr> (tp);
}

BooleanType_ptr TypeMgr::as_boolean(const Type_ptr tp) const
{
    BooleanType_ptr res = dynamic_cast <const BooleanType_ptr> (tp);
    assert(res);

    return res;
}

bool TypeMgr::is_integer(const Type_ptr tp) const
{
    return (NULL != dynamic_cast<const IntegerType_ptr>(tp));
}

IntegerType_ptr TypeMgr::as_integer(const Type_ptr tp) const
{
    IntegerType_ptr res = dynamic_cast<const IntegerType_ptr>(tp);
    assert(res);

    return res;
}

bool TypeMgr::is_int_range(const Type_ptr tp) const
{
    return NULL != dynamic_cast <const IntRangeType_ptr> (tp);
}

IntRangeType_ptr TypeMgr::as_int_range(const Type_ptr tp) const
{
    IntRangeType_ptr res = dynamic_cast <IntRangeType_ptr> (tp);
    assert(res);

    return res;
}

bool TypeMgr::is_enum(const Type_ptr tp) const
{
    return NULL != dynamic_cast <const EnumType_ptr> (tp);
}

bool TypeMgr::is_int_enum(const Type_ptr tp) const
{
    EnumType_ptr tmp;
    return (! (tmp = dynamic_cast <const EnumType_ptr> (tp)))
        ? false
        : ! tmp->has_symbs();
}

EnumType_ptr TypeMgr::as_enum(const Type_ptr tp) const
{
    EnumType_ptr res = dynamic_cast<EnumType_ptr> (tp);
    assert(res);

    return res;
}

bool TypeMgr::is_int_algebraic(const Type_ptr tp) const
{
    return (NULL != dynamic_cast <AlgebraicType*> (tp));
}

AlgebraicType_ptr TypeMgr::as_int_algebraic(const Type_ptr tp) const
{
    AlgebraicType_ptr res = dynamic_cast <const AlgebraicType_ptr> (tp);
    assert(res);

    return res;
}

bool TypeMgr::is_instance(const Type_ptr tp) const
{
    return (NULL != dynamic_cast <Instance_ptr> (tp));
}

Instance_ptr TypeMgr::as_instance(const Type_ptr tp) const
{
    Instance_ptr res = dynamic_cast <Instance_ptr> (tp);
    assert(res);

    return res;
}

void TypeMgr::register_type(const Expr_ptr expr, Type_ptr vtype)
{
    assert ((NULL != expr) && (NULL != vtype) && (! lookup_type(expr)));
    DEBUG << "Registering new type: " << expr << endl;

    f_register [ expr ] = vtype;
}
