#include "inter.h"


extern InterCodes* head;
extern InterCodes* tail;
Operand* zero;
Operand* one;
Operand* four;
extern TableList* hashTable[HASHSIZE + 1];
extern unsigned varCount, tmpCount, labelCount;
extern unsigned char interight;

void initInterCodes()
{
    head = (InterCodes*)malloc(sizeof(InterCodes));
    head->prev = head->next = NULL;
    tail = head;
    zero = (Operand*)malloc(sizeof(Operand));
    zero->kind = CONSTANT;
    zero->u.value = 0;
    one = (Operand*)malloc(sizeof(Operand));
    one->kind = CONSTANT;
    one->u.value = 1;
    four = (Operand*)malloc(sizeof(Operand));
    four->kind = CONSTANT;
    four->u.value = 4;
}


void setVariable()
{
    for (int i = 0; i < HASHSIZE + 1; ++i) 
    {
        if (!hashTable[i]) continue;
        TableList* p = hashTable[i];
        while (p) 
        {
            if (p->type->kind != WRONGFUNC && p->type->kind != FUNC && p->type->kind != STRUCTURE) 
            {
                Operand* op = (Operand*)malloc(sizeof(Operand));
                op->kind = VARIABLE;
                if (p->type->kind == ARRAY || p->type->kind == STRUCTVAR) {
                    op->type = ADDRESS;
                    p->size = countSize(p->type);
                } else op->type = VAL;
                op->u.var_no = varCount;
                p->op = op;
                ++varCount;
            }
            p = p->next;
        }
    }
}

unsigned calculate_n(Type* l, Type* r)
{
    unsigned lsize = l->array.size;
    unsigned rsize = r->array.size;
    unsigned n = lsize > rsize ? rsize : lsize;
    return n;
}

void tranProgram(Node* node)
{
    tranExtDefList(node->child);
}

void tranExtDefList(Node* node)
{
    if (node->no == 2) // ExtDef ExtDefList
    {
        tranExtDef(node->child);
        tranExtDefList(childAt(node, 1));
    }
}

void tranExtDef(Node* node)
{
    if(node->no == 3)
    {
        tranFunDec(childAt(node, 1));
        tranCompSt(childAt(node, 2));
    }
}

void tranFunDec(Node* node)
{
    Operand* func = (Operand*)malloc(sizeof(Operand));
    func->kind = FUNCT;
    func->u.func_name = node->child->val.type_str;
    createSingle(FUNCTION, func);
    if(node->no == 1)  // ID LP VarList RP
    {
        tranVarList(childAt(node, 2), 1);
    }
}

void tranCompSt(Node* node)
{
    tranDefList(childAt(node, 1));
    tranStmtList(childAt(node, 2));
}

void tranVarDec(Node* node, int isFunc, int isStruct)
{
    switch (node->no) {
    case 1: // ID
    {
        if (isFunc) {
            TableList* res = search(node->child->val.type_str);
            res->op->kind = PARAMETER;
            createSingle(PARAM, res->op);
        } else if (isStruct) {
            TableList* res = search(node->child->val.type_str);
            createDec(res->op, res->size);
        }
    } break;
    case 2: // VarDec LB INT RB
    {
        // TODO 数组
        if (node->child->no != 1) {
            printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.\n");
            interight = 0;
            exit(-1);
        }
        TableList* res = search(node->child->child->val.type_str);
        createDec(res->op, res->size);
    } break;
    default:
        break;
    }
}

void tranDefList(Node* node)
{
    if (node->no == 2) // Def DefList
    {
        tranDef(node->child);
        tranDefList(childAt(node, 1));
    }
}

void tranVarList(Node* node, int isFunc)
{
    tranParamDec(node->child, isFunc);
    if(node->no == 1) // ParamDec COMMA VarList
    {
         tranVarList(childAt(node, 2), isFunc);
    }
}

void tranParamDec(Node* node, int isFunc)
{
    // tranSpecifier(node->child);
    tranVarDec(childAt(node, 1), isFunc, 0);
}

void tranDef(Node* node)
{
    int flag = 0;
    if (node->child->no == 2)   flag = 1;
    tranDecList(childAt(node, 1), flag);
}

void tranDecList(Node* node, int isStruct)
{
    tranDec(node->child, isStruct);
    if(node->no == 2)
        tranDecList(childAt(node, 2), isStruct);
    
}

