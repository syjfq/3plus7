#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>

enum co_status {
    CO_NEW = 1, // 新创建，还未执行过
    CO_RUNNING, // 已经执行过
    CO_WAITING, // 在 co_wait 上等待
    CO_DEAD,    // 已经结束，但还未释放资源
};

#define K 1024
#define STACK_SIZE (64 * K)

struct co {
    const char *name;
    void (*func)(void *); // co_start 指定的入口地址和参数
    void *arg;

    enum co_status status;  // 协程的状态
    struct co *    waiter;  // 是否有其他协程在等待当前协程
    jmp_buf        context; // 寄存器现场
    uint8_t  stack[STACK_SIZE]; // 协程的堆栈
};

typedef struct CoNode{
    struct co *coroutine;
    struct CoNode *fd;
} CoNode;

struct co* current;
static CoNode *co_node = NULL;

static void co_node_insert(struct co *coroutine){
    CoNode *victim = (CoNode*)malloc(sizeof(CoNode));
    victim->coroutine = coroutine;
    if (co_node == NULL){
        victim->fd = victim;
        co_node = victim;
    }
    else{
        victim->fd = co_node->fd;
        co_node->fd = victim;
    }
}

static inline void stack_switch_call(void *sp, void *entry, void* arg) {
	asm volatile (
#if __x86_64__
			"movq %%rcx, 0(%0); movq %0, %%rsp; movq %2, %%rdi; call *%1"
			: : "b"((uintptr_t)sp - 16), "d"((uintptr_t)entry), "a"((uintptr_t)arg)
#else
			"movl %%ecx, 4(%0); movl %0, %%esp; movl %2, 0(%0); call *%1"
			: : "b"((uintptr_t)sp - 8), "d"((uintptr_t)entry), "a"((uintptr_t)arg) 
#endif
			);
}

static inline void restore_return() {
	asm volatile (
#if __x86_64__
			"movq 0(%%rsp), %%rcx" : : 
#else
			"movl 4(%%esp), %%ecx" : :  
#endif
			);
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
    struct co *coroutine = (struct co*)malloc(sizeof(struct co));
    coroutine->name = name;
    coroutine->func = func;
    coroutine->arg = arg;
    coroutine->status = CO_NEW;
    coroutine->waiter = NULL;
    co_node_insert(coroutine);
    return coroutine;
}

void co_wait(struct co *co) {
    if (co->status != CO_DEAD)
    {
        co->waiter = current;
        current->status = CO_WAITING;
        co_yield();
    }
    while(co_node->fd->coroutine != co){
        co_node = co_node->fd;
    }
    co_node->fd = co_node->fd->fd;
    free(co);
}

void co_yield() {
    int val = setjmp(current->context);
    if (val == 0) {
        do {
            co_node = co_node->fd;
        } while(co_node->coroutine->status == CO_DEAD || co_node->coroutine->status == CO_WAITING);
        current = co_node->coroutine;
        if (current->status == CO_RUNNING)
        {
            longjmp(current->context, 1);
        }
        else{
            ((struct co volatile*)current)->status = CO_RUNNING;
            stack_switch_call(current->stack + STACK_SIZE, current->func, current->arg);
			restore_return();
            current->status = CO_DEAD;
            if(current->waiter != NULL){
                current->waiter->status = CO_RUNNING;
            }
            co_yield();
        }
    }
}

static __attribute__((constructor)) void co_constructor() {

	current = co_start("main", NULL, NULL);
	current->status = CO_RUNNING;
}