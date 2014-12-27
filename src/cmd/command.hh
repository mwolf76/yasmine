/*
 * @file command.hh
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
#ifndef COMMAND_H
#define COMMAND_H

#include <common.hh>
#include <variant.hh>
#include <interpreter.hh>
#include <expr.hh>
#include <compiler/compiler.hh>
#include <sat.hh>

/* algorithms */
#include <init.hh>
#include <bmc/bmc.hh>
#include <sim/simulation.hh>

class ICommand : public IObject {
public:
    // functor-pattern
    Variant virtual operator()() =0;

    virtual bool blocking() const =0;

    // representation
    friend ostream& operator<<(ostream& os, ICommand& cmd);
};

// -- command definitions --------------------------------------------------
class Command : public ICommand {
public:
    Command(Interpreter& owner);
    virtual ~Command();

protected:
    Interpreter& f_owner;
};

typedef class ICommand* Command_ptr;

class LoadModelCommand : public Command {
public:
    // from FILE
    LoadModelCommand(Interpreter& owner, const string& filename);
    virtual ~LoadModelCommand();

    virtual bool blocking() const
    { return true; }

    Variant virtual operator()();

private:
    string f_filename;
};

class HelpCommand : public Command {
public:
    HelpCommand(Interpreter& owner, Atom topic);
    virtual ~HelpCommand();

    virtual bool blocking() const
    { return true; }

    Variant virtual operator()();

private:
    Atom f_topic;
};

class TimeCommand : public Command {
public:
    TimeCommand(Interpreter& owner);
    virtual ~TimeCommand();

    virtual bool blocking() const
    { return true; }

    Variant virtual operator()();
};

class InitCommand : public Command {
public:
    InitCommand(Interpreter& owner);
    virtual ~InitCommand();

    virtual bool blocking() const
    { return true; }

    Variant virtual operator()();

private:
    Init f_init;
};

/* Performs a new simulation with given constraints and halting
   conditions. Simulation can be resumed unless it's last status is
   UNSAT. */
class SimulateCommand : public Command {
public:
    SimulateCommand(Interpreter& owner,
                    Expr_ptr halt_cond,
                    Expr_ptr resume_id,
                    ExprVector& constraints);

    virtual ~SimulateCommand();

    virtual bool blocking() const
    { return true; } // TODO: change to false

    Variant virtual operator()();

private:
    // Simulation machinery
    Simulation f_sim;

    // simulation constraints (optional)
    Expr_ptr f_expr;

    // HALT condition (optional)
    Expr_ptr f_halt;

    // Witness id
    Expr_ptr f_witness;
};


class VerifyCommand : public Command {
public:
    VerifyCommand(Interpreter& owner, Expr_ptr expr, ExprVector& constraints);
    virtual ~VerifyCommand();

    virtual bool blocking() const
    { return true; } // TODO: change to false

    Variant virtual operator()();

private:
    // BMC machinery
    BMC f_bmc;

    // the formula to be verifyed
    Expr_ptr f_property;

    // additional constraints (optional)
    Expr_ptr f_constraints;
};

class FormatCommand : public Command {
public:
    FormatCommand(const char *fmt, ...);
    virtual ~FormatCommand();

    virtual bool blocking() const
    { return true; }

    Variant virtual operator()() =0;

private:
    const char *fmt;
};

class QuitCommand : public Command {
public:
    QuitCommand(Interpreter& owner, int retcode);
    virtual ~QuitCommand();

    virtual bool blocking() const
    { return true; }

    Variant virtual operator()();

private:
    int f_retcode;
};

class WitnessListCommand : public Command {
public:
    WitnessListCommand(Interpreter& owner);
    virtual ~WitnessListCommand();
    Variant virtual operator()();

    virtual bool blocking() const
    { return true; }
};


class WitnessDumpCommand : public Command {
public:
    WitnessDumpCommand(Interpreter& owner, Expr_ptr wid);
    virtual ~WitnessDumpCommand();
    Variant virtual operator()();

    virtual bool blocking() const
    { return true; }

private:
    Expr_ptr f_wid;
};

#endif
