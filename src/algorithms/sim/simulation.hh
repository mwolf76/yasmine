/**
 *  @file simulation.hh
 *  @brief SAT-based BMC simulation algorithm
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

#ifndef SIMULATION_ALGORITHM_H
#define SIMULATION_ALGORITHM_H

#include <base.hh>
#include <witness.hh>
#include <witness_mgr.hh>

typedef enum {
    SIMULATION_UNSAT,
    SIMULATION_SAT,
    SIMULATION_UNKNOWN,
} simulation_status_t;

class Simulation : public Algorithm {

public:
    Simulation(IModel& model, Expr_ptr halt_cond,
               Expr_ptr witness_id, ExprVector& constraints);

    ~Simulation();

    void process();

    inline simulation_status_t status() const
    { return f_status; }

    const inline Expr_ptr halt_cond() const
    { return f_halt_cond; }

private:
    Expr_ptr f_halt_cond;
    ExprVector f_constraints;

    simulation_status_t f_status;
    void set_status(simulation_status_t status);
};

class SimulationWitness : public Witness {

public:
    SimulationWitness(IModel& model, SAT& engine, step_t k);
};

#endif
