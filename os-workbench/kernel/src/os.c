#include <os.h>

static void os_init() {
  pmm->init(); 
  kmt->init();
}

#ifndef TEST
static void os_run() {
  // for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
  //   // putch(*s == '*' ? '0' + cpu_current() : *s);
  // }
  
  iset(true);
  while (1) {
    // panic("No user task!\n");
    // putch('a');
    yield();
  }
}

#else
static void os_run() {}
#endif

static IRQ* irq_head = NULL;

static void os_on_irq(int seq, int event, handler_t handler) {
  IRQ* new_handler = pmm->alloc(sizeof(IRQ));
  new_handler->seq = seq;
  new_handler->event = event;
  new_handler->handler = handler;
  new_handler->next = NULL;
  if (irq_head == NULL) {
    irq_head = new_handler;
    return;
  }
  IRQ* cur = irq_head;
  if (irq_head->seq >= seq)
  {
    new_handler->next = irq_head;
    irq_head = new_handler;
    return;
  }

  while (cur->next != NULL)
  {
    if (cur->next->seq >= seq)
    {
      new_handler->next = cur->next;
      cur->next = new_handler;
      return;
    }
    cur = cur->next;
  }

  cur->next = new_handler;
  panic_on(irq_head == NULL, "irq_head is NULL");
}

static Context *os_trap(Event ev, Context *context) {
  Context *next = NULL;
  IRQ* irq_ptr = irq_head;
  panic_on(irq_ptr == NULL, "no irq handler");
  while(irq_ptr != NULL) {
    if (irq_ptr->event == EVENT_NULL || irq_ptr->event == ev.event) {
      Context *r = irq_ptr->handler(ev, context);
      if (ev.event == EVENT_SYSCALL) {
        return context;
      }
      panic_on(r && next, "returning multiple contexts");
      if (r) {
        next = r;
      }
    }
    irq_ptr = irq_ptr->next;
  }
  panic_on(!next, "returning NULL context");
  return next;
}


MODULE_DEF(os) = {
    .init = os_init,
    .run = os_run,
    .trap = os_trap,
    .on_irq = os_on_irq
};
