/* vim: tabstop=4:softtabstop=4:expandtab:shiftwidth=4 
 * $Id$
 */
// -------------------------------------------------------------------------
// This file is part of ZeroBugs, Copyright (c) 2010 Cristian L. Vlasceanu
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// -------------------------------------------------------------------------
%{
#include "abstract_decl.h"
#include "additive_expr.h"
#include "assign.h"
#include "bit_expr.h"
#include "cast_expr.h"
#include "conditional_expr.h"
#include "errors.h"
#include "ident.h"
#include "interp.h"
#include "logical_expr.h"
#include "multiplicative_expr.h"
#include "parameter_list.h"
#include "postfix_expr.h"
#include "qualifier_list.h"
#include "relational_expr.h"
#include "type_spec.h"
#include "unary_expr.h"


#define YYPARSE_PARAM   param
#define YYLEX_PARAM     param 
#define YYINTERP        reinterpret_cast<Interp*>(YYPARSE_PARAM)


#define yyerror(msg) assert(YYPARSE_PARAM); \
    reinterpret_cast<Interp*>(YYPARSE_PARAM)->error(msg);

%}

%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME VPTR_OP

%token TYPEDEF EXTERN STATIC AUTO REGISTER
%token CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token KLASS STRUCT UNION ENUM ELIPSIS RANGE

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token DOUBLE_COLON TEMPLATE_TYPENAME 
%token CONST_CAST DYNAMIC_CAST REINTERPRET_CAST STATIC_CAST

%pure-parser


/* %start file */
%start expr
%%

primary_expr
    : identifier
    | CONSTANT { assert(YYPARSE_PARAM); $$ = YYINTERP->yylval(); }
    | STRING_LITERAL
    | '(' expr ')' { $$ = $2; } 
    | CONST_CAST '<' type_name '>' '(' cast_expr ')'
        { $$ = new ConstCast($3, $6); }
    | DYNAMIC_CAST '<' type_name '>' '(' cast_expr ')'
        { $$ = new DynamicCast($3, $6); }
    | REINTERPRET_CAST '<' type_name '>' '(' cast_expr ')'
        { $$ = new ReinterpretCast($3, $6); }
    | STATIC_CAST '<' type_name '>' '(' cast_expr ')'
        { $$ = new StaticCast($3, $6); }
    ;

range_expr
    : postfix_expr '[' expr ':' expr ']'
        { RefPtr<ArgumentList> args(new ArgumentList(YYINTERP, $3));
          args->push_back($5);
          $$ = new PostfixExpr(YYINTERP, PostfixExpr::ARANGE, $1, args);
        }
    | postfix_expr '[' ':' expr ']'
        { RefPtr<ArgumentList> args(new ArgumentList(YYINTERP, NULL));
          args->push_back($4);
          $$ = new PostfixExpr(YYINTERP, PostfixExpr::ARANGE, $1, args);
        }
    | postfix_expr '[' expr ':' ']'
        { RefPtr<ArgumentList> args(new ArgumentList(YYINTERP, $3));
          args->push_back(NULL);
          $$ = new PostfixExpr(YYINTERP, PostfixExpr::ARANGE, $1, args);
        }
    /* D Language array slicing */
    /*
    | postfix_expr '[' expr ".." expr ']'
        { RefPtr<ArgumentList> args(new ArgumentList(YYINTERP, $3));
          args->push_back($5);
          $$ = new PostfixExpr(YYINTERP, PostfixExpr::ARANGE, $1, args);
        } */
    ;

