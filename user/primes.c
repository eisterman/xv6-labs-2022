#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    // read - write
    int pipeline[70];  // 35 pipes max
    int pipeoffset = 0;
    pipe(&pipeline[0]);
    int pid = fork();
    if (pid) {
        // global parent
        close(pipeline[0]);
        for(int i = 2; i <= 35; i++) {
            write(pipeline[1], &i, 4);
        }
        close(pipeline[1]);
        wait(&pid);
    } else {
        // child
        close(pipeline[pipeoffset+1]);
        child_head:
        int handled_number = 0;
        int is_forked = 0;
        int n;
        while (read(pipeline[pipeoffset], &n, 4)) {
            if (n > 35) {
                break;
            }
            if (handled_number == 0) {
                handled_number = n;
                printf("prime %d\n", n);
                continue;
            }
            if (n % handled_number == 0) {
                continue;
            } else {
                if (is_forked == 0) {
                    pipe(&pipeline[pipeoffset+2]);
                    pid = fork();
                    if (pid) {
                        // still himself
                        is_forked = 1;
                        close(pipeline[pipeoffset+2]);
                    } else {
                        // new child
                        close(pipeline[pipeoffset]);  // Parent's Read pipe (granparent->parent)
                        close(pipeline[pipeoffset+3]); // Myself's Write pipe (parent->myself)
                        pipeoffset += 2;
                        goto child_head;
                    }
                }
                write(pipeline[pipeoffset+3], &n, 4);
            }
        }
        // The pipe has been closed! Close the read for the upstream pipe and close the write for the downstream one
        if (handled_number == 0) {
            // This is the full terminator of the pipeline! You need just to close the read pipe.
            close(pipeline[pipeoffset]);
        } else {
            // Just a pipelined child. Remove the connection to the returnpipe
            close(pipeline[pipeoffset]);
            close(pipeline[pipeoffset+3]);
        }
        if (pid) {
            wait(&pid);
            exit(0);
        } else {
            exit(0);
        }
    }
    return 0;
}
