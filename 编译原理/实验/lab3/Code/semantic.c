#include "semantic.h"
#include <assert.h>

int collision = 0;
unsigned char semright = 1;

extern TableList* hashTable[HASHSIZE + 1];

int structcmp(FieldList* s1, FieldList* s2)
{
    if(strcmp(s1->name, s2->name) == 0)
    {
    	return 1;
    }
    return 0;
}

int arraycmp(Type* t1, Type* t2)
{
    int res;
    while (t1 && t2) 
    {
        if (t1 == t2) return 1;
        else if (t1->array.elem->kind != t2->array.elem->kind) 
        {
            return 0;
        } 
        else if (t1->array.elem->kind == STRUCTURE || t1->array.elem->kind == STRUCTVAR) 
        {
            res = structcmp(t1->array.elem->structure, t2->array.elem->structure);
            if (!res) return 0;
            else return 1;
        } 
        else if (t1->array.elem->kind != ARRAY) 
        {
            return 1;
        }
        t1 = t1->array.elem;
        t2 = t2->array.elem;
    }
    if (t1 || t2)   res = 0;
    return res;
}

int funccmp(FieldList* arg1, FieldList* arg2)
{
    int res;
    while (arg1 && arg2) 
    {
        if (arg1 == arg2) return 1;
        else if (arg1->type->kind != arg2->type->kind) 
        {
            return 0;
        } 
        else if (arg1->type->kind == STRUCTVAR) 
        {
            res = structcmp(arg1->type->structvar, arg2->type->structvar);
            if (!res)   return 0;
        } 
        else if (arg1->type->kind == ARRAY) 
        {
            res = arraycmp(arg1->type, arg2->type);
            if (!res) return 0;
        } 
        else    res = 1;
        arg1 = arg1->tail;
        arg2 = arg2->tail;
    }
    if (arg1 || arg2)   res = 0;
    return res;
}

void Program(Node* node)
{
    if (!strcmp("ExtDefList", node->child->name)) {
        ExtDefList(node->child);
    }
}

void ExtDefList(Node* node)
{
    if (node->no == 2) // ExtDef ExtDefList
    {
        ExtDef(node->child);
        ExtDefList(childAt(node, 1));
    }
}

void ExtDef(Node* node)
{
    // printf("ExtDef\n");
    Type* type = (Type*)malloc(sizeof(Type));
    FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
    attr->type = type;
    attr->tail = NULL;
    switch (node->no) {
    case 1: // Specifier ExtDecList SEMI
        node->child->inh = attr;
        Specifier(node->child);
        childAt(node, 1)->inh = node->child->syn;
        ExtDecList(childAt(node, 1));
        break;
    case 2: // Specifier SEMI
        node->child->inh = attr;
        Specifier(node->child);
        break;
    case 3: // Specifier FunDec CompSt
        type->kind = FUNC;
        node->child->inh = attr;
        childAt(node, 1)->inh = attr;
        Specifier(node->child);
        if (node->child->syn->type->kind == STRUCTURE) 
        {
            Type* type = (Type*)malloc(sizeof(Type));
            type->kind = STRUCTVAR;
            type->structvar = node->child->syn->type->structure;
            node->child->inh->type->function.ret = type;
        } 
        else 
        {
            node->child->inh->type->function.ret = node->child->syn->type;
        }
        node->child->inh->type->function.param_num = 0;
        node->child->inh->type->function.args = NULL;
        FunDec(childAt(node, 1));
        childAt(node, 2)->inh = node->child->syn;
        CompSt(childAt(node, 2));
        break;
    default:
        break;
    }
}

void Specifier(Node* node)
{
    // printf("Specifier\n");
    Type* type = (Type*)malloc(sizeof(Type));
    switch (node->no) {
    case 1: // TYPE
        if (!strcmp("int", node->child->val.type_str)) 
        {
            type->kind = INT;
        } 
        else 
        {
            type->kind = FLOAT;
        }
        FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
        attr->type = type;
        attr->tail = NULL;
        node->syn = attr;
        break;
    case 2: // StructSpecifier
    {
        type->kind = STRUCTURE;
        type->structure = NULL;
        FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
        attr->type = type;
        attr->tail = NULL;
        node->child->inh = attr;
        StructSpecifier(node->child);
        node->syn = node->child->syn;
    } break;
    default:
        break;
    }
}

