#include <os.h>
#include <am.h>

#define MIN_SEQ 0
#define MAX_SEQ 10000

CPU_TASKS cpu_list[MAX_CPU];
task_t* task_list[MAX_TASK];
spinlock_t sem_init_lock;
spinlock_t task_init_lock;
int task_cnt = 1;

// kmt_spinlock
void spin_init(spinlock_t *lk, const char *name) {
    lk->locked = 0;
    lk->cpu_num = -1;
    strcpy(lk->name, name);
}

void acquire(spinlock_t *lk){
    push_off(); // disable interrupts to avoid deadlock.
    if(holding(lk))
        panic("acquire");
    //lock_kmt(&lk->locked);
    while (atomic_xchg(&lk->locked, 1) != 0)
    ;
    debug("Obtain Lock successfully\n");

    lk->cpu_num = cpu_current();
}

void release(spinlock_t *lk){
    if(!holding(lk))
        panic("release");
    lk->cpu_num = -1;
    //unlock_kmt(&lk->locked);
    atomic_xchg(&lk->locked, 0);
    debug("Restore Lock successfully\n");

    pop_off();
}

int holding(spinlock_t *lk) {
    push_off();
    int r;
    r = (lk->locked && lk->cpu_num == cpu_current());
    pop_off();
    return r;
}

void push_off() {
    int old = ienabled();
    iset(false);
    int c = cpu_current();
    if (cpu_list[c].interrupt.noff == 0) {
        cpu_list[c].interrupt.intena = old;
    }
    cpu_list[c].interrupt.noff += 1;
}

void pop_off() {
    int c = cpu_current();
    assert(ienabled() == false);
    if (cpu_list[c].interrupt.noff < 1) {
        panic("pop_off() called without push_off()\n");
    }
    cpu_list[c].interrupt.noff -= 1;
    if (cpu_list[c].interrupt.noff == 0 && cpu_list[c].interrupt.intena) {
        iset(true);
    }
}

//kmt_create, teardown
int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg) {
    acquire(&task_init_lock);

    memset(task->name, '\0', strlen(name));
    strcpy(task->name, name);
    memset(task->stack, '\0', sizeof(uint8_t) * STACK_SIZE);
    task->context[0] = kcontext((Area) {(void *) task->stack, (void *) (task->stack + STACK_SIZE)}, entry, arg);

    spin_init(&task->status, name);

    task->block = false;
    task->is_running = false;
    task->pid = task_cnt;
    task->nested_interrupt = 0;
    task_list[task_cnt++] = task;
    release(&task_init_lock);
    return 0;
}

void kmt_teardown(task_t *task) {
    acquire(&task_init_lock);
    // int pid = task->pid;
    // task_list[pid] = task_list[--task_cnt];
    release(&task_init_lock);
}

void kmt_sem_wait(sem_t *sem) {
    acquire(&sem->lock);
    sem->resource--;
    if (sem->resource < 0) {
        int cpu_id = cpu_current();
        if (cpu_list[cpu_id].current_task) {
            sem->task_list[sem->task_cnt++] = cpu_list[cpu_id].current_task;
            acquire(&cpu_list[cpu_id].current_task->status);
            cpu_list[cpu_id].current_task->block = true;
            release(&cpu_list[cpu_id].current_task->status);
        }
    }
    release(&sem->lock);
    if (sem->resource < 0) {
        yield();
    }
}

void kmt_sem_signal(sem_t *sem) {
    acquire(&sem->lock);
    sem->resource++;
    if (sem->resource <= 0 && sem->task_cnt > 0) {
        int task_id = rand() % sem->task_cnt;
        sem->task_list[task_id]->block = false;
        sem->task_list[task_id] = sem->task_list[--sem->task_cnt];
    }
    release(&sem->lock);
}

void kmt_sem_init(sem_t *sem, const char *name, int value) {
    acquire(&sem_init_lock);
    memset(sem->name, '\0', strlen(name));
    strcpy(sem->name, name);
    sem->resource = value;
    sem->task_cnt = 0;
    spin_init(&sem->lock, name);
    memset(sem->task_list, '\0', sizeof(task_t *) * K_MAX_TASK);
    release(&sem_init_lock);
}

