// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	09/23/2017

// name:
// username of iLab:
// iLab Server: 
#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <signal.h>

typedef uint my_pthread_t;

typedef struct my_pthread {
	ucontext_t context;
	int priority;
	int execution_time;
	my_pthread_t pid;
} my_pthread;

typedef struct thread_node {
	my_pthread* thread;
	struct thread_node* next;
} thread_node;

typedef struct mutex_waiting_queue_node {
	my_pthread* thread;
	uint mutex_lock;
	struct mutex_waiting_queue_node* next;
} waiting_queue_node;

typedef struct join_waiting_queue_node {
	my_pthread* thread;
	my_pthread_t pid;
	struct join_waiting_queue_node* next;
}

typedef struct threadControlBlock {
//	The first run queue is round robin with a time quantum of 25 ms
	thread_node* first_running_queue;
//	The second run queue is round robin with a time quantum of 50 ms
	thread_node* second_running_queue;
//	The third run queue is FIFO
	thread_node* third_running_queue;
//	Stores which queue is currently running
	int current_queue_number;
//	The first wait queue is for threads waiting for a mutex lock
	mutex_waiting_queue_node* mutex_waiting_queue;
//	The secon wait queue is for threads waiting to join another thread
	join_waiting_queue_node* join_waiting_queue;
} tcb; 

/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	uint pid;
	int mutex_lock;
	uint mid;
} my_pthread_mutex_t;

/* define your data structures here: */
tcb* scheduler;
struct itimerval* timer;
int scheduler_running = 0;
int modifying_queue = 0;
uint mutex_id = 0;
// Feel free to add your own auxiliary data structures


/* Function Declarations: */

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif
