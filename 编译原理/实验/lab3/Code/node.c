#include "node.h"


Node* TerNode(char* _name, int _no, MyType _type, char* yytext)
{
    Node* res = (Node*)malloc(sizeof(Node));
    res->name = _name;
    res->type = _type;
    res->no = _no;
    res->syn = res->inh = NULL;
    res->parent = res->child = res->next = NULL;
    res->instruct = 0;
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
    } else if (_type == Id || _type == TYpe || _type == Relop || _type == Ter) {
        strcpy(res->val.type_str, yytext);
    }
    return res;
}

Node* NterNode(char *_name, int _no, MyType _type, ...)
{
    va_list list;
    Node* res = (Node*)malloc(sizeof(Node));
    res->name = _name;
    res->type = _type;
    res->no = _no;
    res->syn = res->inh = NULL;
    res->parent = res->child = res->next = NULL;
    res->instruct = 0;
    va_start(list, _type);
    int num = va_arg(list, int);
    res->line = va_arg(list, int);
    res->child = va_arg(list, Node*);
    res->child->parent = res;
    Node* t = res->child;
    for (int i = 0; i < num - 1; i++) {
        t->next = va_arg(list, Node*);
        t = t->next;
    }
    va_end(list);
    return res;
}

Node* NullNode(char* _name, int _no)
{
    Node *res = (Node *)malloc(sizeof(Node));
    res->name = _name;
    res->type = Null;
    res->no = _no;
    res->parent = res->child = res->next = NULL;
    res->instruct = 0;
    return res;
}

void outPut(Node *node, int dep)
{
    if (node->type != Null)
    {
        for (int i = 0; i < dep; i++)
        {
            printf("  ");
        }
    }
    Node *p = node;
    switch (p->type)
    {
    case Nter:
        printf("%s (%d)\n", p->name, p->line);
        break;
    case Relop:
    case Ter:
        printf("%s\n", p->name);
        break;
    case TYpe:
        printf("TYPE: %s\n", p->val.type_str);
        break;
    case Id:
        printf("ID: %s\n", p->val.type_str);
        break;
    case Float:
        printf("FLOAT: %f\n", p->val.type_float);
        break;
    case Int:
        printf("INT: %u\n", p->val.type_int);
        break;
    case Null:
        break;
    default:
        printf("Wrong Type: %s\n", p->name);
        break;
    }
    if (p->child)
    {
        outPut((Node *)p->child, dep + 1);
    }
    if (p->next)
    {
        outPut((Node *)p->next, dep);
    }
}

Node *childAt(Node *node, int index)
{
    Node *p = node->child;
    for (int i = 0; i < index; ++i)
    {
        p = p->next;
    }
    return p;
}


