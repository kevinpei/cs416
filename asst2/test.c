#include <stdio.h>
#include <unistd.h>
#include "mymalloc.h"
#define ARRAY_SIZE 10
int main(){
    int* p = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* q = p;
    int i;
    for(i = 0; i < ARRAY_SIZE; ++i){
        *(p++) = i;
    }
    p = q;
    for(i = 0; i < ARRAY_SIZE; ++i){
        printf("Element %d is %d\n", i, *(p++));
    }
    free(p);
    return 0;
}
