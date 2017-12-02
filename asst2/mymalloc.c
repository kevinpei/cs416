// File:	y_malloc.c
// Author:	Yujie REN
// Date:	09/23/2017

// name: Derek Mao mm2180, Kevin Pei ksp98, Quzhi Li ql88
// username of iLab: ksp98
// iLab Server: ls.cs.rutgers.edu

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "my_pthread_t.h"

static boolean memInit = FALSE;

// Big block of memory that represents main memory.
static char memoryblock[memorySize];
static int pageSize = 4096;
static int metaSize = sizeof(PageData);
int scheduler_memory_ptr = 0;
int pageNumber;
int freePages;
static boolean scheduler_initialized = FALSE;

/*
This function initializes main memory by creating as many thread pages as will fit in main memory.
Each thread pages has free size equal to the page size minus the size of the metadata.
Each of these thread pages begins completely free.
*/
void mymalloc_initialize()
{
	freePages = 0;
	//printf("initialize() from mymalloc\n");
	//Calculates the max number of thread pages that can be stored in main memory.
	pageNumber = memorySize / (pageSize + metaSize);
	int x = 0;/*
	printf("Opening\n");
	int swapfile = open("./swapfile.txt", O_CREAT | O_RDWR);
	//Create the initial swap file by filling it with pagedata followed immediately by data
	printf("Populating swap file\n");
	while (x < pageNumber * 2)
	{
		// printf("Page size is %d\n", pageSize);
		// printf("%d iteration of population\n", x);
		PageData *threadPage = (PageData *)((char *)memoryblock);
		// The size of the memory that is available left for use is this size
		// printf("Setting attributes\n");
		threadPage->pageStart = (MemoryData *)((char *)memoryblock + metaSize); //in each metadata it stores where the addr of the real memory block
		threadPage->pageStart->size = pageSize;
		threadPage->pageStart->isFree = TRUE;
		threadPage->pageStart->next = NULL;
		threadPage->pageStart->prev = NULL;
		// A pid of -1 means that it isn't being used right now by any thread
		threadPage->pid = -1;
		threadPage->page_id = x;
		threadPage->next = NULL;
		threadPage->isContinuous = FALSE;
		write(swapfile, memoryblock, pageSize + metaSize);
		x++;
		freePages++;
	}
	printf("Done populating swap file\n");
	close(swapfile);
	x = 0;*/
	// Creates a representation of each thread page as a struct
	//printf("%d pages\n", pageNumber);
	while (x < pageNumber)
	{
		PageData *threadPage = (PageData *)((char *)memoryblock + x * metaSize); //put all metadata in the front of the memoryblock
		// The size of the memory that is available left for use is this size
		threadPage->pageStart = (MemoryData *)((char *)memoryblock + pageNumber * metaSize + x * pageSize); //in each metadata it stores where the addr of the real memory block
		threadPage->pageStart->size = pageSize - sizeof(MemoryData);
		if (x == pageNumber - 4) {
			threadPage->pageStart->size = 4 * (pageSize - sizeof(MemoryData));
		}
		threadPage->pageStart->isFree = TRUE;
		threadPage->pageStart->next = NULL;
		threadPage->pageStart->prev = NULL;
		// A pid of -1 means that it isn't being used right now by any thread
		threadPage->pid = -1;
		//A pid of -2 means that it's shared malloc space
		if (x > pageNumber - 5) {
			threadPage->pid = -2;
		}
		threadPage->page_id = x;
		threadPage->next = NULL;
		threadPage->isContinuous = FALSE;
		if (x > pageNumber - 4) {
			threadPage->isContinuous = TRUE;
		}
		x++;
		if (x < pageNumber - 4) {
			freePages++;
		}
	}
}

