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
#define MTIME_WIDTH 18

int check_version(int fd, unsigned offset){
    char version[VERSION_SIZE];
    lseek(fd, offset + VERSION, SEEK_SET);
    read(fd, version, VERSION_SIZE);
    if(!strcmp("00", version))
        return 0;
    else 
        return -1;
}

void extract_data(int arch_fd, unsigned offset, int out_fd, 
                unsigned num_blocks){
    unsigned i;
    char *block_str = (char *)malloc(BLOCK_SIZE);

    for(i = 0; i < num_blocks; i++){
        lseek(arch_fd, offset, SEEK_SET);
        read(arch_fd, block_str, BLOCK_SIZE);
        write(out_fd, block_str, strlen(block_str));
        offset += BLOCK_SIZE;
    }
    free(block_str);
}

char *get_xname(char *name){
    char *xname;
    unsigned i;
    int marker = -1;
    for(i = 0; i < strlen(name); i++){
        if(*(name + i) == '/'){
            marker = i;
        }
    }
    if(marker == -1)
        return name;
    xname = (char *)malloc(strlen(name) - marker);
    marker++;
    i = 0;
    while(marker < strlen(name)){
        *(xname + i) = *(name + marker);
        marker++;
        i++;
    }
    *(xname + i) = '\0';
    return xname;
}

char *get_prefix(int fd, unsigned offset){
    char *pref = (char *)malloc(PREFIX_SIZE);
    lseek(fd, offset + PREFIX, SEEK_SET);
    read(fd, pref, PREFIX_SIZE);
    return pref;
}

int is_prefix(char *pref, char *full){
    unsigned i;
    for(i = 0; i < strlen(pref); i++){
        if(*(pref + i) != *(full + i)){
            return 0;
        }
    }
    if(*(pref + i - 1) == '/')
        return 1;
    if(*(full + i) == '\0' || *(full + i) == '/')
        return 1;
    return 0;
}

char *get_mtime(int fd, unsigned offset){
    char *mtime = (char *)malloc(MTIME_SIZE);
    
    lseek(fd, offset + MTIME, SEEK_SET);
    read(fd, mtime, MTIME_SIZE);
    return mtime;
}

void write_mtime(int fd, unsigned offset){
    char *mtime;
    time_t mtime_num;
    struct tm mtime_tm;
    struct tm *mtime_ptr;
    char time_str[MTIME_WIDTH];

    mtime_ptr = &mtime_tm;
    mtime = get_mtime(fd, offset);
    mtime_num = (time_t)strtol(mtime, NULL, BASE8);
    mtime_ptr = localtime(&mtime_num);
    strftime(time_str, MTIME_WIDTH, "%Y-%m-%d %H:%M", mtime_ptr);
    printf("%16s ", time_str);
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
    if(!strlen(owner)){
        free(owner);
        owner = write_uid(fd, offset);
    }
    strncat(owner, "/", strlen("/"));
    lseek(fd, offset + GNAME, SEEK_SET);
    read(fd, group, GNAME_SIZE);
    if(!strlen(group)){
        free(group);
        group = write_gid(fd, offset);
    }
    if(strlen(owner) < 17)
        strncat(owner, group, 17 - strlen(owner));
    printf("%-17s ", owner);
    free(owner);
    free(group);

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

void iterate_arch(int fd, int vflag, char *args[], int num_args,
                int specific, int extract, int sflag){
    int read_val, xfd, prefix, i, version;
    long int size;
    unsigned num_blocks, offset;
    char *name;
    char *pref_str;
    char *mtime_str;
    char *xname;
    char mode[MODE_SIZE];
    char type;
    struct utimbuf times;
    time_t mtime_num;

    offset = 0; 
    if(sflag){
        while((version = check_version(fd, offset)) == -1){
            offset += BLOCK_SIZE;
        }
    }

    name = (char *)malloc(NAME_SIZE);

    prefix = 1;
    while((read_val = read(fd, name, NAME_SIZE)) > 0){
        if(!strlen(name)){
            break;
        }
        if(strlen(pref_str = get_prefix(fd, offset))){
            strncat(pref_str, "/", strlen("/"));
            strncat(pref_str, name, NAME_SIZE);
        }
        size = get_size(fd, offset);
        type = get_type(fd, offset);
        if(specific){
            if(strlen(pref_str)){
                for(i = 3; i < num_args; i++){
                    prefix = is_prefix(args[i], pref_str);
                    if(prefix)
                        break;
                }
            }
            else{
                for(i = 3; i < num_args; i++){
                    prefix = is_prefix(args[i], name);
                    if(prefix)
                        break;
                }
            }
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
            if((vflag && extract) || !extract){
                if(strlen(pref_str)){
                    printf("%s\n", pref_str);
                }
                else{
                    printf("%s\n", name);
                }
            }
        }
        if(type == '0' || type == '\0'){ 

            
            mtime_str = get_mtime(fd, offset);
            mtime_num = (time_t)strtol(get_mtime(fd, offset), 
                        NULL, BASE8);
            free(mtime_str);
            if((size % BLOCK_SIZE) == 0)
                num_blocks = (size / BLOCK_SIZE);
            else
                num_blocks = (size / BLOCK_SIZE) + 1;
            offset += BLOCK_SIZE;
            if(extract && prefix){
                if(strlen(pref_str))
                    xname = get_xname(pref_str);
                else
                    xname = get_xname(name);
                xfd = open(xname, O_WRONLY | O_CREAT | O_TRUNC, 
                S_IRUSR | S_IWUSR);
                times.modtime = mtime_num;
                utime(xname, &times);
                extract_data(fd, offset, xfd, num_blocks);
                free(xname);
            }
            offset += (num_blocks * BLOCK_SIZE);
        }
        else{
            offset += BLOCK_SIZE;
        }
        
        lseek(fd, offset, SEEK_SET);
        free(pref_str);
    }
    free(name);
    if(read_val == -1){
        perror("Read");
        exit(EXIT_FAILURE);
    }
    lseek(fd, 0, SEEK_SET);
}
