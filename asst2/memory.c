#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include "memory.h"

static char memory[MEMORY_SIZE];

/* initalize global values and all page contel blocks in memory */
void initPages()
{
    memset(memory, 0, MEMORY_SIZE);
    current_page_id = 0;
    max_page_size = sysconf(_SC_PAGE_SIZE);
    if (max_page_size == -1)
    {
        if (errno == EINVAL)
        {
            perror("initPage() failed");
            exit(EXIT_FAILURE);
        }
        else
        {
            max_page_size = MEMORY_SIZE - MEMORY_SIZE / (PAGE_CTRL_SIZE + PAGE_DATA_SIZE) * PAGE_CTRL_SIZE - sizeof(MallocCtrl);
        }
    }
    int next_page_id = 1;
    PageCtrl *ptr = memory;
    while (ptr < memory + MEMORY_SIZE)
    {
        ptr->page_id = next_page_id;                               // assign page id
        ptr->free = TRUE;                                          // not allocated at the begining
        ptr->pid = 0;                                              // no owner, init to 0
        ptr->num_page_alloc = 0;                                   // no allocation yet, init to 0
        ptr->next_page = 0;                                        // no next page, init to 0
        next_page_id++;                                            // increment next page id
        ptr += (PAGE_CTRL_SIZE + PAGE_DATA_SIZE) / PAGE_CTRL_SIZE; // move pointer to next page control block
    }
}

/* move all pages back to the original location when a thread is swapped out */
void sortPage()
{
    PageCtrl *alloc_root = memory + (current_page_id - 1) * (PAGE_CTRL_SIZE + PAGE_DATA_SIZE);
    int k;
    char *pointer = alloc_root;
    char buffer[PAGE_CTRL_SIZE + PAGE_DATA_SIZE];
    for (k = 1; k < alloc_root->num_page_alloc; k++)
    {

    }
}

/* make the page contiguous when a thread is swapped in */
void loadPage(my_pthread_t pid)
{
}

/* swap the given page */
void swapPage()
{
}

void *myallocate(unsigned int size, char *file_name, int line_number, int request)
{
    static int initialized = 0;
    MemCtrl *ptr;

    if (!initialized)
    {
        initialized = 1;
        root->prev = NULL;
        root->next = NULL;
        root->free = 1;
        root->size = 5000 - sizeof(MemCtrl);
    }

    ptr = root;
    while ((!ptr->free || ptr->size < size) && ptr->next != NULL)
    {
        ptr = ptr->next;
    }

    if (!ptr->free || ptr->size < size)
    {
        printf("no enough memory available");
        return NULL;
    }
    else
    {
        if (ptr->size > (size + sizeof(MemCtrl)))
        {
            ptr->next = (MemCtrl *)((char *)ptr + sizeof(MemCtrl) + size);
            ptr->next->prev = ptr;
            ptr->next->next = NULL;
            ptr->next->free = 1;
            ptr->next->size = ptr->size - size - sizeof(MemCtrl);
        }
        ptr->free = 0;
        return (char *)ptr + sizeof(MemCtrl);
    }

    printf("no enough memory available");
    return NULL;
}

void mydeallocate(void *target, char *file_name, int line_number, int request)
{
    MemCtrl *ptr = root;
    target = (char *)target - sizeof(MemCtrl);

    if ((char *)target > &(memory[4999]) || (char *)target < memory)
    {
        fprintf(stderr, "error at line %d of file \"%s\": pointer not allocated dynamically\n", line_number, file_name);
        return;
    }

    while (ptr != NULL)
    {
        if ((MemCtrl *)target == ptr)
        {
            break;
        }
        ptr = ptr->next;
    }
    if (ptr == NULL)
    {
        fprintf(stderr, "error at line %d of file \"%s\": pointer not returned by malloc\n", line_number, file_name);
        return;
    }

    if (((MemCtrl *)target)->free == 1)
    {
        fprintf(stderr, "error at line %d of file \"%s\": pointer has been freed before\n", line_number, file_name);
        return;
    }

    ((MemCtrl *)target)->free = 1;
    printf("free succeed\n");

    ptr = ((MemCtrl *)target)->prev;
    if (ptr != NULL)
    {
        if (ptr->free == 1)
        {
            ptr->size += sizeof(MemCtrl) + ptr->next->size;
            ptr->next = ptr->next->next;
        }
    }
}
