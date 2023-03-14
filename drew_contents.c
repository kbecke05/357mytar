#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <utime.h>

#include "contents.h"
#include "util.h"

#define ONE_BYTE 1

int is_prefix(char *pref, char *full){
    unsigned i;
    for(i = 0; i < strlen(pref); i++){
        if(*(pref + i) != *(full + i)){
            return 0;
        }
    }
    if(*(full + i) == '\0' || *(full + i) == '/')
        return 1;
    return 0;
}

void write_mtime(int fd, unsigned offset){
    char mtime[MTIME_SIZE];
    time_t mtime_num;
    struct tm *mtime_tm;
    lseek(fd, offset + MTIME, SEEK_SET);
    read(fd, mtime, MTIME_SIZE);
    mtime_num = (time_t)strtol(mtime, NULL, BASE8);
    mtime_tm = localtime(&mtime_num);
    strftime(mtime, sizeof(mtime), "%x - %I:%M%p", mtime_tm);
    printf("%16s", mtime); 
}

char *write_uid(int fd, unsigned offset){
    char *uid = (char *)malloc(UID_SIZE*sizeof(char));
    lseek(fd, offset + UID, SEEK_SET);
    read(fd, uid, UID_SIZE);
    return uid;
}

char *write_gid(int fd, unsigned offset){
    char *gid = (char *)malloc(GID_SIZE*sizeof(char));
    lseek(fd, offset + GID, SEEK_SET);
    read(fd, gid, GID_SIZE);
    return gid;
}

long int get_size(int fd, unsigned offset){
    char size[SIZE_SIZE];
    long int size_num;
    lseek(fd, offset + SIZE, SEEK_SET);
    read(fd, size, SIZE_SIZE);
    size_num = strtol(size, NULL, BASE8);
    return size_num;
}

void write_owner_group(int fd, unsigned offset){
    char *owner = (char *)malloc(UNAME_SIZE*sizeof(char));
    char *group = (char *)malloc(GNAME_SIZE*sizeof(char));
    lseek(fd, offset + UNAME, SEEK_SET);
    read(fd, owner, UNAME_SIZE);
    if(!strlen(owner))
        owner = write_uid(fd, offset);
    lseek(fd, offset + GNAME, SEEK_SET);
    read(fd, group, GNAME_SIZE);
    if(!strlen(group))
        group = write_gid(fd, offset);
    if(strlen(owner) < 17)
        strncat(owner, group, 17 - strlen(owner));
    printf("%-17s ", owner);

}

char get_type(int fd, unsigned offset){
    char type;
    lseek(fd, offset + TYPEFLAG, SEEK_SET);
    read(fd, &type, ONE_BYTE);
    return type;
}

void write_permissions(char mode[], int fd, unsigned offset){
    long int mode_num;
    char type;
    mode_num = strtol(mode, NULL, BASE8);
    type = get_type(fd, offset);
    if(type == '5')
        putchar('d');
    else if(type == '2')
        putchar('l');
    else
        putchar('-');

    if(mode_num & S_IRUSR)
        putchar('r');
    else
        putchar('-');
    if(mode_num & S_IWUSR)
        putchar('w');
    else
        putchar('-');
    if(mode_num & S_IXUSR)
        putchar('x');
    else
        putchar('-');
    if(mode_num & S_IRGRP)
        putchar('r');
    else
        putchar('-');
    if(mode_num & S_IWGRP)
        putchar('w');
    else
        putchar('-');
    if(mode_num & S_IXGRP)
        putchar('x');
    else
        putchar('-');
    if(mode_num & S_IROTH)
        putchar('r');
    else
        putchar('-');
    if(mode_num & S_IWOTH)
        putchar('w');
    else
        putchar('-');
    if(mode_num & S_IXOTH)
        putchar('x');
    else
        putchar('-');

    putchar(' ');
    
}

void list_files(int fd, int vflag, char *given, int specific, int extract){
    int read_val;
    long int size;
    long int num_blocks;
    unsigned offset;
    char name[NAME_SIZE];
    char mode[MODE_SIZE];
    char type;
    int prefix;
    struct stat statbuf;
    struct utimbuf times;
    struct timespec mtime;

    prefix = 1;
    offset = 0;
    while((read_val = read(fd, name, NAME_SIZE)) > 0){
        if(!strlen(name)){
            break;
        }
        /*if(extract){
            stat(name, &statbuf);
            printf("mtime %d\n", statbuf.st_mtime);
        }*/
        size = get_size(fd, offset);
        if(specific){
            prefix = is_prefix(given, name);
        }
        if(vflag && prefix && !extract){
            lseek(fd, offset + MODE, SEEK_SET);
            read_val = read(fd, mode, MODE_SIZE);
            write_permissions(mode, fd, offset);
            write_owner_group(fd, offset);
            printf("%8ld ", size);
            write_mtime(fd, offset);
        }
        if(prefix){
            if((vflag && extract) || !extract)
                printf("%s\n", name);
        }
        type = get_type(fd, offset);
        if(type == '0' || type == '\0'){
            num_blocks = (size / BLOCK_SIZE) + 1;
            offset += BLOCK_SIZE;
            offset += (num_blocks * BLOCK_SIZE);
        }
        else
            offset += BLOCK_SIZE;
        lseek(fd, offset, SEEK_SET);
        /*if(extract){
            times.modtime = statbuf.st_mtime;
            utime(name, &times);
            stat(name, &statbuf);
            printf("restored mod time = %d\n", statbuf.st_mtime);
        }*/
    }
    if(read_val == -1){
        perror("Read");
        exit(EXIT_FAILURE);
    }
    lseek(fd, 0, SEEK_SET);
}
