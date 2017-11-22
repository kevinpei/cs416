#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "mymalloc.h"

static boolean memInit = FALSE;

// Big block of memory that represents main memory.
static char memoryblock[memorySize];
int pageSize, metaSize;
int pageNumber;
int mainMemAllocated;

mainMemAllocated = 0;
swap1Allocated = 0;
swap2Allocated = 0;

/*
This function initializes main memory by creating as many thread pages as will fit in main memory.
Each thread pages has free size equal to the page size minus the size of the metadata.
Each of these thread pages begins completely free.
*/
boolean initialize()
{
	printf("initialize() from mymalloc\n");
	pageSize = (_SC_PAGE_SIZE);
	metaSize = sizeof(PageData);
	//Calculates the max number of thread pages that can be stored in main memory.
	pageNumber = memorySize / (pageSize + metaSize);
	int x = 0;
	int swapfile = open("./swapfile.txt", O_CREAT | O_RDWR);
	//Create the initial swap file by filling it with pagedata followed immediately by data
	while (x < pageNumber * 2)
	{
		PageData *threadPage = (PageData *)((char *)memoryblock);
		// The size of the memory that is available left for use is this size
		threadPage->pageStart->size = pageSize;
		threadPage->pageStart->isFree = TRUE;
		threadPage->pageStart->next = NULL;
		threadPage->pageStart->prev = NULL;
		// A pid of -1 means that it isn't being used right now by any thread
		threadPage->pid = -1;
		threadPage->next = NULL;
		threadPage->isContinuous = FALSE;
		write(swapfile, memoryblock, pageSize + metaSize);
		x++;
	}
	close(swapfile);
	x = 0;
	// Creates a representation of each thread page as a struct
	while (x < pageNumber)
	{
		PageData *threadPage = (PageData *)((char *)memoryblock + x * metaSize); //put all metadata in the front of the memoryblock
		// The size of the memory that is available left for use is this size
		threadPage->pageStart = (MemoryData *)((char *)memoryblock + pageNumber * metaSize + x * pageSize); //in each metadata it stores where the addr of the real memory block
		threadPage->pageStart->size = pageSize;
		threadPage->pageStart->isFree = TRUE;
		threadPage->pageStart->next = NULL;
		threadPage->pageStart->prev = NULL;
		// A pid of -1 means that it isn't being used right now by any thread
		threadPage->pid = -1;
		threadPage->next = NULL;
		threadPage->isContinuous = FALSE;
		x++;
	}
	x = 0;

	return TRUE;
}

//A function to find the memory page with the given pid.
//You can use pid = -1 to find the first free page, since pages are initialized with pid -1.
PageData *findPage(int pid)
{
	int x = 0;
	//Iterate through the array of thread pages until one with given pid is found.
	while (x < pageNumber)
	{
		//Ignore any pages that are continuous - those won't have valid metadata and are being used by another thread
		if (((PageData *)((char *)memoryblock + x * pageSize))->isContinuous == FALSE)
		{
			if (((PageData *)((char *)memoryblock + x * pageSize))->pid == pid)
			{
				//Return the address of the metadata of the page
				return (PageData *)((char *)memoryblock + x * pageSize);
			}
		}
		x++;
	}
	//If a page with the given pid doesn't exist, return NULL.
	return NULL;
}

//A function to find a page in the swapfile with this pid. Needs the pid and the starting address
//of the pageData corresponding to the page being swapped out.
PageData *findSwapPage(int pid, int pageData)
{
	int x = 0;
	//Store each pagedata and data
	char threadblock[pageSize + sizeof(PageData)];
	int swapfile = open("./swapfile.txt", O_RDWR);
	//Iterate through the array of thread pages in the swapfile util one with the given pid is found
	while (x < pageNumber * 2)
	{
		read(swapfile, threadblock, sizeof(PageData) + pageSize);
		//Page must be non-continuous
		if (((PageData *)threadblock)->isContinuous == FALSE)
		{
			//If we find a page in the swap file with the right pid
			if (((PageData *)threadblock)->pid == pid)
			{
				lseek(swapfile, x * (pageSize + sizeof(PageData)), SEEK_SET);
				//Copy the stuff in the swap file into the temp
				char temp[pageSize + sizeof(pageData)];
				read(swapfile, (void *)temp, pageSize + sizeof(pageData));
				lseek(swapfile, x * (pageSize + sizeof(PageData)), SEEK_SET);
				//Write the pagedata from the swap file to the appropriate page
				write(swapfile, (char *)memoryblock + pageData, sizeof(PageData));
				//Write the data from the swap file to the current page of the pagedata that was overwritten
				write(swapfile, ((PageData *)((char *)memoryblock + pageData))->currentPage, pageSize);
				//Return the pagedata corresponding to the data that was swapped in
				close(swapfile);
				return (PageData *)((char *)memoryblock + pageData);
			}
		}
		x++;
		lseek(swapfile, x * (pageSize + sizeof(PageData)), SEEK_SET);
	}
	close(swapfile);
	return NULL;
}

