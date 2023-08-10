#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char* path, char* filename) {
    char buf[512];
    char* p;
    int fd;
    struct stat st;  // Stat of the file descriptor. Who is it, what is it, where is it?
    struct dirent de;  // DIRectory ENTity, holding the name and the position of a Directory in the Filesystem as it is in the FS
    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    switch(st.type) {
        case T_DEVICE:
        case T_FILE:
            // Get the file name "splitting" the string at char /
            int lastslash;
            for(lastslash = strlen(path)-1; lastslash >= 0; lastslash--) {
                if (path[lastslash] == '/') break;
            }
            // Copy the filename in the buffer
            strcpy(buf, &path[lastslash+1]);
            // If the filename in the buffer is equal to the searched filename then print
            if (strcmp(buf, filename) == 0){
                printf("%s\n", path);
            }
            break;
        case T_DIR:
            // Check path length
            if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
                printf("find: path too long\n");
                break; // TODO: or return?
            }
            strcpy(buf, path);
            p = buf + strlen(buf);
            *p++ = '/';  // Write '/' in the old \0 of the string and then increment the cursor so that it points to the first empty cell
            while(read(fd, &de, sizeof(de)) == sizeof(de)) {  // A DIR contains sequentially a list of dirent struct, one per folder
                if(de.inum == 0) continue;  // The folder slot is empty
                if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;  // Skip . and ..
                memmove(p, de.name, DIRSIZ);  // Copy directory name in the path buf using the cursor p
                p[DIRSIZ] = 0;  // Conclude the C-string with \0
                if(stat(buf, &st) < 0){
                    printf("find: cannot stat %s\n", buf);
                    continue;
                }
                find(buf, filename);
            }
            break;
    }
    close(fd);
}

int main(int argc, char *argv[]) {
    if(argc < 3){
        fprintf(2, "Usage: find <dir> <filename>\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}