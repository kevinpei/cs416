// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include "my_pthread_t.h"

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	
//	Iterate through the running queue and stop when we reach a NULL value
	my_pthread* ptr = scheduler->running_queue;
	while (ptr != NULL) {
		&ptr += sizeof(my_pthread);
	}
	
//	Malloc some space and create a new thread at the end of the running queue.
	ptr = malloc(sizeof(my_pthread));
	getcontext(&(ptr->context));
	ptr->context.uc_link=0;
	
//	Which signals do we want to block?
//	ptr->context.uc_sigmask = 

//	Initializes a stack for the new thread with size 64000 bytes
	ptr->context.uc_stack.ss_sp=malloc(64000);
	ptr->context.uc_stack.ss_size=64000;
	ptr->context.uc_stack.ss_flags=0;
	
//	Sets the pid of the new thread to be the first argument given
	ptr->pid = *thread;
	
//	Make a new context. We assume the function has 0 arguments.
	makecontext(&(ptr.context), function, 0);
	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	return 0;
};

