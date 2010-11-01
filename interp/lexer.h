#ifndef LEXER_H__CCD2F69F_6111_4C6A_82CA_AEB2633DAAB0
#define LEXER_H__CCD2F69F_6111_4C6A_82CA_AEB2633DAAB0
//
// $Id: lexer.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#ifdef yyFlexLexer
 #undef yyFlexLexer
#endif
#include <FlexLexer.h>
#include "yystype.h"

class Interp;


CLASS Lexer : public yyFlexLexer
{
public:
    Lexer(Interp&, std::istream&, std::ostream&);

    int yylex();

    std::ostream& out();

    int column() const { return column_; }

protected:
    void skip_comment();

    void count();

    int constant(int base);

    int constant();

    /**
     * Construct a string constant and return STRING_LITERAL
     */
    int string_literal();

    /**
     * Lookup the type tables and debug symbol tables to
     * determine whether yytext refers to a type or variable.
     * @return TYPE_NAME or IDENTIFIER
     */
    int type_or_ident();

    int template_type();

    int fun_ident();

private:
    Interp& interp_;
    int     column_;
};

#endif // LEXER_H__CCD2F69F_6111_4C6A_82CA_AEB2633DAAB0
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