postfix_expr
    : primary_expr
    | range_expr
    | postfix_expr '[' expr ']'
        { $$ = new PostfixExpr(YYINTERP, PostfixExpr::ARRAY, $1, $3); }
    | postfix_expr '(' ')'
        { $$ = new PostfixExpr(YYINTERP, PostfixExpr::FCALL, $1, NULL); }
    | postfix_expr '(' argument_expr_list ')'
        { $$ = new PostfixExpr(YYINTERP, PostfixExpr::FCALL, $1, $3); }
    | postfix_expr '.' identifier
        { $$ = new PostfixExpr(YYINTERP, PostfixExpr::MEMBER, $1, $3); }
    | postfix_expr '.' '*' identifier
        { $$ = new PostfixExpr(YYINTERP, PostfixExpr::PTR_TO_MEMBER, $1, $4); }
    | postfix_expr PTR_OP '*' identifier
        { $$ = new PostfixExpr(YYINTERP, PostfixExpr::PTR_TO_MEMBER, $1, $4); }
    | postfix_expr PTR_OP identifier
        { $$ = new PostfixExpr(YYINTERP, PostfixExpr::POINTER, $1, $3); }
    | postfix_expr VPTR_OP identifier
        { $$ = new PostfixExpr(YYINTERP, PostfixExpr::VPOINTER, $1, $3); }
    | postfix_expr INC_OP
        { $$ = new PostfixExpr(YYINTERP, PostfixExpr::INC, $1, NULL); }
    | postfix_expr DEC_OP
        { $$ = new PostfixExpr(YYINTERP, PostfixExpr::DEC, $1, NULL); }
    ;

argument_expr_list
    : assignment_expr   
        { $$ = new ArgumentList(YYINTERP, $1); }
    | argument_expr_list ',' assignment_expr
        {
            ArgumentList* list = &interface_cast<ArgumentList&>(*$1);
            assert(list);

            list->push_back($3);
            $$ = $1;
        }
    ;


unary_expr
    : postfix_expr
    | INC_OP unary_expr
        { $$ = new UnaryExpr(YYINTERP, UnaryExpr::INCREMENT, $2); }
    | DEC_OP unary_expr
        { $$ = new UnaryExpr(YYINTERP, UnaryExpr::DECREMENT, $2); }

    | '&' cast_expr
        { $$ = new UnaryExpr(YYINTERP, UnaryExpr::ADDR, $2); }
    | '*' cast_expr
        { $$ = new UnaryExpr(YYINTERP, UnaryExpr::DEREF, $2); }
    | '+' cast_expr
        { $$ = new UnaryExpr(YYINTERP, UnaryExpr::PLUS, $2); }
    | '-' cast_expr
        { $$ = new UnaryExpr(YYINTERP, UnaryExpr::MINUS, $2); }
    | '~' cast_expr
        { $$ = new UnaryExpr(YYINTERP, UnaryExpr::BITNEG, $2); }
    | '!' cast_expr
        { $$ = new UnaryExpr(YYINTERP, UnaryExpr::NEGATE, $2); }
 /* sizeof */
    | SIZEOF unary_expr
        { $$ = new UnaryExpr(YYINTERP, UnaryExpr::SIZE_OF, $2); }
    | SIZEOF '(' type_name ')'
        { $$ = new UnaryExpr(YYINTERP, UnaryExpr::SIZE_OF, $3); }
    ;

cast_expr
    : unary_expr
    | '(' type_name ')' cast_expr { $$ = new CastExpr($2, $4); }
    ;

multiplicative_expr
    : cast_expr
    | multiplicative_expr '*' cast_expr
        { $$ = new MulExpr(YYINTERP, MulExpr::MUL, $1, $3); }
    | multiplicative_expr '/' cast_expr
        { $$ = new MulExpr(YYINTERP, MulExpr::DIV, $1, $3); }
    | multiplicative_expr '%' cast_expr
        { $$ = new MulExpr(YYINTERP, MulExpr::MOD, $1, $3); }
    ;

additive_expr
    : multiplicative_expr
    | additive_expr '+' multiplicative_expr 
        { $$ = new AdditiveExpr(YYINTERP, AdditiveExpr::PLUS, $1, $3); }
    | additive_expr '-' multiplicative_expr
        { $$ = new AdditiveExpr(YYINTERP, AdditiveExpr::MINUS, $1, $3); }
    ;

shift_expr
    : additive_expr
    | shift_expr LEFT_OP additive_expr
        { $$ = new BitExpr(YYINTERP, BitExpr::LEFT_SHIFT, $1, $3); }
    | shift_expr RIGHT_OP additive_expr
        { $$ = new BitExpr(YYINTERP, BitExpr::RIGHT_SHIFT, $1, $3); }
    ;

