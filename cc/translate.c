//
// Created by dadiao on 18-2-7.
//

#include <lzma.h>
#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "translate.h"

struct Tr_level_ {int level; Tr_level parent;F_frame frame;Tr_accessList formals;};

struct Tr_access_ {Tr_level level; F_access access;};

struct patchList_ {Temp_label *head; patchList tail;};
struct Cx {patchList trues; patchList falses; T_stm stm;};

struct Tr_exp_ {
  Tr_expkind kind;
  union{T_exp ex; T_stm nx; struct Cx cx;} u;
};

static Tr_level out = NULL;
static F_fragList proclist = NULL;
static F_fragList strlist = NULL;

static patchList PatchList(Temp_label *head, patchList tail){
  patchList p = checked_malloc(sizeof(*p));
  p->head = head;
  p->tail = tail;
  return p;
}

Tr_accessList Tr_AccessList(Tr_access head,
                            Tr_accessList tail){
  Tr_accessList p = checked_malloc(sizeof(*p));
  p->head = head;
  p->tail = tail;
  return p;
}

Tr_level Tr_outermost(void){
  if(!out) {
    out = checked_malloc(sizeof(*out));
    out->level = 0;
    out->parent = NULL;
    out->frame = NULL;
  }
  return out;
}

static Tr_accessList makeFormalAccessList(Tr_level level){
  F_accessList itlist = F_formals(level->frame)->tail;
  Tr_accessList res = NULL;
  for(;itlist != NULL; itlist=itlist->tail){
    Tr_access p = checked_malloc(sizeof(*p));
    p->access = itlist->head;
    p->level = level;
    res = Tr_AccessList(p, res);
  }
  Tr_accessList tmp = res;
  res = NULL;
  for(;tmp ; tmp=tmp->tail)
    res = Tr_AccessList(tmp->head,res);
  return res;
}

Tr_level Tr_newLevel(Tr_level parent, Temp_label name,
                     U_boolList formals){
  Tr_level p = checked_malloc(sizeof(*p));
  p->level = parent->level+1;
  p->parent = parent;
  p->frame = F_newFrame(name,U_BoolList(TRUE,formals));
  p->formals = makeFormalAccessList(p);
  return p;
}

Tr_accessList Tr_formals(Tr_level level){
  return level->formals;
}

Tr_access Tr_allocLocal(Tr_level level, bool escape){
  Tr_access p = checked_malloc(sizeof(*p));
  p->level = level;
  p->access = F_allocLocal(level->frame,escape);
  return p;
}

void doPatch(patchList tList, Temp_label label){
  for(;tList;tList=tList->tail)
    *(tList->head) = label;
}

patchList joinPatch(patchList first, patchList second){
  if(!first)
    return second;
  for(;first->tail; first=first->tail);
  first->tail=second;
  return first;
}

Tr_expList Tr_ExpList(Tr_exp head, Tr_expList tail)
{
  Tr_expList p = (Tr_expList) checked_malloc (sizeof(*p));
  p->head=head; p->tail=tail;
  return p;
}

static T_exp unEx(Tr_exp e){
  switch(e->kind){
  case Tr_ex:
    return e->u.ex;
  case Tr_cx: {
    Temp_temp r = Temp_newtemp();
    Temp_label t = Temp_newlabel(), f = Temp_newlabel();
    doPatch(e->u.cx.trues, t);
    doPatch(e->u.cx.falses, f);
    return T_Eseq(T_Move(T_Temp(r), T_Const(1)),
                  T_Eseq(e->u.cx.stm,
                         T_Eseq(T_Label(f),
                                T_Eseq(T_Move(T_Temp(r), T_Const(0)),
                                       T_Eseq(T_Label(t), T_Temp(r))))));
  }
  case Tr_nx:
    return T_Eseq(e->u.nx,T_Const(0));
  }
  assert(0);
}

static T_stm unNx(Tr_exp e){
  switch(e->kind){
  case Tr_ex:
    return T_Exp(e->u.ex);
  case Tr_nx:
    return e->u.nx;
  case Tr_cx:
    return e->u.cx.stm;
  }
  assert(0);
}

static struct Cx unCx(Tr_exp e){
  switch(e->kind){
  case Tr_cx:
    return e->u.cx;
  case Tr_ex:{
    struct Cx res;
    res.stm = T_Cjump(T_eq,e->u.ex,T_Const(0),NULL,NULL);
    res.trues = PatchList(&(res.stm->u.CJUMP.true),NULL);
    res.falses = PatchList(&(res.stm->u.CJUMP.false),NULL);
    return res;
  }
  default: assert(0);
  }
}

