%locations

%{

#include "node.h"

typedef unsigned char uint8_t;
int yylex();
int yyerror(const char* msg, ...);

uint8_t right = 1;
int ignore_line = 0;
int syserr = 0;
int myerr = 0;
int line_error[1024]={0};

Node* root;
%}

%union {
    Node* node;
}


%token <node> INT FLOAT ID TYPE 
%token <node> SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT
%token <node> LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE

%type <node> Program ExtDefList ExtDef Specifier ExtDecList FunDec CompSt VarDec StructSpecifier OptTag DefList Tag VarList ParamDec StmtList Exp Stmt Def DecList Dec Args


%right ASSIGNOP    // 确保表达式 a = b = c 的正确解析，b = c 会先被计算，然后结果赋值给 a
%left OR           
%left AND          
%nonassoc RELOP    // 防止像 a < b < c 这样的表达式产生二义性
%left PLUS MINUS   
%left STAR DIV     
%nonassoc NOT 
%left DOT                
%left LP RP      
%left LB RB        
%left LC RC

%nonassoc LOWER_THAN_ELSE // 处理悬空 else 的优先级
%nonassoc ELSE


%%
Program : ExtDefList {root = NterNode("Program", 1, Nter, 1, @1.first_line, $1);}
    ;
ExtDefList :            {$$ = NullNode("ExtDefList", 1);} //0个或多个ExtDef
    | ExtDef ExtDefList {$$ = NterNode("ExtDefList", 2, Nter, 2, @1.first_line, $1, $2);}
    | error ExtDefList  {right = 0;}
    ;
ExtDef : Specifier ExtDecList SEMI  {$$ = NterNode("ExtDef", 1, Nter, 3, @1.first_line, $1, $2, $3);}// 全局变量
    | Specifier SEMI                {$$ = NterNode("ExtDef", 2, Nter, 2, @1.first_line, $1, $2);} // 结构体
    | Specifier FunDec CompSt       {$$ = NterNode("ExtDef", 3, Nter, 3, @1.first_line, $1, $2, $3);}// 函数
    | Specifier error 			    {right = 0; }
    | Specifier error CompSt 		{right = 0; }
    | Specifier FunDec error 		{right = 0; }
    | error SEMI 			        {right = 0; }
    ;
ExtDecList : VarDec             {$$ = NterNode("ExtDecList", 1, Nter, 1, @1.first_line, $1);}
    | VarDec COMMA ExtDecList   {$$ = NterNode("ExtDecList", 2, Nter, 3, @1.first_line, $1, $2, $3);}
    | error COMMA ExtDecList    {right = 0; }
    ;

// Specifiers
Specifier : TYPE        {$$ = NterNode("Specifier", 1, Nter, 1, @1.first_line, $1);}   // 类型描述符
    | StructSpecifier   {$$ = NterNode("Specifier", 2, Nter, 1, @1.first_line, $1);}
    ;
StructSpecifier : STRUCT OptTag LC DefList RC   {$$ = NterNode("StructSpecifier", 1, Nter, 5, @1.first_line, $1, $2, $3, $4, $5);} // 结构体类型
    | STRUCT Tag                                {$$ = NterNode("StructSpecifier", 2, Nter, 2, @1.first_line, $1, $2);}
    | STRUCT error                              {right = 0; }
    | STRUCT error LC DefList RC                {right = 0; }
    | STRUCT OptTag LC error RC	                {right = 0; } 
    ;
OptTag :    {$$ = NullNode("OptTag", 1);}
    | ID    {$$ = NterNode("OptTag", 2, Nter, 1, @1.first_line, $1);}
    ;
Tag : ID    {$$ = NterNode("Tag", 1, Nter, 1, @1.first_line, $1);}
    ;

// Declarators
VarDec : ID                 {$$ = NterNode("VarDec", 1, Nter, 1, @1.first_line, $1);} // 变量的定义
    | VarDec LB INT RB      {$$ = NterNode("VarDec", 2, Nter, 4, @1.first_line, $1, $2, $3, $4);}
    | VarDec LB error RB    {right = 0;}
    | error LB INT RB       {right = 0;}
    | error RB              {right = 0;}
    | VarDec LB INT error   {right = 0;}
    ;
FunDec : ID LP VarList RP   {$$ = NterNode("FunDec", 1, Nter, 4, @1.first_line, $1, $2, $3, $4);}            // 函数头的定义
    | ID LP RP              {$$ = NterNode("FunDec", 2, Nter, 3, @1.first_line, $1, $2, $3);}
    | ID LP error RP        {right = 0;}
    | error LP VarList RP   {right = 0;}
    | error RP              {right = 0;}
    | ID LP VarList error   {{right = 0;}}
    ; 
VarList : ParamDec COMMA VarList    {$$ = NterNode("VarList", 1, Nter, 3, @1.first_line, $1, $2, $3);}// 形参列表
    | ParamDec                      {$$ = NterNode("VarList", 2, Nter, 1, @1.first_line, $1);}
    | error COMMA VarList           {right = 0; }
    ;
ParamDec : Specifier VarDec {$$ = NterNode("ParamDec", 1, Nter, 2, @1.first_line, $1, $2);}
    | Specifier error {right = 0;}
    ;

// Statements
CompSt : LC DefList StmtList RC {$$ = NterNode("CompSt", 1, Nter, 4, @1.first_line, $1, $2, $3, $4);}// 花括号括起来的语句块
    | LC DefList error RC { right = 0; }
    ;
