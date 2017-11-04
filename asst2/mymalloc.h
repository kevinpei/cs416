#include <stdlib.h>
#include <stdio.h>
#include "my_pthread_t.h"

#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ)
#define	free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ)
#define memorySize 8388608
#define FALSE 0
#define TRUE 1
#define boolean char

/*
MemoryData include next and prev pointers in order to merge adjacent free memory blocks both forwards and backwards.
size tracks how many non-metadata bytes are available for storage. isFree tracks whether the memory block is free or in use.
*/
typedef struct _MemoryData {
	struct _MemoryData * next;
	struct _MemoryData * prev; 
	short int size;
	short int pid;
	boolean isFree; 
}MemoryData; 

//Function prototypes
boolean initialize();
MemoryData* findFirstFree(int size, MemoryData * start);
void * myallocate(int size, char* myfile, int line, int req);
void mydeallocate(void * mementry, char * myfile, int line, int req);

//Library constants
int THREADREQ;
