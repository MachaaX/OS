// thread-worker.c

#include "thread-worker.h"
#include "thread_worker_types.h"

#define STACK_SIZE 16 * 1024
#define QUANTUM 10 * 1000

// INITIALIZE ALL YOUR OTHER VARIABLES HERE
int init_scheduler_done = 0;
static runQueue *ready_queue = NULL; // Queue for ready threads
static tcb main_thread_tcb;          // TCB for the main thread
static ucontext_t scheduler_context; // Context for the scheduler

// ... Other function prototypes ...

// Function to initialize the scheduler
static void init_scheduler()
{
  // Initialize the main thread's TCB
  main_thread_tcb.thread_id = 0; // Assign a unique ID to the main thread
  main_thread_tcb.thread_status = RUNNING;
  main_thread_tcb.thread_stack = NULL; // The main thread shares the stack with the benchmark thread
  main_thread_tcb.stack_size = 0;
  main_thread_tcb.priority = 0;
  main_thread_tcb.return_value = NULL;

  // Create the scheduler context
  if (getcontext(&scheduler_context) < 0)
  {
    perror("getcontext");
    exit(1);
  }

  // Allocate space for the scheduler stack
  void *scheduler_stack = malloc(STACK_SIZE);
  if (scheduler_stack == NULL)
  {
    perror("Failed to allocate scheduler stack");
    exit(1);
  }

  scheduler_context.uc_stack.ss_sp = scheduler_stack;
  scheduler_context.uc_stack.ss_size = STACK_SIZE;
  scheduler_context.uc_link = NULL;
  scheduler_context.uc_stack.ss_flags = 0;
  makecontext(&scheduler_context, schedule, 0);
}

// Function to set up the timer
static void setup_timer()
{
  // Use sigaction to register signal handler
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = &timer_handler;
  sigaction(SIGPROF, &sa, NULL);

  // Create timer struct
  struct itimerval timer;

  // Set up what the timer should reset to after the timer goes off
  timer.it_interval.tv_usec = QUANTUM % 1000000;
  timer.it_interval.tv_sec = QUANTUM / 1000000;

  // Set up the current timer to go off in QUANTUM microseconds
  timer.it_value.tv_usec = QUANTUM % 1000000;
  timer.it_value.tv_sec = QUANTUM / 1000000;

  // Set the timer up (start the timer)
  setitimer(ITIMER_PROF, &timer, NULL);
}

// Function to handle the timer signal
static void timer_handler(int signum)
{
  // Save the current thread's context
  if (getcontext(&(current_thread_tcb->thread_context)) < 0)
  {
    perror("getcontext");
    exit(1);
  }

  // Enqueue the current thread back to the ready queue
  enqueue(ready_queue, current_thread_node);

  // Switch to the scheduler context
  swapcontext(&(current_thread_tcb->thread_context), &scheduler_context);
}

// Other function implementations...

// API to create a worker thread
int worker_create(worker_t *thread, pthread_attr_t *attr,
                  void *(*function)(void *), void *arg)
{
  // ... Existing code ...

  if (!init_scheduler_done)
  {
    init_scheduler();
    setup_timer();
    init_scheduler_done = 1;
  }

  // ... Existing code ...
}