//A function to find the memory page with the given pid.
//You can use pid = -1 to find the first free page, since pages are initialized with pid -1.
PageData *findPage(int pid)
{
	int x = 0;
	//printf("findPage()\n");
	//Iterate through the array of thread pages until one with given pid is found.
	while (x < pageNumber)
	{
		// printf("Iterating through\n");
		//Ignore any pages that are continuous - those won't have valid metadata and are being used by another thread
		if (((PageData *)((char *)memoryblock + x * metaSize))->isContinuous == FALSE)
		{
			// printf("Page is not continuous\n");
			if (((PageData *)((char *)memoryblock + x * metaSize))->pid == pid)
			{
				//Return the address of the metadata of the page
				//printf("pid is: %d, Page found\n", ((PageData *)((char *)memoryblock + x * metaSize))->pid);
				// printf("Page found\n");
				return (PageData *)((char *)memoryblock + x * metaSize);
			}
		}
		x++;
	}
	//If a page with the given pid doesn't exist, return NULL.
	return NULL;
}
/*
//A function to find a page in the swapfile with this pid. Needs the pid and the starting address
//of the pageData corresponding to the page being swapped out.
PageData *findSwapPage(int pid, int pageData)
{
	printf("findSwapPage()");
	int x = 0;
	//Store each pagedata and data
	char threadblock[pageSize + metaSize];
	int swapfile = open("./swapfile.txt", O_RDWR);
	//Iterate through the array of thread pages in the swapfile util one with the given pid is found
	while (x < pageNumber * 2)
	{
		read(swapfile, threadblock, metaSize + pageSize);
		//Page must be non-continuous
		if (((PageData *)threadblock)->isContinuous == FALSE)
		{
			//If we find a page in the swap file with the right pid
			if (((PageData *)threadblock)->pid == pid)
			{
				lseek(swapfile, x * (pageSize + metaSize), SEEK_SET);
				//Copy the stuff in the swap file into the temp
				char temp[pageSize + metaSize];
				read(swapfile, (void *)temp, pageSize + metaSize);
				lseek(swapfile, x * (pageSize + metaSize), SEEK_SET);
				//Write the pagedata from the swap file to the appropriate page
				write(swapfile, (char *)memoryblock + pageData, metaSize);
				//Write the data from the swap file to the current page of the pagedata that was overwritten
				write(swapfile, ((PageData *)((char *)memoryblock + pageData))->pageStart, pageSize);
				//Return the pagedata corresponding to the data that was swapped in
				close(swapfile);
				return (PageData *)((char *)memoryblock + pageData);
			}
		}
		x++;
		lseek(swapfile, x * (pageSize + metaSize), SEEK_SET);
	}
	close(swapfile);
	return NULL;
}
*/
/*
This function finds the first free memory block starting at the given MemoryData* pointer.
It iterates through the MemoryData* linked list until it finds a free block.
*/
MemoryData *findFirstFree(int size, MemoryData *start)
{
	//printf("Free pages: %d\n", freePages);
	MemoryData *ptr = start;
	//Iterate through the memory blocks until you find a block that's both free and can fit in the memory we want to malloc, plus its metadata
	while (ptr != NULL)
	{
		// printf("ptr isn't null\n");
		if (ptr->isFree == TRUE)
		{
			//printf("Pointer is free\n");
			if (ptr->size >= size)
			{
				return ptr;
			}
			else if (size <= freePages * 4096)
			{
				//printf("Enough space in memory pages\n");
				return ptr;
			}
		}
		ptr = ptr->next;
	}
	// If not memory block is both free and large enough to hold the given malloced amount, return NULL.
	return NULL;
}

/*
This function swaps the contents of two pages. Used to make thread pages contiguous in memory when a thread has multiple pages.
*/
void swapPages(PageData *firstPage, PageData *secondPage)
{
	char tempArray[pageSize];
	memcpy(tempArray, firstPage->pageStart, pageSize);
	memcpy(firstPage->pageStart, secondPage->pageStart, pageSize);
	memcpy(secondPage->pageStart, tempArray, pageSize);
}
// void swapPages(PageData *firstPage, PageData *secondPage)
// {
// 	char tempArray[metaSize];
// 	int i = 0;
// 	while (i < metaSize)
// 	{
// 		tempArray[i] = *((char *)firstPage + i);
// 		*((char *)secondPage + i) = *((char *)firstPage + i);
// 		*((char *)secondPage + i) = tempArray[i];
// 		i++;
// 	}
// }

/*
A function to get the pageData that corresponds to the given page starting address.
*/
PageData *getPageFromAddress(MemoryData *address)
{
	int x = 0;
	//Iterate through all pages
	while (x < pageNumber)
	{
		//If the current location of the page is the address given as input, then return the metadata corresponding to that page
		if (((PageData *)((char *)memoryblock + x * metaSize))->pageStart == address)
		{
			//printf("Found the right address\n");
			return (PageData *)((char *)memoryblock + x * metaSize);
		}
		x++;
	}
	return NULL;
}

