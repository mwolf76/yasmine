/**
 *  @file Base Algorithm.cc
 *  @brief Base Algorithm
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
#include <base.hh>

Algorithm::Algorithm(IModel& model, ostream &os)
    : f_model(model)
    , f_os(os)
    , f_mm(ModelMgr::INSTANCE())
    , f_em(ExprMgr::INSTANCE())
    , f_tm(TypeMgr::INSTANCE())
    , f_engine(* new SAT())
    , f_witness(NULL)
{
    set_param("alg_name", "test");
    DEBUG  << "Creating algorithm instance "
           << get_param("alg_name")
           << " @" << this
           << endl;
}

Algorithm::~Algorithm()
{
    DEBUG << "Destroying algorithm instance "
          << get_param("alg_name")
          << " @" << this
          << endl;

    delete & f_engine;
}

void Algorithm::set_param(string key, Variant value)
{ f_params [ key ] = value; }

Variant& Algorithm::get_param(const string key)
{
    const ParametersMap::iterator eye = f_params.find(key);

    if (eye != f_params.end()) {
        return (*eye).second;
    }

    else return NilValue;
}

void Algorithm::prepare()
{
    Compiler& cmpl(compiler()); // just a local ref
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

            cmpl.process(ctx, body, true);
            if (! true) {
                while (cmpl.has_next()) {
                    f_init_adds.push_back(cmpl.next());
                }
            }
        }

        /* INVAR */
        const ExprVector invar = module.invar();
        for (ExprVector::const_iterator invar_eye = invar.begin();
             invar_eye != invar.end(); ++ invar_eye) {

            Expr_ptr ctx = module.expr();
            Expr_ptr body = (*invar_eye);

            cmpl.process(ctx, body, true);
            if (! true) {
                while (cmpl.has_next()) {
                    f_invar_adds.push_back(cmpl.next());
                }
            }
        }

        /* TRANS */
        const ExprVector trans = module.trans();
        for (ExprVector::const_iterator trans_eye = trans.begin();
             trans_eye != trans.end(); ++ trans_eye) {

            Expr_ptr ctx = module.expr();
            Expr_ptr body = (*trans_eye);

            cmpl.process(ctx, body, true);
            if (! true) {
                while (cmpl.has_next()) {
                    f_trans_adds.push_back(cmpl.next());
                }
            }
        }
    }
} /* prepare() */

void Algorithm::compile()
{
    Compiler& cmpl(compiler()); // just a local ref
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

            cmpl.process(ctx, body, false);
            while (cmpl.has_next()) {
                f_init_adds.push_back(cmpl.next());
            }
        }

        /* INVAR */
        const ExprVector invar = module.invar();
        for (ExprVector::const_iterator invar_eye = invar.begin();
             invar_eye != invar.end(); ++ invar_eye) {

            Expr_ptr ctx = module.expr();
            Expr_ptr body = (*invar_eye);

            cmpl.process(ctx, body, false);
            while (cmpl.has_next()) {
                f_invar_adds.push_back(cmpl.next());
            }
        }

        /* TRANS */
        const ExprVector trans = module.trans();
        for (ExprVector::const_iterator trans_eye = trans.begin();
             trans_eye != trans.end(); ++ trans_eye) {

            Expr_ptr ctx = module.expr();
            Expr_ptr body = (*trans_eye);

            cmpl.process(ctx, body, false);
            while (cmpl.has_next()) {
                f_trans_adds.push_back(cmpl.next());
            }
        }
    }
} /* compile() */

void Algorithm::assert_fsm_init(step_t time, group_t group, color_t color)
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

void Algorithm::assert_fsm_invar(step_t time, group_t group, color_t color)
{
    clock_t t0 = clock();
    unsigned n = f_invar_adds.size();
    TRACE << "CNFizing INVARs @" << time
          << "... (" << n << " formulas)"
          << endl;

    ADDVector::iterator i;
    for (i = f_invar_adds.begin(); f_invar_adds.end() != i; ++ i) {
        engine().push( *i, time, group, color);
    }

    clock_t elapsed = clock() - t0;
    double secs = (double) elapsed / (double) CLOCKS_PER_SEC;
    TRACE << "Done. (took " << secs << " seconds)" << endl;
}

void Algorithm::assert_fsm_trans(step_t time, group_t group, color_t color)
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

void Algorithm::assert_formula(step_t time, ADDVector& adds,
                               group_t group, color_t color)
{
    clock_t t0 = clock();
    unsigned n = f_trans_adds.size();

    TRACE << "CNFizing formula @" << time
          << "... (" << n << " formulas)"
          << endl;

    ADDVector::iterator i;
    for (i = adds.begin(); adds.end() != i; ++ i) {
        engine().push( *i, time, group, color);
    }

    clock_t elapsed = clock() - t0;
    double secs = (double) elapsed / (double) CLOCKS_PER_SEC;
    TRACE << "Done. (took " << secs << " seconds)" << endl;
}