relational_expr
    : shift_expr
    | relational_expr '<' shift_expr
        { $$ = new RelationalExpr(YYINTERP, RelationalExpr::LT, $1, $3); }
    | relational_expr '>' shift_expr
        { $$ = new RelationalExpr(YYINTERP, RelationalExpr::GT, $1, $3); }
    | relational_expr LE_OP shift_expr
        { $$ = new RelationalExpr(YYINTERP, RelationalExpr::LTE, $1, $3); }
    | relational_expr GE_OP shift_expr
        { $$ = new RelationalExpr(YYINTERP, RelationalExpr::GTE, $1, $3); }
    ;

equality_expr
    : relational_expr
    | equality_expr EQ_OP relational_expr
        { $$ = new RelationalExpr(YYINTERP, RelationalExpr::EQ, $1, $3); }
    | equality_expr NE_OP relational_expr
        { $$ = new RelationalExpr(YYINTERP, RelationalExpr::NEQ, $1, $3); }
    ;

and_expr
    : equality_expr
    | and_expr '&' equality_expr
        { $$ = new BitExpr(YYINTERP, BitExpr::AND, $1, $3); }
    ;

exclusive_or_expr
    : and_expr
    | exclusive_or_expr '^' and_expr
        { $$ = new BitExpr(YYINTERP, BitExpr::XOR, $1, $3); }
    ;

inclusive_or_expr
    : exclusive_or_expr
    | inclusive_or_expr '|' exclusive_or_expr
        { $$ = new BitExpr(YYINTERP, BitExpr::OR, $1, $3); }
    ;

logical_and_expr
    : inclusive_or_expr
    | logical_and_expr AND_OP inclusive_or_expr
        { $$ = new LogicalExpr(YYINTERP, LogicalExpr::AND, $1, $3); }
    ;

logical_or_expr
    : logical_and_expr
    | logical_or_expr OR_OP logical_and_expr
        { $$ = new LogicalExpr(YYINTERP, LogicalExpr::OR, $1, $3); }
    ;

conditional_expr
    : logical_or_expr
    | logical_or_expr '?' logical_or_expr ':' conditional_expr 
        { $$ = new ConditionalExpr(YYINTERP, $1, $3, $5); }
    ;

assignment_expr
    : conditional_expr
/*  | unary_expr assignment_operator assignment_expr */ 
    | unary_expr '=' assignment_expr
        {
            $$ = new Assign(YYINTERP, $1, $3);
        }
    | unary_expr MUL_ASSIGN assignment_expr
        {
            $$ = new MulExpr(YYINTERP, MulExpr::MUL, $1, $3);
            $$ = new Assign(YYINTERP, $1, $$);
        }
    | unary_expr DIV_ASSIGN assignment_expr
        {
            $$ = new MulExpr(YYINTERP, MulExpr::DIV, $1, $3);
            $$ = new Assign(YYINTERP, $1, $$);
        }
    | unary_expr MOD_ASSIGN assignment_expr
        {
            $$ = new MulExpr(YYINTERP, MulExpr::MOD, $1, $3);
            $$ = new Assign(YYINTERP, $1, $$);
        }
    | unary_expr ADD_ASSIGN assignment_expr
        {
            $$ = new AdditiveExpr(YYINTERP, AdditiveExpr::PLUS, $1, $3);
            $$ = new Assign(YYINTERP, $1, $$);
        }
    | unary_expr SUB_ASSIGN assignment_expr
        {
            $$ = new AdditiveExpr(YYINTERP, AdditiveExpr::MINUS, $1, $3);
            $$ = new Assign(YYINTERP, $1, $$);
        }
    | unary_expr LEFT_ASSIGN assignment_expr
        {
            $$ = new BitExpr(YYINTERP, BitExpr::LEFT_SHIFT, $1, $3);
            $$ = new Assign(YYINTERP, $1, $$);
        }
    | unary_expr RIGHT_ASSIGN assignment_expr
        {
            $$ = new BitExpr(YYINTERP, BitExpr::RIGHT_SHIFT, $1, $3);
            $$ = new Assign(YYINTERP, $1, $$);
        }
    | unary_expr AND_ASSIGN assignment_expr
        {
            $$ = new BitExpr(YYINTERP, BitExpr::AND, $1, $3);
            $$ = new Assign(YYINTERP, $1, $$);
        }
    | unary_expr XOR_ASSIGN assignment_expr
        {
            $$ = new BitExpr(YYINTERP, BitExpr::XOR, $1, $3);
            $$ = new Assign(YYINTERP, $1, $$);
        }
    | unary_expr OR_ASSIGN assignment_expr
        {
            $$ = new BitExpr(YYINTERP, BitExpr::OR, $1, $3);
            $$ = new Assign(YYINTERP, $1, $$);
        }
    ;