void ExtDecList(Node* node)
{
    // printf("ExtDecList\n");
    if (node->inh->type->kind == STRUCTURE && !node->inh->type->structure) 
    {
        printf("Error type 17 at Line %d: Undefined structure \n",node->line);
        return;
    }
    node->child->inh = node->inh;
    VarDec(node->child);
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // VarDec
        break;
    case 2: // VarDec COMMA ExtDecList
    {
        childAt(node, 2)->inh = node->inh;
        ExtDecList(childAt(node, 2));
        FieldList* p = node->syn;
        while (p->tail) p = p->tail;
        p->tail = childAt(node, 2)->syn;
    } break;
    default:
        break;
    }
}

void FunDec(Node* node)
{
    // printf("FunDec\n");
    TableList* res = search(node->child->val.type_str);
    if (!res) {
        TableList* item = (TableList*)malloc(sizeof(TableList));
        item->name = node->child->val.type_str;
        item->next = NULL;
        item->op = NULL;
        item->size = 0;
        item->type = node->inh->type;
        insert(item);
    } else {
        printf("Error type 4 at Line %d: Redefined function. \n", node->line);
        node->inh->type->kind = WRONGFUNC;
    }
    switch (node->no) 
    {
    case 1: // ID LP VarList RP
        childAt(node, 2)->inh = node->inh;
        VarList(childAt(node, 2));
        node->inh->type->function.args = childAt(node, 2)->syn;
        break;
    case 2: // ID LP RP
        node->inh->type->function.args = NULL;
        break;
    default:
        break;
    }
}

void CompSt(Node* node)  // LC DefList StmtList RC
{
    // printf("CompSt\n");
    childAt(node, 1)->inh = node->inh;
    childAt(node, 2)->inh = node->inh;
    DefList(childAt(node, 1));
    StmtList(childAt(node, 2));
}

void VarDec(Node* node)
{
    // printf("VarDec\n");
    TableList* res;
    assert(node->inh);
    switch (node->no) {
    case 1: // ID
        res = search(node->child->val.type_str);
        if (!strcmp(node->child->val.type_str, "id_jAh9_Lg")) {
            int a = 0;
        }
        assert(node->inh);
        if (!res) {
            TableList* item = (TableList*)malloc(sizeof(TableList));
            item->name = node->child->val.type_str;
            item->next = NULL;
            item->op = NULL;
            item->size = 0;
            FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
            switch (node->inh->type->kind) {
            case INT:
                item->type = node->inh->type;
                attr->name = node->child->val.type_str;
                attr->type = node->inh->type;
                attr->tail = NULL;
                node->syn = attr;
                item->size = 4;
                break;
            case FLOAT:
                item->type = node->inh->type;
                attr->name = node->child->val.type_str;
                attr->type = node->inh->type;
                attr->tail = NULL;
                node->syn = attr;
                item->size = 4;
                break;
            case ARRAY:
                item->type = node->inh->type;
                attr->name = node->child->val.type_str;
                attr->type = node->inh->type;
                attr->tail = NULL;
                node->syn = attr;
                break;
            case STRUCTVAR:
            case STRUCTURE: {
                if (!node->inh->type->structure) 
                {
                    printf("Error type 17 at Line %d: Undefined structure. \n" , node->line);
                    node->syn = NULL;
                    return;
                }
                Type* type = (Type*)malloc(sizeof(Type));
                type->kind = STRUCTVAR;
                type->structvar = node->inh->type->structure;
                attr->name = node->child->val.type_str;
                attr->type = type;
                attr->tail = NULL;
                item->type = type;
                node->syn = attr;
            } break;
            default:
                printf("name: %s, type: %d\n", node->child->val.type_str, node->inh->type->kind);
                exit(-1);
                break;
            }
            insert(item);
        } else {
            if (node->instruct) 
            {
                printf("Error type 15 at Line %d: Redefined filed. \n", node->line);
            } 
            else 
            {
                printf("Error type 3 at Line %d: Redefined variable. \n", node->line);
            }
        }
        break;
    case 2: // VarDec LB INT RB
    {
        if (node->instruct) node->child->instruct = 1;
        Type* type = (Type*)malloc(sizeof(Type));
        type->kind = ARRAY;
        type->array.size = childAt(node, 2)->val.type_int;
        if (node->inh->type->kind == STRUCTURE) 
        {
            Type* p = (Type*)malloc(sizeof(Type));
            p->kind = STRUCTVAR;
            p->structvar = node->inh->type->structure;
            type->array.elem = p;
        } 
        else 
        {
            type->array.elem = node->inh->type;
        }
        FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
        attr->tail = NULL;
        attr->type = type;
        node->child->inh = attr;
        VarDec(node->child);
        node->syn = node->child->syn;
        break;
    }
    default:
        break;
    }
}

