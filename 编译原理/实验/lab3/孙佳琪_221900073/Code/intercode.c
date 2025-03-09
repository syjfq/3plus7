#include "intercode.h"


extern TableList* hashTable[HASHSIZE + 1];
InterCodes* head;
InterCodes* tail;
unsigned varCount = 1, tmpCount = 1, labelCount = 1;
unsigned char interight = 1;

unsigned countSize(Type* type)
{
    if (type->kind == ARRAY) 
    {
        return type->array.size * countSize(type->array.elem);
    } 
    else if (type->kind == STRUCTVAR) 
    {
        FieldList* field = type->structvar;
        unsigned size = 0;
        while (field) {
            size += countSize(field->type);
            field = field->tail;
        }
        return size;
    } 
    else if (type->kind == INT || type->kind == FLOAT) return 4;
}

Operand* newConst(long long int val)
{
    Operand* op = (Operand*)malloc(sizeof(Operand));
    op->kind = CONSTANT;
    op->u.value = val;
    return op;
}

Operand* newTemp(int addr) // 返回一个立即数，0表示VAL，1表示是地址
{
    Operand* op = (Operand*)malloc(sizeof(Operand));
    op->kind = TEMP;
    if (addr)
        op->type = ADDRESS;
    else
        op->type = VAL;
    op->u.var_no = tmpCount;
    ++tmpCount;
    return op;
}

Operand* newLabel() //返回一个标签label
{
    Operand* op = (Operand*)malloc(sizeof(Operand));
    op->kind = LAB;
    op->u.var_no = labelCount;
    ++labelCount;
    return op;
}

void insert_to_tail(InterCodes* p)
{
    p->next = NULL;
    p->prev = tail;
    tail->next = p;
    tail = tail->next;
}

void createAssign(unsigned type, Operand* left, Operand* right) //创建赋值语句
{
    InterCodes* p = (InterCodes*)malloc(sizeof(InterCodes));
    p->code.kind = ASSIGN;
    p->code.type = type;
    p->code.u.assign.left = left;
    p->code.u.assign.right = right;
    insert_to_tail(p);
}

void createBinop(unsigned kind, unsigned type, Operand* res, Operand* op1, Operand* op2) //创建二元运算语句
{
    InterCodes* p = (InterCodes*)malloc(sizeof(InterCodes));
    p->code.kind = kind;
    p->code.type = type;
    p->code.u.binop.res = res;
    p->code.u.binop.op1 = op1;
    p->code.u.binop.op2 = op2;
    insert_to_tail(p);
}

void createSinop(unsigned kind, Operand* res, Operand* op) //创建一元运算语句
{
    InterCodes* p = (InterCodes*)malloc(sizeof(InterCodes));
    p->code.kind = kind;
    p->code.u.sinop.res = res;
    p->code.u.sinop.op = op;
    insert_to_tail(p);
}

void createSingle(unsigned kind, Operand* op) //创建单目运算语句
{
    InterCodes* p = (InterCodes*)malloc(sizeof(InterCodes));
    p->code.kind = kind;
    p->code.u.single.op = op;
    insert_to_tail(p);
}

void createCond(Operand* op1, Operand* op2, Operand* target, char* re) //创建条件语句
{
    InterCodes* p = (InterCodes*)malloc(sizeof(InterCodes));
    p->code.kind = IF;
    p->code.u.cond.op1 = op1;
    p->code.u.cond.op2 = op2;
    p->code.u.cond.target = target;
    strcpy(p->code.u.cond.relop, re);
    insert_to_tail(p);
}

void createDec(Operand* op, unsigned size) //创建变量声明语句
{
    InterCodes* p = (InterCodes*)malloc(sizeof(InterCodes));
    p->code.kind = DEC;
    p->code.u.dec.op = op;
    p->code.u.dec.size = size;
    insert_to_tail(p);
}

char* printOperand(Operand* op)
{
    char* res = (char*)malloc(40);
    switch (op->kind) {
    case VARIABLE:
        sprintf(res, "v%d", op->u.var_no);
        break;
    case TEMP:
        sprintf(res, "t%d", op->u.var_no);
        break;
    case PARAMETER:
        sprintf(res, "v%d", op->u.var_no);
        break;
    case CONSTANT:
        sprintf(res, "#%lld", op->u.value);
        break;
    case LAB:
        sprintf(res, "label%d", op->u.var_no);
        break;
    case FUNCT:
        sprintf(res, "%s", op->u.func_name);
        break;
    default:
        break;
    }
    return res;
}

