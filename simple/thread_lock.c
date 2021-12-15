// セマフォを用いて共有データにアクセスするプログラム
#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#define N 10000

// 共有データ
int x = 0;
// セマフォ変数
sem_t mutex;

void* FncA() {
    int i;
    for (i = 0; i < N; ++i) {
        // sem_wait(down操作)でセマフォを獲得しようとする
        sem_wait(&mutex);
        // 獲得できたらxへアクセスできる
        x++;
        // セマフォを戻し、ブロックしているタスクがあれば起こす
        sem_post(&mutex);
    }
}

void* FncB() {
    int i;
    for (i = 0; i < N; ++i) {
        // sem_wait(down操作)でセマフォを獲得しようとする
        sem_wait(&mutex);
        // 獲得できたらxへアクセスできる
        x++;
        // セマフォを戻し、ブロックしているタスクがあれば起こす
        sem_post(&mutex);
    }
}

int main(void) {
    // mutexを1で初期化する(無名セマフォ)
    sem_init(&mutex, 0, 1);
    int i;
    pthread_t thread;
    // スレッドの作成
    pthread_create(&thread, NULL, FncA, NULL);
    FncB();
    // 作成したスレッドの終了を待つ
    pthread_join(thread, NULL);
    printf("合計 = %d\n", x);
    return 0;
}
