// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include "my_pthread_t.h"

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
	
//	Malloc some space and create a new thread
	thread_node* new_thread = malloc(sizeof(thread_node));
	new_thread->thread = malloc(sizeof(my_pthread));
	getcontext(&(new_thread->thread->context));
	new_thread->thread->context.uc_link = 0;
	
//	Which signals do we want to block?
//	ptr->context.uc_sigmask = 

//	Initializes a stack for the new thread with size 64000 bytes
	new_thread->thread->context.uc_stack.ss_sp=malloc(64000);
	new_thread->thread->context.uc_stack.ss_size=64000;
	new_thread->thread->context.uc_stack.ss_flags=0;
	
//	Sets the pid of the new thread to be the first argument given
	new_thread->thread->pid = *thread;
	
//	Make a new context. We assume the function has 0 arguments.
	makecontext(&(new_thread->thread->context), function, 0);
//	Initiate the thread to have priority level 1 and priority of 1
	new_thread->thread->priority_level = 1;
	new_thread->thread->priority = 1;
//	Create a new timer and ge tthe current timer value for the real time timer
	struct itimerval* timer = malloc(sizeof(struct itimerval));
	getitimer(ITIMER_REAL, timer);
//	If a signal is not being sent every 25 milliseconds, then set a new timer
	if (timer->it_interval != 25) {
		timer->it_interval = 25;
		timer->it_value = 25;
		setitimer(ITIMER_REAL, timer, NULL);
		signal (SIGALRM, execute);
	}
	if (scheduler->current_thread == NULL) {
		scheduler->current_thread = malloc(sizeof(my_pthread));
		scheduler->current_thread = new_thread->thread;
	} else {
		
	}
	
	add_to_queue(0, new_thread);
	
	return 0;
};

//A function to add a given thread node to the end of either the running queue, the waiting queue, or the mutex locks queue
int add_to_queue(int queue_type, thread_node* node) {
	if (queue_type == 0) {
		if (scheduler->running_queue == NULL) {
			scheduler->running_queue = node;
			return 0;
		}
		thread_node* ptr = scheduler->running_queue;
	} else if (queue_type == 1) {
		if (scheduler->waiting_queue == NULL) {
			scheduler->waiting_queue = node;
			return 0;
		}
		thread_node* ptr = scheduler->waiting_queue;
	}
//	Iterate through the queue and stop when we reach a NULL value	
	while (ptr->next != NULL) {
		ptr = ptr->next;
	}
	ptr->next = node;
	return 0;
}

//Returns the pthread with the highest priority after increasing the priorities of every pthread
thread_node* select_next_pthread() {
	thread_node* next_pthread = NULL;
	int highest_priority = 0;
	thread_node* ptr = scheduler->running_queue;
	while (ptr != NULL) {
		ptr->thread->priority += 1;
		if (ptr->thread->priority > highest_priority) {
			next_pthread = ptr;
		}
		ptr = ptr->next;
	}
	return next_pthread;
}s

//Swaps contexts between the current thread and the thread with the highest priority
int swap_contexts() {
	scheduler->current_thread->priority_level = 1;
	thread_node* next_pthread = select_next_pthread;
	swapcontext(&(next_pthread->thread->context), &(scheduler->current_thread->context);
	scheduler->current_thread = next_pthread->thread;
	return 0;
}

// The signal handler that handles the signal when the itimer reaches 0
int execute() {
//	If the priority level is 1, then it only runs for 25 ms before switching
	if (scheduler->current_thread->priority_level == 1) {
		scheduler->current_thread->priority_level = 2;
		//Swap contexts
		swap_contexts();
//	If the priority level is 2, then it runs for 50 ms before switching
	} else if (scheduler->current_thread->priority_level == 2) {
		if (execution_time == 0) {
			execution_time += 1;
			//Let it continue running
		} else {
			execution_time = 0;
			scheduler->current_thread->priority_level = 3;
			//swap contexts
			swap_contexts();
		}
//	If the priority level is 3, then it runs until it finishes
	} else {
		//Keep running until it finishes
	}
	return 0;
}

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	//Get current_thread ID because you can only yield if you're the currently running thread
	//Move from the running queue to the waiting queue
	
	return 0;
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	//Same as yield, except end the thread
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	//Same as yield, just also check to see if the other thread has finished yet
	/*
	my_pthread* ptr = scheduler.running_queue;
	my_pthread* prev = NULL;
	//Remove the thread from the running queue
	while (ptr->thread.pid != scheduler.current_thread && ptr != NULL) {
		prev = ptr;
		ptr = ptr->next;
	}
	if (prev == NULL) {
		scheduler.running_queue = NULL;
	} else {
		prev->next = ptr->next;
	}
	*/
	
	//Remove from the running queue
	my_pthread_yield();
	
	//Wait for the other thread to finish executing
	finished_executing = 0;
	while (finished_executing == 0) {
		my_pthread* ptr = scheduler->running_queue;
		while (ptr->next != NULL) {
			if (ptr->thread->pid == thread) {
				my_pthread_exit(NULL);
			}
		}
	}
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	mutex->mutex_lock = 0;
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	if (mutex->mutex_lock == 0) {
		mutex->mutex_lock = 1;
		mutex->pid = scheduler->current_thread;
	} else {
		add_to_queue(1, scheduler->current_thread);
	}
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	if (scheduler->current_thread->pid == mutex->mutex_lock->pid) {
		mutex->mutex_lock->value = 0;
		mutex->pid = -1;
	}
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	while (1) {
		if (mutex->mutex_lock->value == 0) {
			free(mutex->mutex_lock);
			return 0;
		}
	}
	return 0;
};

