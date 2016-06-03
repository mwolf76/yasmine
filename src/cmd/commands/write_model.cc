/*
 * @file write_model.cc
 * @brief Command-interpreter subsystem related classes and definitions.
 *
 * Copyright (C) 2012 Marco Pensallorto < marco AT pensallorto DOT gmail DOT com >
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 **/
#include <cstdlib>
#include <cstring>

#include <cmd/commands/write_model.hh>
#include <model/model.hh>
#include <model/model_mgr.hh>

WriteModel::WriteModel(Interpreter& owner)
    : Command(owner)
    , f_output(NULL)
{}

WriteModel::~WriteModel()
{
    free(f_output);
    f_output = NULL;
}

void WriteModel::set_output(pconst_char output)
{
    if (output) {
        free(f_output);
        f_output = strdup(output);
    }
}

Variant WriteModel::operator()()
{
    std::ostringstream oss;

    Model& model
        (ModelMgr::INSTANCE().model());

    oss
        << std::endl
        << std::endl;

    const Modules& modules (model.modules());
    for (Modules::const_iterator m = modules.begin();
         m != modules.end(); ++ m) {

        Module& module = dynamic_cast <Module&> (*m->second);

        oss
            << "MODULE "
            << module.name()
            << std::endl;

        /* INIT */
        const ExprVector init = module.init();
        if (init.begin() != init.end())
            oss << std::endl;
        for (ExprVector::const_iterator init_eye = init.begin();
             init_eye != init.end(); ++ init_eye) {

            Expr_ptr body (*init_eye);

            oss
                << "INIT "
                << body << ";"
                << std::endl;

        }

        /* INVAR */
        const ExprVector invar = module.invar();
        if (invar.begin() != invar.end())
            oss << std::endl;
        for (ExprVector::const_iterator invar_eye = invar.begin();
             invar_eye != invar.end(); ++ invar_eye) {

            Expr_ptr body (*invar_eye);

            oss
                << "INVAR "
                << body << ";"
                << std::endl;
        }

        /* TRANS */
        const ExprVector trans = module.trans();
        if (trans.begin() != trans.end())
            oss << std::endl;
        for (ExprVector::const_iterator trans_eye = trans.begin();
             trans_eye != trans.end(); ++ trans_eye) {

            Expr_ptr body (*trans_eye);

            oss
                << "TRANS "
                << body << ";"
                << std::endl;
        }
    }

    return Variant (oss.str());
}

WriteModelTopic::WriteModelTopic(Interpreter& owner)
    : CommandTopic(owner)
{}

WriteModelTopic::~WriteModelTopic()
{
    TRACE
        << "Destroyed write-model topic"
        << std::endl;
}

void WriteModelTopic::usage()
{
    std::cout
        << "write-model [<filename>] - Write current model to given filename[*].\n"
        << "[*] either in single or double quotes. If no filename is given, model is written to standard output";
}
