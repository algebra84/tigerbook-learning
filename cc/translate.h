//
// Created by dadiao on 18-2-7.
//

#ifndef CC_TRANSLATE_H
#define CC_TRANSLATE_H
/* translate.h */
typedef struct Tr_exp_ *Tr_exp;
typedef struct Tr_access_ *Tr_access;
typedef struct Tr_accessList_ *Tr_accessList;
typedef struct Tr_level_ *Tr_level;
typedef struct patchList_ *patchList;
typedef enum{Tr_ex, Tr_nx, Tr_cx} Tr_expkind;
typedef struct Tr_expList_ *Tr_expList;
struct Tr_accessList_ {Tr_access head; Tr_accessList tail;};
struct Tr_expList_ {Tr_exp head; Tr_expList tail;};
Tr_accessList Tr_AccessList(Tr_access head,
                            Tr_accessList tail);

Tr_level Tr_outermost(void);
Tr_level Tr_newLevel(Tr_level parent, Temp_label name,
                     U_boolList formals);

Tr_expList Tr_ExpList(Tr_exp head, Tr_expList tail);
Tr_accessList Tr_formals(Tr_level level);
Tr_access Tr_allocLocal(Tr_level level, bool escape);
Tr_exp Tr_simpleVar(Tr_access, Tr_level);
Tr_exp Tr_fieldVar(Tr_exp recordbase, int offset);
Tr_exp Tr_subscriptVar(Tr_exp arraybase, Tr_exp index);
Tr_exp Tr_arrayExp(Tr_exp size, Tr_exp init);
Tr_exp Tr_recordExp(int n, Tr_expList list);
Tr_exp Tr_nilExp(void);
Tr_exp Tr_intExp(int consti);
Tr_exp Tr_stringExp(string s);
Tr_exp Tr_callExp(string funcname, Tr_level funclevel, Tr_level level, Tr_expList argslist);
Tr_exp Tr_seqExp(Tr_expList trlist);
Tr_exp Tr_arithExp(Tr_exp left, Tr_exp right, A_oper op);
Tr_exp Tr_relOpExp(Tr_exp left, Tr_exp right, A_oper op);
Tr_exp Tr_strEqExp(Tr_exp left, Tr_exp right, A_oper op);
Tr_exp Tr_assignExp(Tr_exp left, Tr_exp right);
Tr_exp Tr_ifExp(Tr_exp condition, Tr_exp thenn, Tr_exp elsee);
Tr_exp Tr_whileExp(Tr_exp condition, Tr_exp body, Temp_label label_done);
Tr_exp Tr_forExp(Tr_exp lower, Tr_exp upper,
                 Tr_exp body,Tr_exp iter, Temp_label label_done);
Tr_exp Tr_breakExp(Temp_label label_done);
Tr_exp Tr_nullExp(void);
Tr_exp Tr_novalueExp(void);
Tr_exp Tr_errExp(void);

void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals);
F_fragList Tr_getResult(void);
#endif //CC_TRANSLATE_H