/*
A function to set all pages of the given thread to the beginning of memory. Called whenever a thread is swapped in.
*/
void setPagesAtFront(int pid)
{
	int x = 0;
	while (x < pageNumber)
	{
		PageData *page = (PageData *)((char *)memoryblock + x * metaSize);
		if (page->pid == pid)
		{
			int i = 0;
			PageData *ithPage = (PageData *)((char *)memoryblock + i * metaSize);
			//Iterate through all pages owned by a thread and put them successively at the front of the pages.
			if (ithPage != page) {
				while (page->next != NULL)
				{
					swapPages(ithPage, page);
					page = ithPage->next;
					i++;
				}
			}
			//Protect the metadata at the beginning
			//mprotect(memoryblock, pageNumber * metaSize, PROT_READ | PROT_WRITE);
			//Protect all memory beyond the continuous memory set aside
			//mprotect(memoryblock + pageNumber * metaSize + i * pageSize, (pageNumber - i) * pageSize, PROT_READ | PROT_WRITE);
			return;
		}
		x++;
	}
}

/*
This function is a custom malloc function that takes an int size as an input and returns a void * pointer to 
the start of an empty memory block. Depending on the currently executing thread, a different memory block may be used.
*/
void *myallocate(int size, char *myfile, int line, int req)
{
	//printf("malloc\n");
	PageData *threadPage;
	MemoryData *firstFreeAddress;
	// If memory hasn't been initialized yet, then initialize it. Otherwise, call findFirstFree.
	if (memInit == FALSE)
	{
		signal(SIGSEGV, segment_fault_handler);
		mymalloc_initialize();
		// //If memory has just been initialized, the first free thread page will be the first one.
		// threadPage = (PageData *)memoryblock;
		// threadPage->pid = pid;
		memInit = TRUE;
		my_pthread_initialize();
		scheduler_initialized = TRUE;
	}
	// int pid = size;
	int pid = 0;
	if (scheduler_initialized)
	{
		pid = get_current_thread()->thread->pid;
	}
	//printf("pid is %d\n", pid);
	
	//printf("Gaha\n");
	//If the attempted allocated size is 0 or negative, print an error message and return NULL.
	if (size <= 0)
	{
		printf("You have attempted to allocate a non-positive number bytes in File: '%s' Line: '%d'\n", myfile, line);
		return NULL;
	}

	//Can't allocate more memory than the size of main memory
	if (size > pageNumber * pageSize)
	{
		return NULL;
	}
	else
	{
		//printf("Looking for a page\n");
		threadPage = findPage(pid);
		//If there is no page with the given pid, then find the first free thread page (pid -1)
		//printf("Page found\n");
		if (threadPage == NULL)
		{
			threadPage = findPage(-1);
		}
		//If there is no page with pid -1, meaning there are no free pages in memory
		/*if (threadPage == NULL)
		{
			printf("Looking in swap file\n");
			//Placeholder. Will replace the 30th pagedata with swap file page.
			threadPage = findSwapPage(pid, metaSize * 30);
		}
		//If there is no swap page with the given pid, then find the first free swap file thread page (pid -1)
		if (threadPage == NULL)
		{
			threadPage = findSwapPage(-1, metaSize * 30);
		}
		//If there are no empty swap page pagedatas, then memory is full*/
		if (threadPage == NULL)
		{
			return NULL;
		}
		//printf("Setting pid\n");
		threadPage->pid = pid;
		//Find the first free address in that thread page
		firstFreeAddress = findFirstFree(size, threadPage->pageStart);
	}

	// This means that we have enough space in "main memory" to allocate
	if (firstFreeAddress != NULL)
	{

		/*
		If next is null, and there's enough space for another metadata after the first free address, create another free
		memory block after the first free one.
		The first free memory address will hold the data, while the new free memory block will be free.
		*/
		if (firstFreeAddress->next == NULL && firstFreeAddress->size > size + sizeof(MemoryData))
		{
			MemoryData *newFree = (MemoryData *)((char *)firstFreeAddress + sizeof(MemoryData) + size); // Metadata keeps track of the free block.
			newFree->size = firstFreeAddress->size - sizeof(MemoryData) - size;							//This keeps track of how much memory is free.
			newFree->isFree = TRUE;
			// Insert a new free memory block between the firstFreeAddress, which is now being malloced, and its next memory block, which is NULL in this case.
			newFree->next = firstFreeAddress->next;
			newFree->prev = firstFreeAddress;
			firstFreeAddress->next = newFree;
			/*
		If next is not null, then we need to check to make sure there's enough space beween the two memory blocks to create another metadata.
		If not, then we can't create another free memory block between the two. If there is enough space, then we create newFree.
		*/
		}
		else if (firstFreeAddress->size - size > sizeof(MemoryData) && firstFreeAddress->next != NULL)
		{
			MemoryData *newFree = (MemoryData *)((char *)firstFreeAddress + sizeof(MemoryData) + size);
			newFree->size = firstFreeAddress->size - sizeof(MemoryData) - size;
			newFree->next = firstFreeAddress->next;
			if (newFree->next != NULL)
			{
				newFree->next->prev = newFree;
			}
			newFree->prev = firstFreeAddress;
			newFree->isFree = TRUE;
			firstFreeAddress->next = newFree;
		}
		/*
		If next is null and there's not enough space for the malloc, then try to allocate another empty page to this process.
		*/
		else if (firstFreeAddress->size < size + sizeof(MemoryData) && firstFreeAddress->next == NULL)
		{
			//printf("Page isn't big enough\n");
			while (firstFreeAddress->size < size + sizeof(MemoryData))
			{
				PageData *emptyPage;
				emptyPage = findPage(-1);
				//If there are no empty pages, then check the swap file
				/*if (emptyPage == NULL)
				{
					printf("Searching swap file\n");
					emptyPage = findSwapPage(-1, metaSize * 30);
				}*/
				//If there are no empty pages, then there is not enough memory left. Return null.
				if (emptyPage == NULL)
				{
					return NULL;
				}
				//The page is now continuous with another one and the metadata can be overwritten. Additionally, other threads can't use this page.
				emptyPage->pid = threadPage->pid;
				emptyPage->isContinuous = TRUE;
				//printf("Set to continuous\n");
				PageData *ptr = threadPage;
				//Add the new page to the end of the linked list of continuous pages started by this thread
				while (ptr->next != NULL)
				{
					ptr = ptr->next;
				}
				//printf("Found end of page chain\n");
				ptr->next = emptyPage;
				//Swap the page after the last page in the thread and the empty page, then update their locations in the metadata
				//printf("About to swap pages\n");
				swapPages(ptr + 1, emptyPage);
				getPageFromAddress((MemoryData *)((char *)(ptr->pageStart) + pageSize))->pageStart = emptyPage->pageStart;
				emptyPage->pageStart = (MemoryData *)((char *)(ptr->pageStart) + pageSize);
				firstFreeAddress->size += pageSize;
			}
			//Check to see if there's room for another metadata
			if (firstFreeAddress->size > sizeof(MemoryData))
			{
				MemoryData *newFree = (MemoryData *)((char *)firstFreeAddress + sizeof(MemoryData) + size);
				newFree->size = firstFreeAddress->size - sizeof(MemoryData) - size;
				newFree->next = firstFreeAddress->next;
				if (newFree->next != NULL)
				{
					newFree->next->prev = newFree;
				}
				newFree->prev = firstFreeAddress;
				newFree->isFree = TRUE;
				firstFreeAddress->next = newFree;
				firstFreeAddress->size = size;
			}
		}
		// Regardless of whether a new free memory block is created, set the size of firstFreeAddress, set it to not free, and set the pid to the current thread.
		firstFreeAddress->size = size;
		firstFreeAddress->isFree = FALSE;
		// Return the address of the data after the metadata.
		freePages -= ((size / 4096) + 1);
		return (char *)firstFreeAddress + sizeof(MemoryData);
	}
	else
	{
		// If firstFindFree returned NULL, then there wasn't enough memory.
		printf("There is not enough space in memory in order to allocated the amount requested in File: '%s' Line: '%d'\n", myfile, line);
		return NULL;
	}
}

