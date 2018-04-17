#include <lzma.h>
#include <stdlib.h>
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
#include "venv.h"
#include "semant.h"
#include "assem.h"
#include "codegen.h"

static Temp_tempList L(Temp_temp h, Temp_tempList t)
{
  return Temp_TempList(h,t);
}


static AS_instrList iList=NULL, last=NULL;
static Temp_temp munchExp(T_exp e);
Temp_map F_tempMap = NULL;

static void emit(AS_instr inst) {
  if (last != NULL) {
    last->tail = AS_InstrList(inst, NULL);
    last = last->tail;
  } else {
    iList = AS_InstrList(inst, NULL);
    last = iList;
  }
  return;
}

static Temp_tempList munchArgs(int start ,T_expList explist){
  Temp_tempList res = NULL;
  int i = 0;
  for(;explist;explist = explist->tail){
    if(i++ < start)
      continue;
    res = Temp_TempList(munchExp(explist->head),res);
  }
  return res;
}

static Temp_temp munchExp(T_exp e) {
  Temp_temp r = Temp_newtemp();
  switch (e->kind) {
  case T_MEM: {
    if(e->u.MEM->kind == T_BINOP
       && e->u.MEM->u.BINOP.op == T_plus
       && e->u.MEM->u.BINOP.left->kind == T_CONST) {
      char p[30];
      int i = e->u.MEM->u.BINOP.left->u.CONST;
      T_exp e1 = e->u.MEM->u.BINOP.right;
      sprintf(p,"LOAD `d0 <- M[`s0 + %d]\n",i);
      emit(AS_Oper(String(p),
                   L(r, NULL), L(munchExp(e1), NULL), NULL));
      return r;
    }
    if(e->u.MEM->kind == T_BINOP
       && e->u.MEM->u.BINOP.op == T_plus
       && e->u.MEM->u.BINOP.right->kind == T_CONST){
      char p[30];
      int i = e->u.MEM->u.BINOP.right->u.CONST;
      T_exp e1 = e->u.MEM->u.BINOP.left;
      sprintf(p,"LOAD `d0 <- M[`s0 + %d]\n",i);
      emit(AS_Oper(String(p),
                   L(r, NULL), L(munchExp(e1), NULL), NULL));
      return r;
    }
    if(e->u.MEM->kind == T_CONST){
      char p[30];
      int i = e->u.MEM->u.BINOP.right->u.CONST;
      T_exp e1 = e->u.MEM->u.BINOP.left;
      sprintf(p,"LOAD `d0 <- M[r0 + %d]\n",i);
      emit(AS_Oper(String(p),
                   L(r, NULL), NULL, NULL));
      return r;
    }

    T_exp e1 = e->u.MEM;
    emit(AS_Oper("LOAD `d0 <- M[`s0+0]\n",
                 L(r, NULL), L(munchExp(e1), NULL), NULL));
    return r;
  }
  case T_BINOP:{
    switch(e->u.BINOP.op){
    case T_plus:{
      if(e->u.BINOP.left->kind == T_CONST){
        char p[30];
        int i = e->u.BINOP.left->u.CONST;
        T_exp e1 = e->u.BINOP.right;
        sprintf(p,"ADDI `d0 <- `s0 + %d\n",i);
        emit(AS_Oper(String(p),L(r,NULL),L(munchExp(e1),NULL),NULL));
        return r;
      }
      if(e->u.BINOP.right->kind == T_CONST){
        char p[30];
        int i = e->u.BINOP.right->u.CONST;
        T_exp e1 = e->u.BINOP.left;
        sprintf(p, "ADDI `d0 <- `s0 + %d\n",i);
        emit(AS_Oper(String(p),L(r,NULL),L(munchExp(e1),NULL),NULL));
        return r;
      }
      T_exp e1 = e->u.BINOP.left;
      T_exp e2 = e->u.BINOP.right;
      emit(AS_Oper("ADD `d0 <- `s0 + `s1\n",
                   L(r,NULL),L(munchExp(e1),L(munchExp(e2),NULL)),NULL));
      return r;
    }
    case T_minus:{
      if(e->u.BINOP.left->kind == T_CONST){
        char p[30];
        int i = e->u.BINOP.left->u.CONST;
        T_exp e1 = e->u.BINOP.right;
        sprintf(p,"SUBI `d0 <- `s0 - %d\n",i);
        emit(AS_Oper(String(p),L(r,NULL),L(munchExp(e1),NULL),NULL));
        return r;
      }
      if(e->u.BINOP.right->kind == T_CONST){
        char p[30];
        int i = e->u.BINOP.right->u.CONST;
        T_exp e1 = e->u.BINOP.left;
        sprintf(p, "SUBI `d0 <- `s0 - %d\n",i);
        emit(AS_Oper(String(p),L(r,NULL),L(munchExp(e1),NULL),NULL));
        return r;
      }
      T_exp e1 = e->u.BINOP.left;
      T_exp e2 = e->u.BINOP.right;
      emit(AS_Oper("SUB `d0 <- `s0 - `s1\n",
                   L(r,NULL),L(munchExp(e1),L(munchExp(e2),NULL)),NULL));
      return r;

    }
    case T_div:{
      T_exp e1 = e->u.BINOP.left;
      T_exp e2 = e->u.BINOP.right;
      emit(AS_Oper("DIV `d0 <- `s0 / `s1\n",
                   L(r,NULL),L(munchExp(e1),L(munchExp(e2),NULL)),NULL));
      return r;
    }
    case T_mul:{
      T_exp e1 = e->u.BINOP.left;
      T_exp e2 = e->u.BINOP.right;
      emit(AS_Oper("MUL `d0 <- `s0 * `s1\n",
                   L(r,NULL),L(munchExp(e1),L(munchExp(e2),NULL)),NULL));
      return r;
    }
    default:
      assert(0);
    }
  }
  case T_CONST:{
    char p[30];
    int i = e->u.CONST;
    sprintf(p, "MOVI `d0 <- %d\n",i);
    emit(AS_Oper(String(p),L(r,NULL),NULL,NULL));
    return r;
  }
  case T_TEMP:{
    return e->u.TEMP;
  }
    //load string reg;
  case T_NAME:{
    char p[30];
    sprintf(p, "LOAD `d0 <- %s\n",S_name(e->u.NAME));
    emit(AS_Oper(String(p),L(r,NULL),NULL,AS_Targets(Temp_LabelList(e->u.NAME,NULL))));
    return r;
  }

  case T_CALL:{
    Temp_temp funcname = munchExp(e->u.CALL.fun);

    Temp_tempList l = munchArgs(0,e->u.CALL.args);
    // none of callee save regs now. todo?
    emit(AS_Oper("CALL `s0\n", L(F_RV(),NULL), L(funcname,l), NULL));
    return F_RV();
  }
  default:
    break;
  }
  assert(0);
}



