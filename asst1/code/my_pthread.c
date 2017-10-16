// File:	my_pthread.c
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server:

#include "my_pthread_t.h"

//A function to add a given thread node to the end of the given running queue
int add_to_run_queue(int num, thread_node* node) {
//	If the queue is already being modified, wait for the operation to finish, then continue
	printf("Lock value is %d\n", modifying_queue);
	while (__sync_lock_test_and_set(&modifying_queue, 1) == 1) {
		int placeholder = 0;
    }
//	If there are no running threads in the given run queue, make the thread the beginning of the queue
	thread_node* ptr;
	if (num == 1) {
		if (scheduler->first_running_queue == NULL) {
			scheduler->first_running_queue = node;
			__sync_lock_release(&modifying_queue);
			return 0;
		}
		ptr = scheduler->first_running_queue;
	}
	if (num == 2) {
		if (scheduler->second_running_queue == NULL) {
			scheduler->second_running_queue = node;
			__sync_lock_release(&modifying_queue);
			return 0;
		}
		ptr = scheduler->second_running_queue;
	}
	if (num == 3) {
		if (scheduler->third_running_queue == NULL) {
			scheduler->third_running_queue = node;
			__sync_lock_release(&modifying_queue);
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
	__sync_lock_release(&modifying_queue);
	return 0;
}

//A function to add a given node to the first run queue based on its priority
int add_to_run_queue_priority_based(thread_node* node) {
	while (__sync_lock_test_and_set(&modifying_queue, 1) == 1) {
		int placeholder = 0;
    }
	thread_node* ptr = scheduler->first_running_queue;
	thread_node* prev = NULL;
//	Iterate through the first run queue until you reach the end or a thread with lower priority is found
	while (ptr != NULL) {
		if (ptr->thread->priority < node->thread->priority) {
//			If prev isn't next, insert the node between ptr and prev
			if (prev != NULL) {
				prev->next = node;
				node->next = ptr;
				__sync_lock_release(&modifying_queue);
				return 0;
//			If prev is null, then node must be the beginning of the run queue
 			} else {
				scheduler->first_running_queue = node;
				node->next = ptr;
				__sync_lock_release(&modifying_queue);
				return 0;
			}
		}
		prev = ptr;
		ptr = ptr->next;
	}
//	If no threads have lower priority, then the thread must be inserted at the end
	if (prev != NULL) {
		prev->next = node;
//	If prev is null, then that means that the queue is empty
	} else {
		scheduler->first_running_queue = node;
	}
	__sync_lock_release(&modifying_queue);
	return 0;
}

//A function to get the currently running thread.
thread_node* get_current_thread() {
//	Based on the current queue number, return the first thread from that queue
	if (scheduler->current_queue_number == 1) {
		return scheduler->first_running_queue;
	} else if (scheduler->current_queue_number == 2) {
		return scheduler->second_running_queue;
	}  else {
		return scheduler->third_running_queue;
	}
}

int add_to_mutex_wait_queue(mutex_waiting_queue_node* node) {
//	If the mutex wait queue is empty, then set the wait queue to be the thread
	if (scheduler->mutex_waiting_queue == NULL) {
		scheduler->mutex_waiting_queue = node;
		return 0;
	}
	mutex_waiting_queue_node* ptr = scheduler->mutex_waiting_queue;
//	Iterate through the queue and stop when we reach a NULL value	
	while (ptr->next != NULL) {
		ptr = ptr->next;
	}
//	Add the node to the end of the mutex wait queue
	ptr->next = node;
	return 0;
}

int add_to_join_wait_queue(join_waiting_queue_node* node) {
//	If the join wait queue is empty, then set the wait queue to be the thread
	if (scheduler->join_waiting_queue == NULL) {
		scheduler->join_waiting_queue = node;
		return 0;
	}
	join_waiting_queue_node* ptr = scheduler->join_waiting_queue;
//	Iterate through the queue and stop when we reach a NULL value	
	while (ptr->next != NULL) {
		ptr = ptr->next;
	}
//	Add the node to the end of the join wait queue
	ptr->next = node;
	return 0;
}

//A function to return the queue number with the highest priority
//If there is a tie, then the higher priority queue (e.g. first over second) is run
int get_highest_priority() {
//	If the queue is already being modified, wait for the operation to finish, then continue
	while (__sync_lock_test_and_set(&modifying_queue, 1) == 1) {
		int placeholder = 0;
    }
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
	__sync_lock_release(&modifying_queue);
	return highest_priority_queue;
}

//Increases the priority of every thread in each running queue.
int age() {
	thread_node* ptr = scheduler->first_running_queue;
	while (ptr != NULL) {
		ptr->thread->priority += 1;
		ptr = ptr->next;
	}
	ptr = scheduler->second_running_queue;
	while (ptr != NULL) {
		ptr->thread->priority += 1;
		ptr = ptr->next;
	}
	ptr = scheduler->third_running_queue;
	while (ptr != NULL) {
		ptr->thread->priority += 1;
		ptr = ptr->next;
	}
	return 0;
}

//Swaps contexts between the current thread and the thread with the highest priority
int swap_contexts() {
	printf("swap contexts\n");
//	If the scheduler is already running, don't do anything
	if (__sync_lock_test_and_set(&scheduler_running, 1) == 1) {
        return 0;
    }
//	If another function is modifying the queue, wait for it to finish before working
	if (__sync_lock_test_and_set(&modifying_queue, 1) == 1) {
        timer.it_interval.tv_usec = 1000;
        return 0;
    }
	thread_node* ptr;
//	Depending on which run queue was running, change the priority of the current thread
	switch(scheduler->current_queue_number) {
//		If a thread in the first run queue was running, age every other thread, then move it to the second run queue and set its priority to 50.
		thread_node* current_running_queue;
		case 1: 
		current_running_queue = scheduler->first_running_queue;
		ptr = scheduler->first_running_queue;
		scheduler->first_running_queue = ptr->next;
		age();
		yield_handler(ptr);
		break;
//		If a thread in the second run queue was running, age every other thread, then move it to the third run queue and set its priority to 0.
		case 2: 
		current_running_queue = scheduler->second_running_queue;
		ptr = scheduler->second_running_queue;
		scheduler->second_running_queue = ptr->next;
		age();
		yield_handler(ptr);
		break;
//		If a thread in the third run queue was running, then it must be finished, because all threads there run to completion.
		case 3: 
		current_running_queue = scheduler->first_running_queue;
		ptr = scheduler->third_running_queue;
		scheduler->third_running_queue = ptr->next;
		age();
		yield_handler(ptr);
		break;
//		If none of the above, then something went wrong.
		default: 
		__sync_lock_release(&scheduler_running);
		__sync_lock_release(&modifying_queue);
		return -1;
	}
//	Depending on which queue has the highest first priority, switch the context to run that thread


//FIX THIS
	ptr->thread->yield_purpose = 0;
//FIX THIS



	switch (get_highest_priority()) {
//		If there are no more threads, then do nothing.
		case 0:
		__sync_lock_release(&scheduler_running);
		__sync_lock_release(&modifying_queue);
		return 0;
//		If the first queue has the highest priority thread, switch to that one.
		case 1:
		scheduler->current_queue_number = 1;
		timer.it_value.tv_usec = 25000;
		timer.it_interval.tv_usec = 25000;
		__sync_lock_release(&scheduler_running);
		__sync_lock_release(&modifying_queue);
		swapcontext(&(ptr->thread->context), &(scheduler->first_running_queue->thread->context));
		break;
//		If the second queue has the highest priority thread, switch to that one.
		case 2:
		scheduler->current_queue_number = 2;
		timer.it_value.tv_usec = 50000;
		timer.it_interval.tv_usec = 50000;
		__sync_lock_release(&scheduler_running);
		__sync_lock_release(&modifying_queue);
		swapcontext(&(ptr->thread->context), &(scheduler->second_running_queue->thread->context));
		break;
//		If the third queue has the highest priority thread, switch to that one.
		case 3:
		scheduler->current_queue_number = 3;
		__sync_lock_release(&scheduler_running);
		__sync_lock_release(&modifying_queue);
		swapcontext(&(ptr->thread->context), &(scheduler->third_running_queue->thread->context));
		break;
//		If none of the above, then something went wrong.
		__sync_lock_release(&scheduler_running);
		__sync_lock_release(&modifying_queue);
		default: return -1;
	}
		__sync_lock_release(&scheduler_running);
		__sync_lock_release(&modifying_queue);
	return 0;
}

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {
//	Malloc some space and create a new thread
	thread_node* new_thread = malloc(sizeof(thread_node));
	new_thread->thread = malloc(sizeof(my_pthread));
	getcontext(&(new_thread->thread->context));
	printf("Got context\n");
	
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
	makecontext(&(new_thread->thread->context), function, 1, arg);
	printf("Made a new context\n");
//	Initiate the thread to have priority 100, default for threads in priority level 1.
	new_thread->thread->priority = 100;
//	If there's no timer, create a new timer and set an alarm for every 25 ms
	if (timer.it_interval.tv_usec == 0) {
		printf("making a timer\n");
		__sync_lock_release(&scheduler_running);
		__sync_lock_release(&modifying_queue);
		mutex_id = 0;
//		Set the signal handler to be the execute function
		signal (SIGALRM, swap_contexts);
		struct itimerval old;
		timer.it_value.tv_sec = 0;
		timer.it_value.tv_usec = 25000;
		timer.it_interval.tv_sec = 0;
		timer.it_interval.tv_usec = 25000;
		setitimer(ITIMER_REAL, &timer, &old);
	}
//	If the scheduler hasn't been initialized yet, initialize it
	if (scheduler == NULL) {
		printf("making a scheduler\n");
		scheduler = malloc(sizeof(tcb));
	}
//	Add the thread to the end of the first run queue. 
	printf("Adding to run queue\n");
	add_to_run_queue(1, new_thread);
	printf("Added to run queue\n");
	return 0;
};

// helper function for swap_contexts() to handle yield
int yield_handler(thread_node* ptr)
{
    switch (ptr->thread->yield_purpose) {
    case 1: {
        // exit()
        // copy the pid of current thread
        my_pthread_t exit_pid = ptr->thread->pid;
        // remove the thread
        thread_node *temp = ptr;
		free(temp->thread);
		free(temp);
        // iterate through waiting queue, move threads waiting for exit thread to running queue
        join_waiting_queue_node *wait_prev = NULL;
        join_waiting_queue_node *wait_ptr = scheduler->join_waiting_queue;
        while (wait_ptr != NULL)
        {
            if (wait_ptr->thread->pid == exit_pid)
            {
                // add node to run queue
                thread_node *new_node = (thread_node *) malloc(sizeof(thread_node));
                new_node->thread = wait_ptr->thread;
                add_to_run_queue_priority_based(new_node);
                // remove node from wait queue
                if (wait_prev == NULL) // head of queue
                {
                    scheduler->join_waiting_queue = wait_ptr->next;
                    free(wait_ptr);
                }
                else
                {
                    wait_prev->next = wait_ptr->next;
					free(wait_ptr);
                }
			}
			wait_prev = wait_ptr;
			wait_ptr = wait_ptr->next;
		}
        break;
    }
    case 2: {
        // join()
        break;
    }
	case 3: {
		//mutex_lock()
		break;
	} case 4: {
		//yield()
		thread_node* current_running_queue;
		switch(scheduler->current_queue_number) {
		case 1:
		current_running_queue = scheduler->first_running_queue;
		break;
		case 2:
		current_running_queue = scheduler->second_running_queue;
		break;
		case 3:
		current_running_queue = scheduler->third_running_queue;
		break;
		}
		if (ptr->next != NULL) {
			current_running_queue = ptr->next;
			current_running_queue->next = ptr;
			ptr->next = ptr->next->next;
		}
	}
    default:
	switch(scheduler->current_queue_number) {
		case 1:
		ptr->thread->priority = 50;
		add_to_run_queue(2, ptr);
		break;
		case 2:
		ptr->thread->priority = 0;
		add_to_run_queue(3, ptr);
		break;
		default:
		break;
	}
	break;
    }
}

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
    swap_contexts();
    return -1;
}

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
    // lock queue
    if (__sync_lock_test_and_set(&modifying_queue, 1) == 1)
    {
        return -1; // another thread locks the queue, should not happen
    }
    // save return value to the threads waiting for this thread
    if (scheduler->join_waiting_queue != NULL)
    {
        join_waiting_queue_node *wait_ptr = scheduler->join_waiting_queue;
        join_waiting_queue_node *wait_prev = NULL;
        /* if (wait_ptr->pid == current_pid) */
        /* { */
        /*     wait_ptr->ret_val_pos = value_ptr; */
        /* } */
        while (wait_ptr != NULL)
        {
            if (wait_ptr->pid == current_pid)
            {
                wait_ptr->ret_val_pos = value_ptr;
            }
            wait_prev = wait_ptr;
            wait_ptr = wait_ptr->next;
        }
    }
    // set flag to indicate pthread exit
    get_current_thread()->thread->yield_purpose = 1;
    // unlock queue
    __sync_lock_release(&modifying_queue);
    my_pthread_yield();
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
    // lock queue
    if (__sync_lock_test_and_set(&modifying_queue, 1) == 1)
    {
        return -1; // another thead locks the queue, should not happen
    }
    // create new waiting node
    join_waiting_queue_node *new_node = (join_waiting_queue_node *) malloc(sizeof(join_waiting_queue_node));
    new_node->thread = get_current_thread()->thread;
    new_node->pid = get_current_thread()->thread->pid;
    &(new_node->ret_val_pos) = value_ptr;
    // add to wait queue
    if (scheduler->join_waiting_queue == NULL)
    {
        scheduler->join_waiting_queue = new_node;
    }
    else
    {
        join_waiting_queue_node *ptr = scheduler->join_waiting_queue;
        while (ptr->next != NULL)
        {
            ptr = ptr->next;
        }
        ptr->next = new_node;
    }
    // set flag for scheduler
    get_current_thread()->thread->yield_purpose = 2;
    // unlock queue mutex
    __sync_lock_release(&modifying_queue);
    my_pthread_yield();
    //Wait for the other thread to finish executing
    return 0;
};

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
    mutex = malloc(sizeof(my_pthread_mutex_t));
