/* This file is intentionally empty.  You should fill it in with your
   solution to the programming exercise. */
#include "slp.h"
#include "prog1.h"
#include "util.h"
#include<stdio.h>

int maxexprs(A_expList explist){
  if(explist->kind == struct A_expList_::A_lastExpList)
    return 1;
  return maxexprs(explist->pair.tail)+1;
}

int maxargs (A_stm stmt){
  if(stmt->kind == struct A_stm_::A_printStm){
    return maxexprs(stmt->print.exps);
  }
  return 0;
}

int main(int argc, char** argv){
  printf("number is %d\n", maxargs(prog()));
  return 1;
}
