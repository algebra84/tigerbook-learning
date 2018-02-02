#include "venv.h"


E_enventry E_VarEntry(Ty_ty ty){
  E_enventry p = checked_malloc(sizeof(*p));
  p->kind = E_varEntry;
  p->u.var.ty = ty;
  return p;
}

E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result){
  E_enventry p = checked_malloc(sizeof(*p));
  p->kind = E_funEntry;
  p->u.fun.formals = formals;
  p->u.fun.result = result;
  return p;
}
/* Ty_ ty environment */
S_table E_base_tenv(){
  S_table t = S_empty();
  S_enter(t,S_Symbol("int"),Ty_Int());
  S_enter(t,S_Symbol("string"), Ty_String());
  return t;
}
/* E_ enventry environment */
S_table E_base_venv(){
  S_table t = S_empty();
  // print(s:string) 
  Ty_tyList formals = Ty_TyList(Ty_String(),NULL);
  Ty_ty result = Ty_Void();
  E_enventry func = E_FunEntry(formals, result);
  S_enter(t,S_Symbol("print"),func);

  //flush() function
  formals = NULL;
  result = Ty_Void();
  func = E_FunEntry(formals, result);
  S_enter(t,S_Symbol("flush"),func);

  //getchar() :string
  formals = NULL;
  result = Ty_String();
  func = E_FunEntry(formals, result);
  S_enter(t,S_Symbol("getchar"),func);

  // chr(i: int) :string
  formals = Ty_TyList(Ty_Int(), NULL);
  result = Ty_String();
  func = E_FunEntry(formals, result);
  S_enter(t,S_Symbol("chr"),func);

  //size(s:string) :int
  formals = Ty_TyList(Ty_String(), NULL);
  result = Ty_Int();
  func = E_FunEntry(formals, result);
  S_enter(t,S_Symbol("size"),func);

  //ord(s:string) :int
  formals = Ty_TyList(Ty_String(), NULL);
  result = Ty_Int();
  func = E_FunEntry(formals, result);
  S_enter(t,S_Symbol("ord"),func);

  //substring(s:string,first:int, n:int) :string
  formals = Ty_TyList(Ty_String(), Ty_TyList(Ty_Int(),Ty_TyList(Ty_Int(),NULL)));
  result = Ty_String();
  func = E_FunEntry(formals, result);
  S_enter(t,S_Symbol("substring"),func);

  // concat(s1:string, s2:string):string
  formals = Ty_TyList(Ty_String(), Ty_TyList(Ty_String(),NULL));
  result = Ty_String();
  func = E_FunEntry(formals, result);
  S_enter(t,S_Symbol("concat"),func);
  // not(i:integer):integer
  formals = Ty_TyList(Ty_Int(),NULL);
  result = Ty_Int();
  func = E_FunEntry(formals, result);
  S_enter(t,S_Symbol("not"),func);
  // exit(i:int)
  formals = Ty_TyList(Ty_Int(), NULL);
  result = Ty_Void();
  func = E_FunEntry(formals, result);
  S_enter(t,S_Symbol("exit"),func);

  return t;
}