void StructSpecifier(Node* node)
{
    // printf("StructSpecifier\n");
    assert(node->inh);
    assert(node->inh->type->kind == STRUCTURE);
    switch (node->no) {
    case 1: // STRUCT OptTag LC DefList RC
    {
        Type* type = (Type*)malloc(sizeof(Type));
        type->kind = STRUCTURE;
        FieldList* field = (FieldList*)malloc(sizeof(FieldList));
        field->tail = NULL;
        field->type = type;
        childAt(node, 1)->inh = node->inh;
        childAt(node, 3)->inh = node->inh;
        childAt(node, 3)->instruct = 1;
        OptTag(childAt(node, 1));
        DefList(childAt(node, 3));
        if (!node->inh->type->structure) 
        {
            node->inh->type->structure = childAt(node, 3)->syn;
        } 
        else 
        {
            FieldList* p = node->inh->type->structure;
            if (p->type->kind == STRUCTURE) 
            {
                p->type->structure = field;
            } 
            else 
            {
                p->tail = field;
            }
        }
        node->syn = node->inh;
    } break;
    case 2: // STRUCT Tag
        node->inh->type->structure = NULL;
        childAt(node, 1)->inh = node->inh;
        Tag(childAt(node, 1));
        node->syn = node->inh;
        break;
    default:
        break;
    }
}

void OptTag(Node* node)
{
    // printf("OptTag\n");
    assert(node->inh);
    if (node->no == 2) // ID
    {
        TableList* res = search(node->child->val.type_str);
        if (!res) 
        {
            TableList* item = (TableList*)malloc(sizeof(TableList));
            item->name = node->child->val.type_str;
            item->next = NULL;
            item->op = NULL;
            item->size = 0;
            assert(node->inh->type->kind == STRUCTURE);
            item->type = node->inh->type;
            node->inh->name = node->child->val.type_str;
            insert(item);
        } else 
        {
            printf("Error type 16 at Line %d: Duplicated name. \n", node->line);
        }
    }
}

void DefList(Node* node)
{
    assert(node->inh);
    if (node->no == 2) // Def DefList
    {
        if (node->instruct) 
        {
            node->child->instruct = 1;
            childAt(node, 1)->instruct = 1;
        }
        node->child->inh = node->inh;
        childAt(node, 1)->inh = node->inh;
        Def(node->child);
        node->syn = node->child->syn;
        DefList(childAt(node, 1));
        FieldList* p = node->syn;
        if (!p)  return;
        while (p->tail) p = p->tail;
        p->tail = childAt(node, 1)->syn;
    }
}

void Tag(Node* node)
{
    // printf("Tag\n");
    assert(node->inh);
    TableList* res = search(node->child->val.type_str);
    if (!res) 
    {
        node->inh->name = node->child->val.type_str;
    } 
    else 
    {
        node->inh->name = res->name;
        node->inh->type = res->type;
    }
}

void VarList(Node* node)
{
    // printf("VarList\n");
    ParamDec(node->child);
    if (node->child->syn == NULL)
    {
        node->inh->type->kind = WRONGFUNC;
        return;
    }
    else
    {
        node->inh->type->function.param_num++;
    }
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // ParamDec COMMA VarList
    {
        childAt(node, 2)->inh = node->inh;
        VarList(childAt(node, 2));
        FieldList* p = node->syn;
        while (p->tail) {
            p = p->tail;
        }
        p->tail = childAt(node, 2)->syn;
    } break;
    case 2: // ParamDec
        break;
    default:
        break;
    }
}

void ParamDec(Node* node)
{
    // printf("ParamDec\n");
    Specifier(node->child);
    childAt(node, 1)->inh = node->child->syn;
    VarDec(childAt(node, 1));
    node->syn = childAt(node, 1)->syn;
    if (childAt(node, 1)->syn) {
        node->syn = (FieldList*)malloc(sizeof(FieldList));
        node->syn->name = childAt(node, 1)->syn->name;
        node->syn->type = childAt(node, 1)->syn->type;
        node->syn->tail = NULL;
    } else {
        node->syn = NULL;
    }
}