static Tr_exp Tr_Ex(T_exp ex){
  Tr_exp p = checked_malloc(sizeof(*p));
  p->kind = Tr_ex;
  p->u.ex = ex;
  return p;
}

static Tr_exp Tr_Nx(T_stm nx){
  Tr_exp p = checked_malloc(sizeof(*p));
  p->kind = Tr_nx;
  p->u.nx = nx;
  return p;
}

static Tr_exp Tr_Cx(patchList trues, patchList falses, T_stm stm){
  Tr_exp p = checked_malloc(sizeof(*p));
  p->kind = Tr_cx;
  p->u.cx.falses = falses;
  p->u.cx.stm = stm;
  p->u.cx.trues = trues;
  return p;
}

Tr_exp Tr_simpleVar(Tr_access access, Tr_level level){
  Tr_level acc_level = access->level;
  T_exp fp = T_Temp(F_FP());
  for(; acc_level != level; level = level->parent)
    fp = T_Mem(T_Binop(T_plus,fp,T_Const(0)));
  return Tr_Ex(F_Exp(access->access,fp));
}

Tr_exp Tr_fieldVar(Tr_exp recordbase, int offset){
  return Tr_Ex(T_Mem(T_Binop(T_plus,unEx(recordbase),
                             T_Const(offset*F_wordSize))));
}

Tr_exp Tr_subscriptVar(Tr_exp arraybase, Tr_exp index){
  return Tr_Ex(T_Mem(T_Binop(T_plus,unEx(arraybase),
                             T_Binop(T_mul,unEx(index),T_Const(F_wordSize)))));
}

Tr_exp Tr_arrayExp(Tr_exp size, Tr_exp init) {
  return Tr_Ex(F_externalCall(String("initArray"),
                              T_ExpList(unEx(size),T_ExpList(unEx(init),NULL))));
}

// list: reverse order
Tr_exp Tr_recordExp(int n, Tr_expList list){
  T_exp r = T_Temp(Temp_newtemp());
  T_stm alloc = T_Move(r,
                       F_externalCall(String("initRecord"),T_ExpList(T_Const(n*F_wordSize),NULL)));
  T_stm right = NULL;
  for(int i = n-1; i != -1; i--) {
    right = T_Seq(T_Move(T_Mem(T_Binop(T_plus, r, T_Const(i*F_wordSize))), unEx(list->head)),right);
    list=list->tail;
  }
  return Tr_Ex(T_Eseq(T_Seq(alloc,right),r));
}

// don't alloc memory ok?
static Temp_temp nilTemp = NULL;
Tr_exp Tr_nilExp(){
  if(!nilTemp){
    nilTemp = Temp_newtemp();
    T_stm alloc = T_Move(T_Temp(nilTemp),
                         F_externalCall(String("initRecord"),T_ExpList(T_Const(0*F_wordSize),NULL)));
    return Tr_Ex(T_Eseq(alloc,T_Temp(nilTemp)));
  }
  return Tr_Ex(T_Temp(nilTemp));
}

Tr_exp Tr_intExp(int consti){
  return Tr_Ex(T_Const(consti));
}

Tr_exp Tr_stringExp(string s){
  Temp_label label = Temp_namedlabel(s);
  F_frag strflag = F_StringFrag(label,s);
  F_fragList temp_list = checked_malloc(sizeof(*temp_list));
  temp_list->head = strflag;
  temp_list->tail = strlist;
  strlist = temp_list;
  return Tr_Ex(T_Name(label));
}

// Reverse order
static T_expList un_Trlist(Tr_expList trlist){
  T_expList res = NULL;
  while(trlist){
    T_expList p = checked_malloc(sizeof(*res));
    p->head = unEx(trlist->head);
    p->tail = res;
    res = p;
    trlist = trlist->tail;
  }
  return res;
}

static Tr_exp Get_staticLink(Tr_level funclevel, Tr_level level){
  T_exp addr = T_Temp(F_FP());
  T_exp res = NULL;
  // condisder recursive function
  while(funclevel->parent != level){
    F_access staticlink = F_formals(level->frame)->head;
    addr = F_Exp(staticlink,addr);
    level = level->parent;
  }
  return Tr_Ex(addr);
}

Tr_exp Tr_callExp(string funcname, Tr_level funclevel, Tr_level level, Tr_expList argslist){
  Tr_exp staticlink = Get_staticLink(funclevel, level);
  Tr_expList p = checked_malloc(sizeof(*p));
  p->head = staticlink;
  p->tail = argslist;
  T_expList argslist1 = un_Trlist(p);
  return Tr_Ex(T_Call(T_Name(Temp_namedlabel(funcname)),argslist1));
}

