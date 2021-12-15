#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include "pcp.h"

void init_pcp() {
#ifdef MODE_PROCESS
    buf = mmap(NULL, sizeof(int) * N, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
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
#endif
}

// バッファに入れるものを生成する
int produce_item() {
    // 適当なアイテムを作る
    int item = rand() % 100;
    printf("produce item: %d\n", item);
    return item;
}

// itemを消費する
void consume_item(int item) {
    printf("consume item: %d\n", item);
}

// バッファからアイテムを取り出す(消費者側から使う)
int remove_item(int pos) {
    printf("remove item: buf[%d]: %d\n", pos, buf[pos]);
    // アイテムをバッファ内から取り出す
    return buf[pos];
}

// 新たな項目をバッファ内に入れる(生産者側から使う)
void insert_item(int item, int pos) {
    // アイテムをバッファ内に入れる
    buf[pos] = item;
    printf("insert item: buf[%d]: %d\n", pos, buf[pos]);
}

// アイテムを作り(生産し)、バッファ内にそれを入れるタスク
void* producer() {
    int item;
    int buf_pos = 0;
    while (1) {
        // バッファに入れるものを生成
        item = produce_item();
        // 空バッファ数をデクリメント
        // empty=0であれば、バッファに空きがないことを意味するので、ここでブロックされる
        sem_wait(empty);
        // mutexをロック状態にする = クリティカルリージョンへのアクセス権を得る
        // すでにロックされていれば他のタスクがクリティカルリージョンに入っていることを意味するので、ここでブロックされる
#ifdef MODE_PROCESS
        sem_wait(mutex);
#elif defined MODE_THREAD
        pthread_mutex_lock(&mutex);
#endif
        // 新たな項目をバッファ内に入れる
        insert_item(item, buf_pos);
        // クリティカルリージョンから出たので、アクセス権を放棄する(mutex上でブロックされているタスクがあれば起こす)
#ifdef MODE_PROCESS
        sem_post(mutex);
#elif defined MODE_THREAD
        pthread_mutex_unlock(&mutex);
#endif
        // インクリメントした値がバッファサイズ以上なら0に戻す
        if (++buf_pos >= N) {
            buf_pos = 0;
        }
        // 詰まっているスロット数をインクリメント(full上でブロックされているタスクがあれば起こす)
        sem_post(full);
    }
}
// バッファ内のアイテムを取り出し、それを使う(消費する)タスク
void* consumer() {
    int item;
    int buf_pos = 0;
    while (1) {
        // 詰まっているスロット数をデクリメント
        // full=0であれば、消費するアイテムがないことを意味するので、ここでブロックされる
        sem_wait(full);
        // mutexをロック状態にする = クリティカルリージョンへのアクセス権を得る
        // すでにロックあれば他のタスクがクリティカルリージョンに入っていることを意味するので、ここでブロックされる
#ifdef MODE_PROCESS
        sem_wait(mutex);
#elif defined MODE_THREAD
        pthread_mutex_lock(&mutex);
#endif
        // アイテムを取り出す
        item = remove_item(buf_pos);
        // クリティカルリージョンから出たので、アクセス権を放棄する(mutex上でブロックされているタスクがあれば起こす)
#ifdef MODE_PROCESS
        sem_post(mutex);
#elif defined MODE_THREAD
        pthread_mutex_unlock(&mutex);
#endif
        // インクリメントした値がバッファサイズ以上なら0に戻す
        if (++buf_pos >= N) {
            buf_pos = 0;
        }
        // アイテムを取り出したので、空バッファ数をインクリメント(empty上でブロックされているタスクがあれば起こす)
        sem_post(empty);
        // アイテムに対し何か行う
        consume_item(item);
    }

}