//	Set the initial lock to be open
    __sync_lock_release(&(mutex->mutex_lock));
//	Set a new mutex id
    mutex->mid = mutex_id;
    mutex_id++;
    return 0;
};

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
//	All mutexes are initialized with positive id's - if it's negative, then it's been destroyed. 
    if (mutex->mid < 0) {
        return -1;
    }
//	If the mutex is unlocked, then acquire it
    if (__sync_lock_test_and_set(&(mutex->mutex_lock), 1) == 0) {
        mutex->pid = get_current_thread()->thread->pid;
//	Otherwise, move to the wait queue
    } else {
        mutex_waiting_queue_node *new_node = malloc(sizeof(mutex_waiting_queue_node));
//		Remove the current thread from the run queue
		thread_node* current_thread = get_current_thread();
		switch(scheduler->current_queue_number) {
			case 1:
			scheduler->first_running_queue = current_thread->next;
			break;
			case 2:
			scheduler->second_running_queue = current_thread->next;
			break;
			case 3:
			scheduler->third_running_queue = current_thread->next;
			break;
		}
//		Create a new node with a thread equal to the currently running thread
        new_node->thread = current_thread->thread;
//		Set the mutex id the thread is waiting for
        new_node->mutex_lock = mutex->mid;
//		Add the thread to the end of the wait queue
        add_to_mutex_wait_queue(new_node);
//		Swap contexts
        swap_contexts();
    }
    return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
