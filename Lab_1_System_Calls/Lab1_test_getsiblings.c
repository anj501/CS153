#include "types.h"
#include "user.h"

void getSibling(void){
    int pid1, pid2;
    pid1 = fork();
    if (pid1 == 0) {
        sleep(10);
    } else {
        printf(1, "sibling 1 pid: %d\n", pid1);
        pid2 = fork();
        if (pid2 == 0) {
            // check if the sibling is pid1
            printf(1, "getsiblings pid %d\n", getsiblings());
            if (pid1 == getsiblings()) {
                printf(1, "sibling pid matches with sibling1\n");
            } else {
                printf(1, "sibling pid DOES NOT matches with sibling1\n");
            }
        }
    }
    wait(0);
    wait(0);
    return;
}
// test

int main(int argc, char *argv[])
{
    // printf(1, "\nlab1 part1: test getsiblings\n");
    getSibling();
    exit(0);
    // return 0;
}