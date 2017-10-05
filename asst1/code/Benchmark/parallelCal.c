// File:	parallelCal.c
// Author:	Yujie REN
// Date:	09/23/2017

#include <stdio.h>
#include <unistd.h>

#include "../my_pthread_t.h"

#define THREAD_NUM 10

#define C_SIZE 100000
#define R_SIZE 1000

my_pthread_mutex_t   mutex;

int thread[THREAD_NUM];

int*    a[R_SIZE];
int	 pSum[R_SIZE];
int  sum;

/* A CPU-bound task to do parallel array addition */
void parallel_calculate(void* arg) {
	char *t_name = (char *) arg;
	int n = atoi(t_name) - 1;
	for (int j = n; j < R_SIZE; j += THREAD_NUM) {
		for (int i = 0; i < C_SIZE; ++i) {
			pSum[j] += a[j][i] * i;
		}
	}
	for (int j = n; j < R_SIZE; j += THREAD_NUM) {
		my_pthread_mutex_lock(&mutex);
		sum += pSum[j];
		my_pthread_mutex_unlock(&mutex);
	}
}

int main() {

	char name[2];

	// initialize data array
	for (int i = 0; i < R_SIZE; ++i)
		a[i] = (int*)malloc(C_SIZE*sizeof(int));

	for (int i = 0; i < R_SIZE; ++i)
		for (int j = 0; j < C_SIZE; ++j)
			a[i][j] = j;

	for (int i = 0; i < THREAD_NUM; ++i) {
		sprintf(name, "%d", i+1);
		my_pthread_create(&thread[i], NULL, &parallel_calculate, name);
	}

	for (int i = 0; i < THREAD_NUM; ++i)
		my_pthread_join(thread[i], NULL);

	return 0;
}
