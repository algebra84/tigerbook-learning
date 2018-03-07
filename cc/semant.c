#include <lzma.h>
#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "types.h"
#include "absyn.h"
#include "temp.h"
#include "translate.h"
#include "venv.h"
#include "semant.h"
#include "frame.h"

struct expty expTy(Tr_exp exp, Ty_ty ty) {
  struct expty e;
  e.exp=exp;
  e.ty=ty;
  return e;
}
static int in_loop = 0;
Ty_ty actual_ty(Ty_ty namety){
  if(!namety || namety->kind != Ty_name)
    return namety;

  return actual_ty(namety->u.name.ty);
}
//Ty_record, Ty_nil, Ty_int, Ty_string, Ty_array,
//Ty_name, Ty_void
bool EqualTy(Ty_ty left, Ty_ty right){
  Ty_ty l = actual_ty(left);
  Ty_ty r = actual_ty(right);
  switch(l->kind){
    case Ty_array:
      return l == r;
    case Ty_record:
      return l == r || r->kind == Ty_nil;
    case Ty_int:
    case Ty_string:
    case Ty_void:
    case Ty_nil:
      return r->kind == l->kind;
    case Ty_name:
    default:
      return 0;
  }
}

// only tranlate true semantic analysis
struct expty transVar(S_table venv, S_table tenv, Tr_level level,A_var v){
  switch(v->kind){
    case A_simpleVar:{
      E_enventry x = S_look(venv,v->u.simple);
      if(x && x->kind == E_varEntry)
        return expTy(Tr_simpleVar(x->u.var.access,level),
                     actual_ty(x->u.var.ty));
      else{
        EM_error(v->pos,"undefined variable %s",S_name(v->u.simple));
        return expTy(NULL,Ty_Void());
      }
    }
    case A_fieldVar:{
      struct expty recordtype = transVar(venv,tenv,level,v->u.field.var);
      if(recordtype.ty->kind == Ty_record){
        Ty_fieldList itlist = recordtype.ty->u.record;
        int offset = 0;
        for(;itlist != NULL; itlist = itlist->tail){
          Ty_field it = itlist->head;
          if(it->name == v->u.field.sym)
            return expTy(Tr_fieldVar(recordtype.exp,offset),
                         actual_ty(it->ty));
          offset++;
        }
        EM_error(v->pos,"record has no such field %s\n",S_name(v->u.field.sym));
      }
      else
        EM_error(v->pos,"variable not record\n");
      return expTy(NULL, Ty_Void());
    }
    case A_subscriptVar:{
      struct expty arraytype = transVar(venv,tenv,level,v->u.subscript.var);
      struct expty arrayexp = transExp(venv,tenv,level,v->u.subscript.exp,NULL);
      if(arraytype.ty->kind != Ty_array) {
        EM_error(v->u.subscript.var->pos, "value is not array\n");
        return expTy(NULL,actual_ty(arraytype.ty->u.array));
      }
      if(arrayexp.ty->kind != Ty_int) {
        EM_error(v->u.subscript.exp->pos, "subscript should result int\n");
        return expTy(NULL,actual_ty(arraytype.ty->u.array));
      }
      return expTy(Tr_subscriptVar(arraytype.exp,arrayexp.exp),
                   actual_ty(arraytype.ty->u.array));
    }
  }
  
}
struct expty transExp(S_table venv, S_table tenv,
                      Tr_level level,A_exp a,Temp_label breakk){
  switch(a->kind){

    case A_varExp:
      return transVar(venv,tenv,level,a->u.var);
    case A_nilExp:
      return expTy(Tr_nilExp(), Ty_Nil());
    case A_intExp:
      return expTy(Tr_intExp(a->u.intt), Ty_Int());
    case A_stringExp:
      return expTy(Tr_stringExp(a->u.stringg), Ty_String());
    case A_callExp: {
      E_enventry call = S_look(venv, a->u.call.func);
      if (call == NULL || call->kind == E_varEntry) {
        EM_error(a->pos, "function %s not defined\n", S_name(a->u.call.func));
        return expTy(NULL, Ty_Void());
      }

      A_expList iterlist = a->u.call.args;
      A_exp iter;

      Ty_tyList formals = call->u.fun.formals;
      Ty_ty param;
      Tr_expList trexpl = NULL;
      for (; iterlist != NULL; iterlist = iterlist->tail) {
        iter = iterlist->head;
        if (formals == NULL)
          EM_error(iter->pos, "more than param number\n");
        else {
          param = formals->head;
          formals = formals->tail;
          struct expty param_exp = transExp(venv, tenv, level,iter,breakk);

          // paramlist pass to call
          trexpl = Tr_ExpList(param_exp.exp,trexpl);

          if (!EqualTy(param, param_exp.ty))
            EM_error(iter->pos, "param type not match\n");
        }
      }
      if (formals != NULL)
        EM_error(a->pos, "less than param number\n");
      return expTy(Tr_callExp(S_name(a->u.call.func),
                              call->u.fun.level,level,trexpl),
                   actual_ty(call->u.fun.result));
    }
    case A_opExp: {
      A_oper oper = a->u.op.oper;
      struct expty left = transExp(venv, tenv, level, a->u.op.left,breakk);
      struct expty right = transExp(venv, tenv, level, a->u.op.right,breakk);
      switch(oper){
        case A_plusOp:
        case A_divideOp:
        case A_minusOp:
        case A_timesOp:{
          if(left.ty->kind != Ty_int)
            EM_error(a->u.op.left->pos, "integer required\n");
          if(right.ty->kind != Ty_int)
            EM_error(a->u.op.right->pos, "integer required\n");
          return expTy(Tr_arithExp(left.exp,right.exp,oper),
                       Ty_Int());
        }
        case A_geOp:
        case A_gtOp:
        case A_ltOp:
        case A_leOp:{
          if(left.ty->kind != right.ty->kind
             || left.ty->kind != Ty_int)
            EM_error(a->u.op.left->pos,"comparison of imcompatible types\n");
          return expTy(Tr_relOpExp(left.exp,right.exp,oper), Ty_Int());
        }
        case A_eqOp:
        case A_neqOp:{
          if((left.ty->kind == Ty_int)
              &&  EqualTy(left.ty,right.ty))
            return expTy(Tr_relOpExp(left.exp,right.exp,oper),
                         Ty_Int());
          if(left.ty->kind == Ty_string
             && EqualTy(left.ty,right.ty))
            return expTy(Tr_strEqExp(left.exp,right.exp,oper),
                         Ty_Int());
          if((left.ty->kind == Ty_record)
              && EqualTy(left.ty,right.ty))
            return expTy(Tr_relOpExp(left.exp,right.exp,oper),
                         Ty_Int());
          if((left.ty->kind == Ty_array)
             && EqualTy(left.ty,right.ty))
            return expTy(Tr_relOpExp(left.exp,right.exp,oper),
                         Ty_Int());
          if((left.ty->kind == Ty_name)
              && EqualTy(left.ty,right.ty))
            return expTy(Tr_relOpExp(left.exp,right.exp,oper),
                         Ty_Int());
          EM_error(a->u.op.left->pos,"comparison of imcompatible types\n");
          return expTy(NULL, Ty_Int());
        }
      }
    }
    case A_recordExp:{
      Ty_ty recordtype = actual_ty(S_look(tenv,a->u.record.typ));
      if(!recordtype) {
        EM_error(a->pos, "%s is not record type\n", S_name(a->u.record.typ));
        return expTy(NULL, Ty_Record(NULL));
      }
      A_efieldList itlist = a->u.record.fields;
      Ty_fieldList tylist = recordtype->u.record;

      // record tr_expList and number of field
      Tr_expList trlist = NULL;
      int offset = 0;

      for(; itlist != NULL; itlist = itlist->tail){
        A_efield iter = itlist->head;
        offset++;
        struct expty tmpty = transExp(venv,tenv,level,iter->exp,breakk);

        // add field tr_exp to trlist
        trlist = Tr_ExpList(tmpty.exp,trlist);

        if(tylist) {
          Ty_field tf = tylist->head;
          tylist = tylist->tail;
          if(!EqualTy(tf->ty,tmpty.ty))
            EM_error(iter->exp->pos,"type: %s can't be initialed with type: %s in record\n",
            S_name(tf->name),S_name(iter->name));
        }
        else
          EM_error(a->pos,"value:%s is not initial value for record\n",
                   S_name(iter->name));
      }
      if(tylist)
        EM_error(a->pos,"%s isn't initialed in record",
                 S_name(tylist->head->name));
      return expTy(Tr_recordExp(offset, trlist),recordtype);
    }
    case  A_seqExp:{
      A_expList itlist = a->u.seq;
      A_exp iter;

      Tr_expList trlist = NULL;
      for(;itlist != NULL; itlist = itlist->tail){
        iter = itlist->head;
        struct expty res = transExp(venv,tenv,level,iter,breakk);
        trlist = Tr_ExpList(res.exp,trlist);
        if(!itlist->tail)
          return res;
      }
      return expTy(Tr_seqExp(trlist), Ty_Void());
    }
    case A_assignExp:{
      struct expty left = transVar(venv,tenv,level,a->u.assign.var);
      struct expty right = transExp(venv,tenv,level,a->u.assign.exp,breakk);
      if(!EqualTy(left.ty,right.ty))
        EM_error(a->u.assign.var->pos, "same type required by assign\n");
      return expTy(Tr_assignExp(left.exp,right.exp), Ty_Void());
    }
    case A_ifExp:{
      struct expty condition = transExp(venv,tenv,level,a->u.iff.test,breakk);
      if(condition.ty->kind != Ty_int)
        EM_error(a->u.iff.test->pos, "int type required by iff\n");
      struct expty tthen = transExp(venv,tenv,level,a->u.iff.then,breakk);
      if(a->u.iff.elsee) {
        struct expty eelse = transExp(venv, tenv, level, a->u.iff.elsee,breakk);
        if(!EqualTy(tthen.ty,eelse.ty))
          EM_error(a->u.iff.then->pos,"types of then - else differ\n");
        return expTy(Tr_ifExp(condition.exp,tthen.exp,eelse.exp),
                     tthen.ty);
      }
      if(tthen.ty->kind != Ty_void)
        EM_error(a->u.iff.then->pos,"then should produced no value\n");
      return expTy(Tr_ifExp(condition.exp,tthen.exp,NULL), Ty_Void());
    }
    case A_whileExp:{
      struct expty condition = transExp(venv,tenv,level,a->u.whilee.test,breakk);
      if(condition.ty->kind != Ty_int)
        EM_error(a->u.whilee.test->pos, "int type required by while\n");
      in_loop = 1;

      //pass breakk to body
      Temp_label label_done = Temp_newlabel();
      struct expty body = transExp(venv,tenv,level,a->u.whilee.body,breakk);
      in_loop = 0;
      if(body.ty->kind != Ty_void)
        EM_error(a->u.whilee.body->pos,"body of while not void\n");
      return expTy(Tr_whileExp(condition.exp,body.exp,label_done),
                   Ty_Void());
    }
    case A_forExp:{
      struct expty lower = transExp(venv,tenv,level,a->u.forr.lo,breakk);
      struct expty high = transExp(venv,tenv,level,a->u.forr.hi,breakk);
      Temp_label label_done = Temp_newlabel();
      if(lower.ty->kind != Ty_int)
        EM_error(a->u.forr.lo->pos, "lo should be int by forr\n");
      if(high.ty->kind != Ty_int)
        EM_error(a->u.forr.hi->pos, "hi should be int by forr\n");
      S_beginScope(venv);
      Tr_access iter = Tr_allocLocal(level,TRUE);
      S_enter(venv,a->u.forr.var,E_VarEntry(iter,Ty_Int()));
      in_loop = 1;
      struct expty body = transExp(venv,tenv,level,a->u.forr.body,breakk);
      in_loop = 0;
      if(body.ty->kind != Ty_void)
        EM_error(a->u.forr.body->pos, "body should be void type by forr\n");
      Tr_exp forloop = Tr_forExp(lower.exp,high.exp,body.exp,
                                 Tr_simpleVar(iter,level),label_done);
      S_endScope(venv);
      return expTy(forloop, Ty_Void());
    }
    case A_breakExp: {
      if (!in_loop)
        EM_error(a->pos, "break should in loop\n");
      return expTy(Tr_breakExp(breakk), Ty_Void());
    }
    case A_letExp:{
      S_beginScope(tenv);
      S_beginScope(venv);
      A_decList itlist = a->u.let.decs;
      A_dec iter;
      for(;itlist != NULL; itlist=itlist->tail){
        iter = itlist->head;
        transDec(venv,tenv,level,iter,breakk);
      }
      assert(a->u.let.body->kind == A_seqExp);
      A_expList exprlist = a->u.let.body->u.seq;
      struct expty res = expTy(NULL, Ty_Void());
      for(;exprlist != NULL; exprlist = exprlist->tail){
        A_exp exprit;
        exprit = exprlist->head;
        res = transExp(venv,tenv,level,exprit,breakk);
      }
      S_endScope(venv);
      S_endScope(tenv);
      return res;
    }
    case A_arrayExp:{
      Ty_ty arraytype = actual_ty(S_look(tenv,a->u.array.typ));
      if(!arraytype)
        EM_error(a->pos, "unknown array type\n");
      struct expty arraysize = transExp(venv,tenv,level,a->u.array.size,breakk);
      struct expty arrayinit = transExp(venv,tenv,level,a->u.array.init,breakk);
      if(arraysize.ty->kind != Ty_int)
        EM_error(a->u.array.size->pos,"array size should be int type\n");
      if(!EqualTy(arraytype->u.array,arrayinit.ty))
        EM_error(a->u.array.init->pos, "init type conflit array type\n");
      return expTy(NULL, arraytype);
    }

  }
}

