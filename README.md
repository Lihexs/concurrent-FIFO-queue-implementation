# Concurrent FIFO Queue Implementation

## Project Overview

This project implements a generic concurrent FIFO (First-In-First-Out) queue in C as part of the Operating Systems (0368-2162) course assignment. The main goal is to create a thread-safe queue that supports enqueue and dequeue operations, allowing multiple threads to interact with the queue simultaneously.

## Features

1. Thread-safe enqueue and dequeue operations
2. Blocking dequeue when the queue is empty
3. Non-blocking tryDequeue operation
4. FIFO ordering for both items and waiting threads
5. Queue size tracking
6. Waiting threads counter
7. Visit counter for items that have passed through the queue

Key aspects of the implementation include:
- Use of mutex locks for thread synchronization
- Condition variables for managing waiting threads
- A ticketing system to ensure FIFO ordering of waiting threads

## Implementation Details

### Structure

The project consists of a single C file (`queue.c`) that implements the required functions defined in the header file (`queue.h`). The main components are:

1. `Node`: A struct representing an item in the queue
2. `Queue`: A struct containing the queue's state and synchronization primitives
3. Public functions: `initQueue`, `destroyQueue`, `enqueue`, `dequeue`, `tryDequeue`, `size`, `waiting`, and `visited`

### Key Algorithms and Techniques

1. Ticketing System: 
   - Ensures FIFO ordering of waiting threads
   - Uses `serving_ticket` and `next_ticket` to manage the order

2. Condition Variable Usage:
   - `not_empty` condition variable for managing waiting threads
   - `cnd_signal` and `cnd_broadcast` for waking up waiting threads

3. Lock-free Operations:
   - `size`, `waiting`, and `visited` functions do not use locks for better performance

### Libraries and System Calls

The implementation uses the following C libraries and threading primitives:
- `<threads.h>`: For thread management and synchronization
- `<stdlib.h>`: For memory allocation
- `<stdbool.h>`: For boolean type
- `<stddef.h>`: For size_t type

Key threading functions used:
- `mtx_init`, `mtx_lock`, `mtx_unlock`, `mtx_destroy`
- `cnd_init`, `cnd_wait`, `cnd_signal`, `cnd_broadcast`, `cnd_destroy`

### Challenging Aspects

1. Ensuring FIFO ordering for both items and waiting threads:
   - Solved by implementing a ticketing system for thread ordering

2. Implementing lock-free operations for `size`, `waiting`, and `visited`:
   - These functions access shared data without locks, which may result in slightly inaccurate results during concurrent operations but ensures better performance

3. Proper cleanup and resource management:
   - Implemented in the `destroyQueue` function to prevent memory leaks

## Usage Examples

1. Basic usage:

```c
#include "queue.h"

int main() {
    initQueue();
    
    int item1 = 10, item2 = 20;
    enqueue(&item1);
    enqueue(&item2);
    
    int *result = (int *)dequeue();
    printf("Dequeued item: %d\n", *result);  // Output: Dequeued item: 10
    
    destroyQueue();
    return 0;
}
```

2. Using tryDequeue:

```c
void *item;
if (tryDequeue(&item)) {
    printf("Successfully dequeued item: %p\n", item);
} else {
    printf("Queue is empty\n");
}
```

3. Checking queue status:

```c
printf("Current queue size: %zu\n", size());
printf("Number of waiting threads: %zu\n", waiting());
printf("Total items processed: %zu\n", visited());
```

Note: This implementation is designed to be used in a multi-threaded environment. Proper thread creation and management should be handled by the user of this queue library.
