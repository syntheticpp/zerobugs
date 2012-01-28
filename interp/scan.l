/*
 * vim: tabstop=8:softtabstop=8:noexpandtab:shiftwidth=8 
 * $Id$
 */
/* -------------------------------------------------------------------------
 This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu

 Distributed under the Boost Software License, Version 1.0.
 (See accompanying file LICENSE_1_0.txt or copy at
 http://www.boost.org/LICENSE_1_0.txt)
 ---------------------------------------------------------------------------*/
%option c++
%option yyclass="Lexer"

D                       [0-9]
L                       [a-zA-Z_]
H                       [a-fA-F0-9]
E                       [Ee][+-]?{D}+
FS                      (f|F|l|L)
IS                      (u|U|l|L)*

%S TEMPLATE

%{
#ifdef class
/* see comment in wrapscan.cpp */
#undef class
#endif

#include "zdk/config.h"
#include "lexer.h"
#include "grammar.h"

#if !HAVE_YYFLEXLEXER_YYWRAP
 extern "C" int yywrap();
#endif

void Lexer::skip_comment()
{
        for (char c, c1;;)
        {
                while ((c = yyinput()) != '*' && c != 0)
                {
                        // putchar(c);
                }
                
                if ((c1 = yyinput()) != '/' && c != 0)
                {
                        unput(c1);
                        continue;
                }

                if (c != 0)
                {
                        // putchar(c1);
                }
                break;
        }
}

%}

%%

"/*"                    { skip_comment(); }

"auto"                  { count(); return AUTO; }
"break"                 { count(); return BREAK; }
"case"                  { count(); return CASE; }
"char"                  { count(); return CHAR; }
"class"                 { count(); return KLASS; }
"const"                 { count(); return CONST; }
"const_cast"            { count(); return CONST_CAST; }
"continue"              { count(); return CONTINUE; }
"default"               { count(); return DEFAULT; }
"do"                    { count(); return DO; }
"double"                { count(); return DOUBLE; }
"dynamic_cast"          { count(); return DYNAMIC_CAST; }
"else"                  { count(); return ELSE; }
"enum"                  { count(); return ENUM; }
"extern"                { count(); return EXTERN; }
"float"                 { count(); return FLOAT; }
"for"                   { count(); return FOR; }
"goto"                  { count(); return GOTO; }
"if"                    { count(); return IF; }
"int"                   { count(); return INT; }
"long"                  { count(); return LONG; }
"register"              { count(); return REGISTER; }
"reinterpret_cast"      { count(); return REINTERPRET_CAST; }
"return"                { count(); return RETURN; }
"short"                 { count(); return SHORT; }
"signed"                { count(); return SIGNED; }
"sizeof"                { count(); return SIZEOF; }
"static"                { count(); return STATIC; }
"static_cast"           { count(); return STATIC_CAST; }
"struct"                { count(); return STRUCT; }
"switch"                { count(); return SWITCH; }
"typedef"               { count(); return TYPEDEF; }
"typename"              { count(); BEGIN TEMPLATE; return TEMPLATE_TYPENAME; }
"union"                 { count(); return UNION; }
"unsigned"              { count(); return UNSIGNED; }
"void"                  { count(); return VOID; }
"volatile"              { count(); return VOLATILE; }
"while"                 { count(); return WHILE; }

<TEMPLATE>.				{ BEGIN INITIAL; return template_type(); }

{L}({L}|{D})*           { count(); return type_or_ident(); }
{L}({L}|{D})*"("        { count(); unput('('); return fun_ident(); }

0[xX]{H}+{IS}?          { count(); return constant(16); }

0{D}+{IS}?              { count(); return constant(8); }

{D}+{IS}?               { count(); return constant(10); }

'(\\.|[^\\'])+'         { count(); return constant(); }

{D}+{E}{FS}?            { count(); return constant(); }
{D}*"."{D}+({E})?{FS}?  { count(); return constant(); }
{D}+"."{D}*({E})?{FS}?  { count(); return constant(); }

\"(\\.|[^\\"])*\"       { count(); return string_literal(); }

">>="                   { count(); return RIGHT_ASSIGN; }
"<<="                   { count(); return LEFT_ASSIGN; }
"+="                    { count(); return ADD_ASSIGN; }
"-="                    { count(); return SUB_ASSIGN; }
"*="                    { count(); return MUL_ASSIGN; }
"/="                    { count(); return DIV_ASSIGN; }
"%="                    { count(); return MOD_ASSIGN; }
"&="                    { count(); return AND_ASSIGN; }
"^="                    { count(); return XOR_ASSIGN; }
"|="                    { count(); return OR_ASSIGN; }
">>"                    { count(); return RIGHT_OP; }
"<<"                    { count(); return LEFT_OP; }
"++"                    { count(); return INC_OP; }
"--"                    { count(); return DEC_OP; }
"->"                    { count(); return PTR_OP; }
"=>"                    { count(); return VPTR_OP; }
"&&"                    { count(); return AND_OP; }
"||"                    { count(); return OR_OP; }
"<="                    { count(); return LE_OP; }
">="                    { count(); return GE_OP; }
"=="                    { count(); return EQ_OP; }
"!="                    { count(); return NE_OP; }
";"                     { count(); return ';'; }
"{"                     { count(); return '{'; }
"}"                     { count(); return '}'; }
","                     { count(); return ','; }
":"                     { count(); return ':'; }
"="                     { count(); return '='; }
"("                     { count(); return '('; }
")"                     { count(); return ')'; }
"["                     { count(); return '['; }
"]"                     { count(); return ']'; }
"."                     { count(); return '.'; }
"&"                     { count(); return '&'; }
"!"                     { count(); return '!'; }
"~"                     { count(); return '~'; }
"-"                     { count(); return '-'; }
"+"                     { count(); return '+'; }
"*"                     { count(); return '*'; }
"/"                     { count(); return '/'; }
"%"                     { count(); return '%'; }
"<"                     { count(); return '<'; }
">"                     { count(); return '>'; }
"^"                     { count(); return '^'; }
"|"                     { count(); return '|'; }
"?"                     { count(); return '?'; }
"::"                    { count(); return DOUBLE_COLON; }


[ \t\v\n\f]             { count(); }
.                       { /* ignore bad characters */ }

%%

#if HAVE_YYFLEXLEXER_YYWRAP
int yyFlexLexer::yywrap()
#else
int yywrap()
#endif
{
        return 1;
}


void Lexer::count()
{
        for (int i = 0; YYText()[i] != '\0'; i++)
        {
                if (YYText()[i] == '\n')
                {
                        column_ = 0;
                }
                else if (YYText()[i] == '\t')
                {
                        column_ += 8 - (column_ % 8);
                }
                else
                {
                        ++column_;
                }
        }
        ECHO;
}