Tr_exp Tr_arithExp(Tr_exp left, Tr_exp right, A_oper op){
  T_binOp binop;
  switch(op){
  case A_plusOp: binop = T_plus; break;
  case A_minusOp: binop = T_minus; break;
  case A_timesOp: binop = T_mul; break;
  case A_divideOp: binop = T_div; break;
  default: assert(0);
  }

  return Tr_Ex(T_Binop(binop,unEx(left),unEx(right)));
}

Tr_exp Tr_relOpExp(Tr_exp left, Tr_exp right, A_oper op){
  T_relOp binop;
  switch(op){
  case A_gtOp: binop = T_gt; break;
  case A_geOp: binop = T_ge; break;
  case A_leOp: binop = T_le; break;
  case A_ltOp: binop = T_lt; break;
  case A_eqOp: binop = T_eq; break;
  case A_neqOp: binop = T_ne; break;
  default: assert(0);
  }

  T_stm stm = T_Cjump(binop,unEx(left),unEx(right),NULL,NULL);
  patchList trues = PatchList(&(stm->u.CJUMP.true),NULL);
  patchList falses = PatchList(&(stm->u.CJUMP.false),NULL);
  return Tr_Cx(trues,falses,stm);

}

Tr_exp Tr_strEqExp(Tr_exp left, Tr_exp right,A_oper op){
  T_exp res = F_externalCall(String("stringEqual"),
                             T_ExpList(unEx(left),
                                       T_ExpList(unEx(right),NULL)));
  if(op == A_eqOp)
    return Tr_Ex(res);
  T_exp e = T_Binop(T_minus, T_Const(1),res);
  return Tr_Ex(e);
}

Tr_exp Tr_assignExp(Tr_exp left, Tr_exp right){
  return Tr_Nx(T_Move(unEx(left),unEx(right)));
}

// to do: handle cx/nx type of then/else specially
Tr_exp Tr_ifExp(Tr_exp condition, Tr_exp thenn, Tr_exp elsee){
  patchList trues = unCx(condition).trues;
  patchList falses = unCx(condition).falses;

  Temp_label label_then = Temp_newlabel();
  Temp_label label_else = Temp_newlabel();
  // if-then
  if(!elsee){
    T_stm seq = T_Seq(T_Label(label_then),T_Seq(unNx(thenn),T_Label(label_then)));
    doPatch(trues,label_then);
    doPatch(trues,label_else);
    return Tr_Nx(T_Seq(unCx(condition).stm,seq));
  }

  // if-then-else
  Temp_label label_join = Temp_newlabel();
  T_exp r = T_Temp(Temp_newtemp());
  T_stm joinjump = T_Jump(T_Name(label_join),Temp_LabelList(label_join,NULL));

  doPatch(trues,label_then);
  doPatch(falses,label_else);
  T_stm stm_then = T_Seq(T_Label(label_then),T_Seq(T_Move(r,unEx(thenn)),joinjump));
  T_stm stm_else = T_Seq(T_Label(label_else),T_Seq(T_Move(r,unEx(elsee)),joinjump));

  return Tr_Ex(T_Eseq(T_Seq(unCx(condition).stm,T_Seq(stm_then,stm_else)),
                      T_Eseq(T_Label(label_join),r)));

}
// handle cx/nx type
Tr_exp Tr_ifExp1(Tr_exp condition, Tr_exp thenn, Tr_exp elsee){
  patchList trues = unCx(condition).trues;
  patchList falses = unCx(condition).falses;

  Temp_label label_then = Temp_newlabel();
  Temp_label label_else = Temp_newlabel();
  // if-then
  if(!elsee){
    T_stm seq = T_Seq(T_Label(label_then),T_Seq(unNx(thenn),T_Label(label_then)));
    doPatch(trues,label_then);
    doPatch(trues,label_else);
    return Tr_Nx(T_Seq(unCx(condition).stm,seq));
  }

  // if-then-else
  Temp_label label_join = Temp_newlabel();
  T_exp r = T_Temp(Temp_newtemp());
  T_stm joinjump = T_Jump(T_Name(label_join),Temp_LabelList(label_join,NULL));

  doPatch(trues,label_then);
  doPatch(falses,label_else);

  // must be the same return type
  if(thenn->kind == Tr_nx && elsee->kind == Tr_nx){
    T_stm stm_then = T_Seq(T_Label(label_then),unNx(thenn));
    T_stm stm_else = T_Seq(T_Label(label_else),unNx(elsee));
    return Tr_Nx(T_Seq(unCx(condition).stm,T_Seq(stm_then,stm_else)));
  }

  T_stm stm_else;
  T_stm stm_then;
  if(thenn->kind == Tr_cx) {
    Temp_label trues_then = Temp_newlabel();
    Temp_label falses_then = Temp_newlabel();
    doPatch(thenn->u.cx.trues, trues_then);
    doPatch(thenn->u.cx.falses, falses_then);
    stm_then = T_Seq(T_Seq(T_Label(label_then),
                           T_Seq(unCx(thenn).stm,
                                 T_Seq(T_Seq(T_Label(trues_then), T_Move(r, T_Const(1))),
                                       T_Seq(T_Label(falses_then), T_Move(r, T_Const(0)))))),
                     joinjump);
  }
  else
    stm_then = T_Seq(T_Label(label_then),T_Seq(T_Move(r,unEx(thenn)),joinjump));

  if(elsee->kind == Tr_cx){
    Temp_label trues_else = Temp_newlabel();
    Temp_label falses_else = Temp_newlabel();
    doPatch(elsee->u.cx.trues,trues_else);
    doPatch(elsee->u.cx.falses,falses_else);
    stm_else = T_Seq(T_Seq(T_Label(label_else),
                           T_Seq(unCx(elsee).stm,
                                 T_Seq(T_Seq(T_Label(trues_else),T_Move(r,T_Const(1))),
                                       T_Seq(T_Label(falses_else),T_Move(r,T_Const(0)))))),
                     joinjump);
  }
  else
    stm_else = T_Seq(T_Label(label_else), T_Seq(T_Move(r, unEx(elsee)), joinjump));

  return Tr_Ex(T_Eseq(T_Seq(unCx(condition).stm,T_Seq(stm_then,stm_else)),
                      T_Eseq(T_Label(label_join),r)));

}

