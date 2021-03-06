/**
 * @file test_dd.cc
 * @brief DD subsystem unit tests.
 *
 * Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 **/

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <expr.hh>
#include <expr_mgr.hh>
#include <printer.hh>

#include <type.hh>
#include <type_mgr.hh>

#include <cuddObj.hh>

BOOST_AUTO_TEST_SUITE(tests)

BOOST_AUTO_TEST_CASE(dd_boolean)
{
    Cudd dd;
    ADD lhs = dd.addVar();
    ADD rhs = dd.addVar();
    ADD zero (dd.addZero());
    ADD one  (dd.addOne());

    {
        ADD neg_neg = lhs.Cmpl().Cmpl();
        BOOST_CHECK(lhs == neg_neg);
    }

    {
        ADD x_and_y = lhs.Times(rhs);
        ADD y_and_x = rhs.Times(lhs);
        BOOST_CHECK(x_and_y == y_and_x);
    }

    {
        ADD x_and_0 = lhs.Times(zero);
        BOOST_CHECK(x_and_0 == zero);

        ADD x_and_1 = rhs.Times(one);
        BOOST_CHECK(x_and_1 == rhs);

        ADD _0_and_y = zero.Times(rhs);
        BOOST_CHECK(_0_and_y == zero);

        ADD _1_and_y = one.Times(rhs);
        BOOST_CHECK(_1_and_y == rhs);
    }

    {
        ADD x_or_y = lhs.Or(rhs);
        ADD y_or_x = rhs.Or(lhs);
        BOOST_CHECK(x_or_y == y_or_x);
    }

    {
        ADD x_or_0 = lhs.Or(zero);
        BOOST_CHECK(x_or_0 == lhs);

        ADD x_or_1 = rhs.Or(one);
        BOOST_CHECK(x_or_1 == one);

        ADD _0_or_y = zero.Or(rhs);
        BOOST_CHECK(_0_or_y == rhs);

        ADD _1_or_y = one.Or(rhs);
        BOOST_CHECK(_1_or_y == one);
    }

    {
        ADD x_xor_y = lhs.Xor(rhs);
        ADD y_xor_x = rhs.Xor(lhs);
        ADD x_xor_x = lhs.Xor(lhs);
        ADD y_xor_y = rhs.Xor(rhs);
        ADD _0_xor_1 = zero.Xor(one);
        ADD _1_xor_0 = one.Xor(zero);
        BOOST_CHECK(x_xor_y == y_xor_x);
        BOOST_CHECK(x_xor_x == zero);
        BOOST_CHECK(y_xor_y == zero);
        BOOST_CHECK(_0_xor_1 == one);
        BOOST_CHECK(_1_xor_0 == one);
    }

    {
        ADD x_xnor_y = lhs.Xnor(rhs);
        ADD y_xnor_x = rhs.Xnor(lhs);
        BOOST_CHECK(x_xnor_y == y_xnor_x);
    }

    {
        ADD x_xnor_y = lhs.Xnor(rhs);
        ADD y_xnor_x = rhs.Xnor(lhs);
        BOOST_CHECK(x_xnor_y == y_xnor_x);
    }

    {
        ADD x_xnor_y = lhs.Xnor(rhs);
        ADD y_xnor_x = rhs.Xor(lhs).Cmpl();
        BOOST_CHECK(x_xnor_y == y_xnor_x);
    }
}

// duplicate code... :-/
ADD make_integer_encoding(Cudd& mgr, unsigned nbits, bool is_signed)
{
    bool msb = true;

    ADD res = mgr.addOne();

    assert(0 < nbits);
    unsigned i = 0;

    while (i < nbits) {
        ADD two = mgr.constant(2);
        if (msb && is_signed) {
            msb = false;
            two = two.Negate(); // MSB is -2^N in 2's complement
        }
        res *= two;

        // create var and book it
        ADD add_var = mgr.addVar();

        // add it to the encoding
        res += add_var;

        ++ i;
    }

    return res;
}


BOOST_AUTO_TEST_CASE(dd_arithmetic)
{
    Cudd dd;

    ADD lhs = make_integer_encoding(dd, 4, false);
    ADD rhs = make_integer_encoding(dd, 4, false);

    {
        ADD neg_neg = lhs.Negate().Negate();
        BOOST_CHECK(lhs == neg_neg);
    }

    {
        ADD x_plus_y = lhs.Plus(rhs);
        ADD y_plus_x = rhs.Plus(lhs);
        BOOST_CHECK(x_plus_y == y_plus_x);
    }

    {
        ADD x_times_y = lhs.Times(rhs);
        ADD y_times_x = rhs.Times(lhs);
        BOOST_CHECK(x_times_y == y_times_x);
    }

    {
        ADD x_minus_y = lhs.Minus(rhs);
        ADD y_minus_x = rhs.Minus(lhs);
        BOOST_CHECK(x_minus_y != y_minus_x);
    }

    {
        ADD x_divide_y = lhs.Divide(rhs);
        ADD y_divide_x = rhs.Divide(lhs);
        BOOST_CHECK(x_divide_y != y_divide_x);
    }

    {
        ADD x_mod_y = lhs.Modulus(rhs);
        ADD y_mod_x = rhs.Modulus(lhs);
        BOOST_CHECK(x_mod_y != y_mod_x);
    }
}

BOOST_AUTO_TEST_CASE(dd_bitwise)
{
    Cudd dd;
    ADD lhs = dd.addVar();
    ADD rhs = dd.addVar();

    {
        ADD neg_neg = lhs.Cmpl().Cmpl();
        BOOST_CHECK(lhs == neg_neg);
    }

    {
        ADD x_and_y = lhs.BWTimes(rhs);
        ADD y_and_x = rhs.BWTimes(lhs);
        BOOST_CHECK(x_and_y == y_and_x);
    }

    {
        ADD x_or_y = lhs.BWOr(rhs);
        ADD y_or_x = rhs.BWOr(lhs);
        BOOST_CHECK(x_or_y == y_or_x);
    }

    {
        ADD x_xor_y = lhs.BWXor(rhs);
        ADD y_xor_x = rhs.BWXor(lhs);
        BOOST_CHECK(x_xor_y == y_xor_x);
    }

    {
        ADD x_xnor_y = lhs.BWXnor(rhs);
        ADD y_xnor_x = rhs.BWXnor(lhs);
        BOOST_CHECK(x_xnor_y == y_xnor_x);
    }

    {
        ADD x_xnor_y = lhs.BWXnor(rhs);
        ADD y_xnor_x = rhs.BWXnor(lhs);
        BOOST_CHECK(x_xnor_y == y_xnor_x);
    }

}

BOOST_AUTO_TEST_CASE(dd_relational)
{
    Cudd dd;

    ADD lhs = make_integer_encoding(dd, 4, false);
    ADD rhs = make_integer_encoding(dd, 4, false);

    ADD x_equals_y = lhs.Equals(rhs);
    ADD y_equals_x = rhs.Equals(lhs);
    BOOST_CHECK(x_equals_y == y_equals_x);

    ADD x_lt_y = lhs.LT(rhs);
    ADD y_lt_x = rhs.LT(lhs);
    BOOST_CHECK(x_lt_y != y_lt_x);

    BOOST_CHECK( x_equals_y.Or(x_lt_y) == lhs.LEQ(rhs) );
}

BOOST_AUTO_TEST_SUITE_END()
