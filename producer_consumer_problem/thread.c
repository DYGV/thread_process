#include "pcp.h"

int main() {
    init_pcp();
    // スレッドの作成
    pthread_t t_1, t_2;
    pthread_create(&t_1, NULL, producer, NULL);
    pthread_create(&t_2, NULL, consumer, NULL);

    // メインスレッドが、スレッドt_1, t_2の終了を待つ
    pthread_join(t_1, NULL);
    pthread_join(t_2, NULL);
    return 0;
}
