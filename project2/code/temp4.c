// Add these global variables to your thread-worker.c file
static ucontext_t scheduler_context; // Scheduler context
static ucontext_t main_context;      // Main context
static struct itimerval timer;       // Timer struct

// Add this function prototype to your thread-worker.c file
void init_scheduler();

// Add this function definition to your thread-worker.c file
void init_scheduler()
{
  // Initialize the scheduler context
  getcontext(&scheduler_context);
  scheduler_context.uc_link = NULL;
  scheduler_context.uc_stack.ss_sp = malloc(STACK_SIZE);
  scheduler_context.uc_stack.ss_size = STACK_SIZE;
  scheduler_context.uc_stack.ss_flags = 0;

  // Create a context for the main thread
  getcontext(&main_context);
  main_context.uc_link = &scheduler_context;
  main_context.uc_stack.ss_sp = malloc(STACK_SIZE);
  main_context.uc_stack.ss_size = STACK_SIZE;
  main_context.uc_stack.ss_flags = 0;

  // Set up the timer
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = &schedule; // Call schedule() when the timer goes off
  sigaction(SIGPROF, &sa, NULL);

  timer.it_interval.tv_usec = 0;
  timer.it_interval.tv_sec = 0;
  timer.it_value.tv_usec = QUANTUM;
  timer.it_value.tv_sec = 0;
  setitimer(ITIMER_PROF, &timer, NULL);
}

// Modify the worker_create function to initiate the timer
int worker_create(worker_t *thread, pthread_attr_t *attr,
                  void *(*function)(void *), void *arg)
{
  if (!init_scheduler_done)
  {
    init_scheduler();
    init_scheduler_done = 1;
  }

  tcb *new_tcb = (tcb *)malloc(sizeof(tcb));
  if (new_tcb == NULL)
  {
    // Handle memory allocation error
    return -1;
  }

  // ... (rest of your worker_create implementation)

  // Start the timer on the first thread creation
  if (ready_queue->size == 1)
  {
    // Switch to the main context to start the timer
    setcontext(&main_context);
  }

  return 0;
}
