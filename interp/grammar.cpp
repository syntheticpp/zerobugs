
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 11 "grammar.y"

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



/* Line 189 of yacc.c  */
#line 104 "grammar.cpp"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     CONSTANT = 259,
     STRING_LITERAL = 260,
     SIZEOF = 261,
     PTR_OP = 262,
     INC_OP = 263,
     DEC_OP = 264,
     LEFT_OP = 265,
     RIGHT_OP = 266,
     LE_OP = 267,
     GE_OP = 268,
     EQ_OP = 269,
     NE_OP = 270,
     AND_OP = 271,
     OR_OP = 272,
     MUL_ASSIGN = 273,
     DIV_ASSIGN = 274,
     MOD_ASSIGN = 275,
     ADD_ASSIGN = 276,
     SUB_ASSIGN = 277,
     LEFT_ASSIGN = 278,
     RIGHT_ASSIGN = 279,
     AND_ASSIGN = 280,
     XOR_ASSIGN = 281,
     OR_ASSIGN = 282,
     TYPE_NAME = 283,
     VPTR_OP = 284,
     TYPEDEF = 285,
     EXTERN = 286,
     STATIC = 287,
     AUTO = 288,
     REGISTER = 289,
     CHAR = 290,
     SHORT = 291,
     INT = 292,
     LONG = 293,
     SIGNED = 294,
     UNSIGNED = 295,
     FLOAT = 296,
     DOUBLE = 297,
     CONST = 298,
     VOLATILE = 299,
     VOID = 300,
     KLASS = 301,
     STRUCT = 302,
     UNION = 303,
     ENUM = 304,
     ELIPSIS = 305,
     RANGE = 306,
     CASE = 307,
     DEFAULT = 308,
     IF = 309,
     ELSE = 310,
     SWITCH = 311,
     WHILE = 312,
     DO = 313,
     FOR = 314,
     GOTO = 315,
     CONTINUE = 316,
     BREAK = 317,
     RETURN = 318,
     DOUBLE_COLON = 319,
     TEMPLATE_TYPENAME = 320,
     CONST_CAST = 321,
     DYNAMIC_CAST = 322,
     REINTERPRET_CAST = 323,
     STATIC_CAST = 324
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 215 "grammar.cpp"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  124
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   701

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  92
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  38
/* YYNRULES -- Number of rules.  */
#define YYNRULES  145
/* YYNRULES -- Number of states.  */
#define YYNSTATES  260

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   324

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    84,     2,     2,     2,    86,    80,     2,
      70,    71,    78,    81,    79,    82,    77,    85,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    75,    91,
      72,    90,    73,    89,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    74,     2,    76,    87,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    88,     2,    83,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    13,    21,    29,    37,
      45,    52,    58,    64,    66,    68,    73,    77,    82,    86,
      91,    96,   100,   104,   107,   110,   112,   116,   118,   121,
     124,   127,   130,   133,   136,   139,   142,   145,   150,   152,
     157,   159,   163,   167,   171,   173,   177,   181,   183,   187,
     191,   193,   197,   201,   205,   209,   211,   215,   219,   221,
     225,   227,   231,   233,   237,   239,   243,   245,   249,   251,
     257,   259,   263,   267,   271,   275,   279,   283,   287,   291,
     295,   299,   303,   305,   308,   310,   312,   314,   316,   318,
     320,   322,   324,   326,   328,   330,   332,   335,   338,   341,
     343,   345,   348,   350,   352,   354,   357,   360,   364,   367,
     371,   373,   375,   378,   380,   382,   384,   387,   389,   391,
     395,   397,   399,   402,   404,   406,   408,   411,   414,   418,
     421,   425,   429,   434,   437,   441,   445,   450,   452,   455,
     458,   460,   464,   468,   470,   474
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     111,     0,    -1,   127,    -1,     4,    -1,     5,    -1,    70,
     111,    71,    -1,    66,    72,   124,    73,    70,    98,    71,
      -1,    67,    72,   124,    73,    70,    98,    71,    -1,    68,
      72,   124,    73,    70,    98,    71,    -1,    69,    72,   124,
      73,    70,    98,    71,    -1,    95,    74,   111,    75,   111,
      76,    -1,    95,    74,    75,   111,    76,    -1,    95,    74,
     111,    75,    76,    -1,    93,    -1,    94,    -1,    95,    74,
     111,    76,    -1,    95,    70,    71,    -1,    95,    70,    96,
      71,    -1,    95,    77,   127,    -1,    95,    77,    78,   127,
      -1,    95,     7,    78,   127,    -1,    95,     7,   127,    -1,
      95,    29,   127,    -1,    95,     8,    -1,    95,     9,    -1,
     110,    -1,    96,    79,   110,    -1,    95,    -1,     8,    97,
      -1,     9,    97,    -1,    80,    98,    -1,    78,    98,    -1,
      81,    98,    -1,    82,    98,    -1,    83,    98,    -1,    84,
      98,    -1,     6,    97,    -1,     6,    70,   124,    71,    -1,
      97,    -1,    70,   124,    71,    98,    -1,    98,    -1,    99,
      78,    98,    -1,    99,    85,    98,    -1,    99,    86,    98,
      -1,    99,    -1,   100,    81,    99,    -1,   100,    82,    99,
      -1,   100,    -1,   101,    10,   100,    -1,   101,    11,   100,
      -1,   101,    -1,   102,    72,   101,    -1,   102,    73,   101,
      -1,   102,    12,   101,    -1,   102,    13,   101,    -1,   102,
      -1,   103,    14,   102,    -1,   103,    15,   102,    -1,   103,
      -1,   104,    80,   103,    -1,   104,    -1,   105,    87,   104,
      -1,   105,    -1,   106,    88,   105,    -1,   106,    -1,   107,
      16,   106,    -1,   107,    -1,   108,    17,   107,    -1,   108,
      -1,   108,    89,   108,    75,   109,    -1,   109,    -1,    97,
      90,   110,    -1,    97,    18,   110,    -1,    97,    19,   110,
      -1,    97,    20,   110,    -1,    97,    21,   110,    -1,    97,
      22,   110,    -1,    97,    23,   110,    -1,    97,    24,   110,
      -1,    97,    25,   110,    -1,    97,    26,   110,    -1,    97,
      27,   110,    -1,   110,    -1,   110,    91,    -1,   109,    -1,
      35,    -1,    36,    -1,    37,    -1,    38,    -1,    39,    -1,
      40,    -1,    41,    -1,    42,    -1,    43,    -1,    44,    -1,
      45,    -1,   115,    28,    -1,    48,    28,    -1,    49,    28,
      -1,   129,    -1,   114,    -1,    65,   127,    -1,    46,    -1,
      47,    -1,    78,    -1,    78,   118,    -1,    78,   116,    -1,
      78,   118,   116,    -1,    78,   117,    -1,    78,   118,   117,
      -1,    80,    -1,   119,    -1,   118,   119,    -1,    43,    -1,
      44,    -1,   113,    -1,   120,   113,    -1,   122,    -1,   123,
      -1,   122,    79,   123,    -1,   124,    -1,   120,    -1,   120,
     125,    -1,   116,    -1,   117,    -1,   126,    -1,   116,   126,
      -1,   117,   126,    -1,    70,   125,    71,    -1,    74,    76,
      -1,    74,   112,    76,    -1,   126,    74,    76,    -1,   126,
      74,   112,    76,    -1,    70,    71,    -1,    70,   121,    71,
      -1,   126,    70,    71,    -1,   126,    70,   121,    71,    -1,
     128,    -1,    64,   128,    -1,    86,     3,    -1,     3,    -1,
      28,    64,   128,    -1,     3,    64,   128,    -1,    28,    -1,
      28,    64,   129,    -1,     3,    64,   129,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    64,    64,    65,    66,    67,    68,    70,    72,    74,
      79,    84,    89,   104,   105,   106,   108,   110,   112,   114,
     116,   118,   120,   122,   124,   129,   131,   143,   144,   146,
     149,   151,   153,   155,   157,   159,   162,   164,   169,   170,
     174,   175,   177,   179,   184,   185,   187,   192,   193,   195,
     200,   201,   203,   205,   207,   212,   213,   215,   220,   221,
     226,   227,   232,   233,   238,   239,   244,   245,   250,   251,
     256,   258,   262,   267,   272,   277,   282,   287,   292,   297,
     302,   307,   315,   317,   323,   328,   329,   330,   331,   332,
     333,   334,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   347,   357,   358,   363,   365,   370,   375,   381,   386,
     396,   400,   401,   406,   407,   411,   417,   427,   431,   438,
     448,   452,   453,   461,   462,   463,   464,   465,   469,   475,
     476,   477,   481,   488,   489,   491,   496,   504,   505,   513,
     524,   525,   527,   532,   533,   536
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "CONSTANT",
  "STRING_LITERAL", "SIZEOF", "PTR_OP", "INC_OP", "DEC_OP", "LEFT_OP",
  "RIGHT_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP", "AND_OP", "OR_OP",
  "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN", "ADD_ASSIGN", "SUB_ASSIGN",
  "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN", "OR_ASSIGN",
  "TYPE_NAME", "VPTR_OP", "TYPEDEF", "EXTERN", "STATIC", "AUTO",
  "REGISTER", "CHAR", "SHORT", "INT", "LONG", "SIGNED", "UNSIGNED",
  "FLOAT", "DOUBLE", "CONST", "VOLATILE", "VOID", "KLASS", "STRUCT",
  "UNION", "ENUM", "ELIPSIS", "RANGE", "CASE", "DEFAULT", "IF", "ELSE",
  "SWITCH", "WHILE", "DO", "FOR", "GOTO", "CONTINUE", "BREAK", "RETURN",
  "DOUBLE_COLON", "TEMPLATE_TYPENAME", "CONST_CAST", "DYNAMIC_CAST",
  "REINTERPRET_CAST", "STATIC_CAST", "'('", "')'", "'<'", "'>'", "'['",
  "':'", "']'", "'.'", "'*'", "','", "'&'", "'+'", "'-'", "'~'", "'!'",
  "'/'", "'%'", "'^'", "'|'", "'?'", "'='", "';'", "$accept",
  "primary_expr", "range_expr", "postfix_expr", "argument_expr_list",
  "unary_expr", "cast_expr", "multiplicative_expr", "additive_expr",
  "shift_expr", "relational_expr", "equality_expr", "and_expr",
  "exclusive_or_expr", "inclusive_or_expr", "logical_and_expr",
  "logical_or_expr", "conditional_expr", "assignment_expr", "expr",
  "constant_expr", "type_specifier", "template_type", "class_or_struct",
  "pointer", "reference", "type_qualifier_list", "type_qualifier",
  "type_specifier_list", "parameter_type_list", "parameter_list",
  "parameter_declaration", "type_name", "abstract_declarator",
  "abstract_declarator2", "identifier", "qualified_identifier",
  "qualified_type_name", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
      40,    41,    60,    62,    91,    58,    93,    46,    42,    44,
      38,    43,    45,   126,    33,    47,    37,    94,   124,    63,
      61,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    92,    93,    93,    93,    93,    93,    93,    93,    93,
      94,    94,    94,    95,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    96,    96,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    98,    98,
      99,    99,    99,    99,   100,   100,   100,   101,   101,   101,
     102,   102,   102,   102,   102,   103,   103,   103,   104,   104,
     105,   105,   106,   106,   107,   107,   108,   108,   109,   109,
     110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,   111,   111,   112,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   113,   113,   113,   113,   113,
     113,   114,   115,   115,   116,   116,   116,   116,   116,   116,
     117,   118,   118,   119,   119,   120,   120,   121,   122,   122,
     123,   124,   124,   125,   125,   125,   125,   125,   126,   126,
     126,   126,   126,   126,   126,   126,   126,   127,   127,   127,
     128,   128,   128,   129,   129,   129
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     7,     7,     7,     7,
       6,     5,     5,     1,     1,     4,     3,     4,     3,     4,
       4,     3,     3,     2,     2,     1,     3,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     4,     1,     4,
       1,     3,     3,     3,     1,     3,     3,     1,     3,     3,
       1,     3,     3,     3,     3,     1,     3,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     5,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     2,     1,
       1,     2,     1,     1,     1,     2,     2,     3,     2,     3,
       1,     1,     2,     1,     1,     1,     2,     1,     1,     3,
       1,     1,     2,     1,     1,     1,     2,     2,     3,     2,
       3,     3,     4,     2,     3,     3,     4,     1,     2,     2,
       1,     3,     3,     1,     3,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,   140,     3,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    13,    14,    27,    38,    40,    44,    47,    50,    55,
      58,    60,    62,    64,    66,    68,    70,    82,     0,     2,
     137,     0,     0,    36,     0,    28,    29,     0,   138,     0,
       0,     0,     0,   140,   143,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,   102,   103,     0,     0,
       0,     0,   115,   100,     0,   121,     0,    99,    38,    31,
      30,    32,    33,    34,    35,   139,     0,    23,    24,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    83,     1,   142,     0,   141,     0,   143,
       0,     0,     0,     0,     0,     0,    97,    98,   101,     5,
      96,     0,     0,   104,   110,   116,   123,   124,   122,   125,
       0,     0,    21,    22,    16,     0,    25,     0,     0,     0,
      18,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    71,    41,    42,    43,    45,    46,    48,    49,    53,
      54,    51,    52,    56,    57,    59,    61,    63,    65,    67,
       0,    37,     0,     0,     0,     0,     0,     0,   145,   144,
     133,     0,   117,   118,   120,     0,   129,    84,     0,   113,
     114,   106,   108,   105,   111,   126,   127,     0,     0,    39,
      20,    17,     0,     0,     0,    15,    19,     0,     0,     0,
       0,     0,   134,     0,   128,   130,   107,   109,   112,   135,
       0,   131,     0,    26,    11,    12,     0,    69,     0,     0,
       0,     0,   119,   136,   132,    10,     6,     7,     8,     9
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    21,    22,    23,   155,    78,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    71,
     208,    72,    73,    74,   146,   147,   213,   214,    75,   201,
     202,   203,   204,   148,   149,    39,    40,    77
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -141
static const yytype_int16 yypact[] =
{
     399,   -40,  -141,  -141,   406,   427,   427,    -8,    11,   -11,
      -5,    14,    52,   132,   399,   399,   399,   399,   399,   399,
      47,  -141,  -141,    76,   284,  -141,   -21,   -22,    61,    41,
      62,    50,    55,    51,   127,   -14,  -141,    53,   145,  -141,
    -141,    11,   132,  -141,   399,  -141,  -141,    11,  -141,   636,
     636,   636,   636,    83,    84,  -141,  -141,  -141,  -141,  -141,
    -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,   121,   123,
      46,    88,  -141,  -141,   133,   542,    91,  -141,  -141,  -141,
    -141,  -141,  -141,  -141,  -141,  -141,    40,  -141,  -141,    46,
     179,   200,    45,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,   399,   399,   399,   399,   399,   399,   399,
     399,   399,   399,  -141,  -141,  -141,   115,  -141,   125,   126,
      82,   118,   122,   138,    34,    34,  -141,  -141,  -141,  -141,
    -141,   495,   268,   149,  -141,  -141,   -29,   -29,  -141,   -23,
     399,    46,  -141,  -141,  -141,   -56,  -141,   399,     6,    46,
    -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141,
    -141,  -141,  -141,  -141,  -141,   -21,   -21,   -22,   -22,    61,
      61,    61,    61,    41,    41,    62,    50,    55,    51,   127,
      -9,  -141,    35,    35,   124,   147,   150,   151,  -141,  -141,
    -141,   148,   144,  -141,  -141,   155,  -141,  -141,   158,  -141,
    -141,  -141,  -141,   149,  -141,   -23,   -23,   589,   289,  -141,
    -141,  -141,   399,   159,   315,  -141,  -141,   399,   399,   399,
     399,   399,  -141,   636,  -141,  -141,  -141,  -141,  -141,  -141,
     165,  -141,   161,  -141,  -141,  -141,   162,  -141,   168,   169,
     170,   171,  -141,  -141,  -141,  -141,  -141,  -141,  -141,  -141
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -141,  -141,  -141,  -141,  -141,     0,     2,   -19,    19,     8,
      -4,   134,   135,   136,   167,   131,   157,  -140,   -68,     1,
      26,   181,  -141,  -141,  -134,  -133,  -141,    72,  -141,    37,
    -141,    25,   114,   160,   -31,   -34,    -1,  -123
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      24,    38,   207,   121,    43,    45,    46,    48,   121,   211,
     212,   198,   199,    24,     1,   221,    79,    80,    81,    82,
      83,    84,   156,   222,    41,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   138,    53,   128,     7,
     125,   141,    24,     1,    24,   142,   127,   217,     1,     1,
      85,   218,   152,   111,   112,   153,    47,   104,   160,   107,
     108,    49,    54,   129,   105,   106,   227,    50,     7,   198,
     199,   109,   110,     7,     7,   122,   115,   116,   207,   236,
     237,   224,   225,    86,    87,    88,    51,   247,   175,   176,
      24,    24,   158,    24,    24,    24,    24,    24,    24,    24,
      24,    24,    24,    24,     8,    89,   172,   173,   174,     8,
       8,   183,   184,   113,   114,   215,   216,   220,   151,   179,
     180,   181,   182,   159,    52,   226,    20,    76,   177,   178,
     117,    20,    20,   125,   127,    53,     2,     3,     4,   119,
       5,     6,   118,   120,   123,   124,    90,   134,   135,   136,
      91,   137,   219,    92,   243,   194,   126,    24,   223,   139,
      54,   140,   150,   130,   131,   132,   133,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,     1,     2,     3,     4,   191,     5,     6,   192,
     193,   195,   209,   210,   228,   196,     8,    70,     9,    10,
      11,    12,    13,     1,     2,     3,     4,     7,     5,     6,
      14,   197,    15,    16,    17,    18,    19,   229,    20,   232,
     230,   231,    24,   233,    24,   246,   234,   143,     7,   144,
     248,   249,   250,   251,   235,   244,   253,   254,   255,   256,
     257,   258,   259,     8,   242,     9,    10,    11,    12,    13,
     154,   185,   189,   186,   240,   187,   145,    14,   252,    15,
      16,    17,    18,    19,     8,    20,     9,    10,    11,    12,
      13,     1,     2,     3,     4,   157,     5,     6,    14,   190,
      15,    16,    17,    18,    19,   238,    20,   188,     0,     0,
       0,     0,     1,     2,     3,     4,     7,     5,     6,     0,
       0,   205,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,     0,     0,     0,     0,     0,     7,     1,     2,
       3,     4,     0,     5,     6,     0,     0,     0,     0,     0,
       0,     0,     8,     0,     9,    10,    11,    12,    13,     0,
       0,     0,     0,     7,   206,     0,    14,     0,    15,    16,
      17,    18,    19,     8,    20,     9,    10,    11,    12,    13,
       0,     0,     0,     0,     0,   241,     0,    14,     0,    15,
      16,    17,    18,    19,   103,    20,     0,     0,     0,     8,
       0,     9,    10,    11,    12,    13,     0,     0,     0,     0,
       0,   245,     0,    14,     0,    15,    16,    17,    18,    19,
       0,    20,     1,     2,     3,     4,     0,     5,     6,     1,
       2,     3,     4,     0,     5,     6,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     7,     0,     0,
       1,     2,     3,     4,     7,     5,     6,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     7,     0,     0,     0,     0,
       0,     0,     0,     8,     0,     9,    10,    11,    12,    13,
       8,     0,     9,    10,    11,    12,    42,    14,     0,    15,
      16,    17,    18,    19,    14,    20,    15,    16,    17,    18,
      19,     8,    20,     9,    10,    11,    12,    44,   128,     0,
       0,     0,     0,     0,     0,    14,     0,    15,    16,    17,
      18,    19,     0,    20,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   129,     0,     0,     0,     0,     0,     0,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,   128,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      70,     0,     0,     0,     0,   141,   200,     0,     0,   142,
     129,     0,     0,   143,     0,   144,     0,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,   128,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    70,     0,     0,
       0,     0,   141,     0,     0,     0,   142,   129,     0,     0,
     143,     0,   144,     0,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,   128,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    70,     0,     0,     0,     0,     0,
     239,     0,     0,     0,   129,     0,     0,     0,     0,     0,
       0,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    70
};