Context* kmt_context_save(Event ev, Context *c){
    int cpu_id = cpu_current();
    task_t* cur_task = cpu_list[cpu_id].current_task;
    if (cur_task->pid > 0) {
        cur_task->context[cur_task->nested_interrupt++] = c;
    } else {
        cur_task->context[0] = c;
    }
    if (cpu_list[cpu_id].save_task && cpu_list[cpu_id].save_task != cur_task) {
        if (cpu_list[cpu_id].save_task->pid > 0) {
            acquire(&cpu_list[cpu_id].save_task->status);
            cpu_list[cpu_id].save_task->is_running = false;
            cpu_list[cpu_id].save_task->nested_interrupt --;
            assert(cpu_list[cpu_id].save_task->nested_interrupt >= 0 && cpu_list[cpu_id].save_task->nested_interrupt < 3);
            release(&cpu_list[cpu_id].save_task->status);
        }
    }
    cpu_list[cpu_id].save_task = cur_task;
    return NULL;
}

Context* kmt_schedule(Event ev, Context *c) {
    int cpu_id = cpu_current();
    
    if (cpu_list[cpu_id].current_task == NULL) {
        cpu_list[cpu_id].current_task = cpu_list[cpu_id].idle_task;
    }
    bool fine_task = false;
    for (int i = 0; i < task_cnt * 10; i++) {
        int rand_id = (rand() % (task_cnt - 1)) + 1;
        if (task_list[rand_id] == cpu_list[cpu_id].current_task) {
            if (task_list[rand_id]->block) {
                continue;
            }
            cpu_list[cpu_id].current_task = task_list[rand_id];
            cpu_list[cpu_id].current_task->nested_interrupt --;
            fine_task = true;
            panic_on(cpu_list[cpu_id].current_task->block  && fine_task, "Current task is blocked");
            break;
        }
        acquire(&task_list[rand_id]->status);
        if (task_list[rand_id]->is_running || task_list[rand_id]->block) {
            release(&task_list[rand_id]->status);
            continue;
        }
        task_list[rand_id]->is_running = true;
        release(&task_list[rand_id]->status);
        if (!task_list[rand_id]->block) {
            cpu_list[cpu_id].current_task = task_list[rand_id];
            fine_task = true;
            panic_on(cpu_list[cpu_id].current_task->block  && fine_task, "Current task is blocked");
            break;
        }
    }
    panic_on(cpu_list[cpu_id].current_task->block  && fine_task, "Current task is blocked");
    panic_on(cpu_list[cpu_id].current_task == NULL, "No task to schedule");
    if(!fine_task) {
        cpu_list[cpu_id].current_task = cpu_list[cpu_id].idle_task;
    }
    panic_on(cpu_list[cpu_id].current_task->block, "Current task is blocked");
    Context* ret = cpu_list[cpu_id].current_task->context[cpu_list[cpu_id].current_task->nested_interrupt];
    if (!fine_task) {
        ret = cpu_list[cpu_id].idle_task->context[0];
    } else {
        panic_on(ret == NULL, "No context to schedule");
    }
    return ret;
}

void idle_thread(void *arg) {
    while (1) {
        yield();
    }
}

void initialize_idle_task(task_t* idle) {
    memset(idle->name, '\0', strlen("idle"));
    strcpy(idle->name, "idle");
    memset(idle->stack, '\0', sizeof(uint8_t) * STACK_SIZE);
    idle->context[0] = kcontext((Area) {(void *) idle->stack, (void *) (idle->stack + STACK_SIZE)}, idle_thread, NULL);
    assert(idle->context);
    spin_init(&idle->status, "idle");
    idle->pid = 0;
    idle->block = false;
    idle->is_running = false;
    idle->nested_interrupt = 0;
}

void kmt_init() {
    os->on_irq(MIN_SEQ, EVENT_NULL, kmt_context_save);
    os->on_irq(MAX_SEQ, EVENT_NULL, kmt_schedule);       

    task_cnt = 1;
    spin_init(&sem_init_lock, "sem_init_lock");
    spin_init(&task_init_lock, "task_init_lock");
    for (int i = 0; i < cpu_count(); i++) {
        cpu_list[i].idle_task = pmm->alloc(sizeof(task_t));
        initialize_idle_task(cpu_list[i].idle_task);
        cpu_list[i].save_task = NULL;
        cpu_list[i].current_task = NULL;
        cpu_list[i].interrupt.noff = 0;
        cpu_list[i].interrupt.intena = 0;
    }
    memset(task_list, '\0', sizeof(task_t *) * K_MAX_TASK);
}

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = kmt_create,
    .teardown  = kmt_teardown,
    .spin_init = spin_init,
    .spin_lock = acquire,
    .spin_unlock = release,
    .sem_init = kmt_sem_init,
    .sem_wait = kmt_sem_wait,
    .sem_signal = kmt_sem_signal
};