The task involves implementing several non-blocking queues with multiple readers and writers: SimpleQueue, RingsQueue, LLQueue, and BLQueue. Two implementations use standard mutexes, while the other two use atomic operations, including `compare_exchange`.

Each implementation must include:
- A structure `<queue>`.
- Methods:
   - `<queue>* <queue>_new(void)`: Allocates and initializes a new queue.
   - `void <queue>_delete(<queue>* queue)`: Frees memory allocated by queue methods.
   - `void <queue>_push(<queue>* queue, Value value)`: Adds a value to the end of the queue.
   - `Value <queue>_pop(<queue>* queue)`: Retrieves a value from the front of the queue or returns `EMPTY_VALUE` if the queue is empty.
   - `bool <queue>_is_empty(<queue>* queue)`: Checks if the queue is empty.

Queue values are of type `Value`, an `int64_t` (though `void*` would be typical). The users guarantee that `new/delete` are called exactly once, respectively, before and after all `push/pop/is_empty` operations from all threads.

All implementations should behave as if `push`, `pop`, and `is_empty` operations are indivisible, without causing memory leaks or deadlocks. Fairness and lack of starvation are not required for each thread individually. Lock-free queues (LLQueue and BLQueue) must ensure that in any concurrent execution, at least one `push`, `pop`, and `is_empty` operation will complete in a finite number of steps.

### Implementations:

1. **SimpleQueue**: Uses a singly linked list with two mutexes (one for producers and one for consumers). The list nodes include an atomic `next` pointer, a value, a `head` pointer with its mutex, and a `tail` pointer with its mutex. The `head` always contains an initial node with an empty or dummy value to simplify edge cases.

2. **RingsQueue**: Combines a singly linked list with a circular buffer. Each node includes a circular buffer of `RING_SIZE` values, atomic `push_idx` and `pop_idx` counters, and mutexes for `pop` and `push`. The circular buffer size is defined by `RING_SIZE` (default 1024). Push and pop operations are designed to handle concurrency with a focus on avoiding contention.

3. **LLQueue**: A lock-free queue implemented with a singly linked list. Nodes include an atomic `next` pointer, a value (`EMPTY_VALUE` if already popped), and atomic `head` and `tail` pointers. The `push` and `pop` operations must loop, retrying steps if necessary to maintain lock-free behavior.

4. **BLQueue**: Another lock-free queue using a list of buffers. Each node has a buffer of `BUFFER_SIZE` values and atomic `push_idx` and `pop_idx` counters. Nodes are linked together, and buffers are filled or emptied as needed. Push and pop operations must handle buffer overflow and underflow gracefully.

### Hazard Pointer:

Hazard Pointer is used for safe memory reclamation in concurrent data structures. It involves:
- Reserving a node address to protect it from being freed while still in use.
- Retiring nodes by adding them to a set of retired addresses for later cleanup.

Hazard Pointer operations include:
- `HazardPointer_register(int thread_id, int num_threads)`: Registers a thread.
- `HazardPointer_initialize(HazardPointer* hp)`: Initializes the structure.
- `HazardPointer_finalize(HazardPointer* hp)`: Cleans up and frees memory.
- `HazardPointer_protect(HazardPointer* hp, const AtomicPtr* atom)`: Protects a node address.
- `HazardPointer_clear(HazardPointer* hp)`: Clears a protection.
- `HazardPointer_retire(HazardPointer* hp, void* ptr)`: Retires a node for later cleanup.

### Technical Requirements:
- Implement in C, compliant with C11 or later.
- Submit as a tarball (`ab1234567.tar` or `.tgz`) with a folder `ab1234567` containing `CMakeLists.txt`.
- Modify only `.c` files and `HazardPointer.h`.
- Compilation and execution are only on a modern Linux system with gcc and x86_64 architecture.
- Use only standard libraries and pthreads. Avoid using assembler, creating threads, or processes.
- For SimpleQueue and RingsQueue, only use specific pthread functions. For LLQueue and BLQueue, avoid pthreads and use atomic functions explicitly.