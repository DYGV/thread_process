#include <pthread.h>
#include "pcp.h"


int main() {
    init_pcp();
    pthread_t t_1, t_2;
    pthread_create(&t_1, NULL, producer, NULL);
    pthread_create(&t_2, NULL, consumer, NULL);

    pthread_join(t_1, NULL);
    pthread_join(t_2, NULL);
    return 0;
}
