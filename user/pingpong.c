#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    char ballbyte = 123;
    // read - write
    int pipe_to_c[2];
    int pipe_to_p[2];
    if (pipe(pipe_to_c) != 0 && pipe(pipe_to_p) != 0) {
        printf("Pipe Error!");
        return 1;
    }
    int pid = fork();
    if (pid) {
        // Parent
        int mypid = getpid();
        close(pipe_to_c[0]);
        close(pipe_to_p[1]);
        write(pipe_to_c[1], &ballbyte, 1);
        wait(&pid);
        read(pipe_to_p[0], &ballbyte, 1);
        printf("%d: received pong\n", mypid);
    } else {
        // Child
        int mypid = getpid();
        close(pipe_to_c[1]);
        close(pipe_to_p[0]);
        read(pipe_to_c[0], &ballbyte, 1);
        printf("%d: received ping\n", mypid);
        write(pipe_to_p[1], &ballbyte, 1);
    }
    return 0;
}
