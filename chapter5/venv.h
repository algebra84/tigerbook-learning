#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "types.h"
typedef enum {E_varEntry, E_funEntry} Enum_entry;
typedef struct E_enventry_ *E_enventry;
struct E_enventry_ { Enum_entry kind;
  union {struct {Ty_ty ty;} var;
    struct {Ty_tyList formals; Ty_ty result;} fun;
  } u;
};

E_enventry E_VarEntry(Ty_ty ty);
E_enventry E_FunEntry(Ty_tyList formals, Ty_ty result);
S_table E_base_tenv(void); /* Ty_ ty environment */
S_table E_base_venv(void); /* E_ enventry environment */
