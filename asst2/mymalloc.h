#include <stdlib.h>
#include <stdio.h>
#include "my_pthread_t.h"

#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ)
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ)
#define MEMORY_SIZE 8000000
#define PAGE_SIZE 4000
#define META_SIZE 1000
#define FALSE 0
#define TRUE 1
// #define boolean char
typedef char boolean;

/*
PageData includes a next pointer to point to the next memory block. It is the same as MemoryData except that it must store the
pid of the thread using this page and whether it's garbage or not. notGarbage is an int because if it's overwritten it's more
likely not to be 1 (not garbage) than if it was a smaller data type.
*/
typedef struct _PageData
{
	//Stores where the memory is supposed to start
	struct _MemoryData *pageStart;
	int pid;
	int pageid;
	boolean isContinuous;
	//page id of the next page data in the continuous page
	struct _PageData *current_page;
	int current_location;
	short int next;
} PageData;

/*
MemoryData include next and prev pointers in order to merge adjacent free memory blocks both forwards and backwards.
size tracks how many non-metadata bytes are available for storage. isFree tracks whether the memory block is free or in use.
*/
typedef struct _MemoryData
{
	struct _MemoryData *next;
	struct _MemoryData *prev;
	unsigned short int size;
	boolean isFree;
} MemoryData;

//Function prototypes
boolean initialize();
MemoryData *findFirstFree(int size, MemoryData *start);
void *myallocate(int size, char *myfile, int line, int req);
void mydeallocate(void *mementry, char *myfile, int line, int req);

//Library constants
int THREADREQ;