Tr_exp Tr_whileExp(Tr_exp condition, Tr_exp body, Temp_label label_done){
  Temp_label label_test = Temp_newlabel();

  Temp_label label_body = Temp_newlabel();

  doPatch(unCx(condition).trues,label_body);
  doPatch(unCx(condition).falses,label_done);
  T_stm jump_test = T_Jump(T_Name(label_test),Temp_LabelList(label_test,NULL));

  T_stm stm_body = T_Seq(T_Label(label_body),T_Seq(unNx(body),jump_test));
  T_stm stm_test = T_Seq(T_Label(label_test),unCx(condition).stm);
  T_stm stm_done = T_Label(label_done);

  return Tr_Nx(T_Seq(stm_test,T_Seq(stm_body,stm_done)));
}

// do it in abstract tree
Tr_exp Tr_forExp(Tr_exp lower, Tr_exp upper,
                 Tr_exp body,Tr_exp iter, Temp_label label_done){
  Temp_label label_test = Temp_newlabel();
  Temp_label label_body = Temp_newlabel();

  T_stm jump_test = T_Jump(T_Name(label_test),Temp_LabelList(label_test,NULL));
  T_stm stm_body = T_Seq(T_Label(label_body),T_Seq(unNx(body),jump_test));

  T_stm stm_head = T_Move(unEx(iter), unEx(lower));
  T_stm stm_test = T_Seq(T_Label(label_test),
                         T_Cjump(T_le,unEx(iter),unEx(upper),label_body,label_done));
  T_stm stm_done = T_Label(label_done);

  return Tr_Nx(T_Seq(stm_head,T_Seq(stm_test,T_Seq(stm_body,stm_done))));
}

Tr_exp Tr_breakExp(Temp_label label_done){
  return Tr_Nx(T_Jump(T_Name(label_done),
                      Temp_LabelList(label_done, NULL)));
}
// for typedec and funcdec
Tr_exp Tr_nullExp(){
  return Tr_Ex(T_Const(0));
}

Tr_exp Tr_novalueExp(){
  return Tr_Nx(T_Exp(T_Const(1)));
}

Tr_exp Tr_errExp(){
  return Tr_Nx(T_Exp(T_Const(-1)));
}
// reverse order of sequence
Tr_exp Tr_seqExp(Tr_expList trlist){
  if(trlist == NULL)
    return NULL;

  T_exp last_exp = unEx(trlist->head);
  trlist = trlist->tail;
  T_stm seq = NULL;
  for(;trlist; trlist = trlist->tail)
    seq = T_Seq(unNx(trlist->head),seq);

  return Tr_Ex(T_Eseq(seq, last_exp));
}

void Tr_procEntryExit(Tr_level level, Tr_exp body,Tr_accessList formals){
  proclist = F_FragList(F_ProcFrag(unNx(body),level->frame),
                        proclist);
}

F_fragList Tr_getResult(){
  F_fragList cursor = NULL, prev = NULL;
  for(cursor = strlist; cursor; cursor = cursor->tail)
    prev = cursor;
  if(prev)
    prev->tail = proclist;
  return strlist?strlist:proclist;
}
