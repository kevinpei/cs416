// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include "my_pthread_t.h"


//A function to add a given thread node to the end of the given running queue
int add_to_run_queue(int num, thread_node* node) {
//	If there are no running threads in the given run queue, make the thread the beginning of the queue
	thread_node* ptr;
	if (num == 1) {
		if (scheduler->first_running_queue == NULL) {
			scheduler->first_running_queue = node;
			return 0;
		}
		ptr = scheduler->first_running_queue;
	}
	if (num == 2) {
		if (scheduler->second_running_queue == NULL) {
			scheduler->second_running_queue = node;
			return 0;
		}
		ptr = scheduler->second_running_queue;
	}
	if (num == 3) {
		if (scheduler->third_running_queue == NULL) {
			scheduler->third_running_queue = node;
			return 0;
		}
		ptr = scheduler->third_running_queue;
	}
//	Iterate through the run queue and stop when we reach a NULL value	
	while (ptr->next != NULL) {
		ptr = ptr->next;
	}
//	Add the thread to the end of the run queue.
	ptr->next = node;
	return 0;
}

//A function to add a given thread node to the end of the waiting queue
int add_to_wait_queue(waiting_queue_node* node) {
//	If the wait queue is empty, then set the wait queue to be the thread
	if (scheduler->waiting_queue == NULL) {
		scheduler->waiting_queue = node;
		return 0;
	}
	waiting_queue_node* ptr = scheduler->waiting_queue;
//	Iterate through the queue and stop when we reach a NULL value	
	while (ptr->next != NULL) {
		ptr = ptr->next;
	}
//	Add the node to the end of the wait queue
	ptr->next = node;
	return 0;
}

//A function to return the queue number with the highest priority
//If there is a tie, then the higher priority queue (e.g. first over second) is run
int get_highest_priority() {
//	If no queue has any elements, return 0
	int highest_priority = 0;
	int highest_priority_queue = 0;
//	If the first queue isn't empty, then it is the highest so far
	if (scheduler->first_running_queue != NULL) {
		highest_priority = scheduler->first_running_queue->thread->priority;
		highest_priority_queue = 1;
	}
//	Compare the priority of the first element in the second queue
	if (scheduler->second_running_queue != NULL) {
		if (scheduler->second_running_queue->thread->priority > highest_priority) {
			highest_priority = scheduler->second_running_queue->thread->priority;
			highest_priority_queue = 2;
		}
	}
//	Compare the priority of the first element in the third queue
	if (scheduler->third_running_queue != NULL) {
		if (scheduler->third_running_queue->thread->priority > highest_priority) {
			highest_priority = scheduler->third_running_queue->thread->priority;
			highest_priority_queue = 3;
		}
	}
//	Return the highest priority queue number
	return highest_priority_queue;
}

//Swaps contexts between the current thread and the thread with the highest priority
int swap_contexts() {
//	If the scheduler is already running, don't do anything
	if (scheduler_running == 1) {
		return 0;
	}
	scheduler_running = 1;
//	If another function is modifying the queue, wait for it to finish before working
	if (modifying_queue == 1) {
		timer->it_interval.tv_usec = 1000;
		return 0;
	}
	modifying_queue = 1;
	thread_node* ptr;
//	Depending on which run queue was running, change the priority of the current thread
	switch(scheduler->current_queue_number) {
//		If a thread in the first run queue was running, move it to the second run queue and set its priority to 50.
		case 1: 
		ptr = scheduler->first_running_queue;
		scheduler->first_running_queue = ptr->next;
		ptr->thread->priority = 50;
		add_to_run_queue(2, ptr);
		break;
//		If a thread in the second run queue was running, move it to the third run queue and set its priority to 0.
		case 2: 
		ptr = scheduler->second_running_queue;
		scheduler->second_running_queue = ptr->next;
		ptr->thread->priority = 0;
		add_to_run_queue(3, ptr);
		break;
//		If a thread in the third run queue was running, then it must be finished, because all threads there run to completion.
		case 3: 
		ptr = scheduler->third_running_queue;
		scheduler->third_running_queue = ptr->next;
		break;
//		If none of the above, then something went wrong.
		default: 
		scheduler_running = 0;
		return -1;
	}
//	Depending on which queue has the highest first priority, switch the context to run that thread
	switch (get_highest_priority()) {
//		If there are no more threads, then do nothing.
		case 0:
		return 0;
//		If the first queue has the highest priority thread, switch to that one.
		case 1:
		scheduler->current_queue_number = 1;
		timer->it_interval.tv_usec = 25000;
		swapcontext(&(ptr->thread->context), &(scheduler->first_running_queue->thread->context));
		break;
//		If the second queue has the highest priority thread, switch to that one.
		case 2:
		scheduler->current_queue_number = 2;
		timer->it_interval.tv_usec = 50000;
		swapcontext(&(ptr->thread->context), &(scheduler->second_running_queue->thread->context));
		break;
//		If the third queue has the highest priority thread, switch to that one.
		case 3:
		scheduler->current_queue_number = 3;
		swapcontext(&(ptr->thread->context), &(scheduler->third_running_queue->thread->context));
		break;
//		If none of the above, then something went wrong.
		scheduler_running = 0;
		default: return -1;
	}'
	scheduler_running = 0;
	return 0;
}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
//	Malloc some space and create a new thread
	thread_node* new_thread = malloc(sizeof(thread_node));
	new_thread->thread = malloc(sizeof(my_pthread));
	getcontext(&(new_thread->thread->context));
	
