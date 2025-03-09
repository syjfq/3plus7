%locations

%{
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
typedef unsigned char uint8_t;
int yylex();
int yyerror(const char* msg, ...);

typedef enum {
    Int, Float, Id, Type, Relop, Ter, Nter, Null
} MyType;

uint8_t right = 1;


typedef struct Node{
  char* name;
  MyType type;
  int line;
  union {
      unsigned type_int;
      float type_float;
      char type_str[40];
  } val;
  struct Node* child;
  struct Node* sibling;
} Node;

Node* TerNode(char* _name, MyType _type, char* yytext)
{
    Node* res = (Node*)malloc(sizeof(Node));
    res->name = _name;
    res->type = _type;
    res->child = res->sibling = NULL;
    if (_type == Int) {
        if (*res->name == 'O') {
            sscanf(yytext, "%ou", &res->val.type_int);
        } else if (*res->name == 'H') {
            sscanf(yytext, "%xu", &res->val.type_int);
        } else {
            res->val.type_int = (unsigned)atoi(yytext);
        }
    } else if (_type == Float) {
        res->val.type_float = (float)atof(yytext);
    } else if (_type == Id || _type == Type || _type == Relop || _type == Ter) {
        strcpy(res->val.type_str, yytext);
    }
    return res;
}

Node* NterNode(char* _name, MyType _type, ...)
{
    va_list list;
    Node* tmp = (Node*)malloc(sizeof(Node));
    tmp->name = _name;
    tmp->type = _type;
    tmp->child = tmp->sibling = NULL;
    va_start(list, _type);
    int num = va_arg(list, int);
    tmp->line = va_arg(list, int);
    tmp->child = va_arg(list, Node*);
    Node* t = tmp->child;
    for (int i = 1; i < num; i++) {
        t->sibling = va_arg(list, Node*);
        t = t->sibling;
    }
    va_end(list);
    return tmp;
}

Node* NullNode(char* _name)
{
    Node* res = (Node*)malloc(sizeof(Node));
    res->name = _name;
    res->type = Null;
    res->child = res->sibling = NULL;
    return res;
}

void printNodeValue(Node* node)
{
    switch (node->type)
    {
        case Nter: printf("%s (%d)\n", node->name, node->line); break;
        case Relop:
        case Ter: printf("%s\n", node->name); break;
        case Type: printf("TYPE: %s\n", node->val.type_str); break;
        case Id: printf("ID: %s\n", node->val.type_str); break;
        case Float: printf("FLOAT: %f\n", node->val.type_float); break;
        case Int: printf("INT: %u\n", node->val.type_int); break;
        case Null: break;
        default: printf("Wrong Type: %s\n", node->name); break;
    }
}

void print_node(Node* node, int dep)
{
    if (node->type != Null)
    {
        for (int i = 0; i < dep; i++)
        {
            printf("  ");
        }
        printNodeValue(node);
    }
    if (node->child)
    {
        print_node(node->child, dep + 1);
    }
    if (node->sibling)
    {
        print_node(node->sibling, dep);
    }
}

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
Program : ExtDefList {root = NterNode("Program", Nter, 1, @1.first_line, $1);}
    ;
ExtDefList :            {$$ = NullNode("ExtDefList");} //0个或多个ExtDef
    | ExtDef ExtDefList {$$ = NterNode("ExtDefList", Nter, 2, @1.first_line, $1, $2);}
    | error ExtDefList  {right = 0;}
    ;
ExtDef : Specifier ExtDecList SEMI  {$$ = NterNode("ExtDef", Nter, 3, @1.first_line, $1, $2, $3);}// 全局变量
    | Specifier SEMI                {$$ = NterNode("ExtDef", Nter, 2, @1.first_line, $1, $2);} // 结构体
    | Specifier FunDec CompSt       {$$ = NterNode("ExtDef", Nter, 3, @1.first_line, $1, $2, $3);}// 函数
    | Specifier error 			{right = 0; }
    | Specifier error CompSt 		{right = 0; }
    | Specifier FunDec error 		{right = 0; }
    | error SEMI 			{right = 0; }
    ;
ExtDecList : VarDec             {$$ = NterNode("ExtDecList", Nter, 1, @1.first_line, $1);}
    | VarDec COMMA ExtDecList   {$$ = NterNode("ExtDecList", Nter, 3, @1.first_line, $1, $2, $3);}
    | error COMMA ExtDecList    {right = 0; }
    ;

// Specifiers
Specifier : TYPE        {$$ = NterNode("Specifier", Nter, 1, @1.first_line, $1);}   // 类型描述符
    | StructSpecifier   {$$ = NterNode("Specifier", Nter, 1, @1.first_line, $1);}
    ;
StructSpecifier : STRUCT OptTag LC DefList RC   {$$ = NterNode("StructSpecifier", Nter, 5, @1.first_line, $1, $2, $3, $4, $5);} // 结构体类型
    | STRUCT Tag                                {$$ = NterNode("StructSpecifier", Nter, 2, @1.first_line, $1, $2);}
    | STRUCT error {right = 0; }
    | STRUCT error LC DefList RC  {right = 0; }
    | STRUCT OptTag LC error RC	  {right = 0; } 
    ;
OptTag :    {$$ = NullNode("OptTag");}
    | ID    {$$ = NterNode("OptTag", Nter, 1, @1.first_line, $1);}
    ;
Tag : ID    {$$ = NterNode("Tag", Nter, 1, @1.first_line, $1);}
    ;

// Declarators
VarDec : ID                 {$$ = NterNode("VarDec", Nter, 1, @1.first_line, $1);} // 变量的定义
    | VarDec LB INT RB      {$$ = NterNode("VarDec", Nter, 4, @1.first_line, $1, $2, $3, $4);}
    | VarDec LB error RB    {right = 0;}
    | error LB INT RB       {right = 0; }
    | error RB              {right = 0; }
    | VarDec LB INT error   {right = 0; }
    ;
FunDec : ID LP VarList RP   {$$ = NterNode("FunDec", Nter, 4, @1.first_line, $1, $2, $3, $4);}            // 函数头的定义
    | ID LP RP              {$$ = NterNode("FunDec", Nter, 3, @1.first_line, $1, $2, $3);}
    | ID LP error RP        {right = 0;}
    | error LP VarList RP {right = 0;}
    | error RP {right = 0;}
    | ID LP VarList error {{right = 0;}}
    ; 
VarList : ParamDec COMMA VarList    {$$ = NterNode("VarList", Nter, 3, @1.first_line, $1, $2, $3);}// 形参列表
    | ParamDec                      {$$ = NterNode("VarList", Nter, 1, @1.first_line, $1);}
    | error COMMA VarList {right = 0; }
    ;
ParamDec : Specifier VarDec {$$ = NterNode("ParamDec", Nter, 2, @1.first_line, $1, $2);}
    | Specifier error {right = 0;}
    ;

// Statements
CompSt : LC DefList StmtList RC {$$ = NterNode("CompSt", Nter, 4, @1.first_line, $1, $2, $3, $4);}// 花括号括起来的语句块
    | LC DefList error RC { right = 0; }
    ;
StmtList :          {{$$ = NullNode("StmtList");}}// 0个或多个语句
    | Stmt StmtList {$$ = NterNode("StmtList", Nter, 2, @1.first_line, $1, $2);}
    ;
Stmt : Exp SEMI                     {$$ = NterNode("Stmt", Nter, 2, @1.first_line, $1, $2);}   // 语句
    | CompSt                        {$$ = NterNode("Stmt", Nter, 1, @1.first_line, $1);}
    | RETURN Exp SEMI               {$$ = NterNode("Stmt", Nter, 3, @1.first_line, $1, $2, $3);}
    | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE             {$$ = NterNode("Stmt", Nter, 5, @1.first_line, $1, $2, $3, $4, $5);}
    | IF LP Exp RP Stmt ELSE Stmt   {$$ = NterNode("Stmt", Nter, 7, @1.first_line, $1, $2, $3, $4, $5, $6, $7);}
    | WHILE LP Exp RP Stmt          {$$ = NterNode("Stmt", Nter, 5, @1.first_line, $1, $2, $3, $4, $5);}
    | error SEMI { right = 0; }
    | RETURN error SEMI { right = 0; }
    | Exp error /* Empty Semi */  { right = 0; }
    | RETURN Exp error { right = 0; }
    | IF LP error RP Stmt %prec LOWER_THAN_ELSE { right = 0; }
    | IF LP error RP Stmt ELSE Stmt { right = 0; }
    | WHILE LP error RP Stmt { right = 0; }
    ;

// Local Definitions            局部变量
DefList :           {{$$ = NullNode("DefList");}}// 0个或多个变量定义
    | Def DefList   {$$ = NterNode("DefList", Nter, 2, @1.first_line, $1, $2);}
    ;
Def : Specifier DecList SEMI    {$$ = NterNode("Def", Nter, 3, @1.first_line, $1, $2, $3);}//一条变量定义
    | Specifier error SEMI      {right = 0;}
    ;
DecList : Dec           {$$ = NterNode("DecList", Nter, 1, @1.first_line, $1);}
    | Dec COMMA DecList {$$ = NterNode("DecList", Nter, 3, @1.first_line, $1, $2, $3);}
    | error COMMA DecList {right = 0; }
    | Dec COMMA error { right = 0; }
    ;
Dec : VarDec                {$$ = NterNode("Dec", Nter, 1, @1.first_line, $1);}
    | VarDec ASSIGNOP Exp   {$$ = NterNode("Dec", Nter, 3, @1.first_line, $1, $2, $3);} // 定义时可以初始化
    | VarDec ASSIGNOP error { right = 0; }
    ;

// Expressions
Exp : Exp ASSIGNOP Exp      {$$ = NterNode("Exp", Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp AND Exp           {$$ = NterNode("Exp", Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp OR Exp            {$$ = NterNode("Exp", Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp RELOP Exp         {$$ = NterNode("Exp", Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp PLUS Exp          {$$ = NterNode("Exp", Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp MINUS Exp         {$$ = NterNode("Exp", Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp STAR Exp          {$$ = NterNode("Exp", Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp DIV Exp           {$$ = NterNode("Exp", Nter, 3, @1.first_line, $1, $2, $3);}
    | LP Exp RP             {$$ = NterNode("Exp", Nter, 3, @1.first_line, $1, $2, $3);}
    | MINUS Exp %prec NOT   {$$ = NterNode("Exp", Nter, 2, @1.first_line, $1, $2);}
    | NOT Exp               {$$ = NterNode("Exp", Nter, 2, @1.first_line, $1, $2);}
    | ID LP Args RP         {$$ = NterNode("Exp", Nter, 4, @1.first_line, $1, $2, $3, $4);}
    | ID LP RP              {$$ = NterNode("Exp", Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp LB Exp RB         {$$ = NterNode("Exp", Nter, 4, @1.first_line, $1, $2, $3, $4);}
    | Exp DOT ID            {$$ = NterNode("Exp", Nter, 3, @1.first_line, $1, $2, $3);}
    | ID                    {$$ = NterNode("Exp", Nter, 1, @1.first_line, $1);}
    | INT                   {$$ = NterNode("Exp", Nter, 1, @1.first_line, $1);}
    | FLOAT                 {$$ = NterNode("Exp", Nter, 1, @1.first_line, $1);}
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
    
Args : Exp COMMA Args   {$$ = NterNode("Args", Nter, 3, @1.first_line, $1, $2, $3);}
    | Exp               {$$ = NterNode("Args", Nter, 1, @1.first_line, $1);}
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









