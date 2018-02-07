#include "frame.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "table.h"

struct F_frame_{
    size_t size;
    Temp_label label;
    F_accessList formals;
    U_boolList escapes;
    F_accessList locals;
    F_accessList tmp;
    F_accessList savedRegs;
    F_accessList argsPass;
};

typedef enum {inFrame, inReg} Accesstype;
struct F_access_
{Accesstype kind;
  union {
    int offset;
    /* InFrame */
    Temp_temp reg;
    /* InReg */

  } u;
};

static F_access InFrame(int offset){
  F_access p = checked_malloc(sizeof(*p));
  p->kind = inFrame;
  p->u.offset = offset;
  return p;
}

static F_access InReg(Temp_temp reg){
  F_access p = checked_malloc(sizeof(*p));
  p->kind = inReg;
  p->u.reg = reg;
  return p;
}

F_accessList F_newlist(F_access head, F_accessList tail){
  F_accessList p;
  if(!tail)
    p = checked_malloc(sizeof(*p));
  else
    p = tail;
  p->head = head;
  p->tail = tail;
  return p;
}

F_frame F_newFrame(Temp_label name, U_boolList formals){
  F_frame p = checked_malloc(sizeof(*p));
  p->size = 4;
  p->label = name;
  p->escapes = formals;
  p->formals = NULL;
  p->locals = NULL;
  p->tmp = NULL;
  p->savedRegs = NULL;
  p->argsPass = NULL;

  U_boolList itlist = formals;


  // for amd64 0 %ebp = old %EBP, 4 %ebp = %old eip
  for(int offset = 8; itlist != NULL; itlist = itlist->tail){
    p->formals = F_newlist(InFrame(offset),p->formals);
    offset+=4;
  }
  return p;
}

Temp_label F_name(F_frame f){
  return f->label;
}
F_accessList F_formals(F_frame f){
  return f->formals;
}

F_access F_allocLocal(F_frame f, bool escape){
  if(escape) {
    f->locals = F_newlist(InFrame(f->size), f->locals);
    f->size += 4;
  }
  else
    f->locals = F_newlist(InReg(Temp_newtemp()),f->locals);

  return f->locals->head;
}