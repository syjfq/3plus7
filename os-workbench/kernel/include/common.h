#include <kernel.h>
#include <klib-macros.h>
#include <klib.h>
#include <logger.h>

#define MAX_CPU 8
#define K_LOCK_NAME 128
#define STACK_SIZE 8192
#define K_TASK_NAME 128
#define MAX_TASK 32768
#define K_SEM_NAME 128
#define K_MAX_TASK 1024


// lock
typedef int lock_t;

/*

void lock_kmt(lock_t *lk)
{
  while (atomic_xchg(lk, 1) != 0)
    ;
  debug("Obtain Lock successfully\n");
}
void unlock_kmt(lock_t *lk)
{
  atomic_xchg(lk, 0);
  debug("Restore Lock successfully\n");
}
*/