void mydeallocate(void *mementry, char *myfile, int line, int req)
{
	//printf("mydealloc()\n");

	if (in_scheduler == TRUE)
	{
		//printf("freeing scheduler\n");
	 	return;
	}
	int pid = get_current_thread()->thread->pid;
	if ((char*)mementry > (char*)((PageData*)memoryblock + (pageNumber - 4))->pageStart) {
		printf("In shalloc\n");
		pid = -2;
	}
	// We start the pointer at mainMemory, which is the start of the char array.
	PageData *threadPage = findPage(pid);
	//printf("ThreadPage has ID %d\n", threadPage->page_id);
	//Check the swap file for the pid
	/*if (threadPage == NULL)
	{
		threadPage = findSwapPage(pid, metaSize * 30);
	}*/
	//No page exists with that pid. Trying to free non-malloced memory. Seg fault.
	if (threadPage == NULL)
	{
		printf("No such variable has been allocated in File: '%s' Line: '%d'\n", myfile, line);
		return;
	}
	MemoryData *ptr = threadPage->pageStart;
	// Goes through the linked list of memory blocks until it reaches one whose address matches the address of the freed variable
	while (ptr != NULL)
	{
		//printf("ptr isn't null\n");
		if ((MemoryData *)((char *)mementry - sizeof(MemoryData)) == ptr && ptr->isFree == FALSE)
		{
			//printf("Found MemoryData\n");
			/* 
			This code will also merge adjacent free memory blocks, so it checks to see if the next memory block is NULL or not.
			We do not need to iterate through a while loop because this check will take place after every free, ensuring that every
			single adjacent free memory block will be merged, preventing future adjacent free memory blocks.
			*/

			if (ptr->prev != NULL)
			{
				/*
				If the previous memory block is free, then we need to remove the current memory block and merge it with the previous one.
				Because metadata comes before data in our code, we move all of the current memory block's metadata to the previous memory block.
				*/
				if (ptr->prev->isFree == TRUE)
				{
					ptr->prev->size = ptr->size + (char *)ptr - (char *)ptr->prev;
					ptr = ptr->prev;
					ptr->next = ptr->next->next;
					if (ptr->prev != NULL)
					{
						ptr->prev->next = ptr;
					}
				}
				else
				{
					ptr->prev->next = ptr;
				}
			}

			if (ptr->next != NULL)
			{
				/*
				If the next memory block is free, then set the size of the current memory block to its own size plus the size of the
				adjacent memory block, which will be its data size plus its metadata size. Also set the current memory block's next
				pointer to the next pointer of the merged memory block.
				*/
				if (ptr->next->isFree == TRUE)
				{
					ptr->size = ptr->next->size + (char *)ptr->next - (char *)ptr;
					ptr->next = ptr->next->next;
					if (ptr->next != NULL)
					{
						ptr->next->prev = ptr;
					}
				}
				else
				{
					ptr->next->prev = ptr;
				}
			}
			//printf("Doing the loops\n");
			// After checking to make sure all adjacent memory blocks are merged, set the block's isFree to TRUE.
			ptr->isFree = TRUE;
			//If the free size is greater than or equal to page size minus the metadata size, this means that the last page is no longer storing anything
			//Make the thread page free for another thread to store in. Continue until all pages that are empty are freed for other threads.
			//Only do it if not in shalloc
			if (pid != -2) {
				while (ptr->size >= pageSize)
				{
					//printf("Lowering page size\n");
					PageData *pageptr = threadPage;
					PageData *pageprev = NULL;
					while (pageptr->next != NULL)
					{
						pageprev = pageptr;
						pageptr = pageptr->next;
					}
					pageptr->isContinuous = FALSE;
					pageptr->pid = -1;
					pageprev->next = NULL;
					ptr->size -= pageSize;
					freePages++;
				}
			}
			
			//printf("Returning after freeing\n");
			return;
		}
		// Iterate through the linked list of memory blocks.
		ptr = ptr->next;
	}
	// If there is no memory block with a matching address, then no such variable was ever malloced.
	printf("No such variable has been allocated in File: '%s' Line: '%d'\n", myfile, line);
	return;
}

