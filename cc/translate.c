//
// Created by dadiao on 18-2-7.
//

#include <lzma.h>
#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "frame.h"

#include "translate.h"
struct Tr_level_ {int level; Tr_level parent;F_frame frame;};

struct Tr_access_ {Tr_level level; F_access access;};

struct Tr_accessList_ {Tr_access head; Tr_accessList tail;};

Tr_level out = NULL;

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