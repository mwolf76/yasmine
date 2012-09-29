/*
 * @file interpreter.hh
 * @brief Command interpreter subsystem related classes and definitions.
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
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <common.hh>

class ICommand; // fwd decls
typedef class ICommand* ICommand_ptr;

enum SystemStatus {
    READY,
    PARSING,
    PARSED,
    RUNNING,
    LEAVING,
} ;

typedef class Interpreter* Interpreter_ptr;
class Interpreter {
public:
    // singleton
    static Interpreter& INSTANCE();

    // cmd loop
    void operator()();

    // true iff system is shutting down
    inline bool is_leaving() const
    { return f_status == LEAVING; }

    // process retcode
    inline int retcode() const
    { return f_retcode; }

    void quit(int retcode);

protected:
    friend class ICommand;

    Interpreter();
    virtual ~Interpreter();

    // for streams redirection
    inline istream& in() const
    { return *f_in; }

    inline ostream& out() const
    { return *f_out; }

    inline ostream& err() const
    { return *f_err; }

    int f_retcode;
    SystemStatus f_status;

    istream *f_in;
    ostream *f_out;
    ostream *f_err;

    static Interpreter_ptr f_instance;
};

#endif
