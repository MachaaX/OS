#ifndef MTX_TYPES_H
#define MTX_TYPES_H

#include "thread_worker_types.h"

/* mutex struct definition */
typedef struct worker_mutex_t
{
    /* add something here */

    // YOUR CODE HERE
    int is_locked;            // Flag to indicate whether the mutex is locked
    tcb *locking_thread;      // Reference to the thread holding the mutex
    runQueue blocked_threads; // Queue of threads waiting for the mutex
} worker_mutex_t;

#endif