void StmtList(Node* node)
{
    // printf("StmtList\n");
    if (node->no == 2) 
    {
        node->child->inh = node->inh;
        childAt(node, 1)->inh = node->inh;
        Stmt(node->child);
        StmtList(childAt(node, 1));
    }
}

void Stmt(Node* node)
{
    // printf("Stmt\n");
    switch (node->no) {
    case 1: // Exp SEMI
        Exp(node->child);
        break;
    case 2: // CompSt
        node->child->inh = node->inh;
        CompSt(node->child);
        break;
    case 3: // RETURN Exp SEMI
        Exp(childAt(node, 1));
        if (childAt(node, 1)->syn) 
        {
            if (node->inh->type->kind != childAt(node, 1)->syn->type->kind) 
            {
                if (node->inh->type->kind == STRUCTURE && childAt(node, 1)->syn->type->kind == STRUCTVAR) 
                {
                    int res = structcmp(node->inh->type->structure, childAt(node, 1)->syn->type->structvar);
                    if (res)    break;
                }
                printf("Error type 8 at Line %d: Type mismatched for return.\n" , childAt(node, 1)->line);
            }
        }
        break;
    case 4: // IF LP Exp RP Stmt
        childAt(node, 4)->inh = node->inh;
        Exp(childAt(node, 2));
        if (childAt(node, 2)->syn) 
        {
            if (childAt(node, 2)->syn->type->kind != INT) 
            {
                printf("Error type 7 at Line %d: Type mismatched for operands.\n", childAt(node, 2)->line);
            }
        }
        Stmt(childAt(node, 4));
        break;
    case 5: // IF LP Exp RP Stmt ELSE Stmt
        childAt(node, 4)->inh = node->inh;
        childAt(node, 6)->inh = node->inh;
        Exp(childAt(node, 2));
        if (childAt(node, 2)->syn) 
        {
            if (childAt(node, 2)->syn->type->kind != INT) 
            {
                printf("Error type 7 at Line %d: Type mismatched for operands.\n", childAt(node, 2)->line);
            }
        }
        Stmt(childAt(node, 4));
        Stmt(childAt(node, 6));
        break;
    case 6: // WHILE LP Exp RP Stmt
        childAt(node, 4)->inh = node->inh;
        Exp(childAt(node, 2));
        if (childAt(node, 2)->syn) 
        {
            if (childAt(node, 2)->syn->type->kind != INT) 
            {
                printf("Error type 7 at Line %d: Type mismatched for operands.\n", childAt(node, 2)->line);
            }
        }
        Stmt(childAt(node, 4));
        break;
    default:
        break;
    }
}

// 辅助函数：判断类型是否匹配
int checkTypeMatch(Type* left, Type* right, int line) {
    if (left->kind != right->kind) {
        printf("Error type 5 at Line %d: Type mismatched for assignment.\n", line);
        return 0;
    }
    return 1;
}

// 辅助函数：赋值操作左侧检查
int isLeftHandSideVariable(int id, int line) {
    if (id < 14 || id > 16) {
        printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n", line);
        return 0;
    }
    return 1;
}

