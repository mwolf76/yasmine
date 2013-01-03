/**
 *  @file MC Algorithm.cc
 *  @brief MC Algorithm
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
#include <mc.hh>
using namespace Minisat;

MCAlgorithm::MCAlgorithm(IModel& model, Expr_ptr property)
    : Algorithm()
    , f_model(model)
    , f_property(property)
    , f_witness(NULL)
{
    set_param("alg_name", "test");
    DEBUG  << "Creating MC algoritm instance "
           << get_param("alg_name")
           << " @" << this
           << endl;

        /* internal structures are empty */
    assert( 0 == f_init_adds.size() );
    assert( 0 == f_trans_adds.size() );

    const Modules& modules = f_model.modules();
    for (Modules::const_iterator m = modules.begin();
         m != modules.end(); ++ m) {

        Module& module = dynamic_cast <Module&> (*m->second);

        /* INIT */
        const ExprVector init = module.init();
        for (ExprVector::const_iterator init_eye = init.begin();
             init_eye != init.end(); ++ init_eye) {

            Expr_ptr ctx = module.expr();
            Expr_ptr body = (*init_eye);

            f_init_adds.push_back(compiler().process(ctx, body));
        }

        /* TRANS */
        const ExprVector trans = module.trans();
        for (ExprVector::const_iterator trans_eye = trans.begin();
             trans_eye != trans.end(); ++ trans_eye) {

            Expr_ptr ctx = module.expr();
            Expr_ptr body = (*trans_eye);

            f_trans_adds.push_back(compiler().process(ctx, body));
        }
    }

    /* Invariant */
    f_invariant_add = compiler().process( em().make_main(), f_property);

    /* Violation (negation of Invarion) */
    f_violation_add = compiler().process( em().make_main(),
                                          em().make_not( f_property));
}

MCAlgorithm::~MCAlgorithm()
{
    DEBUG << "Destroying MC algoritm instance"
          << get_param("alg_name")
          << " @" << this
          << endl;
}

mc_status_t MCAlgorithm::status() const
{ return f_status; }

bool MCAlgorithm::has_witness() const
{ return NULL != f_witness; }

Witness& MCAlgorithm::get_witness() const
{
    assert (has_witness());
    return *f_witness;
}

void MCAlgorithm::set_status(mc_status_t status)
{ f_status = status; }

void MCAlgorithm::assert_violation(step_t time, group_t group, color_t color)
{
    // TODO: macro to wrap code to be benchmarked (cool!)
    clock_t t0 = clock();
    TRACE << "CNFizing Violation @"
          << time << endl;

    engine().push( f_violation_add, time, group, color);

    clock_t elapsed = clock() - t0;
    double secs = (double) elapsed / (double) CLOCKS_PER_SEC;
    TRACE << "Done. (took " << secs << " seconds)" << endl;
}

void MCAlgorithm::assert_fsm_init(step_t time, group_t group, color_t color)
{
    clock_t t0 = clock();
    unsigned n = f_init_adds.size();
    TRACE << "CNFizing INITs @" << time
          << "... (" << n << " formulas)"
          << endl;

    ADDVector::iterator i;
    for (i = f_init_adds.begin(); f_init_adds.end() != i; ++ i) {
        engine().push( *i, time, group, color);
    }

    clock_t elapsed = clock() - t0;
    double secs = (double) elapsed / (double) CLOCKS_PER_SEC;
    TRACE << "Done. (took " << secs << " seconds)" << endl;
}

void MCAlgorithm::assert_fsm_trans(step_t time, group_t group, color_t color)
{
    clock_t t0 = clock();
    unsigned n = f_trans_adds.size();

    TRACE << "CNFizing TRANSes @" << time
          << "... (" << n << " formulas)"
          << endl;

    ADDVector::iterator i;
    for (i = f_trans_adds.begin(); f_trans_adds.end() != i; ++ i) {
        engine().push( *i, time, group, color);
    }

    clock_t elapsed = clock() - t0;
    double secs = (double) elapsed / (double) CLOCKS_PER_SEC;
    TRACE << "Done. (took " << secs << " seconds)" << endl;
}

SlaveAlgorithm::SlaveAlgorithm(MCAlgorithm& master)
    : Algorithm()
    , f_master(master)
{
}

SlaveAlgorithm::~SlaveAlgorithm()
{
    DEBUG << "Destroying MC algoritm instance"
          << get_param("alg_name")
          << " @" << this
          << endl;
}