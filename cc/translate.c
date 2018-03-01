//
// Created by dadiao on 18-2-7.
//

#include <lzma.h>
#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "frame.h"
#include "tree.h"
#include "translate.h"

struct Tr_level_ {int level; Tr_level parent;F_frame frame;};

struct Tr_access_ {Tr_level level; F_access access;};

struct Tr_accessList_ {Tr_access head; Tr_accessList tail;};

struct patchList_ {Temp_label *head; patchList tail;};
struct Cx {patchList trues; patchList falses; T_stm stm;};

struct Tr_exp_ {
  Tr_expkind kind;
  union{T_exp ex; T_stm nx; struct Cx cx;} u;
};

Tr_level out = NULL;

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

Tr_level Tr_newLevel(Tr_level parent, Temp_label name,
                     U_boolList formals){
  Tr_level p = checked_malloc(sizeof(*p));
  p->level = parent->level+1;
  p->parent = parent;
  p->frame = F_newFrame(name,formals);
  return p;
}

Tr_accessList Tr_formals(Tr_level level){
  F_accessList itlist = F_formals(level->frame);
  Tr_accessList res = NULL;
  for(;itlist != NULL; itlist=itlist->tail){
    Tr_access p = checked_malloc(sizeof(*p));
    Tr_accessList ret = checked_malloc(sizeof(*ret));
    p->access = itlist->head;
    p->level = level;
    ret->head = p;
    ret->tail = res;
    res = ret;
  }
  return res;
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
      res.trues = PatchList(res.stm->u.CJUMP.true,NULL);
      res.falses = PatchList(res.stm->u.CJUMP.false,NULL);
      return res;
    }
  }
  assert(0);
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
  for(; acc_level != level; acc_level = acc_level->parent)
    fp = T_Mem(T_Binop(T_plus,fp,T_Const(0)));
  return F_Exp(access->access,fp);
}

Tr_exp Tr_array() {

}