expr
    : assignment_expr 
        { assert(YYPARSE_PARAM); YYINTERP->set_yylval($1); }
    | assignment_expr ';' 
        { assert(YYPARSE_PARAM); YYINTERP->set_yylval($1); }
/*  | expr ',' assignment_expr */
    ;

constant_expr
    : conditional_expr
    ;


type_specifier
    : CHAR      { $$ = new TypeSpec(YYINTERP, TypeSpec::TF_CHAR); }
    | SHORT     { $$ = new TypeSpec(YYINTERP, TypeSpec::TF_SHORT); }
    | INT       { $$ = new TypeSpec(YYINTERP, TypeSpec::TF_INT); }
    | LONG      { $$ = new TypeSpec(YYINTERP, TypeSpec::TF_LONG); }
    | SIGNED    { $$ = new TypeSpec(YYINTERP, TypeSpec::TF_SIGNED); }
    | UNSIGNED  { $$ = new TypeSpec(YYINTERP, TypeSpec::TF_UNSIGNED); }
    | FLOAT     { $$ = new TypeSpec(YYINTERP, TypeSpec::TF_FLOAT); }
    | DOUBLE    { $$ = new TypeSpec(YYINTERP, TypeSpec::TF_DOUBLE); }
    | CONST     { $$ = new TypeSpec(YYINTERP, TypeSpec::TF_CONST); }
    | VOLATILE  { $$ = new TypeSpec(YYINTERP, TypeSpec::TF_VOLATILE); }
    | VOID      { $$ = new TypeSpec(YYINTERP, TypeSpec::TF_VOID); }
    | class_or_struct TYPE_NAME { $$ = verify_class_or_struct($2); }
    | UNION TYPE_NAME { $$ = verify_union($2); }
    | ENUM TYPE_NAME { $$ = verify_enum($2); }
    | qualified_type_name
    | template_type 
    ;

template_type
    : TEMPLATE_TYPENAME identifier
        {
            RefPtr<Ident> id = &interface_cast<Ident&>(*$2);
            $$ = new TypeSpec(YYINTERP, 
                              TypeSpec::TF_TYPENAME, 
                              id->name().c_str());
        }
    ;

class_or_struct
    : KLASS
    | STRUCT
    ;


pointer
    : '*' 
        { $$ = new PointerDeclarator(YYINTERP); }
    | '*' type_qualifier_list
        {   
            assert($2.get());
            $$ = new PointerDeclarator(YYINTERP, $2); 
        }
    | '*' pointer
        {
            RefPtr<AbstractDeclarator> ptr = new PointerDeclarator(YYINTERP);
            $$ = combine_decl(ptr, $2);
        }
    | '*' type_qualifier_list pointer
        {
            assert($2.get()); 
            RefPtr<AbstractDeclarator> ptr = new PointerDeclarator(YYINTERP, $2);
            $$ = combine_decl(ptr, $3);
        }
    | '*' reference
        {
            RefPtr<AbstractDeclarator> ptr = new PointerDeclarator(YYINTERP);
            $$ = combine_decl(ptr, $2);
        }
    | '*' type_qualifier_list reference
        {
            assert($2.get());

            RefPtr<AbstractDeclarator> ptr = new PointerDeclarator(YYINTERP, $2);
            $$ = combine_decl(ptr, $3);
        }
    ;

reference
    : '&' { $$ = new ReferenceDeclarator(YYINTERP); }
    ;

type_qualifier_list
    : type_qualifier
    | type_qualifier_list type_qualifier
        { interface_cast<QualifierList&>(*$1).add($2); }
    ;

type_qualifier
    : CONST     { $$ = new QualifierList(YYINTERP, QUALIFIER_CONST); }
    | VOLATILE  { $$ = new QualifierList(YYINTERP, QUALIFIER_VOLATILE); }
    ;

