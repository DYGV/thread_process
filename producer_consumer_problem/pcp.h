#ifndef INCLUDED_PCP
#define INCLUDED_PCP

#include <semaphore.h>
#include <pthread.h>

#define N 100

#ifdef MODE_THREAD
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t empty_, *empty;
sem_t full_, *full;
int buf[N];
#elif defined MODE_PROCESS
sem_t* empty;
sem_t* full;
sem_t* mutex;
int* buf;
#endif
void init_pcp();
void* producer();
void* consumer();
#endif
