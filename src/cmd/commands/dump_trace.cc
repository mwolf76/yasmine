/*
 * @file dump_trace.cc
 * @brief Command `dump-trace` class implementation.
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
#include <cmd/commands/dump_trace.hh>

#include <env/environment.hh>

#include <expr/expr.hh>
#include <expr/expr_mgr.hh>

#include <witness/witness.hh>
#include <witness/witness_mgr.hh>

#include <iostream>

#include <boost/preprocessor/repetition/repeat.hpp>

#include <utils/misc.hh>

/* a boost hack to generate indentation consts :-) */
#define _SPACE(z, n, str)  " "
#define SPACES(n) BOOST_PP_REPEAT(n, _SPACE, NULL)

static std::string build_unsupported_format_error_message(pconst_char format)
{
    std::ostringstream oss;

    oss
        << "CommandError: format `"
        << format
        << "` is not supported.";

    return oss.str();
}

UnsupportedFormat::UnsupportedFormat(pconst_char format)
    : CommandException(build_unsupported_format_error_message(format))
{}

DumpTrace::DumpTrace(Interpreter& owner)
    : Command(owner)
    , f_trace_id(NULL)
    , f_format(strdup(TRACE_FMT_DEFAULT))
    , f_output(NULL)
{}

DumpTrace::~DumpTrace()
{
    free(f_trace_id);
    free((pchar) f_format);
    free(f_output);
}

void DumpTrace::set_format(pconst_char format)
{
    f_format = strdup(format);
    if (strcmp(f_format, TRACE_FMT_PLAIN) &&
        strcmp(f_format, TRACE_FMT_JSON) &&
        strcmp(f_format, TRACE_FMT_XML) &&
        strcmp(f_format, TRACE_FMT_YAML))
    throw UnsupportedFormat(f_format);
}

void DumpTrace::set_trace_id(pconst_char trace_id)
{
    free(f_trace_id);
    f_trace_id = strdup(trace_id);
}

void DumpTrace::set_output(pconst_char output)
{
    free(f_output);
    f_output = strdup(output);
}

void DumpTrace::dump_plain_section(std::ostream& os,
                                   const char* section,
                                   ExprVector& ev)
{
    const char* TAB
        (SPACES(3));

    if (ev.empty())
        return;

    os
        << "-- "
        << section
        << std::endl;

    for (ExprVector::const_iterator ei = ev.begin();
         ei != ev.end(); ) {

        Expr_ptr eq
            (*ei);

        os
            << TAB
            << eq->lhs()
            << " = "
            << eq->rhs()
            << std::endl;

        ++ ei ;
    }

    os
        << std::endl;
}


void DumpTrace::dump_plain(std::ostream& os, Witness& w)
{
    os
        << "Witness: "
        << w.id()
        << " [[ " << w.desc() << " ]]"
        << std::endl;

    ExprVector input_assignments;
    process_input(w, input_assignments);

    if (0 < input_assignments.size()) {
        os
            << ":: ENV"
            << std::endl;
        dump_plain_section(os, "input", input_assignments);
    }

    for (step_t time = w.first_time(); time <= w.last_time(); ++ time) {

        os
            << ":: @"
            << time
            << std::endl;

        ExprVector state_vars_assignments;
        ExprVector defines_assignments;

        process_time_frame(w, time,
                           state_vars_assignments,
                           defines_assignments);

        dump_plain_section(os, "state", state_vars_assignments);
        dump_plain_section(os, "defines", defines_assignments);
    }
}

void DumpTrace::dump_json_section(std::ostream& os,
                                  const char* section,
                                  ExprVector& ev)
{
    const char* SECOND_LVL
        (SPACES(18));

    const char *THIRD_LVL
        (SPACES(22));

    os
        << SECOND_LVL
        << "\""
        << section
        << "\": {"
        << std::endl;

    for (ExprVector::const_iterator ei = ev.begin();
         ei != ev.end(); ) {

        Expr_ptr eq
            (*ei);

        os
            << THIRD_LVL
            << "\"" << eq->lhs()
            << "\": \"" << eq->rhs() << "\"" ;

        ++ ei ;

        if (ei != ev.end()) {
            os
                << ", "
                << std::endl;
        }
        else
            os
                << std::endl;
    }

    os
        << SECOND_LVL
        << "}" ;
}

