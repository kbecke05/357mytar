#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<dirent.h>
#include<string.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define ARR_SIZE 10
#define NAME_MAX 256
#define PATH_SIZE 1024

/* Header for ustar-format tar archive.  See the documentation of
   the "pax" utility in [SUSv3] for the the "ustar" format
   specification. */
struct ustar_header
  {
    char name[100];             /* File name.  Null-terminated if room. */
    char mode[8];               /* Permissions as octal string. */
    char uid[8];                /* User ID as octal string. */
    char gid[8];                /* Group ID as octal string. */
    char size[12];              /* File size in bytes as octal string. */
    char mtime[12];             /* Modification time in seconds
                                   from Jan 1, 1970, as octal string. */
    char chksum[8];             /* Sum of octets in header as octal string. */
    char typeflag;              /* An enum ustar_type value. */
    char linkname[100];         /* Name of link target.
                                   Null-terminated if room. */
    char magic[6];              /* "ustar\0" */
    char version[2];            /* "00" */
    char uname[32];             /* User name, always null-terminated. */
    char gname[32];             /* Group name, always null-terminated. */
    char devmajor[8];           /* Device major number as octal string. */
    char devminor[8];           /* Device minor number as octal string. */
    char prefix[155];           /* Prefix to file name.
                                   Null-terminated if room. */
    char padding[12];           /* Pad to 512 bytes. */
  }
header;

int CFLAG, TFLAG, XFLAG, VFLAG, SFLAG;

void archive_options(char * argv[]);
int create_archive_file(char * filename, char * argv[]);
struct ustar_header make_header();
/*void write_file_info(int fd, char *filearr[]);*/
int fill_filearr(int argc, char*argv[], int filearrsize, char ** filearr);
int directory_recurse(char * path, int arr_idx, int filearrsize, char ** filearr);
void print_filearr(char ** filearr, int size);
void list_archive(char *file_name);


int main(int argc, char*argv[]) {
    int outfd;
    char u_mess[] = "usage: mytar[ctxvS]f tarfile [path[...]]";
    char ** filearr = (char **) malloc(ARR_SIZE * sizeof(char *));
    int filearrsize = ARR_SIZE;
    
    CFLAG = TFLAG = XFLAG = VFLAG = SFLAG = 0;

    archive_options(argv);
    if(CFLAG){
      create_archive_file(argv[2], argv);
    }
    if(TFLAG){
      list_archive();
    }
  
    filearrsize = fill_filearr(argc, argv, filearrsize, filearr);
    print_filearr(filearr, filearrsize);
    /*if(CFLAG){
      create_archive_file(argv[2]);
    }
    if(argc > 1){
      
    }
    else{
      write(outfd, u_mess);
      return -1;
    }
    return 0;*/
}

/*void write_file_info(int fd, char *filearr[]){
  struct stat *file_stats;
  unsigned i = 0;
  while(i < ARR_SIZE && filearr[i] != NULL){
    lstat(file_arr[i], file_stats);
    write(fd, file_arr[i]);
    write(fd, file_stats->st_mode);
  }
  
  
}*/

void list_archive(){
  
}

void archive_options(char * argv[]){
  unsigned i;
  for(i = 0; i < strlen(argv[1]); i++){
    if(*(argv[1] + i) == 'c'){
      CFLAG = 1;
    }
    else if(*(argv[1] + i) == 't'){
      TFLAG = 1;
    }
    else if(*(argv[1] + i) == 'x'){
      XFLAG = 1;
    }
    else if(*(argv[1] + i) == 'v'){
      VFLAG = 1;
    }
    else if(*(argv[1] + i) == 'S'){
      SFLAG = 1;
    }
  }
}

int create_archive_file(char * filename, char * argv[]) {
    int fd, size;
    char *msg;

    fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRWXU);

    if (fd < 0) {
        char notOpen[] = "Can not open archive file";
        size = sizeof(notOpen) + sizeof(argv[1]) + 2;
        msg = (char *)malloc(size * sizeof(char));
        snprintf(msg, size, "%s %s", notOpen, argv[1]);
        perror(msg);
        free(msg);
        exit(-1);
    }
    return fd; 
}

int fill_filearr(int argc, char*argv[], int filearrsize, char ** filearr) {
    int i = 3;
    char fullpath[PATH_SIZE+1];
    strcpy(fullpath, argv[i]);
    for(i=3; i< (argc-1); i++) {
        filearrsize = directory_recurse(fullpath, i, filearrsize, filearr);
    }
    free(fullpath);
    return filearrsize;
}

int directory_recurse(char * path, int arr_idx, int filearrsize, char ** filearr) {
    DIR * dir = opendir(path);
    int curr_idx = 0;
    char * filename;
    struct dirent *pdirent;
    if (!dir) {
        perror("opendir");
    }
    while ((pdirent = readdir(dir))) {
        /*if curr is a directory*/
        if (pdirent ->d_type == DT_DIR) {
            /*skip current and parent directories*/
            if (strcmp(pdirent -> d_name, ".") == 0 || 
                strcmp(pdirent -> d_name, "..") == 0) {
                  continue;
            }
          directory_recurse(strcat(path, pdirent -> d_name), arr_idx+ curr_idx, filearrsize, filearr); 
        }
        /*if curr is a regular file or symbolic link*/
        if (pdirent ->d_type == DT_REG || pdirent ->d_type == DT_LNK) {

            /*check if more space needs to alloc'd */
            if ((arr_idx + curr_idx) > filearrsize) {
                filearrsize = filearrsize + ARR_SIZE;
                filearr = (char**) realloc (filearr, filearrsize*sizeof(char));
                if (filearr == NULL) {
                    perror("realloc");
                    exit(-1);
                }
            }

            /*add filename to arr*/
            filename = (char*)malloc(NAME_MAX);
            filename = pdirent -> d_name;
            filearr[arr_idx] = filename;
            curr_idx++;
        }
        /*throw an error for any other file types*/
        else {
            perror("unsupported file type");
            exit(EXIT_FAILURE);
        }
    }
    return filearrsize;
}



void print_filearr(char ** filearr, int size) {
  int i;
  for (i=0; i<size; i++) {
    printf("%s\n", filearr[i]);
  }
}