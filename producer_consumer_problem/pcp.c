/**
 * @file pcp.c
 * 生産者・消費者問題のロジック部分
 * プロセスでもスレッドでも扱えるようにする
 */

#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include "pcp.h"

//! バッファサイズ
#define N 10000

static void lock_mutex();
static void unlock_mutex();
static int produce_item(void);
static void consume_item(int item);
static int remove_item(int pos);
static void insert_item(int item, int pos);

// セマフォと共有資源のアドレスを保持する
#ifdef MODE_THREAD
pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t* mutex;
sem_t empty_, *empty;
sem_t full_, *full;
int buf[N];
#elif defined MODE_PROCESS
sem_t* empty;
sem_t* full;
sem_t* mutex;
int* buf;
#endif

/** 生産者・消費者問題の初期化をする関数 */
void init_pcp(void) {
#ifdef MODE_PROCESS
    buf = mmap(NULL, sizeof(int) * N, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    // 共有資源を0埋めしておく
    memset(buf, 0, sizeof(int)*N);
    empty = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    full = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    sem_init(empty, 1, N);
    sem_init(full, 1, 0);
    sem_init(mutex, 1, 1);
#elif defined MODE_THREAD
    sem_init(&empty_, 0, N);
    sem_init(&full_, 0, 0);
    empty = &empty_;
    full = &full_;
    mutex = &mutex_;
#endif
}

/**
 * ミューテックスのダウン操作をする関数
 * プロセスではセマフォとして操作、スレッドではミューテックスとして操作しているが
 * 実質的に行っていることは同じである。
 */
static void lock_mutex() {
#ifdef MODE_PROCESS
    sem_wait(mutex);
#elif defined MODE_THREAD
    pthread_mutex_lock(mutex);
#endif
}

/**
 * ミューテックスのアップ操作をする関数
 * プロセスではセマフォとして操作、スレッドではミューテックスとして操作しているが
 * 実質的に行っていることは同じである。
 */
static void unlock_mutex() {
#ifdef MODE_PROCESS
    sem_post(mutex);
#elif defined MODE_THREAD
    pthread_mutex_unlock(mutex);
#endif
}

/** バッファに入れるものを生成する関数 */
static int produce_item(void) {
    // 適当なアイテムを作る
    int item = rand() % 100;
    printf("produce item: %d\n", item);
    return item;
}

/** itemを消費する関数 */
static void consume_item(int item) {
    printf("consume item: %d\n", item);
}

/** バッファからアイテムを取り出す(消費者側から使う)関数 */
static int remove_item(int pos) {
    printf("remove item: buf[%d]: %d\n", pos, buf[pos]);
    // アイテムをバッファ内から取り出す
    return buf[pos];
}

/** 新たな項目をバッファ内に入れる(生産者側から使う)関数 */
static void insert_item(int item, int pos) {
    // アイテムをバッファ内に入れる
    buf[pos] = item;
    printf("insert item: buf[%d]: %d\n", pos, buf[pos]);
}

/** アイテムを作り(生産し)、バッファ内にそれを入れる関数 */
void* producer() {
    int item;
    int buf_pos = 0;
    // while (1) {
    for (int i = 0; i < 100000; i++) {
        // バッファに入れるものを生成
        item = produce_item();
        // 空バッファ数をデクリメント
        // empty=0であれば、バッファに空きがないことを意味するので、ここでブロックされる
        sem_wait(empty);
        // mutexをロック状態にする = クリティカルリージョンへのアクセス権を得る
        // すでにロックされていれば他のタスクがクリティカルリージョンに入っていることを意味するので、ここでブロックされる
        lock_mutex();
        // 新たな項目をバッファ内に入れる
        insert_item(item, buf_pos);
        // クリティカルリージョンから出たので、アクセス権を放棄する(mutex上でブロックされているタスクがあれば起こす)
        unlock_mutex();
        // インクリメントした値がバッファサイズ以上なら0に戻す
        if (++buf_pos >= N) {
            buf_pos = 0;
        }
        // 詰まっているスロット数をインクリメント(full上でブロックされているタスクがあれば起こす)
        sem_post(full);
    }
    return NULL;
}

/** バッファ内のアイテムを取り出し、それを使う(消費する)関数 */
void* consumer() {
    int item;
    int buf_pos = 0;
    // while (1) {
    for (int i = 0; i < 100000; i++) {
        // 詰まっているスロット数をデクリメント
        // full=0であれば、消費するアイテムがないことを意味するので、ここでブロックされる
        sem_wait(full);
        // mutexをロック状態にする = クリティカルリージョンへのアクセス権を得る
        // すでにロックあれば他のタスクがクリティカルリージョンに入っていることを意味するので、ここでブロックされる
        lock_mutex();
        // アイテムを取り出す
        item = remove_item(buf_pos);
        // クリティカルリージョンから出たので、アクセス権を放棄する(mutex上でブロックされているタスクがあれば起こす)
        unlock_mutex();
        // インクリメントした値がバッファサイズ以上なら0に戻す
        if (++buf_pos >= N) {
            buf_pos = 0;
        }
        // アイテムを取り出したので、空バッファ数をインクリメント(empty上でブロックされているタスクがあれば起こす)
        sem_post(empty);
        // アイテムに対し何か行う
        consume_item(item);
    }
    return NULL;
}
