//
// $Id: lexer.cpp 719 2010-10-22 03:59:11Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <assert.h>
#include <string>
#include "constant.h"
#include "lexer.h"
#include "grammar.h"
#include "ident.h"
#include "interp.h"


using namespace std;


Lexer::Lexer(Interp& interp, istream& in, ostream& out)
    : yyFlexLexer(&in, &out)
    , interp_(interp)
    , column_(0)
{
}


ostream& Lexer::out()
{
    assert(yyFlexLexer::yyout);

    return *yyFlexLexer::yyout;
}


int Lexer::constant()
{
    const char* text = YYText();

    assert(text);

    if (*text == '\'')
    {
        if (strlen(text) > 3)
        {
            cerr << "*** Warning: multi-character character constant\n";
        }

        YYSTYPE yylval = new IntegerConstant(&interp_, text[1]);
        interp_.set_yylval(yylval);
    }
    else
    {
        YYSTYPE yylval = new FloatingPointConstant(&interp_, text);
        interp_.set_yylval(yylval);
    }
    return CONSTANT;
}


int Lexer::constant(int base)
{
    YYSTYPE yylval = new IntegerConstant(&interp_, YYText(), base);
    interp_.set_yylval(yylval);

    return CONSTANT;
}


int Lexer::type_or_ident()
{
    return interp_.type_or_ident(YYText());
}


int Lexer::template_type()
{
    char ch = 0;
    int n = 0; // count angle brackets
    string buf;

    while ((ch = yyinput()) != 0)
    {
        count();
        buf += ch;

        switch (ch)
        {
        case '<':
            ++n;
            break;

        case '>':
            if (--n == 0)
                return interp_.template_type(buf.c_str());
            break;
        }
    }
    return -1;
}


int Lexer::fun_ident()
{
    if (yyleng)
    {
        yytext[yyleng - 1] = 0;
    }

    YYSTYPE yylval = new Ident(&interp_, yytext, true);
    interp_.set_yylval(yylval);
    return IDENTIFIER;
}


int Lexer::string_literal()
{
    interp_.set_string_literal(YYText());
    return STRING_LITERAL;
}
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
