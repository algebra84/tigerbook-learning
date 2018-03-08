#ifndef CC_FRAME_H
#define CC_FRAME_H
typedef struct F_frame_ *F_frame;
typedef struct F_access_ *F_access;
typedef struct F_accessList_ *F_accessList;
typedef struct F_frag_ *F_frag;
typedef struct F_fragList_ *F_fragList;
typedef enum{F_stringFrag, F_procFrag} Fragkind;
struct F_frag_ {
    Fragkind kind;
    union{struct {Temp_label label;
                  string str;}stringg;
          struct{T_stm body; F_frame frame;}proc;
    }u;
};

struct F_fragList_ {F_frag head; F_fragList tail;};
struct F_accessList_ {F_access head; F_accessList tail;};
F_frame F_newFrame(Temp_label name, U_boolList formals);
Temp_label F_name(F_frame f);
F_accessList F_formals(F_frame f);
F_access F_allocLocal(F_frame f, bool escape);
F_accessList F_newlist(F_access head, F_accessList tail);


/* chapter 7 */
Temp_temp F_FP(void);
Temp_temp F_RV(void);

extern const int F_wordSize;

T_exp F_Exp(F_access acc,T_exp framePtr);
T_exp F_externalCall(string s, T_expList args);
T_stm F_procEntryExit1(F_frame frame, T_stm stm);
T_stm F_procEntrypro(F_frame frame, T_stm stm);
T_stm F_procEntryepi(F_frame frame, T_stm stm);
F_frag F_StringFrag(Temp_label label, string str);
F_frag F_ProcFrag(T_stm body, F_frame frame);
F_fragList F_FragList(F_frag head, F_fragList tail);

#endif // CC_FRAME_H