StmtList :          {{$$ = NullNode("StmtList", 1);}}// 0个或多个语句
    | Stmt StmtList {$$ = NterNode("StmtList", 2, Nter, 2, @1.first_line, $1, $2);}
    ;
Stmt : Exp SEMI                     {$$ = NterNode("Stmt", 1, Nter, 2, @1.first_line, $1, $2);}   // 语句
    | CompSt                        {$$ = NterNode("Stmt", 2, Nter, 1, @1.first_line, $1);}
    | RETURN Exp SEMI               {$$ = NterNode("Stmt", 3, Nter, 3, @1.first_line, $1, $2, $3);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE             {$$ = NterNode("Stmt", 4, Nter, 5, @1.first_line, $1, $2, $3, $4, $5);}
    | IF LP Exp RP Stmt ELSE Stmt   {$$ = NterNode("Stmt", 5, Nter, 7, @1.first_line, $1, $2, $3, $4, $5, $6, $7);}
    | WHILE LP Exp RP Stmt          {$$ = NterNode("Stmt", 6, Nter, 5, @1.first_line, $1, $2, $3, $4, $5);}
    | error SEMI { right = 0; }
    | RETURN error SEMI { right = 0; }
    | Exp error /* Empty Semi */  { right = 0; }
    | RETURN Exp error { right = 0; }
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE { right = 0; }
    | IF LP error RP Stmt ELSE Stmt { right = 0; }
    | WHILE LP error RP Stmt { right = 0; }
    ;

// Local Definitions            局部变量
DefList :           {{$$ = NullNode("DefList", 1);}}// 0个或多个变量定义
    | Def DefList   {$$ = NterNode("DefList", 2, Nter, 2, @1.first_line, $1, $2);}
    ;
Def : Specifier DecList SEMI    {$$ = NterNode("Def", 1, Nter, 3, @1.first_line, $1, $2, $3);}//一条变量定义
    | Specifier error SEMI      {right = 0;}
    ;
DecList : Dec           {$$ = NterNode("DecList", 1, Nter, 1, @1.first_line, $1);}
    | Dec COMMA DecList {$$ = NterNode("DecList", 2, Nter, 3, @1.first_line, $1, $2, $3);}
    | error COMMA DecList {right = 0; }
    | Dec COMMA error { right = 0; }
    ;
Dec : VarDec                {$$ = NterNode("Dec", 1, Nter, 1, @1.first_line, $1);}
    | VarDec ASSIGNOP Exp   {$$ = NterNode("Dec", 2, Nter, 3, @1.first_line, $1, $2, $3);} // 定义时可以初始化
    | VarDec ASSIGNOP error { right = 0; }
    ;

// Expressions
Exp : Exp ASSIGNOP Exp      {$$ = NterNode("Exp", 1, Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp AND Exp           {$$ = NterNode("Exp", 2, Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp OR Exp            {$$ = NterNode("Exp", 3, Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp RELOP Exp         {$$ = NterNode("Exp", 4, Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp PLUS Exp          {$$ = NterNode("Exp", 5, Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp MINUS Exp         {$$ = NterNode("Exp", 6, Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp STAR Exp          {$$ = NterNode("Exp", 7, Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp DIV Exp           {$$ = NterNode("Exp", 8, Nter, 3, @1.first_line, $1, $2, $3);}
    | LP Exp RP             {$$ = NterNode("Exp", 9, Nter, 3, @1.first_line, $1, $2, $3);}
    | MINUS Exp %prec NOT   {$$ = NterNode("Exp", 10, Nter, 2, @1.first_line, $1, $2);}
    | NOT Exp               {$$ = NterNode("Exp", 11, Nter, 2, @1.first_line, $1, $2);}
    | ID LP Args RP         {$$ = NterNode("Exp", 12, Nter, 4, @1.first_line, $1, $2, $3, $4);}
    | ID LP RP              {$$ = NterNode("Exp", 13, Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp LB Exp RB         {$$ = NterNode("Exp", 14, Nter, 4, @1.first_line, $1, $2, $3, $4);}
    | Exp DOT ID            {$$ = NterNode("Exp", 15, Nter, 3, @1.first_line, $1, $2, $3);}
    | ID                    {$$ = NterNode("Exp", 16, Nter, 1, @1.first_line, $1);}
    | INT                   {$$ = NterNode("Exp", 17, Nter, 1, @1.first_line, $1);}
    | FLOAT                 {$$ = NterNode("Exp", 18, Nter, 1, @1.first_line, $1);}
    | Exp ASSIGNOP error    {right = 0;}
    | Exp AND error         {right = 0;}
    | Exp OR error          {right = 0;}
    | Exp RELOP error       {right = 0;}
    | Exp PLUS error        {right = 0;}
    | Exp MINUS error       {right = 0;}
    | Exp STAR error        {right = 0;}
    | Exp DIV error         {right = 0;}
    | LP error RP           {right = 0;}
    | MINUS error           {right = 0;}
    | NOT error             {right = 0;}
    | ID error              {right = 0;}
    | Exp LB error RB       {right = 0;}
    ;
    
Args : Exp COMMA Args   {$$ = NterNode("Args", 1, Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp               {$$ = NterNode("Args", 2, Nter, 1, @1.first_line, $1);}
    ;


%%
#include "lex.yy.c"

int yyerror(const char *msg, ...)
{
    if(line_error[yylineno] == 0)
    {
        printf("Error type B at Line %d: %s.\n", yylineno, msg);
        line_error[yylineno] = 1;
    }
    return 0;
}









