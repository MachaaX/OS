#include "thread-worker.h"
#include "thread_worker_types.h"
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#define STACK_SIZE 16 * 1024
#define QUANTUM 10 * 1000

static ucontext_t main_ctx, scheduler_ctx;
static runQueue scheduler_runqueue;

// Function prototypes
static void schedule();
static void timer_handler(int signo);

void init_scheduler()
{
  // Initialize the scheduler context
  getcontext(&scheduler_ctx);
  scheduler_ctx.uc_link = NULL;
  scheduler_ctx.uc_stack.ss_sp = malloc(STACK_SIZE);
  scheduler_ctx.uc_stack.ss_size = STACK_SIZE;
  scheduler_ctx.uc_stack.ss_flags = 0;
  makecontext(&scheduler_ctx, schedule, 0);

  // Initialize the scheduler run queue
  scheduler_runqueue.head = scheduler_runqueue.tail = NULL;
  scheduler_runqueue.size = 0;

  // Set up the timer
  struct sigaction sa;
  sa.sa_handler = &timer_handler;
  sa.sa_flags = SA_RESTART;
  sigaction(SIGPROF, &sa, NULL);

  struct itimerval timer;
  timer.it_interval.tv_usec = 0;
  timer.it_interval.tv_sec = 0;
  timer.it_value.tv_usec = QUANTUM;
  timer.it_value.tv_sec = 0;
  setitimer(ITIMER_PROF, &timer, NULL);
}

void timer_handler(int signo)
{
  // Handle timer interrupt
  if (signo == SIGPROF)
  {
    // Save the current context to main_ctx and switch to scheduler_ctx
    swapcontext(&main_ctx, &scheduler_ctx);
  }
}

int worker_create(worker_t *thread, pthread_attr_t *attr,
                  void *(*function)(void *), void *arg)
{
  // Create a new TCB
  tcb *new_thread = (tcb *)malloc(sizeof(tcb));
  new_thread->thread_id = *thread;
  new_thread->thread_status = NEW;
  new_thread->thread_stack = malloc(STACK_SIZE);
  new_thread->stack_size = STACK_SIZE;
  new_thread->priority = 0;
  new_thread->return_value = NULL;

  // Set up the context for the new thread
  getcontext(&(new_thread->thread_context));
  new_thread->thread_context.uc_link = NULL;
  new_thread->thread_context.uc_stack.ss_sp = new_thread->thread_stack;
  new_thread->thread_context.uc_stack.ss_size = STACK_SIZE;
  new_thread->thread_context.uc_stack.ss_flags = 0;
  makecontext(&(new_thread->thread_context), (void (*)(void))function, 1, arg);

  // Enqueue the new thread in the scheduler run queue
  tcbNode *new_node = (tcbNode *)malloc(sizeof(tcbNode));
  new_node->thread_data = new_thread;
  new_node->next = NULL;

  if (scheduler_runqueue.tail == NULL)
  {
    scheduler_runqueue.head = scheduler_runqueue.tail = new_node;
  }
  else
  {
    scheduler_runqueue.tail->next = new_node;
    scheduler_runqueue.tail = new_node;
  }

  scheduler_runqueue.size++;

  return 0;
}

static void schedule()
{
  // Scheduler logic goes here
  // For simplicity, just dequeue a thread and switch to its context
  if (scheduler_runqueue.size > 0)
  {
    tcbNode *next_thread_node = scheduler_runqueue.head;
    tcb *next_thread = next_thread_node->thread_data;

    // Dequeue the next thread
    scheduler_runqueue.head = next_thread_node->next;
    if (scheduler_runqueue.head == NULL)
    {
      scheduler_runqueue.tail = NULL;
    }
    scheduler_runqueue.size--;

    // Save the current context to main_ctx and switch to the next_thread context
    swapcontext(&main_ctx, &(next_thread->thread_context));
  }
}

void init_scheduler_done()
{
  init_scheduler_done = 1;
}

int worker_yield()
{
  // Save the current context to main_ctx and switch to scheduler_ctx
  swapcontext(&main_ctx, &scheduler_ctx);
  return 0;
}

void worker_exit(void *value_ptr)
{
  // Handle thread exit
  // Deallocate any dynamic memory created by the thread
  tcb *current_thread = scheduler_runqueue.head->thread_data;
  current_thread->return_value = value_ptr;

  // Simulate thread termination by yielding to the scheduler
  worker_yield();
}

int worker_join(worker_t thread, void **value_ptr)
{
  // Wait for a specific thread to terminate
  // For simplicity, just yield to the scheduler
  worker_yield();
  return 0;
}

int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t *mutexattr)
{
  // Initialize data structures for the mutex
  return 0;
}

int worker_mutex_lock(worker_mutex_t *mutex)
{
  // Acquire the mutex lock
  // For simplicity, just yield to the scheduler
  worker_yield();
  return 0;
}

int worker_mutex_unlock(worker_mutex_t *mutex)
{
  // Release the mutex lock
  // For simplicity, just yield to the scheduler
  worker_yield();
  return 0;
}

int worker_mutex_destroy(worker_mutex_t *mutex)
{
  // Destroy the mutex
  // For simplicity, just yield to the scheduler
  worker_yield();
  return 0;
}
