#include <pthread.h>
#include "dpp.h"

int main(void) {
    init_philosophers();
    pthread_t thread_id[5];
    int i;
    for (i = 0; i < N; i++) {
        pthread_create(&thread_id[i], NULL, philosopher, &i);
    }

    for (i = 0; i < N; i++) {
        pthread_join(thread_id[i], NULL);
    }
    return 0;
}