void tranDec(Node* node, int isStruct)
{
    tranVarDec(node->child, 0, isStruct);
    if(node->no == 2)
    {
        if (node->child->no == 1) // ID
        {
            TableList* res = search(node->child->child->val.type_str);
            Operand* t1 = newTemp(0);
            tranExp(childAt(node, 2), t1);
            createAssign(NORMAL, res->op, t1);
        } 
        else // VarDec LB INT RB
        {
            // TODO 数组赋值
            if (childAt(node, 2)->syn->type->kind != ARRAY) {
                interight = 0;
                exit(-1);
            }
            Operand* t1 = newTemp(1);
            tranExp(childAt(node, 2), t1);
            TableList* res = search(node->child->child->child->val.type_str);
            unsigned n = calculate_n(res->type, childAt(node, 2)->syn->type);
            if (n > 0) {
                Operand* t2 = newTemp(1);
                createAssign(GETADDR, t2, res->op);
                createAssign(COPY, t2, t1);
                for (unsigned i = 1; i < n; ++i) {
                    createBinop(ADD, NORMAL, t2, t2, four);
                    createBinop(ADD, NORMAL, t1, t1, four);
                    createAssign(COPY, t2, t1);
                }
            }
        }
    }
}

void tranStmtList(Node* node)
{
    if (node->no == 2) {
        tranStmt(node->child);
        tranStmtList(childAt(node, 1));
    }
}

void tranStmt(Node* node)
{
    switch (node->no) {
    case 1: // Exp SEMI
        tranExp(node->child, NULL);
        break;
    case 2: // CompSt
        tranCompSt(node->child);
        break;
    case 3: // RETURN Exp SEMI
    {
        Operand* t1 = newTemp(0);
        tranExp(childAt(node, 1), t1);
        createSingle(RETURN, t1);
    } break;
    case 4: // IF LP Exp RP Stmt
    {
        Operand* label1 = newLabel();
        Operand* label2 = newLabel();
        tranCond(childAt(node, 2), label1, label2);
        createSingle(LABEL, label1);
        tranStmt(childAt(node, 4));
        createSingle(LABEL, label2);
    } break;
    case 5: // IF LP Exp RP Stmt ELSE Stmt
    {
        Operand* label1 = newLabel();
        Operand* label2 = newLabel();
        Operand* label3 = newLabel();
        tranCond(childAt(node, 2), label1, label2);
        createSingle(LABEL, label1);
        tranStmt(childAt(node, 4));
        createSingle(GOTO, label3);
        createSingle(LABEL, label2);
        tranStmt(childAt(node, 6));
        createSingle(LABEL, label3);
    } break;
    case 6: // WHILE LP Exp RP Stmt
    {
        Operand* label1 = newLabel();
        Operand* label2 = newLabel();
        Operand* label3 = newLabel();
        createSingle(LABEL, label1);
        tranCond(childAt(node, 2), label2, label3);
        createSingle(LABEL, label2);
        tranStmt(childAt(node, 4));
        createSingle(GOTO, label1);
        createSingle(LABEL, label3);
    } break;
    default:
        break;
    }
}


void tranCond(Node* node, Operand* label_true, Operand* label_false)
{
    switch (node->no) {
    case 2: // Exp AND Exp
    {
        Operand* label1 = newLabel();
        tranCond(node->child, label1, label_false);
        createSingle(LABEL, label1);
        tranCond(childAt(node, 2), label_true, label_false);
    } break;
    case 3: // Exp OR Exp
    {
        Operand* label1 = newLabel();
        tranCond(node->child, label_true, label1);
        createSingle(LABEL, label1);
        tranCond(childAt(node, 2), label_true, label_false);
    } break;
    case 4: // Exp RELOP Exp
    {
        Operand* t1 = newTemp(0);
        Operand* t2 = newTemp(0);
        tranExp(node->child, t1);
        tranExp(childAt(node, 2), t2);
        createCond(t1, t2, label_true, childAt(node, 1)->val.type_str);
        createSingle(GOTO, label_false);
    } break;
    case 11: // NOT Exp
        tranCond(childAt(node, 1), label_false, label_true);
        break;
    default: {
        Operand* t1 = newTemp(0);
        tranExp(node, t1);
        createCond(t1, zero, label_true, "!=");
        createSingle(GOTO, label_false);
    } break;
    }
}


