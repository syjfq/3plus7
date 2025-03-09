#include "intercode.h"
#include <stdbool.h>



void initInterCodes();
void setVariable();


void tranCond(Node* node, Operand* label_true, Operand* label_false);
void tranExp(Node* node, Operand* place);
void tranArgs(Node* node, Operand* arg_list[], int index);

void tranProgram(Node* node);
void tranExtDefList(Node* node);
void tranExtDef(Node* node);
void tranFunDec(Node* node);
void tranCompSt(Node* node);
void tranVarDec(Node* node, int isFunc, int isStruct);
void tranDefList(Node* node);
void tranVarList(Node* node, int isFunc);
void tranParamDec(Node* node, int isFunc);
void tranStmtList(Node* node);
void tranStmt(Node* node);
void tranDef(Node* node);
void tranDecList(Node* node, int isStruct);
void tranDec(Node* node, int isStruct);