/*
This function finds the first free memory block starting at the given MemoryData* pointer.
It iterates through the MemoryData* linked list until it finds a free block.
*/
MemoryData *findFirstFree(int size, MemoryData *start)
{
	MemoryData *ptr = start;
	//Iterate through the memory blocks until you find a block that's both free and can fit in the memory we want to malloc, plus its metadata
	while (ptr != NULL)
	{
		if (ptr->isFree == TRUE && ptr->size >= size)
		{
			return ptr;
		}
		ptr = ptr->next;
	}
	// If not memory block is both free and large enough to hold the given malloced amount, return NULL.
	return NULL;
}

/*
This function swaps the contents of two pages. Used to make thread pages contiguous in memory when a thread has multiple pages.
*/
void swapPages(int firstStartAddress, int secondStartAddress)
{
	char tempArray[pageSize];
	int i = 0;
	while (i < pageSize)
	{
		tempArray[i] = memoryblock[firstStartAddress + i];
		memoryblock[firstStartAddress + i] = memoryblock[secondStartAddress + i];
		memoryblock[secondStartAddress + i] = tempArray[i];
		i++;
	}
}

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
		if (&(*(((PageData *)((char *)memoryblock + x * sizeof(PageData)))->currentPage)) == &(*address))
		{
			return (PageData *)((char *)memoryblock + x * sizeof(PageData));
		}
		x++;
	}
	return NULL;
}

/*
A function to return all pages to their starting positions. Called whenever a thread is switched out.
*/
void resetPages()
{
	int x = 0;

	//Iterate through all pages
	while (x < pageNumber)
	{
		PageData *page = (PageData *)((char *)memoryblock + x * sizeof(PageData));
		//If the page isn't in the position it started in, then swap it with the page that is
		if (page->pageStart != page->currentPage)
		{
			int temp = page->currentPage;
			swapPages(page->pageStart, page->currentPage);
			page->currentPage = page->pageStart;
			getPageFromAddress(page->currentPage)->currentPage = temp;
		}
		x++;
	}
}