void Exp(Node* node)
{
    // printf("Exp\n");
    TableList* res;
    switch (node->no) {
    case 1: // Exp ASSIGNOP Exp
    {
        int id = node->child->no;
        if (!isLeftHandSideVariable(node->child->no, node->child->line)) 
        {
            Exp(childAt(node, 2));
            return;
        } 
        Exp(node->child);
        Exp(childAt(node, 2));
        if (node->child->syn && childAt(node, 2)->syn) 
        {
            if (!checkTypeMatch(node->child->syn->type, childAt(node, 2)->syn->type, node->child->line)) {
                    return;
            }
            else if (node->child->syn->type->kind == STRUCTVAR) 
            {
                if (!structcmp(node->child->syn->type->structvar, childAt(node, 2)->syn->type->structvar)) 
                {
                    printf("Error type 5 at Line %d: Type mismatched for assignment.\n", node->child->line);
                } 
                else 
                {
                    node->syn = node->child->syn;
                }
            } 
            else if (node->child->syn->type->kind == ARRAY) 
            {
                if (!arraycmp(node->child->syn->type, childAt(node, 2)->syn->type)) 
                {
                    printf("Error type 5 at Line %d: Type mismatched for assignment.\n", node->child->line);
                } 
                else 
                {
                    node->syn = node->child->syn;
                }
            } 
            else 
            {
                node->syn = node->child->syn;
            }
        } 
        else 
        {
            printf("Error type 5 at Line %d: Type mismatched for assignment. \n",node->child->line);
        }
    } break;
    case 2: // Exp AND Exp
    case 3: // Exp OR Exp
        Exp(node->child);
        Exp(childAt(node, 2));
        if (node->child->syn && childAt(node, 2)->syn) 
        {
            if (node->child->syn->type->kind != INT || childAt(node, 2)->syn->type->kind != INT) 
            {
                printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->child->line);
            } 
            else 
            {
                Type* type = (Type*)malloc(sizeof(Type));
                type->kind = INT;
                FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
                attr->name = NULL;
                attr->type = type;
                attr->tail = NULL;
                node->syn = attr;
            }
        }
        break;
    case 4: // Exp RELOP Exp
    case 5: // Exp PLUS Exp
    case 6: // Exp MINUS Exp
    case 7: // Exp STAR Exp
    case 8: // Exp DIV Exp
        Exp(node->child);
        Exp(childAt(node, 2));
        if (node->child->syn && childAt(node, 2)->syn) 
        {
            if (node->child->syn->type->kind == INT && childAt(node, 2)->syn->type->kind == INT || node->child->syn->type->kind == FLOAT && childAt(node, 2)->syn->type->kind == FLOAT) 
            {
                Type* type = (Type*)malloc(sizeof(Type));
                type->kind = node->child->syn->type->kind;
                FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
                attr->name = NULL;
                attr->type = type;
                attr->tail = NULL;
                node->syn = attr;
            } 
            else 
            {
                printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->child->line);
            }
        } 
        else 
        {
            printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->child->line);
        }
        break;
    case 9: // LP Exp RP
        Exp(childAt(node, 1));
        node->syn = childAt(node, 1)->syn;
        break;
    case 10: // MINUS Exp
    case 11: // NOT Exp
        Exp(childAt(node, 1));
        if (childAt(node, 1)->syn) 
        {
            if (childAt(node, 1)->syn->type->kind == INT) 
            {
                node->syn = childAt(node, 1)->syn;
            } 
            else 
            {
                printf("Error type 7 at Line %d: Type mismatched for operands.\n",node->child->line);
            }
        }
        break;
    case 12: // ID LP Args RP
        res = search(node->child->val.type_str);
        Args(childAt(node, 2));
        if (!childAt(node, 2)->syn) 
        {
            node->syn = NULL;
            return;
        }
        if (res) 
        {
            if (res->type->kind != FUNC) 
            {
                printf("Error type 11 at Line %d: not a function.\n", node->line);
            } 
            else 
            {
                int tmp = funccmp(res->type->function.args, childAt(node, 2)->syn);
                if (tmp) 
                {
                    FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
                    attr->tail = NULL;
                    attr->name = NULL;
                    attr->type = res->type->function.ret;
                    node->syn = attr;
                } 
                else 
                {
                    printf("Error type 9 at Line %d: params unmatched.\n", childAt(node, 2)->line);
                }
            }
        } 
        else 
        {
            printf("Error type 2 at Line %d: Undefined function.\n", node->line);
        }
        break;
    case 13: // ID LP RP
        res = search(node->child->val.type_str);
        if (res) 
        {
            if (res->type->kind != FUNC) 
            {
                printf("Error type 11 at Line %d: not a function.\n", node->line);
            } 
            else if (res->type->function.args != NULL) 
            {
                printf("Error type 9 at Line %d: params unmatched.\n", node->line);
            } 
            else 
            {
                FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
                attr->tail = NULL;
                attr->name = NULL;
                attr->type = res->type->function.ret;
                node->syn = attr;
            }
        } 
        else 
        {
            printf("Error type 2 at Line %d: Undefined function.\n", node->line);
        }
        break;
    case 14: // Exp LB Exp RB
    {
        Exp(node->child);
        FieldList* attr = node->child->syn;
        if (!attr) return;
        if (attr->type->kind != ARRAY) 
        {
            printf("Error type 10 at Line %d: not a array.\n", node->child->line);
        } 
        else 
        {
            FieldList* p = (FieldList*)malloc(sizeof(FieldList));
            p->tail = NULL;
            p->name = attr->name;
            p->type = attr->type->array.elem;
            node->syn = p;
        }
        Exp(childAt(node, 2));
        attr = childAt(node, 2)->syn;
        if (!attr) return;
        if (attr->type->kind != INT) 
        {
            printf("Error type 12 at Line %d: Index of array is not an integer.\n", childAt(node, 2)->line);
        }
    } break;
    case 15: // Exp DOT ID
    {
        Exp(node->child);
        FieldList* attr = node->child->syn;
        if (!attr) return;
        if (attr->type->kind != STRUCTVAR) 
        {
            printf("Error type 13 at Line %d: not a struct.\n", node->child->line);
        } 
        else 
        {
            int flag = 0;
            for (FieldList* p = attr->type->structvar; p; p = p->tail) 
            {
                if (!strcmp(childAt(node, 2)->val.type_str, p->name)) 
                {
                    flag = 1;
                    node->syn = p;
                    break;
                }
            }
            if (!flag) 
            {
                printf("Error type 14 at Line %d: Non-existent field. \n", node->child->line);
            }
        }
    } break;
    case 16: // ID
        res = search(node->child->val.type_str);
        if (res) 
        {
            FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
            attr->tail = NULL;
            attr->type = res->type;
            attr->name = res->name;
            node->syn = attr;
        } 
        else 
        {
            printf("Error type 1 at Line %d: Undefined variable. \n", node->line);
        }
        break;
    case 17: // INT
    {
        Type* type = (Type*)malloc(sizeof(Type));
        type->kind = INT;
        FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
        attr->tail = NULL;
        attr->type = type;
        attr->name = NULL;
        node->syn = attr;
    } break;
    case 18: // FLOAT
    {
        Type* type = (Type*)malloc(sizeof(Type));
        type->kind = FLOAT;
        FieldList* attr = (FieldList*)malloc(sizeof(FieldList));
        attr->tail = NULL;
        attr->type = type;
        attr->name = NULL;
        node->syn = attr;
    } break;
    default:
        break;
    }
}

