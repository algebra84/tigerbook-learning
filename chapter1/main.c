/* This file is intentionally empty.  You should fill it in with your
   solution to the programming exercise. */
#include "util.h"
#include "slp.h"
#include "prog1.h"
#include <stdio.h>

int maxargs (A_stm stmt);
int maxexprs(A_expList explist){
  if(explist->kind == A_lastExpList)
    return 1;
  return maxexprs(explist->u.pair.tail)+1;
}
int maxexpr1(A_exp expr){
  if(expr->kind == A_idExp
     || expr->kind == A_numExp)
    return 0;
  if (expr->kind == A_opExp){
    int res = maxexpr1(expr->u.op.left);
    int res1 = maxexpr1(expr->u.op.right);
    return res>res1?res:res1;
  }
  if (expr->kind = A_eseqExp){
    int res = maxargs(expr->u.eseq.stm);
    int res1 = maxexpr1(expr->u.eseq.exp);
    return res>res1?res:res1;
  }
  assert(0);
}
int maxexpr2(A_expList explist){
  if(explist->kind == A_lastExpList)
    return maxexpr1(explist->u.last);
  if(explist->kind == A_pairExpList){
    int res = maxexpr1(explist->u.pair.head);
    int res1 = maxexpr2(explist->u.pair.tail);
    return res>res1?res:res1;
  }
  assert(0);
}

int maxargs (A_stm stmt){
  if(stmt->kind == A_printStm){
    int res = maxexprs(stmt->u.print.exps);
    int res1 = maxexpr2(stmt->u.print.exps);
    return res>res1?res:res1;
  }
  if(stmt->kind == A_compoundStm){
    int res = maxargs(stmt->u.compound.stm1);
    int res1 = maxargs(stmt->u.compound.stm2);
    return res>res1?res:res1;
  }
  if(stmt->kind == A_assignStm){
    return maxexpr1(stmt->u.assign.exp);
  }
  return 0;
}

int main(int argc, char** argv){
  printf("number is %d\n", maxargs(prog()));
  return 1;
}
