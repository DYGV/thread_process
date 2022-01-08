#include <pthread.h>
#include "pcp.h"


int main() {
    init_pcp();
    pthread_t thread[16];
    int i;
    for (i = 0; i < 8; i++) {
        pthread_create(&thread[i], NULL, producer, NULL);
    }

    for (i = 8; i < 16; i++) {
        pthread_create(&thread[i], NULL, consumer, NULL);
    }

    for (i = 0; i < 16; i++) {
        pthread_join(thread[i], NULL);
    }
    return 0;
}
