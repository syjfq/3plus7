#include "type.h"

TableList* hashTable[HASHSIZE + 1];

unsigned hash_pjw(char* name)
{
    unsigned val = 0, i;
    for (; *name; ++name) {
        val = (val << 2) + *name;
        if (i = val & ~HASHSIZE)
            val = (val ^ (i >> 12)) & HASHSIZE;
    }
    return val;
}

void initTable()
{
    for (int i = 0; i < HASHSIZE + 1; ++i) {
        hashTable[i] = NULL;
    }
    Type* type = (Type*)malloc(sizeof(Type));
    type->kind = INT;
    Type* functype = (Type*)malloc(sizeof(Type));
    functype->kind = FUNC;
    functype->function.args = NULL;
    functype->function.ret = type;
    TableList* item = (TableList*)malloc(sizeof(TableList));
    item->name = "read";
    item->next = NULL;
    item->type = functype;
    insert(item);
    FieldList* arg = (FieldList*)malloc(sizeof(FieldList));
    arg->name = "";
    arg->tail = NULL;
    arg->type = type;
    functype = (Type*)malloc(sizeof(Type));
    functype->kind = FUNC;
    functype->function.args = arg;
    functype->function.ret = type;
    item = (TableList*)malloc(sizeof(TableList));
    item->name = "write";
    item->next = NULL;
    item->type = functype;
    insert(item);
}

TableList* search(char* name)
{
    unsigned index = hash_pjw(name);
    for (TableList* node = hashTable[index]; node; node = node->next) 
    {
        if (!strcmp(name, node->name))
            return node;
    }
    return NULL;
}

void insert(TableList* item)
{
    unsigned index = hash_pjw(item->name);
    item->next = hashTable[index];
    hashTable[index] = item;
}