void tranExp(Node* node, Operand* place)
{
    switch (node->no) {
    case 1: // Exp ASSIGNOP Exp
    {
        if (node->child->no == 16) // ID 
        {
            TableList* res = search(node->child->child->val.type_str);
            Operand* t1 = newTemp(0);
            if (childAt(node, 2)->syn->type->kind != INT)
                t1->type = ADDRESS;
            tranExp(childAt(node, 2), t1);
            if (res->op->type == VAL)
                createAssign(NORMAL, res->op, t1);
            else {
                // 数组赋值
                if (childAt(node, 2)->syn->type->kind != ARRAY)
                {
                    interight = 0;
                    exit(-1);
                } 
                else 
                {
                    if (childAt(node, 2)->syn->type->array.elem->kind != INT) 
                    {
                        interight = 0;
                        exit(-1);
                    }
                    unsigned n = calculate_n(res->type,childAt(node, 2)->syn->type);
                    if (n > 0) 
                    {
                        Operand* t2 = newTemp(1);
                        if (res->op->kind == VARIABLE)
                            createAssign(GETADDR, t2, res->op);
                        else
                            createAssign(NORMAL, t2, res->op);
                        createAssign(COPY, t2, t1);
                        for (unsigned i = 1; i < n; ++i) {
                            createBinop(ADD, NORMAL, t2, t2, four);
                            createBinop(ADD, NORMAL, t1, t1, four);
                            createAssign(COPY, t2, t1);
                        }
                    }
                }
            }
            if (place) {
                createAssign(NORMAL, place, res->op);
            }
        } 
        else if (node->child->no == 14) //数组
        {
            Operand* t1 = newTemp(1);
            tranExp(node->child, t1);
            Operand* t2 = newTemp(0);
            if (childAt(node, 2)->syn->type->kind != INT)
                t2->type = ADDRESS;
            tranExp(childAt(node, 2), t2);
            if (t2->type == VAL)
                createAssign(SETVAL, t1, t2);
            else {
                if (childAt(node, 2)->syn->type->kind == ARRAY) {
                    printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.\n");
                    interight = 0;
                    exit(-1);
                } else {
                    interight = 0;
                    exit(-1);
                }
            }
            if (place) {
                if (place->type == VAL) {
                    createAssign(GETVAL, place, t1);
                } else {
                    createAssign(NORMAL, place, t1);
                }
            }
        } 
        else if (node->child->no == 15) //结构体中的域
        {
            Operand* t1 = newTemp(1);
            tranExp(node->child, t1);
            Operand* t2 = newTemp(0);
            if (childAt(node, 2)->syn->type->kind != INT)
                t2->type = ADDRESS;
            tranExp(childAt(node, 2), t2);
            if (t2->type == VAL)
                createAssign(SETVAL, t1, t2);
            else {
                if (childAt(node, 2)->syn->type->kind != ARRAY) {
                    interight = 0;
                    exit(-1);
                } else {
                    // 数组赋值
                    if (node->child->syn->type->array.elem->kind != INT) 
                    {
                        interight = 0;
                        exit(-1);
                    }
                    unsigned n = calculate_n(node->child->syn->type, childAt(node, 2)->syn->type);
                    if (n > 0) {
                        createAssign(COPY, t1, t2);
                        for (unsigned i = 1; i < n; ++i) {
                            createBinop(ADD, NORMAL, t1, t1, four);
                            createBinop(ADD, NORMAL, t2, t2, four);
                            createAssign(COPY, t1, t2);
                        }
                        if (n > 1 && place) {
                            Operand* con = (Operand*)malloc(sizeof(Operand));
                            con->kind = CONSTANT;
                            con->u.value = 4 * (n - 1);
                            createBinop(SUB, NORMAL, t1, t1, con);
                        }
                    }
                }
            }
            if (place) {
                if (place->type == VAL) {
                    createAssign(GETVAL, place, t1);
                } else {
                    createAssign(NORMAL, place, t1);
                }
            }
        }
    } break;
    case 2: // Exp AND Exp
    case 3: // Exp OR Exp
    case 4: // Exp RELOP Exp
    case 11: // NOT Exp
    {
        Operand* label1 = newLabel();
        Operand* label2 = newLabel();
        if (place)
            createAssign(NORMAL, place, zero);
        tranCond(node, label1, label2);
        createSingle(LABEL, label1);
        if (place)
            createAssign(NORMAL, place, one);
        createSingle(LABEL, label2);
    } break;
    case 5: // Exp PLUS Exp
    case 6: // Exp MINUS Exp
    case 7: // Exp STAR Exp
    case 8: // Exp DIV Exp
    {
        Operand* t1 = newTemp(0);
        Operand* t2 = newTemp(0);
        tranExp(node->child, t1);
        tranExp(childAt(node, 2), t2);
        if (place)
        {
            if(node->no == 8) createBinop(DIV, NORMAL, place, t1, t2);
            else if(node->no == 7) createBinop(MUL, NORMAL, place, t1, t2);
            else if(node->no == 6) createBinop(SUB, NORMAL, place, t1, t2);
            else if(node->no == 5) createBinop(ADD, NORMAL, place, t1, t2);
        }
    } break;
    case 9: // LP Exp RP
        tranExp(childAt(node, 1), place);
        break;
    case 10: // MINUS Exp
    {
        Operand* t1 = newTemp(0);
        tranExp(childAt(node, 1), t1);
        if (place)
            createBinop(SUB, NORMAL, place, zero, t1);
    } break;
    case 12: // ID LP Args RP
    {
        TableList* function = search(node->child->val.type_str);
        int argCount = function->type->function.param_num;
        Operand* arg_list[argCount];
        tranArgs(childAt(node, 2), arg_list, argCount - 1);
        if (!strcmp(function->name, "write")) {
            createSingle(WRITE, arg_list[0]);
            if (place)
                createAssign(NORMAL, place, zero);
        } else {
            for (int i = 0; i < argCount; ++i) {
                createSingle(ARG, arg_list[i]);
            }
            Operand* func = (Operand*)malloc(sizeof(Operand));
            func->kind = FUNCT;
            func->u.func_name = function->name;
            if (place)
                createSinop(CALL, place, func);
            else {
                Operand* t1 = newTemp(0);
                createSinop(CALL, t1, func);
            }
        }
    } break;
    case 13: // ID LP RP
    {
        TableList* function = search(node->child->val.type_str);
        if (!strcmp(function->name, "read")) {
            if (place)
                createSingle(READ, place);
            else {
                Operand* t1 = newTemp(0);
                createSingle(READ, t1);
            }
        } else {
            Operand* func = (Operand*)malloc(sizeof(Operand));
            func->kind = FUNCT;
            func->u.func_name = function->name;
            if (place)
                createSinop(CALL, place, func);
            else {
                Operand* t1 = newTemp(0);
                createSinop(CALL, t1, func);
            }
        }
    } break;
    case 14: // Exp LB Exp RB
    {
        if (node->child->no == 16) {
            TableList* res = search(node->child->child->val.type_str);
            Operand* t1 = newTemp(0);
            tranExp(childAt(node, 2), t1);
            unsigned elemSize = res->size / node->child->syn->type->array.size;
            Operand* esize = (Operand*)malloc(sizeof(Operand));
            esize->kind = CONSTANT;
            esize->u.value = elemSize;
            Operand* t2 = newTemp(0);
            createBinop(MUL, NORMAL, t2, t1, esize);
            if (place) {
                if (place->type == VAL) {
                    Operand* t3 = newTemp(1);
                    if (res->op->kind == VARIABLE) {
                        createBinop(ADD, GETADDR, t3, res->op, t2);
                    } else {
                        createBinop(ADD, NORMAL, t3, res->op, t2);
                    }
                    createAssign(GETVAL, place, t3);
                } else {
                    if (res->op->kind == VARIABLE)
                        createBinop(ADD, GETADDR, place, res->op, t2);
                    else
                        createBinop(ADD, NORMAL, place, res->op, t2);
                }
            }
        } else {
            if (node->child->no == 14) {
                printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.\n");
                interight = 0;
                exit(-1);
            }
            Operand* t0 = newTemp(1);
            tranExp(node->child, t0);
            Operand* t1 = newTemp(0);
            tranExp(childAt(node, 2), t1);
            TableList* res = search(node->child->syn->name);
            unsigned elemSize = res->size / node->child->syn->type->array.size;
            Operand* esize = (Operand*)malloc(sizeof(Operand));
            esize->kind = CONSTANT;
            esize->u.value = elemSize;
            Operand* t2 = newTemp(0);
            createBinop(MUL, NORMAL, t2, t1, esize);
            if (place) {
                if (node->syn->type->kind == INT) {
                    if (place->type == VAL) {
                        Operand* t3 = newTemp(1);
                        createBinop(ADD, NORMAL, t3, t0, t2);
                        createAssign(GETVAL, place, t3);
                    } else {
                        createBinop(ADD, NORMAL, place, t0, t2);
                    }
                } else {
                    createBinop(ADD, NORMAL, place, t0, t2);
                }
            }
        }
    } break;
    case 15: // Exp DOT ID
    {
        if (!place)
            break;
        if (node->child->no == 16) // ID
        {
            TableList* res = search(node->child->child->val.type_str);
            FieldList* field = res->type->structvar;
            unsigned offset = 0;
            while (strcmp(field->name, childAt(node, 2)->val.type_str)) 
            {
                TableList* tmp = search(field->name);
                offset += tmp->size;
                field = field->tail;
            }
            Operand* con = (Operand*)malloc(sizeof(Operand));
            con->kind = CONSTANT;
            con->u.value = offset;
            if (place) {
                if (place->type == VAL) {
                    Operand* t1 = newTemp(1);
                    if (res->op->kind == VARIABLE)
                        createBinop(ADD, GETADDR, t1, res->op, con);
                    else
                        createBinop(ADD, NORMAL, t1, res->op, con);
                    createAssign(GETVAL, place, t1);
                } else {
                    if (res->op->kind == VARIABLE)
                        createBinop(ADD, GETADDR, place, res->op, con);
                    else
                        createBinop(ADD, NORMAL, place, res->op, con);
                }
            }
        } else {
            Operand* t0 = newTemp(1);
            tranExp(node->child, t0);
            FieldList* field = node->child->syn->type->structvar;
            unsigned offset = 0;
            while (strcmp(field->name, childAt(node, 2)->val.type_str)) {
                TableList* tmp = search(field->name);
                offset += tmp->size;
                field = field->tail;
            }
            Operand* con = (Operand*)malloc(sizeof(Operand));
            con->kind = CONSTANT;
            con->u.value = offset;
            if (place) {
                if (node->syn->type->kind == INT) {
                    if (place->type == VAL) {
                        Operand* t1 = newTemp(1);
                        createBinop(ADD, NORMAL, t1, t0, con);
                        createAssign(GETVAL, place, t1);
                    } else {
                        createBinop(ADD, NORMAL, place, t0, con);
                    }
                } else {
                    createBinop(ADD, NORMAL, place, t0, con);
                }
            }
        }
    } break;
    case 16: // ID
    {
        if (place) {
            TableList* res = search(node->child->val.type_str);
            if (res->op->type == VAL) {
                if (place->type == VAL)
                    createAssign(NORMAL, place, res->op);
                else {
                    createAssign(SETVAL, place, res->op);
                }
            } else {
                if (res->op->kind == VARIABLE)
                    createAssign(GETADDR, place, res->op);
                else
                    createAssign(NORMAL, place, res->op);
            }
        }
    } break;
    case 17: // INT
    {
        if (place) {
            Operand* op = (Operand*)malloc(sizeof(Operand));
            op->kind = CONSTANT;
            op->u.value = node->child->val.type_int;
            createAssign(NORMAL, place, op);
        }
    } break;
    default:
        break;
    }
}


void tranArgs(Node* node, Operand* arg_list[], int index)
{
    if (node->child->syn->type->kind == ARRAY) {
        printf("Cannot translate: Code contains variables of multi-dimensional array type or parameters of array type.\n");
        interight = 0;
        exit(-1);
    }
    switch (node->no) {
    case 1: // Exp COMMA Args
    {
        tranArgs(childAt(node, 2), arg_list, index - 1);
        Operand* t1 = newTemp(0);
        if (node->child->syn->type->kind != INT)
            t1->type = ADDRESS;
        tranExp(node->child, t1);
        arg_list[index] = t1;
    } break;
    case 2: // Exp
    {
        Operand* t1 = newTemp(0);
        if (node->child->syn->type->kind != INT)
            t1->type = ADDRESS;
        tranExp(node->child, t1);
        arg_list[index] = t1;
    } break;
    default:
        break;
    }
}