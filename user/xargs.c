#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

const uint BUFSIZE = 512;

int main(int argc, char *argv[]) {
    if(argc < 2){
        fprintf(2, "Usage: xargs <command & arguments>\n");
        exit(1);
    }
    // Read STDIN inside the buffer
    vector* stdins = vector_init();
    vector* inputbuffer = vector_init();
    char bufchr;
    int readed;
    while ((readed = read(0, &bufchr, 1))) {
        // printf("%c - %x - %d\n", bufchr, bufchr, readed);
        if (bufchr == '\n') {
            vector_byteappend(inputbuffer, 0);
            vector_append(stdins, &inputbuffer, sizeof(vector*));
            inputbuffer = vector_init();
            continue;
        }
        if (bufchr == 0) break;
        vector_byteappend(inputbuffer, bufchr);
    }
    if (inputbuffer->length > 0) {
        vector_byteappend(inputbuffer, 0);
        vector_append(stdins, &inputbuffer, sizeof(vector*));
    } else {
        vector_free(inputbuffer);
    }
    // Read the arguments from argv
    vector* argvs = vector_init();
    for(int i = 1; i < argc; i++) {
        vector* single_arg = vector_init();
        vector_append(single_arg, argv[i], strlen(argv[i]));
        vector_append(argvs, &single_arg, sizeof(vector*));
    }
    // Fork, parse stdin and launch!
    vector* pids = vector_init();
    int pid,stdidx;
    for(stdidx = 0; stdidx < stdins->length/sizeof(vector*); stdidx++) {
        pid = fork();
        if (pid) {
            vector_append(pids, &pid, sizeof(int));
        } else {
            break;
        }
    }
    if (pid) {
        // parent
        for(int j = 0; j < pids->length; j+=sizeof(int)) {
            wait(&VECTOR_RAWACCESS(stdins, int, j));  // *(int*)(pids->data+j*sizeof(int))
        }
    } else {
        // child
        vector* my_stdin = VECTOR_ACCESS(stdins, vector*, stdidx); //*(vector**)(stdins->data+i*sizeof(vector*)
        // Parse mystdin inside argvs
        vector* argbuffer = vector_init();
        for(int k = 0; k < my_stdin->length; k++) {
            char l = VECTOR_ACCESS(my_stdin, char, k);
            if (l == ' ') {
                vector_byteappend(argbuffer, 0);
                vector_append(argvs, &argbuffer, sizeof(vector*));
                argbuffer = vector_init();
                continue;
            }
            if (l == 0) break;
            vector_byteappend(argbuffer, l);
        }
        if (argbuffer->length > 0) {
            vector_byteappend(argbuffer, 0);
            vector_append(argvs, &argbuffer, sizeof(vector*));
        } else {
            vector_free(argbuffer);
        }
        // LAUNCH!
        // TODO: check if more than MAXARGS
        char** fin_argv = malloc(argvs->length/sizeof(vector*)*sizeof(char*));  // Get num of vector*. For every of them we need a char*
        for(int i = 0; i < argvs->length/sizeof(vector*); i++) {
            fin_argv[i] = VECTOR_ACCESS(argvs, vector*, i)->data;
        }
        exec(argv[1], fin_argv);
        exit(1);
    }
    exit(0);
}