void * shalloc(size_t size) {
	PageData *threadPage;
	MemoryData *firstFreeAddress;
	if (memInit == FALSE)
	{
		signal(SIGSEGV, segment_fault_handler);
		mymalloc_initialize();
		// //If memory has just been initialized, the first free thread page will be the first one.
		// threadPage = (PageData *)memoryblock;
		// threadPage->pid = pid;
		memInit = TRUE;
		my_pthread_initialize();
		scheduler_initialized = TRUE;
	}
	if (size <= 0)
	{
		printf("You have attempted to allocate a non-positive number bytes using shalloc.\n");
		return NULL;
	}

	//Can't allocate more memory than the size of shalloc
	if (size > 4 * pageSize)
	{
		return NULL;
	}
	else
	{
		//Look for the shalloc page
		threadPage = findPage(-2);
		if (threadPage == NULL)
		{
			//Something went wrong
			return NULL;
		}
		//Find the first free address in that thread page
		firstFreeAddress = findFirstFree(size, threadPage->pageStart);
	}

	// This means that we have enough space in "main memory" to allocate
	if (firstFreeAddress != NULL)
	{

		/*
		If next is null, and there's enough space for another metadata after the first free address, create another free
		memory block after the first free one.
		The first free memory address will hold the data, while the new free memory block will be free.
		*/
		if (firstFreeAddress->next == NULL && firstFreeAddress->size > size + sizeof(MemoryData))
		{
			MemoryData *newFree = (MemoryData *)((char *)firstFreeAddress + sizeof(MemoryData) + size); // Metadata keeps track of the free block.
			newFree->size = firstFreeAddress->size - sizeof(MemoryData) - size;							//This keeps track of how much memory is free.
			newFree->isFree = TRUE;
			// Insert a new free memory block between the firstFreeAddress, which is now being malloced, and its next memory block, which is NULL in this case.
			newFree->next = firstFreeAddress->next;
			newFree->prev = firstFreeAddress;
			firstFreeAddress->next = newFree;
			/*
		If next is not null, then we need to check to make sure there's enough space beween the two memory blocks to create another metadata.
		If not, then we can't create another free memory block between the two. If there is enough space, then we create newFree.
		*/
		}
		else if (firstFreeAddress->size - size > sizeof(MemoryData) && firstFreeAddress->next != NULL)
		{
			MemoryData *newFree = (MemoryData *)((char *)firstFreeAddress + sizeof(MemoryData) + size);
			newFree->size = firstFreeAddress->size - sizeof(MemoryData) - size;
			newFree->next = firstFreeAddress->next;
			if (newFree->next != NULL)
			{
				newFree->next->prev = newFree;
			}
			newFree->prev = firstFreeAddress;
			newFree->isFree = TRUE;
			firstFreeAddress->next = newFree;
		}
		// Regardless of whether a new free memory block is created, set the size of firstFreeAddress, set it to not free, and set the pid to the current thread.
		firstFreeAddress->size = size;
		firstFreeAddress->isFree = FALSE;
		// Return the address of the data after the metadata.
		return (char *)firstFreeAddress + sizeof(MemoryData);
	}
	else
	{
		// If firstFindFree returned NULL, then there wasn't enough memory.
		printf("There is not enough space in shalloc in order to allocated the amount requested.");
		return NULL;
	}
}

