#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "frame.h"
#include "tree.h"

// registers
static Temp_temp fp = NULL;
static Temp_temp rv = NULL;
F_WORD_SIZE = 4;

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

struct F_frame_ {
    size_t size;
    Temp_label label;
    F_accessList formals;
    U_boolList escapes;
    F_accessList locals;
    F_accessList tmp;
    F_accessList savedRegs;
    F_accessList argsPass;
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
  F_accessList p = checked_malloc(sizeof(*p));
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

//  increase offset by F_alloc
//  // for amd64 0 %ebp = old %EBP, 4 %ebp = %old eip
//  for(int offset = 8; itlist != NULL; itlist = itlist->tail){
//    p->formals = F_newlist(InFrame(offset),p->formals);
//    offset+=4;
//  }
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

Temp_temp F_FP(void){
  if(!fp)
    fp = Temp_newtemp();
  return fp;
}

Temp_temp F_RV(void){
  if(!rv)
    rv = Temp_newtemp();
  return rv;
}
T_exp F_Exp(F_access acc,T_exp framePtr){
  if(acc->kind == inReg)
    return T_Temp(acc->u.reg);
  else {
    assert(acc->kind == inFrame);
    return T_Mem(T_Binop(T_plus, framePtr, T_Const(acc->u.offset)));
  }
}

T_exp F_externalCall(string s, T_expList args){
  return T_Call(Temp_namedlabel(s),args);
}

T_stm F_procEntryExit1(F_frame frame, T_stm stm){

}