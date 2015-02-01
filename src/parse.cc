/**
 * @file parse.cc
 * @brief Parsing services
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
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **/
#include <sstream>
#include <common.hh>

#include <cmd.hh>

#include <expr.hh>
#include <printer.hh>

#include <model.hh>

#include <opts.hh>

#include <smvLexer.h>
#include <smvParser.h>

#define HAVE_ANTLR_34 0

class FileInputException : public Exception {

    virtual const char* what() const throw()
    {
        std::ostringstream oss;
        oss
            << "Can not read file `"
            << f_filename << "`";

        return (strdup(oss.str().c_str()));
    }

    std::string f_filename;

public:
    FileInputException(const std::string &filename)
        : f_filename(filename)
    {}

    virtual ~FileInputException() throw()
    {}
};

// TODO: proper error handling
void parseFile(const char* fName)
{
    pANTLR3_INPUT_STREAM input;
    pANTLR3_COMMON_TOKEN_STREAM tstream;

    psmvParser psr;
    psmvLexer  lxr;

    DEBUG
        << "Preparing for parsing file " << fName
        << std::endl;

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

    psr->smv(psr);

    psr->free(psr);
    tstream->free(tstream);
    lxr->free(lxr);
    input->close(input);
}

// TODO: proper error handling
CommandVector_ptr parseCommand(const char *command_line)
{
    pANTLR3_INPUT_STREAM input;
    pANTLR3_COMMON_TOKEN_STREAM tstream;

    psmvParser psr;
    psmvLexer  lxr;

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

    CommandVector_ptr res = psr->command_line(psr);

    psr->free(psr);
    tstream->free(tstream);
    lxr->free(lxr);
    input->close(input);

    return res;
}

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

    Expr_ptr res = psr->toplevel_expression(psr);

    psr->free(psr);
    tstream->free(tstream);
    lxr->free(lxr);
    input->close(input);

    return res;
}

Expr_ptr parseTypedef(const char *string)
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

    Expr_ptr res = psr->type(psr) -> repr();

    psr->free(psr);
    tstream->free(tstream);
    lxr->free(lxr);
    input->close(input);

    return res;
}
