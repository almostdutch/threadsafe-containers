# threadsafe-containers
threadsafe implementation of C++ containers: queue and stack

**Four implementations of threadsafe queue:
1.  Lock-based thread-safe unbounded queue implemented using library queue, locks, a single mutex, and a condition variable.
2. Lock-based thread-safe unbounded queue implemented using a singly-linked list, locks, fined-tuned mutexes (front and back mutex), and a condition variable.
3. Lock-free thread-safe unbounded queue implemented using a singly-linked list, and atomic operations with the strict memory models
4. Lock-free thread-safe unbounded queue implemented using a singly-linked list, and atomic operations with the relaxed memory models

**Three implementations of threadsafe stack:
1. Lock-based thread-safe unbounded stack implemented using library stack, locks, a single mutex, and a condition variable.
2. Lock-free thread-safe unbounded stack implemented using a singly-linked list, and atomic operations with the strict memory models
3. Lock-free thread-safe unbounded stack implemented using a singly-linked list, and atomic operations with the relaxed memory models