void transDec_recur(S_table venv, S_table tenv,
                    A_nametyList begin, A_nametyList end){
  S_table local_env = S_empty();
  A_nametyList it;
  for(it = begin; it != end; it = it->tail){
    A_namety nty = it->head;
    assert(nty->ty->kind == A_arrayTy || nty->ty->kind == A_recordTy);
    if(S_look(local_env,nty->name))
      EM_error(nty->ty->pos, "%s redecalared in recur dec\n",
      S_name(nty->name));
    S_enter(local_env,nty->name, Ty_Name(nty->name,NULL));
    S_enter(tenv,nty->name,Ty_Name(nty->name,NULL));
  }

  for(it = begin; it != end; it = it->tail){
    Ty_ty dty = S_look(tenv, it->head->name);
    assert(dty->kind == Ty_name);
    dty->u.name.ty = transTy(venv, tenv,it->head->ty);
  }
}

Tr_exp  transDec(S_table venv, S_table tenv, Tr_level level,A_dec d,Temp_label breakk){
  switch(d->kind){
    case A_typeDec:{
      A_nametyList itlist = d->u.type;
      A_nametyList begin = NULL,end = NULL;
      int in_recur = 0;
      for(;itlist != NULL; itlist=itlist->tail){
        A_namety it = itlist->head;
        if(!in_recur &&
           (it->ty->kind == A_recordTy || it->ty->kind == A_arrayTy)){
          begin = itlist;
          in_recur = 1;
          continue;
        }
        if(in_recur) {
          if (it->ty->kind == A_nameTy) {
            end = itlist;
            transDec_recur(venv, tenv, begin, end);
            in_recur = 0;
          }
          else
            continue;
        }

        S_enter(tenv,it->name,transTy(venv,tenv,it->ty));
      }
      if(in_recur)
        transDec_recur(venv,tenv,begin,NULL);
      return Tr_nullExp();
    }
    case A_varDec:{
      struct expty initexp = transExp(venv,tenv,level,d->u.var.init,breakk);
      if(d->u.var.typ){
        Ty_ty vartype = S_look(tenv,d->u.var.typ);
        if(!vartype)
          EM_error(d->pos, "%s is not a type\n",S_name(d->u.var.typ));
        else {
          if (!EqualTy(vartype, initexp.ty))
            EM_error(d->u.var.init->pos,"initial value conflicts type %s\n",
                     S_name(d->u.var.typ));
            S_enter(venv, d->u.var.var, E_VarEntry(Tr_allocLocal(level,TRUE),vartype));
        }
        break;
      }
      if(initexp.ty->kind == Ty_nil)
        EM_error(d->u.var.init->pos, "unvalid initial exp Nil\n");
      S_enter(venv, d->u.var.var, E_VarEntry(Tr_allocLocal(level,TRUE),initexp.ty));
      break;
    }
    case A_functionDec:{

      A_fundecList funcs;
      S_table local_venv = S_empty();
      // trans declare of functions
      for(funcs = d->u.function;funcs != NULL; funcs = funcs->tail){
        A_fundec func = funcs->head;
        if(S_look(local_venv,func->name))
          EM_error(func->pos,"%s redeclared in recur fundecs\n",
                   S_name(func->name));
        A_fieldList fields = func->params;
        Ty_tyList formals = NULL;
        Ty_ty res = Ty_Void();
        if(func->result) {
          res = S_look(tenv, func->result);
          if(!res) {
            EM_error(func->pos, "return type %s undefined\n", S_name(func->result));
            res = Ty_Int();
          }
        }

        for(;fields != NULL; fields = fields->tail){
          Ty_ty field = S_look(tenv,fields->head->typ);
          if(!field) {
            EM_error(fields->head->pos, "field %s type undefined\n", S_name(fields->head->name));
            formals = Ty_TyList(Ty_Int(),formals);
          }
          else {
            formals = Ty_TyList(field, formals);
          }
        }

        // reverse formals orders: up to down
        Ty_tyList tmp = formals;
        formals = NULL;
        U_boolList ublist = NULL;
        for(;tmp;tmp = tmp->tail){
          ublist = U_BoolList(TRUE,ublist);
          Ty_ty field = tmp->head;
          formals = Ty_TyList(field,formals);
        }
        Temp_label func_label = Temp_newlabel();
        S_enter(local_venv,func->name,
                E_FunEntry(Tr_newLevel(level,func_label,ublist),
                           func_label,formals, res));
        S_enter(venv,func->name,E_FunEntry(Tr_newLevel(level,func_label,ublist),
                                           func_label,formals, res));
      }

      // trans body of functions
      for(funcs = d->u.function;funcs != NULL; funcs = funcs->tail){
        S_beginScope(venv);
        A_fundec func = funcs->head;
        A_fieldList fields = func->params;
        E_enventry funentry = S_look(venv,func->name);
        Tr_accessList formals = Tr_formals(funentry->u.fun.level);

        for(;fields != NULL; fields = fields->tail){
          Ty_ty field = S_look(tenv,fields->head->typ);
          if(field) {
            S_enter(venv, fields->head->name, E_VarEntry(formals->head, field));
            formals=formals->tail;
          }
        }

        struct expty ret = transExp(venv,tenv,level,func->body,breakk);
        Ty_ty res = ((E_enventry) (S_look(venv,func->name)))->u.fun.result;
        if(!EqualTy(res,ret.ty))
          EM_error(func->body->pos,"conflict return type \n");
        S_endScope(venv);
      }

    }
  }
}

