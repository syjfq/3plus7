%{
#include <stdio.h>

#include "syntax.tab.h"
int yycolumn = 1;
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;

extern uint8_t right;
extern int line_error[1024];
%}

%option yylineno
delim       [ \t]
ws          {delim}+
INT  0|[1-9][0-9]*
FLOAT (0|[1-9][0-9]*)\.[0-9]+
ID [a-zA-Z_]+([a-zA-Z_]|[0-9])*
TYPE  int|float
RELOP       >|<|>=|<=|==|!=
OCT  0[0-7]+
HEX  0[xX][0-9a-fA-F]+

%%
\n      {yycolumn = 1;}
\r      {}
{ws}    {}
struct  {yylval.node = TerNode("STRUCT", -1, Ter, yytext); return STRUCT;}
return  {yylval.node = TerNode("RETURN", -1, Ter, yytext); return RETURN;}
if      {yylval.node = TerNode("IF", -1, Ter, yytext); return IF;}
else    {yylval.node = TerNode("ELSE", -1, Ter, yytext); return ELSE;}
while   {yylval.node = TerNode("WHILE", -1, Ter, yytext); return WHILE;}
{TYPE}  {yylval.node = TerNode("TYPE", -1, TYpe, yytext); return TYPE;}
"("     {yylval.node = TerNode("LP", -1, Ter, yytext); return LP;}
")"     {yylval.node = TerNode("RP", -1, Ter, yytext); return RP;}
"["     {yylval.node = TerNode("LB", -1, Ter, yytext); return LB;}
]       {yylval.node = TerNode("RB", -1, Ter, yytext); return RB;}
"{"     {yylval.node = TerNode("LC", -1, Ter, yytext); return LC;}
"}"     {yylval.node = TerNode("RC", -1, Ter, yytext); return RC;}
;       {yylval.node = TerNode("SEMI", -1, Ter, yytext); return SEMI;}
,       {yylval.node = TerNode("COMMA", -1, Ter, yytext); return COMMA;}
=       {yylval.node = TerNode("ASSIGNOP", -1, Ter, yytext); return ASSIGNOP;}
{RELOP} {yylval.node = TerNode("RELOP", -1, Relop, yytext); return RELOP;}
"+"     {yylval.node = TerNode("PLUS", -1, Ter, yytext); return PLUS;}
-       {yylval.node = TerNode("MINUS", -1, Ter, yytext); return MINUS;}
"*"     {yylval.node = TerNode("STAR", -1, Ter, yytext); return STAR;}
"/"     {yylval.node = TerNode("DIV", -1, Ter, yytext); return DIV;}
&&      {yylval.node = TerNode("AND", -1, Ter, yytext); return AND;}
"||"    {yylval.node = TerNode("OR", -1, Ter, yytext); return OR;}
"."     {yylval.node = TerNode("DOT", -1, Ter, yytext); return DOT;}
!       {yylval.node = TerNode("NOT", -1, Ter, yytext); return NOT;}
{INT}   {yylval.node = TerNode("INT", -1, Int, yytext); return INT;}
{OCT}   {yylval.node = TerNode("OCT", -1, Int, yytext); return INT;}
{HEX}   {yylval.node = TerNode("HEX", -1, Int, yytext); return INT;}
{FLOAT} {yylval.node = TerNode("FLOAT", -1, Float, yytext); return FLOAT;}
{ID}    {yylval.node = TerNode("ID", -1, Id, yytext); return ID;}

0[0-7]*[89]+[0-9]*                      {if(line_error[yylineno] == 0){ printf("Error type A at Line %d: Illegal OCT \'%s\'.\n", yylineno, yytext); yylval.node = TerNode("OCT", -1, Int, yytext); right = 0; line_error[yylineno]=1;}  return INT;}
0[xX][0-9a-fA-F]*[g-zG-Z]+[0-9a-zA-Z]*  {if(line_error[yylineno] == 0) {printf("Error type A at Line %d: Illegal HEX \'%s\'.\n", yylineno, yytext); yylval.node = TerNode("HEX", -1, Int, yytext); right = 0; line_error[yylineno]=1;}   return INT;}
[0-9]+\.|\.[0-9]+                      {if(line_error[yylineno] == 0) {printf("Error type A at Line %d: Illegal FLOAT \'%s\'.\n", yylineno, yytext); yylval.node = TerNode("FLOAT", -1, Float, yytext); right = 0; line_error[yylineno]=1;} return FLOAT;}
[0-9]+([0-9]|([a-zA-Z_]))+              {if(line_error[yylineno] == 0) {printf("Error type B at Line %d: Illegal ID \'%s\'.\n", yylineno, yytext); yylval.node = TerNode("ID", -1, Id, yytext); right = 0; line_error[yylineno]=1;} return ID;}


[0-9]+\.|\.[0-9]+|(([0-9]*\.?[0-9]+|[0-9]+\.)[Ee][+-]?([0-9]+|([0-9]*\.?[0-9]+|[0-9]+\.))) {if(line_error[yylineno] == 0) {printf("Error type A at Line %d: Illegal FLOAT \'%s\'.\n", yylineno, yytext); yylval.node = TerNode("FLOAT", -1, Float, yytext); right = 0; line_error[yylineno]=1;} return FLOAT;}

"//".*                 {if(line_error[yylineno] == 0) {printf("Error type B at Line %d: Mysterious character \'%s\'.\n", yylineno, yytext); right = 0; line_error[yylineno]=1;}}               
"/*"         {if(line_error[yylineno] == 0) {printf("Error type B at Line %d: Mysterious character \'%s\'.\n", yylineno, yytext); right = 0; line_error[yylineno]=1;}}
"*/"      {if(line_error[yylineno] == 0) {printf("Error type B at Line %d: Mysterious character \'%s\'.\n", yylineno, yytext); right = 0; line_error[yylineno]=1;}}


.   {if(line_error[yylineno] == 0) {printf("Error type A at Line %d: Mysterious character \'%s\'.\n", yylineno, yytext); right = 0; line_error[yylineno]=1;}}
%%



