
typedef struct F_frame_ *F_frame;
typedef struct F_access_ *F_access;


typedef struct F_accessList_ *F_accessList;
struct F_accessList_ {F_access head; F_accessList tail;};
F_frame F_newFrame(Temp_label name, U_boolList formals);
Temp_label F_name(F_frame f);
F_accessList F_formals(F_frame f);
F_access F_allocLocal(F_frame f, bool escape);
F_accessList F_newlist(F_access head, F_accessList tail);

/* chapter 7 */
Temp_temp F_FP(void);

extern const int F_wordSize;

T_exp F_Exp(F_access acc,T_exp framePtr);