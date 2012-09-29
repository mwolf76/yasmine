/*
 * @file command.cc
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
#include <common.hh>
#include <command.hh>

#include <expr.hh>
#include <mc.hh>
#include <sat.hh>
#include <expr/compilers/be_compiler.hh>

Command::Command(Interpreter& owner)
    : f_owner(owner)
{ DEBUG << "Initialized command @" << this << endl; }

Command::~Command()
{ DEBUG << "Deinitialized command @" << this << endl; }

LoadModelCommand::LoadModelCommand(Interpreter& owner, const string &filename)
    : Command(owner)
    , f_filename(filename)
{}

extern void parseFile(const char *filepath); // in utils.cc
Variant LoadModelCommand::operator()()
{
    parseFile(f_filename.c_str());
    return Variant("OK");
}

LoadModelCommand::~LoadModelCommand()
{}

// -- Normalize ---------------------------------------------------------------
NormalizeCommand::NormalizeCommand(Interpreter& owner, Expr_ptr expr)
    : Command(owner)
    , f_expr(expr)
{}

// sends a signal to the owner
Variant NormalizeCommand::operator()()
{
}

NormalizeCommand::~NormalizeCommand()
{}

// -- SAT ----------------------------------------------------------------------
SATCommand::SATCommand(Interpreter& owner, Expr_ptr expr)
    : Command(owner)
    , f_factory(CuddMgr::INSTANCE().dd())
    , f_engine(f_factory)
    , f_compiler()
    , f_expr(expr)
{
    DEBUG << "Expr @" << f_expr << endl;
}

// sends a signal to the owner
Variant SATCommand::operator()()
{
    Expr_ptr ctx = ExprMgr::INSTANCE().make_main(); // default ctx
    TRACE << "compiling EXPR " << ctx << "::" << f_expr << endl;

    ADD add = f_compiler.process(ctx, f_expr, 0);
    f_engine.push(add);

    ostringstream tmp;
    tmp << f_engine.solve();

    return Variant(tmp.str());
}

SATCommand::~SATCommand()
{}

// -- QUIT ---------------------------------------------------------------------
QuitCommand::QuitCommand(Interpreter& owner, int retcode)
    : Command(owner)
    , f_retcode(retcode)
{}

// sends a signal to the owner
Variant QuitCommand::operator()()
{
    f_owner.quit(f_retcode);
    return Variant("BYE");
}

QuitCommand::~QuitCommand()
{}