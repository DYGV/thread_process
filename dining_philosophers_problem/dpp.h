#ifndef INCLUDED_DPP
#define INCLUDED_DPP

#include <semaphore.h>
#include <pthread.h>

#define N 5
#define LEFT(i) (i+N-1) % N
#define RIGHT(i) (i+1) % N
#define THINKING 0
#define HUNGRY 1
#define EATING 2

#ifdef MODE_THREAD
int state[N] = {0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t s[N];
#elif defined MODE_PROCESS
int* state;
sem_t* s;
sem_t* mutex;
#endif

void* philosopher(void* i);
void init_philosophers();
#endif