//	Set this linkt to be the swap contexts function
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
//	Initiate the thread to have priority level 1, where it runs for 25 ms.
	new_thread->thread->priority = 100;
//	Create a new timer and set an alarm for every 25 ms
	if (timer == NULL) {
		timer = malloc(sizeof(struct itimerval));
		timer->it_interval.tv_usec = 25000;
		setitimer(ITIMER_VIRTUAL, timer, NULL);
//		Set the signal handler to be the execute function
		signal (SIGVTALRM, swap_contexts);
	}
//	If the scheduler hasn't been initialized yet, initialize it
	if (scheduler == NULL) {
		scheduler = malloc(sizeof(tcb));
	}
//	Add the thread to the end of the first run queue. Priority is based on position in the queue.
	add_to_run_queue(1, new_thread);
	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	scheduler_running = 1;
	swap_contexts();
	timer->it_value.tv_usec = 25000;
	scheduler_running = 0;
	return 0;
}

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	//Same as yield, except end the thread
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	scheduler_running = 1;
	my_pthread_yield();
	
	//Wait for the other thread to finish executing
	int finished_executing = 0;
	while (finished_executing == 0) {
		if (scheduler->running_queue->thread->pid == thread) {
			my_pthread_exit(*value_ptr);
			return 0;
		}
	}
	timer->it_value.tv_usec = 25000;
	scheduler_running = 0;
	return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	scheduler_running = 1;
//	Set the initial lock to be open
	mutex->mutex_lock = 0;
//	Set a new mutex id
	mutex->mid = mutex_id;
	mutex_id++;
	scheduler_running = 0;
	return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	scheduler_running = 1;
//	All mutexes are initialized with positive id's - if it's negative, then it's been destroyed. 
	if (mutex->mid < 0) {
		return -1;
	}
//	If the mutex is unlocked, then acquire it
	if (mutex->mutex_lock == 0) {
		mutex->mutex_lock = 1;
		mutex->pid = scheduler->running_queue->thread->pid;
//	Otherwise, move to the wait queue
	} else {
		waiting_queue_node* new_node = malloc(sizeof(waiting_queue_node));
//		Create a new node with a thread equal to the currently running thread
		new_node->thread = scheduler->running_queue->thread;
//		Remove the current thread from the run queue
		scheduler->running_queue = scheduler->running_queue->next;
//		Set the mutex id the thread is waiting for
		new_node->mutex_lock = mutex->mid;
//		Add the thread to the end of the wait queue
		add_to_wait_queue(new_node);
//		Swap contexts
		swap_contexts();
	}
	scheduler_running = 0;
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	scheduler_running = 1;
//	All mutexes are initialized with positive id's - if it's negative, then it's been destroyed. 
	if (mutex->mid < 0) {
		return -1;
	}
//	If the current thread is holding the lock, then release it
	if (scheduler->running_queue->thread->pid == mutex->pid) {
		mutex->mutex_lock = 0;
		mutex->pid = -1;
//		Remove all nodes from the wait queue that were waiting for this mutex
		waiting_queue_node* ptr = scheduler->waiting_queue;
		waiting_queue_node* prev = NULL;
		while (ptr != NULL) {
			if (ptr->mutex_lock == mutex->mid) {
				if (prev == NULL) {
					scheduler->waiting_queue = ptr->next;
				} else {
					prev->next = ptr;
				}
//				Add any nodes that were removed from the wait queue to the end of the run queue
				thread_node* new_node = malloc(sizeof(thread_node));
				new_node->thread = ptr->thread;
				add_to_run_queue(new_node);	
			}
			prev = ptr;
			ptr = ptr->next;
		}
	}
	scheduler_running = 0;
	return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	scheduler_running = 1;
//	Wait for the mutex lock to be released before destroying it
	if (mutex->mutex_lock == 0) {
//		Set the id to be negative. This means it's not usable.
		mutex->mid = -1;
	}
	scheduler_running = 0;
	return 0;
};

