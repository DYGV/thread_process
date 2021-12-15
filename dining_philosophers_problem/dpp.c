#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include "dpp.h"

static void think(int i);
static void take_forks(int i);
static void eat(int i);
static void put_forks(int i);
static void test(int i);


void init_philosophers() {
    int is_inter_process = 0;
#ifdef MODE_PROCESS
    is_inter_process = 1;
    state = mmap(NULL, sizeof(int) * N, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    memset(state, 0, sizeof(int)*N);
    s = mmap(NULL, sizeof(sem_t) * N, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(mutex, 1, 1);
#endif
    for (int i = 0; i < N; i++) {
        sem_init(&s[i], is_inter_process, 0);
    }
}

static void cleanup(int p) {
    sem_post(&s[p]);
#ifdef MODE_PROCESS
    sem_post(mutex);
#elif defined MODE_THREAD
    pthread_mutex_unlock(&mutex);
#endif
}

void* philosopher(void* i) {
    // intへのポインタ型にキャストしてから間接参照してpへ値を入れる
    int p = *((int*)i);
    // while (1) {
    for (int j = 0; j < 100000; j++) {
        // 思考する
        think(p);
        // 2本のフォークを取るか、あるいはブロック
        take_forks(p);
        // ここまで到達できればフォークが取れているので、スパゲッティを食べることができる
        eat(p);
        // 両方のフォークをテーブルに戻す
        put_forks(p);
    }

    // ループ終了時にブロックされているタスク(哲学者)があってもwait(join)で無限に待つことにならないように
    cleanup(p);
}

static void think(int i) {
    printf("philosopher %d is thinking.\n", i);
}

static void take_forks(int i) {
#ifdef MODE_PROCESS
    sem_wait(mutex);
#elif defined MODE_THREAD
    pthread_mutex_lock(&mutex);
#endif
    // 哲学者iが空腹であることを記憶しておく
    state[i] = HUNGRY;
    printf("philosopher %d is hungry.\n", i);
    // フォークを取ろうとする(もし取れなくてもここでブロックはされない)
    test(i);
#ifdef MODE_PROCESS
    sem_post(mutex);
#elif defined MODE_THREAD
    pthread_mutex_unlock(&mutex);
#endif
    // 哲学者iがフォークを持っているかを示すセマフォ
    // s[i]が0であればフォークを取れていないことを意味するのでここでブロック
    sem_wait(&s[i]);
}

static void eat(int i) {
    printf("philosopher %d is eating spaghetti.\n", i);
}


static void put_forks(int i) {
#ifdef MODE_PROCESS
    sem_wait(mutex);
#elif defined MODE_THREAD
    pthread_mutex_lock(&mutex);
#endif
    // 哲学者iが食事を終了
    state[i] = THINKING;
    printf("philosopher %d put forks.\n", i);
    // 左隣の哲学者が今食事できるか調べる
    test(LEFT(i));
    // 右隣の哲学者が今食事できるか調べる
    test(RIGHT(i));
    // クリティカルリージョンから出る
#ifdef MODE_PROCESS
    sem_post(mutex);
#elif defined MODE_THREAD
    pthread_mutex_unlock(&mutex);
#endif
}

static void test(int i) {
    // 哲学者iがフォークを取りたがっており(空腹状態で)、
    // その哲学者iの左隣、右隣が食事中でないなら
    // 哲学者iの状態をEATINGにすることができる(フォークが取れる)
    if (state[i] == HUNGRY && state[LEFT(i)] != EATING && state[RIGHT(i)] != EATING) {
        printf("philosopher %d could take forks.\n", i);
        state[i] = EATING;
        // 哲学者iがフォークが取れたことを示すためup操作を行う
        // 哲学者ごとにセマフォs[0..N-1のいずれ]があるので、他の哲学者をここで起こすことはない
        sem_post(&s[i]);
    } else if (state[i] != HUNGRY) {
        printf("philosopher %d was not hungry, so he did not take forks.\n", i);
    } else {
        printf("philosopher %d could not take forks.\n", i);
    }
}

