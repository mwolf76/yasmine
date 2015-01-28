/**
 *  @file expr.hh
 *  @brief Expression management
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
#ifndef EXPR_H
#define EXPR_H

#include <vector>
#include <set>

#include <common.hh>

typedef enum {

    // -- linear temporal logic expressions ------------------------------------
    F, G, X, U, R,

    // -- primary expressions --------------------------------------------------

    /* time shift operator (nesting supported) */
    NEXT,

    /* arithmetical operators */
    NEG, PLUS, SUB, DIV, MUL, MOD,

    /* bitwise operators */
    BW_NOT, BW_AND, BW_OR, BW_XOR, BW_XNOR,

    /* logical operators */
    NOT, AND, OR, IMPLIES,

    /* shift operators */
    LSHIFT, RSHIFT,

    /* type operators */
    TYPE, CAST,

    /* relational operators */
    EQ, NE, GE, GT, LE, LT,

    /* conditionals */
    ITE, COND,

    /* identifiers */
    IDENT, DOT,

    /* declarations */
    BOOL, SIGNED, UNSIGNED,

    /* defines */
    PARAMS, // (), binary

    /* arrays */
    SUBSCRIPT, // [], binary

    // TODO
    SET, // {}, unary

    COMMA,

    // -- Nullary
    ICONST, // decimal constants
    HCONST, // hex constants
    OCONST, // octal constants

    FCONST, // fxd constants

    // undefined
    UNDEF,

} ExprType;

// An Expression consists of an AST symbol, which is the expression
// main operator, operands which depend on the type of operator and a
// context, in which the expression has to evaluated.

/* using STL string as basic atom class */
typedef std::string Atom;
typedef Atom* Atom_ptr;

typedef struct Expr_TAG *Expr_ptr;
typedef struct Expr_TAG {

    // AST symb type
    ExprType f_symb;

    union {
        // identifiers
        Atom_ptr f_atom;

        // numeric constants
        value_t f_value;

        // operators
        struct  {
            Expr_ptr f_lhs;
            Expr_ptr f_rhs; // NULL for unary ops
        };
    } u;

    // accessors
    inline ExprType symb() const
    { return f_symb; }

    inline Atom& atom() const
    {
        assert (IDENT == f_symb);
        return *u.f_atom;
    }

    inline value_t value()
    {
        assert (ICONST == f_symb ||
                FCONST == f_symb ||
                HCONST == f_symb ||
                OCONST == f_symb );
        return u.f_value;
    }

    inline Expr_ptr lhs()
    { return u.f_lhs; }
    inline Expr_ptr rhs()
    { return u.f_rhs; }

    // identifiers
    inline Expr_TAG(const Atom& atom)
    : f_symb(IDENT)
    { u.f_atom = const_cast<Atom *>(& atom); }

    // binary expr (rhs is NULL for unary ops)
    inline Expr_TAG(ExprType symb, Expr_ptr lhs, Expr_ptr rhs)
        : f_symb(symb)
    {
        u.f_lhs = lhs;
        u.f_rhs = rhs;
    }

    // numeric constants, are treated as machine size consts.
    inline Expr_TAG(ExprType symb, value_t value)
        : f_symb(symb)
    {
        assert (symb == ICONST ||
                symb == FCONST ||
                symb == HCONST ||
                symb == OCONST );

        u.f_value = value;
    }

    // nullary nodes (errors, undefined)
    inline Expr_TAG(ExprType symb)
        : f_symb(symb)
    {
        assert( symb == UNDEF );
    }

} Expr;

typedef std::vector<Expr_ptr> ExprVector;
typedef ExprVector* ExprVector_ptr;

struct LexicographicOrdering {
    int operator() (const Expr_ptr x, const Expr_ptr y) const;
};

typedef std::set<Expr_ptr, LexicographicOrdering> ExprSet;
typedef ExprSet* ExprSet_ptr;

// TODO: split this into multiple headers

class TimedExpr {
public:
    TimedExpr(Expr_ptr expr, step_t time);

    inline Expr_ptr expr() const
    { return f_expr; }

    inline step_t time() const
    { return f_time; }

    inline bool operator==(const TimedExpr& other)
    { return f_expr == other.f_expr && f_time == other.f_time; }

private:
    Expr_ptr f_expr;
    step_t f_time;
};

/* Normal forms literals */
class PolarizedLiteral {
public:
    PolarizedLiteral(Expr_ptr literal, bool polarity = true);

    inline Expr_ptr literal() const
    { return f_literal; }

    inline bool polarity() const
    { return f_polarity; }

private:
    // literal
    Expr_ptr f_literal;

    // polarity
    bool f_polarity;
};

/* Untimed Canonical Bit Identifiers */
class UCBI {
public:
    UCBI(Expr_ptr expr, step_t time_ofs, unsigned bitno);
    UCBI(const UCBI& ucbi);

    inline Expr_ptr expr() const
    { return f_expr; }

    inline step_t time() const
    { return f_time; }

    inline unsigned bitno() const
    { return f_bitno; }

private:
    // expression body
    Expr_ptr f_expr;

    // expression time (default is 0)
    step_t f_time;

    // bit number
    unsigned f_bitno;
};

/* Timed Canonical Bit Identifiers */
class TCBI {
public:
    // TCBI(Expr_ptr expr, step_t timeofs, unsigned bitno, step_t timebase);
    TCBI(const UCBI& ucbi, step_t timebase);
    TCBI(const TCBI& tcbi);

    inline Expr_ptr expr() const
    { return f_expr; }

    inline step_t time() const
    { return f_time; }

    inline unsigned bitno() const
    { return f_bitno; }

    inline step_t base() const
    { return f_base; }

private:

    // expression body
    Expr_ptr f_expr;

    // expression time (default is 0)
    step_t f_time;

    // bit number
    unsigned f_bitno;

    // time base (default is 0)
    step_t f_base;
};

std::ostream& operator<<(std::ostream& os, const Expr_ptr expr);
std::ostream& operator<<(std::ostream& os, const TimedExpr& expr);
std::ostream& operator<<(std::ostream& os, const UCBI& ucbi);
std::ostream& operator<<(std::ostream& os, const TCBI& tcbi);

/** -- shortcurts to simplify the manipulation of the internal ctx stack -- */
#define TOP_CTX(tp)                            \
    const Expr_ptr (tp)(f_ctx_stack.back())

#define DROP_CTX()                             \
    f_ctx_stack.pop_back()

#define POP_CTX(tp)                            \
    TOP_CTX(tp); DROP_CTX()

#define PUSH_CTX(tp)                           \
    f_ctx_stack.push_back(tp)

/** -- shortcurts to simplify the manipulation of the internal time stack -- */
#define TOP_TIME(step)                          \
    const step_t (step)(f_time_stack.back())

#define DROP_TIME()                             \
    f_time_stack.pop_back()

#define POP_TIME(step)                          \
    TOP_TIME(step); DROP_TIME()

#define PUSH_TIME(step)                         \
    f_time_stack.push_back(step)

#endif
