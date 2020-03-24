## Multi threaded library
+ Implement a user level multithreading library. Since it's Linux systems, the library should come with options to do one-one, or many-one or many-many scheudling of user threads on kernel threads. It should support following functions:
thread_create() ; // provide option to use a desired mapping.
thread_join()
thread_exit()
thread_lock(); // a spinlock
thread_unlock();  // spin-unlock
thread_kill();
+ The code should be written as a single header file and single C file to make it a library.
+ Note: you need to use the SIGALRM signals to do scheduling. Learn setjmp and longjmp in C library.