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
   mlen = plen;
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

%x STR COMMENTS VSTR
%%
<COMMENTS>{
  "*/"  {adjust(); BEGIN(0);}
  <<EOF>>  {adjust(); EM_error(EM_tokPos,"EOF in comment");return 0;}
  \n  {adjust(); EM_newline();continue;}
  . {adjust();}
}

<STR>{
  \"  {adjust();BEGIN(0);yylval.sval = String(str);return STRING;}
  \\n  {adjust(); str_add('\n'); continue;}
  \\t  {adjust(); str_add('\t'); continue;}
  \^[A-Z] {adjust(); continue;} // to do
  \\[0-9]{3}  {adjust(); str_add(atoi(yytext+1)); continue;}
  \\\"  {adjust(); str_add('\"'); continue;}
  \\  {adjust(); str_add('\\'); continue;}
  \\f  {adjust(); BEGIN(VSTR);}
  \\^[ntf\"\\] {adjust(); EM_error(EM_tokPos,"no such control characters");return 0;}

}

<VSTR>{
  f\\ {adjust(); BEGIN(STR);}
  \n {adjust(); EM_newline(); continue;}
  \t {adjust(); continue;}
  \f {adjust(); continue;}
  . {adjust(); EM_error(EM_tokPos, "not formatting characters"); return 0;}
 }

"/*" {adjust();BEGIN(COMMENTS);}
\"  {adjust();str_init();BEGIN(STR);}
" "	 {adjust(); continue;}
""  {adjust(); continue;}
\n	 {adjust(); EM_newline(); continue;}
\t  {adjust(); continue;}
","	 {adjust(); return COMMA;}
":"  {adjust(); return COLON;}
";"  {adjust(); return SEMICOLON;}
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
"<="  {adjust(); return LE;}
">"  {adjust(); return GT;}
">="  {adjust(); return GE;}
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
if  {adjust();return IF;}
then {adjust();return THEN;}
else  {adjust();return ELSE;}
do  {adjust();return DO;}
of  {adjust();return OF;}
nil {adjust();return NIL;}

[0-9]+	 {adjust(); yylval.ival=atoi(yytext); return INT;}
[a-zA-Z][a-zA-Z0-9_]* {adjust(); yylval.sval=String(yytext); return ID;}
.	 {adjust(); EM_error(EM_tokPos,"illegal token");}



