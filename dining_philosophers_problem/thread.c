#include <pthread.h>
#include "dpp.h"

int main(void) {
    pthread_t thread_id[N];
    int phseq[N];
    int i;

    init_philosophers();

    for (i = 0; i < N; i++) {
        phseq[i] = i;
        pthread_create(&thread_id[i], NULL, philosopher, &phseq[i]);
    }
    for (i = 0; i < N; i++) {
        pthread_join(thread_id[i], NULL);
    }
    return 0;
}
