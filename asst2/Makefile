CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib


Target: my_pthread.a mymalloc.a

mymalloc.a: mymalloc.o
	$(AR) libmymalloc.a mymalloc.o
	$(RANLIB) libmymalloc.a
	
mymalloc.o: mymalloc.h
	$(CC) $(CFLAGS) mymalloc.c

my_pthread.a: my_pthread.o
	$(AR) libmy_pthread.a my_pthread.o
	$(RANLIB) libmy_pthread.a

my_pthread.o: my_pthread_t.h
	$(CC) -pthread $(CFLAGS) my_pthread.c	
	
clean:
	rm -rf testfile *.o *.a
