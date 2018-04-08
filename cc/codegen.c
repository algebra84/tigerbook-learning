static Temp_temp munchExp(T_exp e) {
  switch( e )
  case MEM ( BINOP ( PLUS ,e1, CONST (i))): {
    Temp_temp r = Temp_newtemp();
    emit(AS_Oper("LOAD 'd0 <- M['s0+\" + i + \"]\n",
                 L(r,NULL), L(munchExp(e1),NULL), NULL));
    return r; 
  }
 case MEM ( BINOP ( PLUS , CONST (i),e1)): {
   Temp_temp r = Temp_newtemp();
   emit(AS_Oper("LOAD 'd0 <- M['s0+\" + i + \"]\n",
                L(r,NULL), L(munchExp(e1),NULL), NULL));
   return r; 
 }
 case MEM ( CONST (i)): {
   Temp_temp r = Temp_newtemp();
   emit(AS_Oper("LOAD 'd0 <- M[r0+\" + i + \"]\n",
                L(r,NULL), NULL, NULL));
   return r; 
 }
 case MEM (e1): {
   Temp_temp r = Temp_newtemp();
   emit(AS_Oper("LOAD 'd0 <- M['s0+0]\n",
                L(r,NULL), L(munchExp(e1),NULL), NULL));
   return r; 
 }
 case BINOP ( PLUS ,e1, CONST (i)): {
   Temp_temp r = Temp_newtemp();
   emit(AS_Oper("ADDI 'd0 <- 's0+" + i + "\n",
                L(r,NULL), L(munchExp(e1),NULL), NULL));
   return r; 
 }
 case BINOP ( PLUS , CONST (i),e1): {
   Temp_temp r = Temp_newtemp();
   emit(AS_Oper("ADDI 'd0 <- 's0+" + i + "\n",
                L(r,NULL), L(munchExp(e1),NULL), NULL));
   return r; 
 }
 case CONST (i): {
   Temp_temp r = Temp_newtemp();
   emit(AS_Oper("ADDI 'd0 <- r0+" + i + "\n",
                NULL, L(munchExp(e1),NULL), NULL));
   return r; 
 }
 case BINOP ( PLUS ,e1,e2): {
   Temp_temp r = Temp_newtemp();
   emit(AS_Oper("ADD 'd0 <- 's0+'s1\n",
                L(r,NULL), L(munchExp(e1),L(munchExp(e2),NULL)), NULL));
   return r; 
 }
 case TEMP (t):
   return t;
}


Temp_tempList L(Temp_temp h, Temp_tempList t) {return Temp_TempList(h,t);}
static void munchStm(T_stm s) {
  switch ( s )
  case MOVE ( MEM ( BINOP ( PLUS ,e1, CONST (i))),e2):
    emit(AS_Oper("STORE M['s0+\" + i + \"] <- 's1\n",
                 NULL, L(munchExp(e1), L(munchExp(e2), NULL)), NULL));
 case MOVE( MEM ( BINOP ( PLUS , CONST (i),e1)),e2):
   emit(AS_Oper("STORE M['s0+\" + i + \"] <- 's1\n",
                NULL, L(munchExp(e1), L(munchExp(e2), NULL)),NULL));
 case MOVE ( MEM (e1), MEM (e2)):
   emit(AS_Oper("MOVE M['s0] <- M['s1]\n",
                NULL, L(munchExp(e1), L(munchExp(e2), NULL)),NULL));
 case MOVE ( MEM ( CONST (i)),e2):
   emit(AS_Oper("STORE M[r0+\" + i + \"] <- 's0\n",
                NULL, L(munchExp(e2), NULL), NULL));
 case MOVE ( MEM (e1),e2):
   emit(AS_Oper("STORE M['s0] <- 's1\n",
                NULL, L(munchExp(e1), L(munchExp(e2), NULL)),NULL));
 case MOVE ( TEMP (i), e2):
   emit(AS_Move("ADD 'd0 <- 's0 + r0\n",
                i, munchExp(e2)));
 case LABEL (lab):
   emit(AS_Label(Temp_labelstring(lab) + ":\n", lab));

}

static AS_instrList iList=NULL, last=NULL;
static void emit(AS_instr inst) {
  if (last!=NULL)
    last = last->tail = AS_InstrList(inst,NULL);
  else last = iList = AS_InstrList(inst,NULL);

}
AS_instrList F_codegen(F_frame f, T_stmList stmList){
  AS_instrList list; T_stmList sl;
  /* miscellaneous initializations as necessary */
  {
    for (sl=stmList; sl; sl=sl->tail) munchStm(sl->head);
    list=iList; iList=last=NULL; return list;

  }
}
