/**
 *  @file ltl.hh
 *  @brief SAT-based LTL Algorithms for property checking
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

#ifndef LTL_ALGORITHM_H
#define LTL_ALGORITHM_H

#include <expr/expr.hh>

#include <algorithms/base.hh>
#include <witness/witness.hh>

class LTL : public Algorithm {

public:
    LTL(Command& command, Model& model);
    ~LTL();

    void process(const Expr_ptr phi);

    inline mc_status_t status() const
    { return f_status; }

    inline void set_status(mc_status_t status)
    { f_status = status; }

private:
    mc_status_t f_status;
};

/* Specialized for LTL CEX */
class LTLCounterExample : public Witness {
public:
    LTLCounterExample(Expr_ptr property, Model& model,
                      Engine& engine, unsigned k, bool use_coi);
};

#endif