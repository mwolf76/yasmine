/**
 *  @file leaves.cc
 *
 *  Copyright (C) 2011-2015 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
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
#include <utility>
#include <compiler.hh>

#include <proxy.hh>

static inline value_t pow2(unsigned exp);

void Compiler::walk_leaf(const Expr_ptr expr)
{
    if (! cache_miss(expr))
        return;

    ExprMgr& em
        (f_owner.em());
    TypeMgr& tm
        (f_owner.tm());

    Expr_ptr ctx
        (f_ctx_stack.back());
    Expr_ptr full
        (em.make_dot(ctx, expr));
    step_t time
        (f_time_stack.back());

    /* 1. Explicit int or fxd constants, perform booleanization *
     * immediately. An exception will be thrown if conversion could
     * not be completed. */
    if (em.is_int_numeric(expr)) {
        unsigned ww
            (OptsMgr::INSTANCE().word_width());
        f_type_stack.push_back(tm.find_unsigned(ww, false));
        algebraic_constant(expr, ww);
        return;
    }
    if (em.is_fxd_numeric(expr)) {
        unsigned ww
            (OptsMgr::INSTANCE().word_width());
        f_type_stack.push_back(tm.find_unsigned(ww, true));
        algebraic_constant(expr, ww);
        return;
    }

    ResolverProxy resolver;
    Symbol_ptr symb
        (resolver.symbol(full));

    /* 2. bool constant leaves */
    if (symb->is_const()) {
        Constant& konst (symb->as_const());

        f_type_stack.push_back(konst.type());
        f_add_stack.push_back(f_enc.constant(konst.value()));
        return;
    }

    /* 3. enum literals */
    if (symb->is_literal()) {
        Literal& lit (symb->as_literal());

        // push into type stack
        Type_ptr type
            (lit.type());

        // if encoding for variable is available reuse it,
        // otherwise create and cache it.
        TimedExpr key
            (full, time);

        Encoding_ptr enc
            (f_enc.find_encoding(key));

        /* build a new encoding for this symbol if none is available. */
        if (!enc) {
            enc = f_enc.make_encoding(type);
            f_enc.register_encoding(key, enc);
        }

        EnumEncoding_ptr eenc
            (dynamic_cast<EnumEncoding_ptr>(enc));
        assert( NULL != eenc );

        f_type_stack.push_back(type);
        f_add_stack.push_back(f_enc.constant(eenc -> value(expr)));
        return;
    }

    /* 4. variables, encodings will be created on-the-fly, if
     *    necessary */
    else if (symb->is_variable()) {

        Type_ptr type
            (symb->as_variable().type());

        if (type -> is_instance()) {
            f_type_stack.push_back(type);
            return;
        }

        TimedExpr key
            (full, time);

        Encoding_ptr enc
            (find_encoding(key, type));

        push_dds(enc, type);
        return;
    }

    /* 5. parameters, must be resolved against the Param map which is
     *    maintained by the ModelMgr */
    else if (symb->is_parameter()) {
        Expr_ptr rewrite
            (f_owner.rewrite_parameter(full));

        f_ctx_stack.push_back( rewrite -> lhs());
        (*this) (rewrite -> rhs());
        f_ctx_stack.pop_back();
        return;
    }

    /* 6. defines, simply compile them recursively :-) */
    else if (symb->is_define()) {
        Expr_ptr body
            (symb -> as_define().body());

        (*this) (body);
        return;
    }

    assert( false ); /* give up, TODO: exception */
}

/* private service of walk_leaf */
void Compiler::push_dds(Encoding_ptr enc, Type_ptr type)
{
    assert (NULL != enc);
    DDVector& dds = enc->dv();
    unsigned width = dds.size();
    assert( 0 < width );

    // push into type stack
    f_type_stack.push_back(type);

    /* booleans, monoliths are just one DD */
    if (type->is_monolithic())
        f_add_stack.push_back(dds[0]);

    /* algebraics, reversed list of encoding DDs */
    else if (type->is_algebraic()) {
        // type and enc width info has to match
        assert( type -> as_algebraic()-> width() == width );
        for (DDVector::reverse_iterator ri = dds.rbegin(); ri != dds.rend(); ++ ri)
            f_add_stack.push_back(*ri);
    }

    /* array of algebraics, same as above, times nelems */
    else if (type->is_array()) {
        // type and enc width info has to match
        assert( type -> as_array() -> of() -> as_algebraic()-> width() ==
                width / type -> as_array() -> nelems());
        for (DDVector::reverse_iterator ri = dds.rbegin(); ri != dds.rend(); ++ ri)
            f_add_stack.push_back(*ri);
    }

    else assert( false ); // unexpected
}

static inline value_t pow2(unsigned exp)
{
    value_t res = 1;

    if ( !exp )
        return res;
    ++ res;

    while ( -- exp )
        res <<= 1;

    return res;
}

/* encodes constant value into a DD vector */
void Compiler::algebraic_constant(Expr_ptr konst, unsigned width)
{
    const unsigned base
        (2);

    value_t value
        (konst -> value());

    if (value < 0)
        value += pow2(width); // 2's complement

    for (unsigned i = 0; i < width; ++ i) {
        ADD digit
            (f_enc.constant(value % base));

        f_add_stack.push_back(digit);
        value /= base;
    }

    if (value)
        throw ConstantTooLarge(konst);
}
