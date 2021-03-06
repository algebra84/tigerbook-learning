/*
 * parse.c - Parse source file.
 */

#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "errormsg.h"
#include "parse.h"
#include "prabsyn.h"

extern int yyparse(void);
extern A_exp absyn_root;
extern FILE* yyin, *yyout;
/* parse source file fname; 
   return abstract syntax data structure */
A_exp parse(string fname) 
{EM_reset(fname);
 if (yyparse() == 0) /* parsing worked */
   {
     pr_exp(yyout, absyn_root, 2);
     return absyn_root;
   }
 fprintf(stderr, "parsing failed");
 return NULL;
}

int main(int argc, char **argv) {
  yyin = stdin;
  yyout = stdout;
  if (argc!=2) {fprintf(stderr,"usage: a.out filename\n"); exit(1);}
  parse(argv[1]);
 
  return 0;
}
