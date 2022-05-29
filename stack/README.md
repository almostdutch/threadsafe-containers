# thread-safe-queues
Performance comparison of six implementations of unbounded thread-safe queues with varying potential for concurrency <br/>

**Queue 1:**<br/>
Lock-based wait-based thread-safe unbounded queue implemented using library queue, <br/>
locks, a single mutex, and a condition variable. <br/>

**Queue 2:**<br/>
Lock-based wait-free thread-safe unbounded queue implemented using library queue, <br/>
locks, and a single mutex. <br/>

**Queue 3:**<br/>
Lock-based wait-based thread-safe unbounded queue implemented using a singly-linked list, <br/>
locks, fined-tuned mutexes (front and back mutex), and a condition variable. <br/>

**Queue 4:**<br/>
Lock-based wait-free thread-safe unbounded queue implemented using a singly-linked list, <br/>
locks, and fined-tuned mutexes (front and back mutex). <br/>

**Queue 5:**<br/>
Lock-free wait-based thread-safe unbounded queue implemented using a singly-linked list, <br/>
and atomic operations with the strict memory models. <br/>

**Queue 6:**<br/>
Lock-free wait-based thread-safe unbounded queue implemented using a singly-linked list, <br/>
and atomic operations with the tuned memory models. <br/>

**Test:**<br/>
100000 (100k) INTs are PUSHed onto and POPed off the queue concurrently. Each test was run 100 times.

**Result:**<br/>
<p align="center">
  <img src="result_of_test_runs.png" width="1400" height="360"/>
</p>
