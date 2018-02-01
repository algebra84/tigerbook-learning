#include "semant.h"
#include "absyn.h"
#include "types.h"

struct expty expTy(Tr_exp exp, Ty_ty ty) {
  struct expty e;
  e.exp=exp;
  e.ty=ty;
  return e;
}
//Ty_record, Ty_nil, Ty_int, Ty_string, Ty_array,
//Ty_name, Ty_void
bool EqualTy(Ty_ty left, Ty_ty right){
  switch(left->kind){
    case Ty_array:
      return left == right;
    case Ty_record:
      return left == right || right->kind == Ty_nil;
    case Ty_int:
    case Ty_string:
      return right->kind == left->kind;
    default:
      return 0;
  }
}

struct expty transVar(S_table venv, S_table tenv, A_var v){
  switch(v->kind){
    case A_simpleVar:{
      E_enventry x = S_look(venv,v->u.simple);
      if(x && x->kind == E_varEntry)
        return expTy(NULL,x->u.var.ty);
      else{
        EM_error(v->pos,"undefined variable %s",S_name(v->u.simple));
        return expTy(NULL,Ty_Void());
      }
    }
    case A_fieldVar:{
      struct expty recordtype = transVar(venv,tenv,v->u.field.var);
      if(recordtype.ty->kind == Ty_record){
        Ty_fieldList itlist = recordtype.ty->u.record;
        for(;itlist != NULL; itlist = itlist->tail){
          Ty_field it = itlist->head;
          if(it->name == v->u.field.sym)
            return expTy(NULL,it->ty);
        }
      }
      EM_error(v->pos,"record has no such field %s\n",S_name(v->u.field.sym));
      return expTy(NULL, Ty_Void());
    }
    case A_subscriptVar:{
      struct expty arraytype = transVar(venv,tenv,v->u.subscript.var);
      struct expty arrayexp = transExp(venv,tenv,v->u.subscript.exp);
      if(arrayexp.ty->kind != Ty_int)
        EM_error(v->u.subscript.exp->pos,"subscript should result int\n");
      return expTy(NULL,arraytype.ty);
    }
  }
  
}
struct expty transExp(S_table venv, S_table tenv, A_exp a){
  switch(a->kind){

    case A_varExp:
      return transVar(venv,tenv,a->u.var);
    case A_nilExp:
      return expTy(NULL, Ty_Nil());
    case A_intExp:
      return expTy(NULL, Ty_Int());
    case A_stringExp:
      return expTy(NULL, Ty_String());
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
      for (; iterlist != NULL; iterlist = iterlist->tail) {
        iter = iterlist->head;
        if (formals == NULL)
          EM_error(iter->pos, "more than param number\n");
        else {
          param = formals->head;
          formals = formals->tail;
          struct expty param_exp = transExp(venv, tenv, iter);
          if (!EqualTy(param, param_exp.ty))
            EM_error(iter->pos, "param type not match\n");
        }
      }
      if (formals != NULL)
        EM_error(a->pos, "less than param number\n");
      return expTy(NULL, call->u.fun.result);
    }
    case A_opExp: {
      A_oper oper = a->u.op.oper;
      struct expty left = transExp(venv, tenv, a->u.op.left);
      struct expty right = transExp(venv, tenv, a->u.op.right);
      switch(oper){
        case A_plusOp:
        case A_divideOp:
        case A_minusOp:
        case A_timesOp:{
          if(left.ty->kind != Ty_int)
            EM_error(a->u.op.left->pos, "integer required\n");
          if(right.ty->kind != Ty_int)
            EM_error(a->u.op.right->pos, "integer required\n");
          return expTy(NULL, Ty_Int());
        }
        case A_geOp:
        case A_gtOp:
        case A_ltOp:
        case A_leOp:{
          if(left.ty->kind != right.ty->kind
             || (left.ty->kind != Ty_int
                 && left.ty->kind != Ty_string))
            EM_error(a->u.op.left->pos,"type can't be compared\n");
          return expTy(NULL, Ty_Int());
        }
        case A_eqOp:
        case A_neqOp:{
          if(left.ty->kind == Ty_int || left.ty->kind == Ty_string)
            if(right.ty->kind != left.ty->kind)
              EM_error(a->u.op.left->pos, "same type required\n");
            else
              return expTy(NULL, Ty_Int());
          if(left.ty->kind == Ty_record)
            if(left.ty != right.ty)
              EM_error(a->u.op.left->pos,"same record type required\n");
            else
              return expTy(NULL,Ty_Int());
          if(left.ty->kind == Ty_array)
            if(left.ty != right.ty)
              EM_error(a->u.op.left->pos, "same array type required\n");
            else
              return expTy(NULL, Ty_Int());
          EM_error(a->u.op.left->pos,"type can't be compared\n");
          return expTy(NULL, Ty_Int());
        }
      }
    }
    case A_recordExp:{
      E_enventry vartype = S_look(venv,a->u.record.typ);
      if(!vartype)
        EM_error(a->pos,"value type not declared by recordexp\n");
      assert(vartype->kind == E_varEntry);
      A_efieldList itlist = a->u.record.fields;
      A_efield iter;
      for(; itlist != NULL; itlist = itlist->tail){
        iter = itlist->head;
        transExp(venv,tenv,iter->exp);
      }
      return expTy(NULL,vartype->u.var.ty);
    }
    case  A_seqExp:{
      A_expList itlist = a->u.seq;
      A_exp iter;
      for(;itlist != NULL; itlist = itlist->tail){
        iter = itlist->head;
        transExp(venv,tenv,iter);
      }
      return expTy(NULL, Ty_Void());
    }
    case A_assignExp:{
      struct expty left = transVar(venv,tenv,a->u.assign.var);
      struct expty right = transExp(venv,tenv,a->u.assign.exp);
      if(!EqualTy(left.ty,right.ty))
        EM_error(a->u.assign.var->pos, "same type required by assign\n");
      return expTy(NULL, Ty_Void());
    }
    case A_ifExp:{
      struct expty condition = transExp(venv,tenv,a->u.iff.test);
      if(condition.ty->kind != Ty_int)
        EM_error(a->u.iff.test->pos, "int type required by iff\n");
      struct expty tthen = transExp(venv,tenv,a->u.iff.then);
      if(a->u.iff.elsee) {
        struct expty eelse = transExp(venv, tenv, a->u.iff.elsee);
        if(!EqualTy(tthen.ty,eelse.ty))
          EM_error(a->u.iff.then->pos,"same type required by iff\n");
        return expTy(NULL, tthen.ty);
      }
      if(tthen.ty->kind != Ty_void)
        EM_error(a->u.iff.then->pos,"then should produced no value by iff\n");
      return expTy(NULL, Ty_Void());
    }
    case A_whileExp:{
      struct expty condition = transExp(venv,tenv,a->u.whilee.test);
      if(condition.ty->kind != Ty_int)
        EM_error(a->u.whilee.test->pos, "int type required by while\n");
      struct expty body = transExp(venv,tenv,a->u.whilee.body);
      if(body.ty->kind != Ty_void)
        EM_error(a->u.whilee.body->pos,"body type should be void by while\n");
      return expTy(NULL, Ty_Void());
    }
    case A_forExp:{
      struct expty lower = transExp(venv,tenv,a->u.forr.lo);
      struct expty high = transExp(venv,tenv,a->u.forr.hi);
      if(lower.ty->kind != Ty_int)
        EM_error(a->u.forr.lo->pos, "lo should be int by forr\n");
      if(high.ty->kind != Ty_int)
        EM_error(a->u.forr.hi->pos, "hi should be int by forr\n");
      S_beginScope(venv);
      S_enter(venv,a->u.forr.var,E_VarEntry(Ty_Int()));
      struct expty body = transExp(venv,tenv,a->u.forr.body);
      if(body.ty->kind != Ty_void)
        EM_error(a->u.forr.body->pos, "body should be void type by forr\n");
      S_endScope(venv);
      return expTy(NULL, Ty_Void());
    }
    case A_breakExp:
      return expTy(NULL, Ty_Void());
    case A_letExp:{
      S_beginScope(tenv);
      S_beginScope(venv);
      A_decList itlist = a->u.let.decs;
      A_dec iter;
      for(;itlist != NULL; itlist=itlist->tail){
        iter = itlist->head;
        transDec(venv,tenv,iter);
      }
      assert(a->u.let.body->kind == A_seqExp);
      A_expList exprlist = a->u.let.body->u.seq;
      struct expty res = expTy(NULL, Ty_Void());
      for(;exprlist != NULL; exprlist = exprlist->tail){
        A_exp exprit;
        exprit = exprlist->head;
        res = transExp(venv,tenv,exprit);
      }
      S_endScope(venv);
      S_endScope(tenv);
      return res;
    }
    case A_arrayExp:{
      Ty_ty arraytype = S_look(tenv,a->u.array.typ);
      if(!arraytype)
        EM_error(a->pos, "unknown array type\n");
      struct expty arraysize = transExp(venv,tenv,a->u.array.size);
      struct expty arrayinit = transExp(venv,tenv,a->u.array.init);
      if(arraysize.ty->kind != Ty_int)
        EM_error(a->u.array.size->pos,"array size should be int type\n");
      if(!EqualTy(arraytype,arrayinit.ty))
        EM_error(a->u.array.init->pos, "init type conflit array type\n");
      return expTy(NULL, Ty_Array(arraytype));
    }

  }
}
void  transDec(S_table venv, S_table tenv, A_dec d){
  switch(d->kind){
    case A_typeDec:{
      A_nametyList itlist = d->u.type;
      for(;itlist != NULL; itlist=itlist->tail){
        A_namety it = itlist->head;
        S_enter(tenv,it->name,transTy(venv,tenv,it->ty));
      }
      break;
    }
    case A_varDec:{
      struct expty initexp = transExp(venv,tenv,d->u.var.init);
      if(d->u.var.typ){
        Ty_ty vartype = S_look(tenv,d->u.var.typ);
        if(!vartype)
          EM_error(d->pos, "%s is not a type\n",S_name(d->u.var.typ));
        else if(!EqualTy(vartype,initexp.ty))
          EM_error(d->u.var.init->pos,"init conflicts type %s\n",S_name(d->u.var.typ));
        else
          S_enter(tenv,d->u.var.var,vartype);
        break;
      }
      S_enter(tenv, d->u.var.var, initexp.ty);
      break;
    }
    case A_functionDec:{

      A_fundecList funcs = d->u.function;
      for(;funcs != NULL; funcs = funcs->tail){
        S_beginScope(venv);
        A_fundec = funcs->head;
        // todo
        S_endScope(venv);
      }

    }
  }
}
Ty_ty transTy (S_table venv, S_table tenv, A_ty a){
  
}
