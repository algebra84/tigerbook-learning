/*
 * parse.c - Parse source file.
 */

#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "types.h"
#include "absyn.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "translate.h"
#include "semant.h"
#include "venv.h"
#include "printtree.h"
#include "parse.h"


extern int yyparse(void);
extern A_exp absyn_root;
extern FILE* yyin, *yyout;
/* parse source file fname; 
   return abstract syntax data structure */
A_exp parse(string fname) 
{EM_reset(fname);
 if (yyparse() == 0) /* parsing worked */
   {
     fprintf(stderr, "parsing success!\n");
     return absyn_root;
   }
 fprintf(stderr, "parsing failed");
 return NULL;
}

int main(int argc, char **argv) {
  yyin = stdin;
  yyout = stdout;
  if (argc!=2) {fprintf(stderr,"usage: a.out filename\n"); exit(1);}
  printFragList(stdout,SEM_transProg(parse(argv[1])));

  return 0;
}
