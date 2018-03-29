#ifndef CC_VENV_H
#define CC_VENV_H

typedef enum {E_varEntry, E_funEntry} Enum_entry;
typedef struct E_enventry_ *E_enventry;
struct E_enventry_ { Enum_entry kind;
  union {struct {Tr_access access; Ty_ty ty;} var;

         struct {Tr_level level; Temp_label label;
                 Ty_tyList formals; Ty_ty result;} fun;
  } u;
};
typedef struct E_escentry_ *E_escentry;
struct E_escentry_ {int depth; bool escape;};

E_escentry EscapeEntry(int depth, bool escape);
E_enventry E_VarEntry(Tr_access access, Ty_ty ty);
E_enventry E_FunEntry(Tr_level level, Temp_label label,
                      Ty_tyList formals, Ty_ty result);
S_table E_base_tenv(void); /* Ty_ ty environment */
S_table E_base_venv(void); /* E_ enventry environment */

#endif