void DumpTrace::dump_json(std::ostream& os, Witness& w)
{
    const char* FIRST_LVL
        (SPACES(4));

    const char* SECOND_LVL
        (SPACES(14));

    os
        << "{"
        << std::endl << FIRST_LVL << "\"id\": " << "\"" << w.id() << "\"" << ","
        << std::endl << FIRST_LVL << "\"description\": " << "\"" << w.desc() << "\"" << "," ;

    ExprVector input_vars_assignments;
    process_input(w, input_vars_assignments);

    if (0 < input_vars_assignments.size()) {
        os
            << std::endl << FIRST_LVL << "\"env\": {"
            << std::endl;

        dump_json_section(os, "input", input_vars_assignments);

        os
            << std::endl << FIRST_LVL << "}, " ;
    }

    os
        << std::endl << FIRST_LVL << "\"steps\": [{" << std::endl;

    for (step_t time = w.first_time(); time <= w.last_time(); ++ time) {

        ExprVector state_vars_assignments;
        ExprVector defines_assignments;

        process_time_frame(w, time,
                           state_vars_assignments,
                           defines_assignments);

        dump_json_section(os, "state", state_vars_assignments);
        os
            << ", "
            << std::endl;

        dump_json_section(os, "defines", defines_assignments);
        os
            << std::endl;

        if (time < w.last_time())
            os
                << SECOND_LVL << "},  {"
                << std::endl;
        else
            os
                << SECOND_LVL << "}]"
                << std::endl;
    }

    os
        << "}"
        << std::endl;
}

void DumpTrace::dump_yaml_section(YAML::Emitter& out,
                                  const char* section,
                                  ExprVector& ev)
{
    out
        << YAML::BeginMap
        << YAML::Key << section
        << YAML::Value << YAML::BeginSeq;

    for (ExprVector::const_iterator ei = ev.begin(); ei != ev.end(); ++ ei) {
        Expr_ptr eq { *ei };

        std::stringstream key_stream;
        key_stream << eq->lhs();

        std::stringstream value_stream;
        value_stream << eq->rhs();

        out
            << YAML::BeginMap
            << YAML::Key << key_stream.str()
            << YAML::Value << value_stream.str()
            << YAML::EndMap;
    }

    out
        << YAML::EndSeq
        << YAML::EndMap;
}

void DumpTrace::dump_yaml(std::ostream& os, Witness& w)
{
    YAML::Emitter out;

    out
        << YAML::BeginMap
        << YAML::Key << "witness"

        << YAML::Value
        << YAML::BeginMap
        << YAML::Key << "id"
        << YAML::Value << w.id()
        << YAML::Key << "description"
        << YAML::Value << w.desc()
        << YAML::EndMap;

    ExprVector input_assignments;
    process_input(w, input_assignments);

    if (0 < input_assignments.size()) {
        out
            << YAML::Key << "env" ;

        dump_yaml_section(out, "input", input_assignments);
    }

    out
        << YAML::BeginMap
        << YAML::Key << "steps"
        << YAML::Value << YAML::BeginSeq;

    for (step_t time = w.first_time(); time <= w.last_time(); ++ time) {
        out
            << YAML::BeginMap
            << YAML::Key << "time"
            << YAML::Value << time;

        ExprVector state_vars_assignments;
        ExprVector defines_assignments;

        process_time_frame(w, time,
                           state_vars_assignments,
                           defines_assignments);

        dump_yaml_section(out, "state", state_vars_assignments);
        dump_yaml_section(out, "defines", defines_assignments);

        out
            << YAML::EndMap;
    }

    out
        << YAML::EndSeq
        << YAML::EndMap
        << YAML::EndMap;

    assert(out.good());
    os
        << out.c_str()
        << std::endl;
}

void DumpTrace::process_input(Witness& w,
                              ExprVector& input_assignments)
{
    ExprMgr& em
        (ExprMgr::INSTANCE());

    Model& model
        (ModelMgr::INSTANCE().model());

    SymbIter symbs
        (model);

    while (symbs.has_next()) {

        std::pair< Expr_ptr, Symbol_ptr > pair
            (symbs.next());

        Symbol_ptr symb
            (pair.second);

        if (symb->is_hidden())
            continue;

        Expr_ptr ctx
            (pair.first);
        Expr_ptr name
            (symb->name());
        Expr_ptr full
            (em.make_dot( ctx, name));

        if (symb->is_variable())  {

            Variable& var
                (symb->as_variable());

            /* we're interested onlyl in INPUT vars here ... */
            if (! var.is_input())
                continue;

            Expr_ptr value
                (Environment::INSTANCE().get(name));

            if (!value)
                value = em.make_undef();

            input_assignments.push_back(em.make_eq(full, value));
        }
    } /* while (symbs.has_next()) */

    OrderingPreservingComparisonFunctor fun
        (model);

    sort(input_assignments.begin(),
         input_assignments.end(),
         fun);
}

/* here UNDEF is used to fill up symbols not showing up in the witness where
   they're expected to. (i. e. UNDEF is only a UI entity) */
