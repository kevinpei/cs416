#include <stdlib.h>
#include <stdio.h>

#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ)
#define	free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ)
#define memorySize 8192
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
	boolean isFree; 
}MemoryData; 

//Function prototypes
boolean initialize();
MemoryData* findFirstFree(int size, MemoryData * start);
void * mymalloc(int size, char* myfile, int line);
void myfree(void * mementry, char * myfile, int line);

//Library constants
int THREADREQ;
