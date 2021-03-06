/**
 * @file parse.cc
 * @brief Parsing services implementation.
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

#include <sstream>
#include <common/common.hh>

#include <cmd/cmd.hh>

#include <expr/expr.hh>
#include <expr/printer/printer.hh>

#include <model/model.hh>

#include <opts/opts_mgr.hh>

#include <parser/grammars/smvLexer.h>
#include <parser/grammars/smvParser.h>

#include <utils/misc.hh>
#include <utils/clock.hh>

/* Antlr 3.4 has a slightly different interface. Enable this if necessary */
#define HAVE_ANTLR_34 0

static bool parseErrors;
static void yasmvdisplayRecognitionError (pANTLR3_BASE_RECOGNIZER recognizer,
                                          pANTLR3_UINT8 * tokenNames);
static void reportParserStatus(bool parseErrors, timespec start,
                               timespec stop);

/**
 * Runs the parser SMV rule on an input .smv file.
 *
 * @returns true if parsing was successful, false otherwise.
 */
bool parseFile(const char* fName)
{
    pANTLR3_INPUT_STREAM input;
    pANTLR3_COMMON_TOKEN_STREAM tstream;

    psmvParser psr;
    psmvLexer  lxr;

    DEBUG
        << "Parsing smv file "
        << fName
        << " ..."
        << std::endl;

    struct timespec start_clock;
    clock_gettime(CLOCK_MONOTONIC, &start_clock);

#if HAVE_ANTLR_34
    input = antlr3FileStreamNew((pANTLR3_UINT8) fName, ANTLR3_ENC_UTF8);
#else
    input = antlr3AsciiFileStreamNew((pANTLR3_UINT8) fName);
#endif

    if (!input)
        throw FileInputException(fName);

    lxr = smvLexerNew(input); // smvLexerNew is generated by ANTLR
    assert(lxr);

    tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));
    assert(tstream);

    psr = smvParserNew(tstream);  // smvParserNew is generated by ANTLR3
    assert(psr);

    parseErrors = false;
    psr->pParser->rec->displayRecognitionError = yasmvdisplayRecognitionError;

    psr->smv(psr);

    // cleanup
    psr->free(psr);
    tstream->free(tstream);
    lxr->free(lxr);
    input->close(input);

    struct timespec stop_clock;
    clock_gettime(CLOCK_MONOTONIC, &stop_clock);

    reportParserStatus(parseErrors, start_clock, stop_clock);
    return ! parseErrors;
}

/**
 * Runs the parser COMMAND_LINE rule on an input line string.
 */
// FIXME: proper error handling
CommandVector_ptr parseCommand(const char *command_line)
{
    pANTLR3_INPUT_STREAM input;
    pANTLR3_COMMON_TOKEN_STREAM tstream;

    psmvParser psr;
    psmvLexer  lxr;

    DEBUG
        << "Parsing command ..."
        << std::endl;

    struct timespec start_clock;
    clock_gettime(CLOCK_MONOTONIC, &start_clock);

#if HAVE_ANTLR_34
    input = antlr3StringStreamNew((pANTLR3_UINT8) command_line,
                                  ANTLR3_ENC_UTF8, strlen(command_line), NULL);
#else
    input = antlr3NewAsciiStringInPlaceStream((pANTLR3_UINT8) command_line,
                                              strlen(command_line), NULL);
#endif

    assert(input);

    lxr = smvLexerNew(input); // smvLexerNew is generated by ANTLR
    assert(lxr);

    tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));
    assert(tstream);

    psr = smvParserNew(tstream);  // smvParserNew is generated by ANTLR3
    assert(psr);

    parseErrors = false;
    psr->pParser->rec->displayRecognitionError = yasmvdisplayRecognitionError;

    CommandVector_ptr res
        (psr -> command_line(psr));

    // cleanup
    psr->free(psr);
    tstream->free(tstream);
    lxr->free(lxr);
    input->close(input);

    struct timespec stop_clock;
    clock_gettime(CLOCK_MONOTONIC, &stop_clock);

    reportParserStatus(parseErrors, start_clock, stop_clock);
    return ! parseErrors
        ? res : NULL;
}

/**
 * Runs the parser TOPLEVEL_EXPRESSION rule on a single string.
 */
Expr_ptr parseExpression(const char *string)
{
    pANTLR3_INPUT_STREAM input;
    pANTLR3_COMMON_TOKEN_STREAM tstream;

    psmvParser psr;
    psmvLexer lxr;

#if HAVE_ANTLR_34
    input = antlr3StringStreamNew((pANTLR3_UINT8) string, ANTLR3_ENC_UTF8, strlen(string), NULL);
#else
    input = antlr3NewAsciiStringInPlaceStream((pANTLR3_UINT8) string, strlen(string), NULL);
#endif
    assert(input);

    lxr = smvLexerNew(input); // smvLexerNew is generated by ANTLR
    assert(lxr);

    tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));
    assert(tstream);

    psr = smvParserNew(tstream);  // smvParserNew is generated by ANTLR3
    assert(psr);

    parseErrors = false;
    psr->pParser->rec->displayRecognitionError = yasmvdisplayRecognitionError;

    Expr_ptr res { psr -> toplevel_expression(psr) };

    psr->free(psr);
    tstream->free(tstream);
    lxr->free(lxr);
    input->close(input);

    return ! parseErrors
        ? res
        : NULL;
}

/**
 * Runs the parser TYPE rule on a single string.
 */
Type_ptr parseTypedef(const char *string)
{
    pANTLR3_INPUT_STREAM input;
    pANTLR3_COMMON_TOKEN_STREAM tstream;

    psmvParser psr;
    psmvLexer lxr;

#if HAVE_ANTLR_34
    input = antlr3StringStreamNew((pANTLR3_UINT8) string, ANTLR3_ENC_UTF8, strlen(string), NULL);
#else
    input = antlr3NewAsciiStringInPlaceStream((pANTLR3_UINT8) string, strlen(string), NULL);
#endif
    assert(input);

    lxr = smvLexerNew(input); // smvLexerNew is generated by ANTLR
    assert(lxr);

    tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));
    assert(tstream);

    psr = smvParserNew(tstream);  // smvParserNew is generated by ANTLR3
    assert(psr);

    parseErrors = false;
    psr->pParser->rec->displayRecognitionError = yasmvdisplayRecognitionError;

    Type_ptr res
        (psr -> type(psr));

    if (!res)
        WARN
            << "Could not parse input!"
            << std::endl;

    psr->free(psr);
    tstream->free(tstream);
    lxr->free(lxr);
    input->close(input);

    return ! parseErrors
        ? res : NULL;
}

/* -- static helpers ------------------------------------------------------- */
static void yasmvdisplayRecognitionError (pANTLR3_BASE_RECOGNIZER recognizer,
                                          pANTLR3_UINT8 * tokenNames)
{
    if (! parseErrors) {
        std::cerr
            << "Syntax error";

        if (0 <= recognizer->state->exception->charPositionInLine) {
            std::cerr << " in line "
                      << recognizer->state->exception->line
                      << ", offset "
                      << recognizer->state->exception->charPositionInLine ;
        }

        std::cerr << "."
                  << std::endl;
    }

    parseErrors = true;
}

static void reportParserStatus(bool parseErrors, timespec start, timespec stop)
{
    const std::string elapsed { elapsed_repr(start, stop) };
    if (parseErrors)
        DEBUG
            << "Parser terminated with errors in "
            << elapsed
            << "."
            << std::endl;
    else
        DEBUG
            << "Parser terminated successfully in "
            << elapsed
            << "."
            << std::endl;
}