static void munchStm(T_stm s) {
  switch (s->kind) {
  case T_MOVE: {
    T_exp dst = s->u.MOVE.dst;
    T_exp src = s->u.MOVE.src;
    // kind of store
    if (dst->kind == T_MEM) {
      if (dst->u.MEM->kind == T_BINOP
          && dst->u.MEM->u.BINOP.op == T_plus
          && dst->u.MEM->u.BINOP.left->kind == T_CONST) {
        T_exp e1 = dst->u.MEM->u.BINOP.right;
        T_exp e2 = src;
        int i = dst->u.MEM->u.BINOP.left->u.CONST;
        char p[30];
        sprintf(p, "STORE M[`s0 + %d] <- `s1\n", i);
        emit(AS_Oper(String(p), NULL, L(munchExp(e1), L(munchExp(e2), NULL)), NULL));
        return;
      }
      if (dst->u.MEM->kind == T_BINOP
          && dst->u.MEM->u.BINOP.op == T_plus
          && dst->u.MEM->u.BINOP.right->kind == T_CONST) {
        T_exp e1 = dst->u.MEM->u.BINOP.left;
        T_exp e2 = src;
        int i = dst->u.MEM->u.BINOP.right->u.CONST;
        char p[30];
        sprintf(p, "STORE M[`s0 + %d] <- `s1\n", i);
        emit(AS_Oper(String(p), NULL, L(munchExp(e1), L(munchExp(e2), NULL)), NULL));
        return;
      }
      if (src->kind == T_MEM) {
        T_exp e1 = dst->u.MEM;
        T_exp e2 = src->u.MEM;
        emit(AS_Oper("MOVEM M[`s0] <- M[`s1]\n", NULL, L(munchExp(e1), L(munchExp(e2), NULL)), NULL));
        return;
      }
      if (dst->u.MEM->kind == T_CONST) {
        T_exp e2 = src;
        int i = dst->u.MEM->u.CONST;
        char p[30];
        sprintf(p, "STORE M[r0 + %d] <- `s0\n", i);
        emit(AS_Oper(String(p), NULL, L(munchExp(e2), NULL), NULL));
        return;
      }
      T_exp e1 = dst->u.MEM;
      T_exp e2 = src;
      emit(AS_Oper("STORE M[`s0] <- `s1\n", NULL, L(munchExp(e1), L(munchExp(e2), NULL)), NULL));
      return;
    }
    // kind of move
    if (dst->kind == T_TEMP) {
      T_exp e1 = dst;
      T_exp e2 = src;
      emit(AS_Move("MOVE `d0 <- `s0\n",
                   L(munchExp(e1), NULL), L(munchExp(e2), NULL)));
      return;
    }
    assert(0);
  }
    // kind of label
  case T_LABEL: {
    char p[30];
    sprintf(p, "%s:\n", Temp_labelstring(s->u.LABEL));
    emit(AS_Label(String(p), s->u.LABEL));
    return;
  }
  case T_CJUMP:{
    T_exp left = s->u.CJUMP.left;
    T_exp right = s->u.CJUMP.right;
    T_exp temp = T_Binop(T_minus,left,right);
    T_exp temp1 = T_Binop(T_minus, right,left);
    switch(s->u.CJUMP.op){
    case T_eq:
      emit(AS_Oper("BRANCHEQ `s0, `j0\n",NULL,L(munchExp(temp),NULL),
                   AS_Targets(Temp_LabelList(s->u.CJUMP.true,NULL))));
      break;
    case T_ne:
      emit(AS_Oper("BRANCHNE `s0, `j0\n",NULL,L(munchExp(temp),NULL),
                   AS_Targets(Temp_LabelList(s->u.CJUMP.true,NULL))));
      break;
    case T_ge:
      emit(AS_Oper("BRANCHGE `s0, `j0\n",NULL,L(munchExp(temp),NULL),
                   AS_Targets(Temp_LabelList(s->u.CJUMP.true,NULL))));
      break;
    case T_lt:
      emit(AS_Oper("BRANCHLT `s0, `j0\n",NULL,L(munchExp(temp),NULL),
                   AS_Targets(Temp_LabelList(s->u.CJUMP.true,NULL))));
      break;
    case T_gt:
      emit(AS_Oper("BRANCHLT `s0, `j0\n",NULL,L(munchExp(temp1),NULL),
                   AS_Targets(Temp_LabelList(s->u.CJUMP.true,NULL))));
      break;
    case T_le:
      emit(AS_Oper("BRANCHGT `s0, `j0\n",NULL,L(munchExp(temp1),NULL),
                   AS_Targets(Temp_LabelList(s->u.CJUMP.true,NULL))));
      break;
    default:
      assert(0);
    }
    return;
  }
  case T_JUMP:{
    emit(AS_Oper("JUMP `j0\n",NULL,NULL,AS_Targets(s->u.JUMP.jumps)));
    return;
  }
  case T_EXP:{
    munchExp(s->u.EXP);
    return;
  }
    // should not be here
  default:
    assert(0);
  }
  return;
}

static void RegInit(){
  if(!F_tempMap) {
    F_tempMap = Temp_empty();
    Temp_enter(F_tempMap,F_FP(),"fp");
    Temp_enter(F_tempMap,F_RV(),"rv");
    Temp_enter(F_tempMap,F_IP(),"ra");
    Temp_enter(F_tempMap,F_ZERO(),"r0");
  }
  //do functional init;
}

AS_instrList F_codegen(F_frame f, T_stmList stmList){
  AS_instrList list;
  T_stmList sl;
  //handle special register
  RegInit();
  /* miscellaneous initializations as necessary */
  {
    for (sl=stmList; sl; sl=sl->tail)
      munchStm(sl->head);
    list=iList;
    iList=last=NULL;
    return F_procEntryExit2(list);
  }
}
