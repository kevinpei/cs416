CC = gcc
CFLAGS = -g -w

all:: parallelCal vectorMultiply externalCal

parallelCal: 
	$(CC) $(CFLAGS) -pthread -o parallelCal parallelCal.c -L../ -lmymalloc

vectorMultiply: 
	$(CC) $(CFLAGS) -pthread -o vectorMultiply vectorMultiply.c -L../ -lmymalloc

externalCal: 
	$(CC) $(CFLAGS) -pthread -o externalCal externalCal.c -L../ -lmymalloc

clean:
	rm -rf parallelCal vectorMultiply externalCal *.o ./record/
