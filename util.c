#include "util.h"

void * calloc_error_check(size_t numelements, size_t elementsize) {
    void * memptr;

    memptr = calloc(numelements, elementsize);
    /*error check*/
    if (memptr == NULL) {
        perror("Cannot calloc");
        exit(EXIT_FAILURE);
    }
    return memptr;
}

void * malloc_error_check(size_t size) {
    void * memptr;

    memptr = malloc(size);
    /*error check*/
    if (memptr == NULL) {
        perror("Cannot malloc");
        exit(EXIT_FAILURE);
    }
    return memptr;
}

FILE * fopen_file_error_check(char * filename, char* permissions) {
    FILE * file;

    file = fopen(filename, permissions);

    if (!file) {
        perror("cannot open file");
        exit(EXIT_FAILURE);
    }
    return file; 
}

void * realloc_error_check(void * ptr, size_t size) {
    void * memptr;

    memptr = realloc(ptr, size);
    /*error check*/
    if (memptr == NULL) {
        perror("Cannot realloc");
        exit(EXIT_FAILURE);
    }
    return memptr;
}

DIR *opendir_error_check(const char *name) {
    DIR * dir;
    int errno = 0;
    dir = opendir(name);

    if(!dir) {
        printf("Errno: %d\n", errno);
        perror("Cannot open dir");
        exit(EXIT_FAILURE);
    }

    return dir;
}

/*void lstat_with_error(const char *path, struct stat *buf) {
    int res;

    res = lstat(path, &buf);

    if(res == -1) {
        perror("lstat failed");
        exit(EXIT_FAILURE);
    }
}*/