//	All mutexes are initialized with positive id's - if it's negative, then it's been destroyed. 
    if (mutex->mid < 0) {
        return -1;
    }
//	If the current thread is holding the lock, then release it
    if (get_current_thread()->thread->pid == mutex->pid) {
        __sync_lock_release(&(mutex->mutex_lock));
        mutex->pid = -1;
//		Remove all nodes from the wait queue that were waiting for this mutex
        mutex_waiting_queue_node* ptr = scheduler->mutex_waiting_queue;
        mutex_waiting_queue_node* prev = NULL;
        while (ptr != NULL) {
            if (ptr->mutex_lock == mutex->mid) {
                if (prev == NULL) {
                    scheduler->mutex_waiting_queue = ptr->next;
                } else {
                    prev->next = ptr;
                }
//				Add any nodes that were removed from the wait queue to the end of the run queue
                thread_node* new_node = malloc(sizeof(thread_node));
                new_node->thread = ptr->thread;
                add_to_run_queue_priority_based(new_node);
            }
            prev = ptr;
            ptr = ptr->next;
        }
    }
    return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
//	Wait for the mutex lock to be released before destroying it
    if (__sync_lock_test_and_set(&(mutex->mutex_lock), 1) == 1) {
//		Set the id to be negative. This means it's not usable.
        mutex->mid = -1;
    }
    return 0;
};
