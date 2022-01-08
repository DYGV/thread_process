/**
 * @file process_producer_consumer_problem.c
 * 生産者・消費者問題のプロセスバージョン
 */

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include "pcp.h"

int main() {
    // 商品棚とその空き(共有資源)の設定は1回でよい
    // 複数いる生産者と消費者はこの商品棚を使う
    init_pcp();
    // それぞれのプロセスでexitしないとそれぞれの子プロセスでforkを呼ぶことを繰り返してしまうので注意(プログラム開始時のプロセス以外はループ処理をしないようにする)
    for (int i = 0; i < 8; i++) {
        if (fork() == 0) {
            producer();
            exit(0);
        } else if (fork() == 0) {
            consumer();
            exit(0);
        }
    }
    for (int i = 0; i < 16; i++) {
        wait(NULL);
    }
    return 0;
}
