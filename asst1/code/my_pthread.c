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
    if (__sync_lock_test_and_set(&scheduler_running, 1) == 1) {
        return 0;
    }
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
//	Initiate the thread to have priority 100, default for threads in priority level 1.
    new_thread->thread->priority = 100;
//	If there's no timer, create a new timer and set an alarm for every 25 ms
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
//	Add the thread to the end of the first run queue.
    add_to_run_queue(1, new_thread);
    return 0;
}

// helper function for swap_contexts() to handle yield
int yield_handler()
{
    switch (scheduler->current_running_queue->thread->yield_purpose) {
    case 1: {
        // exit()
        // copy the pid of current thread
        my_pthread_t exit_pid = scheduler->current_running_queue->thread->pid;
        // remove the thread
        thread_node *temp = scheduler->current_running_queue;
        // TO-DO: head of queue point to next
        free(temp->thread);
        free(temp);

        // iterate through waiting queue, move threads waiting for exit thread to running queue
        waiting_thread_queue_node *wait_prev = NULL;
        waiting_thread_queue_node *wait_ptr = scheduler->waiting_thread_queue;
        while (wait_ptr != NULL)
        {
            if (wait_ptr->pid == exit_pid)
            {
                // add node to run queue
                thread_node *new_node = (thread_node *) malloc(sizeof(thread_node));
                new_node->thread = wait_ptr->thread;
                add_to_run_queue_priority_based(new_node);
                // remove node from wait queue
                if (wait_prev == NULL) // head of queue
                {
                    scheduler->waiting_thread_queue_node = scheduler->waiting_thread_queue_node->next;
                    free(wait_ptr);
                    wait_ptr = scheduler->waiting_thread_queue;
                }
                else
                {
                    wait_prev = wait_ptr;
                    wait_ptr = wait_ptr->next;
                    free(wait_prev);
                }
        }
        break;
    }
    case 2: {
        // join()
        break;
    }
    default:
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
    if (__sync_lock_test_and_set(&queue_lock, 1) == 1)
    {
        return -1; // another thread locks the queue, should not happen
    }
    my_pthread_t current_pid = scheduler->current_running_queue->thread->pid;
    thread_node *ptr = scheduler->current_running_queue;
    thread_node *prev = NULL;
    while (ptr->thread.pid != pid)
    {
        prev = ptr;
        ptr = ptr->next;
    }

    if (prev == NULL) // current thread is the only thread in current running queue
    {
        scheduler->current_running_queue = NULL;
    }
    else
    {
        prev->next = ptr->next;
    }
    /* free(ptr); */
    // save return value to the threads waiting for this thread
    if (scheduler->waiting_thread_queue != NULL)
    {
        waiting_thread_queue_node *wait_ptr = scheduler->waiting_thread_queue;
        waiting_thread_queue_node *wait_prev = NULL;
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
            wait_ptr = wait_ptr;
        }
    }
    // set flag to indicate pthread exit
    scheduler->current_running_queue->thread->yield_purpose = 1;
    // unlock queue
    __sync_lock_release(&queue_lock);
    my_pthread_yield();
};

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
    // lock queue
    if (__sync_lock_test_and_set(&queue_lock, 1) == 1)
    {
        return -1; // another thead locks the queue, should not happen
    }
    // create new waiting node
    waiting_thread_queue_node *new_node = (waiting_thread_queue_node *) malloc(sizeof(waiting_thread_queue_node));
    new_node->thread = scheduler->current_running_queue->thread;
    new_node->pid = thread->pid;
    &(new_node->ret_val_pos) = value_ptr;
    // add to wait queue
    if (scheduler->waiting_thread_queue == NULL)
    {
        scheduler->waiting_thread_queue = new_node;
    }
    else
    {
        waiting_thread_queue_node *ptr = scheduler->waiting_thread_queue;
        while (ptr->next != NULL)
        {
            ptr = ptr->next;
        }
        ptr->next = new_node;
    }
    // set flag for scheduler
    scheduler->current_running_queue->thread->yield_purpose = 2;
    // unlock queue mutex
    __sync_lock_release(&queue_lock);
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
    /* if (__sync_lock_test_and_set(&queue_lock, 1) == 1) */
    /* { */
    /*     return -1; // queue is locked by other thread, should not happen */
    /* } */
//	All mutexes are initialized with positive id's - if it's negative, then it's been destroyed.
    if (mutex->mid < 0) {
        return -1;
    }
//	If the mutex is unlocked, then acquire it
    if (__sync_lock_test_and_set(&(mutex->mutex_lock), 1) == 0) {
        mutex->pid = scheduler->current_running_queue->thread->pid;
//	Otherwise, move to the wait queue
    } else {
        waiting_mutex_queue_node *new_node = malloc(sizeof(waiting_mutex_queue_node));
//		Create a new node with a thread equal to the currently running thread
        new_node->thread = scheduler->current_running_queue->thread;
//		Set the mutex id the thread is waiting for
        new_node->mutex_lock = mutex->mid;
//    set flag and call scheduler
        yield_purpose = 3;
        /* __sync_lock_release(&queue_lock); */
        my_pthread_yield();
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
    if (scheduler->running_queue->thread->pid == mutex->pid) {
        mutex->mutex_lock = 0;
        mutex->pid = -1;
//		Remove all nodes from the wait queue that were waiting for this mutex
        waiting_queue_node* ptr = scheduler->waiting_queue;
        waiting_queue node* prev = NULL;
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
    return 0;
};

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
//	Wait for the mutex lock to be released before destroying it
    if (mutex->mutex_lock == 0) {
//		Set the id to be negative. This means it's not usable.
        mutex->mid = -1;
    }
    return 0;
};

