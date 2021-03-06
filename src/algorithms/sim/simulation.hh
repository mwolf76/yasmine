/**
 * @file simulation.hh
 * @brief SAT-based BMC simulation algorithm
 *
 * This header file contains the declarations required to implement
 * the FSM simulation algorithm.
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

#ifndef SIMULATION_ALGORITHM_H
#define SIMULATION_ALGORITHM_H

#include <algorithms/base.hh>

#include <witness/witness.hh>
#include <witness/witness_mgr.hh>

#include <cmd/command.hh>

typedef enum {
    SIMULATION_DONE,
    SIMULATION_INITIALIZED,
    SIMULATION_DEADLOCKED,
    SIMULATION_INTERRUPTED,
    SIMULATION_HALTED,
} simulation_status_t;

class Simulation : public Algorithm {

public:
    Simulation(Command& command, Model& model);
    ~Simulation();

    void pick_state(bool allsat,
                    value_t limit,
                    ExprVector constraints);

    void simulate(Expr_ptr invar_condition,
                  Expr_ptr until_condition,
                  ExprVector constraints,
                  step_t k,
                  pconst_char trace_uid);

    inline simulation_status_t status() const
    { return f_status; }

private:
    /* None of 'em, one of 'em, not both. */
    Expr_ptr f_halt_cond;
    Expr_ptr f_nsteps;

    ExprVector f_constraints;

    simulation_status_t f_status;
};

class SimulationWitness : public Witness {

public:
    SimulationWitness(Model& model, Engine& engine, step_t k);
};

#endif /* SIMULATION_ALGORITHM_H */
