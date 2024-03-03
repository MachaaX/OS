// File:	thread-worker.c

// List all group member's name:
/*
 */
// username of iLab:
// iLab Server:

#include "thread-worker.h"
#include "thread_worker_types.h"

#define STACK_SIZE 16 * 1024
#define QUANTUM 10 * 1000

// INITIALIZE ALL YOUR OTHER VARIABLES HERE
int init_scheduler_done = 0;
static runQueue *ready_queue = NULL;
static tcb *running_thread = NULL; // Queue for ready threads
static struct itimerval timer;
static ucontext_t scheduler_context, main_context;

/* create a new thread */
int worker_create(worker_t *thread, pthread_attr_t *attr,
                  void *(*function)(void *), void *arg)
{
    // - create Thread Control Block (TCB)
    // - create and initialize the context of this worker thread
    // - allocate space of stack for this thread to run
    // after everything is set, push this thread into run queue and
    // - make it ready for the execution.

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

    // Initialize the new thread
    new_tcb->thread_id = *thread;
    new_tcb->thread_status = READY;
    new_tcb->thread_stack = malloc(STACK_SIZE);
    if (new_tcb->thread_stack == NULL)
    {
        // Handle memory allocation error
        return -1;
    }
    new_tcb->stack_size = STACK_SIZE;
    new_tcb->priority = 0; // Set a default priority for new threads
    new_tcb->return_value = NULL;

    // Initialize the thread context
    if (getcontext(&(new_tcb->thread_context)) < 0)
    {
        perror("getcontext");
        exit(1);
    }
    new_tcb->thread_context.uc_stack.ss_sp = new_tcb->thread_stack;
    new_tcb->thread_context.uc_stack.ss_size = STACK_SIZE;
    new_tcb->thread_context.uc_link = NULL;
    new_tcb->thread_context.uc_stack.ss_flags = 0;
    makecontext(&new_tcb->thread_context, (void (*)(void))function, 1, arg);

    tcbNode *new_thread_node = (tcbNode *)malloc(sizeof(tcbNode));
    if (new_thread_node == NULL)
    {
        // Handle memory allocation error
        return -1;
    }
    // Initialize the new thread node
    new_thread_node->thread_data = new_tcb;
    new_thread_node->next = NULL;

    // Add the new thread to the ready queue
    enqueue(ready_queue, new_thread_node);

    return 0;
}

/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield()
{

    // - change worker thread's state from Running to Ready
    // - save context of this thread to its thread control block
    // - switch from thread context to scheduler context
    if (running_thread == NULL || ready_queue->size == 0 || running_thread->thread_status != RUNNING)
    {
        // Nothing to yield to
        return -1;
    }

    running_thread->thread_status = READY;
    if (getcontext(&(running_thread->thread_context)) < 0)
    {
        perror("getcontext");
        exit(1);
    }

    swapcontext(&(running_thread->thread_context), &scheduler_context);
    return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr)
{
    // - if value_ptr is provided, save return value
    // - de-allocate any dynamic memory created when starting this thread (could be done here or elsewhere)
    if (running_thread == NULL)
    {
        return;
    }
    if (value_ptr != NULL)
    {
        running_thread->return_value = value_ptr;
    }
    running_thread->thread_status = TERMINATED;

    // Free the stack memory allocated for this thread
    free(running_thread->thread_stack);

    // Free the TCB memory allocated for this thread
    free(running_thread);

    // Context switch back to the scheduler
    swapcontext(&(scheduler_context), &(main_context));
}

/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr)
{

    // - wait for a specific thread to terminate
    // - if value_ptr is provided, retrieve return value from joining thread
    // - de-allocate any dynamic memory created by the joining thread
    return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex,
                      const pthread_mutexattr_t *mutexattr)
{
    //- initialize data structures for this mutex
    if (mutex == NULL)
    {
        return -1;
    }
    // Check if the mutex is already initialized
    if (mutex->is_locked)
    {
        return -1; // Mutex is already initialized
    }

    // Initialize the mutex
    mutex->is_locked = 0;
    mutex->locking_thread = NULL;
    mutex->blocked_threads.head = NULL;
    mutex->blocked_threads.tail = NULL;
    mutex->blocked_threads.size = 0;
    return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex)
{

    // - use the built-in test-and-set atomic function to test the mutex
    // - if the mutex is acquired successfully, enter the critical section
    // - if acquiring mutex fails, push current thread into block list and
    // context switch to the scheduler thread
    if (mutex == NULL)
    {
        return -1; // Invalid mutex
    }
    // Check if the mutex is already locked
    while (__sync_lock_test_and_set(&(mutex->locked), 1))
    {

        // If the mutex is already locked, block the current thread
        tcbNode *blocked_thread_node = (tcbNode *)malloc(sizeof(tcbNode));
        blocked_thread_node->thread_data = running_thread;
        blocked_thread_node->next = NULL;
        enqueue(&(mutex->blocked_threads), blocked_thread_node);
        running_thread->thread_status = BLOCKED;

        // Switch to the scheduler context
        swapcontext(&(running_thread->thread_context), &scheduler_context);
    }

    // Lock the mutex
    mutex->is_locked = 1;
    mutex->locking_thread = running_thread;

    return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex)
{
    // - release mutex and make it available again.
    // - put one or more threads in block list to run queue
    // so that they could compete for mutex later.
    // Check if the calling thread holds the mutex
    if (mutex == NULL)
    {
        return -1;
    }
    if (mutex->locking_thread != running_thread)
    {
        return -1; // Calling thread does not hold the mutex
    }

    // Unlock the mutex
    mutex->is_locked = 0;
    mutex->locking_thread = NULL;
    // Move a waiting thread from the blocked list to the ready queue
    if (mutex->blocked_threads.size > 0)
    {
        tcbNode *thread_to_release = dequeue(&(mutex->blocked_threads));
        enqueue(ready_queue, thread_to_release);
    }
    return 0;
};

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex)
{
    // - make sure mutex is not being used
    // - de-allocate dynamic memory created in worker_mutex_init
    // Check if the mutex is currently locked
    if (mutex == NULL)
    {
        return -1;
    }
    if (mutex->is_locked)
    {
        return -1; // Mutex is still locked, cannot destroy
    }

    // Reset the mutex
    mutex->is_locked = 0;
    mutex->locking_thread = NULL;

    while (mutex->blocked_threads.head != NULL)
    {
        tcbNode *temp = mutex->blocked_threads.head;
        mutex->blocked_threads.head = mutex->blocked_threads.head->next;
        free(temp);
    }
    mutex->blocked_threads.head = mutex->blocked_threads.tail = NULL;

    return 0;
};

