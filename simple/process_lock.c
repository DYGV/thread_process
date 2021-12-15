/**
 * @file process_lock.c
 * プロセス間で同期を取る方法はシグナルハンドラを使う方法や、
 * POSIXの名前付きセマフォなどがあるが、
 * 今回は、System V セマフォを触ってみる。
 *
 * 共有メモリを複数プロセスから読み書きするプログラム
 */

/**
 * System V IPC: System Vが提供するプロセス間通信手段の総称
 *
 * System V IPCオブジェクトの作成にはget操作を用いる
 * - msgget(): メッセージキューの作成(今回は使わない)
 * - shmget(): 共有メモリの作成
 * - semget(): セマフォの作成
 * 各関数の引数で、key_t型(実体は整数値)を用いるが、
 * これはファイルディスクリプタに相当するものである。
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // fork()で新規(子)プロセスを作りたい
#include <sys/shm.h> // System V プロセス間で共有メモリを扱いたい
#include <sys/stat.h> // 共有メモリを扱うときのパーミッション設定用
#include <sys/wait.h> // 自プロセスの子の終了を待つため
#include <sys/sem.h> // System Vのセマフォを使ってみる

#define N 10000 // ループ回数

/**
 * semctl()で使う共用体
 * 最近のglibcには定義されていないらしく
 * プログラマが定義する必要があるらしい
 * 今回はセマフォの値だけ使えれば良い
 */
union semun {
    int val;
};

void parent_process(int shm_id, int sem_id); // 親プロセスが処理する内容
void child_process(int shm_id, int sem_id); // fork()により作成された子プロセスが処理する内容
int new_shm(); // 新規の共有資源を作る関数
int new_sem(int val); // 新規のセマフォを作る関数
void reserve_sem(int sem_id); // セマフォのdown操作をする関数
void release_sem(int sem_id); // セマフォのup操作をする関数

int main(void) {
    int sem_id = new_sem(1); // 初期値1のセマフォを作成してIPCオブジェクトのIDをもらう
    int shm_id = new_shm(); // 新規共有データを作成してIPCオブジェクトのIDをもらう
    // 新しく1つ子プロセスを作成する
    pid_t child_pid;
    if ((child_pid = fork()) == 0) {
        // このブロック中はすでに子プロセスの処理である。
        // よくexecファミリ系が用いられるが今回はプロセスとして単に関数を走らせる。
        child_process(shm_id, sem_id);
    } else {
        // 親プロセスの処理
        parent_process(shm_id, sem_id);
        // 子プロセスの終了を待つ
        wait(NULL);
        // 最終的に共有メモリの値がどうなったかを知る
        int* shm = (int*)shmat(shm_id, NULL, 0);
        printf("合計 = %d\n", *shm);
    }
}

void parent_process(int shm_id, int sem_id) {
    // まずは共有メモリIDであるshm_idを使ってメモリマッピング
    int* shm = (int*)shmat(shm_id, NULL, 0);
    // 共有データをN回インクリメントする
    for (int i = 0; i < N; i++) {
        reserve_sem(sem_id); // セマフォを獲得する
        (*shm)++; // クリティカルリージョン
        release_sem(sem_id); // セマフォを解放する
    }
}

void child_process(int shm_id, int sem_id) {
    // まずは共有メモリIDであるshm_idを使ってメモリマッピング
    int* shm = (int*)shmat(shm_id, NULL, 0);
    // 共有データをN回インクリメントする
    for (int i = 0; i < N; i++) {
        reserve_sem(sem_id); // セマフォを獲得する
        (*shm)++; // クリティカルリージョン
        release_sem(sem_id); // セマフォ解放する
    }
}

int new_shm() {
    /**
     * int shmget(key_t key, size_t size, int shmflg)
     * 共有メモリの新規作成とオープン
     * shmget()は共有メモリを新規作成、または既存の共有メモリのIDを返す
     * プロセス間通信のIDはファイルディスクリプタに相当する
     *
     * 第1引数: IPC_PRIVATEで一意のキー(ファイルディスクリプタに似たようなもの)が作成できる。
     * 第2引数: 共有メモリのサイズ。今回はint型のサイズでよい。
     * 第3引数: パーミッションの設定。今回はユーザが読み書き可能であるフラグを立てる。
     *
     * このID(shm_id)を各プロセスからアタッチ(マッピング)して読み書きすれば良い。
     */
    int shm_id = shmget(IPC_PRIVATE, sizeof(int), S_IRUSR | S_IWUSR);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }
    return shm_id;
}

int new_sem(int val) {
    /**
     * int semget(key_t key, int nsems, int semflg)
     * セマフォの新規作成をする関数
     *
     * 第1引数: IPC_PRIVATEで一意のキー(ファイルディスクリプタに似たようなもの)が作成できる。
     * 第2引数: セマフォの個数 (今回は1つのセマフォを作成したい)
     * 第3引数: パーミッションの設定。ユーザがセマフォ操作(読み書き)可能であるフラグを立てる。
     * 戻り値: 成功時はセマフォセットのID、エラー時は-1
     *
     * 以下のコードにより。sem_getにより作成される。
     */
    int sem_id = semget(IPC_PRIVATE, 1, S_IRUSR | S_IWUSR);
    if (sem_id == -1) {
        perror("semget");
        exit(1);
    }
    /**
     * semget()ではセマフォの新規作成をするが、初期化は行っていない。
     * semctl()でSETVALもしくはSETALLにより初期化できる。
     * int semctl(int semid, int semnum, int cmd, union semun arg)
     * セマフォ制御をする関数
     *
     * 第1引数: semgetで作成したセマフォオブジェクトのID
     * 第2引数: セマフォセットのインデックス(今回は1つのセマフォなので0)
     * 第3引数: どの操作をするか(今回は単純に初期化をしたいのでSETVAL)
     * 第4引数: (argが必要かどうかは行う制御によるが初期化では使う)
     * 戻り値: 成功時は非負の整数、エラー時は-1
     */
    // セマフォの初期化に使う共用体
    union semun arg;
    arg.val = val; // バイナリセマフォなので1
    if (semctl(sem_id, 0, SETVAL, arg) == -1) {
        perror("semctl");
        exit(1);
    }
    return sem_id;
}

void reserve_sem(int sem_id) {
    struct sembuf sem_op;
    // sem_idにおける0番目が今回使うセマフォ
    sem_op.sem_num = 0;
    // down操作をしたいので-1を入れておく
    sem_op.sem_op = -1;
    // セマフォのUNDOを使用しないこととし、0に設定
    sem_op.sem_flg = 0;
    /**
     * sem_idにおいてsem_opの操作(down)をする
     * 第3引数はセマフォセットの大きさである。
     * 1つのセマフォしか作っていないので1でよい。
     */
    semop(sem_id, &sem_op, 1);
}

void release_sem(int sem_id) {
    struct sembuf sem_op;
    // sem_idにおける0番目が今回使うセマフォ
    sem_op.sem_num = 0;
    // up操作をしたいので1を入れておく
    sem_op.sem_op = 1;
    // セマフォのUNDOを使用しないこととし、0に設定
    sem_op.sem_flg = 0;
    /**
     * sem_idにおいてsem_opの操作(up)をする
     * 第3引数はセマフォセットの大きさである。
     * 1つのセマフォしか作っていないので1でよい。
     */
    semop(sem_id, &sem_op, 1);
}