type_specifier_list
    : type_specifier
        {
            RefPtr<TypeSpecList> list = new TypeSpecList(YYINTERP);
            list->push_back($1);
            $$ = list;
        }
    | type_specifier_list type_specifier
        { 
            TypeSpecList& list = interface_cast<TypeSpecList&>(*$1);
            list.push_back($2);
            $$ = $1;
        }
    ;


parameter_type_list
    : parameter_list
    ;

parameter_list
    : parameter_declaration
        {
            RefPtr<ParamDeclList> list = new ParamDeclList(YYINTERP);
            list->push($1);

            $$ = list;
        }
    | parameter_list ',' parameter_declaration
        {
            RefPtr<ParamDeclList> list = &interface_cast<ParamDeclList&>(*$1);
            list->push($3);

            $$ = $1;
        }
    ;

parameter_declaration 
    : type_name
    ;

type_name
    : type_specifier_list
    | type_specifier_list abstract_declarator
        { 
            assert($2.get()); 
            $$ = interface_cast<AbstractDeclarator&>(*$2).apply($1);
        }
    ;

abstract_declarator
    : pointer
    | reference
    | abstract_declarator2
    | pointer abstract_declarator2      { $$ = combine_decl($1, $2); }
    | reference abstract_declarator2    { $$ = combine_decl($1, $2); }
    ;

abstract_declarator2
    : '(' abstract_declarator ')'
        {
            $$ = new RightAssocExpr(YYINTERP, $2); 
        }
/* abstract array declarators */
/*  | '[' ']'                       { $$ = new ArrayDeclarator(YYINTERP); } */
    | '[' ']'                       { $$ = new PointerDeclarator(YYINTERP); }
    | '[' constant_expr ']'         { $$ = new ArrayDeclarator(YYINTERP, $2); }
    | abstract_declarator2 '[' ']'
        {
            $$ = new PointerDeclarator(YYINTERP); 
        }
    | abstract_declarator2 '[' constant_expr ']'
        {
            RefPtr<ArrayDeclarator> decl = new ArrayDeclarator(YYINTERP, $3); 
            $$ = combine_decl($1, decl);
        }

/* abstract function declarators */
    | '(' ')'                       { $$ = new FunctionDeclarator(YYINTERP); }
    | '(' parameter_type_list ')'   { $$ = new FunctionDeclarator(YYINTERP, $2); }
    
    | abstract_declarator2 '(' ')'
        { 
            RefPtr<FunctionDeclarator> decl = new FunctionDeclarator(YYINTERP); 
            $$ = combine_decl($1, decl);
        }
    | abstract_declarator2 '(' parameter_type_list ')'
        { 
            RefPtr<FunctionDeclarator> decl = new FunctionDeclarator(YYINTERP, $3); 
            $$ = combine_decl($1, decl);
        }
    ;

identifier
    : qualified_identifier
    | DOUBLE_COLON qualified_identifier 
        { 
            /* $$ = $2 */
            /* hack, force a global lookup */
            RefPtr<Ident> ident = &interface_cast<Ident&>(*$2);
            YYINTERP->type_or_ident(("::" + ident->name()).c_str());
            $$ = YYINTERP->yylval();
        }
    | '%' IDENTIFIER 
        {
            RefPtr<Ident> ident = &interface_cast<Ident&>(*$2);
            /* prepending the percent forces the interpreter 
               to treat it as a register name */
            YYINTERP->type_or_ident(("%" + ident->name()).c_str());
            $$ = YYINTERP->yylval();
        }
    ;

qualified_identifier 
    : IDENTIFIER 
    | TYPE_NAME DOUBLE_COLON qualified_identifier
        { $$ = new Ident($1, $3); }  
    | IDENTIFIER DOUBLE_COLON qualified_identifier
        { $$ = new Ident($1, $3); }
    ;

qualified_type_name 
    : TYPE_NAME 
    | TYPE_NAME DOUBLE_COLON qualified_type_name
        { $$ = $3; /* fixme */ }

    | IDENTIFIER DOUBLE_COLON qualified_type_name
        { $$ = $3; /* fixme */ }
    ;
%%

int Interp::yyparse()
{
    return ::yyparse(this);
}
