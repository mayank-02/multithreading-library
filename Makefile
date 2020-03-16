gcc -c mthread.c -g -Wall -fpic
gcc  mthread.o -shared -o mthread.so

# gcc -o main main.c -L. -R. -lmthreadW