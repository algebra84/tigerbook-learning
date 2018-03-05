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
Tr_accessList Tr_formals(Tr_level level);
Tr_access Tr_allocLocal(Tr_level level, bool escape);
Tr_exp Tr_simpleVar(Tr_access, Tr_level);

void Tr_procEntryExit(Tr_level level, Tr_exp body, Tr_accessList formals);
F_fragList Tr_getResult(void);
#endif //CC_TRANSLATE_H