static const yytype_int16 yycheck[] =
{
       0,     0,   142,    17,     4,     5,     6,     8,    17,   143,
     143,   134,   135,    13,     3,    71,    14,    15,    16,    17,
      18,    19,    90,    79,    64,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,    70,     3,     3,    28,
      41,    70,    42,     3,    44,    74,    47,    70,     3,     3,
       3,    74,    86,    12,    13,    89,    64,    78,    92,    81,
      82,    72,    28,    28,    85,    86,    75,    72,    28,   192,
     193,    10,    11,    28,    28,    89,    14,    15,   218,   213,
     213,    75,    76,     7,     8,     9,    72,   227,   107,   108,
      90,    91,    91,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,    64,    29,   104,   105,   106,    64,
      64,   115,   116,    72,    73,   146,   147,   151,    78,   111,
     112,   113,   114,    78,    72,   159,    86,    13,   109,   110,
      80,    86,    86,   134,   135,     3,     4,     5,     6,    88,
       8,     9,    87,    16,    91,     0,    70,    64,    64,    28,
      74,    28,   150,    77,   222,    73,    42,   157,   157,    71,
      28,    28,    71,    49,    50,    51,    52,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,     3,     4,     5,     6,    71,     8,     9,    64,
      64,    73,    43,    44,    70,    73,    64,    65,    66,    67,
      68,    69,    70,     3,     4,     5,     6,    28,     8,     9,
      78,    73,    80,    81,    82,    83,    84,    70,    86,    71,
      70,    70,   222,    79,   224,   224,    71,    78,    28,    80,
     228,   229,   230,   231,    76,    76,    71,    76,    76,    71,
      71,    71,    71,    64,   218,    66,    67,    68,    69,    70,
      71,   117,   121,   118,   217,   119,    75,    78,   233,    80,
      81,    82,    83,    84,    64,    86,    66,    67,    68,    69,
      70,     3,     4,     5,     6,    75,     8,     9,    78,   122,
      80,    81,    82,    83,    84,   213,    86,   120,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,    28,     8,     9,    -1,
      -1,   141,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    -1,    -1,    -1,    -1,    -1,    28,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    64,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    -1,    -1,    28,    76,    -1,    78,    -1,    80,    81,
      82,    83,    84,    64,    86,    66,    67,    68,    69,    70,
      -1,    -1,    -1,    -1,    -1,    76,    -1,    78,    -1,    80,
      81,    82,    83,    84,    90,    86,    -1,    -1,    -1,    64,
      -1,    66,    67,    68,    69,    70,    -1,    -1,    -1,    -1,
      -1,    76,    -1,    78,    -1,    80,    81,    82,    83,    84,
      -1,    86,     3,     4,     5,     6,    -1,     8,     9,     3,
       4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,
       3,     4,     5,     6,    28,     8,     9,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    66,    67,    68,    69,    70,
      64,    -1,    66,    67,    68,    69,    70,    78,    -1,    80,
      81,    82,    83,    84,    78,    86,    80,    81,    82,    83,
      84,    64,    86,    66,    67,    68,    69,    70,     3,    -1,
      -1,    -1,    -1,    -1,    -1,    78,    -1,    80,    81,    82,
      83,    84,    -1,    86,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,    -1,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,     3,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      65,    -1,    -1,    -1,    -1,    70,    71,    -1,    -1,    74,
      28,    -1,    -1,    78,    -1,    80,    -1,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,     3,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65,    -1,    -1,
      -1,    -1,    70,    -1,    -1,    -1,    74,    28,    -1,    -1,
      78,    -1,    80,    -1,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,     3,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    65,    -1,    -1,    -1,    -1,    -1,
      71,    -1,    -1,    -1,    28,    -1,    -1,    -1,    -1,    -1,
      -1,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    65
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     8,     9,    28,    64,    66,
      67,    68,    69,    70,    78,    80,    81,    82,    83,    84,
      86,    93,    94,    95,    97,    98,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   127,
     128,    64,    70,    97,    70,    97,    97,    64,   128,    72,
      72,    72,    72,     3,    28,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      65,   111,   113,   114,   115,   120,   124,   129,    97,    98,
      98,    98,    98,    98,    98,     3,     7,     8,     9,    29,
      70,    74,    77,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    90,    78,    85,    86,    81,    82,    10,
      11,    12,    13,    72,    73,    14,    15,    80,    87,    88,
      16,    17,    89,    91,     0,   128,   124,   128,     3,    28,
     124,   124,   124,   124,    64,    64,    28,    28,   127,    71,
      28,    70,    74,    78,    80,   113,   116,   117,   125,   126,
      71,    78,   127,   127,    71,    96,   110,    75,   111,    78,
     127,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   110,    98,    98,    98,    99,    99,   100,   100,   101,
     101,   101,   101,   102,   102,   103,   104,   105,   106,   107,
     108,    71,    64,    64,    73,    73,    73,    73,   129,   129,
      71,   121,   122,   123,   124,   125,    76,   109,   112,    43,
      44,   116,   117,   118,   119,   126,   126,    70,    74,    98,
     127,    71,    79,   111,    75,    76,   127,    75,    70,    70,
      70,    70,    71,    79,    71,    76,   116,   117,   119,    71,
     121,    76,   112,   110,    76,    76,   111,   109,    98,    98,
      98,    98,   123,    71,    76,    76,    71,    71,    71,    71
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */





/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 3:

/* Line 1455 of yacc.c  */
#line 65 "grammar.y"
    { assert(YYPARSE_PARAM); (yyval) = YYINTERP->yylval(); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 67 "grammar.y"
    { (yyval) = (yyvsp[(2) - (3)]); ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 69 "grammar.y"
    { (yyval) = new ConstCast((yyvsp[(3) - (7)]), (yyvsp[(6) - (7)])); ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 71 "grammar.y"
    { (yyval) = new DynamicCast((yyvsp[(3) - (7)]), (yyvsp[(6) - (7)])); ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 73 "grammar.y"
    { (yyval) = new ReinterpretCast((yyvsp[(3) - (7)]), (yyvsp[(6) - (7)])); ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 75 "grammar.y"
    { (yyval) = new StaticCast((yyvsp[(3) - (7)]), (yyvsp[(6) - (7)])); ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 80 "grammar.y"
    { RefPtr<ArgumentList> args(new ArgumentList(YYINTERP, (yyvsp[(3) - (6)])));
          args->push_back((yyvsp[(5) - (6)]));
          (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::ARANGE, (yyvsp[(1) - (6)]), args);
        ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 85 "grammar.y"
    { RefPtr<ArgumentList> args(new ArgumentList(YYINTERP, NULL));
          args->push_back((yyvsp[(4) - (5)]));
          (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::ARANGE, (yyvsp[(1) - (5)]), args);
        ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 90 "grammar.y"
    { RefPtr<ArgumentList> args(new ArgumentList(YYINTERP, (yyvsp[(3) - (5)])));
          args->push_back(NULL);
          (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::ARANGE, (yyvsp[(1) - (5)]), args);
        ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 107 "grammar.y"
    { (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::ARRAY, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 109 "grammar.y"
    { (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::FCALL, (yyvsp[(1) - (3)]), NULL); ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 111 "grammar.y"
    { (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::FCALL, (yyvsp[(1) - (4)]), (yyvsp[(3) - (4)])); ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 113 "grammar.y"
    { (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::MEMBER, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 115 "grammar.y"
    { (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::PTR_TO_MEMBER, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 117 "grammar.y"
    { (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::PTR_TO_MEMBER, (yyvsp[(1) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 119 "grammar.y"
    { (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::POINTER, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 121 "grammar.y"
    { (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::VPOINTER, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 123 "grammar.y"
    { (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::INC, (yyvsp[(1) - (2)]), NULL); ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 125 "grammar.y"
    { (yyval) = new PostfixExpr(YYINTERP, PostfixExpr::DEC, (yyvsp[(1) - (2)]), NULL); ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 130 "grammar.y"
    { (yyval) = new ArgumentList(YYINTERP, (yyvsp[(1) - (1)])); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 132 "grammar.y"
    {
            ArgumentList* list = &interface_cast<ArgumentList&>(*(yyvsp[(1) - (3)]));
            assert(list);

            list->push_back((yyvsp[(3) - (3)]));
            (yyval) = (yyvsp[(1) - (3)]);
        ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 145 "grammar.y"
    { (yyval) = new UnaryExpr(YYINTERP, UnaryExpr::INCREMENT, (yyvsp[(2) - (2)])); ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 147 "grammar.y"
    { (yyval) = new UnaryExpr(YYINTERP, UnaryExpr::DECREMENT, (yyvsp[(2) - (2)])); ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 150 "grammar.y"
    { (yyval) = new UnaryExpr(YYINTERP, UnaryExpr::ADDR, (yyvsp[(2) - (2)])); ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 152 "grammar.y"
    { (yyval) = new UnaryExpr(YYINTERP, UnaryExpr::DEREF, (yyvsp[(2) - (2)])); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 154 "grammar.y"
    { (yyval) = new UnaryExpr(YYINTERP, UnaryExpr::PLUS, (yyvsp[(2) - (2)])); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 156 "grammar.y"
    { (yyval) = new UnaryExpr(YYINTERP, UnaryExpr::MINUS, (yyvsp[(2) - (2)])); ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 158 "grammar.y"
    { (yyval) = new UnaryExpr(YYINTERP, UnaryExpr::BITNEG, (yyvsp[(2) - (2)])); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 160 "grammar.y"
    { (yyval) = new UnaryExpr(YYINTERP, UnaryExpr::NEGATE, (yyvsp[(2) - (2)])); ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 163 "grammar.y"
    { (yyval) = new UnaryExpr(YYINTERP, UnaryExpr::SIZE_OF, (yyvsp[(2) - (2)])); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 165 "grammar.y"
    { (yyval) = new UnaryExpr(YYINTERP, UnaryExpr::SIZE_OF, (yyvsp[(3) - (4)])); ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 170 "grammar.y"
    { (yyval) = new CastExpr((yyvsp[(2) - (4)]), (yyvsp[(4) - (4)])); ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 176 "grammar.y"
    { (yyval) = new MulExpr(YYINTERP, MulExpr::MUL, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 178 "grammar.y"
    { (yyval) = new MulExpr(YYINTERP, MulExpr::DIV, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 180 "grammar.y"
    { (yyval) = new MulExpr(YYINTERP, MulExpr::MOD, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 186 "grammar.y"
    { (yyval) = new AdditiveExpr(YYINTERP, AdditiveExpr::PLUS, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 188 "grammar.y"
    { (yyval) = new AdditiveExpr(YYINTERP, AdditiveExpr::MINUS, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 194 "grammar.y"
    { (yyval) = new BitExpr(YYINTERP, BitExpr::LEFT_SHIFT, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 196 "grammar.y"
    { (yyval) = new BitExpr(YYINTERP, BitExpr::RIGHT_SHIFT, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 202 "grammar.y"
    { (yyval) = new RelationalExpr(YYINTERP, RelationalExpr::LT, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 204 "grammar.y"
    { (yyval) = new RelationalExpr(YYINTERP, RelationalExpr::GT, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 206 "grammar.y"
    { (yyval) = new RelationalExpr(YYINTERP, RelationalExpr::LTE, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 208 "grammar.y"
    { (yyval) = new RelationalExpr(YYINTERP, RelationalExpr::GTE, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 214 "grammar.y"
    { (yyval) = new RelationalExpr(YYINTERP, RelationalExpr::EQ, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 216 "grammar.y"
    { (yyval) = new RelationalExpr(YYINTERP, RelationalExpr::NEQ, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 222 "grammar.y"
    { (yyval) = new BitExpr(YYINTERP, BitExpr::AND, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 228 "grammar.y"
    { (yyval) = new BitExpr(YYINTERP, BitExpr::XOR, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 234 "grammar.y"
    { (yyval) = new BitExpr(YYINTERP, BitExpr::OR, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 240 "grammar.y"
    { (yyval) = new LogicalExpr(YYINTERP, LogicalExpr::AND, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 246 "grammar.y"
    { (yyval) = new LogicalExpr(YYINTERP, LogicalExpr::OR, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 252 "grammar.y"
    { (yyval) = new ConditionalExpr(YYINTERP, (yyvsp[(1) - (5)]), (yyvsp[(3) - (5)]), (yyvsp[(5) - (5)])); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 259 "grammar.y"
    {
            (yyval) = new Assign(YYINTERP, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
        ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 263 "grammar.y"
    {
            (yyval) = new MulExpr(YYINTERP, MulExpr::MUL, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
            (yyval) = new Assign(YYINTERP, (yyvsp[(1) - (3)]), (yyval));
        ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 268 "grammar.y"
    {
            (yyval) = new MulExpr(YYINTERP, MulExpr::DIV, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
            (yyval) = new Assign(YYINTERP, (yyvsp[(1) - (3)]), (yyval));
        ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 273 "grammar.y"
    {
            (yyval) = new MulExpr(YYINTERP, MulExpr::MOD, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
            (yyval) = new Assign(YYINTERP, (yyvsp[(1) - (3)]), (yyval));
        ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 278 "grammar.y"
    {
            (yyval) = new AdditiveExpr(YYINTERP, AdditiveExpr::PLUS, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
            (yyval) = new Assign(YYINTERP, (yyvsp[(1) - (3)]), (yyval));
        ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 283 "grammar.y"
    {
            (yyval) = new AdditiveExpr(YYINTERP, AdditiveExpr::MINUS, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
            (yyval) = new Assign(YYINTERP, (yyvsp[(1) - (3)]), (yyval));
        ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 288 "grammar.y"
    {
            (yyval) = new BitExpr(YYINTERP, BitExpr::LEFT_SHIFT, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
            (yyval) = new Assign(YYINTERP, (yyvsp[(1) - (3)]), (yyval));
        ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 293 "grammar.y"
    {
            (yyval) = new BitExpr(YYINTERP, BitExpr::RIGHT_SHIFT, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
            (yyval) = new Assign(YYINTERP, (yyvsp[(1) - (3)]), (yyval));
        ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 298 "grammar.y"
    {
            (yyval) = new BitExpr(YYINTERP, BitExpr::AND, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
            (yyval) = new Assign(YYINTERP, (yyvsp[(1) - (3)]), (yyval));
        ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 303 "grammar.y"
    {
            (yyval) = new BitExpr(YYINTERP, BitExpr::XOR, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
            (yyval) = new Assign(YYINTERP, (yyvsp[(1) - (3)]), (yyval));
        ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 308 "grammar.y"
    {
            (yyval) = new BitExpr(YYINTERP, BitExpr::OR, (yyvsp[(1) - (3)]), (yyvsp[(3) - (3)]));
            (yyval) = new Assign(YYINTERP, (yyvsp[(1) - (3)]), (yyval));
        ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 316 "grammar.y"
    { assert(YYPARSE_PARAM); YYINTERP->set_yylval((yyvsp[(1) - (1)])); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 318 "grammar.y"
    { assert(YYPARSE_PARAM); YYINTERP->set_yylval((yyvsp[(1) - (2)])); ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 328 "grammar.y"
    { (yyval) = new TypeSpec(YYINTERP, TypeSpec::TF_CHAR); ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 329 "grammar.y"
    { (yyval) = new TypeSpec(YYINTERP, TypeSpec::TF_SHORT); ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 330 "grammar.y"
    { (yyval) = new TypeSpec(YYINTERP, TypeSpec::TF_INT); ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 331 "grammar.y"
    { (yyval) = new TypeSpec(YYINTERP, TypeSpec::TF_LONG); ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 332 "grammar.y"
    { (yyval) = new TypeSpec(YYINTERP, TypeSpec::TF_SIGNED); ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 333 "grammar.y"
    { (yyval) = new TypeSpec(YYINTERP, TypeSpec::TF_UNSIGNED); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 334 "grammar.y"
    { (yyval) = new TypeSpec(YYINTERP, TypeSpec::TF_FLOAT); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 335 "grammar.y"
    { (yyval) = new TypeSpec(YYINTERP, TypeSpec::TF_DOUBLE); ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 336 "grammar.y"
    { (yyval) = new TypeSpec(YYINTERP, TypeSpec::TF_CONST); ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 337 "grammar.y"
    { (yyval) = new TypeSpec(YYINTERP, TypeSpec::TF_VOLATILE); ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 338 "grammar.y"
    { (yyval) = new TypeSpec(YYINTERP, TypeSpec::TF_VOID); ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 339 "grammar.y"
    { (yyval) = verify_class_or_struct((yyvsp[(2) - (2)])); ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 340 "grammar.y"
    { (yyval) = verify_union((yyvsp[(2) - (2)])); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 341 "grammar.y"
    { (yyval) = verify_enum((yyvsp[(2) - (2)])); ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 348 "grammar.y"
    {
            RefPtr<Ident> id = &interface_cast<Ident&>(*(yyvsp[(2) - (2)]));
            (yyval) = new TypeSpec(YYINTERP, 
                              TypeSpec::TF_TYPENAME, 
                              id->name().c_str());
        ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 364 "grammar.y"
    { (yyval) = new PointerDeclarator(YYINTERP); ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 366 "grammar.y"
    {   
            assert((yyvsp[(2) - (2)]).get());
            (yyval) = new PointerDeclarator(YYINTERP, (yyvsp[(2) - (2)])); 
        ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 371 "grammar.y"
    {
            RefPtr<AbstractDeclarator> ptr = new PointerDeclarator(YYINTERP);
            (yyval) = combine_decl(ptr, (yyvsp[(2) - (2)]));
        ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 376 "grammar.y"
    {
            assert((yyvsp[(2) - (3)]).get()); 
            RefPtr<AbstractDeclarator> ptr = new PointerDeclarator(YYINTERP, (yyvsp[(2) - (3)]));
            (yyval) = combine_decl(ptr, (yyvsp[(3) - (3)]));
        ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 382 "grammar.y"
    {
            RefPtr<AbstractDeclarator> ptr = new PointerDeclarator(YYINTERP);
            (yyval) = combine_decl(ptr, (yyvsp[(2) - (2)]));
        ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 387 "grammar.y"
    {
            assert((yyvsp[(2) - (3)]).get());

            RefPtr<AbstractDeclarator> ptr = new PointerDeclarator(YYINTERP, (yyvsp[(2) - (3)]));
            (yyval) = combine_decl(ptr, (yyvsp[(3) - (3)]));
        ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 396 "grammar.y"
    { (yyval) = new ReferenceDeclarator(YYINTERP); ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 402 "grammar.y"
    { interface_cast<QualifierList&>(*(yyvsp[(1) - (2)])).add((yyvsp[(2) - (2)])); ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 406 "grammar.y"
    { (yyval) = new QualifierList(YYINTERP, QUALIFIER_CONST); ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 407 "grammar.y"
    { (yyval) = new QualifierList(YYINTERP, QUALIFIER_VOLATILE); ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 412 "grammar.y"
    {
            RefPtr<TypeSpecList> list = new TypeSpecList(YYINTERP);
            list->push_back((yyvsp[(1) - (1)]));
            (yyval) = list;
        ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 418 "grammar.y"
    { 
            TypeSpecList& list = interface_cast<TypeSpecList&>(*(yyvsp[(1) - (2)]));
            list.push_back((yyvsp[(2) - (2)]));
            (yyval) = (yyvsp[(1) - (2)]);
        ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 432 "grammar.y"
    {
            RefPtr<ParamDeclList> list = new ParamDeclList(YYINTERP);
            list->push((yyvsp[(1) - (1)]));

            (yyval) = list;
        ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 439 "grammar.y"
    {
            RefPtr<ParamDeclList> list = &interface_cast<ParamDeclList&>(*(yyvsp[(1) - (3)]));
            list->push((yyvsp[(3) - (3)]));

            (yyval) = (yyvsp[(1) - (3)]);
        ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 454 "grammar.y"
    { 
            assert((yyvsp[(2) - (2)]).get()); 
            (yyval) = interface_cast<AbstractDeclarator&>(*(yyvsp[(2) - (2)])).apply((yyvsp[(1) - (2)]));
        ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 464 "grammar.y"
    { (yyval) = combine_decl((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 465 "grammar.y"
    { (yyval) = combine_decl((yyvsp[(1) - (2)]), (yyvsp[(2) - (2)])); ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 470 "grammar.y"
    {
            (yyval) = new RightAssocExpr(YYINTERP, (yyvsp[(2) - (3)])); 
        ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 475 "grammar.y"
    { (yyval) = new PointerDeclarator(YYINTERP); ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 476 "grammar.y"
    { (yyval) = new ArrayDeclarator(YYINTERP, (yyvsp[(2) - (3)])); ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 478 "grammar.y"
    {
            (yyval) = new PointerDeclarator(YYINTERP); 
        ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 482 "grammar.y"
    {
            RefPtr<ArrayDeclarator> decl = new ArrayDeclarator(YYINTERP, (yyvsp[(3) - (4)])); 
            (yyval) = combine_decl((yyvsp[(1) - (4)]), decl);
        ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 488 "grammar.y"
    { (yyval) = new FunctionDeclarator(YYINTERP); ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 489 "grammar.y"
    { (yyval) = new FunctionDeclarator(YYINTERP, (yyvsp[(2) - (3)])); ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 492 "grammar.y"
    { 
            RefPtr<FunctionDeclarator> decl = new FunctionDeclarator(YYINTERP); 
            (yyval) = combine_decl((yyvsp[(1) - (3)]), decl);
        ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 497 "grammar.y"
    { 
            RefPtr<FunctionDeclarator> decl = new FunctionDeclarator(YYINTERP, (yyvsp[(3) - (4)])); 
            (yyval) = combine_decl((yyvsp[(1) - (4)]), decl);
        ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 506 "grammar.y"
    { 
            /* $$ = $2 */
            /* hack, force a global lookup */
            RefPtr<Ident> ident = &interface_cast<Ident&>(*(yyvsp[(2) - (2)]));
            YYINTERP->type_or_ident(("::" + ident->name()).c_str());
            (yyval) = YYINTERP->yylval();
        ;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 514 "grammar.y"
    {
            RefPtr<Ident> ident = &interface_cast<Ident&>(*(yyvsp[(2) - (2)]));
            /* prepending the percent forces the interpreter 
               to treat it as a register name */
            YYINTERP->type_or_ident(("%" + ident->name()).c_str());
            (yyval) = YYINTERP->yylval();
        ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 526 "grammar.y"
    { (yyval) = new Ident((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 528 "grammar.y"
    { (yyval) = new Ident((yyvsp[(1) - (3)]), (yyvsp[(3) - (3)])); ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 534 "grammar.y"
    { (yyval) = (yyvsp[(3) - (3)]); /* fixme */ ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 537 "grammar.y"
    { (yyval) = (yyvsp[(3) - (3)]); /* fixme */ ;}
    break;



/* Line 1455 of yacc.c  */
#line 2647 "grammar.cpp"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 539 "grammar.y"


int Interp::yyparse()
{
    return ::yyparse(this);
}

