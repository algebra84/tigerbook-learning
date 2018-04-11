#ifndef CC_SEMANT_H
#define CC_SEMANT_H

#include"types.h"
struct expty {Tr_exp exp; Ty_ty ty;};

struct expty expTy(Tr_exp exp, Ty_ty ty);
struct expty transVar(S_table venv, S_table tenv, Tr_level level, A_var v);
struct expty transExp(S_table venv, S_table tenv,
                      Tr_level level, A_exp a,Temp_label breakk);
Tr_exp transDec(S_table venv, S_table tenv,
                Tr_level level, A_dec d,Temp_label breakk);
Ty_ty transTy (S_table venv, S_table tenv, A_ty a);
bool EqualTy(Ty_ty left, Ty_ty right);

F_fragList SEM_transProg(A_exp exp);

#endif