/**
 * @file process_lock_pthread_ver.c
 *
 * 共有メモリを複数プロセスから読み書きするプログラム
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork()で新規(子)プロセスを作りたい
#include <sys/wait.h> // 自プロセスの子の終了を待つため
#include <sys/mman.h> // mmap
#include <semaphore.h> // POSIXセマフォ
#define N 10000 // ループ回数

void parent_process(int* x, sem_t* mutex); // 親プロセスが処理する内容
void child_process(int* x, sem_t* mutex); // fork()により作成された子プロセスが処理する内容

int main(void) {
    // 初期値を1とする名前付きセマフォを作る
    sem_t* mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(mutex, 1, 1);
    // 共有データ
    int* x = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // 新しく1つ子プロセスを作成する
    if (fork() == 0) { // この時点で新しいプロセスが作られている
        // 子プロセスの処理
        child_process(x, mutex);
        _exit(0);
    } else {
        // 親プロセスの処理
        parent_process(x, mutex);
        // 子プロセスの終了を待つ
        wait(NULL);
        // 最終的に共有メモリの値がどうなったかを知る
        printf("合計 = %d\n", *x);
    }
}

void parent_process(int* x, sem_t* mutex) {
    // 共有データをN回インクリメントする
    for (int i = 0; i < N; i++) {
        sem_wait(mutex);
        (*x)++;
        sem_post(mutex);
    }
}

void child_process(int* x, sem_t* mutex) {
    // 共有データをN回インクリメントする
    for (int i = 0; i < N; i++) {
        sem_wait(mutex);
        (*x)++;
        sem_post(mutex);
    }
}