void write_memory_to_file()
{
	int i;
	FILE *memory_file = fopen("memory.txt", "w+");
	fprintf(memory_file, "main memory:\n====================\n");
	for (i = 0; i < pageNumber; i++)
	{
		PageData *ptr = (PageData *)memoryblock + i;
		fprintf(memory_file, "address: %#x, ", ptr);
		fprintf(memory_file, "pageStart: %#x, ", ptr->pageStart);
		fprintf(memory_file, "pid: %d, ", ptr->pid);
		fprintf(memory_file, "page_id: %d, ", ptr->page_id);
		fprintf(memory_file, "isContinuous: %d, ", ptr->isContinuous);
		fprintf(memory_file, "next: %#x\n", ptr->next);
	}
	
	fprintf(memory_file, "\n\nswap file:\n====================\n");
	FILE *swap_file = fopen("swapfile.txt", "r");
	for (i = 0; i < pageNumber * 2; i++)
	{
		char buffer[metaSize];
		fseek(swap_file, i * metaSize, SEEK_SET);
		fgets(buffer, metaSize, swap_file);
		PageData *ptr = (PageData *)buffer;
		fprintf(memory_file, "address: %#x, ", ptr);
		fprintf(memory_file, "pageStart: %#x, ", ptr->pageStart);
		fprintf(memory_file, "pid: %d, ", ptr->pid);
		fprintf(memory_file, "page_id: %d, ", ptr->page_id);
		fprintf(memory_file, "isContinuous: %d, ", ptr->isContinuous);
		fprintf(memory_file, "next: %#x\n", ptr->next);
	}
	fclose(swap_file);
	fclose(memory_file);
}

void segment_fault_handler(int signum)
{
	write_memory_to_file();
	printf("segfault, exit");
	_exit(0);
}
