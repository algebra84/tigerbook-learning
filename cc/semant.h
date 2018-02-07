#include <lzma.h>
#include "venv.h"
#include "errormsg.h"
#include "absyn.h"

typedef void *Tr_exp;
struct expty {Tr_exp exp; Ty_ty ty;};

struct expty expTy(Tr_exp exp, Ty_ty ty);
struct expty transVar(S_table venv, S_table tenv, A_var v);
struct expty transExp(S_table venv, S_table tenv, A_exp a);
void  transDec(S_table venv, S_table tenv, A_dec d);
Ty_ty transTy (S_table venv, S_table tenv, A_ty a);
bool EqualTy(Ty_ty left, Ty_ty right);

void SEM_transProg(A_exp exp);