void Def(Node* node)
{
    // Specifier DecList SEMI
    assert(node->inh);
    if (node->instruct) 
    {
        node->child->instruct = 1;
        childAt(node, 1)->instruct = 1;
    }
    node->child->inh = node->inh;
    Specifier(node->child);
    childAt(node, 1)->inh = node->child->syn;
    DecList(childAt(node, 1));
    node->syn = childAt(node, 1)->syn;
}

void DecList(Node* node)
{
    // printf("DecList\n");
    assert(node->inh);
    if (node->instruct)
        node->child->instruct = 1;
    if (node->inh->type->kind == STRUCTURE && !node->inh->type->structure) 
    {
        printf("Error type 17 at Line %d: Undefined structure \n", node->line);
        return;
    }
    node->child->inh = node->inh;
    Dec(node->child);
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // Dec
        break;
    case 2: // Dec COMMA DecList
    {
        if (node->instruct) childAt(node, 2)->instruct = 1;
        childAt(node, 2)->inh = node->inh;
        DecList(childAt(node, 2));
        FieldList* p = node->syn;
        while (p->tail) 
        {
            p = p->tail;
        }
        p->tail = childAt(node, 2)->syn;
    } break;
    default:
        break;
    }
}

void Dec(Node* node)
{
    // printf("Dec\n");
    assert(node->inh);
    if (node->instruct)
        node->child->instruct = 1;
    node->child->inh = node->inh;
    VarDec(node->child);
    node->syn = node->child->syn;
    switch (node->no) {
    case 1: // VarDec
        break;
    case 2: // VarDec ASSIGNOP Exp
        if (node->instruct) 
        {
            printf("Error type 15 at Line %d: Can't assign value to constant.\n", node->line);
            return;
        }
        Exp(childAt(node, 2));
        if (node->child->syn && childAt(node, 2)->syn) 
        {
            if (node->child->syn->type->kind != childAt(node, 2)->syn->type->kind) 
            {
                printf("Error type 5 at Line %d: Type mismatched for assignment.\n", node->child->line);
            }
        }
        break;
    default:
        break;
    }
}

void Args(Node* node)
{
    // printf("Args\n");
    Exp(node->child);
    if (node->child->syn) {
        node->syn = (FieldList*)malloc(sizeof(FieldList));
        node->syn->name = node->child->syn->name;
        node->syn->type = node->child->syn->type;
        node->syn->tail = NULL;
    } else {
        node->syn = NULL;
    }
    switch (node->no) {
    case 1: // Exp COMMA Args
        Args(childAt(node, 2));
        if (node->syn)
            node->syn->tail = childAt(node, 2)->syn;
        break;
    case 2: // Exp
        break;
    default:
        break;
    }
}






