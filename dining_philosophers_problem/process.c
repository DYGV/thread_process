#include <unistd.h>
#include <sys/wait.h>
#include "dpp.h"

int main(void) {
    int i;
    init_philosophers();
    for (i = 0; i < N; i++) {
        if (fork() == 0) {
            philosopher(&i);
            _exit(0);
        }
    }
    for (i = 0; i < N; i++) {
        wait(NULL);
    }
    return 0;
}
