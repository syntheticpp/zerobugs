#ifndef INTERP_H__69DA4C14_3B6A_4752_8B3E_22EA39E33A0F
#define INTERP_H__69DA4C14_3B6A_4752_8B3E_22EA39E33A0F
//
// $Id: interp.h 714 2010-10-17 10:03:52Z root $
//
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------

#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include "lexer.h"
#include "yystype.h"
#include "zdk/expr.h"
#include "zdk/weak_ptr.h"

class AbstractDeclarator;
class Context;
class CPUStateSaver;
class Expr;

/**
 * The expressions interpreter binds together the
 * lexer, the parser and the eval context. It evaluates
 * a C/C++-like, one-liner expression.
 * Limitations: functions that are inlined or optimized out
 * cannot be invoked. Variables that are optimized out
 * cannot be referenced. Types in the expression must
 * exist in the debugged program.
 *
 * @todo separate the parsing functionality from the
 * evaluation part
 */
CLASS Interp : public RefCountedImpl<>
{
protected:
    Interp(Context&, std::istream&);

public:
    enum Result
    {
        EVAL_ERROR  = -1,
        EVAL_DONE   = 0,
        EVAL_AGAIN  = 1,
    };

    ~Interp() throw();

    /// Instantiate a new interpreter
    static RefPtr<Interp> create(Context&, std::istream&);

    /// evaluate the input -- this is NOT idempotent
    Result run(ExprEvents* = NULL, int numericBase = 0);

    /// Resume expression evaluation after returning from
    /// a function call into the debuggee.
    /// @param addr where function returned
    /// @param state CPU saved state
    /// @param interactive if not NULL, will contain
    /// a flag indicating if it's ok to enter interactive mode
    Result resume(  addr_t addr = 0,
                    CPUStateSaver* state = NULL,
                    bool* interactive = NULL);

    void error(const char* errMsg);

    std::string output() const { return output_.str(); }

    std::ostream& output_stream() { return output_; }

    const char* yytext() { return lexer().YYText(); }

    int yylex();

    YYSTYPE yylval() const;

    void set_yylval(YYSTYPE);

    void set_string_literal(const char*);

    /// determine if the name is an identifier or typename
    /// @return IDENTIFIER, TYPE_NAME, or CONSTANT
    int type_or_ident(const char* name);

    int template_type(const char* name);

    int numeric_base() const { return numericBase_; }

    void set_numeric_base(int base) { numericBase_ = base; }

    Context& context();

    static bool debug_enabled();

    void switch_to(const RefPtr<Interp>&);

    /// Create and switch to an alternative Interp
    RefPtr<Interp> alt_interp(std::istream&);

    bool is_32_bit() const;

protected:
    bool parse();

    Lexer& lexer();

    RefPtr<Variant> eval(RefPtr<Expr>);

private:
    bool is_bool(const char* name);

    int yyparse();

    /// Associate a symbol with the result if there isn't one already;
    /// this accomodates the VariableView in the UI that was designed
    /// to work with DebugSymbols, rather than variants.
    RefPtr<Variant> ensure_symbol(Expr&, Variant&, const std::string&);

private:
    typedef std::auto_ptr<Lexer> LexerPtr;

    // macros are being evaluated on separate
    // interpretor instances
    typedef std::vector<RefPtr<Interp> > MacroStack;

    // non-copyable, non-assignable
    Interp(const Interp&);
    Interp& operator=(const Interp&);

    RefPtr<Context>     context_;
    std::ostringstream  output_;
    LexerPtr            lexer_;
    YYSTYPE             yylval_;    // root of the parse tree
    int                 numericBase_;
    std::string         ident_;     // last ident or typename
    // turn on/off type lookup in type_or_ident()
    bool                lookupTypes_;
    MacroStack          stack_;     // for evaluating macros
};


// For interfacing with the bison-generated parser
int yylex(YYSTYPE*, void*);

void yyerror(const char*);

#endif // INTERP_H__69DA4C14_3B6A_4752_8B3E_22EA39E33A0F
// vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4
