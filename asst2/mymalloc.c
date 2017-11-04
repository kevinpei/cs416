#include "mymalloc.h"

static boolean memInit = FALSE;

// Big block of memory that represents main memory.
static char memoryblock[memorySize]; 
// Represents the array of pointers to the start of each thread page.
static MemoryData** threadPage; 
static int pageSize = sysconf( _SC_PAGE_SIZE);

/*
This function initializes main memory by creating as many thread pages as will fit in main memory.
Each thread pages has free size equal to the page size minus the size of the metadata.
Each of these thread pages begins completely free.
*/
boolean initialize() {
	//Calculates the max number of thread pages that can be stored in main memory.
	int pageNumber = memorySize/pageSize;
	int x = 0;
	// Creates a representation of each thread page as a struct
	while (x < pageNumber) {
		*(threadPage + x) = (MemoryData *)((char)memoryblock + x * pageSize);
		// The size of the memory that is available left for use is this size   
		*(threadPage + x)->size = pageSize - sizeof(MemoryData); 
		*(threadPage + x)->isFree = TRUE;
		*(threadPage + x)->next = NULL;
		*(threadPage + x)->prev = NULL;
		// A pid of -1 means that it isn't being used right now by any thread
		*(threadPage + x)->pid = -1;
	}
	return TRUE;
}

/*
This function finds the first free memory block starting at the given MemoryData* pointer.
It iterates through the MemoryData* linked list until it finds a free block.
*/
MemoryData* findFirstFree(int size, MemoryData * start) {
	MemoryData * ptr = start;
	//Iterate through the memory blocks until you find a block that's both free and can fit in the memory we want to malloc, plus its metadata
	while ( ptr != NULL) {
		if (ptr->isFree == TRUE && ptr->size >= size) {
			return ptr;
		}
		ptr = ptr->next;
	}
	// If not memory block is both free and large enough to hold the given malloced amount, return NULL.
	return NULL;
}

/*
This function is a custom malloc function that takes an int size as an input and returns a void * pointer to 
the start of an empty memory block.
*/
void * mymalloc(int size, char* myfile, int line, int req) {
	
	MemoryData* firstFreeAddress; 
	
	//If the attempted allocated size is 0 or negative, print an error message and return NULL.
	if(size <= 0) { 
		printf("You have attempted to allocate a non-positive number bytes in File: '%s' Line: '%d'\n", myfile, line); 
		return NULL;
	}	

	// If memory hasn't been initialized yet, then initialize it. Otherwise, call findFirstFree.
	if(memInit == FALSE) {
		initialize(); 
		firstFreeAddress = mainMemory;
		memInit = TRUE;
	} else {	
		firstFreeAddress = findFirstFree(size,mainMemory);
	}
		
	// This means that we have enough space in "main memory" to allocate
	if(firstFreeAddress != NULL) {  

		/*
		If next is null, and there's enough space for another metadata after the first free address, create another free
		memory block after the first free one.
		The first free memory address will hold the data, while the new free memory block will be free.
		*/
		if(firstFreeAddress->next == NULL && firstFreeAddress->size > size + sizeof(MemoryData)) {	
			MemoryData* newFree = (MemoryData *)((char *)firstFreeAddress + sizeof(MemoryData) + size); // Metadata keeps track of the free block.
			newFree->size = firstFreeAddress->size - sizeof(MemoryData) - size; //This keeps track of how much memory is free.
			newFree->isFree = TRUE; 
			// Insert a new free memory block between the firstFreeAddress, which is now being malloced, and its next memory block, which is NULL in this case.
			newFree->next = firstFreeAddress->next;	
			newFree->prev = firstFreeAddress;
			firstFreeAddress->next = newFree;		
		/*
		If next is not null, then we need to check to make sure there's enough space beween the two memory blocks to create another metadata.
		If not, then we can't create another free memory block between the two. If there is enough spacec, then we create newFree.
		*/
		} else if(firstFreeAddress->size - size > sizeof(MemoryData) && firstFreeAddress->next != NULL) { 
			MemoryData* newFree = (MemoryData *)((char *) firstFreeAddress + sizeof(MemoryData) + size);
			newFree->size = firstFreeAddress->size - sizeof(MemoryData) - size;
			newFree->next = firstFreeAddress->next;
			if(newFree->next != NULL) {
				newFree->next->prev = newFree;
			}
			newFree->prev = firstFreeAddress;
			newFree->isFree = TRUE;
			firstFreeAddress->next = newFree;
		}
		// Regardless of whether a new free memory block is created, set the size of firstFreeAddress and set it to not free.
		firstFreeAddress->size = size;
		firstFreeAddress->isFree = FALSE;
		// Return the address of the data after the metadata.
		return (char*)firstFreeAddress + sizeof(MemoryData);
	} else {
		// If firstFindFree returned NULL, then there wasn't enough memory.
		printf("There is not enough space in memory in order to allocated the amount requested in File: '%s' Line: '%d'\n", myfile, line);
		return NULL;
	}				 
}

void myfree(void * mementry, char * myfile, int line) {
	
	// We start the pointer at mainMemory, which is the start of the char array.
	
	MemoryData* ptr = mainMemory;
	
	// Goes through the linked list of memory blocks until it reaches one whose address matches the address of the freed variable
	while (ptr != NULL) {

		if (mementry - sizeof(MemoryData) == ptr && ptr->isFree == FALSE) {
			/* 
			This code will also merge adjacent free memory blocks, so it checks to see if the next memory block is NULL or not.
			We do not need to iterate through a while loop because this check will take place after every free, ensuring that every
			single adjacent free memory block will be merged, preventing future adjacent free memory blocks.
			*/

			if (ptr->prev != NULL) {
				/*
				If the previous memory block is free, then we need to remove the current memory block and merge it with the previous one.
				Because metadata comes before data in our code, we move all of the current memory block's metadata to the previous memory block.
				*/
				if (ptr->prev->isFree == TRUE) {
					ptr->prev->size = ptr->size + (char *)ptr - (char*)ptr->prev;
					ptr = ptr->prev;
					ptr->next = ptr->next->next;
					if (ptr->prev != NULL) {
						ptr->prev->next = ptr;
					}
				} else {
					ptr->prev->next = ptr;
				}
			}
			
			if (ptr->next != NULL) {
				/*
				If the next memory block is free, then set the size of the current memory block to its own size plus the size of the
				adjacent memory block, which will be its data size plus its metadata size. Also set the current memory block's next
				pointer to the next pointer of the merged memory block.
				*/
				if (ptr->next->isFree == TRUE) {
					ptr->size = ptr->next->size + (char *)ptr->next - (char*)ptr;
					ptr->next = ptr->next->next;
					if (ptr->next != NULL) {
						ptr->next->prev = ptr;
					}
				} else {
					ptr->next->prev = ptr;
				}
			}
			
			// After checking to make sure all adjacent memory blocks are merged, set the block's isFree to TRUE.
			ptr->isFree = TRUE;
			return;
		}
		// Iterate through the linked list of memory blocks.
		ptr = ptr->next;
	}
	// If there is no memory block with a matching address, then no such variable was ever malloced.
	printf("No such variable has been allocated in File: '%s' Line: '%d'\n", myfile, line);
	return;
}