void DumpTrace::process_time_frame(Witness& w, step_t time,
                                   ExprVector& state_vars_assignments,
                                   ExprVector& defines_assignments)
{
    ExprMgr& em
        (ExprMgr::INSTANCE());

    WitnessMgr& wm
        (WitnessMgr::INSTANCE());

    TimeFrame& tf
        (w[time]);

    Model& model
        (ModelMgr::INSTANCE().model());

    SymbIter symbs
        (model);

    while (symbs.has_next()) {

        std::pair< Expr_ptr, Symbol_ptr > pair
            (symbs.next());

        Symbol_ptr symb
            (pair.second);

        if (symb->is_hidden())
            continue;

        Expr_ptr ctx
            (pair.first);
        Expr_ptr name
            (symb->name());
        Expr_ptr full
            (em.make_dot(ctx, name));

        if (symb->is_variable())  {

            Variable& var
                (symb->as_variable());

            /* INPUT vars do not belong in traces */
            if (var.is_input())
                continue;

            Expr_ptr value
                (tf.has_value(full)
                 ? tf.value(full)
                 : em.make_undef());

            state_vars_assignments.push_back( em.make_eq( full, value));
        }

        else if (symb->is_define()) {

            Define& define
                (symb->as_define());

            Expr_ptr body
                (define.body());

            Expr_ptr value
                (wm.eval( w, ctx, body, time));

            if (! value)
                value = em.make_undef();

            defines_assignments.push_back(em.make_eq(full, value));
        }
    } /* while(symbs.has_next()) */

    OrderingPreservingComparisonFunctor fun
        (model);

    sort(state_vars_assignments.begin(),
         state_vars_assignments.end(), fun);

    sort(defines_assignments.begin(),
         defines_assignments.end(), fun);
}

void DumpTrace::dump_xml_section(std::ostream& os, const char* section, ExprVector& ev)
{
    const char *SECOND_LVL
        (SPACES(8));

    const char *THIRD_LVL
        (SPACES(12));

    if (ev.empty()) {
        os
            << SECOND_LVL
            << "<" << section << "/>"
            << std::endl;

        return;
    }

    os
        << SECOND_LVL
        << "<" << section << ">"
        << std::endl;

    for (ExprVector::const_iterator ei = ev.begin();
         ei != ev.end(); ++ ei) {

        Expr_ptr eq
            (*ei);

        os
            << THIRD_LVL
            << "<item name=\"" << eq -> lhs() << "\" "
            << "value=\"" << eq -> rhs() << "\"/>"
            << std::endl;
    }

    os
        << SECOND_LVL
        << "</" << section << ">"
        << std::endl;
}


void DumpTrace::dump_xml(std::ostream& os, Witness& w)
{
    const char* FIRST_LVL
        (SPACES(4));

    os
        << "<?xml version=\"1.0\"?>"
        << std::endl
        << "<witness"
        << " id=\"" << w.id() << "\""
        << " description=\"" << w.desc() << "\""
        << ">"
        << std::endl;

    ExprVector input_assignments;
    process_input(w, input_assignments);

    if (0 < input_assignments.size()) {
        os
            << FIRST_LVL
            << "<env>"
            << std::endl;

        dump_xml_section(os, "input", input_assignments);

        os
            << FIRST_LVL
            << "</env>"
            << std::endl;
    }

    for (step_t time = w.first_time(); time <= w.last_time(); ++ time) {

        os
            << FIRST_LVL
            << "<step time=\"" << time << "\">"
            << std::endl;

        ExprVector state_vars_assignments;
        ExprVector defines_assignments;

        process_time_frame(w, time,
                           state_vars_assignments,
                           defines_assignments);

        dump_xml_section(os, "state", state_vars_assignments);
        dump_xml_section(os, "defines", defines_assignments);

        os
            << FIRST_LVL
            << "</step>"
            << std::endl;
    }

    os
        << "</witness>" << std::endl;
}

std::ostream& DumpTrace::get_output_stream()
{
    std::ostream* res
        (&std::cout);

    if (f_output) {
        if (f_outfile == NULL) {
            DEBUG
                << "Writing output to file `"
                << f_output
                << "`"
                << std::endl;

            f_outfile = new std::ofstream(f_output, std::ofstream::binary);
        }
        res = f_outfile;
    }

    return *res;
}

Variant DumpTrace::operator()()
{
    WitnessMgr& wm
        (WitnessMgr::INSTANCE());

    std::ostream& os
        (get_output_stream());

    Atom wid = { f_trace_id
                 ? Atom(f_trace_id)
                 : wm.current().id() };
    Witness& w
        (wm.witness(wid));

    if (! strcmp( f_format, TRACE_FMT_PLAIN))
        dump_plain(os, w);

    else if (!strcmp( f_format, TRACE_FMT_JSON))
        dump_json(os, w);

    else if (!strcmp( f_format, TRACE_FMT_XML))
        dump_xml(os, w);

    else if (!strcmp( f_format, TRACE_FMT_YAML))
        dump_yaml(os, w);

    else assert(false); /* unsupported */

    return Variant(okMessage);
}

DumpTraceTopic::DumpTraceTopic(Interpreter& owner)
    : CommandTopic(owner)
{}

DumpTraceTopic::~DumpTraceTopic()
{
    TRACE
        << "Destroyed dump-trace topic"
        << std::endl;
}

void DumpTraceTopic::usage()
{ display_manpage("dump-trace"); }
