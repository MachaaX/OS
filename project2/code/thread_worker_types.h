#ifndef TW_TYPES_H
#define TW_TYPES_H

#include <ucontext.h>

typedef unsigned int worker_t;

typedef enum status
{
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
} status;

typedef struct TCB
{
    /* add important states in a thread control block */
    // thread Id
    // thread status
    // thread context
    // thread stack
    // thread priority
    // And more ...

    // YOUR CODE HERE
    worker_t thread_id;
    status thread_status;
    ucontext_t thread_context;
    void *thread_stack; // Base address of the thread's stack
    size_t stack_size;
    int priority;
    void *return_value;

} tcb;

typedef struct tcbNode
{
    tcb *thread_data;
    struct tcbNode *next;
} tcbNode;

typedef struct runQueue
{
    tcbNode *head;
    tcbNode *tail;
    int size;
} runQueue;

#endif
