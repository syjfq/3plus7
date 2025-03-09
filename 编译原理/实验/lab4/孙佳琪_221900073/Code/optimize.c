#include "optimize.h"
#include <stdbool.h>

extern InterCodes* head;
extern unsigned varCount, tmpCount;
extern bool optimization;
BasicBlock* bbhead;
BasicBlock* bbtail;
EntryList* entryhead;



// 初始化基本块链表
void initBasicBlockList()
{
    bbhead = (BasicBlock*)malloc(sizeof(BasicBlock));
    bbhead->first = bbhead->last = NULL;
    bbhead->prev = bbhead->next = NULL;
    bbhead->predecessor = bbhead->successor = NULL;
    bbtail = bbhead;
    entryhead = (EntryList*)malloc(sizeof(EntryList));
    entryhead->entry = NULL;
    entryhead->next = NULL;
}

// 添加基本块
void addBasicBlock(BasicBlock* block)
{
    bbtail->next = block;
    block->prev = bbtail;
    bbtail = bbtail->next;
}

// 创建前驱后继链表
BasicBlocks* newBasicBlocks(BasicBlock* block)
{
    BasicBlocks* res = (BasicBlocks*)malloc(sizeof(BasicBlocks));
    res->block = block;
    res->next = NULL;
    return res;
}

// 添加流图入口
void addEntry(BasicBlock* entry)
{
    EntryList* newEntry = (EntryList*)malloc(sizeof(EntryList));
    newEntry->entry = entry;
    newEntry->exit = NULL;
    newEntry->blocknum = 0;
    newEntry->next = entryhead->next;
    entryhead->next = newEntry;
}

// 添加前驱后继关系
void addPred(BasicBlock* block, BasicBlock* pre)
{
    BasicBlocks* p = block->predecessor;
    if (!p)
        block->predecessor = newBasicBlocks(pre);
    else {
        while (p->next)
            p = p->next;
        p->next = newBasicBlocks(pre);
    }
}

void addSucc(BasicBlock* block, BasicBlock* suc)
{
    BasicBlocks* p = block->successor;
    if (!p)
        block->successor = newBasicBlocks(suc);
    else {
        while (p->next)
            p = p->next;
        p->next = newBasicBlocks(suc);
    }
}

void addPredSucc(BasicBlock* pre, BasicBlock* suc)
{
    addSucc(pre, suc);
    addPred(suc, pre);
}

// 查找label所在基本块
BasicBlock* findLabel(int no)
{
    BasicBlock* p = bbhead->next;
    while (p) {
        if (p->first->code.kind == LABEL && p->first->code.u.single.op->u.var_no == no)
            return p;
        p = p->next;
    }
}

// 构建基本块
void buildBasicBlocks()
{
    int blockid = 1;
    InterCodes* p = head->next;
    BasicBlock* newBlock = (BasicBlock*)malloc(sizeof(BasicBlock));
    newBlock->id = blockid;
    ++blockid;
    newBlock->first = p;
    newBlock->prev = newBlock->next = NULL;
    newBlock->predecessor = newBlock->successor = NULL;
    p = p->next;
    while (p->next) {
        int kind = p->code.kind;
        int prekind = p->prev->code.kind;
        if (kind == FUNCTION || kind == LABEL || prekind == RETURN || prekind == IF || prekind == GOTO) {
            newBlock->last = p->prev;
            addBasicBlock(newBlock);
            if (kind == FUNCTION)
                blockid = 1;
            newBlock = (BasicBlock*)malloc(sizeof(BasicBlock));
            newBlock->id = blockid;
            ++blockid;
            newBlock->first = p;
            newBlock->prev = newBlock->next = NULL;
            newBlock->predecessor = newBlock->successor = NULL;
        }
        p = p->next;
    }
    int kind = p->code.kind;
    if (kind == RETURN || kind == IF || kind == GOTO) {
        newBlock->last = p;
        addBasicBlock(newBlock);
    } else {
        newBlock->last = p->prev;
        addBasicBlock(newBlock);
        newBlock = (BasicBlock*)malloc(sizeof(BasicBlock));
        newBlock->id = blockid;
        ++blockid;
        newBlock->first = newBlock->last = p;
        newBlock->prev = newBlock->next = NULL;
        newBlock->predecessor = newBlock->successor = NULL;
        addBasicBlock(newBlock);
    }
    //newBlock->last = p;
    //addBasicBlock(newBlock);
}

// 构建流图
void buildFlow()
{
    BasicBlock* p = bbhead->next;
    while (p) {
        if (p->first->code.kind == FUNCTION) {
            BasicBlock* mark = p;
            BasicBlock* entry = (BasicBlock*)malloc(sizeof(BasicBlock));
            entry->id = 0;
            entry->first = entry->last = NULL;
            entry->predecessor = NULL;
            entry->successor = newBasicBlocks(p);
            p->predecessor = newBasicBlocks(entry);
            addEntry(entry);
            BasicBlock* exit = (BasicBlock*)malloc(sizeof(BasicBlock));
            entryhead->next->exit = exit;
            exit->id = -1;
            exit->first = exit->last = NULL;
            exit->predecessor = exit->successor = NULL;
            while (p) {
                entryhead->next->blocknum++;
                if (p != mark && p->first->code.kind == FUNCTION) {
                    entryhead->next->blocknum--;
                    break;
                } else if (p->last->code.kind == RETURN) {
                    addPredSucc(p, exit);
                    p = p->next;
                } else if (p->last->code.kind == IF) {
                    int label_no = p->last->code.u.cond.target->u.var_no;
                    BasicBlock* res = findLabel(label_no);
                    addPredSucc(p, p->next);
                    addPredSucc(p, res);
                    p = p->next;
                } else if (p->last->code.kind == GOTO) {
                    int label_no = p->last->code.u.single.op->u.var_no;
                    BasicBlock* res = findLabel(label_no);
                    addPredSucc(p, res);
                    p = p->next;
                } else {
                    if (p->next && p->next->first->code.kind != FUNCTION)
                        addPredSucc(p, p->next);
                    p = p->next;
                }
            }
        } else {
            //printf("Block id: %d, Code kind: %d\n", p->id, p->first->code.kind);
            //printf("NOT a Function?!\n");
            exit(-1);
        }
    }
}




void optimize()
{
    optimization = true;
    initBasicBlockList();
    buildBasicBlocks();
    buildFlow();
    constProp();
    liveVariables();
}