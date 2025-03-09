#include "node.h"
#define HASHSIZE 0x3fff

typedef struct Operand_ Operand;

struct Type_ {
    enum { INT, FLOAT, ARRAY, STRUCTURE, STRUCTVAR, FUNC, WRONGFUNC } kind;
    //基本类型
    int basic;
    //数组：元素类型，数组大小
    struct
    {
        Type* elem;
        int size;
    } array;
    //结构体：链表
    FieldList* structure;
    FieldList* structvar;
    //函数：参数链表，返回类型
    struct
    {
        Type* ret;
        int param_num;
        FieldList* args;
    } function;
    ;
};

struct FieldList_ {
    char* name; //域的名字
    Type* type; //域的类型
    FieldList* tail; //下一个域
};

struct TableList_ {
    char* name;
    Type* type;
    unsigned size;
    Operand* op;
    TableList* next;
};

unsigned hash_pjw(char *name);
void initTable();
TableList* search(char* name);
void insert(TableList* item);