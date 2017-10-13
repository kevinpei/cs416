// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include "my_pthread_t.h"


//A function to add a given thread node to the end of the running queue
int add_to_run_queue(thread_node* node) {
//	If there are no running threads, make the thread the beginning of the queue
	if (scheduler->running_queue == NULL) {
		scheduler->running_queue = node;
		return 0;
	}
	thread_node* ptr = scheduler->running_queue;
//	Iterate through the queue and stop when we reach a NULL value	
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

//Swaps contexts between the current thread and the next thread in the queue
int swap_contexts() {
//	if there are no running threads, then nothing happens
	if (scheduler->running_queue != NULL) {
//		The next thread should be the next one in the run queue
		thread_node* next_pthread = scheduler->running_queue->next;
//		If there is a next thread, then swap to it; otherwise, do nothing
		if (next_pthread != NULL) {
			swapcontext(&(next_pthread->thread->context), &(scheduler->running_queue->thread->context));
		}
	}
	return 0;
}

// The signal handler that handles the signal when the itimer reaches 0
int execute() {
//	Don't do anything if the scheduler is already running.
	if (scheduler_running == 1) {
		return 0;
	}
	scheduler_running = 1;
//	If the priority level is 1, then it only runs for 25 ms before switching
	if (scheduler->running_queue->thread->priority_level == 1) {
		scheduler->running_queue->thread->priority_level = 2;
		//Swap contexts
		swap_contexts();
//	If the priority level is 2, then it runs for 50 ms before switching
	} else if (scheduler->running_queue->thread->priority_level == 2) {
//		Let it continue running
		if (execution_time == 0) {
			execution_time += 1;
//		If it's already run for 25 ms, swap
		} else {
			execution_time = 0;
			swap_contexts();
		}
	}
//	If the priority level is 3, then do nothing - wait until it finishes running
	scheduler_running = 0;
	return 0;
}


/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
//	Scheduler is running, so don't swap contexts
	scheduler_running = 1;
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
	new_thread->thread->priority_level = 1;
//	Create a new timer and ge tthe current timer value for the real time timer
	struct itimerval* timer = malloc(sizeof(struct itimerval));
	getitimer(ITIMER_REAL, timer);
//	If a signal is not being sent every 25 milliseconds, then set a new timer
	if (timer->it_interval != 25) {
		timer->it_interval = 25;
		timer->it_value = 25;
		setitimer(ITIMER_REAL, timer, NULL);
//		Set the signal handler to be the execute function
		signal (SIGALRM, execute);
	}
//	If the scheduler hasn't been initialized yet, initialize it
	if (scheduler == NULL) {
		scheduler = malloc(sizeof(tcb));
	}
//	Add the thread to the end of the run queue. Priority is based on position in the queue.
	add_to_run_queue(new_thread);
//	Scheduler isn't running anymore.
	scheduler_running = 0;
	return 0;
};

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	scheduler_running = 1;
	swap_contexts();
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

