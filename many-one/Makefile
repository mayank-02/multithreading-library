# gcc -c mthread.c -g -Wall -fpic
# gcc  mthread.o -shared -o mthread.so
# gcc -o main main.c -L. -R. -lmthread

program: test3.o test2.o test1.o mthread.o queue.o spin_lock.o interrupt.o
		gcc test1.o mthread.o queue.o spin_lock.o interrupt.o -o test1 -g -Wall
		gcc test2.o mthread.o queue.o spin_lock.o interrupt.o -o test2 -g -Wall -lm
		gcc test3.o mthread.o queue.o spin_lock.o interrupt.o -o test3 -g -Wall

test3.o: test3.c
		gcc test3.c -c -g -Wall

test2.o: test2.c
		gcc test2.c -c -g -Wall -lm

test1.o: test1.c
		gcc test1.c -c -g -Wall

interrupt.o: interrupt.c interrupt.h
		gcc interrupt.c -c -g -Wall

spin_lock.o: spin_lock.c spin_lock.h
		gcc spin_lock.c -c -g -Wall

mthread.o: mthread.c mthread.h interrupt.c interrupt.h mangle.h
		gcc mthread.c interrupt.c -c -g -Wall

queue.o: queue.c queue.h
		gcc queue.c -c -g -Wall

run:
	./program

clean:
	rm *.o test1 test2 test3