/*
A function to set all pages of the given thread to the beginning of memory. Called whenever a thread is swapped in.
*/
void setPagesAtFront(int pid)
{
	int x = 0;
	while (x < pageNumber)
	{
		PageData *page = (PageData *)((char *)memoryblock + x * sizeof(PageData));
		if (page->pid == pid)
		{
			int i = 0;
			//Iterate through all pages owned by a thread and put them successively at the front of the pages.
			while (page->next != NULL)
			{
				PageData *ithPage = (PageData *)((char *)memoryblock + i * sizeof(PageData));
				swapPages(ithPage->pageStart, page->pageStart);
				ithPage->currentPage = page->pageStart;
				page->currentPage = ithPage->pageStart;
				page = page->next;
				i++;
			}
			//Protect the metadata at the beginning
			mprotect(memoryblock, pageNumber * sizeof(PageData), PROT_READ | PROT_WRITE);
			//Protect all memory beyond the continuous memory set aside
			mprotect(memoryblock + pageNumber * sizeof(PageData) + i * pageSize, (pageNumber - i) * pageSize, PROT_READ | PROT_WRITE);
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

	//Can't allocate more memory than the size of main memory
	if (size > pageNumber * pageSize)
	{
		return NULL;
	}

	PageData *threadPage;
	MemoryData *firstFreeAddress;
	int pid = get_current_thread()->thread->pid;

	//If the attempted allocated size is 0 or negative, print an error message and return NULL.
	if (size <= 0)
	{
		printf("You have attempted to allocate a non-positive number bytes in File: '%s' Line: '%d'\n", myfile, line);
		return NULL;
	}

	// If memory hasn't been initialized yet, then initialize it. Otherwise, call findFirstFree.
	if (memInit == FALSE)
	{
		initialize();
		//If memory has just been initialized, the first free thread page will be the first one.
		threadPage = (PageData *)memoryblock;
		threadPage->pid = pid;
		memInit = TRUE;
	}
	else
	{
		threadPage = findPage(pid);
		//If there is no page with the given pid, then find the first free thread page (pid -1)
		if (threadPage == NULL)
		{
			threadPage = findPage(-1);
		}
		//If there is no page with pid -1, meaning there are no free pages, then return NULL; there is no space left
		if (threadPage == NULL)
		{
			//Placeholder. Will replace the 30th pagedata with swap file page.
			threadPage = findSwapPage(pid, sizeof(PageData) * 30);
		}
		//If there is no swap page with the given pid, then find the first free swap file thread page (pid -1)
		if (threadPage == NULL)
		{
			threadPage = findSwapPage(-1, sizeof(PageData) * 30);
		}
		//If there are no empty swap page pagedatas, then memory is full
		if (threadPage == NULL)
		{
			return NULL;
		}
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
			while (firstFreeAddress->size < size + sizeof(MemoryData))
			{
				PageData *emptyPage;
				emptyPage = findPage(-1);
				//If there are no empty pages, then check the swap file
				if (emptyPage == NULL)
				{
					emptyPage = findSwapPage(-1, sizeof(PageData) * 30);
				}
				//If there are no empty pages, then there is not enough memory left. Return null.
				if (emptyPage == NULL)
				{
					return NULL;
				}
				//The page is now continuous with another one and the metadata can be overwritten. Additionally, other threads can't use this page.
				emptyPage->isContinuous = 1;
				PageData *ptr = threadPage;
				//Add the new page to the end of the linked list of continuous pages started by this thread
				while (ptr != NULL)
				{
					ptr = ptr->next;
				}
				ptr->next = emptyPage;
				//Swap the page after the last page in the thread and the empty page, then update their locations in the metadata
				swapPages(ptr->pageStart + pageSize, emptyPage->pageStart);
				getPageFromAddress(ptr->pageStart + pageSize)->currentPage = emptyPage->pageStart;
				emptyPage->currentPage = ptr->pageStart + pageSize;
				firstFreeAddress->size += pageSize;
			}
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
		printf("There is not enough space in memory in order to allocated the amount requested in File: '%s' Line: '%d'\n", myfile, line);
		return NULL;
	}
}

void mydeallocate(void *mementry, char *myfile, int line, int req)
{

	// We start the pointer at mainMemory, which is the start of the char array.
	int pid = get_current_thread()->thread->pid;

	PageData *threadPage = findPage(pid);
	//Check the swap file for the pid
	if (threadPage == NULL)
	{
		threadPage = findSwapPage(pid, sizeof(PageData) * 30);
	}
	//No page exists with that pid. Trying to free non-malloced memory. Seg fault.
	if (threadPage == NULL)
	{
		return;
	}
	MemoryData *ptr = threadPage->pageStart;
	// Goes through the linked list of memory blocks until it reaches one whose address matches the address of the freed variable
	while (ptr != NULL)
	{

		if (mementry - sizeof(MemoryData) == ptr && ptr->isFree == FALSE)
		{
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

			// After checking to make sure all adjacent memory blocks are merged, set the block's isFree to TRUE.
			ptr->isFree = TRUE;
			//If the free size is greater than or equal to page size minus the metadata size, this means that the last page is no longer storing anything
			//Make the thread page free for another thread to store in. Continue until all pages that are empty are freed for other threads.
			while (ptr->size >= pageSize - sizeof(MemoryData))
			{
				PageData *pageptr = threadPage;
				PageData *pageprev = NULL;
				while (pageptr->next != NULL)
				{
					pageprev = pageptr;
					pageptr = pageptr->next;
				}
				pageptr->isContinuous = 0;
				pageptr->pid = -1;
				pageprev->next = NULL;
				ptr->size -= pageSize;
			}
			return;
		}
		// Iterate through the linked list of memory blocks.
		ptr = ptr->next;
	}
	// If there is no memory block with a matching address, then no such variable was ever malloced.
	printf("No such variable has been allocated in File: '%s' Line: '%d'\n", myfile, line);
	return;
}