/* scheduler */
static void schedule()
{
// - every time a timer interrupt occurs, your worker thread library
// should be contexted switched from a thread context to this
// schedule() function

// - invoke scheduling algorithms according to the policy (RR or MLFQ)

// - schedule policy
#ifndef MLFQ
    // Choose RR
    sched_rr();
#else
    // Choose MLFQ
    sched_mlfq();
#endif
}

static void sched_rr()
{
    // - your own implementation of RR
    // (feel free to modify arguments and return types)
    // If there are no threads in the ready queue, return
    if (ready_queue->size == 0)
    {
        return;
    }

    // Dequeue the next thread from the ready queue
    tcbNode *next_thread_node = dequeue(ready_queue);
    tcb *next_thread = next_thread_node->thread_data;

    // Update the status of the next thread to RUNNING
    next_thread->thread_status = RUNNING;

    // Save the context of the currently running thread
    tcb *current_thread = running_thread;
    if (current_thread != NULL)
    {
        current_thread->thread_status = READY;
        if (getcontext(&(current_thread->thread_context)) < 0)
        {
            perror("getcontext");
            exit(1);
        }
    }

    // Switch to the context of the next thread
    running_thread = next_thread;
    swapcontext(&(current_thread->thread_context), &(next_thread->thread_context));
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq()
{
    // - your own implementation of MLFQ
    // (feel free to modify arguments and return types)
}

// Feel free to add any other functions you need.
// You can also create separate files for helper functions, structures, etc.
// But make sure that the Makefile is updated to account for the same.

// Initialize the scheduler
void init_scheduler()
{
    // Initialize main context
    if (getcontext(&main_context) < 0)
    {
        perror("getcontext");
        exit(1);
    }

    // Allocate space for the scheduler stack
    void *scheduler_stack = malloc(STACK_SIZE);

    if (scheduler_stack == NULL)
    {
        perror("Failed to allocate stack for scheduler");
        exit(1);
    }

    // Initialize scheduler context
    if (getcontext(&scheduler_context) < 0)
    {
        perror("getcontext");
        exit(1);
    }
    scheduler_context.uc_link = &main_context;
    scheduler_context.uc_stack.ss_sp = scheduler_stack;
    scheduler_context.uc_stack.ss_size = STACK_SIZE;
    scheduler_context.uc_stack.ss_flags = 0;

    // Make the scheduler context to start running at the schedule() function
    makecontext(&scheduler_context, schedule, 0);

    // Initialize the ready queue
    ready_queue = (runQueue *)malloc(sizeof(runQueue));
    ready_queue->head = ready_queue->tail = NULL;
    ready_queue->size = 0;

    // setup timer
    setup_timer();
}
// Function to set up the timer
static void setup_timer()
{
    // Use sigaction to register signal handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &schedule; // schedule function is the signal handler everytime theres an interrupt
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
// Function to enqueue a thread in the run queue
void enqueue(runQueue *queue, tcbNode *thread)
{
    if (queue->tail == NULL)
    {
        queue->head = queue->tail = thread;
    }
    else
    {
        queue->tail->next = thread;
        queue->tail = thread;
    }
    // thread->next = NULL;
    queue->size++;
}
// Function to dequeue a thread from the run queue
tcbNode *dequeue(runQueue *queue)
{
    if (queue->head == NULL)
    {
        // Queue is empty
        return NULL;
    }

    tcbNode *dequeued_thread = queue->head;
    queue->head = queue->head->next;

    if (queue->head == NULL)
    {
        // If the queue becomes empty after dequeue, update the tail pointer
        queue->tail = NULL;
    }

    queue->size--;

    return dequeued_thread;
}

// tcb *get_running_thread()
// {
//     tcbNode *current_node = ready_queue->head;

//     while (current_node != NULL)
//     {
//         tcb *current_thread = current_node->thread_data;
//         if (current_thread->thread_status == RUNNING)
//         {
//             return current_thread;
//         }

//         current_node = current_node->next;
//     }

//     return NULL;
// }