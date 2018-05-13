/*
 * @file on.cc
 * @brief Command `on` class implementation.
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

#include <cstdlib>
#include <cstring>

#include <cmd/commands/commands.hh>
#include <cmd/commands/on.hh>

#include <cmd/cmd.hh>

On::On(Interpreter& owner)
    : Command(owner)
    , f_then(NULL)
    , f_else(NULL)
{}

On::~On()
{
    if (f_then)
        delete f_then;

    if (f_else)
        delete f_else;
}

void On::set_then(Command_ptr c)
{
    if (! c)
        throw CommandException("Empty THEN branch");

    f_then = c;
}

void On::set_else(Command_ptr c)
{
    if (! c)
        throw CommandException("Empty ELSE branch");

    f_else = c;
}

Variant On::operator()()
{
    CommandMgr& cm { CommandMgr::INSTANCE() } ;
    Variant& res { Interpreter::INSTANCE().last_result() };

    if (cm.is_success(res)) {
        if (f_then)
            res = f_then->operator()();
    } else if (cm.is_failure(res)) {
        if (f_else)
            res = f_else->operator()();
    } else {
        std::cout
            << "###"
            << res
            << std::endl;

        assert(false);
    }

    return res;
}

OnTopic::OnTopic(Interpreter& owner)
    : CommandTopic(owner)
{}

OnTopic::~OnTopic()
{
    TRACE
        << "Destroyed on topic"
        << std::endl;
}

void OnTopic::usage()
{
    std::cout
        << "on <command> - performs conditional execution\n";
}
