// When we test your kernel implementation, the framework
// directory will be replaced. Any modifications you make
// to its files (e.g., kernel.h) will be lost. 

// Note that some code requires data structure definitions 
// (such as `sem_t`) to compile, and these definitions are
// not present in kernel.h. 

// Include these definitions in os.h.
#include<common.h>

// kmt.c _ spinlock
struct spinlock {
    lock_t locked;
    char name[K_LOCK_NAME];
    int cpu_num;
};

void push_off();
void pop_off();
int holding(spinlock_t *lk);
void acquire(spinlock_t *lk);
void release(spinlock_t *lk);
void spin_init(spinlock_t *lk, const char *name);

typedef struct i_cpu_ I_CPU;
typedef struct cpu_tasks_ CPU_TASKS;
typedef struct _irq IRQ;


struct i_cpu_{
    int noff; // Depth of push_off() nesting.
    int intena; // Were interrupts enabled before push_off()
};

struct cpu_tasks_ {
    I_CPU interrupt;
    task_t* current_task;
    task_t* save_task;
    task_t* idle_task;
};

struct task {
    uint8_t stack[STACK_SIZE];
    char name[K_TASK_NAME];
    spinlock_t status;
    Context *context[2];
    int pid;
    int nested_interrupt;
    bool block;
    bool is_running;
};

struct semaphore {
    char name[K_SEM_NAME];
    volatile int resource;
    volatile int task_cnt;
    spinlock_t lock;
    volatile task_t* task_list[K_MAX_TASK];
};

struct _irq {
  int seq;
  int event;
  handler_t handler;
  IRQ* next;
};