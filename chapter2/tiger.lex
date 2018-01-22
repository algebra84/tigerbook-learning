%{
#include <string.h>
#include "util.h"
#include "tokens.h"
#include "errormsg.h"

int charPos=1;

int yywrap(void)
{
 charPos=1;
 return 1;
}


void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}

 char *str = NULL;
 int len,mlen;
 const int plen = 100;
 void str_init(){
   str = checked_malloc(plen);
   mlem = plen;
   str[0]='\0';
   len = 0;
 }
 void str_add(char c){
   if(len+1 == mlen){
     str = realloc(str,mlen+plen);
     if(!str){
       printf("malloc error\n");
       exit(1);
     }
     mlen += plen;
   }
   str[len++] = c;
   str[len]='\0';
 }
%}

%x STR COMMENTS
%%
<COMMENTS>{
  ("*/")+  {adjust(); BEGIN(0);}
  <<EOF>>  {adjust(); EM_error(EM_TokPos,"EOF in comment");return 0;}
  \n  {adjust(); EM_newline();continue;}
  . {adjust();}
}

<STR>{
  \"  {adjust();BEGIN(0);yylval.sval = String(str);return STRING;}
  "\n"  {adjust(); str_add('\n');}
  "\t"  {adjust(); str_add('\t');}
  \\^[a-zA-Z]  {adjust();}
  \\[0-9]{3}  {adjust(); str_add(atoi(yytext+1));}
  "\""  {adjust(); str_add('\"');}
  \\  {adjust(); std_add('\\');}
  \\[^ntf\"\\]
  \\.*\\ {adjust();}

}

"/*" {adjust();BEGIN(COMMENTS);}
\"  {adjust();str_init();BEGIN(STR);}
" "	 {adjust(); continue;}
\n	 {adjust(); EM_newline(); continue;}
","	 {adjust(); return COMMA;}
":"  {adjust(); return COLON;}
"("  {adjust(); return LPAREN;}
")"  {adjust(); return RPAREN;}
"["  {adjust(); return LBRACK;}
"]"  {adjust(); return RBRACK;}
"{"  {adjust(); return LBRACE;}
"}"  {adjust(); return RBRACE;}
"."  {adjust(); return DOT;}
"+"  {adjust(); return PLUS;}
"-"  {adjust(); return MINUS;}
"*"  {adjust(); return TIMES;}
"/"  {adjust(); return DIVIDE;}
"="  {adjust(); return EQ;}
"<>"  {adjust(); return NEQ;}
"<"  {adjust(); return LT;}
"<="  {adjust(); return LEQ;}
">"  {adjust(); return GT;}
">="  {adjust(); return GEQ;}
"&"  {adjust(); return AND;}
"|"  {adjust(); return OR;}
":="  {adjust(); return ASSIGN;}

for  	 {adjust(); return FOR;}
while {adjust();return WHILE;}
to  {adjust();return TO;}
break  {adjust();return BREAK;}
let  {adjust();return LET;}
in  {adjust();return IN;}
end {adjust();return END;}
function {adjust();return FUNCTION;}
var  {adjust();return VAR;}
type  {adjust();return TYPE;}
array  {adjust();return ARRAY;}
if  {adjust();return ADJUST;}
then {adjust();return THEN;}
else  {adjust();return ELSE;}
do  {adjust();return DO;}
of  {adjust();return OF;}
nil {adjust();return NIL}

[0-9]+	 {adjust(); yylval.ival=atoi(yytext); return INT;}
.	 {adjust(); EM_error(EM_tokPos,"illegal token");}



