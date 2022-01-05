#include <pthread.h>
#include "pcp.h"


int main() {
    init_pcp();
    pthread_t thread[200];
    int i;
    for (i = 0; i < 100; i++) {
        pthread_create(&thread[i], NULL, producer, NULL);
    }

    for (i = 100; i < 200; i++) {
        pthread_create(&thread[i], NULL, consumer, NULL);
    }

    for (i = 0; i < 200; i++) {
        pthread_join(thread[i], NULL);
    }
    return 0;
}
