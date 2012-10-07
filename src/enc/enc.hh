/**
 *  @file enc.hh
 *  @brief Encoder module
 *
 *  This module contains definitions and services that implement an
 *  encoder for symbols. For each symbol a boolean encoding is
 *  maintained, the encoder takes care of ADD variables definitions
 *  and is provides mapback services as well.
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

#ifndef ENCODER_H
#define ENCODER_H

#include <common.hh>
#include <expr.hh>
#include <expr_mgr.hh>

#include <type.hh>

#include <cudd_mgr.hh>
#include <enc_mgr.hh>

// -- primary decls  --------------------------------------------------------------
typedef class IEncoding *IEncoding_ptr;
class IEncoding : public IObject {
public:
    virtual ADD add() const =0;
};

class Encoding : public IEncoding {
public:
    ADD add() const
    { return f_add; }

protected:
    Encoding()
        : f_mgr(EncodingMgr::INSTANCE())
        , f_add(f_mgr.dd().addZero())
    {}

    virtual ~Encoding() =0;

    EncodingMgr& f_mgr;
    EncodingBits f_bits;
    ADD f_add;

    // shared services
    unsigned range_repr_bits (value_t range)
    { return ceil(log2(range)); }

    ADD make_integer_encoding(unsigned nbits, bool is_signed =false);
};


// 1-bit boolean var (identity encoding)
class BooleanEncoding : public Encoding {
friend class EncodingMgr; // expose ctors only to mgr
protected:
    virtual ~BooleanEncoding()
    { assert(0); }

    // boolean 1(1 bit) var
    BooleanEncoding();
};


// n-bits 2's complement signed integer
class IntEncoding : public Encoding {
friend class EncodingMgr; // expose ctors only to mgr
protected:
    virtual ~IntEncoding()
    { assert(0); }

    IntEncoding(unsigned nbits, bool is_signed);
};

// 2's complement ranged integer
class RangeEncoding : public Encoding {
friend class EncodingMgr; // expose ctors only to mgr
protected:
    virtual ~RangeEncoding()
    { assert(0); }

    // n bits bounded integer var (D = 0..2^n -1)
    RangeEncoding(value_t min_value, value_t max_value);

    value_t f_min;
    value_t f_max;
};

// finite enumerative
class EnumEncoding : public Encoding {
friend class EncodingMgr; // expose ctors only to mgr
protected:
    virtual ~EnumEncoding()
    { assert(0); }

    // n bits bounded integer var (D = 0..2^n -1)
    EnumEncoding(ExprSet lits);

    ExprSet f_lits;
};

#endif