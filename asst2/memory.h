#ifndef MEMORY_H
#define MEMORY_H

#include <stdlib.h>
#include "my_pthread_t.h"

#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ)
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ)

#define MEMORY_SIZE 8000000
#define PAGE_CTRL_SIZE 1000
#define PAGE_DATA_SIZE 4000
#define TRUE 1
#define FALSE 0

/* typedef: */
typedef char boolean;

/* structs: */
typedef struct PageCtrl_
{
    int page_id;        // unique page id, used to sort and identify
    boolean free;       // whether this page has been allocated or not
    my_pthread_t pid;   // owner thread of this page
    int num_page_alloc; // number of pages allocated by this thread
    int next_page;      // page id of next page if the threads allocates multiple pages
} PageCtrl;

typedef struct MallocCtrl_
{
    struct MallocCtrl_ *prev; // address for previous MallocCtrl
    struct MallocCtrl_ *next; // address for next MallocCtrl
    boolean free;             // whether the space after this MallocCtrl block is freed or not
    unsigned int size;        // size of data/space after this MallocCtrl
} MallocCtrl;

/* functions: */
void *myMalloc(unsigned int, char *, int);
void myFree(void *, char *, int);

/* global variables: */
int current_page_id;
long max_page_size;

#endif
