#ifndef UTIL
#define UTIL

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <libgen.h>
#include <wchar.h>
#include <arpa/inet.h>
#include <utime.h>

#define NAME 0
#define MODE 100
#define UID 108
#define GID 116
#define SIZE 124
#define MTIME 136
#define CHKSUM 148
#define TYPEFLAG 156
#define LINKNAME 157
#define MAGIC 257
#define VERSION 263
#define UNAME 265
#define GNAME 297
#define DEVMAJOR 329
#define DEVMINOR 337
#define PREFIX 345
#define BASE8 8

#define BLOCK_SIZE 512

#define NAME_SIZE 100
#define MODE_SIZE 8
#define UID_SIZE 8
#define GID_SIZE 8
#define SIZE_SIZE 12
#define MTIME_SIZE 12
#define CHKSUM_SIZE 8
#define TYPEFLAG_SIZE 1
#define LINKNAME_SIZE 100
#define MAGIC_SIZE 6
#define VERSION_SIZE 2
#define UNAME_SIZE 32
#define GNAME_SIZE 32
#define DEVMAJOR_SIZE 8
#define DEVMINOR_SIZE 8
#define PREFIX_SIZE 155
#define PADDING_SIZE 12

#define SPACE 32
#define MAGIC_VAL "ustar\0"
#define VERSION_VAL "00"

#define REGFILE 0
#define SYMLINK 2
#define DIRECTORY 5

typedef struct ustar_header * HeaderPtr;
typedef struct ustar_header
{
    char name[NAME_SIZE];      /* File name.  Null-terminated if room. */
    char mode[MODE_SIZE];       /* Permissions as octal string. */
    char uid[UID_SIZE];         /* User ID as octal string. */
    char gid[GID_SIZE];       /* Group ID as octal string. */
    char size[SIZE_SIZE]; /* File size in bytes as octal string. */
    char mtime[MTIME_SIZE];     /* Modification time in seconds
                                    from Jan 1, 1970, as octal string. */
    char chksum[CHKSUM_SIZE]; /* Sum of octets in header as octal string. */
    char typeflag[TYPEFLAG_SIZE];     /* An enum ustar_type value. */
    char linkname[LINKNAME_SIZE];         /* Name of link target.
                                    Null-terminated if room. */
    char magic[MAGIC_SIZE];              /* "ustar\0" */
    char version[VERSION_SIZE];            /* "00" */
    char uname[UNAME_SIZE]; /* User name, always null-terminated. */
    char gname[GNAME_SIZE]; /* Group name, always null-terminated. */
    char devmajor[DEVMAJOR_SIZE]; /* Device major number as octal string. */
    char devminor[DEVMINOR_SIZE]; /* Device minor number as octal string. */
    char prefix[PREFIX_SIZE];           /* Prefix to file name.
                                    Null-terminated if room. */
    char padding[PADDING_SIZE];   /* Pad to 512 bytes. */
} Header;

void * calloc_error_check(size_t numelements, size_t elementsize);
void * malloc_error_check(size_t size);
FILE * fopen_file_error_check(char * filename, char* permissions);
void * realloc_error_check(void * ptr, size_t size);
DIR *opendir_error_check(const char *name);
void lstat_with_error(const char *path, struct stat *buf);

#endif
