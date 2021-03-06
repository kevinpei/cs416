// File:	y_malloc.h
// Author:	Yujie REN
// Date:	09/23/2017

// name: Derek Mao mm2180, Kevin Pei ksp98, Quzhi Li ql88
// username of iLab: ksp98
// iLab Server: ls.cs.rutgers.edu

#ifndef MYMALLOC_H
#define MYMALLOC_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <ucontext.h>
#include <sys/time.h>
#include <signal.h>
#include "mymalloc.h"

#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ)
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ)
#define memorySize 8388608
#define FALSE 0
#define TRUE 1
#define boolean int

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
	int page_id;
	boolean isContinuous;
	struct _PageData *next;
} PageData;

/*
MemoryData include next and prev pointers in order to merge adjacent free memory blocks both forwards and backwards.
size tracks how many non-metadata bytes are available for storage. isFree tracks whether the memory block is free or in use.
*/
typedef struct _MemoryData
{
	struct _MemoryData *next;
	struct _MemoryData *prev;
	int size;
	boolean isFree;
} MemoryData;

//Function prototypes
void mymalloc_initialize();
MemoryData *findFirstFree(int size, MemoryData *start);
void *myallocate(int size, char *myfile, int line, int req);
void mydeallocate(void *mementry, char *myfile, int line, int req);

void write_memory_to_file();
void segment_fault_handler(int signum);

//Library constants
int THREADREQ;

#endif