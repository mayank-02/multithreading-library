# gcc -c mthread.c -g -Wall -fpic
# gcc  mthread.o -shared -o mthread.so
# gcc -o main main.c -L. -R. -lmthread

program: test1.o mthread.o queue.o
		gcc test1.o mthread.o queue.o -o program -g -Wall

test1.o: test1.c
		gcc test1.c -c -g -Wall

mthread.o: mthread.c mthread.h
		gcc mthread.c -c -g -Wall

queue.o: queue.c queue.h
		gcc queue.c -c -g -Wall

run:
	./program

clean:
	rm *.o program