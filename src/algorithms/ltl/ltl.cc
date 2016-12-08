/**
 * @file ltl.cc
 * @brief SAT-based SBMC Algorithm for LTL properties checking
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

#include <boost/thread.hpp>

#include <algorithms/ltl/ltl.hh>
#include <witness_mgr.hh>

LTL::LTL(Command& command, Model& model)
    : Algorithm(command, model)
{
    const void* instance(this);
    setup();
    DRIVEL
        << "Created LTL @"
        << instance
        << std::endl;
}

LTL::~LTL()
{
    const void* instance(this);
    DRIVEL
        << "Destroyed LTL @"
        << instance
        << std::endl;
}

void LTL::process(const Expr_ptr phi)
{
    set_status( LTL_UNKNOWN );
    assert(false); // XXX: TODO

    TRACE << "Done." << std::endl;
}