Ty_ty transTy (S_table venv, S_table tenv, A_ty a){
  switch(a->kind){
    case A_nameTy: {
      Ty_ty res = S_look(tenv, a->u.name);
      if(!res) {
        EM_error(a->pos, "type %s undefined\n", S_name(a->u.name));
        res = Ty_Int();
      }
      return res;
    }
    case A_recordTy:{
      Ty_fieldList tflist = NULL;
      A_fieldList aflist = a->u.record;
      for(;aflist != NULL; aflist=aflist->tail){
        A_field af = aflist->head;
        Ty_ty ty = S_look(tenv,af->typ);
        if(!ty){
          EM_error(af->pos,"type %s undefined\n",S_name(af->typ));
          ty = Ty_Int();
        }
        Ty_field tf = Ty_Field(af->name,ty);
        tflist = Ty_FieldList(tf,tflist);
      }
      // reverse field order: up to down
      Ty_fieldList tmplist = tflist;
      tflist = NULL;
      for(;tmplist;tmplist = tmplist->tail){
        Ty_field tmp = tmplist->head;
        tflist = Ty_FieldList(tmp,tflist);
      }
      return Ty_Record(tflist);
    }
    case A_arrayTy:{
      Ty_ty tf = S_look(tenv,a->u.array);
      if(!tf){
        EM_error(a->pos,"type %s undefined\n",S_name(a->u.array));
        tf = Ty_Int();
      }
      return Ty_Array(tf);
    }
  }
}

F_fragList SEM_transProg(A_exp prog){
  S_table venv = E_base_venv();
  S_table tenv = E_base_tenv();
  Tr_level level = Tr_newLevel(Tr_outermost(),Temp_newlabel(),NULL);
  transExp(venv,tenv,level,prog,NULL);
  return NULL;
}