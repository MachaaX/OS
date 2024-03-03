#include "thread-worker.h"
#include "thread_worker_types.h"

#define STACK_SIZE 16 * 1024
#define QUANTUM 10 * 1000

// Scheduler and Main Contexts
static ucontext_t scheduler_context;
static ucontext_t main_context;

// Timer
static struct itimerval timer;

static void schedule(); // Scheduler function declaration

void timer_handler(int signum)
{
  // Your timer handling logic here
  schedule();
}

// Initialize scheduler and main contexts
void init_scheduler()
{
  // Initialize the scheduler context
  getcontext(&scheduler_context);
  scheduler_context.uc_stack.ss_sp = malloc(STACK_SIZE);
  scheduler_context.uc_stack.ss_size = STACK_SIZE;
  scheduler_context.uc_link = NULL;

  // Initialize the main context
  getcontext(&main_context);
  main_context.uc_stack.ss_sp = malloc(STACK_SIZE);
  main_context.uc_stack.ss_size = STACK_SIZE;
  main_context.uc_link = NULL;

  // Set up the timer signal handler
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = &timer_handler;
  sigaction(SIGPROF, &sa, NULL);
}

// Set up and start the timer
void start_timer()
{
  // Set up what the timer should reset to after the timer goes off
  timer.it_interval.tv_usec = QUANTUM;
  timer.it_interval.tv_sec = 0;

  // Set up the current timer to go off in the specified quantum
  timer.it_value.tv_usec = QUANTUM;
  timer.it_value.tv_sec = 0;

  // Set the timer up (start the timer)
  setitimer(ITIMER_PROF, &timer, NULL);
}

// Scheduler function
static void schedule()
{
  // Your scheduling logic goes here
  // This function will be called each time the timer goes off
  // You can switch between the scheduler and worker threads here
}

// Other worker functions (worker_create, worker_yield, worker_exit, worker_join, etc.) go here
