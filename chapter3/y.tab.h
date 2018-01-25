/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    ID = 258,
    STRING = 259,
    INT = 260,
    AND = 261,
    OR = 262,
    EQ = 263,
    NEQ = 264,
    GT = 265,
    GE = 266,
    LE = 267,
    LT = 268,
    PLUS = 269,
    MINUS = 270,
    TIMES = 271,
    DIV = 272,
    UMINUS = 273,
    COMMA = 274,
    COLON = 275,
    SEMICOLON = 276,
    LPAREN = 277,
    RPAREN = 278,
    LBRACK = 279,
    RBRACK = 280,
    LBRACE = 281,
    RBRACE = 282,
    DOT = 283,
    DIVIDE = 284,
    ASSIGN = 285,
    ARRAY = 286,
    IF = 287,
    THEN = 288,
    ELSE = 289,
    WHILE = 290,
    FOR = 291,
    TO = 292,
    DO = 293,
    LET = 294,
    IN = 295,
    END = 296,
    OF = 297,
    BREAK = 298,
    NIL = 299,
    FUNCTION = 300,
    VAR = 301,
    TYPE = 302
  };
#endif
/* Tokens.  */
#define ID 258
#define STRING 259
#define INT 260
#define AND 261
#define OR 262
#define EQ 263
#define NEQ 264
#define GT 265
#define GE 266
#define LE 267
#define LT 268
#define PLUS 269
#define MINUS 270
#define TIMES 271
#define DIV 272
#define UMINUS 273
#define COMMA 274
#define COLON 275
#define SEMICOLON 276
#define LPAREN 277
#define RPAREN 278
#define LBRACK 279
#define RBRACK 280
#define LBRACE 281
#define RBRACE 282
#define DOT 283
#define DIVIDE 284
#define ASSIGN 285
#define ARRAY 286
#define IF 287
#define THEN 288
#define ELSE 289
#define WHILE 290
#define FOR 291
#define TO 292
#define DO 293
#define LET 294
#define IN 295
#define END 296
#define OF 297
#define BREAK 298
#define NIL 299
#define FUNCTION 300
#define VAR 301
#define TYPE 302

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 15 "tiger.grm" /* yacc.c:1909  */

	int pos;
	int ival;
	string sval;
	

#line 155 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