void writeInterCodes(const char* fielname, bool opt)
{
    InterCodes* p = head->next;
    FILE* f = fopen(fielname, "w");
    while (p) {
        if (!p->isDelete || !opt)
            switch (p->code.kind) {
            case LABEL:
                fprintf(f, "LABEL %s :\n", printOperand(p->code.u.single.op));
                break;
            case FUNCTION:
                fprintf(f, "FUNCTION %s :\n", printOperand(p->code.u.single.op));
                break;
            case ASSIGN: {
                char* l = printOperand(p->code.u.assign.left);
                char* r = printOperand(p->code.u.assign.right);
                switch (p->code.type) {
                case NORMAL:
                    fprintf(f, "%s := %s\n", l, r);
                    break;
                case GETADDR:
                    fprintf(f, "%s := &%s\n", l, r);
                    break;
                case GETVAL:
                    fprintf(f, "%s := *%s\n", l, r);
                    break;
                case SETVAL:
                    fprintf(f, "*%s := %s\n", l, r);
                    break;
                case COPY:
                    fprintf(f, "*%s := *%s\n", l, r);
                    break;
                default:
                    break;
                }
            } break;
            case ADD: {
                char* r = printOperand(p->code.u.binop.res);
                char* op1 = printOperand(p->code.u.binop.op1);
                char* op2 = printOperand(p->code.u.binop.op2);
                if (p->code.type == NORMAL)
                    fprintf(f, "%s := %s + %s\n", r, op1, op2);
                else if (p->code.type == GETADDR)
                    fprintf(f, "%s := &%s + %s\n", r, op1, op2);
            } break;
            case SUB: {
                char* r = printOperand(p->code.u.binop.res);
                char* op1 = printOperand(p->code.u.binop.op1);
                char* op2 = printOperand(p->code.u.binop.op2);
                fprintf(f, "%s := %s - %s\n", r, op1, op2);
            } break;
            case MUL: {
                char* r = printOperand(p->code.u.binop.res);
                char* op1 = printOperand(p->code.u.binop.op1);
                char* op2 = printOperand(p->code.u.binop.op2);
                fprintf(f, "%s := %s * %s\n", r, op1, op2);
            } break;
            case DIV: {
                char* r = printOperand(p->code.u.binop.res);
                char* op1 = printOperand(p->code.u.binop.op1);
                char* op2 = printOperand(p->code.u.binop.op2);
                fprintf(f, "%s := %s / %s\n", r, op1, op2);
            } break;
            case GOTO:
                fprintf(f, "GOTO %s\n", printOperand(p->code.u.single.op));
                break;
            case IF: {
                char* op1 = printOperand(p->code.u.cond.op1);
                char* op2 = printOperand(p->code.u.cond.op2);
                char* t = printOperand(p->code.u.cond.target);
                fprintf(f, "IF %s %s %s GOTO %s\n", op1, p->code.u.cond.relop, op2, t);
            } break;
            case RETURN:
                fprintf(f, "RETURN %s\n", printOperand(p->code.u.single.op));
                break;
            case DEC:
                fprintf(f, "DEC %s %u\n", printOperand(p->code.u.dec.op), p->code.u.dec.size);
                break;
            case ARG:
                fprintf(f, "ARG %s\n", printOperand(p->code.u.single.op));
                break;
            case CALL: {
                char* res = printOperand(p->code.u.sinop.res);
                char* op = printOperand(p->code.u.sinop.op);
                fprintf(f, "%s := CALL %s\n", res, op);
            } break;
            case PARAM:
                fprintf(f, "PARAM %s\n", printOperand(p->code.u.single.op));
                break;
            case READ:
                fprintf(f, "READ %s\n", printOperand(p->code.u.single.op));
                break;
            case WRITE:
                if (p->code.u.single.op->type == VAL || p->code.u.single.op->kind == CONSTANT)
                    fprintf(f, "WRITE %s\n", printOperand(p->code.u.single.op));
                else
                    fprintf(f, "WRITE *%s\n", printOperand(p->code.u.single.op));
                break;
            default:
                break;
            }
        p = p->next;
    }
    fclose(f);
}