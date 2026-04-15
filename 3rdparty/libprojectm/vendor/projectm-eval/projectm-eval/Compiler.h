/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_PRJM_EVAL_COMPILER_H_INCLUDED
# define YY_PRJM_EVAL_COMPILER_H_INCLUDED
/* Debug traces.  */
#ifndef PRJM_EVAL_DEBUG
# if defined YYDEBUG
#if YYDEBUG
#   define PRJM_EVAL_DEBUG 1
#  else
#   define PRJM_EVAL_DEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define PRJM_EVAL_DEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined PRJM_EVAL_DEBUG */
#if PRJM_EVAL_DEBUG
extern int prjm_eval_debug;
#endif
/* "%code requires" blocks.  */


#include "CompilerFunctions.h"

#include <stdio.h>

typedef void* yyscan_t;


/* Token kinds.  */
#ifndef PRJM_EVAL_TOKENTYPE
# define PRJM_EVAL_TOKENTYPE
  enum prjm_eval_tokentype
  {
    PRJM_EVAL_EMPTY = -2,
    PRJM_EVAL_EOF = 0,             /* "end of file"  */
    PRJM_EVAL_error = 256,         /* error  */
    PRJM_EVAL_UNDEF = 257,         /* "invalid token"  */
    GMEM = 258,                    /* GMEM  */
    ADDOP = 259,                   /* ADDOP  */
    SUBOP = 260,                   /* SUBOP  */
    MODOP = 261,                   /* MODOP  */
    OROP = 262,                    /* OROP  */
    ANDOP = 263,                   /* ANDOP  */
    DIVOP = 264,                   /* DIVOP  */
    MULOP = 265,                   /* MULOP  */
    POWOP = 266,                   /* POWOP  */
    EQUAL = 267,                   /* EQUAL  */
    BELEQ = 268,                   /* BELEQ  */
    ABOEQ = 269,                   /* ABOEQ  */
    NOTEQ = 270,                   /* NOTEQ  */
    BOOLOR = 271,                  /* BOOLOR  */
    BOOLAND = 272,                 /* BOOLAND  */
    NUM = 273,                     /* NUM  */
    VAR = 274,                     /* VAR  */
    FUNC = 275,                    /* FUNC  */
    NEG = 276,                     /* NEG  */
    POS = 277                      /* POS  */
  };
  typedef enum prjm_eval_tokentype prjm_eval_token_kind_t;
#endif

/* Value type.  */
#if ! defined PRJM_EVAL_STYPE && ! defined PRJM_EVAL_STYPE_IS_DECLARED
union PRJM_EVAL_STYPE
{
  PRJM_EVAL_F NUM;                         /* NUM  */
  char* VAR;                               /* VAR  */
  char* FUNC;                              /* FUNC  */
  prjm_eval_compiler_arg_list_t* yykind_46; /* function-arglist  */
  prjm_eval_compiler_node_t* program;      /* program  */
  prjm_eval_compiler_node_t* function;     /* function  */
  prjm_eval_compiler_node_t* parentheses;  /* parentheses  */
  prjm_eval_compiler_node_t* yykind_48;    /* instruction-list  */
  prjm_eval_compiler_node_t* expression;   /* expression  */


};
typedef union PRJM_EVAL_STYPE PRJM_EVAL_STYPE;
# define PRJM_EVAL_STYPE_IS_TRIVIAL 1
# define PRJM_EVAL_STYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined PRJM_EVAL_LTYPE && ! defined PRJM_EVAL_LTYPE_IS_DECLARED
typedef struct PRJM_EVAL_LTYPE PRJM_EVAL_LTYPE;
struct PRJM_EVAL_LTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define PRJM_EVAL_LTYPE_IS_DECLARED 1
# define PRJM_EVAL_LTYPE_IS_TRIVIAL 1
#endif




int prjm_eval_parse (prjm_eval_compiler_context_t* cctx, yyscan_t scanner);

/* "%code provides" blocks.  */


    #define YYSTYPE PRJM_EVAL_STYPE
    #define YYLTYPE PRJM_EVAL_LTYPE

    #define YYERROR_VERBOSE

   #define YY_DECL \
       int yylex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, prjm_eval_compiler_context_t* cctx, yyscan_t yyscanner)
   YY_DECL;

   void yyerror(YYLTYPE* yyllocp, prjm_eval_compiler_context_t* cctx, yyscan_t yyscanner, const char* message);


#endif /* !YY_PRJM_EVAL_COMPILER_H_INCLUDED  */
