/**
 * @file process_producer_consumer_problem.c
 * 生産者・消費者問題のプロセスバージョン
 */

#include <sys/wait.h>
#include <unistd.h>
#include "pcp.h"

int main() {
    init_pcp();
    if (fork() == 0) {
        consumer();
    } else {
        producer();
        wait(NULL);
    }
    return 0;
}
